#ifndef WALLY_COMPILER_H
#define WALLY_COMPILER_H

#include "chunk.h"
#include "scanner.h"
#include "object.h"
#include "list.h"

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

    Node* statements;

    int localCount;
    int scopeDepth;
} Compiler;

typedef struct {
    bool hadError;
    bool panicMode;

    Token current;
    Token previous;
} Parser;

/*
The further we go, the biggest number we get
PREC_NONE is 0
and PREC_PRIMARY is 10
*/
typedef enum {
    PREC_NONE,
    PREC_ASSIGNMENT,  // =
    PREC_TERNARY,     // ?:
    PREC_OR,          // or
    PREC_AND,         // and
    PREC_EQUALITY,    // == !=
    PREC_COMPARISON,  // < > <= >=
    PREC_TERM,        // + -
    PREC_FACTOR,      // * /
    PREC_UNARY,       // ! -
    PREC_CALL,        // . ()
    PREC_PRIMARY
} Precedence;

typedef Expr* (*ParseFn)(bool canAssign);

typedef struct {
    ParseFn prefix;
    ParseFn infix;
    Precedence precedence;
} ParseRule;

Node* compile(const char* source);

#endif //WALLY_COMPILER_H
