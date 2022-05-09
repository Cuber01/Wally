#include <stdlib.h>
#include <stdarg.h>

#include "parser.h"
#include "scanner.h"
#include "chunk.h"
#include "object.h"
#include "memory.h"

#ifdef DEBUG_PRINT_BYTECODE
#include "disassembler.h"
#include "emitter.h"
#include "memory.h"

#endif

#ifdef DEBUG_PRINT_TOKENS
#include "token_printer.h"
#endif

Parser parser;

ParseRule rules[];

// region ERROR

static void errorAt(Token* token, const char* message)
{
    if (parser.panicMode) return;

    parser.panicMode = true;
    fprintf(stderr, "[line %d] Error", token->line);

    if (token->type == TOKEN_EOF)
    {
        fprintf(stderr, " at end");
    } else if (token->type == TOKEN_ERROR)
    {
        // Nothing.
    } else
    {
        fprintf(stderr, " at '%.*s'", token->length, token->start);
    }

    fprintf(stderr, ": %s\n", message);
    parser.hadError = true;
}

static void errorAtCurrent(const char* message)
{
    errorAt(&parser.current, message);
}

static void error(const char* message)
{
    errorAt(&parser.previous, message);
}

// endregion

// region UTIL

static void advance()
{
    parser.previous = parser.current;

    for (;;)
    {
        parser.current = scanToken();
        if (parser.current.type != TOKEN_ERROR) break;

        errorAtCurrent(parser.current.start);
    }
}

static void consume(TokenType type, const char* message)
{
    if (parser.current.type == type)
    {
        advance();
        return;
    }

    errorAtCurrent(message);
}

static bool check(TokenType type)
{
    return parser.current.type == type;
}

static bool match(TokenType type)
{
    if (!check(type)) return false;

    advance();
    return true;
}

static TokenType matchMultiple(TokenType first, ...)
{
    TokenType next = first;
    va_list lst;
    va_start(lst, first);

    while((void *) next != NULL)
    {
        if(match(next))
        {
            return next;
        }

        next = va_arg(lst, TokenType);
    }

    va_end(lst);

    return 0;
}

static bool doubleMatch(TokenType type)
{
    return ( match(type) && match(type) );
}

static bool identifiersEqual(Token* a, Token* b)
{
    if (a->length != b->length) return false;
    return memcmp(a->start, b->start, a->length) == 0;
}

static void synchronize()
{
    parser.panicMode = false;

    while (parser.current.type != TOKEN_EOF)
    {
        // Go until we find something that looks like a new statement boundary
        if (parser.previous.type == TOKEN_SEMICOLON) return;
        switch (parser.current.type)
        {
            case TOKEN_CLASS:
            case TOKEN_FUNCTION:
            case TOKEN_VAR:
            case TOKEN_FOR:
            case TOKEN_IF:
            case TOKEN_WHILE:
            case TOKEN_RETURN:
                return;

            default: ; // Nothing.
        }

        advance();
    }
}

// endregion

// region PRECEDENCE

static ParseRule* getRule(TokenType type)
{
    return &rules[type];
}

static Expr* parsePrecedence(Precedence precedence)
{
    advance();

    ParsePrefixFn prefixRule = getRule(parser.previous.type)->prefix;
    if (prefixRule == NULL)
    {
        error("Expect expression.");
        return NULL;
    }

    Expr* expr = prefixRule();

    while (precedence <= getRule(parser.current.type)->precedence)
    {
        advance();
        ParseInfixFn infixRule = getRule(parser.previous.type)->infix;
        expr = (Expr*)infixRule(expr);
    }

    return expr;
}

static Expr* expression()
{
    return parsePrecedence(PREC_ASSIGNMENT);
}

// endregion

// region EXPRESSIONS

static Expr* binary(Expr* previous)
{
    TokenType operatorType = parser.previous.type;
    ParseRule* rule = getRule(operatorType);
    // We want to use a higher level because binary is left associative
    Expr* right = parsePrecedence((Precedence)(rule->precedence + 1));

    return (Expr*)newBinaryExpr(previous, operatorType, right, parser.line);
}

