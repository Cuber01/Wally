#include <string.h>
#include <stdio.h>
#include "preprocessor.h"

void initPreprocessor(char* source)
{
    preprocessor.start = source;
    preprocessor.current = source;
    preprocessor.line = 1;
}

// region Util

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

static bool skipWhitespace()
{
    bool whiteSpaceEncountered = false;

    for (;;)
    {
        char c = peek();
        switch (c)
        {
            case ' ':
            case '\r':
            case '\t':
                whiteSpaceEncountered = true;
                advance();
                break;

            case '\n':
                whiteSpaceEncountered = true;
                preprocessor.line++;
                advance();
                break;

            case '/':
                if (peekNext() == '/')
                {
                    whiteSpaceEncountered = true;

                    // A comment goes until the end of the line.
                    while (peek() != '\n' && !isAtEnd()) advance();
                }
                else if (peekNext() == '*')
                {
                    whiteSpaceEncountered = true;

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
                    return whiteSpaceEncountered;
                }
                break;

            default:
                return whiteSpaceEncountered;
        }
    }
}

static bool isAlpha(char c)
{
    return (c >= 'a' && c <= 'z') ||
           (c >= 'A' && c <= 'Z') ||
           c == '_';
}

// endregion

// region Symbols

static void symbol()
{
    printf("symbol?");
}

// endregion

// region Directives

static DirectiveType checkKeyword(int start, int length, const char* rest, DirectiveType type)
{
    if (preprocessor.current - preprocessor.start == start + length &&          // Check length
        memcmp(preprocessor.start + start, rest, length) == 0)  // Check contents
    {
        return type;
    }

    return DIRECTIVE_NONE;
}

static DirectiveType getDirectiveType()
{

    switch (preprocessor.start[1])
    {

        case 'd': return checkKeyword(1, 5, "efine", DIRECTIVE_DEFINE);
        case 'u': return checkKeyword(1, 4, "ndef", DIRECTIVE_UNDEF);
        case 'e': return checkKeyword(1, 4, "rror", DIRECTIVE_ERROR);

        case 'i':
        {
            if (preprocessor.current - preprocessor.start > 1)
            {
                switch (preprocessor.start[2])
                {
                    case 'f':
                    {
                        if (preprocessor.current - preprocessor.start > 2)
                        {
                            switch (preprocessor.start[3])
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

    }

    return DIRECTIVE_NONE;
}

static void directive()
{
    while (isAlpha(peek())) advance();
    DirectiveType type = getDirectiveType();

    switch (type)
    {
        case DIRECTIVE_INCLUDE:
        case DIRECTIVE_DEFINE:
        case DIRECTIVE_UNDEF:
        case DIRECTIVE_IFDEF:
        case DIRECTIVE_IFNDEF:
        case DIRECTIVE_ERROR:
        break;

        default:
        {
            printf("Expect directive after '#'.\n"); // todo error
        }
    }
}


// endregion

char* preprocess(char* text)
{
    initPreprocessor(text);

    skipWhitespace();
    preprocessor.start = preprocessor.current;

    for(;;)
    {
        if (isAtEnd()) return preprocessor.start;

        char c = advance();

        if(c == '#')
        {
            advance();
            directive();
        }
        else if(isAlpha(c))
        {
            symbol();
        }

    }


}