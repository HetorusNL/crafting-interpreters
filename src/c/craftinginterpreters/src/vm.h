#pragma once

#include <src/object.h>
#include <src/table.h>
#include <src/value.h>

#define FRAMES_MAX 64
#define STACK_MAX (FRAMES_MAX * UINT8_COUNT)

typedef struct CallFrame {
    ObjClosure* closure;
    uint8_t* ip;
    Value* slots;
} CallFrame;

typedef struct VM {
    CallFrame frames[FRAMES_MAX];
    int frame_count;

    Value stack[STACK_MAX];
    Value* stack_top;
    Table globals;
    Table strings;
    ObjString* init_string;
    ObjUpvalue* open_upvalues;

    size_t bytes_allocated;
    size_t next_gc;
    Obj* objects;

    int gray_count;
    int gray_capacity;
    Obj** gray_stack;
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
