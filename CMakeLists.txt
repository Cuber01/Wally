cmake_minimum_required(VERSION 3.21) # I guess you shouldn't touch the version
project(Wally)

set(CMAKE_CXX_STANDARD 14)

if (DEBUG_ENABLED)
    add_definitions(-DDEBUG_PRINT_BYTECODE=1 -DDEBUG_TRACE_EXECUTION=1)
endif (DEBUG_ENABLED)

if(TOKEN_PRINT_ENABLED)
    add_definitions(-DEBUG_PRINT_TOKENS=1)
endif(TOKEN_PRINT_ENABLED)

include_directories(Wally PRIVATE include/ include/data_structs/ include/misc/ include/debug/
                    include/memory/ include/scanner/ include/parser include/vm/)
add_executable(Wally src/main.c
                     src/memory/memory.c
                     src/data_structs/chunk.c
                     src/debug/disassembler.c
                     src/data_structs/value.c
                     src/vm/vm.c
                     src/scanner/scanner.c
                     src/parser/compiler.c
                     src/debug/token_printer.c
                     src/data_structs/object.c
                     src/data_structs/table.c
        src/data_structs/ast_tree.c)