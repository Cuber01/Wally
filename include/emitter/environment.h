#ifndef WALLY_ENVIRONMENT_H
#define WALLY_ENVIRONMENT_H

#include "table.h"

typedef struct Environment
{
    Table* values;
} Environment;

#endif //WALLY_ENVIRONMENT_H
