#include "value.h"
#include "scanner.h"
#include "object.h"
#include "list.h"

typedef struct Node Node;

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
    FOR_STATEMENT,
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
    uint16_t line;
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

    ObjString* name;
} VarExpr;

typedef struct
{
    Expr expr;

    ObjString* name;
    Expr* value;
} AssignExpr;

typedef struct
{
    Expr expr;

    ObjString* callee;
    uint16_t argCount;
    Node* args;
} CallExpr;

// ------------ STATEMENTS ------------

typedef struct Stmt {
    StmtType type;
    uint16_t line;
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

    ObjString* name;
    Expr* initializer;
} VariableStmt;

typedef struct
{
    Stmt stmt;

    Stmt* declaration;
    Expr* condition;
    Expr* increment;
    Stmt* body;
} ForStmt;

typedef struct
{
    Stmt stmt;

    ObjString* name;
    Node* body;

    ObjString** params;
    uint16_t paramCount;

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

LiteralExpr* newLiteralExpr(Value value, uint16_t line);
BinaryExpr* newBinaryExpr(Expr* left, TokenType operator, Expr* right, uint16_t line);
LogicalExpr* newLogicalExpr(Expr* left, TokenType operator, Expr* right, uint16_t line);
GroupedExpr* newGroupedExpr(Expr* in, uint16_t line);
TernaryExpr* newTernaryExpr(Expr* condition, Expr* thenBranch, Expr* elseBranch, uint16_t line);
UnaryExpr* newUnaryExpr(Expr* target, TokenType operator, uint16_t line);
VarExpr* newVarExpr(ObjString* name, uint16_t line);
AssignExpr* newAssignExpr(ObjString* name, Expr* value, uint16_t line);
CallExpr* newCallExpr(ObjString* callee, uint16_t argCount, Node* args, uint16_t line);

// ------------ STATEMENT CONSTRUCTORS ------------

ExpressionStmt* newExpressionStmt(Expr* expr, uint16_t line);
BlockStmt* newBlockStmt(Node* statements, uint16_t line);
IfStmt* newIfStmt(Expr* condition, Stmt* thenBranch, Stmt* elseBranch, uint16_t line);
WhileStmt* newWhileStmt(Expr* condition, Stmt* body, uint16_t line);
ForStmt* newForStmt(Stmt* declaration, Expr* condition, Expr* increment, Stmt* body, uint16_t line);
SwitchStmt* newSwitchStmt(Node* caseConditions, Node* caseBodies, Stmt* defaultBranch, uint16_t line);
VariableStmt* newVariableStmt(ObjString* name, Expr* initializer, uint16_t line);
FunctionStmt* newFunctionStmt(ObjString* name, Node* body, ObjString** params, uint16_t paramCount, uint16_t line);
ReturnStmt* newReturnStmt(Expr* value, uint16_t line);
BreakStmt* newBreakStmt(uint16_t line);
ContinueStmt* newContinueStmt(uint16_t line);