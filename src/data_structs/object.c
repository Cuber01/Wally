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

    object->next = vm.objects;
    vm.objects = object;

    return object;
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

ObjNative* newNative(NativeFn function)
{
    ObjNative* native = ALLOCATE_OBJ(ObjNative, OBJ_NATIVE);
    native->function = function;
    return native;
}

static ObjString* allocateString(char* chars, unsigned int length, uint32_t hash)
{
    // Allocate new string object and set fields
    ObjString* string = ALLOCATE_OBJ(ObjString, OBJ_STRING);
    string->length = length;
    string->chars = chars;
    string->hash = hash;

    tableSet(&vm.strings, string, NULL_VAL);

    return string;
}

// FNV-1a hashing
static uint32_t hashString(const char* key, unsigned int length)
{
    uint32_t hash = 2166136261u;

    for (int i = 0; i < length; i++)
    {
        hash ^= (uint8_t)key[i];
        hash *= 16777619;
    }

    return hash;
}

ObjString* takeString(char* chars, unsigned int length)
{
    uint32_t hash = hashString(chars, length);
    return allocateString(chars, length, hash);
}

ObjString* copyString(const char* chars, unsigned int length)
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

        case OBJ_NATIVE:
            printf("<native fn>");
            break;

        case OBJ_FUNCTION:
            printFunction(AS_FUNCTION(value));
            break;
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
            break;
    }
}


