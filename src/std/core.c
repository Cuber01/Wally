#include "core.h"
#include "wally_math.h"
#include "native_utils.h"
#include "vm.h"

ObjString* boolStringConst;
ObjString* nullStringConst;
ObjString* numberStringConst;
ObjString* functionStringConst;
ObjString* classStringConst;
ObjString* instanceStringConst;
ObjString* stringStringConst;

NATIVE_FUNCTION(print)
{
    printRawValue(args[0]);
    putchar('\n');
    return NULL_VAL;
}

NATIVE_FUNCTION(include)
{
    ObjString* moduleName = AS_STRING(args[0]);

    if(true) //stringEqualToChar(moduleName, "math", 4)
    {
        defineMath(&vm.nativeEnvironment->values);
    }
    else
    {
        // todo error handle
    }

    return NULL_VAL;
}

NATIVE_FUNCTION(type)
{
    Value value = args[0];

    if (IS_BOOL(value))
    {
        return OBJ_VAL((Obj*)boolStringConst);
    }
    else if (IS_NULL(value))
    {
        return OBJ_VAL((Obj*)nullStringConst);
    }
    else if (IS_NUMBER(value))
    {
        return OBJ_VAL((Obj*)numberStringConst);
    }
    else if (IS_STRING(value))
    {
        return OBJ_VAL((Obj*)stringStringConst);
    }
    else if (IS_INSTANCE(value))
    {
        return OBJ_VAL((Obj*)instanceStringConst);
    }
    else if (IS_CLASS(value))
    {
        return OBJ_VAL((Obj*)classStringConst);
    }
    else if (IS_FUNCTION(value) || IS_BOUND_METHOD(value) || IS_NATIVE(value))
    {
        return OBJ_VAL((Obj*)functionStringConst);
    }
    else
    {
        printf("Unreachable reached");
        return NULL_VAL;
    }

}

void defineCore(Table* table)
{
    defineNativeFunction(table, "print", printNative);
    defineNativeFunction(table, "type", typeNative);
    defineNativeFunction(table, "include",includeNative);

    boolStringConst = copyString("bool", 4);
    nullStringConst = copyString("null", 4);
    numberStringConst = copyString("string", 6);
    functionStringConst = copyString("function", 8);
    classStringConst = copyString("class", 5);
    instanceStringConst = copyString("instance", 8);
    stringStringConst = copyString("string", 5);
}
