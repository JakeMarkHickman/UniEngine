#pragma once

#include <stdexcept>
#include <cstddef>

class memoryPool
{
public:
    //TODO: allow for dynamic allocation of extra memory
    memoryPool(size_t chunkSize, size_t chunkCount);
    ~memoryPool();

    void* Allocate();
    void Deallocate(void* chunk);
private:

    size_t m_ChunkSize;
    size_t m_ChunkCount;
    void* m_Memory;
    void* m_FreeChunkList;

    void FreeChunkListInit();
};

