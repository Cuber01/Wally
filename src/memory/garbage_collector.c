#include <malloc.h>
#include <stdlib.h>


#include "common.h"
#include "garbage_collector.h"
#include "object.h"
#include "vm.h"
#include "memory.h"
#include "colors.h"
#include "emitter.h"
#include "value.h"

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

bool gcStarted = false;

void markObject(Obj* object)
{
    if (object == NULL) return;
    if (object->isMarked) return;

    #ifdef DEBUG_LOG_GC
    printf("%p mark ", (void*)object);
    printValue(OBJ_VAL(object));
    printf("\n");
    #endif

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
        markValue(*slot); //todo try to remove
    }

    if(vm.currentEnvironment != NULL)
    {
        markEnvironment(vm.currentEnvironment);
    }

    markObject((Obj*)vm.initString);
    markObject((Obj*)vm.thisString);

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

        case OBJ_BOUND_METHOD: {
            ObjBoundMethod* bound = (ObjBoundMethod*)object;
            markObject((Obj*)bound->instance);
            markObject((Obj*)bound->method);
            break;
        }

        case OBJ_CLASS:
        {
            ObjClass* klass = (ObjClass*)object;
            markObject((Obj*)klass->name);
            markTable(&klass->methods);
            break;
        }

        case OBJ_INSTANCE:
        {
            ObjInstance* instance = (ObjInstance*)object;
            markObject((Obj*)instance->klass);
            markTable(&instance->fields);
            break;
        }

        case OBJ_FUNCTION:
        {
            // todo do something with environment (current environment and its branches are marked already though) and MAYBE calledFromFunction (yeah no?)
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
    colorWriteLine(PURPLE, "-- Garbage Collector Begin");

    size_t before = vm.bytesAllocated;
    #endif

    markRoots(); // Mark objects as gray
    traceReferences(); // Mark gray objects as black

    tableRemoveWhite(&vm.strings); // Remove white objects from the string table
    sweep(); // Remove remaining whites

    vm.nextGC = vm.bytesAllocated * GC_HEAP_GROW_FACTOR;

    #ifdef DEBUG_LOG_GC
    printf("Collected %zu bytes (from %zu to %zu) next at %zu.\n",
           before - vm.bytesAllocated, before, vm.bytesAllocated,
           vm.nextGC);
    colorWriteLine(PURPLE, "-- Garbage Collector End");
    putchar('\n');
    #endif
}

