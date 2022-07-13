#include "list.h"
#include "environment.h"
#include "object.h"

#ifndef WALLY_EMITTER_H
#define WALLY_EMITTER_H

typedef struct ObjFunction ObjFunction;
typedef struct Node Node;

typedef enum
{
    TYPE_FUNCTION,
    TYPE_INITIALIZER,
    TYPE_METHOD,
    TYPE_SCRIPT
} FunctionType;

typedef struct Compiler {
    ObjFunction* function;

    struct Compiler* enclosing;
} Compiler;

ObjFunction* emit(Node* statements);
void markCompilerRoots();

#endif //WALLY_EMITTER_H