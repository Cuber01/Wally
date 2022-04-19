#include "environment.h"
#include "memory.h"

Environment* newEnvironment()
{
    Environment* newEnv = reallocate(NULL, 0, sizeof(Environment));

    Table* values = NULL;
    initTable(values);
    newEnv->values = values;

    return newEnv;
}

void environmentDefine(Environment* env, ObjString* name, Value value)
{
    tableSet(env->values,name, value); // todo check overwrite
}

Value* environmentGet(Environment* env, ObjString* name)
{
    Value* result = NULL;
    tableGet(env->values, name, result); // todo check if we actually returned anything
    return result;
}

void freeEnvironment(Environment* env)
{

}