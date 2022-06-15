#ifndef WALLY_PREPROCESSOR_H
#define WALLY_PREPROCESSOR_H

#include "common.h"
#include "table.h"

typedef enum
{
    DIRECTIVE_NONE,
    DIRECTIVE_INCLUDE,
    DIRECTIVE_DEFINE,
    DIRECTIVE_UNDEF,
    DIRECTIVE_IFDEF,
    DIRECTIVE_IFNDEF,
    DIRECTIVE_ERROR,
//    DIRECTIVE_FILE,
//    DIRECTIVE_LINE,
} DirectiveType;


typedef struct {
    Table* defines;

    char* start;
    char* current;
    uint line;
} Preprocessor;

Preprocessor preprocessor;

char* preprocess(char* text);

#endif
