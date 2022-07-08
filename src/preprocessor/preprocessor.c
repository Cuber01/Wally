#include <string.h>
#include <stdio.h>
#include "preprocessor.h"
#include "colors.h"
#include "object.h"
#include "scanner.h"
#include "memory.h"

Preprocessor preprocessor;

// region Main

void initPreprocessor(char* source)
{
    preprocessor.defines = ALLOCATE_TABLE(); // todo zamiast edytować source directly przepisuj chary które pasują
    initTable(preprocessor.defines);

    preprocessor.start = source;
    preprocessor.current = source;
    preprocessor.escapeNextChar = false;
    preprocessor.line = 1;

    preprocessor.outputStart = preprocessor.output;
}

void freePreprocessor()
{
    freeTable(preprocessor.defines);
}

// endregion

// region Util

static bool isAtEnd()
{
    return *preprocessor.current == '\0';
}

static char advanceNoSend()
{
    preprocessor.current++;
    return preprocessor.current[-1];
}

static char advance()
{
    preprocessor.output = preprocessor.current;
    preprocessor.output++;

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

static bool skipWhitespace(bool newLineIsWhitespace)
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
                if(!newLineIsWhitespace) return whiteSpaceEncountered;

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

static bool isValidDefineChar(char c)
{
    // TODO wip

    if(preprocessor.escapeNextChar)
    {
        preprocessor.escapeNextChar = false;

        // Successfully escaped newline
        if(c == '\n')
        {
            return true;
        }

    }

    if(c == '\\')
    {
        preprocessor.escapeNextChar = true;
        return true;
    }
}

// endregion

// region Symbols

static void symbol()
{
    printf("symbol?");
}

// endregion

// region Directive Execution

static void includeDirective()
{

}

static void defineDirective()
{
//    if(!skipWhitespace(false))
//    {
//        colorWriteline(RED, "Expect whitespace after define directive.");
//        return;
//    }
    skipWhitespace(false);

    preprocessor.start = preprocessor.current;
    while (isAlpha(peek())) advanceNoSend();
    ObjString* name = copyString(preprocessor.start, preprocessor.current - preprocessor.start);

    skipWhitespace(false);

    preprocessor.start = preprocessor.current;
    while (isValidDefineChar(peek())) advanceNoSend();
    Value content = OBJ_VAL(copyString(preprocessor.start, preprocessor.current - preprocessor.start));

    tableSet(preprocessor.defines, name, content);

}

static void undefDirective()
{

}

static void ifdefDirective()
{

}

static void ifndefDirective()
{

}

static void errorDirective()
{

}

// endregion

// region Directives

static DirectiveType checkKeyword(int start, int length, const char* rest, DirectiveType type)
{
    // +1 is for the '#'
    if (preprocessor.current - preprocessor.start == start + length + 1 &&      // Check length
        memcmp(preprocessor.start + start + 1, rest, length ) == 0)  // Check contents
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
    while (isAlpha(peek())) advanceNoSend();
    DirectiveType type = getDirectiveType();

    switch (type)
    {
        case DIRECTIVE_INCLUDE: includeDirective();  break;
        case DIRECTIVE_DEFINE:  defineDirective();   break;
        case DIRECTIVE_UNDEF:   undefDirective();    break;
        case DIRECTIVE_IFDEF:   ifdefDirective();    break;
        case DIRECTIVE_IFNDEF:  ifndefDirective();   break;
        case DIRECTIVE_ERROR:   errorDirective();    break;

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

    for(;;)
    {
        if (isAtEnd())
        {
            return preprocessor.output;
        }

        skipWhitespace(true);
        preprocessor.start = preprocessor.current;

        char c = advance();

        if(c == '#')
        {
            advanceNoSend();
            directive();
        }
        else if(isAlpha(c))
        {
            symbol();
        }

    }


}