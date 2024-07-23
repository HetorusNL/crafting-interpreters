#pragma once

#include <src/common.h>
#include <src/value.h>

#define OBJ_TYPE(value) (AS_OBJ(value)->type)

#define IS_STRING(value) is_obj_type(value, OBJ_STRING)

#define AS_STRING(value) ((ObjString*)AS_OBJ(value))
#define AS_CSTRING(value) (AS_STRING(value)->chars)

typedef enum ObjType {
    OBJ_STRING,
} ObjType;

struct Obj {
    ObjType type;
    Obj* next;
};

struct ObjString {
    Obj obj;
    int length;
    char* chars;
};

ObjString* take_string(char* chars, int length);
ObjString* copy_string(const char* chars, int length);

void print_object(Value value);

static inline bool is_obj_type(Value value, ObjType type) { return IS_OBJ(value) && AS_OBJ(value)->type == type; }
