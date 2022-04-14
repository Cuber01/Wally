#ifndef WALLY_OBJECT_H
#define WALLY_OBJECT_H

#include "common.h"
#include "value.h"
#include "chunk.h"

// Objects are kept on the heap, and we just keep a pointer to them in Value.
// This allows us to make objects as big, or as small, as we want (in theory).

#define ALLOCATE_OBJ(type, objectType) \
    (type*)allocateObject(sizeof(type), objectType)

#define ALLOCATE(type, count) \
    (type*)reallocate(NULL, 0, sizeof(type) * (count))

#define OBJ_TYPE(value)        (AS_OBJ(value)->type)

#define IS_STRING(value)       isObjType(value, OBJ_STRING)
#define IS_FUNCTION(value)     isObjType(value, OBJ_FUNCTION)
#define IS_NATIVE(value)       isObjType(value, OBJ_NATIVE)

#define AS_STRING(value)        ((ObjString*)AS_OBJ(value))
#define AS_CSTRING(value)       (((ObjString*)AS_OBJ(value))->chars)
#define AS_FUNCTION(value)      ((ObjFunction*)AS_OBJ(value))
#define AS_NATIVE(value)        (((ObjNative*)AS_OBJ(value))->function)


typedef enum {
    OBJ_STRING,
    OBJ_FUNCTION,
    OBJ_NATIVE,
} ObjType;

struct Obj {
    ObjType type;
    struct Obj* next;
};

// "Struct Inheritance"
// Thanks to how C works, we can cast ObjString* to Obj* and access its fields,
// but only as long as Obj is the first field in the child struct
struct ObjString {
    Obj obj;
    int length;
    char* chars;

    uint32_t hash; // Used in hashtables
};

typedef struct {
    Obj obj;
    int arity;
    Chunk chunk;
    ObjString* name;
} ObjFunction;

typedef Value (*NativeFn)(int argCount, Value* args);

typedef struct {
    Obj obj;
    NativeFn function;
} ObjNative;

static inline bool isObjType(Value value, ObjType type)
{
    return IS_OBJ(value) && AS_OBJ(value)->type == type;
}

ObjString* copyString(const char* chars, int length);
ObjString* takeString(char* chars, int length);

void printObject(Value value);
ObjString* objectToString(Value value);

ObjFunction* newFunction();
ObjNative* newNative(NativeFn function);

#endif //WALLY_OBJECT_H
