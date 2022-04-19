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
    function->upvalueCount = 0;
    function->name = NULL;
    initChunk(&function->chunk);
    return function;
}

ObjClosure* newClosure(ObjFunction* function)
{
    ObjUpvalue** upvalues = ALLOCATE(ObjUpvalue*, function->upvalueCount);
    for (int i = 0; i < function->upvalueCount; i++) {
        upvalues[i] = NULL;
    }

    ObjClosure* closure = ALLOCATE_OBJ(ObjClosure, OBJ_CLOSURE);
    closure->upvalues = upvalues;
    closure->upvalueCount = function->upvalueCount;
    closure->function = function;
    return closure;
}

ObjUpvalue* newUpvalue(Value* slot)
{
    ObjUpvalue* upvalue = ALLOCATE_OBJ(ObjUpvalue, OBJ_UPVALUE);
    upvalue->location = slot;
    return upvalue;
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
        case OBJ_FUNCTION:
            printFunction(AS_FUNCTION(value));
            break;
        case OBJ_NATIVE:
            printf("<native fn>");
            break;
        case OBJ_CLOSURE:
            printFunction(AS_CLOSURE(value)->function);
            break;
        case OBJ_UPVALUE:
            printf("<upvalue>");
            break;
    }
}

ObjString* objectToString(Value value)
{
    switch (OBJ_TYPE(value))
    {
        case OBJ_STRING:
            return AS_STRING(value);
        case OBJ_FUNCTION:
            return AS_FUNCTION(value)->name;
        case OBJ_NATIVE:
            return copyString("<native fn>", 11);
        case OBJ_CLOSURE:
            return AS_CLOSURE(value)->function->name;
        case OBJ_UPVALUE:
            return copyString("<upvalue>", 9);
    }
}


