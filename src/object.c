#include <stdio.h>
#include <string.h>

#include "memory.h"
#include "object.h"
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

static ObjString* allocateString(char* chars, int length)
{
    // Allocate new string object and set fields
    ObjString* string = ALLOCATE_OBJ(ObjString, OBJ_STRING);
    string->length = length;
    string->chars = chars;

    return string;
}

ObjString* takeString(char* chars, int length)
{
    return allocateString(chars, length);
}

ObjString* copyString(const char* chars, int length)
{
    // Allocate memory and then copy over the chars
    char* heapChars = ALLOCATE(char, length + 1);
    memcpy(heapChars, chars, length);

    // Terminate string
    heapChars[length] = '\0';

    return allocateString(heapChars, length);
}

void printObject(Value value)
{
    switch (OBJ_TYPE(value))
    {
        case OBJ_STRING:
            printf("%s", AS_CSTRING(value));
            break;
    }
}

ObjString* objectToString(Value value)
{
    switch (OBJ_TYPE(value))
    {
        case OBJ_STRING:
            return AS_STRING(value);
    }
}

