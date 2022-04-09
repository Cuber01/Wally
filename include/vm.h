#ifndef WALLY_VM_H
#define WALLY_VM_H

#include "chunk.h"
#include "table.h"
#define STACK_MAX 256


typedef struct {
    Chunk* chunk;
    uint8_t* ip; // Instruction pointer. Points towards the next instruction to be executed.

    Value stack[STACK_MAX];
    Value* stackTop; // Points towards where the next pushed value will go, a.k.a. an empty place in the stack array.

    Table strings;
    Table globals;
    Obj* objects;
} VM;

typedef enum {
    INTERPRET_OK,
    INTERPRET_COMPILE_ERROR,
    INTERPRET_RUNTIME_ERROR
} InterpretResult;

extern VM vm;

void initVM();
void freeVM();
InterpretResult interpret(const char* source);

#endif //WALLY_VM_H
