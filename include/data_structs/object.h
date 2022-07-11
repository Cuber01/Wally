#ifndef WALLY_OBJECT_H
#define WALLY_OBJECT_H

#include "common.h"
#include "value.h"
#include "chunk.h"
#include "environment.h"
#include "emitter.h"

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
#define IS_CLASS(value)        isObjType(value, OBJ_CLASS)
#define IS_INSTANCE(value)     isObjType(value, OBJ_INSTANCE)
#define IS_BOUND_METHOD(value) isObjType(value, OBJ_BOUND_METHOD)

#define AS_STRING(value)        ((ObjString*)AS_OBJ(value))
#define AS_CSTRING(value)       (((ObjString*)AS_OBJ(value))->chars)
#define AS_FUNCTION(value)      ((ObjFunction*)AS_OBJ(value))
#define AS_NATIVE(value)        (((ObjNative*)AS_OBJ(value))->function)
#define AS_CLASS(value)         ((ObjClass*)AS_OBJ(value))
#define AS_INSTANCE(value)     ((ObjInstance*)AS_OBJ(value))
#define AS_BOUND_METHOD(value) ((ObjBoundMethod*)AS_OBJ(value))


typedef Value (*NativeFn)(uint8_t argCount, uint16_t line, const Value* args);

typedef enum {
    OBJ_CLASS,
    OBJ_STRING,
    OBJ_FUNCTION,
    OBJ_INSTANCE,
    OBJ_NATIVE,
    OBJ_BOUND_METHOD,
} ObjType;

struct Obj {
    ObjType type;
    struct Obj* next;

    bool isMarked;
};

// "Struct Inheritance"
// Thanks to how C works, we can cast ObjString* to Obj* and access its fields,
// but only as long as Obj is the first field in the child struct
struct ObjString {
    Obj obj;
    uint length;
    char* chars;

    uint32_t hash; // Used in hashtables
};

typedef struct {
    Obj obj;
    ObjString* name;
    Table* methods;

    struct ObjClass* parent;
} ObjClass;

typedef struct {
    Obj obj;
    ObjClass* klass;
    Table* fields;
} ObjInstance;

typedef struct ObjFunction {
    Obj obj;

    uint8_t arity;
    Chunk chunk;
    ObjString* name;
    FunctionType type;

    Environment* closure;

    struct ObjFunction* calledFromFunction;
    uint8_t* calledFromIp;
    Environment* calledFromEnvironment;
} ObjFunction;

typedef struct {
    Obj obj;
    ObjFunction* method;
    ObjInstance* instance;
} ObjBoundMethod;

typedef struct {
    Obj obj;
    NativeFn function;
} ObjNative;

static inline bool isObjType(Value value, ObjType type)
{
    return IS_OBJ(value) && AS_OBJ(value)->type == type;
}

static inline bool charsEqual(char* a, char* b, uint lengthA, uint lengthB)
{
    return (lengthA == lengthB && strcmp(a, b) == 0);
}

ObjString* copyString(const char* chars, uint length);
ObjString* takeString(char* chars, uint length);

void printObject(Value value);
ObjString* objectToString(Value value);

ObjNative* newNative(NativeFn function);
ObjFunction* newFunction(ObjString* name, uint8_t arity, FunctionType type);
ObjClass* newClass(ObjString* name);
ObjInstance* newInstance(ObjClass* klass);
ObjBoundMethod* newBoundMethod(ObjInstance* instance, ObjFunction* method);

char* objectTypeToChar(ObjType type);

#endif //WALLY_OBJECT_H
