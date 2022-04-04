#ifndef WALLY_MEMORY_H
#define WALLY_MEMORY_H

#include "common.h"

inline int GROW_CAPACITY(int capacity)
{
    if(capacity < 8)
    {
        // If the capacity is 0, we initialize the array with capacity 8
        return 8;
    }
    else
    {
        // Else we grow the array by 2
        return capacity * 2;
    }
}

#define GROW_ARRAY(type, pointer, oldCount, newCount) \
        (type*)reallocate(pointer, sizeof(type) * (oldCount), sizeof(type) * (newCount))

#define FREE_ARRAY(type, pointer, oldCount) \
        reallocate(pointer, sizeof(type) * (oldCount), 0)

void* reallocate(void* pointer, size_t oldSize, size_t newSize);

#endif //WALLY_MEMORY_H
