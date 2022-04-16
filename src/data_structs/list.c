#include <stddef.h>
#include <malloc.h>

#include "native_error.h"
#include "list.h"

Node* newNode(int value)
{
    Node* newNode = malloc(sizeof(Node));
    newNode->next = NULL;
    newNode->value = value;

    return newNode;
}

void listAdd(Node* node, int value)
{
    while (node->next != NULL)
    {
        node = node->next;
    }

    node->next = newNode(value);
}

void listWriteValue(Node* root, int index, int value)
{
    Node* node = root;
    while(index > 0)
    {
        index--;

        if(node->next == NULL)
        {
            nativeError("Index %d is outside the bounds of the list.", index);
            return;
        }

        node = node->next;
    }

    node->value = value;
}

int listGet(Node* root, unsigned int index)
{
    Node* node = root;
    while(index > 0)
    {
        index--;

        if(node->next == NULL)
        {
            nativeError("Index %d is outside the bounds of the list.", index);
            return -999;
        }

        node = node->next;
    }

    return node->value;
}

void freeList(Node* root)
{
    Node* toFree;
    while (root != NULL)
    {
        toFree = root;
        root = root->next;
        free(toFree);
    }

}


