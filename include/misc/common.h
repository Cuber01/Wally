#ifndef WALLY_COMMON_H
#define WALLY_COMMON_H

#define UINT8_COUNT (UINT8_MAX + 1)

// #define DEBUG_TRACE_EXECUTION
// #define DEBUG_PRINT_BYTECODE
// #define DEBUG_PRINT_TOKENS

#define INTERPRET_OK 0
#define INTERPRET_RUNTIME_ERROR 70
#define INTERPRET_COMPILE_ERROR 65

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#endif