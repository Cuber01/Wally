#include "core.h"
#include "native_utils.h"

NATIVE_FUNCTION(print)
{
    printRawValue(args[0]);
    putchar('\n');
    return NULL_VAL;
}

NATIVE_FUNCTION(type)
{
    printRawValue(args[0]);
    putchar('\n');
    return NULL_VAL;
}

void defineCore(Table* table)
{
    defineNativeFunction(table, "print", printNative);
}
