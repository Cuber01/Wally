#ifndef WALLY_LIST_H
#define WALLY_LIST_H

#include "ast_tree.h"

#define AS_STATEMENT(node)  (node)->value.as.statement
#define AS_EXPRESSION(node) (node)->value.as.expression

typedef struct NodeValue
{
    union {
        Expr* expression;
        Stmt* statement;
    } as;
} NodeValue;

typedef struct Node
{
    struct Node* next;
    NodeValue value;
} Node;

Node* newNode(NodeValue value);
void listAdd(Node* node, NodeValue value);
NodeValue listGet(Node* root, unsigned int index);
void listWriteValue(Node* root, int index, NodeValue value);
void freeList(Node* root);

#endif //WALLY_LIST_H