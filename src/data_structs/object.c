#include <stdio.h>
#include <string.h>

#include "memory.h"
#include "object.h"
#include "table.h"
#include "value.h"
#include "vm.h"

static Obj* allocateObject(size_t size, ObjType type)
{
    // Allocate new object and set fields
    Obj* object = (Obj*)reallocate(NULL, 0, size);
    object->type = type;
    object->isMarked = false;

    object->next = vm.objects;
    vm.objects = object;

    #ifdef DEBUG_LOG_GC
    printf("%p allocate %zu for %s\n", (void*)object, size, objectTypeToChar(type));
    #endif

    return object;
}

ObjBoundMethod* newBoundMethod(Value receiver,  ObjFunction* method)
{
    ObjBoundMethod* bound = ALLOCATE_OBJ(ObjBoundMethod,
                                         OBJ_BOUND_METHOD);
    bound->receiver = receiver;
    bound->method = method;
    return bound;
}



ObjFunction* newFunction()
{
    ObjFunction* function = ALLOCATE_OBJ(ObjFunction, OBJ_FUNCTION);

    function->arity = 0;
    function->name = NULL;
    function->calledFromFunction = NULL;
    function->calledFromIp = NULL;

    initChunk(&function->chunk);

    return function;
}

ObjClass* newClass(ObjString* name)
{
    ObjClass* klass = ALLOCATE_OBJ(ObjClass, OBJ_CLASS);
    klass->name = name;
    initTable(&klass->methods);
    return klass;
}

ObjInstance* newInstance(ObjClass* klass)
{
    ObjInstance* instance = ALLOCATE_OBJ(ObjInstance, OBJ_INSTANCE);
    instance->klass = klass;
    initTable(&instance->fields);

    return instance;
}

ObjNative* newNative(NativeFn function)
{
    ObjNative* native = ALLOCATE_OBJ(ObjNative, OBJ_NATIVE);
    native->function = function;
    return native;
}

static ObjString* allocateString(char* chars, uint length, uint32_t hash)
{
    // Allocate new string object and set fields
    ObjString* string = ALLOCATE_OBJ(ObjString, OBJ_STRING);
    string->length = length;
    string->chars = chars;
    string->hash = hash;

    push(OBJ_VAL(string));
    tableSet(&vm.strings, string, NULL_VAL);
    pop();

    return string;
}

// FNV-1a hashing
static uint32_t hashString(const char* key, uint length)
{
    uint32_t hash = 2166136261u;

    for (int i = 0; i < length; i++)
    {
        hash ^= (uint8_t)key[i];
        hash *= 16777619;
    }

    return hash;
}

ObjString* takeString(char* chars, uint length)
{
    uint32_t hash = hashString(chars, length);
    return allocateString(chars, length, hash);
}

ObjString* copyString(const char* chars, uint length)
{
    // Generate hash
    uint32_t hash = hashString(chars, length);

    // If the string was already interned, just return it
    ObjString* interned = tableFindString(&vm.strings, chars, length, hash);
    if (interned != NULL) return interned;

    // Allocate memory and then copy over the chars
    char* heapChars = ALLOCATE(char, length + 1);
    memcpy(heapChars, chars, length);

    // Terminate string
    heapChars[length] = '\0';

    return allocateString(heapChars, length, hash);
}

static void printFunction(ObjFunction* function)
{
    if (function->name == NULL)
    {
        printf("<script>");
        return;
    }

    printf("<fn %s>", function->name->chars);
}

void printObject(Value value)
{
    switch (OBJ_TYPE(value))
    {
        case OBJ_STRING:
            printf("%s", AS_CSTRING(value));
            break;

        case OBJ_CLASS:
            printf("%s", AS_CLASS(value)->name->chars);
            break;

        case OBJ_NATIVE:
            printf("<native fn>");
            break;

        case OBJ_FUNCTION:
            printFunction(AS_FUNCTION(value));
            break;

        case OBJ_INSTANCE:
            printf("%s instance", AS_INSTANCE(value)->klass->name->chars);
            break;

        case OBJ_BOUND_METHOD:
            printFunction(AS_BOUND_METHOD(value)->method);
            break;
    }
}

char* objectTypeToChar(ObjType type)
{
    switch (type)
    {
        case OBJ_FUNCTION: return "OBJ_FUNCTION";
        case OBJ_NATIVE: return "OBJ_NATIVE";
        case OBJ_STRING: return "OBJ_STRING";

        default: return "UNREACHABLE REACHED";
    }
}

ObjString* objectToString(Value value)
{
    switch (OBJ_TYPE(value))
    {
        case OBJ_STRING:
            return AS_STRING(value);
        case OBJ_NATIVE:
            return copyString("<native fn>", 11);
        case OBJ_FUNCTION:
            return copyString(AS_FUNCTION(value)->name->chars, strlen(AS_FUNCTION(value)->name->chars));
        default:
            printf("Reached unreachable.");
            return NULL;
    }
}


