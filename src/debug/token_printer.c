#include "token_printer.h"
#include "scanner.h"
#include "colors.h"

char* tokenEnumToChar(TokenType type)
{
    switch (type)
    {
        case TOKEN_LEFT_BRACE: return "TOKEN_LEFT_BRACE";
        case TOKEN_RIGHT_BRACE: return "TOKEN_RIGHT_BRACE";
        case TOKEN_COMMA: return "TOKEN_COMMA";
        case TOKEN_DOT: return "TOKEN_DOT";
        case TOKEN_MINUS: return "TOKEN_MINUS";
        case TOKEN_PLUS: return "TOKEN_PLUS";
        case TOKEN_SEMICOLON: return "TOKEN_SEMICOLON";
        case TOKEN_SLASH: return "TOKEN_SLASH";
        case TOKEN_STAR: return "TOKEN_STAR";
        case TOKEN_BANG: return "TOKEN_BANG";
        case TOKEN_BANG_EQUAL: return "TOKEN_BANG_EQUAL";
        case TOKEN_EQUAL: return "TOKEN_EQUAL";
        case TOKEN_EQUAL_EQUAL: return "TOKEN_EQUAL_EQUAL";
        case TOKEN_GREATER: return "TOKEN_GREATER";
        case TOKEN_GREATER_EQUAL: return "TOKEN_GREATER_EQUAL";
        case TOKEN_LESS_EQUAL: return "TOKEN_LESS_EQUAL";
        case TOKEN_IDENTIFIER: return "TOKEN_IDENTIFIER";
        case TOKEN_STRING: return "TOKEN_STRING";
        case TOKEN_NUMBER: return "TOKEN_NUMBER";
        case TOKEN_AND: return "TOKEN_AND";
        case TOKEN_CLASS: return "TOKEN_CLASS";
        case TOKEN_ELSE: return "TOKEN_ELSE";
        case TOKEN_FALSE: return "TOKEN_FALSE";
        case TOKEN_FOR: return "TOKEN_FOR";
        case TOKEN_FUNCTION: return "TOKEN_FUNCTION";
        case TOKEN_IF: return "TOKEN_IF";
        case TOKEN_NULL: return "TOKEN_NULL";
        case TOKEN_OR: return "TOKEN_OR";
        case TOKEN_RETURN: return "TOKEN_RETURN";
        case TOKEN_SUPER: return "TOKEN_SUPER";
        case TOKEN_THIS: return "TOKEN_THIS";
        case TOKEN_TRUE: return "TOKEN_TRUE";
        case TOKEN_VAR: return "TOKEN_VAR";
        case TOKEN_WHILE: return "TOKEN_WHILE";
        case TOKEN_ERROR: return "TOKEN_ERROR";
        case TOKEN_EOF: return "TOKEN_EOF";
        case TOKEN_LEFT_PAREN: return "TOKEN_LEFT_PAREN";
        case TOKEN_RIGHT_PAREN: return "TOKEN_RIGHT_PAREN";
        case TOKEN_LESS: return "TOKEN_LESS";
        case TOKEN_DOLLAR: return "TOKEN_DOLLAR";
        case TOKEN_COLON:  return "TOKEN_COLON";
        case TOKEN_QUESTION_MARK: return "TOKEN_QUESTION_MARK";
        case TOKEN_BREAK: return "TOKEN_BREAK";
        case TOKEN_CONTINUE: return "TOKEN_CONTINUE";
        case TOKEN_CASE:    return "TOKEN_CASE";
        case TOKEN_DEFAULT: return "TOKEN_DEFAULT";
        case TOKEN_SWITCH:  return "TOKEN_SWITCH";
        default: return "! PRINTED THE UNPRINTABLE !";
    }
}

void printToken(Token token, bool isNewLine)
{
    if (isNewLine)
    {
        printf("%4d ", token.line);
    }
    else
    {
        printf("   | ");
    }

    char* text = tokenEnumToChar(token.type);
    // 19 is longest token's length (TOKEN_GREATER_EQUAL)
    uint8_t spacesToPrint = 19 - strlen(text);

    printf(GREEN);
    printf("%s  ", text);
    while (spacesToPrint--)
    {
        putchar(' ');
    }
    printf(COLOR_CLEAR);

    printf("'%.*s'\n", token.length, token.start);

}