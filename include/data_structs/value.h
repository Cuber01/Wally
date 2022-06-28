#ifndef WALLY_VALUE_H
#define WALLY_VALUE_H

#include "common.h"

typedef struct Obj Obj;
typedef struct ObjString ObjString;

#ifdef NAN_BOXING

#include <string.h>

typedef uint64_t Value;

#define QNAN     ((uint64_t)0x7ffc000000000000)
#define SIGN_BIT ((uint64_t)0x8000000000000000)

#define TAG_NULL  1 // 01.
#define TAG_FALSE 2 // 10.
#define TAG_TRUE  3 // 11.

static inline Value numToValue(double num)
{
    return *((Value*) &num);
}

static inline double valueToNum(Value value)
{
    return *((double*) &value);
}

#define NUMBER_VAL(num) numToValue(num)
#define NULL_VAL        ((Value)(uint64_t)(QNAN | TAG_NULL))
#define FALSE_VAL       ((Value)(uint64_t)(QNAN | TAG_FALSE))
#define TRUE_VAL        ((Value)(uint64_t)(QNAN | TAG_TRUE))
#define BOOL_VAL(b)     ((b) ? TRUE_VAL : FALSE_VAL)
#define OBJ_VAL(obj)    (Value)(SIGN_BIT | QNAN | (uint64_t)(uintptr_t)(obj))



#define AS_NUMBER(value) valueToNum(value)
#define AS_BOOL(value)   ((value) == TRUE_VAL)
#define AS_OBJ(value)    ((Obj*)(uintptr_t)((value) & ~(SIGN_BIT | QNAN)))



#define IS_NUMBER(value) (((value) & QNAN) != QNAN)
#define IS_NULL(value)   ((value) == NULL_VAL)
#define IS_BOOL(value)   (((value) | 1) == TRUE_VAL)
#define IS_OBJ(value)    (((value) & (QNAN | SIGN_BIT)) == (QNAN | SIGN_BIT))


#else

// C value => Wally value
#define BOOL_VAL(value)   ((Value){VAL_BOOL, {.boolean = (value)}})
#define NULL_VAL          ((Value){VAL_NULL, {.number = 0}})
#define NUMBER_VAL(value) ((Value){VAL_NUMBER, {.number = (value)}})
#define OBJ_VAL(object)   ((Value){VAL_OBJ, {.obj = (Obj*)(object)}})

// Wally value => C value
#define AS_BOOL(value)    ((value).as.boolean)
#define AS_NUMBER(value)  ((value).as.number)
#define AS_OBJ(value)     ((value).as.obj)

// Check type
#define IS_BOOL(value)    ((value).type == VAL_BOOL)
#define IS_NULL(value)     ((value).type == VAL_NULL)
#define IS_NUMBER(value)  ((value).type == VAL_NUMBER)
#define IS_OBJ(value)     ((value).type == VAL_OBJ)

typedef enum {
    VAL_BOOL,
    VAL_NULL,
    VAL_NUMBER,
    VAL_OBJ
} ValueType;

typedef struct 
{
    ValueType type;
    
    union {
        bool boolean;
        double number;
        Obj* obj;
    } as;
} Value;

#endif

typedef struct {
    int capacity;
    int count;
    Value* values;
} ValueArray;


void initValueArray(ValueArray* array);
void writeValueArray(ValueArray* array, Value value);
void freeValueArray(ValueArray* array);

void printValue(Value value);
void printRawValue(Value value);
bool valuesEqual(Value a, Value b);
ObjString* valueToString(Value value);

#endif //WALLY_VALUE_H
