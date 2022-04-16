
#include "value.h"
#include "scanner.h"

typedef enum {
    LITERAL_EXPRESSION,
    BINARY_EXPRESSION,
    UNARY_EXPRESSION,
    VAR_EXPRESSION,
    LOGICAL_EXPRESSION,
    GROUPED_EXPRESSION,
    ASSIGN_EXPRESSION,
    CALL_EXPRESSION,
    TERNARY_EXPRESSION,
    SET_EXPRESSION,
    GET_EXPRESSION,
    OBJECT_EXPRESSION
} ExprType;

typedef enum {
    EXPRESSION_STATEMENT,
    BLOCK_STATEMENT,
    IF_STATEMENT,
    WHILE_STATEMENT,
    FOR_STATEMENT,
    VAR_STATEMENT,
    CONTINUE_STATEMENT,
    BREAK_STATEMENT,
    FUNCTION_STATEMENT,
    RETURN_STATEMENT
} StmtType;

// ------------ EXPRESSIONS ------------

typedef struct {
    ExprType type;
    unsigned int line;
} Expr;

typedef struct
{
    Expr expr;

    Value value;
} LiteralExpr;

typedef struct
{
    Expr expr;

    Expr* left;
    TokenType operator;
    Expr* right;
} BinaryExpr;

typedef struct
{
    Expr expr;

    Expr* left;
    TokenType operator;
    Expr* right;
} LogicalExpr;

typedef struct
{
    Expr expr;

    Expr* in;
} GroupedExpr;

typedef struct
{
    Expr expr;

    Expr* condition;
    Expr* thenBranch;
    Expr* elseBranch;
} TernaryExpr;

typedef struct
{
    Expr expr;

    Expr* target;
    TokenType operator;
} UnaryExpr;

typedef struct
{
    Expr expr;

    const char* name;
} VarExpr;


typedef struct
{
    Expr expr;

    const char* name;
    Expr* value;
} AssignExpr;

typedef struct
{
    Expr expr;

    Expr* callee;
    // TODO args
} CallExpr;

// ------------ STATEMENTS ------------

typedef struct Stmt {
    StmtType type;
    unsigned int line;
} Stmt;

typedef struct
{
    Stmt stmt;

    Expr* toPrint;
} PrintStmt;

typedef struct
{
    Stmt stmt;

    // TODO list of stmt
} BlockStmt;

typedef struct
{
    Stmt stmt;

    Expr* expr;
} ExpressionStmt;

typedef struct
{
    Stmt stmt;

    Expr* condition;
    Stmt* thenBranch;
    Stmt* elseBranch;
} IfStmt;

typedef struct
{
    Stmt stmt;

    Expr* condition;
    Stmt* body;
} WhileStmt;

typedef struct
{
    Stmt stmt;

    // TODO list of conditions
    // TODO list of bodies
    Stmt* defaultBranch;
} SwitchStmt;

typedef struct
{
    Stmt stmt;

    const char* name;
    Expr* initializer;
} VariableStmt;


typedef struct
{
    Stmt stmt;

    const char* name;
    Stmt* body;
    // TODO list of params
} FunctionStmt;

typedef struct
{
    Stmt stmt;

    Expr* value;
} ReturnStmt;

typedef struct
{
    Stmt stmt;
} ContinueStmt;

typedef struct
{
    Stmt stmt;
} BreakStmt;
