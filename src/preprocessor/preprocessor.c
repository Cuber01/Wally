#include <string.h>
#include "preprocessor.h"

char* preprocess(char* text)
{
    char* c = text;

    while(*c != '\0')
    {
        if (*c == '#')
        {

        }
        else
        {
            c++;
        }

    }

    return text;
}

static char* processDirective(char* text)
{
    char* c = text;

    switch (c)
    {
        case ' ':
        case '\r':
        case '\t':
        case '\n':
        case '/':
        break;

    }

}


void initPreprocessor(const char* source)
{
    preprocessor.start = source;
    preprocessor.current = source;
    preprocessor.line = 1;
}

static bool isAtEnd()
{
    return *preprocessor.current == '\0';
}

static char advance()
{
    preprocessor.current++;
    return preprocessor.current[-1];
}

static bool match(char expected)
{
    if (isAtEnd()) return false;
    if (*preprocessor.current != expected) return false;
    preprocessor.current++;
    return true;
}

static char peek()
{
    return *preprocessor.current;
}

static char peekNext()
{
    if (isAtEnd()) return '\0';
    return preprocessor.current[1];
}

static void skipWhitespace()
{
    for (;;)
    {
        char c = peek();
        switch (c)
        {
            case ' ':
            case '\r':
            case '\t':
                advance();
                break;

            case '\n':
                preprocessor.line++;
                advance();
                break;

            case '/':
                if (peekNext() == '/')
                {
                    // A comment goes until the end of the line.
                    while (peek() != '\n' && !isAtEnd()) advance();
                }
                else if (peekNext() == '*')
                {
                    advance();

                    while (!isAtEnd())
                    {
                        // If we don't find a '*', we can continue safely
                        if (advance() != '*') continue;

                        // If we find */, stop the comment
                        if (match('/'))
                        {
                            break;
                        }

                    }
                }
                else
                {
                    return;
                }
                break;

            default:
                return;
        }
    }
}



static DirectiveType checkKeyword(int start, int length, const char* rest, DirectiveType type)
{
    if (preprocessor.current - preprocessor.start == start + length &&          // Check length
        memcmp(preprocessor.start + start, rest, length) == 0)  // Check contents
    {
        return type;
    }

    return DIRECTIVE_SYMBOL;
}

static DirectiveType identifierType()
{
    switch (preprocessor.start[0])
    {
        case 'd': return checkKeyword(1, 5, "efine", DIRECTIVE_DEFINE);
        case 'u': return checkKeyword(1, 4, "ndef", DIRECTIVE_UNDEF);
        case 'f': return checkKeyword(1, 3, "ile", DIRECTIVE_FILE);
        case 'e': return checkKeyword(1, 4, "rror", DIRECTIVE_ERROR);
        case 'l': return checkKeyword(1, 3, "ine", DIRECTIVE_LINE);

        case 'i':
        {
            if (preprocessor.current - preprocessor.start > 1)
            {
                switch (preprocessor.start[1])
                {
                    case 'f':
                    {
                        if (preprocessor.current - preprocessor.start > 2)
                        {
                            switch (preprocessor.start[2])
                            {
                                case 'd': return checkKeyword(3, 2, "ef", DIRECTIVE_IFDEF);
                                case 'n': return checkKeyword(3, 3, "def", DIRECTIVE_IFNDEF);
                            }
                        }

                    }

                    case 'n': return checkKeyword(2, 5, "clude", DIRECTIVE_INCLUDE);
                }



                }
            }
            return checkKeyword(1, 6, "nclude", DIRECTIVE_INCLUDE);
        }

}
