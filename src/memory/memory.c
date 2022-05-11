#include <stdlib.h>
#include <stdio.h>
#include "memory.h"
#include "object.h"
#include "vm.h"
#include "emitter.h"

#ifdef DEBUG_LOG_GC
#include <stdio.h>
#include "gc_logger.h"
#include "colors.h"
#endif


// oldSize      newSize                 Operation
// 0 	        Non‑zero 	            Allocate new block.
// Non‑zero 	0 	                    Free allocation.
// Non‑zero 	Smaller than oldSize 	Shrink existing allocation.
// Non‑zero 	Larger than oldSize 	Grow existing allocation.
void* reallocate(void* pointer, size_t oldSize, size_t newSize)
{
    if (newSize > oldSize) // todo
    {
        #ifdef DEBUG_STRESS_GC
        collectGarbage();
        #endif
    }

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
    #ifdef DEBUG_LOG_GC
    printf("%p free type %d\n", (void*)object, object->type);
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

// region Garbage Collector

// This is a Mark-Sweep garbage collector.

// Every object starts as white.
// GC traverses objects and adds them into the gray stack.
// After that, we traverse through the gray stack, mark newly found objects gray and the one we went from as black.
// When we're done marking, we free every object that is white.

// # White
// isMarked = false
// # Gray
// isMarked = true
// Is on the gray stack
// # Black
// isMarked = true
// Is not on the gray stack

void markObject(Obj* object)
{
    #ifdef DEBUG_LOG_GC
    printf("%p mark ", (void*)object);
    printValue(OBJ_VAL(object));
    printf("\n");
    #endif

    if (object == NULL) return;
    if (object->isMarked) return;

    object->isMarked = true;

    if (vm.grayCapacity < vm.grayCount + 1)
    {
        vm.grayCapacity = GROW_CAPACITY(vm.grayCapacity);
        vm.grayStack = (Obj**)realloc(vm.grayStack,
                                      sizeof(Obj*) * vm.grayCapacity);
    }

    vm.grayStack[vm.grayCount++] = object;

    if (vm.grayStack == NULL)
    {
        printf("Failed to allocate on the gray stack.");
        exit(1);
    }

}

void markValue(Value value)
{
    if (IS_OBJ(value)) markObject(AS_OBJ(value));
}

static void markArray(ValueArray* array)
{
    for (int i = 0; i < array->count; i++)
    {
        markValue(array->values[i]);
    }
}

static void markRoots()
{
    for (Value* slot = vm.stack; slot < vm.stackTop; slot++)
    {
        markValue(*slot);
    }

    markTable(&vm.currentEnvironment->values); // TODO implement freeing environments

    markCompilerRoots();
}

static void blackenObject(Obj* object)
{
    #ifdef DEBUG_LOG_GC
    printf("%p blacken ", (void*)object);
    printValue(OBJ_VAL(object));
    printf("\n");
    #endif

    switch (object->type)
    {
        case OBJ_NATIVE:
        case OBJ_STRING: break;

        case OBJ_FUNCTION:
        {
            // todo do something with environment and MAYBE calledFromFunction
            ObjFunction* function = (ObjFunction*)object;
            markObject((Obj*)function->name);
            markArray(&function->chunk.constants);
            break;
        }
    }
}

static void traceReferences()
{
    while (vm.grayCount > 0)
    {
        Obj* object = vm.grayStack[--vm.grayCount];
        blackenObject(object);
    }
}

static void sweep()
{
    Obj* previous = NULL;
    Obj* object = vm.objects;

    while (object != NULL)
    {
        if (object->isMarked)
        {
            object->isMarked = false;

            previous = object;
            object = object->next;
        }
        else
        {
            Obj* unreached = object;
            object = object->next;

            if (previous != NULL)
            {
                previous->next = object;
            }
            else
            {
                vm.objects = object;
            }

            freeObject(unreached);
        }
    }
}



void collectGarbage()
{
    #ifdef DEBUG_LOG_GC
    printf(BOLD_WHITE);
    printf("-- Garbage Collector Begin\n");
    printf(COLOR_CLEAR);
    #endif

    markRoots();
    traceReferences();
    tableRemoveWhite(&vm.strings);
    sweep();

}







// endregion