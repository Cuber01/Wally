#include <glob.h>

#include "colors.h"
#include "allocation_logger.h"

void logAllocation(void* pointer, size_t oldSize, size_t newSize)
{
    if(oldSize == 0 && newSize > 0)
    {
        colorWriteline(GREEN, "[ %zu -> %zu ] Allocate %p", oldSize, newSize, pointer);
        return;
    }

    if(newSize == 0)
    {
        colorWriteline(RED, "[ %zu -> %zu ] Free %p", oldSize, newSize, pointer);
        return;
    }

    if(newSize > oldSize)
    {
        colorWriteline(BLUE, "[ %zu -> %zu ] Grow %p", oldSize, newSize, pointer);
        return;
    }

    if(oldSize < newSize)
    {
        colorWriteline(CYAN, "[ %zu -> %zu ] Shrink %p", oldSize, newSize, pointer);
        return;
    }
}
