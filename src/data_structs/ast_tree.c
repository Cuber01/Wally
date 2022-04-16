#include <malloc.h>

#include "ast_tree.h"
#include "memory.h"

#define ALLOCATE_EXPRESSION(objectType, enumType) \
    newExpression(sizeof(objectType), enumType)

static Expr* newExpression(size_t size, ExprType type)
{
    Expr* object = malloc(size);

    object->type = type;
    object->line = 0; // TODO

    return object;
}

#define ALLOCATE_STATEMENT(objectType, enumType) \
    newStatement(sizeof(objectType), enumType)

static Stmt* newStatement(size_t size, StmtType type)
{
    Stmt* object = malloc(size);

    object->type = type;
    object->line = 0; // TODO

    return object;
}


void freeExpression(Expr* expr)
{
    switch (expr->type)
    {
        case LITERAL_EXPRESSION:
        {
            FREE(LiteralExpr, expr);
            break;
        }

        case BINARY_EXPRESSION:
        {
            BinaryExpr* expression = (BinaryExpr*) expr;

            freeExpression(expression->right);
            freeExpression(expression->left);

            FREE(BinaryExpr, expr);
            break;
        }

        case UNARY_EXPRESSION:
        {
            UnaryExpr* expression = (UnaryExpr*) expr;

            freeExpression(expression->target);

            FREE(UnaryExpr, expr);
            break;
        }

        case VAR_EXPRESSION:
        {
            FREE(VarExpr, expr);
            break;
        }

        case ASSIGN_EXPRESSION:
        {
            AssignExpr* expression = (AssignExpr*) expr;

            freeExpression(expression->value);

            FREE(AssignExpr, expr);
            break;
        }

        case CALL_EXPRESSION:
        {
            CallExpr* expression = (CallExpr*) expr;

            freeExpression(expression->callee);
            // TODO free arg list

            FREE(CallExpr, expr);
            break;
        }

        case OBJECT_EXPRESSION:
        {
            ObjectExpr *expression = (ObjectExpr *) expr;

            freeObject(&expression->value);

            FREE(ObjectExpr, expr);
            break;
        }

        case LOGICAL_EXPRESSION:
        {
            LogicalExpr* expression = (LogicalExpr*) expr;

            freeExpression(expression->right);
            freeExpression(expression->left);

            FREE(LogicalExpr, expr);
            break;
        }

        case GROUPED_EXPRESSION:
        {
            GroupedExpr* expression = (GroupedExpr*) expr;

            freeExpression(expression->in);

            FREE(GroupedExpr, expr);
            break;
        }

        case TERNARY_EXPRESSION:
        {
            TernaryExpr* expression = (TernaryExpr*) expr;

            freeExpression(expression->condition);
            freeExpression(expression->thenBranch);
            freeExpression(expression->elseBranch);

            FREE(TernaryExpr, expr);
            break;
        }
    }
}

void freeStatement(Stmt* stmt)
{
    switch (stmt->type)
    {

        case EXPRESSION_STATEMENT:
        {
            ExpressionStmt* statement = (ExpressionStmt*) stmt;

            freeExpression(statement->expr);

            FREE(ExpressionStmt, stmt);
            break;
        }

        case BLOCK_STATEMENT:
        {
            BlockStmt* statement = (BlockStmt*) stmt;

            // todo free all statements in block

            FREE(BlockStmt, stmt);
            break;
        }

        case VARIABLE_STATEMENT:
        {
            VariableStmt* statement = (VariableStmt*) stmt;

            freeExpression(statement->initializer);

            FREE(VariableStmt, stmt);
            break;
        }

        case IF_STATEMENT:
        {
            IfStmt* statement = (IfStmt*) stmt;

            freeExpression(statement->condition);
            freeStatement(statement->thenBranch);
            freeStatement(statement->elseBranch);

            FREE(IfStmt, stmt);
            break;
        }

        case WHILE_STATEMENT:
        {
            WhileStmt* statement = (WhileStmt*) stmt;

            freeExpression(statement->condition);
            freeStatement(statement->body);

            FREE(WhileStmt, stmt);
            break;
        }


        case CONTINUE_STATEMENT:
        {
            FREE(ContinueStmt, stmt);
            break;
        }

        case BREAK_STATEMENT:
        {
            FREE(BreakStmt, stmt);
            break;
        }

        case FUNCTION_STATEMENT:
        {
            FunctionStmt* statement = (FunctionStmt*) stmt;

            // todo free params
            freeStatement(statement->body);

            FREE(FunctionStmt, stmt);
            break;
        }

        case RETURN_STATEMENT:
        {
            FREE(ReturnStmt, stmt);
            break;
        }

        case SWITCH_STATEMENT:
        {
            SwitchStmt* statement = (SwitchStmt*) stmt;

            // todo free list of cases
            // todo free list of condition
            freeStatement(statement->defaultBranch);

            FREE(SwitchStmt, stmt);
            break;
        }

    }
}

// ------------ EXPRESSIONS ------------
ContinueStmt* newContinueStmt()
{
    ContinueStmt* stmt = (ContinueStmt*) ALLOCATE_STATEMENT(ContinueStmt, CONTINUE_STATEMENT);
    return stmt;
}

BreakStmt* newBreakStmt()
{
    BreakStmt* stmt = (BreakStmt*) ALLOCATE_STATEMENT(BreakStmt, BREAK_STATEMENT);
    return stmt;
}

