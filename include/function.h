#pragma once
#include <stddef.h>

#define MAX_NAME_LEN 32
#define FUNCTION_CAPACITY 1024

typedef void* (*function)();

typedef struct {
    function func;
    char name[MAX_NAME_LEN];
} record_func_t;

// register a new function
function new_func(function source, const char* name);

// get a function by name
function get_func(const char* name);

// optional cleanup
void free_function_registry();
