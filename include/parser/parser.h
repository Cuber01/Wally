#ifndef WALLY_PARSER_H
#define WALLY_PARSER_H

#include "chunk.h"
#include "scanner.h"
#include "object.h"
#include "list.h"

typedef struct {
    Node* statements;
    uint16_t line;

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
    PREC_ASSIGNMENT,          // =
    PREC_TERNARY,             // ?:
    PREC_OR,                  // or
    PREC_AND,                 // and
    PREC_EQUALITY,            // == !=
    PREC_COMPARISON,          // < > <= >=
    PREC_TERM,                // + -
    PREC_FACTOR,              // * /
    PREC_UNARY,               // ! -
    PREC_CALL,                // . ()
    PREC_INCR_DECR,           // ++ --
    PREC_PRIMARY
} Precedence;

typedef Expr* (*ParsePrefixFn)(bool canAssign);
typedef Expr* (*ParseInfixFn)(Expr* previous, bool canAssign);

typedef struct {
    ParsePrefixFn prefix;
    ParseInfixFn infix;
    Precedence precedence;
} ParseRule;

Node* compile(const char* source);

#endif //WALLY_PARSER_H
