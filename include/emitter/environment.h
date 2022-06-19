#ifndef WALLY_ENVIRONMENT_H
#define WALLY_ENVIRONMENT_H

#include "table.h"

typedef struct Environment
{
    Table values;
    struct Environment* enclosing;
} Environment;

Environment* newEnvironment();
bool environmentDefine(Environment* env, ObjString* name, Value value);
bool environmentGet(Environment* env, ObjString* name, Value* result);
bool environmentSet(Environment* env, ObjString* name, Value value);

void markEnvironment(Environment* env);
void freeEnvironment(Environment* env);
void printVariables(Environment* env);

#endif //WALLY_ENVIRONMENT_H
