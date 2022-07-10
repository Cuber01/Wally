#include <math.h>

#include "value.h"
#include "native_utils.h"

NATIVE_FUNCTION(abs)
{
    CHECK_ARG_COUNT("abs", 1);
    return NUMBER_VAL(fabs(AS_NUMBER(args[0])));
}

NATIVE_FUNCTION(acos)
{
    CHECK_ARG_COUNT("acos", 1);
    return NUMBER_VAL(acos(AS_NUMBER(args[0])));
}

NATIVE_FUNCTION(asin)
{
    CHECK_ARG_COUNT("asin", 1);
    return NUMBER_VAL(asin(AS_NUMBER(args[0])));
}

NATIVE_FUNCTION(atan)
{
    CHECK_ARG_COUNT("atan", 1);
    return NUMBER_VAL(atan(AS_NUMBER(args[0])));
}

NATIVE_FUNCTION(atan2)
{
    CHECK_ARG_COUNT("atan2", 2);
    return NUMBER_VAL(atan2(AS_NUMBER(args[0]), AS_NUMBER(args[1])));
}

NATIVE_FUNCTION(ceil)
{
    CHECK_ARG_COUNT("ceil", 1);
    return NUMBER_VAL(ceil(AS_NUMBER(args[0])));
}

NATIVE_FUNCTION(floor)
{
    CHECK_ARG_COUNT("floor", 1);
    return NUMBER_VAL(floor(AS_NUMBER(args[0])));
}

NATIVE_FUNCTION(radiansToDegrees)
{
    CHECK_ARG_COUNT("radiansToDegrees", 1);
    return NUMBER_VAL(AS_NUMBER(args[0]) * (180 / M_PI));
}

NATIVE_FUNCTION(degreesToRadians)
{
    CHECK_ARG_COUNT("degreesToRadians", 1);
    return NUMBER_VAL((AS_NUMBER(args[0]) / 180) * M_PI);
}

NATIVE_FUNCTION(exp)
{
    CHECK_ARG_COUNT("exp", 1);
    return NUMBER_VAL(exp(AS_NUMBER(args[0])));
}

NATIVE_FUNCTION(mod)
{
    CHECK_ARG_COUNT("mod", 2);
    return NUMBER_VAL(fmod(AS_NUMBER(args[0]), AS_NUMBER(args[1])));
}

NATIVE_FUNCTION(max)
{
    CHECK_ARG_COUNT("max", 2);
    return NUMBER_VAL(fmax(AS_NUMBER(args[0]), AS_NUMBER(args[1])));
}

NATIVE_FUNCTION(min)
{
    CHECK_ARG_COUNT("min", 2);
    return NUMBER_VAL(fmin(AS_NUMBER(args[0]), AS_NUMBER(args[1])));
}

NATIVE_FUNCTION(sin)
{
    CHECK_ARG_COUNT("sin", 1);
    return NUMBER_VAL(sin(AS_NUMBER(args[0])));
}

NATIVE_FUNCTION(cos)
{
    CHECK_ARG_COUNT("cos", 1);
    return NUMBER_VAL(cos(AS_NUMBER(args[0])));
}

NATIVE_FUNCTION(tan)
{
    CHECK_ARG_COUNT("tan", 1);
    return NUMBER_VAL(tan(AS_NUMBER(args[0])));
}

NATIVE_FUNCTION(sqrt)
{
    CHECK_ARG_COUNT("sqrt", 1);
    return NUMBER_VAL(sqrt(AS_NUMBER(args[0])));
}

NATIVE_FUNCTION(round)
{
    CHECK_ARG_COUNT("round", 1);
    return NUMBER_VAL(round(AS_NUMBER(args[0])));
}

void defineMath(Table* table)
{
    ObjClass* math = newClass(copyString("math", 4));

    #define DEFINE_MATH_METHOD(name, method) defineNativeFunction(math->methods, name, method)

    DEFINE_MATH_METHOD("abs",   absNative);
    DEFINE_MATH_METHOD("round", roundNative);
    DEFINE_MATH_METHOD("sqrt",  sqrtNative);
    DEFINE_MATH_METHOD("tan",   tanNative);
    DEFINE_MATH_METHOD("cos",   cosNative);
    DEFINE_MATH_METHOD("sin",   sinNative);
    DEFINE_MATH_METHOD("min",   minNative);
    DEFINE_MATH_METHOD("max",   maxNative);
    DEFINE_MATH_METHOD("mod",   modNative);
    DEFINE_MATH_METHOD("exp",   expNative);
    DEFINE_MATH_METHOD("degreesToRadians",   degreesToRadiansNative);
    DEFINE_MATH_METHOD("radiansToDegrees",   radiansToDegreesNative);
    DEFINE_MATH_METHOD("floor", floorNative);
    DEFINE_MATH_METHOD("ceil",  ceilNative);
    DEFINE_MATH_METHOD("atan2", atan2Native);
    DEFINE_MATH_METHOD("atan",  atanNative);
    DEFINE_MATH_METHOD("asin",  asinNative);
    DEFINE_MATH_METHOD("acos",  acosNative);
    DEFINE_MATH_METHOD("atan2", atan2Native);
    DEFINE_MATH_METHOD("atan2", atan2Native);

    #undef DEFINE_MATH_METHOD

    tableDefineEntry(
            table,
            math->name,
            OBJ_VAL((Obj*)newInstance(math))
    );

}

