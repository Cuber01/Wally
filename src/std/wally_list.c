#include "list.h"
#include "native_utils.h"

NATIVE_FUNCTION(join)
{
    CHECK_ARG_COUNT("join", 2);

    // if strings
    if(IS_STRING(args[0]) && IS_STRING(args[1]))
    {
        return OBJ_VAL(addStrings(AS_STRING(args[0]) , AS_STRING(args[1]) ) );
    }

    // else lists
    ObjWList* list1 = AS_LIST(args[0]);
    ObjWList* list2 = AS_LIST(args[1]);

    for(uint i = 0; i < list2->count; i++)
    {
        addWList(list1, getIndexWList(list2, i));
    }

    return OBJ_VAL(list1);
}

NATIVE_FUNCTION(append)
{
    CHECK_ARG_COUNT("append", 2);

    addWList(AS_LIST(args[0]), args[1]);

    return NULL_VAL;
}

NATIVE_FUNCTION(remove)
{
    CHECK_ARG_COUNT("remove", 2);

    removeIndexWList(AS_LIST(args[0]), AS_NUMBER(args[1]));

    return NULL_VAL;
}

NATIVE_FUNCTION(count)
{
    CHECK_ARG_COUNT("count", 1);

    return NUMBER_VAL(AS_LIST(args[0])->count);
}

void defineList(Table* table)
{
    ObjClass* list = newClass(copyString("list", 4));

    #define DEFINE_LIST_METHOD(name, method) defineNativeFunction(list->methods, name, method)

    DEFINE_LIST_METHOD("join",   joinNative);
    DEFINE_LIST_METHOD("remove", removeNative);
    DEFINE_LIST_METHOD("count",  countNative);
    DEFINE_LIST_METHOD("append", appendNative);

    #undef DEFINE_LIST_METHOD

    ObjInstance* instance = newInstance(list);

    tableDefineEntry(
            table,
            list->name,
            OBJ_VAL(instance)
    );
}