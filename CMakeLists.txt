cmake_minimum_required(VERSION 3.21) # I guess you shouldn't touch the version
project(Wally)

set(CMAKE_CXX_STANDARD 14)

include_directories(Wally PRIVATE include/)
add_executable(Wally src/main.c src/memory.c
                     src/chunk.c src/debug.c
                     src/value.c src/vm.c
                     src/scanner.c src/compiler.c
        src/token_printer.c)