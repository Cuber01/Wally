#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "chunk.h"
#include "disassembler.h"
#include "list.h"
#include "vm.h"


static char* readFile(const char* path)
{
    FILE* file = fopen(path, "rb");
    if (file == NULL)
    {
        fprintf(stderr, "Could not open file \"%s\".\n", path);
        exit(74);
    }

    fseek(file, 0L, SEEK_END);
    size_t fileSize = ftell(file);
    rewind(file);

    char* buffer = (char*)malloc(fileSize + 1);
    if (buffer == NULL)
    {
        fprintf(stderr, "Not enough memory to read \"%s\".\n", path);
        exit(74);
    }

    size_t bytesRead = fread(buffer, sizeof(char), fileSize, file);

    if (bytesRead < fileSize)
    {
        fprintf(stderr, "Could not read file \"%s\".\n", path);
        exit(74);
    }
    buffer[bytesRead] = '\0';

    fclose(file);
    return buffer;
}

static void runFile(const char* path)
{
    char* source = readFile(path);
    int result = interpret(source);
    free(source);

    exit(result);
}

static void repl()
{
    char line[1024];

    for (;;)
    {
        printf("> ");

        if (!fgets(line, sizeof(line), stdin))
        {
            printf("\n");
            break;
        }

        line[strlen(line)-1] = '\0';

        interpret(line);
    }
}

int main(int argc, const char* argv[])
{
    Node* root = newNode(0);
    listAdd(root, 1);
    listAdd(root, 2);
    printf("%d %d %d", listGet(root, 0), listGet(root, 1), listGet(root, 2) );
    listWriteValue(root, 0, 9);
    printf("%d", listGet(root, 0));


    /*
    initVM();

    if (argc == 1)
    {
        repl();
    } else if (argc == 2)
    {
        runFile(argv[1]);
    } else
    {
        fprintf(stderr, "Usage: Wally [path]\n");
        exit(64);
    }

    freeVM();

    return 0;
    */
}


