#ifndef WALLY_CHUNK_H
#define WALLY_CHUNK_H

#include "common.h"
#include "value.h"

typedef enum {
    // Literals
    OP_CONSTANT,
    OP_NULL,
    OP_TRUE,
    OP_FALSE,

    // Unary operations
    OP_NEGATE,
    OP_NOT,

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
    OP_DEFINE_VARIABLE,
    OP_GET_VARIABLE,
    OP_SET_VARIABLE,

    // Block
    OP_BLOCK_START,
    OP_BLOCK_END,

    // Control flow
    OP_JUMP_IF_FALSE,
    OP_JUMP_IF_TRUE,
    OP_JUMP,
    OP_LOOP,

    // Functions
    OP_CALL,
    OP_RETURN,
    OP_DEFINE_FUNCTION,

    // Misc
    OP_POP,
    OP_POP_N,
    OP_TERNARY,
    OP_SWITCH_EQUAL,
} OpCode;

typedef struct {
    unsigned int codeCount;
    unsigned int codeCapacity;

    unsigned int lineCount;
    unsigned int lineCapacity;

    uint8_t* code;
    uint32_t* lines;

    ValueArray constants;
} Chunk;

void initChunk(Chunk* chunk);
void writeChunk(Chunk* chunk, uint8_t byte, unsigned int line);
void freeChunk(Chunk* chunk);
int addConstant(Chunk* chunk, Value value);

#endif //WALLY_CHUNK_H
