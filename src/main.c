#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "vm.h"

// TODO for some reason release configuration doesn't work
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

    char* buffer = (char*)malloc(fileSize + 1); // todo free?
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

    freeVM();

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

    freeVM();
}

int main(int argc, const char* argv[])
{

//    Node* root = newNode((NodeValue){.as.expression = (Expr*)newLiteralExpr(NUMBER_VAL(1))});
//    listAdd(root, (NodeValue){.as.expression = (Expr*)newLiteralExpr(NUMBER_VAL(2))});
//    listAdd(root, (NodeValue){.as.expression = (Expr*)newLiteralExpr(NUMBER_VAL(3))});
//    printf("%f %f %f", AS_NUMBER(((LiteralExpr*)listGet(root, 0).as.expression)->value), AS_NUMBER(((LiteralExpr*)listGet(root, 1).as.expression)->value),
//           AS_NUMBER(((LiteralExpr*)listGet(root, 2).as.expression)->value) );
//    listWriteValue(root, 0, (NodeValue){.as.expression = (Expr*)newLiteralExpr(NUMBER_VAL(3))});
//    printf(" %f", AS_NUMBER(((LiteralExpr*)listGet(root, 0).as.expression)->value));
//    freeList(root);

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

    return 0;

}


