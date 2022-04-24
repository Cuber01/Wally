#include "environment.h"
#include "native_error.h"
#include "memory.h"

Environment* newEnvironment()
{
    Environment* newEnv = reallocate(NULL, 0, sizeof(Environment));

    Table* values = reallocate(NULL, 0, sizeof(Table));
    initTable(values);
    newEnv->values = *values;

    return newEnv;
}

void environmentDefine(Environment* env, ObjString* name, Value value)
{
    bool success = tableSetNoOverwrite(&env->values,name, value);
    if(!success)
    {
        nativeError("Tried to declare symbol %s, but it already exists.", name->chars);
    }
}

bool environmentGet(Environment* env, ObjString* name, Value* result)
{
    bool success = tableGet(&env->values, name, result);
    if(!success)
    {
        if(env->enclosing != NULL)
        {
            return environmentGet(env->enclosing, name, result);
        }

        nativeError("Tried to get value of %s, but it doesn't exist.", name->chars);
        return false;
    }

    return true;
}

void freeEnvironment(Environment* env)
{
    FREE_ARRAY(Entry, env->values.entries, env->values.capacity);
    FREE(Environment, env);
}