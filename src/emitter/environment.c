#include "environment.h"
#include "native_error.h"
#include "memory.h"

Environment* newEnvironment()
{
    Environment* newEnv = reallocate(NULL, 0, sizeof(Environment));

    Table* values;
    initTable(values);
    newEnv->values = values;

    return newEnv;
}

void environmentDefine(Environment* env, ObjString* name, Value value)
{
    bool success = tableSetNoOverwrite(env->values,name, value);
    if(!success)
    {
        nativeError("Tried to declare symbol %s, but it already exists.", name->chars);
    }
}

Value environmentGet(Environment* env, ObjString* name)
{
    Value* result = NULL;
    bool success = tableGet(env->values, name, result);
    if(!success)
    {
        nativeError("Tried to get value of %s, but it doesn't exist.", name->chars);
    }

    return *result;
}

void freeEnvironment(Environment* env)
{

}