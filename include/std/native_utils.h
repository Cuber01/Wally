#ifndef WALLY_NATIVE_UTILS_H
#define WALLY_NATIVE_UTILS_H

#include "object.h"

#define NATIVE_FUNCTION(name) static Value name##Native(uint8_t argCount, uint16_t line, const Value* args)
#define CHECK_ARG_COUNT(name, expected) checkArgCount(name, line, expected, argCount)

void defineNativeFunction(Table* table, const char* name, NativeFn function);
void nativeError(uint16_t line, const char* fooName, const char* format, ...);
bool checkArgCount(const char* fooName, uint16_t line, uint8_t expected, uint8_t got);

#endif //WALLY_NATIVE_UTILS_H
