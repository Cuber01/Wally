#ifndef WALLY_COMPILER_H
#define WALLY_COMPILER_H

#include "chunk.h"
#include "scanner.h"

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

typedef void (*ParseFn)();

typedef struct {
    ParseFn prefix;
    ParseFn infix;
    Precedence precedence;
} ParseRule;

bool compile(const char* source, Chunk* chunk);

#endif //WALLY_COMPILER_H
