#ifndef WALLY_VM_H
#define WALLY_VM_H

#include "chunk.h"
#include "table.h"
#include "object.h"
#include "environment.h"

#define FRAMES_MAX 64
#define STACK_MAX (FRAMES_MAX * UINT8_COUNT)

#define INTERPRET_OK 0
#define INTERPRET_RUNTIME_ERROR 70
#define INTERPRET_COMPILE_ERROR 65

typedef struct
{
    // -- Vm Runtime --
    Environment* currentEnvironment;
    Environment* currentClosure;
    ObjFunction* currentFunction;
    ObjClass* currentParent;

    Environment* nativeEnvironment;

    uint8_t* ip; // Instruction pointer. Points towards the next instruction to be executed.

    // -- Global strings --

    ObjString* thisString;
    ObjString* initString;

    // -- Vm Runtime Data --
    Value stack[STACK_MAX];
    Value* stackTop; // Points towards where the next pushed value will go, a.k.a. an empty place in the stack array.

    Table* strings;
    Obj* objects;

    // -- Garbage Collector --
    int grayCount;
    int grayCapacity;
    Obj** grayStack;

    size_t bytesAllocated;
    size_t nextGC;
} VM;

extern VM vm;

Value pop();
void push(Value value);

void initVM();
void freeVM();
int interpret(const char* source);

void runtimeError(uint16_t line, const char* format, ...);

#endif //WALLY_VM_H
