#include <bits/stdint-uintn.h>
#include <stdio.h>

#include "emitter.h"
#include "chunk.h"
#include "list.h"
#include "disassembler.h"
#include "memory.h"
#include "garbage_collector.h"
#include "array.h"

Emitter* emitter = NULL;

DECLARE_ARRAY(Continues, uint)
DEFINE_ARRAY_FUNCTIONS(Continues, continues, uint);

// region ERROR

static void error(const char* message)
{
    printf("%s", message);
}

// endregion

// region EMITTING BYTES

static Chunk* currentChunk()
{
    return &emitter->current.function->chunk;
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

static uint emitJump(uint8_t instruction, uint16_t line)
{
    emitByte(instruction, line);

    // Emit placeholder values we'll replace after compiling the body
    emitByte(0xff, line);
    emitByte(0xff, line);

    return currentChunk()->codeCount - 2;
}

static void patchJump(uint offset)
{
    // -2 to adjust for the bytecode for the jump offset itself
    uint jump = currentChunk()->codeCount - offset - 2;

    if (jump > UINT16_MAX)
    {
        error("Too much code to jump over.");
    }

    currentChunk()->code[offset] = (jump >> 8) & 0xff;
    currentChunk()->code[offset + 1] = jump & 0xff;
}

static void emitLoop(uint loopStart, uint16_t line)
{
    emitByte(OP_LOOP, line);

    uint offset = currentChunk()->codeCount - loopStart + 2;
    if (offset > UINT16_MAX) error("Loop body too large.");

    emitByte((offset >> 8) & 0xff, line);
    emitByte(offset & 0xff, line);
}

static void emitReturn(uint16_t line)
{
    emitByte(OP_RETURN, line);
}

// endregion
static void initCompiler(Compiler* compiler, ObjString* functionName, uint16_t functionArity, FunctionType type);
static ObjFunction* endCompiler(uint16_t line);

static void compileExpression(Expr* expression)
{
    if(expression == NULL)
    {
        return;
    }

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

            emitConstant(OBJ_VAL(expr->name), line);
            emitByte(OP_GET_VARIABLE, line);

            break;
        }

        case ASSIGN_EXPRESSION:
        {
            AssignExpr* expr = (AssignExpr*)expression;

            emitConstant(OBJ_VAL(expr->name), line);
            compileExpression(expr->value);

            emitByte(OP_SET_VARIABLE, line);

            break;
        }

        case GROUPED_EXPRESSION:
        {
            GroupedExpr* expr = (GroupedExpr*)expression;

            compileExpression(expr->in);
            break;
        }

        case CALL_EXPRESSION:
        {
            CallExpr* expr = (CallExpr*)expression;

            Node* node = expr->args;

            while(node != NULL)
            {
                compileExpression(node->value.as.expression);
                node = node->next;
            }

            emitConstant(NUMBER_VAL(expr->argCount), line);
            emitConstant(OBJ_VAL(expr->callee), line);
            emitByte(OP_CALL, line);

            break;
        }

        case LOGICAL_EXPRESSION:
        {
            LogicalExpr* expr = (LogicalExpr*)expression;

            switch (expr->operator)
            {
                case TOKEN_AND:
                {
                    compileExpression(expr->left);
                    uint endJump = emitJump(OP_JUMP_IF_FALSE, line);

                    emitByte(OP_POP, line);
                    compileExpression(expr->right);

                    patchJump(endJump);
                    break;
                }

                case TOKEN_OR:
                {
                    compileExpression(expr->left);
                    uint endJump = emitJump(OP_JUMP_IF_TRUE, line);

                    emitByte(OP_POP, line);
                    compileExpression(expr->right);

                    patchJump(endJump);
                    break;
                }

                default:
                    printf("Reached unreachable");
            }

            break;
        }

    }
}

static void compileExpressionStatement(ExpressionStmt* statement)
{
    Expr* expr = statement->expr;
    compileExpression(statement->expr);

    if(expr->pop)
    {
        emitByte(OP_POP, 0);
    }
}

static void compileVariable(ObjString* name, Expr* initializer, uint16_t line)
{
    emitConstant(OBJ_VAL(name), line);

    if(initializer == NULL)
    {
        emitConstant(NULL_VAL, line);
    }
    else
    {
        compileExpression(initializer);
    }

    emitByte(OP_DEFINE_VARIABLE, line);
}