static Expr* ternary(Expr* previous)
{
    Expr* thenBranch = parsePrecedence(PREC_TERNARY);
    consume(TOKEN_COLON, "Expect ':' after first ternary branch.");
    Expr* elseBranch = parsePrecedence(PREC_ASSIGNMENT);
    return (Expr*)newTernaryExpr(previous, thenBranch, elseBranch, parser.line);
}

static Expr* literal()
{
    switch (parser.previous.type)
    {
        case TOKEN_FALSE: return (Expr*)newLiteralExpr(BOOL_VAL(false), parser.line);
        case TOKEN_NULL:  return (Expr*)newLiteralExpr(NULL_VAL, parser.line);
        case TOKEN_TRUE:  return (Expr*)newLiteralExpr(BOOL_VAL(true), parser.line);
        default:
            return NULL; // Unreachable.
    }
}

static Expr* number()
{
    double value = strtod(parser.previous.start, NULL);
    return (Expr*)newLiteralExpr(NUMBER_VAL(value), parser.line);
}

static void escapeSequences(char* destination, char* source)
{
    typedef enum
    {
        ST_COPY,
        ST_REPLACE
    } CopyType;

    static CopyType state = ST_COPY;

    while (*source != 0)
    {

        switch (state)
        {
            case ST_COPY:
                if(*source != '\\')
                {
                    *destination = *source;
                    destination++;
                }
                else
                {
                    state = ST_REPLACE;
                }
                break;

            case ST_REPLACE:
                switch (*source)
                {
                    case 'n': *destination = '\n'; break;
                    case 'f': *destination = '\f'; break;
                    case 'r': *destination = '\r'; break;
                    case 'b': *destination = '\b'; break;
                    case 't': *destination = '\t'; break;
                    case 'v': *destination = '\v'; break;
                    case '"': *destination = '\"'; break;

                    default: *destination = *source;
                }

                destination++;

                state = ST_COPY;
                break;

            default:
                break; // Unreachable

        }

        source++;

    }

    *destination = 0;

}

static Expr* string()
{
    // Math is for trimming ""
    uint32_t length = strlen(parser.previous.start);
    char str[length];

    strcpy( str, parser.previous.start + 1 );
    str[parser.previous.length - 2] = 0;

    escapeSequences(str, str);

    return (Expr*)newLiteralExpr(OBJ_VAL(copyString(str, parser.previous.length - 2)), parser.line);
}

static Expr* interpolatedString()
{

}

static Expr* unary()
{
    TokenType operatorType = parser.previous.type;

    Expr* expr = parsePrecedence(PREC_UNARY);

    return (Expr*) newUnaryExpr(expr, operatorType, parser.line);
}

static Expr* grouping()
{
    Expr* expr = expression();
    consume(TOKEN_RIGHT_PAREN, "Expect ')' after expression.");
    return expr;
}

static Node* argumentList(uint16_t* argCount)
{
    Node* root = NULL;

    if (!check(TOKEN_RIGHT_PAREN))
    {
        do
        {

            Expr* arg = expression();
            listAdd(&root, NODE_EXPRESSION_VALUE(arg));
            (*argCount)++;

        } while (match(TOKEN_COMMA));
    }
    consume(TOKEN_RIGHT_PAREN, "Expect ')' after arguments.");

    return root;
}

static ObjString* parseVariableName(const char* errorMessage)
{
    consume(TOKEN_IDENTIFIER, errorMessage);

    return copyString(parser.previous.start,parser.previous.length);
}

static Expr* call(Expr* previous)
{
    uint16_t argCount = 0;
    Node* args = argumentList(&argCount);
    return (Expr*) newCallExpr(((VarExpr*)previous)->name, argCount, args, parser.line);
}

// endregion

// region STATEMENTS
static Stmt* declaration();
static Stmt* statement();
static Stmt* varDeclaration();

