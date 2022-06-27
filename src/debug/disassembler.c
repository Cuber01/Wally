#include <stdio.h>

#include "disassembler.h"
#include "colors.h"
#include "object.h"

void disassembleChunk(Chunk* chunk, const char* name)
{
    printf("== %s ==\n", name);

    for (int offset = 0; offset < chunk->codeCount;)
    {
        offset = disassembleInstruction(chunk, offset);
    }

    putchar('\n');
}

static int simpleInstruction(const char* name, int offset)
{
    colorWriteLine(GREEN, "%s", name);
    return offset + 1;
}

static int invokeInstruction(const char* name, Chunk* chunk, int offset)
{
    uint8_t constant = chunk->code[offset + 1];
    uint8_t argCount = chunk->code[offset + 2];

    colorWrite(CYAN, "%-16s (%d args) %4d '", name, argCount, constant);
    printValue(chunk->constants.values[constant]);
    printf("'\n");

    return offset + 3;
}

static int byteInstruction(const char* name, Chunk* chunk, int offset)
{
    uint8_t slot = chunk->code[offset + 1];

    colorWrite(CYAN, "%-16s ", name);
    printf("%4d\n", slot);
    return offset + 2;
}

static int constantInstruction(const char* name, Chunk* chunk, int offset)
{
    uint8_t constant = chunk->code[offset + 1];

    colorWrite(BLUE, "%-17s ", name);
    printf("%d   '", constant);

    printValue(chunk->constants.values[constant]);

    printf("'\n");
    return offset + 2;
}

static int jumpInstruction(const char* name, int sign, Chunk* chunk, int offset)
{
    uint16_t jump = (uint16_t)(chunk->code[offset + 1] << 8);

    jump |= chunk->code[offset + 2];

    colorWrite(BOLD_PURPLE, "%-17s ", name);
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
        case OP_SCOPE_START:
            return simpleInstruction("OP_SCOPE_START", offset);
        case OP_SCOPE_END:
            return simpleInstruction("OP_SCOPE_END", offset);
        case OP_DEFINE_FUNCTION:
            return simpleInstruction("OP_DEFINE_FUNCTION", offset);
        case OP_DEFINE_CLASS:
            return simpleInstruction("OP_DEFINE_CLASS", offset);
        case OP_DEFINE_METHOD:
            return simpleInstruction("OP_DEFINE_METHOD", offset);
        case OP_INHERIT:
            return simpleInstruction("OP_INHERIT", offset);

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
        case OP_GET_VARIABLE:
            return constantInstruction("OP_GET_VARIABLE", chunk, offset);
        case OP_SET_VARIABLE:
            return constantInstruction("OP_SET_VARIABLE", chunk, offset);
        case OP_GET_PROPERTY:
            return constantInstruction("OP_GET_PROPERTY", chunk, offset);
        case OP_SET_PROPERTY:
            return constantInstruction("OP_SET_PROPERTY", chunk, offset);
        case OP_DEFINE_VARIABLE:
            return constantInstruction("OP_DEFINE_VARIABLE", chunk, offset);
        case OP_DEFINE_ARGUMENT:
            return constantInstruction("OP_DEFINE_ARGUMENT", chunk, offset);
        case OP_GET_BASE:
            return constantInstruction("OP_GET_BASE", chunk, offset);

        case OP_INVOKE:
            return invokeInstruction("OP_INVOKE", chunk, offset);

        case OP_CALL:
            return byteInstruction("OP_CALL", chunk, offset);

        default:
            printf("Unknown opcode %d\n", instruction);
            return offset + 1;
    }
}
