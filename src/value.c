#include <stdio.h>
#include <string.h>

#include "memory.h"
#include "value.h"
#include "colors.h"
#include "object.h"

void initValueArray(ValueArray* array)
{
    array->values = NULL;
    array->capacity = 0;
    array->count = 0;
}

void writeValueArray(ValueArray* array, Value value)
{
    if (array->capacity < array->count + 1)
    {
        int oldCapacity = array->capacity;
        array->capacity = GROW_CAPACITY(oldCapacity);
        array->values = GROW_ARRAY(Value, array->values,
                                   oldCapacity, array->capacity);
    }

    array->values[array->count] = value;
    array->count++;
}

void freeValueArray(ValueArray* array)
{
    FREE_ARRAY(Value, array->values, array->capacity);
    initValueArray(array);
}

bool valuesEqual(Value a, Value b)
{
    if (a.type != b.type) return false;

    switch (a.type)
    {
        case VAL_BOOL:   return AS_BOOL(a) == AS_BOOL(b);
        case VAL_NIL:    return true;
        case VAL_NUMBER: return AS_NUMBER(a) == AS_NUMBER(b);
        case VAL_OBJ:
        {
            ObjString* aString = AS_STRING(a);
            ObjString* bString = AS_STRING(b);

            return aString->length == bString->length &&
                   memcmp(aString->chars, bString->chars,
                          aString->length) == 0;
        }
        default:         return false; // Unreachable.
    }
}

void printValue(Value value)
{
    switch (value.type)
    {
        case VAL_BOOL:
        {
            printf(BOLD_BLUE);
            printf(AS_BOOL(value) ? "true" : "false");
            break;
        }

        case VAL_NIL:
        {
            printf(BOLD_RED);
            printf("nil");
            break;
        }


        case VAL_NUMBER:
        {
            printf(BOLD_YELLOW);
            printf("%g", AS_NUMBER(value));
            break;
        }

        case VAL_OBJ:
        {
            printf(BOLD_GREEN);
            printObject(value);
            break;
        }
    }
    printf(COLOR_CLEAR);
}

ObjString* valueToString(Value value)
{
    switch (value.type)
    {
        case VAL_BOOL:

            break;
        case VAL_NIL:
        {
            return copyString("nil", 3);
        }

        case VAL_NUMBER:

            break;
        case VAL_OBJ:

            break;
    }

}
