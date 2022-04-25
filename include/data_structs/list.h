#ifndef WALLY_LIST_H
#define WALLY_LIST_H

#include "ast_tree.h"

#define AS_STATEMENT(node)  (node)->value.as.statement
#define AS_EXPRESSION(node) (node)->value.as.expression

#define NODE_STATEMENT_VALUE(value)  (NodeValue){.as.statement = value}
#define NODE_EXPRESSION_VALUE(value) (NodeValue){.as.expression = value}
#define NODE_VAL_VALUE(value)        (NodeValue){.as.val = value}

typedef struct NodeValue
{
    union {
        Expr* expression;
        Stmt* statement;
        Value* val; // Changing this to a non-pointer causes a segfault
    } as;
} NodeValue;

typedef struct Node
{
    struct Node* next;
    NodeValue value;
} Node;

void listAdd(Node** node, NodeValue value);
NodeValue listGet(Node* root, unsigned int index);
void listWriteValue(Node* root, int index, NodeValue value);
void freeList(Node* root);
int listGetLength(Node* root);

#endif //WALLY_LIST_H