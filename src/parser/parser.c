#include <stdlib.h>

#include "parser.h"
#include "scanner.h"
#include "chunk.h"
#include "object.h"

#ifdef DEBUG_PRINT_BYTECODE
#include "disassembler.h"
#include "emitter.h"

#endif

#ifdef DEBUG_PRINT_TOKENS
#include "token_printer.h"
#endif


Parser parser;

ParseRule rules[];

int innermostLoopStart = -1;
int innermostLoopDepth = 0;

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

    bool canAssign = precedence <= PREC_ASSIGNMENT;
    Expr* expr = prefixRule(canAssign);

    while (precedence <= getRule(parser.current.type)->precedence)
    {
        advance();
        ParseInfixFn infixRule = getRule(parser.previous.type)->infix;
        expr = (Expr*)infixRule(expr, canAssign);
    }

    if (canAssign && match(TOKEN_EQUAL))
    {
        error("Invalid assignment target.");
    }

    return expr;
}

static Expr* expression()
{
    return parsePrecedence(PREC_ASSIGNMENT);
}

// endregion

// region EMITTING BYTES (LEGACY)

static void emitByte(uint8_t byte)
{

}

static void emitBytes(uint8_t byte1, uint8_t byte2)
{

}

static void emitReturn()
{

}

// endregion

// region EXPRESSIONS

static Expr* binary(Expr* previous, __attribute__((unused)) bool canAssign)
{
    TokenType operatorType = parser.previous.type;
    ParseRule* rule = getRule(operatorType);
    // We want to use a higher level because binary is left associative
    Expr* right = parsePrecedence((Precedence)(rule->precedence + 1));

    return (Expr*)newBinaryExpr(previous, operatorType, right, parser.line);
}

static Expr* ternary(Expr* previous, __attribute__((unused)) bool canAssign)
{
    Expr* thenBranch = parsePrecedence(PREC_TERNARY);
    consume(TOKEN_COLON, "Expect ':' after first ternary branch.");
    Expr* elseBranch = parsePrecedence(PREC_ASSIGNMENT);
    return (Expr*)newTernaryExpr(previous, thenBranch, elseBranch, parser.line);
}

static Expr* literal(__attribute__((unused)) bool canAssign)
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

static Expr* number(__attribute__((unused)) bool canAssign)
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

static Expr* string(__attribute__((unused)) bool canAssign)
{
    // Math is for trimming ""
    uint32_t length = strlen(parser.previous.start);
    char str[length];

    strcpy( str, parser.previous.start + 1 );
    str[parser.previous.length - 2] = 0;

    escapeSequences(str, str);

    return (Expr*)newLiteralExpr(OBJ_VAL(copyString(str, parser.previous.length - 2)), parser.line);
}

static Expr* interpolatedString(__attribute__((unused)) bool canAssign)
{

}

static Expr* unary(__attribute__((unused)) bool canAssign)
{
    TokenType operatorType = parser.previous.type;

    Expr* expr = parsePrecedence(PREC_UNARY);

    return (Expr*) newUnaryExpr(expr, operatorType, parser.line);
}

static Expr* grouping(__attribute__((unused)) bool canAssign)
{
    Expr* expr = expression();
    consume(TOKEN_RIGHT_PAREN, "Expect ')' after expression.");
    return expr;
}

