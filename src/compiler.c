#include "compiler.h"
#include "scanner.h"
#include "pretty_printer.h"

void compile(const char* source)
{
    initScanner(source);

    int line = -1;
    for (;;)
    {
        Token token = scanToken();
        bool isNewLine = false;

        if (token.line != line)
        {
            line = token.line;
            isNewLine = true;
        }

        printToken(token, isNewLine);

        if (token.type == TOKEN_EOF) break;
    }
}
