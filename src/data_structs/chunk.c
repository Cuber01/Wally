#include "memory.h"
#include "chunk.h"



void initChunk(Chunk* chunk)
{
    chunk->codeCount = 0;
    chunk->codeCapacity = 0;
    chunk->lineCount = 0;
    chunk->lineCapacity = 0;
    chunk->code = NULL;
    chunk->lines = NULL;

    initValueArray(&chunk->constants);
}

void writeChunk(Chunk* chunk, uint8_t byte, unsigned int line)
{
    // Grow the array if we don't have enough space
    if (chunk->codeCapacity < chunk->codeCount + 1)
    {
        unsigned int oldCapacity = chunk->codeCapacity;
        chunk->codeCapacity = GROW_CAPACITY(oldCapacity);
        chunk->code = GROW_ARRAY(uint8_t, chunk->code, oldCapacity, chunk->codeCapacity);
    }

    chunk->code[chunk->codeCount] = byte;
    chunk->codeCount++;

    if(chunk->lineCapacity < chunk->lineCount + 1)
    {
        uint32_t oldCapacity = chunk->lineCapacity;
        chunk->lineCapacity = ((oldCapacity) < 32 ? 32 : (oldCapacity) * 2);
        chunk->lines = (uint32_t*)reallocate(chunk->lines, sizeof(uint32_t) * (oldCapacity), sizeof(uint32_t) * (chunk->lineCapacity));
    }

    chunk->lines[chunk->lineCount] = line;
    chunk->lineCount++;
}



void freeChunk(Chunk* chunk)
{
    FREE_ARRAY(uint8_t, chunk->code, chunk->codeCapacity);
    FREE_ARRAY(int, chunk->lines, chunk->codeCapacity);

    freeValueArray(&chunk->constants);
    initChunk(chunk);
}

int addConstant(Chunk* chunk, Value value)
{
    writeValueArray(&chunk->constants, value);
    return chunk->constants.count - 1;
}