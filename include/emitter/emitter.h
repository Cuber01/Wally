#include "list.h"
#include "environment.h"

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
    Chunk* chunk;
} Compiler;

Chunk* emit(Node* statements);

#endif //WALLY_EMITTER_H
