#include "list.h"
#include "memory.h"

#define ALLOCATE_EXPRESSION(objectType, enumType, pop, line) \
    newExpression(sizeof(objectType), enumType, pop, line)

DEFINE_ARRAY_FUNCTIONS(Statements, statements, Stmt*)

static Expr* newExpression(size_t size, ExprType type, bool pop, uint16_t line)
{
    Expr* object = reallocate(NULL, 0, size);

    object->type = type;
    object->line = line;
    object->pop = pop;

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

        case DOT_EXPRESSION:
        {
            DotExpr* expression = (DotExpr*) expr;

            freeExpression(expression->instance);
            freeExpression(expression->value);
            freeObject((Obj*)expression->fieldName);

            FREE(DotExpr, expr);
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

            freeObject((Obj*)expression->callee);
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

        case BASE_EXPRESSION:
        {
            BaseExpr* expression = (BaseExpr*) expr;

            FREE(BaseExpr, expr);
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

        case FOR_STATEMENT:
        {
            ForStmt* statement = (ForStmt*) stmt;

            if(statement->condition != NULL)
            {
                freeExpression(statement->condition);
            }

            if(statement->declaration != NULL)
            {
                freeStatement(statement->declaration);
            }

            if(statement->increment != NULL)
            {
                freeExpression(statement->increment);
            }

            freeStatement(statement->body);
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

            freeObject((Obj*)statement->name); // todo check if this works
            freeList(statement->body);

            FREE(FunctionStmt, stmt);
            break;
        }

        case CLASS_STATEMENT:
        {
            ClassStmt* statement = (ClassStmt*) stmt;

            freeObject((Obj*)statement->name); // todo check if this works
            freeStatements(&statement->methods);

            FREE(ClassStmt, stmt);
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

// region Statements
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

ClassStmt* newClassStmt(ObjString* name, Expr* parent, Statements methods, uint16_t line)
{
    ClassStmt* stmt = (ClassStmt*) ALLOCATE_STATEMENT(ClassStmt, CLASS_STATEMENT, line);

    stmt->parent = parent;
    stmt->name = name;
    stmt->methods = methods;

    return stmt;
}

FunctionStmt* newFunctionStmt(ObjString* name, Node* body, ObjString** params, uint16_t paramCount, uint16_t line)
{
    FunctionStmt* stmt = (FunctionStmt*) ALLOCATE_STATEMENT(FunctionStmt, FUNCTION_STATEMENT, line);

    stmt->name = name;
    stmt->body = body;
    stmt->params = params;
    stmt->paramCount = paramCount;

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

ForStmt* newForStmt(Stmt* declaration, Expr* condition, Expr* increment, Stmt* body, uint16_t line)
{
    ForStmt* stmt = (ForStmt*) ALLOCATE_STATEMENT(ForStmt, FOR_STATEMENT, line);

    stmt->declaration = declaration;
    stmt->increment = increment;
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

// endregion

// region Expressions

CallExpr* newCallExpr(Expr* callee, uint8_t argCount, Node* args, uint16_t line)
{
    CallExpr* expr = (CallExpr*) ALLOCATE_EXPRESSION(CallExpr, CALL_EXPRESSION, true, line);

    expr->callee = callee;
    expr->args = args;
    expr->argCount = argCount;

    return expr;
}


AssignExpr* newAssignExpr(ObjString* name, Expr* value, uint16_t line)
{
    AssignExpr* expr = (AssignExpr*) ALLOCATE_EXPRESSION(AssignExpr, ASSIGN_EXPRESSION, true, line);

    expr->name = name;
    expr->value = value;

    return expr;
}

DotExpr* newDotExpr(Expr* instance, ObjString* fieldName, Expr* value,
                    bool isCall, Node* args, uint8_t argCount, uint16_t line)
{
    DotExpr* expr = (DotExpr*) ALLOCATE_EXPRESSION(DotExpr, DOT_EXPRESSION, true, line);

    expr->fieldName = fieldName;
    expr->instance = instance;
    expr->value = value;
    expr->args = args;
    expr->argCount = argCount;
    expr->isCall = isCall;

    return expr;
}

VarExpr* newVarExpr(ObjString* name, uint16_t line)
{
    VarExpr* expr = (VarExpr*) ALLOCATE_EXPRESSION(VarExpr, VAR_EXPRESSION, true, line);

    expr->name = name;

    return expr;
}

UnaryExpr* newUnaryExpr(Expr* target, TokenType op, uint16_t line)
{
    UnaryExpr* expr = (UnaryExpr*) ALLOCATE_EXPRESSION(UnaryExpr, UNARY_EXPRESSION, true, line);

    expr->target = target;
    expr->op = op;

    return expr;
}

BaseExpr* newBaseExpr(ObjString* methodName, uint16_t line)
{
    BaseExpr* expr = (BaseExpr*) ALLOCATE_EXPRESSION(BaseExpr, BASE_EXPRESSION, true, line);

    expr->methodName = methodName;

    return expr;
}

TernaryExpr* newTernaryExpr(Expr* condition, Expr* thenBranch, Expr* elseBranch, uint16_t line)
{
    TernaryExpr* expr = (TernaryExpr*) ALLOCATE_EXPRESSION(TernaryExpr, TERNARY_EXPRESSION, true, line);

    expr->condition = condition;
    expr->thenBranch = thenBranch;
    expr->elseBranch = elseBranch;

    return expr;
}

LogicalExpr* newLogicalExpr(Expr* left, TokenType op, Expr* right, uint16_t line)
{
    LogicalExpr* expr = (LogicalExpr*) ALLOCATE_EXPRESSION(LogicalExpr, LOGICAL_EXPRESSION, true, line);

    expr->left = left;
    expr->op = op;
    expr->right = right;

    return expr;
}

BinaryExpr* newBinaryExpr(Expr* left, TokenType op, Expr* right, uint16_t line)
{
    BinaryExpr* expr = (BinaryExpr*) ALLOCATE_EXPRESSION(BinaryExpr, BINARY_EXPRESSION, true, line);

    expr->left = left;
    expr->op = op;
    expr->right = right;

    return expr;
}

ListExpr* newListExpr(Node* expressions, uint16_t line)
{
    ListExpr* expr = (ListExpr*) ALLOCATE_EXPRESSION(ListExpr, LIST_EXPRESSION, true, line);
    expr->expressions = expressions;
    return expr;
}

SubscriptExpr* newSubscriptExpr(Expr* list, Expr* index, Expr* valueToStore, uint16_t line)
{
    bool pop = valueToStore == NULL ? true : false;

    SubscriptExpr* expr = (SubscriptExpr*) ALLOCATE_EXPRESSION(SubscriptExpr, SUBSCRIPT_EXPRESSION, pop, line);
    expr->index = index;
    expr->value = valueToStore;
    expr->list = list;

    return expr;
}

LiteralExpr* newLiteralExpr(Value value, uint16_t line)
{
    LiteralExpr* expr = (LiteralExpr*) ALLOCATE_EXPRESSION(LiteralExpr, LITERAL_EXPRESSION, true, line);
    expr->value = value;
    return expr;
}

// endregion
