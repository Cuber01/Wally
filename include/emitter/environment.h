#ifndef WALLY_ENVIRONMENT_H
#define WALLY_ENVIRONMENT_H

#include "table.h"

typedef struct Environment
{
    Table values;
    struct Environment* enclosing;
} Environment;

Environment* newEnvironment();
void environmentDefine(Environment* env, ObjString* name, Value value);
bool environmentGet(Environment* env, ObjString* name, Value* result);
void freeEnvironment(Environment* env);

#endif //WALLY_ENVIRONMENT_H
