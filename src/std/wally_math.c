#include <math.h>

#include "value.h"
#include "native_utils.h"

NATIVE_FUNCTION(abs)
{
    return NUMBER_VAL(fabs(AS_NUMBER(args[0])));
}

NATIVE_FUNCTION(acos)
{
    return NUMBER_VAL(acos(AS_NUMBER(args[0])));
}

NATIVE_FUNCTION(asin)
{
    return NUMBER_VAL(asin(AS_NUMBER(args[0])));
}

NATIVE_FUNCTION(atan)
{
    return NUMBER_VAL(atan(AS_NUMBER(args[0])));
}

NATIVE_FUNCTION(atan2)
{
    return NUMBER_VAL(atan2(AS_NUMBER(args[0]), AS_NUMBER(args[1])));
}

NATIVE_FUNCTION(ceil)
{
    return NUMBER_VAL(ceil(AS_NUMBER(args[0])));
}

NATIVE_FUNCTION(floor)
{
    return NUMBER_VAL(floor(AS_NUMBER(args[0])));
}

NATIVE_FUNCTION(radiansToDegrees)
{
    return NUMBER_VAL(AS_NUMBER(args[0]) * (180 / M_PI));
}

NATIVE_FUNCTION(degreesToRadians)
{
    return NUMBER_VAL((AS_NUMBER(args[0]) / 180) * M_PI);
}

NATIVE_FUNCTION(exp)
{
    return NUMBER_VAL(exp(AS_NUMBER(args[0])));
}

NATIVE_FUNCTION(mod)
{
    return NUMBER_VAL(fmod(AS_NUMBER(args[0]), AS_NUMBER(args[1])));
}

NATIVE_FUNCTION(max)
{
    return NUMBER_VAL(fmax(AS_NUMBER(args[0]), AS_NUMBER(args[1])));
}

NATIVE_FUNCTION(min)
{
    return NUMBER_VAL(fmin(AS_NUMBER(args[0]), AS_NUMBER(args[1])));
}

NATIVE_FUNCTION(sin)
{
    return NUMBER_VAL(sin(AS_NUMBER(args[0])));
}

NATIVE_FUNCTION(cos)
{
    return NUMBER_VAL(cos(AS_NUMBER(args[0])));
}

NATIVE_FUNCTION(tan)
{
    return NUMBER_VAL(tan(AS_NUMBER(args[0])));
}

NATIVE_FUNCTION(sqrt)
{
    return NUMBER_VAL(sqrt(AS_NUMBER(args[0])));
}

NATIVE_FUNCTION(round)
{
    return NUMBER_VAL(round(AS_NUMBER(args[0])));
}

void defineMath(Table* table)
{
    ObjClass* math = newClass(copyString("math", 4));

    #define DEFINE_MATH_METHOD(name, method) defineNativeFunction(&math->methods, name, method)

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

    //defineNativeClass(table, math);
    tableDefineEntry(
            table,
            math->name,
            OBJ_VAL((Obj*)newInstance(math))
    );

}

