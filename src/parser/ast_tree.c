#include "list.h"
#include "memory.h"

#define ALLOCATE_EXPRESSION(objectType, enumType, line) \
    newExpression(sizeof(objectType), enumType, line)

static Expr* newExpression(size_t size, ExprType type, uint16_t line)
{
    Expr* object = reallocate(NULL, 0, size);

    object->type = type;
    object->line = line;

    return object;
}

#define ALLOCATE_STATEMENT(objectType, enumType, line) \
    newStatement(sizeof(objectType), enumType, line)

static Stmt* newStatement(size_t size, StmtType type, uint16_t line)
{
    Stmt* object = reallocate(NULL, 0, size);

    object->type = type;
    object->line = line;

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
            freeList(expression->args);

            FREE(CallExpr, expr);
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

            freeList(statement->statements);

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

            freeList(statement->params);
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

            freeList(statement->caseBodies);
            freeList(statement->conditions);

            freeStatement(statement->defaultBranch);

            FREE(SwitchStmt, stmt);
            break;
        }

    }
}

// ------------ EXPRESSIONS ------------
ContinueStmt* newContinueStmt(uint16_t line)
{
    ContinueStmt* stmt = (ContinueStmt*) ALLOCATE_STATEMENT(ContinueStmt, CONTINUE_STATEMENT, line);
    return stmt;
}

BreakStmt* newBreakStmt(uint16_t line)
{
    BreakStmt* stmt = (BreakStmt*) ALLOCATE_STATEMENT(BreakStmt, BREAK_STATEMENT, line);
    return stmt;
}

ReturnStmt* newReturnStmt(Expr* value, uint16_t line)
{
    ReturnStmt* stmt = (ReturnStmt*) ALLOCATE_STATEMENT(ReturnStmt, RETURN_STATEMENT, line);

    stmt->value = value;

    return stmt;
}

FunctionStmt* newFunctionStmt(ObjString* name, Stmt* body, Node* params, uint16_t line)
{
    FunctionStmt* stmt = (FunctionStmt*) ALLOCATE_STATEMENT(FunctionStmt, FUNCTION_STATEMENT, line);

    stmt->name = name;
    stmt->body = body;
    stmt->params = params;

    return stmt;
}

VariableStmt* newVariableStmt(ObjString* name, Expr* initializer, uint16_t line)
{
    VariableStmt* stmt = (VariableStmt*) ALLOCATE_STATEMENT(VariableStmt, VARIABLE_STATEMENT, line);

    stmt->name = name;
    stmt->initializer = initializer;

    return stmt;
}

SwitchStmt* newSwitchStmt(Node* caseConditions, Node* caseBodies, Stmt* defaultBranch, uint16_t line)
{
    SwitchStmt* stmt = (SwitchStmt*) ALLOCATE_STATEMENT(SwitchStmt, SWITCH_STATEMENT, line);

    stmt->defaultBranch = defaultBranch;

    stmt->caseBodies = caseBodies;
    stmt->conditions = caseConditions;

    return stmt;
}

WhileStmt* newWhileStmt(Expr* condition, Stmt* body, uint16_t line)
{
    WhileStmt* stmt = (WhileStmt*) ALLOCATE_STATEMENT(WhileStmt, WHILE_STATEMENT, line);

    stmt->condition = condition;
    stmt->body = body;

    return stmt;
}

IfStmt* newIfStmt(Expr* condition, Stmt* thenBranch, Stmt* elseBranch, uint16_t line)
{
    IfStmt* stmt = (IfStmt*) ALLOCATE_STATEMENT(IfStmt, IF_STATEMENT, line);

    stmt->condition = condition;
    stmt->thenBranch = thenBranch;
    stmt->elseBranch = elseBranch;

    return stmt;
}

BlockStmt* newBlockStmt(Node* statements, uint16_t line)
{
    BlockStmt* stmt = (BlockStmt*) ALLOCATE_STATEMENT(BlockStmt, BLOCK_STATEMENT, line);

    stmt->statements = statements;

    return stmt;
}

ExpressionStmt* newExpressionStmt(Expr* expr, uint16_t line)
{
    ExpressionStmt* stmt = (ExpressionStmt*) ALLOCATE_STATEMENT(ExpressionStmt, EXPRESSION_STATEMENT, line);

    stmt->expr = expr;

    return stmt;
}

CallExpr* newCallExpr(ObjString* callee, Node* args, uint16_t line)
{
    CallExpr* expr = (CallExpr*) ALLOCATE_EXPRESSION(CallExpr, CALL_EXPRESSION, line);

    expr->callee = callee;
    expr->args = args;

    return expr;
}

AssignExpr* newAssignExpr(ObjString* name, Expr* value, uint16_t line)
{
    AssignExpr* expr = (AssignExpr*) ALLOCATE_EXPRESSION(AssignExpr, ASSIGN_EXPRESSION, line);

    expr->name = name;
    expr->value = value;

    return expr;
}

VarExpr* newVarExpr(ObjString* name, uint16_t line)
{
    VarExpr* expr = (VarExpr*) ALLOCATE_EXPRESSION(VarExpr, VAR_EXPRESSION, line);

    expr->name = name;

    return expr;
}

UnaryExpr* newUnaryExpr(Expr* target, TokenType operator, uint16_t line)
{
    UnaryExpr* expr = (UnaryExpr*) ALLOCATE_EXPRESSION(UnaryExpr, UNARY_EXPRESSION, line);

    expr->target = target;
    expr->operator = operator;

    return expr;
}

GroupedExpr* newGroupedExpr(Expr* in, uint16_t line)
{
    GroupedExpr* expr = (GroupedExpr*) ALLOCATE_EXPRESSION(GroupedExpr, GROUPED_EXPRESSION, line);
    expr->in = in;
    return expr;
}

TernaryExpr* newTernaryExpr(Expr* condition, Expr* thenBranch, Expr* elseBranch, uint16_t line)
{
    TernaryExpr* expr = (TernaryExpr*) ALLOCATE_EXPRESSION(TernaryExpr, TERNARY_EXPRESSION, line);

    expr->condition = condition;
    expr->thenBranch = thenBranch;
    expr->elseBranch = elseBranch;

    return expr;
}

LogicalExpr* newLogicalExpr(Expr* left, TokenType operator, Expr* right, uint16_t line)
{
    LogicalExpr* expr = (LogicalExpr*) ALLOCATE_EXPRESSION(LogicalExpr, LOGICAL_EXPRESSION, line);

    expr->left = left;
    expr->operator = operator;
    expr->right = right;

    return expr;
}

BinaryExpr* newBinaryExpr(Expr* left, TokenType operator, Expr* right, uint16_t line)
{
    BinaryExpr* expr = (BinaryExpr*) ALLOCATE_EXPRESSION(BinaryExpr, BINARY_EXPRESSION, line);

    expr->left = left;
    expr->operator = operator;
    expr->right = right;

    return expr;
}

LiteralExpr* newLiteralExpr(Value value, uint16_t line)
{
    LiteralExpr* expr = (LiteralExpr*) ALLOCATE_EXPRESSION(LiteralExpr, LITERAL_EXPRESSION, line);
    expr->value = value;
    return expr;
}