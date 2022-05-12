#include <stdio.h>
#include <stdarg.h>
#include "colors.h"

void colorWriteline(const char* colorCode, const char* format, ...)
{
    va_list args;
    fprintf( stdout, "%s", colorCode );
    va_start( args, format );
    vfprintf( stdout, format, args );
    va_end( args );
    fprintf( stdout, "\n" );
    fprintf( stdout, COLOR_CLEAR);
}

void colorWrite(const char* colorCode, const char* format, ...)
{
    va_list args;
    fprintf( stdout, "%s", colorCode );
    va_start( args, format );
    vfprintf( stdout, format, args );
    va_end( args );
    fprintf( stdout, COLOR_CLEAR);
}

void stderrPrint(const char* format, ... )
{
    va_list args;
    fprintf( stderr, RED);
    va_start( args, format );
    vfprintf( stderr, format, args );
    va_end( args );
    fprintf( stderr, "\n" );
}