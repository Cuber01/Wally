#include "core.h"

/*
static Value printNative(int argCount, Value* args)
{
    printRawValue(*args);
    putchar('\n');
    return NULL_VAL;
}
 */

void print(Value* args)
{
    printRawValue(args[0]);
    putchar('\n');
}