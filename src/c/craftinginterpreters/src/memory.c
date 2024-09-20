#include <stdlib.h>

#include <src/compiler.h>
#include <src/memory.h>
#include <src/table.h>
#include <src/vm.h>

#ifdef DEBUG_LOG_GC
#include <stdio.h>

#include <src/debug.h>
#endif

#define GC_HEAP_GROW_FACTOR 2

void* reallocate(void* pointer, size_t old_size, size_t new_size) {
    vm.bytes_allocated += new_size - old_size;

#ifdef DEBUG_LOG_GC
    printf("%p reallocate (size %zu -> %zu)\n", pointer, old_size, new_size);
#endif

    if (new_size > old_size) {
#ifdef DEBUG_STRESS_GC
        collect_garbage();
#endif
    }

    if (vm.bytes_allocated > vm.next_gc)
        collect_garbage();

    if (new_size == 0) {
        free(pointer);
        return NULL;
    }

    void* result = realloc(pointer, new_size);
    if (result == NULL)
        exit(1);
    return result;
}

void mark_object(Obj* object) {
    if (object == NULL)
        return;
    if (object->is_marked)
        return;

#ifdef DEBUG_LOG_GC
    printf("%p mark ", (void*)object);
    print_value(OBJ_VAL(object));
    printf("\n");
#endif

    object->is_marked = true;

    // increase the size of the gray stack if it cannot hold any more objects
    if (vm.gray_capacity < vm.gray_count + 1) {
        vm.gray_capacity = GROW_CAPACITY(vm.gray_capacity);
        vm.gray_stack = (Obj**)realloc(vm.gray_stack, sizeof(Obj*) * (size_t)(vm.gray_capacity));

        if (vm.gray_stack == NULL)
            exit(1);
    }

    // add the currently marked item to the gray stack
    vm.gray_stack[vm.gray_count++] = object;
}

void mark_value(Value value) {
    if (IS_OBJ(value))
        mark_object(AS_OBJ(value));
}

void mark_array(ValueArray* array) {
    for (int i = 0; i < array->count; i++)
        mark_value(array->values[i]);
}

static void blacken_instance(ObjInstance* instance) {
    mark_object((Obj*)instance->klass);
    mark_table(&instance->fields);
}

static void blacken_function(ObjFunction* function) {
    mark_object((Obj*)function->name);
    mark_array(&function->chunk.constants);
}

static void blacken_closure(ObjClosure* closure) {
    mark_object((Obj*)closure->function);
    for (int i = 0; i < closure->upvalue_count; i++)
        mark_object((Obj*)closure->upvalues[i]);
}

static void blacken_class(ObjClass* class) { mark_object((Obj*)class->name); }

static void blacken_object(Obj* object) {
#ifdef DEBUG_LOG_GC
    printf("%p blacken ", (void*)object);
    print_value(OBJ_VAL(object));
    printf("\n");
#endif

    switch (object->type) {
    case OBJ_NATIVE:
    case OBJ_STRING:
        break;
    case OBJ_UPVALUE:
        mark_value(((ObjUpvalue*)object)->closed);
        break;
    case OBJ_INSTANCE:
        blacken_instance((ObjInstance*)object);
        break;
    case OBJ_FUNCTION:
        blacken_function((ObjFunction*)object);
        break;
    case OBJ_CLOSURE:
        blacken_closure((ObjClosure*)object);
        break;
    case OBJ_CLASS:
        blacken_class((ObjClass*)object);
        break;
    }
}

static void free_class(Obj* object) { FREE(ObjClass, object); }

static void free_closure(Obj* object) {
    ObjClosure* closure = (ObjClosure*)object;
    FREE_ARRAY(ObjUpvalue*, closure->upvalues, closure->upvalue_count);
    FREE(ObjClosure, object);
}

static void free_function(Obj* object) {
    ObjFunction* function = (ObjFunction*)object;
    free_chunk(&function->chunk);
    FREE(ObjFunction, object);
}

static void free_instance(Obj* object) {
    ObjInstance* instance = (ObjInstance*)object;
    free_table(&instance->fields);
    FREE(OBJ_INSTANCE, object);
}

static void free_string(Obj* object) {
    ObjString* string = (ObjString*)object;
    FREE_ARRAY(char, string->chars, string->length + 1);
    FREE(ObjString, object);
}

static void free_object(Obj* object) {
#ifdef DEBUG_LOG_GC
    printf("%p free type %d\n", (void*)object, object->type);
#endif

    switch (object->type) {
    case OBJ_CLASS:
        free_class(object);
        break;
    case OBJ_CLOSURE:
        free_closure(object);
        break;
    case OBJ_FUNCTION:
        free_function(object);
        break;
    case OBJ_INSTANCE:
        free_instance(object);
        break;
    case OBJ_NATIVE:
        FREE(ObjNative, object);
        break;
    case OBJ_STRING:
        free_string(object);
        break;
    case OBJ_UPVALUE:
        FREE(ObjUpvalue, object);
        break;
    }
}

static void mark_roots() {
    for (Value* slot = vm.stack; slot < vm.stack_top; slot++)
        mark_value(*slot);

    for (int i = 0; i < vm.frame_count; i++)
        mark_object((Obj*)vm.frames[i].closure);

    for (ObjUpvalue* upvalue = vm.open_upvalues; upvalue != NULL; upvalue = upvalue->next)
        mark_object((Obj*)upvalue);

    mark_table(&vm.globals);
    mark_compiler_roots();
}

static void trace_references() {
    while (vm.gray_count > 0) {
        Obj* object = vm.gray_stack[--vm.gray_count];
        blacken_object(object);
    }
}

static void sweep() {
    Obj* previous = NULL;
    Obj* object = vm.objects;
    while (object != NULL) {
        if (object->is_marked) {
            object->is_marked = false;
            previous = object;
            object = object->next;
        } else {
            Obj* unreachable = object;
            object = object->next;
            if (previous == NULL)
                vm.objects = object;
            else
                previous->next = object;

            printf("%p reap ", (void*)object);
            print_value(OBJ_VAL(object));
            printf("\n");

            free_object(unreachable);
        }
    }
}

void collect_garbage() {
#ifdef DEBUG_LOG_GC
    printf("-- gc begin\n");
    size_t before = vm.bytes_allocated;
#endif

    mark_roots();
    trace_references();
    table_remove_white(&vm.strings);
    sweep();

    vm.next_gc = vm.bytes_allocated * GC_HEAP_GROW_FACTOR;

#ifdef DEBUG_LOG_GC
    printf("-- gc end\n");
    printf("   collected %zu bytes (from %zu to %zu) next at %zu\n", before - vm.bytes_allocated, before,
           vm.bytes_allocated, vm.next_gc);
#endif
}

void free_objects() {
    Obj* object = vm.objects;
    while (object != NULL) {
        Obj* next = object->next;
        free_object(object);
        object = next;
    }

    free(vm.gray_stack);
}
