#include "scanner.h"

Scanner scanner;

void initScanner(const char* source)
{
    scanner.start = source;
    scanner.current = source;
    scanner.line = 1;
    scanner.returnNext = TOKEN_NONE;
}

static Token makeToken(TokenType type)
{
    Token token;
    token.type = type;
    token.start = scanner.start;
    token.length = (int)(scanner.current - scanner.start);
    token.line = scanner.line;
    return token;
}

static Token errorToken(const char* message)
{
    Token token;
    token.type = TOKEN_ERROR;
    token.start = message;
    token.length = (int)strlen(message);
    token.line = scanner.line;
    return token;
}

static bool isAtEnd()
{
    return *scanner.current == '\0';
}

static char advance()
{
    scanner.current++;
    return scanner.current[-1];
}

static bool match(char expected)
{
    if (isAtEnd()) return false;
    if (*scanner.current != expected) return false;
    scanner.current++;
    return true;
}

static char peek()
{
    return *scanner.current;
}

static char peekNext()
{
    if (isAtEnd()) return '\0';
    return scanner.current[1];
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
                scanner.line++;
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

bool isDigit(char c)
{
    return c >= '0' && c <= '9';
}

bool isAlpha(char c)
{
    return (c >= 'a' && c <= 'z') ||
           (c >= 'A' && c <= 'Z') ||
           c == '_';
}

static Token number()
{
    while (isDigit(peek())) advance();

    // Look for a fractional part.
    if (peek() == '.' && isDigit(peekNext()))
    {
        // Consume the ".".
        advance();

        while (isDigit(peek())) advance();
    }

    return makeToken(TOKEN_NUMBER);
}

static Token string()
{
    while (peek() != '"' && !isAtEnd())
    {
        if (peek() == '\n') scanner.line++;
        if (peek() == '\\')
        {
            if(peekNext() == '"')
            {
               advance();
            }
        }
        advance();
    }

    if (isAtEnd()) return errorToken("Unterminated string.");

    // Advance through the closing quote.
    advance();
    return makeToken(TOKEN_STRING);
}


static TokenType checkKeyword(int start, int length, const char* rest, TokenType type)
{
    if (scanner.current - scanner.start == start + length &&          // Check length
        memcmp(scanner.start + start, rest, length) == 0)  // Check contents
    {
        return type;
    }

    return TOKEN_IDENTIFIER;
}

static TokenType identifierType()
{
    switch (scanner.start[0])
    {
        case 'e': return checkKeyword(1, 3, "lse", TOKEN_ELSE);
        case 'i': return checkKeyword(1, 1, "f", TOKEN_IF);
        case 'n': return checkKeyword(1, 3, "ull", TOKEN_NULL);
        case 'r': return checkKeyword(1, 5, "eturn", TOKEN_RETURN);
        case 'v': return checkKeyword(1, 2, "ar", TOKEN_VAR);
        case 'w': return checkKeyword(1, 4, "hile", TOKEN_WHILE);

        case 'd': return checkKeyword(1, 6, "efault", TOKEN_DEFAULT);
        case 's': return checkKeyword(1, 5, "witch", TOKEN_SWITCH);

        case 'b':
        {
            if (scanner.current - scanner.start > 1)
            {
                switch (scanner.start[1])
                {
                    case 'r': return checkKeyword(2, 3, "eak", TOKEN_BREAK);
                    case 'a': return checkKeyword(2, 2, "se", TOKEN_BASE);
                }
            }
        }

        case 'c':
        {
            if (scanner.current - scanner.start > 1)
            {
                switch (scanner.start[1])
                {
                    case 'o': return checkKeyword(2, 6, "ntinue", TOKEN_CONTINUE);
                    case 'l': return checkKeyword(2, 3, "ass", TOKEN_CLASS);
                    case 'a': return checkKeyword(2, 2, "se", TOKEN_CASE);
                }
            }
            break;
        }

        case 'f':
        {
            if (scanner.current - scanner.start > 1) {
                switch (scanner.start[1]) {
                    case 'a':
                        return checkKeyword(2, 3, "lse", TOKEN_FALSE);
                    case 'o':
                        return checkKeyword(2, 1, "r", TOKEN_FOR);
                    case 'u':
                        return checkKeyword(2, 6, "nction", TOKEN_FUNCTION);
                }
            }
            break;
        }

        case 't':
        {
            if (scanner.current - scanner.start > 1) {
                switch (scanner.start[1]) {
                    case 'h':
                        return checkKeyword(2, 2, "is", TOKEN_THIS);
                    case 'r':
                        return checkKeyword(2, 2, "ue", TOKEN_TRUE);
                }
            }
            break;
        }

    }

    return TOKEN_IDENTIFIER;
}

static Token identifier()
{
    while (isAlpha(peek()) || isDigit(peek())) advance();
    return makeToken(identifierType());
}

Token scanToken()
{
    if(scanner.returnNext != TOKEN_NONE)
    {
        TokenType type = scanner.returnNext;
        scanner.returnNext = TOKEN_NONE;

        return makeToken(type);
    }

    skipWhitespace();
    scanner.start = scanner.current;

    if (isAtEnd()) return makeToken(TOKEN_EOF);

    char c = advance();
    if (isDigit(c)) return number();
    if (isAlpha(c)) return identifier();

    switch (c) {
        case '(':
            return makeToken(TOKEN_LEFT_PAREN);
        case ')':
            return makeToken(TOKEN_RIGHT_PAREN);
        case '{':
            return makeToken(TOKEN_LEFT_BRACE);
        case '}':
            return makeToken(TOKEN_RIGHT_BRACE);
        case ';':
            return makeToken(TOKEN_SEMICOLON);
        case ':':
            return makeToken(TOKEN_COLON);
        case ',':
            return makeToken(TOKEN_COMMA);
        case '.':
            return makeToken(TOKEN_DOT);
        case '$':
            return makeToken(TOKEN_DOLLAR);
        case '?':
            return makeToken(TOKEN_QUESTION_MARK);

        case '+':
            if (match('='))
            {
                scanner.returnNext = TOKEN_PLUS;
                return makeToken(TOKEN_EQUAL);
            }
            else if (match('+'))
            {
                return makeToken(TOKEN_PLUS_PLUS);
            }
            else return makeToken(TOKEN_PLUS);

        case '-':
            if (match('='))
            {
                scanner.returnNext = TOKEN_MINUS_E;
                return makeToken(TOKEN_EQUAL);
            }
            else if (match('-'))
            {
                return makeToken(TOKEN_MINUS_MINUS);
            }
            else return makeToken(TOKEN_MINUS);

        case '/':
            if (match('='))
            {
                scanner.returnNext = TOKEN_SLASH;
                return makeToken(TOKEN_EQUAL);
            }
            else return makeToken(TOKEN_SLASH);

        case '*':
            if (match('='))
            {
                scanner.returnNext = TOKEN_STAR;
                return makeToken(TOKEN_EQUAL);
            }
            else return makeToken(TOKEN_STAR);

        case '&':
            if(match('&'))
            {
                return makeToken(TOKEN_AND);
            }
            else
            {
                return errorToken("Expected '&' after '&'.");
            }

        case '|':
            if(match('|'))
            {
                return makeToken(TOKEN_OR);
            }
            else
            {
                return errorToken("Expected '|' after '|'.");
            }


        case '!':
            return makeToken(
                    match('=') ? TOKEN_BANG_EQUAL : TOKEN_BANG);
        case '=':
            return makeToken(
                    match('=') ? TOKEN_EQUAL_EQUAL : TOKEN_EQUAL);
        case '<':
            return makeToken(
                    match('=') ? TOKEN_LESS_EQUAL : TOKEN_LESS);
        case '>':
            return makeToken(
                    match('=') ? TOKEN_GREATER_EQUAL : TOKEN_GREATER);

        case '"': return string();

        default:
            printf("Reached unreachable.");
            break;
    }



    return errorToken("Unexpected character.");
}
