#include <stdio.h>
#include <stdarg.h>

#include "object.h"

void nativeError(uint16_t line, const char* format, ...)
{
    va_list args;
    va_start(args, format);
    fprintf(stderr, "[line %d] Native Function Error : ", line);
    vfprintf(stderr, format, args);
    va_end(args);
    fputs("\n", stderr);
}

void defineNativeFunction(Table* table, const char* name, NativeFn function)
{
    tableDefineEntry(
            table,
            copyString(name, (int)strlen(name)),
            OBJ_VAL((Obj*)newNative(function))
    );
}

void defineNativeClass(Table* table, ObjClass* class)
{
    tableDefineEntry(
            table,
            class->name,
            OBJ_VAL((Obj*)class)
    );
}
