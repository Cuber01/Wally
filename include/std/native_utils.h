#ifndef WALLY_NATIVE_UTILS_H
#define WALLY_NATIVE_UTILS_H

#include "object.h"

void defineNativeFunction(Table* table, const char* name, NativeFn function);
void defineNativeClass(Table* table, const char* name, ObjClass* class);
void addNativeMethodToClass(ObjClass* class, const char* name, NativeFn method);

#endif //WALLY_NATIVE_UTILS_H
