cmake_minimum_required(VERSION 3.16.3)
project(Wally)

set(CMAKE_CXX_STANDARD 14)

add_executable(Wally src/main.c src/memory.c src/chunk.c src/debug.c src/value.c)
target_include_directories(Wally PRIVATE include/)
