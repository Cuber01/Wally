#include <stdlib.h>

#include "compiler.h"
#include "scanner.h"
#include "chunk.h"
#include "object.h"

#pragma clang diagnostic push
#pragma ide diagnostic ignored "UnusedParameter"
#ifdef DEBUG_PRINT_BYTECODE
#include "disassembler.h"
#endif

#ifdef DEBUG_PRINT_TOKENS
#include "token_printer.h"
#endif


Parser parser;
Compiler* current = NULL;

Chunk* compilingChunk;
ParseRule rules[];

// ---------- ERROR ----------

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

// ---------- UTIL ----------

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
            case TOKEN_FUN:
            case TOKEN_VAR:
            case TOKEN_FOR:
            case TOKEN_IF:
            case TOKEN_WHILE:
            case TOKEN_PRINT:
            case TOKEN_RETURN:
                return;

            default: ; // Nothing.
        }

        advance();
    }
}

static Chunk* currentChunk()
{
    return compilingChunk;
}

// ---------- PRECEDENCE ----------

static ParseRule* getRule(TokenType type)
{
    return &rules[type];
}

static void parsePrecedence(Precedence precedence)
{
    advance();

    ParseFn prefixRule = getRule(parser.previous.type)->prefix;
    if (prefixRule == NULL)
    {
        error("Expect expression.");
        return;
    }

    bool canAssign = precedence <= PREC_ASSIGNMENT;
    prefixRule(canAssign);

    while (precedence <= getRule(parser.current.type)->precedence)
    {
        advance();
        ParseFn infixRule = getRule(parser.previous.type)->infix;
        infixRule(canAssign);
    }

    if (canAssign && match(TOKEN_EQUAL))
    {
        error("Invalid assignment target.");
    }
}

static void expression()
{
    parsePrecedence(PREC_ASSIGNMENT);
}

// ---------- EMITTING BYTES ----------

static void emitByte(uint8_t byte)
{
    writeChunk(currentChunk(), byte, parser.previous.line);
}

static void emitBytes(uint8_t byte1, uint8_t byte2)
{
    emitByte(byte1);
    emitByte(byte2);
}

static uint8_t makeConstant(Value value)
{
    int constant = addConstant(currentChunk(), value);
    if (constant > UINT8_MAX)
    {
        error("Too many constants in one chunk.");
        return 0;
    }

    return (uint8_t)constant;
}

static void emitConstant(Value value)
{
    emitBytes(OP_CONSTANT, makeConstant(value));
}


static void emitReturn()
{
    emitByte(OP_RETURN);
}

// ---------- EXPRESSIONS ----------

static void binary(bool canAssign)
{
    TokenType operatorType = parser.previous.type;
    ParseRule* rule = getRule(operatorType);
    // We want to use a higher level because binary is left associative
    parsePrecedence((Precedence)(rule->precedence + 1));

    switch (operatorType)
    {
        case TOKEN_BANG_EQUAL:    emitByte(OP_NOT_EQUAL);     break;
        case TOKEN_EQUAL_EQUAL:   emitByte(OP_EQUAL);         break;
        case TOKEN_GREATER:       emitByte(OP_GREATER);       break;
        case TOKEN_GREATER_EQUAL: emitByte(OP_GREATER_EQUAL); break;
        case TOKEN_LESS:          emitByte(OP_LESS);          break;
        case TOKEN_LESS_EQUAL:    emitByte(OP_LESS_EQUAL);    break;

        case TOKEN_PLUS:          emitByte(OP_ADD); break;
        case TOKEN_MINUS:         emitByte(OP_SUBTRACT); break;
        case TOKEN_STAR:          emitByte(OP_MULTIPLY); break;
        case TOKEN_SLASH:         emitByte(OP_DIVIDE); break;
        default: return; // Unreachable.
    }
}

