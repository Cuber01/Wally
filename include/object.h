#ifndef WALLY_OBJECT_H
#define WALLY_OBJECT_H

#include "common.h"
#include "value.h"

// Objects are kept on the heap, and we just keep a pointer to them in Value.
// This allows us to make objects as big, or as small, as we want (in theory).

#define ALLOCATE_OBJ(type, objectType) \
    (type*)allocateObject(sizeof(type), objectType)

#define ALLOCATE(type, count) \
    (type*)reallocate(NULL, 0, sizeof(type) * (count))

#define OBJ_TYPE(value)        (AS_OBJ(value)->type)

#define IS_STRING(value)       isObjType(value, OBJ_STRING)

#define AS_STRING(value)       ((ObjString*)AS_OBJ(value))
#define AS_CSTRING(value)      (((ObjString*)AS_OBJ(value))->chars)

typedef enum {
    OBJ_STRING,
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
};

static inline bool isObjType(Value value, ObjType type)
{
    return IS_OBJ(value) && AS_OBJ(value)->type == type;
}

ObjString* copyString(const char* chars, int length);
ObjString* takeString(char* chars, int length);

void printObject(Value value);

#endif //WALLY_OBJECT_H
