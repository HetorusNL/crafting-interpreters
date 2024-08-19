#pragma once

#include <src/chunk.h>
#include <src/object.h>
#include <src/vm.h>

ObjFunction* compile(const char* source);
