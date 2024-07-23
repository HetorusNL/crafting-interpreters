#include <stdlib.h>

#include <src/memory.h>
#include <src/vm.h>

void* reallocate(void* pointer, size_t old_size, size_t new_size) {
    if (new_size == 0) {
        free(pointer);
        return NULL;
    }

    void* result = realloc(pointer, new_size);
    if (result == NULL)
        exit(1);
    return result;
}

static void free_string(Obj* object) {
    ObjString* string = (ObjString*)object;
    FREE_ARRAY(char, string->chars, string->length + 1);
    FREE(ObjString, object);
}

static void free_object(Obj* object) {
    switch (object->type) {
    case OBJ_STRING:
        free_string(object);
        break;
    }
}

void free_objects() {
    Obj* object = vm.objects;
    while (object != NULL) {
        Obj* next = object->next;
        free_object(object);
        object = next;
    }
}
