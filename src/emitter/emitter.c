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
    return current->chunk;
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

static int emitJump(uint8_t instruction, uint16_t line)
{
    emitByte(instruction, line);

    // Emit placeholder values we'll replace after compiling the body
    emitByte(0xff, line);
    emitByte(0xff, line);

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
                    error("Unknown operator in binary expression.");
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
        {
            VarExpr* expr = (VarExpr*)expression;

            emitByte(OP_GET_GLOBAL, line);
            emitConstant(OBJ_VAL(expr->name), line);

            break;
        }

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

static void compileExpressionStatement(ExpressionStmt* statement)
{
    compileExpression(statement->expr);
}

static void compileStatement(Stmt* statement)
{
    uint16_t line = statement->line;

    switch (statement->type)
    {
        case EXPRESSION_STATEMENT:
        {
            compileExpressionStatement((ExpressionStmt*) statement);
            break;
        }

        case BLOCK_STATEMENT:
        {
            BlockStmt* stmt = (BlockStmt*)statement;
            Node* toExecute = stmt->statements;

            int length = getLength(toExecute);

            for(int i = 0; i <= length - 1; i++)
            {
                compileStatement(listGet(toExecute, i).as.statement);
            }

            break;
        }

        case IF_STATEMENT:
        {
            IfStmt* stmt = (IfStmt*) statement;

            compileExpression(stmt->condition);

            int thenJump = emitJump(OP_JUMP_IF_FALSE, line);
            emitByte(OP_POP, line);

            compileStatement(stmt->thenBranch);

            int elseJump = emitJump(OP_JUMP, line);
            patchJump(thenJump);
            emitByte(OP_POP, line);
            if(stmt->elseBranch != NULL) // todo if more statements will need it we can just check for null at the start of the function
            {
                compileStatement(stmt->elseBranch);
            }

            patchJump(elseJump);

            break;
        }

        case VARIABLE_STATEMENT:
        {
            VariableStmt* stmt = (VariableStmt*) statement;

            emitByte(OP_DEFINE_GLOBAL, line);
            emitConstant(OBJ_VAL(stmt->name), line);
            compileExpression(stmt->initializer);

            break;
        }

        case WHILE_STATEMENT:
            break;
        case SWITCH_STATEMENT:
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

static void initCompiler(Compiler* compiler)
{
    compiler->enclosing = (struct Compiler*) current;
    current = compiler;
    initChunk(current->chunk);
}

static Chunk* endCompiler()
{
    emitReturn();
    Chunk* chunk = current->chunk;

    #ifdef DEBUG_PRINT_BYTECODE
    if (!hadError)
    {
        disassembleChunk(currentChunk(), "<script>");
    }
    #endif

    current = (Compiler*) current->enclosing; // todo
    return chunk;
}


Chunk* emit(Node* statements)
{
    Compiler compiler;
    initCompiler(&compiler);

    Node* stmt = statements;

    while (stmt != NULL)
    {
        compileStatement(AS_STATEMENT(stmt));
        stmt = stmt->next;
    }

    Chunk* chunk = endCompiler();
    return hadError ? NULL : chunk;
}