static Stmt* expressionStatement()
{
    Expr* expr = expression();
    consume(TOKEN_SEMICOLON, "Expect ';' after expression.");
    return (Stmt*)newExpressionStmt(expr, parser.line);
}

static Stmt* block()
{
    Node* statements = NULL;

    while (!match(TOKEN_RIGHT_BRACE) && !check(TOKEN_EOF))
    {
        listAdd(&statements, NODE_STATEMENT_VALUE(declaration()));
    }

    return (Stmt*)newBlockStmt(statements, parser.line);
}

static Expr* logical(Expr* previous)
{
    TokenType operatorType = parser.previous.type;
    ParseRule* rule = getRule(operatorType);
    Expr* right = parsePrecedence((Precedence)(rule->precedence + 1));

    return (Expr*)newLogicalExpr(previous, operatorType, right, parser.line);
}

static Stmt* forStatement()
{
    consume(TOKEN_LEFT_PAREN, "Expect '(' after 'for'.");

    // Initializer clause
    Stmt* declaration = NULL;
    if (match(TOKEN_SEMICOLON))
    {
        // No initializer.
    }
    else if (match(TOKEN_VAR))
    {
        declaration = varDeclaration();
    }
    else
    {
        declaration = expressionStatement();
    }

    Expr* condition = NULL;
    if (!match(TOKEN_SEMICOLON))
    {
        condition = expression();
        consume(TOKEN_SEMICOLON, "Expect ';' after loop condition.");
    }

    // Increment clause
    Expr* increment = NULL;
    if (!match(TOKEN_RIGHT_PAREN))
    {
        increment = expression();

        consume(TOKEN_RIGHT_PAREN, "Expect ')' after for clauses.");
    }

    Stmt* body = statement();

    return (Stmt*)newForStmt(declaration, condition, increment, body, parser.line);
}

static Stmt* breakStatement()
{
//    if(innermostLoopDepth == -1)
//    {
//        error("Can't use 'break' statements outside of loops.");
//    }
//
//    consume(TOKEN_SEMICOLON, "Expect ';' after 'break'.");
//
//    for (int i = current->localCount - 1;
//         i >= 0 && current->locals[i].depth > innermostLoopDepth;
//         i--)
//    {
//        emitByte(OP_POP); POPN
//    }
//
//    emitJump(innermostLoopEnd);
}

static Stmt* continueStatement()
{
//    if (innermostLoopStart == -1) {
//        error("Can't use 'continue' outside of a loop.");
//    }
//
//    consume(TOKEN_SEMICOLON, "Expect ';' after 'continue'.");
//
//    // Discard any locals created inside the loop.
//    emitMultiplePop(current->localCount - 1);
//    current->localCount = 0;
//
//    // Jump to top of current innermost loop.
//    emitLoop(innermostLoopStart);
}

static Stmt* whileStatement()
{
    consume(TOKEN_LEFT_PAREN, "Expect '(' after 'while'.");
    Expr* condition = expression();
    consume(TOKEN_RIGHT_PAREN, "Expect ')' after condition.");

    Stmt* body = statement();

    return (Stmt*)newWhileStmt(condition, body, parser.line);
}

static Stmt* ifStatement()
{
    consume(TOKEN_LEFT_PAREN, "Expect '(' after 'if'.");
    Expr* condition = expression();
    consume(TOKEN_RIGHT_PAREN, "Expect ')' after condition.");

    Stmt* thenBranch = statement();
    Stmt* elseBranch = NULL;

    if (match(TOKEN_ELSE))
    {
        elseBranch = statement();
    }

    return (Stmt*)newIfStmt(condition, thenBranch, elseBranch, parser.line);
}

static Stmt* returnStatement()
{
    Expr* returnValue = NULL;

    if (!match(TOKEN_SEMICOLON))
    {
        returnValue = expression();
        consume(TOKEN_SEMICOLON, "Expect ';' after return value.");
    }

    return (Stmt*)newReturnStmt(returnValue, parser.line);
}

