cmake_minimum_required(VERSION 3.16.3) # I guess you shouldn't touch the version
# Release flags: -O3 -DNDEBUG
# Debug flags: -g

project(Wally)

if (DEBUG_ENABLED)
    add_definitions(-DDEBUG_PRINT_BYTECODE=1 -DDEBUG_TRACE_EXECUTION=1)
endif(DEBUG_ENABLED)

if(TOKEN_PRINT_ENABLED)
    add_definitions(-DEBUG_PRINT_TOKENS=1)
endif(TOKEN_PRINT_ENABLED)

if(NO_OPTIMIZE)
    set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -O0")
endif()

if(EMSCRIPTEN)
    set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -s EXIT_RUNTIME")
endif()

include_directories(Wally PRIVATE include/ include/data_structs/ include/misc/ include/debug/
                    include/memory/ include/scanner/ include/parser include/vm/ include/emitter
                    include/std/ include/preprocessor/)

if(BUILD_LIBRARY)
    add_compile_definitions(LIBRARY)

    add_library(Wally STATIC
                src/main.c
                src/memory/memory.c
                src/data_structs/chunk.c
                src/debug/disassembler.c
                src/data_structs/value.c
                src/vm/vm.c
                src/scanner/scanner.c
                src/parser/parser.c
                src/debug/token_printer.c
                src/data_structs/object.c
                src/data_structs/table.c
                src/parser/ast_tree.c
                src/data_structs/list.c
                src/emitter/emitter.c
                src/emitter/environment.c
                src/std/core.c
                src/memory/garbage_collector.c
                src/misc/colors.c
                src/debug/allocation_logger.c
                src/data_structs/array.c
                src/preprocessor/preprocessor.c
                src/std/native_utils.c
                src/std/wally_math.c
                src/std/wally_os.c
                src/std/wally_random.c
                )
else()
    add_executable(Wally
              src/main.c
              src/memory/memory.c
              src/data_structs/chunk.c
              src/debug/disassembler.c
              src/data_structs/value.c
              src/vm/vm.c
              src/scanner/scanner.c
              src/parser/parser.c
              src/debug/token_printer.c
              src/data_structs/object.c
              src/data_structs/table.c
              src/parser/ast_tree.c
              src/data_structs/list.c
              src/emitter/emitter.c
              src/emitter/environment.c
              src/std/core.c
              src/memory/garbage_collector.c
              src/misc/colors.c
              src/debug/allocation_logger.c
              src/data_structs/array.c
              src/preprocessor/preprocessor.c
              src/std/native_utils.c
              src/std/wally_math.c
              src/std/wally_os.c
              src/std/wally_random.c
            src/std/wally_list.c)
endif(BUILD_LIBRARY)

target_link_libraries(Wally m)
