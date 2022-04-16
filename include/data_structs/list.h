#ifndef WALLY_LIST_H
#define WALLY_LIST_H

typedef struct Node
{
    struct Node* next;
    int value;
} Node;

Node* newNode(int value);
void listAdd(Node* node, int value);
int listGet(Node* root, unsigned int index);
void listWriteValue(Node* root, int index, int value);
void freeList(Node* root);

#endif //WALLY_LIST_H