static Node* argumentList()
{
    Node* root = NULL;

    if (!check(TOKEN_RIGHT_PAREN))
    {
        do
        {

            Expr* arg = expression();
            listAdd(&root, NODE_EXPRESSION_VALUE(arg));

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

static Expr* call(Expr* previous, bool canAssign)
{
    ObjString* name = parseVariableName("Expect function name in call expression.");
    Node* args = argumentList();
    return (Expr*) newCallExpr(name, args, parser.line);
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
    while (!check(TOKEN_RIGHT_BRACE) && !check(TOKEN_EOF))
    {
        listAdd(statements, NODE_STATEMENT_VALUE(declaration()));
    }

    consume(TOKEN_RIGHT_BRACE, "Expect '}' after block.");
    return (Stmt*)newBlockStmt(statements, parser.line);
}

static Expr * or(Expr *canAssign, bool b)
{
//    int endJump = emitJump(OP_JUMP_IF_TRUE);
//
//    emitByte(OP_POP);
//    parsePrecedence(PREC_OR);
//
//    patchJump(endJump);
}

static Expr* and(Expr *canAssign, bool b)
{
//    int endJump = emitJump(OP_JUMP_IF_FALSE);
//
//    emitByte(OP_POP);
//    parsePrecedence(PREC_AND);
//
//    patchJump(endJump);
}

static void emitLoop(int loopStart)
{
//    emitByte(OP_LOOP);
//
//    int offset = currentChunk()->count - loopStart + 2;
//    if (offset > UINT16_MAX) error("Loop body too large.");
//
//    emitByte((offset >> 8) & 0xff);
//    emitByte(offset & 0xff);
}

static Stmt* forStatement()
{
//    beginScope();
//    consume(TOKEN_LEFT_PAREN, "Expect '(' after 'for'.");
//
//    // Initializer clause
//    if (match(TOKEN_SEMICOLON))
//    {
//        // No initializer.
//    } else if (match(TOKEN_VAR))
//    {
//        varDeclaration();
//    } else
//    {
//        expressionStatement();
//    }
//
//    int surroundingLoopStart = innermostLoopStart;
//    int surroundingLoopScopeDepth = innermostLoopDepth;
//    innermostLoopStart = currentChunk()->count;
//    innermostLoopDepth = current->scopeDepth;
//
//    // Condition clause
//    int exitJump = -1;
//    if (!match(TOKEN_SEMICOLON))
//    {
//        expression();
//        consume(TOKEN_SEMICOLON, "Expect ';' after loop condition.");
//
//        // Jump out of the loop if the condition is false.
//        exitJump = emitJump(OP_JUMP_IF_FALSE);
//        emitByte(OP_POP); // Condition.
//    }
//
//
//    // Increment clause
//    if (!match(TOKEN_RIGHT_PAREN))
//    {
//        int bodyJump = emitJump(OP_JUMP);
//        int incrementStart = currentChunk()->count;
//        expression();
//        emitByte(OP_POP);
//        consume(TOKEN_RIGHT_PAREN, "Expect ')' after for clauses.");
//
//        emitLoop(innermostLoopStart);
//        innermostLoopStart = incrementStart;
//        patchJump(bodyJump);
//    }
//
//    statement();
//    emitLoop(innermostLoopStart);
//
//    if (exitJump != -1)
//    {
//        patchJump(exitJump);
//        emitByte(OP_POP); // Condition.
//    }
//
//    innermostLoopStart = surroundingLoopStart;
//    innermostLoopDepth = surroundingLoopScopeDepth;
//
//    endScope();
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
//    innermostLoopDepth = current->scopeDepth;
//    innermostLoopStart = currentChunk()->count;
//
//    consume(TOKEN_LEFT_PAREN, "Expect '(' after 'while'.");
//    expression();
//    consume(TOKEN_RIGHT_PAREN, "Expect ')' after condition.");
//
//    int exitJump = emitJump(OP_JUMP_IF_FALSE);
//    emitByte(OP_POP);
//
//    statement();
//    emitLoop(innermostLoopStart);
//
//    patchJump(exitJump);
//    emitByte(OP_POP);
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
//    if (match(TOKEN_SEMICOLON))
//    {
//        emitReturn();
//    }
//    else
//    {
//        expression();
//        consume(TOKEN_SEMICOLON, "Expect ';' after return value.");
//        emitByte(OP_RETURN);
//    }
}

static Stmt* switchStatement()
{
//    consume(TOKEN_LEFT_PAREN, "Expect '(' after 'switch'.");
//    expression();
//    consume(TOKEN_RIGHT_PAREN, "Expect ')' after condition.");
//    consume(TOKEN_LEFT_BRACE, "Expect '{' after ')'.");
//
//    for(;;)
//    {
//        if(match(TOKEN_CASE))
//        {
//            expression();
//            emitByte(OP_SWITCH_EQUAL);
//
//            int thenJump = emitJump(OP_JUMP_IF_FALSE);
//
//            consume(TOKEN_COLON, "Expect ':' after expression.");
//
//            statement();
//
//            patchJump(thenJump);
//            emitByte(OP_POP);
//        }
//        else if (match(TOKEN_DEFAULT))
//        {
//            consume(TOKEN_COLON, "Expect ':' after 'default'.");
//
//            statement();
//        }
//        else
//        {
//            break;
//        }
//    }
//
//    emitByte(OP_POP);
//    consume(TOKEN_RIGHT_BRACE, "Expect '}' at the end of switch statement.");
}

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

static Expr* variable(__attribute__((unused)) bool canAssign)
{
    ObjString* name = copyString(parser.previous.start, strlen(parser.previous.start) - 1); // TODO

    if(!match(TOKEN_EQUAL))
    {
        return (Expr*)newVarExpr(name, parser.line);
    }
    else
    {
        Expr* value = expression();
        return (Expr*)newAssignExpr(name, value, parser.line);
    }
}

static Stmt* functionDeclaration()
{
    ObjString* name = parseVariableName("Expect function name.");

    consume(TOKEN_LEFT_PAREN, "Expect '(' after function name.");

    do
    {

    } while(!match(TOKEN_LEFT_PAREN))


    consume(TOKEN_LEFT_BRACE, "Expect '{' after ')' in function.");

    Stmt* body = block();

    return (Stmt*)newFunctionStmt(name, body, NULL, parser.line);
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
        [TOKEN_AND]           = {NULL,                 and,    PREC_AND},
        [TOKEN_CLASS]         = {NULL,                 NULL,   PREC_NONE},
        [TOKEN_ELSE]          = {NULL,                 NULL,   PREC_NONE},
        [TOKEN_QUESTION_MARK] = {NULL,                 ternary,PREC_TERNARY},
        [TOKEN_COLON]         = {NULL,                 NULL,   PREC_NONE},
        [TOKEN_FALSE]         = {literal,              NULL,   PREC_NONE},
        [TOKEN_FOR]           = {NULL,                 NULL,   PREC_NONE},
        [TOKEN_FUNCTION]      = {NULL,                 NULL,   PREC_NONE},
        [TOKEN_IF]            = {NULL,                 NULL,   PREC_NONE},
        [TOKEN_NULL]          = {literal,              NULL,   PREC_NONE},
        [TOKEN_OR]            = {NULL,                 or,     PREC_OR},
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