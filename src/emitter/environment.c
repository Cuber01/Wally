#include <string.h>
#include <stdio.h>
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

bool environmentDefine(Environment* env, ObjString* name, Value value)
{
    uint8_t success = tableDefineEntry(&env->values, name, value);

    if(success != TABLE_SUCCESS)
    {
        nativeError("Tried to declare symbol %s, but it already exists.", name->chars);
        return false;
    }

    return true;
}

bool environmentSet(Environment* env, ObjString* name, Value value)
{
    uint8_t success = tableSetExistingEntry(&env->values, name, value);
    if(success != TABLE_SUCCESS)
    {
        if(env->enclosing != NULL)
        {
            return environmentSet(env->enclosing, name, value);
        }

        if(success == TABLE_ERROR_UNDEFINED_SET)
        {
            nativeError("Tried to set value of '%s', but it doesn't exist.", name->chars);
        }

        if(success == TABLE_ERROR_FUNCTION_SET)
        {
            nativeError("Changing value of functions is illegal.", name->chars);
        }

        return false;
    }

    return true;
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

        if(strcmp("print", name->chars) != 0) // todo temporary code
        {
            nativeError("Tried to get value of '%s', but it doesn't exist.", name->chars);
        }

        return false;
    }

    return true;
}

void printVariables(Environment* env)
{
    while(env != NULL)
    {
        Table table = env->values;

        for(uint i = 0; i < table.count; i++)
        {
            if(table.entries[i].key == NULL) continue;

            printf("%s: ", table.entries[i].key->chars);
            printRawValue(table.entries[i].value);
            putc('\n', stdout);
        }

        env = env->enclosing;
    }
}

void markEnvironment(Environment* env)
{
    markTable(&env->values);

    if(env->enclosing != NULL)
    {
        markEnvironment(env->enclosing);
    }
}

void freeEnvironment(Environment* env)
{
    FREE_ARRAY(Entry, env->values.entries, env->values.capacity);
    FREE(Environment, env);
}