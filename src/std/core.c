#include "core.h"

Value printNative(int argCount, Value* args)
{
    printRawValue(*args);
    putchar('\n');
    return NULL_VAL;
}
