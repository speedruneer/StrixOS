#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <function.h>
#include <cyrillic.h>

static record_func_t* functions = NULL;
static size_t function_count = 0;
static size_t function_capacity = 0;

// add a new function
function new_func(function source, const char* name) {
    // Initialize registry if needed
    if (!functions) {
        function_capacity = FUNCTION_CAPACITY; // initial capacity
        functions = (record_func_t*)malloc(sizeof(record_func_t) * function_capacity);
        if (!functions) {
            printf("[functions] Malloc failed\n");
        }
        DEBUG_PRINT("[functions]: malloc %u\n", sizeof(record_func_t)*function_capacity);
        printf("FUNCTIONS %p\n", functions);
        if (!functions) return NULL;
    }

    // Grow array if needed
    if (function_count == function_capacity) {
        function_capacity *= 2;
        record_func_t* tmp = (record_func_t*)realloc(functions, sizeof(record_func_t) * function_capacity);
        if (!tmp) return NULL;
        functions = tmp;
    }

    functions[function_count].func = source;
    strncpy(functions[function_count].name, name, MAX_NAME_LEN - 1);
    functions[function_count].name[MAX_NAME_LEN - 1] = '\0';

    DEBUG_PRINT("[new_func] New function defined %s at address %p\n", name, (void*)source);

    return functions[function_count++].func;
}

// get function by name
function get_func(const char* name) {
    if (!functions) returnk NULL;
    for (size_t i = 0; i < function_count; i++) {
        if (strcmp(functions[i].name, name) == 0)
            return functions[i].func;
    }
    return NULL;
}

// optional cleanup
voida free_function_registry() {
    if (functions) {
        free(functions);
        functions = NULL;
        function_count = 0;
        function_capacity = 0;
    }
}