static void literal(bool canAssign)
{
    switch (parser.previous.type)
    {
        case TOKEN_FALSE: emitByte(OP_FALSE); break;
        case TOKEN_NIL:   emitByte(OP_NIL);   break;
        case TOKEN_TRUE:  emitByte(OP_TRUE);  break;
        default: return; // Unreachable.
    }
}

static void number(bool canAssign)
{
    double value = strtod(parser.previous.start, NULL);
    emitConstant(NUMBER_VAL(value));
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
        }

        source++;

    }

    *destination = 0;

}

static void string(bool canAssign)
{
    // Math is for trimming ""
    uint32_t length = strlen(parser.previous.start);
    char str[length];

    strcpy( str, parser.previous.start + 1 );
    str[parser.previous.length - 2] = 0;

    escapeSequences(str, str);

    emitConstant(OBJ_VAL(copyString(str, parser.previous.length - 2)));
}

static void interpolatedString(bool canAssign)
{

}

static void unary(bool canAssign)
{
    TokenType operatorType = parser.previous.type;

    // Compile the operand.
    parsePrecedence(PREC_UNARY);

    // Emit the operator instruction.
    switch (operatorType)
    {
        case TOKEN_BANG:  emitByte(OP_NOT);    break;
        case TOKEN_MINUS: emitByte(OP_NEGATE); break;
        default: return; // Unreachable.
    }
}

static void grouping(bool canAssign)
{
    expression();
    consume(TOKEN_RIGHT_PAREN, "Expect ')' after expression.");
}

// ---------- STATEMENTS ------------
static void declaration();
static void statement();
static void varDeclaration();

static void expressionStatement()
{
    expression();
    consume(TOKEN_SEMICOLON, "Expect ';' after expression.");
    emitByte(OP_POP);
}

static void printStatement()
{
    expression();
    consume(TOKEN_SEMICOLON, "Expect ';' after value.");
    emitByte(OP_PRINT);
}

static void block()
{
    while (!check(TOKEN_RIGHT_BRACE) && !check(TOKEN_EOF))
    {
        declaration();
    }

    consume(TOKEN_RIGHT_BRACE, "Expect '}' after block.");
}

static void endScope()
{
    current->scopeDepth--;

    // Remove all locals from the scope we left
    while (current->localCount > 0 &&
           current->locals[current->localCount - 1].depth >
           current->scopeDepth)
    {
        emitByte(OP_POP);
        current->localCount--;
    }
}

static void beginScope()
{
    current->scopeDepth++;
}

static int emitJump(uint8_t instruction)
{
    emitByte(instruction);

    // Emit placeholder values we'll replace after compiling the body
    emitByte(0xff);
    emitByte(0xff);

    return currentChunk()->count - 2;
}

static void patchJump(int offset)
{
    // -2 to adjust for the bytecode for the jump offset itself
    int jump = currentChunk()->count - offset - 2;

    if (jump > UINT16_MAX)
    {
        error("Too much code to jump over.");
    }

    currentChunk()->code[offset] = (jump >> 8) & 0xff;
    currentChunk()->code[offset + 1] = jump & 0xff;
}

static void or(bool canAssign)
{
    int endJump = emitJump(OP_JUMP_IF_TRUE);

    emitByte(OP_POP);
    parsePrecedence(PREC_OR);

    patchJump(endJump);
}

static void and(bool canAssign)
{
    int endJump = emitJump(OP_JUMP_IF_FALSE);

    emitByte(OP_POP);
    parsePrecedence(PREC_AND);

    patchJump(endJump);
}

static void emitLoop(int loopStart)
{
    emitByte(OP_LOOP);

    int offset = currentChunk()->count - loopStart + 2;
    if (offset > UINT16_MAX) error("Loop body too large.");

    emitByte((offset >> 8) & 0xff);
    emitByte(offset & 0xff);
}

