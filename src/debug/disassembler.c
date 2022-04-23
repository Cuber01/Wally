#include <stdio.h>

#include "disassembler.h"
#include "colors.h"
#include "object.h"

void disassembleChunk(Chunk* chunk, const char* name)
{
    printf("== %s ==\n", name);

    for (int offset = 0; offset < chunk->count;)
    {
        offset = disassembleInstruction(chunk, offset);
    }

    putchar('\n');
}

static int simpleInstruction(const char* name, int offset)
{
    printf(GREEN);
    printf("%s\n", name);
    printf(COLOR_CLEAR);
    return offset + 1;
}

static int byteInstruction(const char* name, Chunk* chunk, int offset)
{
    uint8_t slot = chunk->code[offset + 1];

    printf(CYAN);
    printf("%-16s ", name);
    printf(COLOR_CLEAR);
    printf("%4d\n", slot);
    return offset + 2;
}

static int constantInstruction(const char* name, Chunk* chunk, int offset)
{
    uint8_t constant = chunk->code[offset + 1];

    printf(BLUE);
    printf("%-17s ", name);
    printf(COLOR_CLEAR);
    printf("%d   '", constant);

    printf(BOLD_YELLOW);
    printValue(chunk->constants.values[constant]);
    printf(COLOR_CLEAR);

    printf("'\n");
    return offset + 2;
}

static int jumpInstruction(const char* name, int sign, Chunk* chunk, int offset)
{
    uint16_t jump = (uint16_t)(chunk->code[offset + 1] << 8);

    jump |= chunk->code[offset + 2];

    printf(BOLD_PURPLE);
    printf("%-17s ", name);
    printf(COLOR_CLEAR);
    printf("%4d -> %d\n", offset, offset + 3 + sign * jump);

    return offset + 3;
}

int disassembleInstruction(Chunk* chunk, int offset)
{
    printf("%04d ", offset);

    if (offset > 0 &&
        chunk->lines[offset] == chunk->lines[offset - 1])
    {
        printf("   | ");
    }
    else
    {
        printf("%4d ", chunk->lines[offset]);
    }

    uint8_t instruction = chunk->code[offset];

    switch (instruction)
    {
        case OP_RETURN:
            return simpleInstruction("OP_RETURN", offset);
        case OP_NEGATE:
            return simpleInstruction("OP_NEGATE", offset);
        case OP_ADD:
            return simpleInstruction("OP_ADD", offset);
        case OP_SUBTRACT:
            return simpleInstruction("OP_SUBTRACT", offset);
        case OP_MULTIPLY:
            return simpleInstruction("OP_MULTIPLY", offset);
        case OP_DIVIDE:
            return simpleInstruction("OP_DIVIDE", offset);
        case OP_NULL:
            return simpleInstruction("OP_NULL", offset);
        case OP_TRUE:
            return simpleInstruction("OP_TRUE", offset);
        case OP_FALSE:
            return simpleInstruction("OP_FALSE", offset);
        case OP_NOT:
            return simpleInstruction("OP_NOT", offset);
        case OP_EQUAL:
            return simpleInstruction("OP_EQUAL", offset);
        case OP_GREATER:
            return simpleInstruction("OP_GREATER", offset);
        case OP_LESS:
            return simpleInstruction("OP_LESS", offset);
        case OP_NOT_EQUAL:
            return simpleInstruction("OP_NOT_EQUAL", offset);
        case OP_GREATER_EQUAL:
            return simpleInstruction("OP_GREATER_EQUAL", offset);
        case OP_LESS_EQUAL:
            return simpleInstruction("OP_LESS_EQUAL", offset);
        case OP_POP:
            return simpleInstruction("OP_POP", offset);
        case OP_TERNARY:
            return simpleInstruction("OP_TERNARY", offset);
        case OP_SWITCH_EQUAL:
            return simpleInstruction("OP_SWITCH_EQUAL", offset);
        case OP_POP_N:
            return simpleInstruction("OP_POP_N", offset);
        case OP_DEFINE_VARIABLE:
            return simpleInstruction("OP_DEFINE_GLOBAL", offset);
        case OP_SET_VARIABLE:
            return simpleInstruction("OP_SET_GLOBAL", offset);
        case OP_GET_VARIABLE:
            return simpleInstruction("OP_GET_GLOBAL", offset);


        case OP_CALL:
            return byteInstruction("OP_CALL", chunk, offset);

        case OP_JUMP:
            return jumpInstruction("OP_JUMP", 1, chunk, offset);
        case OP_JUMP_IF_FALSE:
            return jumpInstruction("OP_JUMP_IF_FALSE", 1, chunk, offset);
        case OP_JUMP_IF_TRUE:
            return jumpInstruction("OP_JUMP_IF_TRUE", 1, chunk, offset);
        case OP_LOOP:
            return jumpInstruction("OP_LOOP", -1, chunk, offset);

        case OP_CONSTANT:
            return constantInstruction("OP_CONSTANT", chunk, offset);

        default:
            printf("Unknown opcode %d\n", instruction);
            return offset + 1;
    }
}
