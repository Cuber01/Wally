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

#endif //WALLY_LIST_H