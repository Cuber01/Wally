#ifndef WALLY_VM_H
#define WALLY_VM_H

#include "chunk.h"
#include "table.h"
#include "object.h"

#define FRAMES_MAX 64
#define STACK_MAX (FRAMES_MAX * UINT8_COUNT)

typedef struct {
    ObjFunction* function;
    uint8_t* ip;
    Value* slots;
} CallFrame;

typedef struct {
    CallFrame frames[FRAMES_MAX];
    int frameCount; // Instruction pointer. Points towards the next instruction to be executed.

    Value stack[STACK_MAX];
    Value* stackTop; // Points towards where the next pushed value will go, a.k.a. an empty place in the stack array.

    Table strings;
    Table globals;
    Obj* objects;
} VM;

extern VM vm;

void initVM();
void freeVM();
int interpret(const char* source);

#endif //WALLY_VM_H
