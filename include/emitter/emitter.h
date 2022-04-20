#include "list.h"
#include "environment.h"

#ifndef WALLY_EMITTER_H
#define WALLY_EMITTER_H

typedef struct {
    struct Compiler* enclosing;
    Chunk* chunk;
} Compiler;

Chunk* emit(Node* statements);

#endif //WALLY_EMITTER_H
