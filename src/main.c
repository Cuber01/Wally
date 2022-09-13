#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

    #ifdef DEBUG_PRINT_PREPROCESSOR
    printf("%s\n", preprocess(source));
    return;
    #endif

    int result = interpret(source);
    free(source);

    if(result == 0) freeVM();

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

int runWally(int argc, const char* argv[])
{
    switch(argc)
    {
        case 1:
        {
            repl();
            break;
        }

        case 2:
        {
            if(strcmp(argv[1], "--help") == 0)
            {
                printf("Wally is a dynamically-typed interpreted programming language.\n");
                printf("Repo: https://github.com/Cuber01/Wally\n");
                printf("Documentation: https://github.com/Cuber01/Wally/wiki\n\n");

                printf("Commandline arguments:\n");
                printf("    --help                - Display this message\n");
                printf("    --interpret \"code\"    - Run \"code\" string\n");
                printf("    [path to file]        - Run Wally script\n");
                printf("    [none]                - Run interactive repl\n");
            }
            else
            {
                runFile(argv[1]);
            }
            break;
        }

        case 3:
        {
            if(strcmp(argv[1], "--interpret") == 0)
            {
                int result = interpret(argv[2]);

                if(result == 0) freeVM();

                exit(result);
            }
            else
            {
                fprintf(stderr, "Usage: Wally [path to file]\n");
                exit(64);
            }
            break;
        }

        default:
        {
            fprintf(stderr, "Usage: Wally [path to file]\n");
            exit(64);
        }
    }

    return 0;
}

#ifndef LIBRARY
int main(int argc, const char* argv[])
{
    initVM();
    return runWally(argc, argv);
}
#endif