ReturnStmt* newReturnStmt(Expr* value)
{
    ReturnStmt* stmt = (ReturnStmt*) ALLOCATE_STATEMENT(ReturnStmt, RETURN_STATEMENT);

    stmt->value = value;

    return stmt;
}

FunctionStmt* newFunctionStmt(const char* name, Stmt* body)
{
    FunctionStmt* stmt = (FunctionStmt*) ALLOCATE_STATEMENT(FunctionStmt, FUNCTION_STATEMENT);

    stmt->name = name;
    stmt->body = body;

    // TODO list of params

    return stmt;
}

VariableStmt* newVariableStmt(const char* name, Expr* initializer)
{
    VariableStmt* stmt = (VariableStmt*) ALLOCATE_STATEMENT(VariableStmt, VARIABLE_STATEMENT);

    stmt->name = name;
    stmt->initializer = initializer;

    return stmt;
}

SwitchStmt* newSwitchStmt(Stmt* defaultBranch)
{
    SwitchStmt* stmt = (SwitchStmt*) ALLOCATE_STATEMENT(SwitchStmt, SWITCH_STATEMENT);

    stmt->defaultBranch = defaultBranch;

    // TODO list of conditions
    // TODO list of bodies

    return stmt;
}

WhileStmt* newWhileStmt(Expr* condition, Stmt* body)
{
    WhileStmt* stmt = (WhileStmt*) ALLOCATE_STATEMENT(WhileStmt, WHILE_STATEMENT);

    stmt->condition = condition;
    stmt->body = body;

    return stmt;
}

IfStmt* newIfStmt(Expr* condition, Stmt* thenBranch, Stmt* elseBranch)
{
    IfStmt* stmt = (IfStmt*) ALLOCATE_STATEMENT(IfStmt, IF_STATEMENT);

    stmt->condition = condition;
    stmt->thenBranch = thenBranch;
    stmt->elseBranch = elseBranch;

    return stmt;
}

BlockStmt* newBlockStmt()
{
    BlockStmt* stmt = (BlockStmt*) ALLOCATE_STATEMENT(BlockStmt, BLOCK_STATEMENT);

    // TODO list of statements

    return stmt;
}

ExpressionStmt* newExpressionStmt(Expr* expr)
{
    ExpressionStmt* stmt = (ExpressionStmt*) ALLOCATE_STATEMENT(ExpressionStmt, EXPRESSION_STATEMENT);

    stmt->expr = expr;

    return stmt;
}

CallExpr* newCallExpr(Expr* callee)
{
    CallExpr* expr = (CallExpr*) ALLOCATE_EXPRESSION(CallExpr, CALL_EXPRESSION);

    expr->callee = callee;
    // TODO args

    return expr;
}

AssignExpr* newAssignExpr(const char* name, Expr* value)
{
    AssignExpr* expr = (AssignExpr*) ALLOCATE_EXPRESSION(AssignExpr, ASSIGN_EXPRESSION);

    expr->name = name;
    expr->value = value;

    return expr;
}

VarExpr* newVarExpr(const char* name)
{
    VarExpr* expr = (VarExpr*) ALLOCATE_EXPRESSION(VarExpr, VAR_EXPRESSION);

    expr->name = name;

    return expr;
}

UnaryExpr* newUnaryExpr(Expr* target, TokenType operator)
{
    UnaryExpr* expr = (UnaryExpr*) ALLOCATE_EXPRESSION(UnaryExpr, UNARY_EXPRESSION);

    expr->target = target;
    expr->operator = operator;

    return expr;
}

GroupedExpr* newGroupedExpr(Expr* in)
{
    GroupedExpr* expr = (GroupedExpr*) ALLOCATE_EXPRESSION(GroupedExpr, GROUPED_EXPRESSION);
    expr->in = in;
    return expr;
}

TernaryExpr* newTernaryExpr(Expr* condition, Expr* thenBranch, Expr* elseBranch)
{
    TernaryExpr* expr = (TernaryExpr*) ALLOCATE_EXPRESSION(TernaryExpr, TERNARY_EXPRESSION);

    expr->condition = condition;
    expr->thenBranch = thenBranch;
    expr->elseBranch = elseBranch;

    return expr;
}

LogicalExpr* newLogicalExpr(Expr* left, TokenType operator, Expr* right)
{
    LogicalExpr* expr = (LogicalExpr*) ALLOCATE_EXPRESSION(LogicalExpr, LOGICAL_EXPRESSION);

    expr->left = left;
    expr->operator = operator;
    expr->right = right;

    return expr;
}

BinaryExpr* newBinaryExpr(Expr* left, TokenType operator, Expr* right)
{
    BinaryExpr* expr = (BinaryExpr*) ALLOCATE_EXPRESSION(BinaryExpr, BINARY_EXPRESSION);

    expr->left = left;
    expr->operator = operator;
    expr->right = right;

    return expr;
}

ObjectExpr* newObjectExpr(Obj value)
{
    ObjectExpr* expr = (ObjectExpr*) ALLOCATE_EXPRESSION(ObjectExpr, OBJECT_EXPRESSION);
    expr->value = value;
    return expr;
}

LiteralExpr* newLiteralExpr(Value value)
{
    LiteralExpr* expr = (LiteralExpr*) ALLOCATE_EXPRESSION(LiteralExpr, LITERAL_EXPRESSION);
    expr->value = value;
    return expr;
}