static void forStatement()
{
    beginScope();
    consume(TOKEN_LEFT_PAREN, "Expect '(' after 'for'.");

    // Initializer clause
    if (match(TOKEN_SEMICOLON))
    {
        // No initializer.
    } else if (match(TOKEN_VAR))
    {
        varDeclaration();
    } else {
        expressionStatement();
    }

    int loopStart = currentChunk()->count;

    // Condition clause
    int exitJump = -1;
    if (!match(TOKEN_SEMICOLON))
    {
        expression();
        consume(TOKEN_SEMICOLON, "Expect ';' after loop condition.");

        // Jump out of the loop if the condition is false.
        exitJump = emitJump(OP_JUMP_IF_FALSE);
        emitByte(OP_POP); // Condition.
    }


    // Increment clause
    if (!match(TOKEN_RIGHT_PAREN))
    {
        int bodyJump = emitJump(OP_JUMP);
        int incrementStart = currentChunk()->count;
        expression();
        emitByte(OP_POP);
        consume(TOKEN_RIGHT_PAREN, "Expect ')' after for clauses.");

        emitLoop(loopStart);
        loopStart = incrementStart;
        patchJump(bodyJump);
    }

    statement();
    emitLoop(loopStart);

    if (exitJump != -1)
    {
        patchJump(exitJump);
        emitByte(OP_POP); // Condition.
    }

    endScope();
}

static void whileStatement()
{
    int loopStart = currentChunk()->count;
    consume(TOKEN_LEFT_PAREN, "Expect '(' after 'while'.");
    expression();
    consume(TOKEN_RIGHT_PAREN, "Expect ')' after condition.");

    int exitJump = emitJump(OP_JUMP_IF_FALSE);
    emitByte(OP_POP);

    statement();
    emitLoop(loopStart);

    patchJump(exitJump);
    emitByte(OP_POP);
}

static void ifStatement()
{
    consume(TOKEN_LEFT_PAREN, "Expect '(' after 'if'.");
    expression();
    consume(TOKEN_RIGHT_PAREN, "Expect ')' after condition.");

    int thenJump = emitJump(OP_JUMP_IF_FALSE);
    emitByte(OP_POP);
    statement();

    int elseJump = emitJump(OP_JUMP);

    patchJump(thenJump);
    emitByte(OP_POP);

    if (match(TOKEN_ELSE)) statement();
    patchJump(elseJump);
}

static void statement()
{
    if (match(TOKEN_PRINT))
    {
        printStatement();
    }
    else if (match(TOKEN_LEFT_BRACE))
    {
        beginScope();
        block();
        endScope();
    }
    else if (match(TOKEN_IF))
    {
        ifStatement();
    }
    else if (match(TOKEN_WHILE))
    {
        whileStatement();
    }
    else if (match(TOKEN_FOR))
    {
        forStatement();
    }
    else
    {
        expressionStatement();
    }
}

static uint8_t identifierConstant(Token* name)
{
    return makeConstant(
            OBJ_VAL(copyString(name->start,name->length)));
}

static int resolveLocal(Compiler* compiler, Token* name)
{

    for (int i = compiler->localCount - 1; i >= 0; i--)
    {
        Local* local = &compiler->locals[i];

        if (identifiersEqual(name, &local->name))
        {
            return i;
        }
    }

    return -1;
}

static void namedVariable(Token name, bool canAssign)
{
    uint8_t getOp, setOp;
    int arg = resolveLocal(current, &name);

    if (arg != -1)
    {
        getOp = OP_GET_LOCAL;
        setOp = OP_SET_LOCAL;
    }
    else
    {
        arg = identifierConstant(&name);
        getOp = OP_GET_GLOBAL;
        setOp = OP_SET_GLOBAL;
    }

    if (canAssign && match(TOKEN_EQUAL))
    {
        expression();
        emitBytes(setOp, (uint8_t)arg);
    }
    else
    {
        emitBytes(getOp, (uint8_t)arg);
    }
}

static void variable(bool canAssign)
{
    namedVariable(parser.previous, canAssign);
}

static void addLocal(Token name)
{
    if (current->localCount == UINT8_COUNT)
    {
        error("Too many local variables in function.");
        return;
    }

    Local* local = &current->locals[current->localCount++];
    local->name = name;
    local->depth = -1;
}