static uint16_t compileStatement(Stmt* statement)
{
    if(statement == NULL)
    {
        return 0;
    }

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

            emitByte(OP_SCOPE_START, line);

            int length = listGetLength(toExecute);

            for(int i = 0; i <= length - 1; i++)
            {
                compileStatement(listGet(toExecute, i).as.statement);
            }

            emitByte(OP_SCOPE_END, line);

            break;
        }

        case IF_STATEMENT:
        {
            IfStmt* stmt = (IfStmt*) statement;

            compileExpression(stmt->condition);

            uint thenJump = emitJump(OP_JUMP_IF_FALSE, line);
            emitByte(OP_POP, line);

            compileStatement(stmt->thenBranch);

            uint elseJump = emitJump(OP_JUMP, line);
            patchJump(thenJump);
            emitByte(OP_POP, line);

            compileStatement(stmt->elseBranch);

            patchJump(elseJump);

            break;
        }

        case VARIABLE_STATEMENT:
        {
            VariableStmt* stmt = (VariableStmt*) statement;

            compileVariable(stmt->name, stmt->initializer, line);

            break;
        }

        case WHILE_STATEMENT:
        {
            WhileStmt* stmt = (WhileStmt*) statement;

            uint loopStart = currentChunk()->codeCount;

            compileExpression(stmt->condition);
            uint exitJump = emitJump(OP_JUMP_IF_FALSE, line);
            emitByte(OP_POP, line);

            compileStatement(stmt->body);
            emitLoop(loopStart, line);

            patchJump(exitJump);
            emitByte(OP_POP, line);
            break;
        }

        case FOR_STATEMENT:
        {
            ForStmt* stmt = (ForStmt*) statement;

            emitByte(OP_SCOPE_START, line);

            // Declaration/Initializer
            compileStatement(stmt->declaration);

            // Start the loop before condition
            uint loopStart = currentChunk()->codeCount;
            int exitJump = -1;

            // Condition
            if(stmt->condition == NULL)
            {
                emitConstant(BOOL_VAL(true), line);
            }
            else
            {
                compileExpression(stmt->condition);
            }

            // Jump out of the loop if the condition is false.
            exitJump = emitJump(OP_JUMP_IF_FALSE, line);
            emitByte(OP_POP, line); // Pop Condition

            compileStatement(stmt->body);

            compileExpression(stmt->increment);

            emitLoop(loopStart, line);

            emitByte(OP_SCOPE_END, line);

            // Jump out of the loop
            if (exitJump != -1)
            {
                patchJump(exitJump);
                emitByte(OP_SCOPE_END, line);
                emitByte(OP_POP, line);
            }

            break;
        }

        case FUNCTION_STATEMENT:
        {
            FunctionStmt* stmt = (FunctionStmt*) statement;

            Compiler compiler;
            initCompiler((&compiler), stmt->name, stmt->paramCount, TYPE_FUNCTION);

            emitByte(OP_SCOPE_START, line);

            // Params
            ObjString** params = stmt->params;
            int paramCount = stmt->paramCount - 1;

            while (paramCount >= 0)
            {
                emitConstant(OBJ_VAL(params[paramCount]), line);
                emitByte(OP_DEFINE_ARGUMENT, line);
                paramCount--;
            }

            // Body
            Node* body = stmt->body;

            while (body != NULL)
            {
                compileStatement(AS_STATEMENT(body));
                body = body->next;
            }

            emitByte(OP_SCOPE_END, line);

            ObjFunction* function = endCompiler(line);

            // Function definition data
            emitBytes(OP_CONSTANT, makeConstant(OBJ_VAL(function)), line);
            emitByte(OP_DEFINE_FUNCTION, line);

            break;
        }

        case RETURN_STATEMENT:
        {
            if (emitter->current.type == TYPE_SCRIPT)
            {
                error("Can't return from top-level code.");
            }

            ReturnStmt* stmt = (ReturnStmt*) statement;

            compileExpression(stmt->value);
            emitByte(OP_RETURN, line);

            break;
        }

        case SWITCH_STATEMENT:
            break;
        case CONTINUE_STATEMENT:
            break;
        case BREAK_STATEMENT:
            break;
    }

    return line;
}

// region MAIN

static void initCompiler(Compiler* compiler, ObjString* functionName, uint16_t functionArity, FunctionType type)
{
    compiler->enclosing = (struct Compiler*) &emitter->current;
    compiler->function = NULL;
    compiler->type = type;

    emitter->current = *compiler;

    compiler->function = newFunction();
    compiler->function->arity = functionArity;

    if(functionName != NULL)
    {
        emitter->current.function->name = copyString(functionName->chars,
                                             functionName->length);
    }
}

static ObjFunction* endCompiler(uint16_t line)
{
    emitReturn(line);
    ObjFunction* function = emitter->current.function;

    #ifdef DEBUG_PRINT_BYTECODE
    if (!emitter->hadError)
    {
        disassembleChunk(currentChunk(), function->name != NULL
                                         ? function->name->chars : "<script>");
    }
    #endif

    emitter->current = *((Compiler*) emitter->current.enclosing);
    return function;
}

void initEmitter()
{
    emitter = reallocate(emitter, 0, sizeof(Emitter));

//    _emitter->current = NULL;
//    _emitter->current = reallocate( _emitter->current, 0, sizeof(Compiler));
//
//    _emitter->current->enclosing = NULL;
//    _emitter->current->enclosing = reallocate( _emitter->current->enclosing, 0, sizeof(Compiler));

    initCompiler(&(emitter->current), NULL, 0, TYPE_SCRIPT);
    initContinues(emitter->continues);
    emitter->hadError = false;
}

ObjFunction* emit(Node* statements)
{
    initEmitter();

    Node* stmt = statements;
    Node* root = stmt;

    uint16_t lastLine = 0;
    while (stmt != NULL)
    {
        lastLine = compileStatement(AS_STATEMENT(stmt));
        stmt = stmt->next;
    }

    freeList(root);

    ObjFunction* function = endCompiler(lastLine);
    return emitter->hadError ? NULL : function;
}

void markCompilerRoots()
{
    Compiler* compiler = &emitter->current;

    while (compiler != NULL)
    {
        markObject((Obj*)compiler->function);
        compiler = (Compiler *) compiler->enclosing;
    }
}
