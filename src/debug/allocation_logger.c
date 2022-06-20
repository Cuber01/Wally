#include <glob.h>

#include "colors.h"
#include "allocation_logger.h"

void logAllocation(void* pointer, size_t oldSize, size_t newSize)
{
    if(oldSize == 0 && newSize > 0)
    {
        colorWriteLine(GREEN, "[ %zu -> %zu ] Allocate %p", oldSize, newSize, pointer);
        return;
    }

    if(newSize == 0 && oldSize == 0)
    {
        colorWriteLine(WHITE, "[-] Ignore null");
        return;
    }

    if(newSize == 0)
    {
        colorWriteLine(RED, "[ %zu -> %zu ] Free %p", oldSize, newSize, pointer);
        return;
    }

    if(newSize > oldSize)
    {
        colorWriteLine(BLUE, "[ %zu -> %zu ] Grow %p", oldSize, newSize, pointer);
        return;
    }

    if(oldSize < newSize)
    {
        colorWriteLine(CYAN, "[ %zu -> %zu ] Shrink %p", oldSize, newSize, pointer);
        return;
    }
}
