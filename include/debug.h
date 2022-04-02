#ifndef WALLY_DEBUG_H
#define WALLY_DEBUG_H

#include "chunk.h"

void disassembleChunk(Chunk* chunk, const char* name);
int disassembleInstruction(Chunk* chunk, int offset);


#endif //WALLY_DEBUG_H
