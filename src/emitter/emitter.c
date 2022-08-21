#include <stdio.h>

#include "emitter.h"
#include "chunk.h"
#include "list.h"
#include "garbage_collector.h"
#include "array.h"

#ifdef DEBUG_PRINT_BYTECODE
#include "disassembler.h"
#endif

static uint16_t compileStatement(Stmt* statement);

// TODO make it not global
UInts* breaks;
UInts* continues;
uint loopDepth = 0;

Compiler* current = NULL;
bool hadError = false;

// region ERROR

static void error(const char* message, uint16_t line)
{
    fprintf(stderr, "[line %d] Emitter Error : %s\n", line, message);
    hadError = true;
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

static uint8_t makeConstant(Value value, uint16_t line)
{
    int constant = addConstant(currentChunk(), value);
    if (constant > UINT8_MAX)
    {
        error("Too many constants in one chunk.", line);
        return 0;
    }

    return (uint8_t)constant;
}

static void emitConstant(Value value, uint16_t line)
{
    emitBytes(OP_CONSTANT, makeConstant(value, line), line);
}

static uint emitJump(uint8_t instruction, uint16_t line)
{
    emitByte(instruction, line);

    // Emit placeholder values we'll replace after compiling the body
    emitByte(0xff, line);
    emitByte(0xff, line);

    return currentChunk()->codeCount - 2;
}

static void patchJump(uint offset, uint16_t line)
{
    // -2 to adjust for the bytecode for the jump offset itself
    uint jump = currentChunk()->codeCount - offset - 2;

    if (jump > UINT16_MAX)
    {
        error("Too much code to jump over.", line);
    }

    currentChunk()->code[offset] = (jump >> 8) & 0xff;
    currentChunk()->code[offset + 1] = jump & 0xff;
}

static void patchLoopJumps(UInts* jumps, uint16_t line)
{
    for (uint i = 0; i < jumps->count; i++)
    {
        patchJump(jumps->values[i], line);
    }

    freeUInts(jumps);
}

static void emitLoop(uint loopStart, uint16_t line)
{
    emitByte(OP_LOOP, line);

    uint offset = currentChunk()->codeCount - loopStart + 2;
    if (offset > UINT16_MAX) error("Loop body too large.", line);

    emitByte((offset >> 8) & 0xff, line);
    emitByte(offset & 0xff, line);
}

static void emitReturn(uint16_t line)
{
    emitByte(OP_RETURN, line);
}

// endregion
static void initCompiler(Compiler* compiler, ObjString* fooName, uint16_t fooArity, FunctionType type);
static ObjFunction* endCompiler(bool emitNull, uint16_t line);

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

            #ifdef NAN_BOXING

            Value value = expr->value;

            if (IS_BOOL(value))
            {
                emitConstant(BOOL_VAL(AS_BOOL(value)), line);
            }
            else if (IS_NULL(value))
            {
                emitByte(OP_NULL, line);
            }
            else if (IS_NUMBER(value))
            {
                emitConstant(NUMBER_VAL(AS_NUMBER(value)), line);
            }
            else if (IS_STRING(value))
            {
                emitConstant(OBJ_VAL(AS_STRING(value)), line);
            }

            #else

            switch (expr->value.type)
            {
                case VAL_BOOL:
                    emitConstant(BOOL_VAL(expr->value.as.boolean), line);
                    break;

                case VAL_NULL:
                    emitByte(OP_NULL, line);
                    break;

                case VAL_NUMBER:
                    emitConstant(NUMBER_VAL(expr->value.as.number), line);
                    break;

                case VAL_OBJ:
                    emitConstant(OBJ_VAL(expr->value.as.obj), line);
                    break;
            }

            #endif

            break;
        }

        case BINARY_EXPRESSION:
        {
            BinaryExpr* expr = (BinaryExpr*)expression;

            compileExpression(expr->left);
            compileExpression(expr->right);

            switch (expr->op)
            {
                case TOKEN_PLUS:
                    emitByte(OP_ADD, line);
                    break;
                case TOKEN_MINUS:
                case TOKEN_MINUS_E:
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
                    error("Unknown operator in binary expression.", line);
            }

            break;
        }

        case UNARY_EXPRESSION:
        {
            UnaryExpr* expr = (UnaryExpr*)expression;

            compileExpression(expr->target);

            switch (expr->op)
            {
                case TOKEN_MINUS:
                    emitByte(OP_NEGATE, line);
                    break;

                case TOKEN_BANG:
                    emitByte(OP_NOT, line);
                    break;

                default:
                    error("Unrecognized operand in unary expression.", line);
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

            emitBytes(OP_GET_VARIABLE, makeConstant(OBJ_VAL(expr->name), line), line);

            break;
        }

        case ASSIGN_EXPRESSION:
        {
            AssignExpr* expr = (AssignExpr*)expression;

            compileExpression(expr->value);

            emitBytes(OP_SET_VARIABLE, makeConstant(OBJ_VAL(expr->name), line), line);

            break;
        }

        case DOT_EXPRESSION:
        {
            DotExpr* expr = (DotExpr*)expression;

            compileExpression(expr->instance);

            if(expr->isCall) // Call to a method
            {
                Node* node = expr->args;

                while(node != NULL)
                {
                    compileExpression(node->value.as.expression);
                    node = node->next;
                }

                emitBytes(OP_INVOKE, makeConstant(OBJ_VAL(expr->fieldName), line), line);
                emitByte(expr->argCount, line);
            }
            else if (expr->value != NULL) // Setter
            {
                compileExpression(expr->value);
                emitBytes(OP_SET_PROPERTY, makeConstant(OBJ_VAL(expr->fieldName), line), line);
            }
            else // Getter
            {
                emitBytes(OP_GET_PROPERTY, makeConstant(OBJ_VAL(expr->fieldName), line), line);
            }

            break;
        }

        case BASE_EXPRESSION:
        {
            BaseExpr* expr = (BaseExpr*)expression;

            emitBytes(OP_GET_BASE, makeConstant(OBJ_VAL(expr->methodName), line), line);
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

            compileExpression(expr->callee);
            emitBytes(OP_CALL, expr->argCount, line);

            break;
        }

        case LOGICAL_EXPRESSION:
        {
            LogicalExpr* expr = (LogicalExpr*)expression;

            switch (expr->op)
            {
                case TOKEN_AND:
                {
                    compileExpression(expr->left);
                    uint endJump = emitJump(OP_JUMP_IF_FALSE, line);

                    emitByte(OP_POP, line);
                    compileExpression(expr->right);

                    patchJump(endJump, line);
                    break;
                }

                case TOKEN_OR:
                {
                    compileExpression(expr->left);
                    uint endJump = emitJump(OP_JUMP_IF_TRUE, line);

                    emitByte(OP_POP, line);
                    compileExpression(expr->right);

                    patchJump(endJump, line);
                    break;
                }

                default:
                    printf("Reached unreachable");
            }

            break;
        }

        case LIST_EXPRESSION:
        {
            ListExpr* expr = (ListExpr*)expression;

            Node* processed = expr->expressions;
            uint count = 0;

            while(processed != NULL)
            {
                compileExpression(AS_EXPRESSION(processed));
                count++;

                processed = processed->next;
            }

            emitBytes(OP_BUILD_LIST, makeConstant(NUMBER_VAL(count), line), line);

            break;
        }

        case SUBSCRIPT_EXPRESSION:
        {
            SubscriptExpr* expr = (SubscriptExpr*)expression;

            compileExpression(expr->list);

            // Get
            if(expr->value == NULL)
            {
                compileExpression(expr->index);
                emitByte(OP_LIST_GET, line);
            }
            // Set
            else
            {
                compileExpression(expr->index);
                compileExpression(expr->value);
                emitByte(OP_LIST_STORE, line);
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
    if(initializer == NULL)
    {
        emitByte(OP_NULL, line);
    }
    else
    {
        compileExpression(initializer);
    }

    emitBytes(OP_DEFINE_VARIABLE, makeConstant(OBJ_VAL(name), line), line);
}

static void compileFunction(FunctionStmt* stmt, bool isMethod, uint16_t line)
{
    Compiler compiler;

    FunctionType type = isMethod ? TYPE_METHOD : TYPE_FUNCTION;

    if (isMethod && stmt->name->length == 4 && memcmp(stmt->name->chars, "init", 4) == 0)
    {
        type = TYPE_INITIALIZER;
    }

    initCompiler(&compiler,
             stmt->name,
              stmt->paramCount,
                          type);

    // Params
    ObjString** params = stmt->params;
    int paramCount = stmt->paramCount - 1;

    while (paramCount >= 0)
    {
        emitBytes(OP_DEFINE_ARGUMENT, makeConstant(OBJ_VAL(params[paramCount]), line), line);
        paramCount--;
    }

    // Body
    Node* body = stmt->body;

    while (body != NULL)
    {
        compileStatement(AS_STATEMENT(body));
        body = body->next;
    }

    ObjFunction* function = endCompiler(true, line);

    // Function definition data
    emitConstant(OBJ_VAL(function), line);
    emitByte(isMethod ? OP_DEFINE_METHOD : OP_DEFINE_FUNCTION, line);
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
                compileStatement(listGet(toExecute, i, line).as.statement);
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
            patchJump(thenJump, line);
            emitByte(OP_POP, line);

            compileStatement(stmt->elseBranch);

            patchJump(elseJump, line);

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
            loopDepth++;

            compileExpression(stmt->condition);
            uint exitJump = emitJump(OP_JUMP_IF_FALSE, line);
            emitByte(OP_POP, line);

            compileStatement(stmt->body);

            patchLoopJumps(continues, line);
            emitLoop(loopStart, line);
            patchJump(exitJump, line);
            patchLoopJumps(breaks, line);

            emitByte(OP_POP, line);
            loopDepth--;
            break;
        }

        case FOR_STATEMENT:
        {
            ForStmt* stmt = (ForStmt*) statement;

            emitByte(OP_SCOPE_START, line);
            loopDepth++;

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
            patchLoopJumps(continues, line);

            compileExpression(stmt->increment);

            emitLoop(loopStart, line);

            patchLoopJumps(breaks, line);
            emitByte(OP_SCOPE_END, line);

            // Jump out of the loop
            if (exitJump != -1)
            {
                patchJump(exitJump, line);
                emitByte(OP_SCOPE_END, line);
                emitByte(OP_POP, line);
            }

            loopDepth--;
            break;
        }

        case FUNCTION_STATEMENT:
        {
            FunctionStmt* stmt = (FunctionStmt*) statement;

            compileFunction(stmt, false, line);
            break;
        }

        case CLASS_STATEMENT:
        {
            ClassStmt* stmt = (ClassStmt*) statement;

            emitConstant(OBJ_VAL(newClass(stmt->name)), line);

            for(uint i = 0; i < stmt->methods.count; i++)
            {
                compileFunction((FunctionStmt*)(stmt->methods.values[i]), true, line);
            }

            emitByte(OP_DEFINE_CLASS, line);

            if(stmt->parent != NULL)
            {
                VarExpr* parent = (VarExpr*)(stmt->parent);

                emitBytes(OP_GET_VARIABLE,
                          makeConstant(OBJ_VAL( parent->name ), line),
                                line);

                emitByte(OP_INHERIT, line);
            }

            emitByte(OP_POP, line);
            break;
        }

        case RETURN_STATEMENT:
        {
            if (current->function->type == TYPE_SCRIPT)
            {
                error("Can't return from top-level code.", line);
            }

            if (current->function->type == TYPE_INITIALIZER)
            {
                error("Can't return custom values from initializer. It always returns the instance of your class.", line);
            }

            ReturnStmt* stmt = (ReturnStmt*) statement;

            if(stmt->value == NULL)
            {
                emitByte(OP_NULL, line);
            }
            else
            {
                compileExpression(stmt->value);
            }

            emitByte(OP_RETURN, line);

            break;
        }

        case CONTINUE_STATEMENT:
            if (loopDepth == 0)
            {
                error("Can't 'continue' from top-level code.", line);
            }

            uintsWrite(continues, emitJump(OP_JUMP, statement->line));
            break;

        case BREAK_STATEMENT:
            if (loopDepth == 0)
            {
                error("Can't break from top-level code.", line);
            }

            uintsWrite(breaks, emitJump(OP_JUMP, statement->line));
            break;

        case SWITCH_STATEMENT:
            break;

    }

    return line;
}


