#ifndef WALLY_SCANNER_H
#define WALLY_SCANNER_H

#include <stdio.h>
#include <string.h>

#include "common.h"

typedef enum {
    TOKEN_NONE,

    // Single-character tokens.
    TOKEN_LEFT_PAREN, TOKEN_RIGHT_PAREN, TOKEN_MINUS_MINUS,
    TOKEN_LEFT_BRACE, TOKEN_RIGHT_BRACE,
    TOKEN_COMMA, TOKEN_DOT, TOKEN_MINUS, TOKEN_PLUS,
    TOKEN_SEMICOLON, TOKEN_SLASH, TOKEN_STAR,
    TOKEN_DOLLAR, TOKEN_PLUS_PLUS,

    // One or two character tokens.
    TOKEN_BANG, TOKEN_BANG_EQUAL,
    TOKEN_EQUAL, TOKEN_EQUAL_EQUAL,
    TOKEN_GREATER, TOKEN_GREATER_EQUAL,
    TOKEN_LESS, TOKEN_LESS_EQUAL,
    TOKEN_AND, TOKEN_OR, TOKEN_COLON,
    TOKEN_QUESTION_MARK,

    // Literals.
    TOKEN_IDENTIFIER, TOKEN_STRING, TOKEN_NUMBER,

    // Keywords.
    TOKEN_CLASS, TOKEN_ELSE, TOKEN_FALSE,
    TOKEN_FOR, TOKEN_FUNCTION, TOKEN_IF, TOKEN_NULL,
    TOKEN_RETURN, TOKEN_BASE, TOKEN_THIS,
    TOKEN_TRUE, TOKEN_VAR, TOKEN_WHILE, TOKEN_BREAK,
    TOKEN_CONTINUE, TOKEN_SWITCH, TOKEN_CASE,
    TOKEN_DEFAULT,

    TOKEN_ERROR, TOKEN_EOF
} TokenType;

typedef struct {
    TokenType type;

    const char* start;
    int length;
    uint16_t line;
} Token;

typedef struct {
    const char* start;
    const char* current;
    uint line;

    TokenType returnNext;
} Scanner;

extern Scanner scanner;

void initScanner(const char* source);
Token scanToken();

bool isDigit(char c);
bool isAlpha(char c);

#endif //WALLY_SCANNER_H
