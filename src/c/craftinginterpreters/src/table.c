#include <stdlib.h>
#include <string.h>

#include <src/memory.h>
#include <src/object.h>
#include <src/table.h>
#include <src/value.h>

void init_table(Table* table) {
    table->count = 0;
    table->capacity = 0;
    table->entries = NULL;
}

void free_table(Table* table) {
    FREE_ARRAY(Entry, table->entries, table->capacity);
    init_table(table);
}

static Entry* find_entry(Entry* entries, int capacity, ObjString* key) {
    uint32_t index = key->hash & (uint32_t)(capacity - 1);
    Entry* tombstone = NULL;
    for (;;) {
        Entry* entry = &entries[index];
        if (entry->key == NULL) {
            if (IS_NIL(entry->value)) {
                // empty entry
                return tombstone == NULL ? entry : tombstone;
            } else {
                // we found a tombstone
                tombstone = entry;
            }
        } else if (entry->key == key) {
            // we found the key
            return entry;
        }

        index = (index + 1) & (uint32_t)(capacity - 1);
    }
}

bool table_get(Table* table, ObjString* key, Value* value) {
    if (table->count == 0)
        return false;

    Entry* entry = find_entry(table->entries, table->capacity, key);
    if (entry->key == NULL)
        return false;

    *value = entry->value;
    return true;
}

static void adjust_capacity(Table* table, int capacity) {
    Entry* entries = ALLOCATE(Entry, capacity);
    for (int i = 0; i < capacity; i++) {
        entries[i].key = NULL;
        entries[i].value = NIL_VAL;
    }

    table->count = 0;
    for (int i = 0; i < table->capacity; i++) {
        Entry* entry = &table->entries[i];
        if (entry->key == NULL)
            continue;

        Entry* dest = find_entry(entries, capacity, entry->key);
        dest->key = entry->key;
        dest->value = entry->value;
        table->count++;
    }

    FREE_ARRAY(Entry, table->entries, table->capacity);
    table->entries = entries;
    table->capacity = capacity;
}

bool table_set(Table* table, ObjString* key, Value value) {
    if (table->count + 1 > table->capacity * TABLE_MAX_LOAD) {
        int capacity = GROW_CAPACITY(table->capacity);
        adjust_capacity(table, capacity);
    }

    Entry* entry = find_entry(table->entries, table->capacity, key);
    bool is_new_key = entry->key == NULL;
    if (is_new_key && IS_NIL(entry->value))
        table->count++;

    entry->key = key;
    entry->value = value;
    return is_new_key;
}

bool table_delete(Table* table, ObjString* key) {
    if (table->count == 0)
        return false;

    // find the entry
    Entry* entry = find_entry(table->entries, table->capacity, key);
    if (entry->key == NULL)
        return false;

    // place a tombstone in the entry
    entry->key = NULL;
    entry->value = BOOL_VAL(true);
    return true;
}

void table_add_all(Table* from, Table* to) {
    for (int i = 0; i < from->capacity; i++) {
        Entry* entry = &from->entries[i];
        if (entry->key != NULL)
            table_set(to, entry->key, entry->value);
    }
}

ObjString* table_find_string(Table* table, const char* chars, int length, uint32_t hash) {
    if (table->count == 0)
        return NULL;

    uint32_t index = hash & (uint32_t)(table->capacity - 1);
    for (;;) {
        Entry* entry = &table->entries[index];
        if (entry->key == NULL) {
            // stop if we find an empty non-tombstone entry
            if (IS_NIL(entry->value))
                return NULL;
        } else if (entry->key->length == length && entry->key->hash == hash) {
            // equal length and hash, only now we need to compare the whole strings
            if (memcmp(entry->key->chars, chars, (size_t)length) == 0)
                return entry->key;
        }

        index = (index + 1) & (uint32_t)(table->capacity - 1);
    }
}

void table_remove_white(Table* table) {
    for (int i = 0; i < table->capacity; i++) {
        Entry* entry = &table->entries[i];
        if (entry->key != NULL && !entry->key->obj.is_marked)
            table_delete(table, entry->key);
    }
}

void mark_table(Table* table) {
    for (int i = 0; i < table->capacity; i++) {
        Entry* entry = &table->entries[i];
        mark_object((Obj*)entry->key);
        mark_value(entry->value);
    }
}
