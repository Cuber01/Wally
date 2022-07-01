#include <stdio.h>
#include <stdarg.h>
#include "native_utils.h"

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

void defineNativeClass(Table* table, const char* name, ObjClass* class)
{
    tableDefineEntry(
            table,
            copyString(name, (int)strlen(name)),
            OBJ_VAL((Obj*)class)
    );
}

void addNativeMethodToClass(ObjClass* class, const char* name, NativeFn method)
{
    tableDefineEntry(&class->methods,
                     copyString(name, (int)strlen(name)),
                     OBJ_VAL((Obj*)newNative(method)));
}

inline bool checkArgCount(uint8_t expected, uint8_t got)
{
    if(expected == got) return true;

    nativeError(0, "Expected '%d' arguments but got '%d'.", expected, got); // todo line and foo name
    return false;
}