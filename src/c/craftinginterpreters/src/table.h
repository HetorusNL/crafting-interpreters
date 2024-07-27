#pragma once

#include <src/common.h>
#include <src/value.h>

#define TABLE_MAX_LOAD 0.75

typedef struct Entry {
    ObjString* key;
    Value value;
} Entry;

typedef struct Table {
    int count;
    int capacity;
    Entry* entries;
} Table;

void init_table(Table* table);
void free_table(Table* table);
bool table_get(Table* table, ObjString* key, Value* value);
bool table_set(Table* table, ObjString* key, Value value);
bool table_delete(Table* table, ObjString* key);
void table_add_all(Table* from, Table* to);
ObjString* table_find_string(Table* table, const char* chars, int length, uint32_t hash);
