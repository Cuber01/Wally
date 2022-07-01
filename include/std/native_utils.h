#ifndef WALLY_NATIVE_UTILS_H
#define WALLY_NATIVE_UTILS_H

#include "object.h"

#define NATIVE_FUNCTION(name) static Value name##Native(uint8_t argCount, Value* args)

void defineNativeFunction(Table* table, const char* name, NativeFn function);
void defineNativeClass(Table* table, const char* name, ObjClass* class);
void addNativeMethodToClass(ObjClass* class, const char* name, NativeFn method);
void nativeError(uint16_t line, const char* format, ...);

inline bool checkArgCount(uint8_t expected, uint8_t got)
{
    if(expected == got) return true;

    nativeError(0, "Expected '%d' arguments but got '%d'.", expected, got); // todo line and foo name
    return false;
}

#endif //WALLY_NATIVE_UTILS_H
