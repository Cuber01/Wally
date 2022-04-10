#ifndef WALLY_CHUNK_H
#define WALLY_CHUNK_H

#include "common.h"
#include "value.h"

typedef enum {
    OP_RETURN,

    // Literals
    OP_CONSTANT,
    OP_NIL,
    OP_TRUE,
    OP_FALSE,

    // Unary operations
    OP_NEGATE,
    OP_NOT,

    // Statements
    OP_PRINT,

    // Comparison
    OP_EQUAL,
    OP_NOT_EQUAL,
    OP_GREATER,
    OP_GREATER_EQUAL,
    OP_LESS,
    OP_LESS_EQUAL,

    // Mathematical binary operations
    OP_ADD,
    OP_SUBTRACT,
    OP_MULTIPLY,
    OP_DIVIDE,

    // Variables
    OP_DEFINE_GLOBAL,
    OP_GET_GLOBAL,
    OP_SET_GLOBAL,
    OP_GET_LOCAL,
    OP_SET_LOCAL,

    // Misc
    OP_POP,
} OpCode;

typedef struct {
    int count;
    int capacity;
    uint8_t* code;

    int* lines;
    ValueArray constants;
} Chunk;

void initChunk(Chunk* chunk);
void writeChunk(Chunk* chunk, uint8_t byte, int line);
void freeChunk(Chunk* chunk);
int addConstant(Chunk* chunk, Value value);

#endif //WALLY_CHUNK_H
