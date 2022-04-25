#include "core.h"

void print(Node* args)
{
    printRawValue(* (listGet(args, 0).as.val) );
    putchar('\n');
}