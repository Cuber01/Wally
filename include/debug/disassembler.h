#ifndef WALLY_DISASSEMBLER_H
#define WALLY_DISASSEMBLER_H

#include "chunk.h"

void disassembleChunk(Chunk* chunk, const char* name);
int disassembleInstruction(Chunk* chunk, int offset);

#endif //WALLY_DISASSEMBLER_H
