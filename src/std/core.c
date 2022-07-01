#include "core.h"
#include "native_utils.h"

Value printNative(int argCount, Value* args)
{
    printRawValue(*args);
    putchar('\n');
    return NULL_VAL;
}

void defineCore(Table* table)
{
    defineNativeFunction(table, "print", printNative);
}
