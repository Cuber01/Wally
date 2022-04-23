#include <stddef.h>
#include <malloc.h>

#include "native_error.h"
#include "list.h"
#include "memory.h"

static Node* newNode(NodeValue value)
{
    Node* newNode = reallocate(NULL, 0, sizeof(Node));
    newNode->next = NULL;
    newNode->value = value;

    return newNode;
}

void listAdd(Node** node, NodeValue value)
{

    if((*node) == NULL)
    {
        *node = newNode(value);
        return;
    }

    Node* lastNode = *node;

    while (lastNode->next != NULL)
    {
        lastNode = lastNode->next;
    }

    lastNode->next = newNode(value);
}

void listWriteValue(Node* root, int index, NodeValue value)
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

NodeValue listGet(Node* root, unsigned int index)
{
    Node* node = root;
    while(index > 0)
    {
        if(node->next == NULL)
        {
            nativeError("Index %d is outside the bounds of the list.", index);
        }

        node = node->next;
        index--;
    }

    return node->value;
}

int getLength(Node* root)
{
    Node* node = root;
    int length = 0;

    while (node != NULL)
    {
        node = node->next;
        length++;
    }

    return length;
}

void freeList(Node* root)
{
    Node* toFree;
    while (root != NULL)
    {
        toFree = root;
        root = root->next; // todo free values?
        free(toFree);
    }
}


