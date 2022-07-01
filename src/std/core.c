#include "core.h"
#include "native_utils.h"

NATIVE_FUNCTION(print)
{
    printRawValue(*args);
    putchar('\n');
    return NULL_VAL;
}

void defineCore(Table* table)
{
    defineNativeFunction(table, "print", printNative);
}
