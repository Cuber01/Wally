#include "value.h"
#include <math.h>

#include "native_utils.h"

NATIVE_FUNCTION(abs)
{
    return NUMBER_VAL(fabs(AS_NUMBER(*args)));
}

