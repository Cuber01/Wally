#include "list.h"

#ifndef WALLY_EMITTER_H
#define WALLY_EMITTER_H

typedef struct {
    Token name;
    int depth;
} Local;

typedef enum {
    TYPE_FUNCTION,
    TYPE_SCRIPT
} FunctionType;

typedef struct {
    uint8_t index;
    bool isLocal;
} Upvalue;


typedef struct {
    struct Compiler* enclosing;
    ObjFunction* function;
    FunctionType type;

    Upvalue upvalues[UINT8_COUNT];
    Local locals[UINT8_COUNT];

    int localCount;
    int scopeDepth;
} Compiler;

ObjFunction* emit(Node* statements);

#endif //WALLY_EMITTER_H
