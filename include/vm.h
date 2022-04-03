#ifndef WALLY_VM_H
#define WALLY_VM_H

#include "chunk.h"

typedef struct {
    Chunk* chunk;
    uint8_t* ip; // Instruction pointer. Points towards the next instruction to be executed.
} VM;

typedef enum {
    INTERPRET_OK,
    INTERPRET_COMPILE_ERROR,
    INTERPRET_RUNTIME_ERROR
} InterpretResult;

void initVM();
void freeVM();
InterpretResult interpret(Chunk* chunk);

#endif //WALLY_VM_H