static Stmt* switchStatement()
{
    consume(TOKEN_LEFT_PAREN, "Expect '(' after 'switch'.");
    Expr* value = expression();
    consume(TOKEN_RIGHT_PAREN, "Expect ')' after condition.");

    consume(TOKEN_LEFT_BRACE, "Expect '{' after ')'.");

    Stmt* defaultCase = NULL;
    Node* caseConditions = NULL;
    Node* caseBodies = NULL;

    for(;;)
    {
        if(match(TOKEN_CASE))
        {
            listAdd(&caseConditions, NODE_EXPRESSION_VALUE(expression()));

            consume(TOKEN_COLON, "Expect ':' after expression.");

            listAdd(&caseBodies, NODE_STATEMENT_VALUE(statement()));
        }
        else if (match(TOKEN_DEFAULT))
        {
            consume(TOKEN_COLON, "Expect ':' after 'default'.");

            defaultCase = statement();
        }
        else
        {
            break;
        }
    }

    consume(TOKEN_RIGHT_BRACE, "Expect '}' at the end of switch statement.");

    return (Stmt*)newSwitchStmt(caseConditions, caseBodies, defaultCase, parser.line);
}

/*
 *
 *   consume(TOKEN_LEFT_PAREN, "Expect '(' after 'switch'.");
    expression();
    consume(TOKEN_RIGHT_PAREN, "Expect ')' after condition.");
    consume(TOKEN_LEFT_BRACE, "Expect '{' after ')'.");

    for(;;)
    {
        if(match(TOKEN_CASE))
        {
            expression();
            emitByte(OP_SWITCH_EQUAL);

            int thenJump = emitJump(OP_JUMP_IF_FALSE);

            consume(TOKEN_COLON, "Expect ':' after expression.");

            statement();

            patchJump(thenJump);
            emitByte(OP_POP);
        }
        else if (match(TOKEN_DEFAULT))
        {
            consume(TOKEN_COLON, "Expect ':' after 'default'.");

            statement();
        }
        else
        {
            break;
        }
    }

    emitByte(OP_POP);
    consume(TOKEN_RIGHT_BRACE, "Expect '}' at the end of switch statement.");
}
 *
 */

static Stmt* statement()
{
    parser.line = parser.previous.line;

    if (match(TOKEN_IF))              return ifStatement();
    else if (match(TOKEN_WHILE))      return whileStatement();
    else if (match(TOKEN_FOR))        return forStatement();
    else if (match(TOKEN_LEFT_BRACE)) return block();
    else if (match(TOKEN_BREAK))      return breakStatement();
    else if (match(TOKEN_CONTINUE))   return continueStatement();
    else if (match(TOKEN_SWITCH))     return switchStatement();
    else if (match(TOKEN_RETURN))     return returnStatement();
    else return expressionStatement();

}

static Expr* variable()
{
    ObjString* name = copyString(parser.previous.start,parser.previous.length);

    if(match(TOKEN_EQUAL))
    {
        TokenType operator = matchMultiple(TOKEN_PLUS, TOKEN_MINUS, TOKEN_STAR, TOKEN_SLASH);

        if(operator == 0)
        {
            Expr* value = expression();
            return (Expr*)newAssignExpr(name, value, parser.line);
        }
        else
        {
            return (Expr*)newAssignExpr(name, (Expr*)newBinaryExpr(
                                                (Expr*)newVarExpr(name, parser.line),
                                                operator,
                                                expression(),
                                                parser.line),
                                        parser.line);
        }

    }
    else if(doubleMatch(TOKEN_PLUS))
    {
        return (Expr*)newAssignExpr(name, (Expr*)newBinaryExpr(
                                            (Expr*)newVarExpr(name, parser.line),
                                            TOKEN_PLUS,
                                            (Expr*)newLiteralExpr(NUMBER_VAL(1), parser.line),
                                            parser.line),
                                    parser.line);
    }
    else if(doubleMatch(TOKEN_MINUS))
    {
        return (Expr*)newAssignExpr(name, (Expr*)newBinaryExpr(
                                            (Expr*)newVarExpr(name, parser.line),
                                            TOKEN_MINUS,
                                            (Expr*)newLiteralExpr(NUMBER_VAL(1), parser.line),
                                            parser.line),
                                    parser.line);
    }
    else
    {
        return (Expr*)newVarExpr(name, parser.line);
    }

}