static void markInitialized()
{
    current->locals[current->localCount - 1].depth = current->scopeDepth;
}

// Define = variable is available for use
static void defineVariable(uint8_t global)
{
    // If it's a local variable, don't do anything
    if (current->scopeDepth > 0)
    {
        markInitialized();
        return;
    }

    emitBytes(OP_DEFINE_GLOBAL, global);
}

// Declare = variable is added to the scope
static void declareVariable()
{
    // If it's a global variable, don't do anything
    if (current->scopeDepth == 0) return;

    Token* name = &parser.previous;

    // Check if there's already an identifier with the same name in current scope
    for (int i = current->localCount - 1; i >= 0; i--)
    {
        Local* local = &current->locals[i];
        if (local->depth != -1 && local->depth < current->scopeDepth)
        {
            break;
        }

        if (identifiersEqual(name, &local->name))
        {
            error("Already a variable with this name in this scope.");
        }
    }


    addLocal(*name);
}

static uint8_t parseVariable(const char* errorMessage)
{
    consume(TOKEN_IDENTIFIER, errorMessage);

    declareVariable();
    if (current->scopeDepth > 0) return 0;

    return identifierConstant(&parser.previous);
}

static void varDeclaration()
{
    // Get name
    uint8_t global = parseVariable("Expect variable name.");

    // Get initializer
    if (match(TOKEN_EQUAL))
    {
        expression();
    }
    else
    {
        // Else default to nil
        emitByte(OP_NIL);
    }

    consume(TOKEN_SEMICOLON, "Expect ';' after variable declaration.");

    defineVariable(global);
}

static void declaration()
{
    if (match(TOKEN_VAR))
    {
        varDeclaration();
    }
    else
    {
        statement();
    }

    if (parser.panicMode) synchronize();
}

// ---------- MAIN ----------

static void initCompiler(Compiler* compiler)
{
    compiler->localCount = 0;
    compiler->scopeDepth = 0;
    current = compiler;
}

static void endCompiler()
{
    emitReturn();

    #ifdef DEBUG_PRINT_BYTECODE
    if (!parser.hadError)
    {
        disassembleChunk(currentChunk(), "code");
    }
    #endif
}

#ifdef DEBUG_PRINT_TOKENS
void printTokens();
#endif
bool compile(const char* source, Chunk* chunk)
{
    initScanner(source);

    Compiler compiler;
    initCompiler(&compiler);

    compilingChunk = chunk;
    parser.hadError = false;
    parser.panicMode = false;

    #ifdef DEBUG_PRINT_TOKENS
    printTokens();
    return false;
    #endif

    advance();

    while (!match(TOKEN_EOF))
    {
        declaration();
    }

    endCompiler();
    consume(TOKEN_EOF, "Expect end of expression.");

    return !parser.hadError;
}

ParseRule rules[] =
{
        // Prefix vs Infix
        // Prefix examples: -1, (1, 1, "str"
        // In prefix the expression token is on the left of the rest of the expression or is the expression itself (1, "str")
        //
        // Infix examples: 1 + 1, 2 * 2,
        // In prefix the expression token is in the middle of other two expressions

        //                           prefix    infix   precedence
        [TOKEN_LEFT_PAREN]    = {grouping,             NULL,   PREC_NONE},
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
        [TOKEN_FALSE]         = {literal,              NULL,   PREC_NONE},
        [TOKEN_FOR]           = {NULL,                 NULL,   PREC_NONE},
        [TOKEN_FUN]           = {NULL,                 NULL,   PREC_NONE},
        [TOKEN_IF]            = {NULL,                 NULL,   PREC_NONE},
        [TOKEN_NIL]           = {literal,              NULL,   PREC_NONE},
        [TOKEN_OR]            = {NULL,                 or,     PREC_OR},
        [TOKEN_PRINT]         = {NULL,                 NULL,   PREC_NONE},
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
