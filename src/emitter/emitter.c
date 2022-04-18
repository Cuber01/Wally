#include <bits/stdint-uintn.h>
#include <stdio.h>

#include "emitter.h"
#include "chunk.h"
#include "list.h"
#include "disassembler.h"

Compiler* current = NULL;
bool hadError = false;

// region ERROR

static void error(const char* message)
{
    printf("%s", message);
}

// endregion

// region EMITTING BYTES

static Chunk* currentChunk()
{
    return &current->function->chunk;
}

static void emitByte(uint8_t byte)
{
    writeChunk(currentChunk(), byte, -1); // todo line
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

static void emitMultiplePop(int amount)
{
    emitConstant(NUMBER_VAL(amount));
    emitByte(OP_POP_N);
}

static void emitReturn()
{
    emitByte(OP_RETURN);
}

// endregion

static void expressionStatement(ExpressionStmt* stmt)
{
    #define EXPRESSION stmt->expr

    switch (EXPRESSION->type)
    {
        case LITERAL_EXPRESSION:
        {
            LiteralExpr* expr = (LiteralExpr*)EXPRESSION;

            switch (expr->value.type)
            {
                case VAL_BOOL:
                    emitConstant(BOOL_VAL(expr->value.as.boolean));
                    break;

                case VAL_NULL:
                    emitConstant(NULL_VAL);
                    break;

                case VAL_NUMBER:
                    emitConstant(NUMBER_VAL(expr->value.as.number));
                    break;

                case VAL_OBJ:
                    emitConstant(OBJ_VAL(expr->value.as.obj));
                    break;
            }
            
            break;
        }

        case BINARY_EXPRESSION:
            break;
        case UNARY_EXPRESSION:
            break;
        case VAR_EXPRESSION:
            break;
        case LOGICAL_EXPRESSION:
            break;
        case GROUPED_EXPRESSION:
            break;
        case ASSIGN_EXPRESSION:
            break;
        case CALL_EXPRESSION:
            break;
        case TERNARY_EXPRESSION:
            break;
        case OBJECT_EXPRESSION:
            break;
    }
}

static void compileStatement(Stmt* stmt)
{
    switch (stmt->type)
    {
        case EXPRESSION_STATEMENT:
        {
            expressionStatement((ExpressionStmt*) stmt);
            break;
        }

        case BLOCK_STATEMENT:
            break;
        case IF_STATEMENT:
            break;
        case WHILE_STATEMENT:
            break;
        case SWITCH_STATEMENT:
            break;
        case VARIABLE_STATEMENT:
            break;
        case CONTINUE_STATEMENT:
            break;
        case BREAK_STATEMENT:
            break;
        case FUNCTION_STATEMENT:
            break;
        case RETURN_STATEMENT:
            break;
    }
}

// region MAIN

static void initCompiler(Compiler* compiler, FunctionType type)
{
    compiler->enclosing = (struct Compiler*) current;
    compiler->function = NULL;
    compiler->type = type;

    compiler->localCount = 0;
    compiler->scopeDepth = 0;
    compiler->function = newFunction();
    current = compiler;

    Local* local = &current->locals[current->localCount++];
    local->depth = 0;
    local->name.start = "";
    local->name.length = 0;
}

static ObjFunction* endCompiler()
{
    emitReturn();
    ObjFunction* function = current->function;

    #ifdef DEBUG_PRINT_BYTECODE
    if (!hadError)
    {
        disassembleChunk(currentChunk(), function->name != NULL ? function->name->chars : "<script>");
    }
    #endif

    current = (Compiler*) current->enclosing;
    return function;
}


void emit(Node* statements)
{
    Compiler compiler;
    initCompiler(&compiler, TYPE_SCRIPT);

    Node* stmt = statements;

    while (stmt != NULL)
    {
        compileStatement(AS_STATEMENT(stmt));
        stmt = stmt->next;
    }

    endCompiler();
}