#ifndef WALLY_ENVIRONMENT_H
#define WALLY_ENVIRONMENT_H

#include "table.h"

typedef struct Environment
{
    Table* values;
    struct Environment* enclosing;
} Environment;

Environment* newEnvironment();
bool environmentDefine(Environment* env, ObjString* name, Value value, uint16_t line);
bool environmentSet(Environment* env, ObjString* name, Value value, uint16_t line);
bool environmentGet(Environment* env, ObjString* name, Value* result);

void markEnvironment(Environment* env);
void freeEnvironmentsRecursively(Environment* env);
void freeEnvironment(Environment* env);
void printVariables(Environment* env);

#endif //WALLY_ENVIRONMENT_H
