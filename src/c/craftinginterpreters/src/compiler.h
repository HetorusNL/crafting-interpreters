#pragma once

#include <src/chunk.h>
#include <src/object.h>
#include <src/vm.h>

bool compile(const char* source, Chunk* chunk);
