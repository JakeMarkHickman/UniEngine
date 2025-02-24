
#include <stdio.h>
#include "ECS.h"
#include "DeltaTime.h"
#include <iostream>

struct Transform
{
	Transform(float x, float y, float z) : x(x), y(y), z(z) {};

	float x, y, z;
};

void anyTransformSystem(anyECS& ecs, float deltaTime)
{
	for (int i = 0; i < ecs.GetAllEntities().size(); i++)
	{
		if (ecs.HasComponents<Transform>(i))
		{
			Transform transform = ecs.GetComponent<Transform>(i);

			transform.x += 5;

			ecs.SetComponent<>(i, transform);
		}
	}
}

void memoryTransformSystem(memoryECS& ecs, float deltaTime)
{
	for (int i = 0; i < ecs.GetAllEntities().size(); i++)
	{
		if (ecs.HasComponents<Transform>(i))
		{
			Transform* transform = ecs.GetComponent<Transform>(i);

			transform->x += 5;

		}
	}
}

void main()
{
	bool memorypool = false;
	int entityAmount = 10000;

	DeltaTime time;

	if (memorypool)
	{
		memoryECS ecs;

		for (int i = 0; i < entityAmount; i++)
		{
			uint64_t ent = ecs.CreateEntity();
			ecs.AttachComponent(ent, Transform(1.0f, 0.0f, 1.0f));
		}


		ecs.AddSystem(memoryTransformSystem);

		while (true)
		{
			float delta = time.CalculateDeltaTime();
			std::cout << delta << "\n";
			ecs.UpdateSystems(delta);
		}
	}
	else
	{
		anyECS ecs;

		for (int i = 0; i < entityAmount; i++)
		{
			uint64_t ent = ecs.CreateEntity();
			ecs.AttachComponents(ent, Transform(1.0f, 0.0f, 1.0f));
		}

		ecs.AddSystem(anyTransformSystem);

		while (true)
		{
			float delta = time.CalculateDeltaTime();
			std::cout << delta << "\n";
			ecs.UpdateSystems(delta);
		}
	}
}