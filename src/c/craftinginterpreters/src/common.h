#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// if defined, prints the chunk's bytecode before executing it
#define DEBUG_PRINT_CODE
// if defined, prints the disassembled instruction before executing it
#define DEBUG_TRACE_EXECUTION
// stress test the garbage collector by running it as often as possible
#define DEBUG_STRESS_GC
// print debug information surrounding the garbage collector
#define DEBUG_LOG_GC

#define UINT8_COUNT (UINT8_MAX + 1)
