#ifndef WALLY_LIST_H
#define WALLY_LIST_H

typedef struct Node
{
    struct Node* next;
    void* value;
} Node;

Node* newNode(void* value);
void listAdd(Node* node, void* value);
void* listGet(Node* root, unsigned int index);
void listWriteValue(Node* root, int index, void* value);
void freeList(Node* root);

#endif //WALLY_LIST_H