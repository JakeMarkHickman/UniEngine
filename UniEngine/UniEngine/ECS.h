#pragma once

#include <vector>
#include <typeindex>
#include <memory>
#include <any>
#include <unordered_map>
#include "SparseSet.h"
#include "memoryPool.h"


class anyECS;

using anySystemFunc = void (*)(anyECS&, float);

class anyECS
{
public:
        anyECS() {};
        ~anyECS() {};

        /////////////////////////////////////
        //              ENTITY             //
        /////////////////////////////////////

        std::vector<uint64_t> GetAllEntities() { return m_Entities; };

        uint64_t CreateEntity();

        void RemoveEntity(uint64_t entityToRemove);


        /////////////////////////////////////
        //             COMPONENT           //
        /////////////////////////////////////

        SparseSet<SparseSet<std::any>> GetEntityComponents() { return m_EntityComponents; }

        //Add Component(s) to an Entity
        template<typename... Components>
        void AttachComponents(uint64_t entity, const Components&... comp)
        {
            ([&] {
                uint64_t typeHash = typeid(comp).hash_code(); // Gets the Hash code of the data parsed for the sparse set
                uint64_t CompID;

                if (!IsRegisteredComponent(typeHash))
                {
                    CompID = RegisterComponent(typeHash); // Register the component

                    SparseSet<std::any> newCompSet; // Create a new sparse set
                    newCompSet.Insert(entity, std::any(comp)); // populate the new sparse set

                    m_EntityComponents.Insert(CompID, newCompSet); // add the new set to the already existing one
                }
                else
                {
                    CompID = GetComponentIndex(typeHash);

                    SparseSet<std::any> CompSet = m_EntityComponents.GetData(CompID);

                    CompSet.Insert(entity, std::any(comp));

                }
                }(), ...);
        }

        //Remove Component(s) from an Entity
        template<typename... Components>
        void RemoveComponents(uint64_t entity, Components... comp)
        {
            //([&] {
                //do things
            //}(), ...);
        }

        template<typename... Components>
        bool HasComponents(uint64_t entity)
        {
            bool result = true;
            ([&] {
                uint64_t typeHash = typeid(Components).hash_code();

                if (!IsRegisteredComponent(typeHash))
                {
                    result = false;
                }

                //TODO: Check for component on entity
                uint64_t compID = GetComponentIndex(typeHash);

                if (!m_EntityComponents.GetData(compID).HasIndex(entity))
                {
                    result = false;
                }

                }(), ...);
            return result;
        }

        template<typename Component>
        Component GetComponent(uint64_t entity)
        {
            uint64_t typeHash = typeid(Component).hash_code();
            if (!IsRegisteredComponent(typeHash))
            {

            }
            uint64_t compID = GetComponentIndex(typeHash);
            try
            {
                return std::any_cast<Component>(m_EntityComponents.GetData(compID).GetData(entity));
            }
            catch (const std::bad_any_cast& e)
            {
                // Handle the case where the cast fails (e.g., wrong type)
                // Maybe log an error or throw an exception?
                // ...
                // For now, let's return a default-constructed Component:

            }
        }
        template<typename Component>
        void SetComponent(uint64_t entity, const Component& comp)
        {
            uint64_t typeHash = typeid(Component).hash_code();
            uint64_t compID = GetComponentIndex(typeHash);
            m_EntityComponents.GetData(compID).SetData(entity, comp);
        }

        /////////////////////////////////////
        //             SYSTEMS             //
        /////////////////////////////////////

        void UpdateSystems(float DeltaTime)
        {
            for (anySystemFunc& Func : m_Systems)
            {
                Func(*this, DeltaTime);
            }
        };

        void AddSystem(anySystemFunc System)
        {
            m_Systems.push_back(System);
        }

    private:

        uint64_t RegisterComponent(uint64_t hashCode)
        {
            uint64_t compID = m_NextComponent;

            m_RegisteredComponents.insert({ hashCode, compID });

            m_NextComponent++;

            return compID;
        }

        bool IsRegisteredComponent(uint64_t hashCode)
        {
            return m_RegisteredComponents.find(hashCode) != m_RegisteredComponents.end();
        }

        uint64_t GetComponentIndex(uint64_t hashCode)
        {
            return m_RegisteredComponents.at(hashCode);
        }

        std::vector<uint64_t> m_Entities;
        uint64_t m_NextEntity = 0;

        std::unordered_map<uint64_t, uint64_t> m_RegisteredComponents;
        uint64_t m_NextComponent = 0;

        SparseSet<SparseSet<std::any>> m_EntityComponents;

        std::vector<anySystemFunc> m_Systems;
};

class memoryECS;

using SystemFunc = void (*)(memoryECS&, float);

class memoryECS
{
public:
    memoryECS() {};
    ~memoryECS() {};

    /////////////////////////////////////
    //              ENTITY             //
    /////////////////////////////////////

    std::vector<uint64_t> GetAllEntities() { return m_Entities; };

