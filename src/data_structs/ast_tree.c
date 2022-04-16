#include <malloc.h>

#include "ast_tree.h"

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

// ------------ EXPRESSIONS ------------

LiteralExpr* newLiteralExpr(Value value)
{
    LiteralExpr* expr = (LiteralExpr*) ALLOCATE_EXPRESSION(LiteralExpr, LITERAL_EXPRESSION);
    expr->value = value;
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

LogicalExpr* newLogicalExpr(Expr* left, TokenType operator, Expr* right)
{
    LogicalExpr* expr = (LogicalExpr*) ALLOCATE_EXPRESSION(LogicalExpr, LOGICAL_EXPRESSION);

    expr->left = left;
    expr->operator = operator;
    expr->right = right;

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

UnaryExpr* newUnaryExpr(Expr* target, TokenType operator)
{
    UnaryExpr* expr = (UnaryExpr*) ALLOCATE_EXPRESSION(UnaryExpr, UNARY_EXPRESSION);

    expr->target = target;
    expr->operator = operator;

    return expr;
}

VarExpr* newVarExpr(const char* name)
{
    VarExpr* expr = (VarExpr*) ALLOCATE_EXPRESSION(VarExpr, VAR_EXPRESSION);

    expr->name = name;

    return expr;
}

AssignExpr* newAssignExpr(const char* name, Expr* value)
{
    AssignExpr* expr = (AssignExpr*) ALLOCATE_EXPRESSION(AssignExpr, ASSIGN_EXPRESSION);

    expr->name = name;
    expr->value = value;

    return expr;
}

CallExpr* newCallExpr(Expr* callee)
{
    CallExpr* expr = (CallExpr*) ALLOCATE_EXPRESSION(CallExpr, CALL_EXPRESSION);

    expr->callee = callee;
    // TODO args

    return expr;
}

// ------------ STATEMENTS ------------

//typedef struct
//{
//    Stmt stmt;
//
//    Expr* toPrint;
//} PrintStmt;
//
//typedef struct
//{
//    Stmt stmt;
//
//    // TODO list of stmt
//} BlockStmt;
//
//typedef struct
//{
//    Stmt stmt;
//
//    Expr* expr;
//} ExpressionStmt;
//
//typedef struct
//{
//    Stmt stmt;
//
//    Expr* condition;
//    Stmt* thenBranch;
//    Stmt* elseBranch;
//} IfStmt;
//
//typedef struct
//{
//    Stmt stmt;
//
//    Expr* condition;
//    Stmt* body;
//} WhileStmt;
//
//typedef struct
//{
//    Stmt stmt;
//
//    // TODO list of conditions
//    // TODO list of bodies
//    Stmt* defaultBranch;
//} SwitchStmt;
//
//typedef struct
//{
//    Stmt stmt;
//
//    const char* name;
//    Expr* initializer;
//} VariableStmt;
//
//
//typedef struct
//{
//    Stmt stmt;
//
//    const char* name;
//    Stmt* body;
//    // TODO list of params
//} FunctionStmt;
//
//typedef struct
//{
//    Stmt stmt;
//
//    Expr* value;
//} ReturnStmt;
//
//typedef struct
//{
//    Stmt stmt;
//} ContinueStmt;
//
//typedef struct
//{
//    Stmt stmt;
//} BreakStmt;
