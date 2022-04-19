#ifndef WALLY_ENVIRONMENT_H
#define WALLY_ENVIRONMENT_H

#include "table.h"

typedef struct Environment
{
    Table* values;
} Environment;

Environment* newEnvironment();
void environmentDefine(Environment* env, ObjString* name, Value value);
Value* environmentGet(Environment* env, ObjString* name);
void freeEnvironment(Environment* env);

#endif //WALLY_ENVIRONMENT_H