static Stmt* functionDeclaration()
{
    ObjString* name = parseVariableName("Expect function name.");

    consume(TOKEN_LEFT_PAREN, "Expect '(' after function name.");

    // Params
    ObjString** params = reallocate(NULL, 0, sizeof(*params));
    uint16_t paramCount = 0;

    if(!match(TOKEN_RIGHT_PAREN))
    {

        while(true)
        {
            if (paramCount == 0)
            {
                *params = parseVariableName("Expect parameter name.");
            }
            else
            {
                params = reallocate(params, sizeof(params), (paramCount + 1) * sizeof(*params));
                params[paramCount] = parseVariableName("Expect parameter name.");
            }

            paramCount++;

            if(match(TOKEN_RIGHT_PAREN)) break;

            consume(TOKEN_COMMA, "Expect ',' after parameter in function.");
        }
    }

    // Body, can't be handled with 'block' because we need to emit scope start at custom locations
    Node* body = NULL;
    consume(TOKEN_LEFT_BRACE, "Expect '{' at the start of function body.");

    while (!match(TOKEN_RIGHT_BRACE) && !check(TOKEN_EOF))
    {
        listAdd(&body, NODE_STATEMENT_VALUE(declaration()));
    }

    return (Stmt*)newFunctionStmt(name, body, params, paramCount, parser.line);
}

static Stmt* varDeclaration()
{
    // Get name
    ObjString* name = parseVariableName("Expect variable name after 'var'.");

    // Get initializer
    Expr* initializer;
    if (match(TOKEN_EQUAL))
    {
        initializer = expression();
    }
    else
    {
        // Else default to null
        initializer = NULL;
    }

    consume(TOKEN_SEMICOLON, "Expect ';' after variable declaration.");

    return (Stmt*)newVariableStmt(name, initializer, parser.line);
}

static Stmt* declaration()
{
    Stmt* stmt;

    if (match(TOKEN_VAR))           stmt = varDeclaration();
    else if (match(TOKEN_FUNCTION)) stmt = functionDeclaration();
    else
    {
        stmt = statement();
    }

    if (parser.panicMode) synchronize();
    return stmt;
}

// endregion

// region MAIN

#ifdef DEBUG_PRINT_TOKENS
void printTokens();
#endif
Node* compile(const char* source)
{
    initScanner(source);

    parser.hadError = false;
    parser.panicMode = false;
    parser.line = 1;

    #ifdef DEBUG_PRINT_TOKENS
    printTokens();
    return NULL;
    #endif

    advance();

    Node* statements = NULL;

    while (!match(TOKEN_EOF))
    {
        listAdd(&statements, (NodeValue){.as.statement = declaration()});
    }

    consume(TOKEN_EOF, "Expect end of expression.");

    return parser.hadError ? NULL : statements;
}

