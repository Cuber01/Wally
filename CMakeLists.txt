cmake_minimum_required(VERSION 3.21) # I guess you shouldn't touch the version
project(Wally)

set(CMAKE_CXX_STANDARD 14)

include_directories(Wally PRIVATE include/ include/data/ include/misc/ include/debug/)
add_executable(Wally src/main.c
                     src/memory.c
                     src/data/chunk.c
                     src/debug/disassembler.c
                     src/data/value.c
                     src/vm.c
                     src/scanner.c
                     src/compiler.c
                     src/debug/token_printer.c
                     src/data/object.c
                     src/data/table.c
        )