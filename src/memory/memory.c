#include <stdlib.h>
#include <stdio.h>

#include "memory.h"
#include "object.h"
#include "vm.h"
#include "emitter.h"
#include "garbage_collector.h"

#ifdef DEBUG_LOG_ALLOCATION
#include "allocation_logger.h"
#endif

// oldSize      newSize                 Operation
// 0 	        Non‑zero 	            Allocate new block.
// Non‑zero 	0 	                    Free allocation.
// Non‑zero 	Smaller than oldSize 	Shrink existing allocation.
// Non‑zero 	Larger than oldSize 	Grow existing allocation.
void* reallocate(void* pointer, size_t oldSize, size_t newSize)
{
    vm.bytesAllocated += newSize - oldSize;

    #ifdef DEBUG_STRESS_GC
    if (newSize > oldSize)
    {
        collectGarbage();
    }
    #else
    if (vm.bytesAllocated > vm.nextGC)
    {
        collectGarbage();
    }
    #endif


    if (newSize == 0)
    {
        #ifdef DEBUG_LOG_ALLOCATION
        logAllocation(pointer, oldSize, newSize);
        #endif

        free(pointer);
        return NULL;
    }

    void* result = realloc(pointer, newSize);

    if(result == NULL)
    {
        printf("Failed to reallocate.");
        exit(1);
    }

    #ifdef DEBUG_LOG_ALLOCATION
    logAllocation(result, oldSize, newSize);
    #endif

    return result;
}


void freeObject(Obj* object)
{
    #ifdef DEBUG_LOG_GC
    printf("%p free type %s\n", (void*)object, objectTypeToChar(object->type));
    #endif

    switch (object->type)
    {
        case OBJ_STRING:
        {
            ObjString* string = (ObjString*)object;
            FREE_ARRAY(char, string->chars, string->length + 1);
            FREE(ObjString, object);
            break;
        }

        case OBJ_FUNCTION:
        {
            ObjFunction* function = (ObjFunction*)object;
            freeChunk(&function->chunk);
            FREE(ObjFunction, object);
            break;
        }

        case OBJ_NATIVE:
            FREE(ObjNative, object);
            break;
    }
}

void freeObjects()
{
    Obj* object = vm.objects;

    // Walk the linked list and free every object
    while (object != NULL)
    {
        Obj* next = object->next;
        freeObject(object);
        object = next;
    }

    free(vm.grayStack);
}
