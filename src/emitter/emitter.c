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

static void emitByte(uint8_t byte, uint16_t line)
{
    writeChunk(currentChunk(), byte, line);
}

static void emitBytes(uint8_t byte1, uint8_t byte2, uint16_t line)
{
    emitByte(byte1, line);
    emitByte(byte2, line);
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

static void emitConstant(Value value, uint16_t line)
{
    emitBytes(OP_CONSTANT, makeConstant(value), line);
}

static void emitMultiplePop(int amount, uint16_t line)
{
    emitConstant(NUMBER_VAL(amount), line);
    emitByte(OP_POP_N, line);
}

static void emitReturn()
{
    emitByte(OP_RETURN, 999);
}

// endregion

static void compileExpression(Expr* expression)
{
    uint16_t line = expression->line;

    switch (expression->type)
    {
        case LITERAL_EXPRESSION:
        {
            LiteralExpr* expr = (LiteralExpr*)expression;

            switch (expr->value.type)
            {
                case VAL_BOOL:
                    emitConstant(BOOL_VAL(expr->value.as.boolean), line);
                    break;

                case VAL_NULL:
                    emitConstant(NULL_VAL, line);
                    break;

                case VAL_NUMBER:
                    emitConstant(NUMBER_VAL(expr->value.as.number), line);
                    break;

                case VAL_OBJ:
                    emitConstant(OBJ_VAL(expr->value.as.obj), line);
                    break;
            }

            break;
        }

        case BINARY_EXPRESSION:
        {
            BinaryExpr* expr = (BinaryExpr*)expression;

            compileExpression(expr->left);
            compileExpression(expr->right);

            switch (expr->operator)
            {
                case TOKEN_PLUS:
                    emitByte(OP_ADD, line);
                    break;
                case TOKEN_MINUS:
                    emitByte(OP_SUBTRACT, line);
                    break;
                case TOKEN_SLASH:
                    emitByte(OP_DIVIDE, line);
                    break;
                case TOKEN_STAR:
                    emitByte(OP_MULTIPLY, line);
                    break;
                case TOKEN_EQUAL_EQUAL:
                    emitByte(OP_EQUAL, line);
                    break;
                case TOKEN_BANG_EQUAL:
                    emitByte(OP_NOT_EQUAL, line);
                    break;
                case TOKEN_GREATER_EQUAL:
                    emitByte(OP_GREATER_EQUAL, line);
                    break;
                case TOKEN_LESS_EQUAL:
                    emitByte(OP_LESS_EQUAL, line);
                    break;
                case TOKEN_LESS:
                    emitByte(OP_LESS, line);
                    break;
                case TOKEN_GREATER:
                    emitByte(OP_GREATER, line);
                    break;

                default:
                    error("Unknown operator");
            }

            break;
        }

        case UNARY_EXPRESSION:
        {
            UnaryExpr* expr = (UnaryExpr*)expression;

            compileExpression(expr->target);

            switch (expr->operator)
            {
                case TOKEN_MINUS:
                    emitByte(OP_NEGATE, line);
                    break;

                case TOKEN_BANG:
                    emitByte(OP_NOT, line);
                    break;

                default:
                    error("Unrecognized operand in unary expression.");
            }
            break;
        }

        case TERNARY_EXPRESSION:
        {
            TernaryExpr* expr = (TernaryExpr*)expression;

            compileExpression(expr->condition);
            compileExpression(expr->thenBranch);
            compileExpression(expr->elseBranch);

            emitByte(OP_TERNARY, line);
            break;
        }

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

    }
}

static void compileExpressionStatement(ExpressionStmt* stmt)
{
    compileExpression(stmt->expr);
}

static void compileStatement(Stmt* stmt)
{
    switch (stmt->type)
    {
        case EXPRESSION_STATEMENT:
        {
            compileExpressionStatement((ExpressionStmt*) stmt);
            break;
        }

        case BLOCK_STATEMENT:
        {
            BlockStmt* statement = (BlockStmt*)stmt;
            Node* toExecute = statement->statements;

            int length = getLength(toExecute);

            for(int i = 0; i <= length - 1; i++)
            {
                compileStatement(listGet(toExecute, i).as.statement);
            }

            break;
        }

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


ObjFunction* emit(Node* statements)
{
    Compiler compiler;
    initCompiler(&compiler, TYPE_SCRIPT);

    Node* stmt = statements;

    while (stmt != NULL)
    {
        compileStatement(AS_STATEMENT(stmt));
        stmt = stmt->next;
    }

    ObjFunction* function = endCompiler();
    return hadError ? NULL : function;
}