#include <malloc.h>

#include "ast_tree.h"
#include "memory.h"

#define ALLOCATE_EXPRESSION(objectType, enumType) \
    newExpression(sizeof(objectType), enumType)

#define DEALLOCATE_EXPRESSION(expr, type) \
    reallocate(expr, sizeof(type), 0)

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

void freeStatement()
{

}

void freeExpression(Expr* expr)
{
    switch (expr->type)
    {
        case LITERAL_EXPRESSION:
        {
            DEALLOCATE_EXPRESSION(expr, LiteralExpr);
            break;
        }

        case BINARY_EXPRESSION:
        {
            BinaryExpr* expression = (BinaryExpr*) expr;

            freeExpression(expression->right);
            freeExpression(expression->left);

            DEALLOCATE_EXPRESSION(expr, BinaryExpr);
            break;
        }

        case UNARY_EXPRESSION:
        {
            UnaryExpr* expression = (UnaryExpr*) expr;

            freeExpression(expression->target);

            DEALLOCATE_EXPRESSION(expr, UnaryExpr);
            break;
        }

        case VAR_EXPRESSION:
        {
            DEALLOCATE_EXPRESSION(expr, VarExpr);
            break;
        }

        case ASSIGN_EXPRESSION:
        {
            AssignExpr* expression = (AssignExpr*) expr;

            freeExpression(expression->value);

            DEALLOCATE_EXPRESSION(expr, AssignExpr);
            break;
        }

        case CALL_EXPRESSION:
        {
            CallExpr* expression = (CallExpr*) expr;

            freeExpression(expression->callee);
            // TODO free arg list

            DEALLOCATE_EXPRESSION(expr, CallExpr);
            break;
        }

        case OBJECT_EXPRESSION:
        {
            ObjectExpr *expression = (ObjectExpr *) expr;

            freeObject(&expression->value);

            DEALLOCATE_EXPRESSION(expr, ObjectExpr);
            break;
        }

        case LOGICAL_EXPRESSION:
        {
            LogicalExpr* expression = (LogicalExpr*) expr;

            freeExpression(expression->right);
            freeExpression(expression->left);

            DEALLOCATE_EXPRESSION(expr, LogicalExpr);
            break;
        }

        case GROUPED_EXPRESSION:
        {
            GroupedExpr* expression = (GroupedExpr*) expr;

            freeExpression(expression->in);

            DEALLOCATE_EXPRESSION(expr, GroupedExpr);
            break;
        }

        case TERNARY_EXPRESSION:
        {
            TernaryExpr* expression = (TernaryExpr*) expr;

            freeExpression(expression->condition);
            freeExpression(expression->thenBranch);
            freeExpression(expression->elseBranch);

            DEALLOCATE_EXPRESSION(expr, TernaryExpr);
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
    VariableStmt* stmt = (VariableStmt*) ALLOCATE_STATEMENT(VariableStmt, VAR_STATEMENT);

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
