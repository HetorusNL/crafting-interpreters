#pragma once

#include <src/chunk.h>

void disassemble_chunk(Chunk* chunk, const char* name);
int disassemble_instruction(Chunk* chunk, int offset);
