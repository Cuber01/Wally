#include <bits/stdint-uintn.h>
#include <stdio.h>

#include "emitter.h"
#include "chunk.h"
#include "list.h"

// region ERROR

static void error(const char* message)
{
    printf("%s", message);
}

static Chunk* currentChunk()
{
    // todo
}

// endregion

// region EMITTING BYTES

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
    switch (stmt->expr->type)
    {
        case LITERAL_EXPRESSION:
        {
            printValue(((LiteralExpr*)stmt->expr)->value);
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

static void parseStatement(Stmt* stmt)
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

void emit(Node* statements)
{
    Node* stmt = statements;

    while (stmt != NULL)
    {
        parseStatement(AS_STATEMENT(stmt));
        stmt = stmt->next;
    }
}