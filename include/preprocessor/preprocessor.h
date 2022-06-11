#ifndef WALLY_PREPROCESSOR_H
#define WALLY_PREPROCESSOR_H

#include "common.h"

typedef enum
{
    DIRECTIVE_INCLUDE,
    DIRECTIVE_DEFINE,
    DIRECTIVE_UNDEF,
    DIRECTIVE_IFDEF,
    DIRECTIVE_IFNDEF,
    DIRECTIVE_ERROR,
    DIRECTIVE_FILE,
    DIRECTIVE_LINE,
    DIRECTIVE_SYMBOL
} DirectiveType;

typedef struct {
    const char* start;
    const char* current;
    uint line;
} Preprocessor;

Preprocessor preprocessor;

#endif