    uint64_t CreateEntity()
    {
        uint64_t CreatedEntity;

        if (!m_RemovedEntities.empty()) //Check to see if there is any deleted entites
        {
            CreatedEntity = m_RemovedEntities.back();
            m_Entities.push_back(CreatedEntity);
            m_RemovedEntities.pop_back();

            return CreatedEntity;
        }

        m_Entities.push_back(m_NextEntity);
        CreatedEntity = m_NextEntity;
        m_NextEntity++;

        return CreatedEntity;
    };

    bool RemoveEntity(uint64_t entityToRemove)
    {
        for (size_t i = 0; i < m_Entities.size(); i++)
        {
            if (entityToRemove == m_Entities[i])
            {
                m_RemovedEntities.push_back(entityToRemove);
                m_Entities.erase(m_Entities.begin() + i); // because it needs a iterator for some reason
                return true;
            }
        }

        return false;
    };


    /////////////////////////////////////
    //             COMPONENT           //
    /////////////////////////////////////

    //Add Component(s) to an Entity
    template<typename Component>
    void AttachComponent(uint64_t entity, const Component comp)
    {
            uint64_t hashCode = typeid(Component).hash_code();
            uint64_t compID;

            if (!IsRegisteredComponent(hashCode))
            {
                compID = RegisterComponent(hashCode);

                // MemoryPool needs to be constructed inside the vector as moving data causes a crash
                m_ComponentPool.emplace_back(sizeof(Component), 10000); //TODO: Scale this with the amount of entities available

                SparseSet<void*> newCompSet;
                m_EntityComponents.emplace_back(newCompSet);
            }
            else
            {
                compID = GetComponentIndex(hashCode);
            }

            void* rawMemory = m_ComponentPool[compID].Allocate();

            if (!rawMemory)
            {
                throw std::runtime_error("Component Pool is full!");
            }

            Component* allocatedComp = new (rawMemory) Component(comp);

            m_EntityComponents[compID].Insert(entity, allocatedComp);
    }

    //Remove Component(s) from an Entity
    template<typename... Components>
    void RemoveComponents(uint64_t entity, Components... comp)
    {
        ([&] {
            uint64_t hashCode = typeid(Components).hash_code();
            uint64_t compID;

            if (!IsRegisteredComponent(hashCode))
            {
                return;
            }

            compID = GetComponentIndex(hashCode);

            void* compData = m_EntityComponents[compID].GetData(entity);

            if (compData)
            {
                // call the destructor of the data
                static_cast<Components*>(compData)->~Components();

                // Deallocate the memory from the pool
                m_ComponentPool[compID].Deallocate(compData);

                // Remove entity from sparse set
                m_EntityComponents[compID].Remove(entity);
            }
        }(), ...);
    }

    template<typename Component>
    bool HasComponents(uint64_t entity)
    {
        uint64_t hashCode = typeid(Component).hash_code();

        if (!IsRegisteredComponent(hashCode))
        {
            return false;
        }

        uint64_t compID = GetComponentIndex(hashCode);

        if (!m_EntityComponents[compID].HasIndex(entity))
        {
            return false;
        }

        return true;
    }

    template<typename Component>
    Component* GetComponent(uint64_t entity)
    {
        uint64_t hashCode = typeid(Component).hash_code();

        if (!IsRegisteredComponent(hashCode))
        {
            return nullptr;
        }

        uint64_t compID = GetComponentIndex(hashCode);

        return static_cast<Component*>(m_EntityComponents[compID].GetData(entity));
    }

    /////////////////////////////////////
    //             SYSTEMS             //
    /////////////////////////////////////

    void UpdateSystems(float DeltaTime)
    {
        for (SystemFunc& Func : m_Systems)
        {
            Func(*this, DeltaTime);
        }
    };

    void AddSystem(SystemFunc System)
    {
        m_Systems.push_back(System);
    }

private:

    uint64_t RegisterComponent(uint64_t hashCode)
    {
        uint64_t RegisteredComp = m_RegisteredComponents;

        m_ComponentRegisty.insert({ hashCode, RegisteredComp });
        m_RegisteredComponents++;

        return RegisteredComp;
    }

    bool IsRegisteredComponent(uint64_t hashCode)
    {
        return m_ComponentRegisty.find(hashCode) != m_ComponentRegisty.end();
    }

    uint64_t GetComponentIndex(uint64_t hashCode)
    {
        return m_ComponentRegisty.at(hashCode);
    }

    std::vector<uint64_t> m_Entities;
    std::vector<uint64_t> m_RemovedEntities;
    uint64_t m_NextEntity = 0;

    std::unordered_map<uint64_t, uint64_t> m_ComponentRegisty;
    std::vector<memoryPool> m_ComponentPool;
    std::vector<SparseSet<void*>> m_EntityComponents;
    uint64_t m_RegisteredComponents = 0;

    std::vector<SystemFunc> m_Systems;
};