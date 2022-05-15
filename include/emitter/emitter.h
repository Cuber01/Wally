#include "list.h"
#include "environment.h"

#ifndef WALLY_EMITTER_H
#define WALLY_EMITTER_H

typedef enum
{
    TYPE_FUNCTION,
    TYPE_METHOD,
    TYPE_SCRIPT
} FunctionType;

typedef struct {
    ObjFunction* function;
    FunctionType type;

    struct Compiler* enclosing;
} Compiler;

ObjFunction* emit(Node* statements);
void markCompilerRoots();

#endif //WALLY_EMITTER_H