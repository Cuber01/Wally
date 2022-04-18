#include "value.h"
#include "scanner.h"
#include "object.h"
#include "list.h"

typedef struct Node Node; // todo

typedef enum {
    BINARY_EXPRESSION,
    LITERAL_EXPRESSION,
    UNARY_EXPRESSION,
    VAR_EXPRESSION,
    LOGICAL_EXPRESSION,
    GROUPED_EXPRESSION,
    ASSIGN_EXPRESSION,
    CALL_EXPRESSION,
    TERNARY_EXPRESSION
} ExprType;

typedef enum {
    EXPRESSION_STATEMENT,
    BLOCK_STATEMENT,
    IF_STATEMENT,
    WHILE_STATEMENT,
    SWITCH_STATEMENT,
    VARIABLE_STATEMENT,
    CONTINUE_STATEMENT,
    BREAK_STATEMENT,
    FUNCTION_STATEMENT,
    RETURN_STATEMENT
} StmtType;

// ------------ EXPRESSIONS ------------

typedef struct {
    ExprType type;
    uint32_t line;
} Expr;

typedef struct
{
    Expr expr;

    Value value;
} LiteralExpr;

typedef struct
{
    Expr expr;

    Obj value;
} ObjectExpr;

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
    Node* args;
} CallExpr;

// ------------ STATEMENTS ------------

typedef struct Stmt {
    StmtType type;
    uint32_t line;
} Stmt;

typedef struct
{
    Stmt stmt;

    Expr* toPrint;
} PrintStmt;

typedef struct
{
    Stmt stmt;

    Node* statements;
} BlockStmt;

typedef struct
{
    Stmt stmt;

    ExprType type;
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

    Node* conditions;
    Node* caseBodies;

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
    Node* params;
    Stmt* body;

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

// ------------ EXPRESSION CONSTRUCTORS ------------

LiteralExpr* newLiteralExpr(Value value, uint32_t line);
BinaryExpr* newBinaryExpr(Expr* left, TokenType operator, Expr* right, uint32_t line);
LogicalExpr* newLogicalExpr(Expr* left, TokenType operator, Expr* right, uint32_t line);
GroupedExpr* newGroupedExpr(Expr* in, uint32_t line);
TernaryExpr* newTernaryExpr(Expr* condition, Expr* thenBranch, Expr* elseBranch, uint32_t line);
UnaryExpr* newUnaryExpr(Expr* target, TokenType operator, uint32_t line);
VarExpr* newVarExpr(const char* name, uint32_t line);
AssignExpr* newAssignExpr(const char* name, Expr* value, uint32_t line);
CallExpr* newCallExpr(Expr* callee, Node* args, uint32_t line);

// ------------ STATEMENT CONSTRUCTORS ------------

ExpressionStmt* newExpressionStmt(Expr* expr, uint32_t line);
BlockStmt* newBlockStmt(Node* statements, uint32_t line);
IfStmt* newIfStmt(Expr* condition, Stmt* thenBranch, Stmt* elseBranch, uint32_t line);
WhileStmt* newWhileStmt(Expr* condition, Stmt* body, uint32_t line);
SwitchStmt* newSwitchStmt(Node* caseConditions, Node* caseBodies, Stmt* defaultBranch, uint32_t line);
VariableStmt* newVariableStmt(const char* name, Expr* initializer, uint32_t line);
FunctionStmt* newFunctionStmt(const char* name, Stmt* body, Node* params, uint32_t line);
ReturnStmt* newReturnStmt(Expr* value, uint32_t line);
BreakStmt* newBreakStmt(uint32_t line);
ContinueStmt* newContinueStmt(uint32_t line);