ParseRule rules[] =
{
        // Prefix vs Infix
        // Prefix examples: -1, (1, 1, "str"
        // In prefix the expression token is on the left of the rest of the expression or is the expression itself (1, "str")
        //
        // Infix examples: 1 + 1, 2 * 2,
        // In prefix the expression token is in the middle of other two expressions

        //                       prefix                infix   precedence
        [TOKEN_LEFT_PAREN]    = {grouping,             call,   PREC_CALL},
        [TOKEN_RIGHT_PAREN]   = {NULL,                 NULL,   PREC_NONE},
        [TOKEN_LEFT_BRACE]    = {NULL,                 NULL,   PREC_NONE},
        [TOKEN_RIGHT_BRACE]   = {NULL,                 NULL,   PREC_NONE},
        [TOKEN_COMMA]         = {NULL,                 NULL,   PREC_NONE},
        [TOKEN_DOT]           = {NULL,                 NULL,   PREC_NONE},
        [TOKEN_MINUS]         = {unary,                binary, PREC_TERM},
        [TOKEN_PLUS]          = {NULL,                 binary, PREC_TERM},
        [TOKEN_SEMICOLON]     = {NULL,                 NULL,   PREC_NONE},
        [TOKEN_SLASH]         = {NULL,                 binary, PREC_FACTOR},
        [TOKEN_STAR]          = {NULL,                 binary, PREC_FACTOR},
        [TOKEN_BANG]          = {unary,                NULL,   PREC_NONE},
        [TOKEN_BANG_EQUAL]    = {NULL,                 binary, PREC_EQUALITY},
        [TOKEN_EQUAL]         = {NULL,                 binary, PREC_COMPARISON},
        [TOKEN_EQUAL_EQUAL]   = {NULL,                 binary, PREC_EQUALITY},
        [TOKEN_GREATER]       = {NULL,                 binary, PREC_COMPARISON},
        [TOKEN_GREATER_EQUAL] = {NULL,                 binary, PREC_COMPARISON},
        [TOKEN_LESS]          = {NULL,                 binary, PREC_COMPARISON},
        [TOKEN_LESS_EQUAL]    = {NULL,                 binary, PREC_COMPARISON},
        [TOKEN_IDENTIFIER]    = {variable,             NULL,   PREC_NONE},
        [TOKEN_DOLLAR]        = {interpolatedString,   NULL,   PREC_NONE},
        [TOKEN_STRING]        = {string,               NULL,   PREC_NONE},
        [TOKEN_NUMBER]        = {number,               NULL,   PREC_NONE},
        [TOKEN_AND]           = {NULL,                 logical,PREC_AND},
        [TOKEN_CLASS]         = {NULL,                 NULL,   PREC_NONE},
        [TOKEN_ELSE]          = {NULL,                 NULL,   PREC_NONE},
        [TOKEN_QUESTION_MARK] = {NULL,                 ternary,PREC_TERNARY},
        [TOKEN_COLON]         = {NULL,                 NULL,   PREC_NONE},
        [TOKEN_FALSE]         = {literal,              NULL,   PREC_NONE},
        [TOKEN_FOR]           = {NULL,                 NULL,   PREC_NONE},
        [TOKEN_FUNCTION]      = {NULL,                 NULL,   PREC_NONE},
        [TOKEN_IF]            = {NULL,                 NULL,   PREC_NONE},
        [TOKEN_NULL]          = {literal,              NULL,   PREC_NONE},
        [TOKEN_OR]            = {NULL,                 logical,PREC_OR},
        [TOKEN_RETURN]        = {NULL,                 NULL,   PREC_NONE},
        [TOKEN_SUPER]         = {NULL,                 NULL,   PREC_NONE},
        [TOKEN_THIS]          = {NULL,                 NULL,   PREC_NONE},
        [TOKEN_TRUE]          = {literal,              NULL,   PREC_NONE},
        [TOKEN_VAR]           = {NULL,                 NULL,   PREC_NONE},
        [TOKEN_WHILE]         = {NULL,                 NULL,   PREC_NONE},
        [TOKEN_ERROR]         = {NULL,                 NULL,   PREC_NONE},
        [TOKEN_EOF]           = {NULL,                 NULL,   PREC_NONE},
};

#ifdef DEBUG_PRINT_TOKENS
void printTokens()
{
    int line = -1;
    for (;;)
    {
        Token token = scanToken();
        bool isNewLine = false;

        if (token.line != line)
        {
            line = token.line;
            isNewLine = true;
        }

        printToken(token, isNewLine);

        if (token.type == TOKEN_EOF) break;
    }
}
#endif

// endregion