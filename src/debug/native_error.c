#include <stdio.h>
#include <stdarg.h>
#include "native_error.h"


void nativeError(const char* format, ...)
{
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    fputs("\n", stderr);
}