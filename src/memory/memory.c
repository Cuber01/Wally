#include <stdlib.h>
#include <stdio.h>
#include "memory.h"
#include "object.h"
#include "vm.h"

// oldSize      newSize                 Operation
// 0 	        Non‑zero 	            Allocate new block.
// Non‑zero 	0 	                    Free allocation.
// Non‑zero 	Smaller than oldSize 	Shrink existing allocation.
// Non‑zero 	Larger than oldSize 	Grow existing allocation.
void* reallocate(void* pointer, size_t oldSize, size_t newSize)
{
    if (newSize == 0)
    {
        free(pointer);
        return NULL;
    }

    void* result = realloc(pointer, newSize);

    if(result == NULL)
    {
        printf("Failed to reallocate.");
        exit(1);
    }

    return result;
}


void freeObject(Obj* object)
{
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

        case OBJ_UPVALUE:
        {
            FREE(ObjUpvalue, object);
        }

        case OBJ_CLOSURE:
        {
            ObjClosure* closure = (ObjClosure*)object;
            FREE_ARRAY(ObjUpvalue*, closure->upvalues, closure->upvalueCount);
            FREE(ObjClosure, object);
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
}




