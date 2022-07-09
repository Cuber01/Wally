#include <stdio.h>
#include <stdarg.h>

#include "object.h"

void nativeError(uint16_t line, char* fooName, const char* format, ...)
{
    va_list args;
    va_start(args, format);
    fprintf(stderr, "[line %d] Native function %s() : ", line, fooName);
    vfprintf(stderr, format, args);
    va_end(args);
    fputs("\n", stderr);
}

bool checkArgCount(char* fooName, uint16_t line, uint8_t expected, uint8_t got)
{
    if(expected == got) return true;

    nativeError(line, fooName, "Expected '%d' arguments but got '%d'.", expected, got);
    return false;
}

void defineNativeFunction(Table* table, const char* name, NativeFn function)
{
    tableDefineEntry(
            table,
            copyString(name, (int)strlen(name)),
            OBJ_VAL((Obj*)newNative(function))
    );
}

