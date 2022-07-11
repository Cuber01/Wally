#include <stdlib.h>
#include <time.h>
#include "value.h"
#include "native_utils.h"

NATIVE_FUNCTION(init)
{
    CHECK_ARG_COUNT("init", 0);
    srand(time(NULL));

    return NULL_VAL;
}

NATIVE_FUNCTION(bool)
{
    CHECK_ARG_COUNT("bool", 1);

    double chance = AS_NUMBER(args[0]);

    if(chance > 1.0 || chance < 0.0)
    {
        nativeError(line, "bool", "Chance equals '%g' and is outside of the 0-1 range. For 0% chance provide '0' and for 100% '1'.", chance);
    }

    bool result = chance > (rand() / RAND_MAX);

    return BOOL_VAL(result);
}

NATIVE_FUNCTION(integer)
{
    CHECK_ARG_COUNT("integer", 0);
    return NUMBER_VAL(rand());
}

NATIVE_FUNCTION(integerBetween)
{
    CHECK_ARG_COUNT("integerBetween", 2);

    int min = AS_NUMBER(args[0]);
    int max = AS_NUMBER(args[1]);

    return AS_NUMBER(rand() % (max + 1 - min) + min);
}

NATIVE_FUNCTION(between)
{
    CHECK_ARG_COUNT("between", 2);

    double min = AS_NUMBER(args[0]);
    double max = AS_NUMBER(args[1]);

    double range = (max - min);
    double div = RAND_MAX / range;

    return NUMBER_VAL(min + (rand() / div));
}

void defineRandom(Table* table)
{
    ObjClass* random = newClass(copyString("random", 6));

    #define DEFINE_RANDOM_METHOD(name, method) defineNativeFunction(random->methods, name, method)

    DEFINE_RANDOM_METHOD("between", betweenNative);
    DEFINE_RANDOM_METHOD("integerBetween", integerBetweenNative);
    DEFINE_RANDOM_METHOD("integer", integerNative);
    DEFINE_RANDOM_METHOD("bool", boolNative);
    DEFINE_RANDOM_METHOD("init", initNative);

    #undef DEFINE_MATH_METHOD

    tableDefineEntry(
            table,
            random->name,
            OBJ_VAL(newInstance(random))
    );
}
