#include <stddef.h>
#include <malloc.h>
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

int listGet(Node* root, unsigned int index)
{
    Node* node = root;
    while(index + 1 > 0)
    {
        index--;

        if(node->next == NULL)
        {
            return NULL;
        }

        node = node->next;
    }

    return node->value;
}