// region MAIN

static void initEmitter()
{
    continues = initUInts(continues);
    breaks = initUInts(breaks);
}

static void initCompiler(Compiler* compiler, ObjString* fooName, uint16_t fooArity, FunctionType type)
{
    compiler->enclosing = (struct Compiler*) current;
    compiler->function = NULL;

    current = compiler;

    compiler->function = newFunction(fooName, fooArity, type);
//    compiler->function->arity = functionArity;
//
//    if(functionName != NULL)
//    {
//        current->function->name = copyString(functionName->chars,
//                                             functionName->length);
//    }
}

static ObjFunction* endCompiler(bool emitNull, uint16_t line)
{
    // We emit null if user didn't return anything else via the return statement
    if(current->function->type != TYPE_INITIALIZER && emitNull)
    {
        emitByte(OP_NULL, line);
    }

    emitReturn(line);
    ObjFunction* function = current->function;

    #ifdef DEBUG_PRINT_BYTECODE
    if (!hadError)
    {
        disassembleChunk(currentChunk(), function->name != NULL
                                         ? function->name->chars : "<script>");
    }
    #endif

    current = (Compiler*) current->enclosing;
    return function;
}

ObjFunction* emit(Node* statements)
{
    initEmitter();

    Compiler compiler;
    initCompiler(&compiler, NULL, 0, TYPE_SCRIPT);

    Node* stmt = statements;
    Node* root = stmt;

    uint16_t lastLine = 0;
    while (stmt != NULL)
    {
        lastLine = compileStatement(AS_STATEMENT(stmt));
        stmt = stmt->next;
    }

    freeList(root);

    ObjFunction* function = endCompiler(false, lastLine);
    return hadError ? NULL : function;
}

void markCompilerRoots()
{
    Compiler* compiler = current;

    while (compiler != NULL)
    {
        markObject((Obj*)compiler->function);
        compiler = (Compiler *) compiler->enclosing;
    }
}