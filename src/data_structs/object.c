#include <stdio.h>
#include <string.h>

#include "memory.h"
#include "object.h"
#include "table.h"
#include "value.h"
#include "vm.h"
#include "emitter.h"

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

ObjBoundMethod* newBoundMethod(ObjInstance* instance, ObjFunction* method)
{
    ObjBoundMethod* bound = ALLOCATE_OBJ(ObjBoundMethod,
                                         OBJ_BOUND_METHOD);
    bound->instance = instance;
    bound->method = method;
    return bound;
}

ObjFunction* newFunction(ObjString* name, uint8_t arity, FunctionType type)
{
    ObjFunction* function = ALLOCATE_OBJ(ObjFunction, OBJ_FUNCTION);

    function->arity = arity;
    function->name = name;
    function->type = type;
    function->calledFromFunction = NULL;
    function->calledFromIp = NULL;
    function->calledFromEnvironment = NULL;

    initChunk(&function->chunk);

    return function;
}

ObjClass* newClass(ObjString* name)
{
    ObjClass* klass = ALLOCATE_OBJ(ObjClass, OBJ_CLASS);
    klass->name = name;
    klass->methods = ALLOCATE_TABLE();
    initTable(klass->methods);
    return klass;
}

ObjInstance* newInstance(ObjClass* klass)
{
    ObjInstance* instance = ALLOCATE_OBJ(ObjInstance, OBJ_INSTANCE);
    instance->klass = klass;
    instance->fields = ALLOCATE_TABLE();
    initTable(instance->fields);

    return instance;
}

ObjNative* newNative(NativeFn function)
{
    ObjNative* native = ALLOCATE_OBJ(ObjNative, OBJ_NATIVE);
    native->function = function;
    return native;
}

ObjWList* newWList()
{
    ObjWList* list = ALLOCATE_OBJ(ObjWList, OBJ_LIST);

    list->items = NULL;
    list->count = 0;
    list->capacity = 0;

    return list;
}

void addWList(ObjWList* list, Value value)
{
    if (list->capacity < list->count + 1)
    {
        uint oldCapacity = list->capacity;
        list->capacity = GROW_CAPACITY(oldCapacity);
        list->items = GROW_ARRAY(Value, list->items, oldCapacity, list->capacity);
    }

    list->items[list->count] = value;
    list->count++;
}

void storeWList(ObjWList* list, Value value, uint index)
{
    list->items[index] = value;
}

Value getIndexWList(ObjWList* list, uint index)
{
    return list->items[index];
}

void removeIndexWList(ObjWList* list, uint index)
{
    for (uint i = index; i < list->count - 1; i++)
    {
        list->items[i] = list->items[i+1];
    }

    list->items[list->count - 1] = NULL_VAL;
    list->count--;
}

bool isValidWListIndex(ObjWList* list, uint index)
{
    if (index > list->count - 1)
    {
        return false;
    }

    return true;
}

static ObjString* allocateString(char* chars, uint length, uint32_t hash)
{
    // Allocate new string object and set fields
    ObjString* string = ALLOCATE_OBJ(ObjString, OBJ_STRING);
    string->length = length;
    string->chars = chars;
    string->hash = hash;

    push(OBJ_VAL(string));
    tableSet(vm.strings, string, NULL_VAL);
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
    ObjString* interned = tableFindString(vm.strings, chars, length, hash);
    if (interned != NULL) return interned;

    // Allocate memory and then copy over the chars
    char* heapChars = ALLOCATE(char, length + 1);
    memcpy(heapChars, chars, length);

    // Terminate string
    heapChars[length] = '\0';

    return allocateString(heapChars, length, hash);
}

bool isValidStringIndex(ObjString* string, uint index)
{
    // uint index cannot be < 0
    return index <= string->length;
}

ObjString* getIndexString(ObjString* string, uint index)
{
    char* c = &string->chars[index];
    return copyString(c, 1);
}

void replaceIndexString(ObjString* string, uint index, char c)
{
    string->chars[index] = c;
}

ObjString* addStrings(ObjString* a, ObjString* b)
{
    uint length = a->length + b->length;
    char* chars = ALLOCATE(char, length + 1);

    memcpy(chars, a->chars, a->length);
    memcpy(chars + a->length, b->chars, b->length);
    chars[length] = '\0';

    return takeString(chars, length);
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

static void printList(ObjWList* list)
{
    printf("{ ");

    for(uint i = 0; i < list->count; i++)
    {
        printValue(list->items[i]);

        if(i+1 < list->count) printf(", ");
    }

    printf(" }");
}

void printObject(Value value)
{
    switch (OBJ_TYPE(value))
    {
        case OBJ_STRING:
            printf("%s", AS_CSTRING(value));
            break;

        case OBJ_CLASS:
            printf("%s class", AS_CLASS(value)->name->chars);
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

        case OBJ_LIST:
            printList(AS_LIST(value));
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
        case OBJ_CLASS: return "OBJ_CLASS";
        case OBJ_INSTANCE: return "OBJ_INSTANCE";
        case OBJ_BOUND_METHOD: return "OBJ_BOUND_METHOD";

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


