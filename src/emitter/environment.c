#include <string.h>
#include <stdio.h>

#include "vm.h"
#include "environment.h"
#include "memory.h"

Environment* newEnvironment()
{
    Environment* newEnv = reallocate(NULL, 0, sizeof(Environment));

    Table* values = ALLOCATE_TABLE();
    initTable(values);
    newEnv->values = values;

    newEnv->enclosing = NULL;

    return newEnv;
}

bool environmentDefine(Environment* env, ObjString* name, Value value, uint16_t line)
{
    uint8_t success = tableDefineEntry(env->values, name, value);

    if(success != TABLE_SUCCESS)
    {
        runtimeError(line, "Tried to declare symbol %s, but it already exists.", name->chars);
        return false;
    }

    return true;
}

bool environmentSet(Environment* env, ObjString* name, Value value, uint16_t line)
{
    uint8_t success = tableSetExistingEntry(env->values, name, value);
    if(success != TABLE_SUCCESS)
    {
        if(env->enclosing != NULL)
        {
            return environmentSet(env->enclosing, name, value, line);
        }

        if(success == TABLE_ERROR_UNDEFINED_SET)
        {
            runtimeError(line, "Tried to set value of '%s', but it doesn't exist.", name->chars);
        }

        if(success == TABLE_ERROR_FUNCTION_SET)
        {
            runtimeError(line, "Changing value of functions is illegal.", name->chars);
        }

        return false;
    }

    return true;
}

bool environmentGet(Environment* env, ObjString* name, Value* result)
{
    bool success = tableGet(env->values, name, result);
    if(!success)
    {
        if(env->enclosing != NULL)
        {
            return environmentGet(env->enclosing, name, result);
        }

        return false;
    }

    return true;
}

void printVariables(Environment* env)
{
     while(env != NULL)
     {
         Table* table = env->values;

         for(uint i = 0; i < table->count; i++)
         {
             ObjString* key = table->keys[i];

             Value value;
             tableGet(table, key, &value);

             printf("%s: ", key->chars);
             printRawValue(value);
             putc('\n', stdout);
         }

         env = env->enclosing;
     }
}

void markEnvironment(Environment* env)
{
    markTable(env->values);

    if(env->enclosing != NULL)
    {
        markEnvironment(env->enclosing);
    }
}

void freeEnvironmentsRecursively(Environment* env)
{
    if(env->enclosing != NULL)
    {
        freeEnvironmentsRecursively(env->enclosing);
    }


    freeTable(env->values);
    FREE(Environment, env);
}

void freeEnvironment(Environment* env)
{
    freeTable(env->values);
    FREE(Environment, env);
}


//    FREE_ARRAY(Entry, env->values.entries, env->values.capacity);
//    FREE_ARRAY(Entry, env->values.keys, env->values.capacity);