#pragma once

#include <src/chunk.h>
#include <src/table.h>
#include <src/value.h>

#define STACK_MAX 256

typedef struct VM {
    Chunk* chunk;
    uint8_t* ip;
    Value stack[STACK_MAX];
    Value* stack_top;
    Table strings;
    Obj* objects;
} VM;

extern VM vm;

typedef enum InterpretResult {
    INTERPRET_OK,
    INTERPRET_COMPILE_ERROR,
    INTERPRET_RUNTIME_ERROR,
} InterpretResult;

void init_vm();
void free_vm();
InterpretResult interpret(const char* source);
void push(Value value);
Value pop();
