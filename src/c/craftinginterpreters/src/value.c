#include <stdio.h>
#include <string.h>

#include <src/memory.h>
#include <src/object.h>
#include <src/value.h>

void init_value_array(ValueArray* array) {
    array->capacity = 0;
    array->count = 0;
    array->values = NULL;
}

void free_value_array(ValueArray* array) {
    FREE_ARRAY(Value, array->values, array->capacity);
    init_value_array(array);
}

void write_value_array(ValueArray* array, Value value) {
    if (array->capacity < array->count + 1) {
        int old_capacity = array->capacity;
        array->capacity = GROW_CAPACITY(old_capacity);
        array->values = GROW_ARRAY(Value, array->values, old_capacity, array->capacity);
    }

    array->values[array->count] = value;
    array->count++;
}

void print_value(Value value) {
    switch (value.type) {
    case VAL_BOOL:
        printf(AS_BOOL(value) ? "true" : "false");
        break;
    case VAL_NIL:
        printf("nil");
        break;
    case VAL_NUMBER:
        printf("%g", AS_NUMBER(value));
        break;
    case VAL_OBJ:
        print_object(value);
        break;
    }
}

static inline bool obj_string_equal(Value a, Value b) {
    ObjString* string_a = AS_STRING(a);
    ObjString* string_b = AS_STRING(b);
    return string_a->length == string_b->length &&
           memcmp(string_a->chars, string_b->chars, (size_t)string_a->length) == 0;
}

bool values_equal(Value a, Value b) {
    if (a.type != b.type)
        return false;
    switch (a.type) {
    case VAL_BOOL:
        return AS_BOOL(a) == AS_BOOL(b);
    case VAL_NIL:
        return true;
    case VAL_NUMBER:
        return AS_NUMBER(a) == AS_NUMBER(b);
    case VAL_OBJ:
        return obj_string_equal(a, b);
    default:
        // unreachable
        return false;
    }
}
