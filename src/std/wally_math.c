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

