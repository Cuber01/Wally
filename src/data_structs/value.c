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
    #ifdef NAN_BOXING

    #ifdef NAN_EQUAL_NAN

    if (IS_NUMBER(a) && IS_NUMBER(b))
    {
        return AS_NUMBER(a) == AS_NUMBER(b);
    }

    #endif

    return (a == b);

    #else

    if (a.type != b.type) return false;

    switch (a.type)
    {
        case VAL_BOOL:   return AS_BOOL(a) == AS_BOOL(b);
        case VAL_NULL:    return true;
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

    #endif
}

void printRawValue(Value value)
{
    #ifdef NAN_BOXING

    if (IS_BOOL(value))
    {
        printf(AS_BOOL(value) ? "true" : "false");
    }
    else if (IS_NULL(value))
    {
        printf("null");
    }
    else if (IS_NUMBER(value))
    {
        printf("%g", AS_NUMBER(value));
    }
    else if (IS_OBJ(value))
    {
        printObject(value);
    }

    #else

    switch (value.type)
    {
        case VAL_BOOL:
        {
            printf(AS_BOOL(value) ? "true" : "false");
            break;
        }

        case VAL_NULL:
        {
            printf("null");
            break;
        }


        case VAL_NUMBER:
        {
            printf("%g", AS_NUMBER(value));
            break;
        }

        case VAL_OBJ:
        {
            printObject(value);
            break;
        }
    }

    #endif
}

void printValue(Value value)
{
    #ifdef NAN_BOXING

    if (IS_BOOL(value))
    {
        printf(BOLD_BLUE);
        printf(AS_BOOL(value) ? "true" : "false");
    }
    else if (IS_NULL(value))
    {
        printf(BOLD_RED);
        printf("null");
    }
    else if (IS_NUMBER(value))
    {
        printf(BOLD_YELLOW);
        printf("%g", AS_NUMBER(value));
    }
    else if (IS_OBJ(value))
    {
        if(OBJ_TYPE(value) != OBJ_LIST)
            printf(BOLD_GREEN);

        printObject(value);
    }

    #else

    switch (value.type)
    {
        case VAL_BOOL:
        {
            printf(BOLD_BLUE);
            printf(AS_BOOL(value) ? "true" : "false");
            break;
        }

        case VAL_NULL:
        {
            printf(BOLD_RED);
            printf("null");
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

    #endif

    printf(COLOR_CLEAR);
}

// 1.100000 => 1.1
void removeTrailingZeros(char* source)
{
    uint16_t length = strlen(source);

    for(int i = length-1; i > 0; i--)
    {
        if(source[i] == '0' || source[i] == '.' )
        {
            source[i] = 0;
        }
        else
        {
            break;
        }
    }

}

ObjString* valueToString(Value value)
{
    // todo true, false, null should be global constants instead of being allocated each time

    #ifdef NAN_BOXING

    if (IS_BOOL(value))
    {
        if (AS_BOOL(value))
        {
            return copyString("true", 4);
        }
        else
        {
            return copyString("false", 5);
        }
    }
    else if (IS_NULL(value))
    {
        return copyString("null", 4);
    }
    else if (IS_NUMBER(value))
    {
        char output[UINT8_MAX];
        snprintf(output, UINT8_MAX, "%.5lf", AS_NUMBER(value));
        removeTrailingZeros(output);

        return copyString(output, strlen(output));
    }
    else if (IS_OBJ(value))
    {
        return objectToString(value);
    }

    #else

    switch (value.type)
    {
        case VAL_BOOL:
        {
            if (AS_BOOL(value))
            {
                return copyString("true", 4);
            }
            else
            {
                return copyString("false", 5);
            }
        }

        case VAL_NULL:
        {
            return copyString("null", 4);
        }

        case VAL_NUMBER:
        {
            char output[UINT8_MAX];
            snprintf(output, UINT8_MAX, "%.5lf", AS_NUMBER(value));
            removeTrailingZeros(output);

            return copyString(output, strlen(output));
        }

        default:
        {
            printf("Reached unreachable.");
        }
    }

    #endif

    return NULL;
}
