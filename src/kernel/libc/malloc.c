#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include <cyrillic.h>


extern char _heap_start;
extern char _heap_end;

typedef struct block {
    size_t size;
    struct block* next;
} block_t;

static block_t* free_list = (block_t*)&_heap_start;
static int is_init = 0;

void heap_init() {
    if (is_init == 1) return;
    is_init = 1;
    free_list->size = &_heap_end - (char*)free_list - sizeof(block_t);
    free_list->next = NULL;
}

void* malloc(size_t size) {
    size_t real_size = size;
    size = (size + 7) & ~7;

    heap_init();

    block_t** curr = &free_list;
    while (*curr) {
        if ((*curr)->size >= size + sizeof(block_t) + 8) {
            block_t* new_block = (block_t*)((char*)(*curr) + sizeof(block_t) + size);
            new_block->size = (*curr)->size - size - sizeof(block_t);
            new_block->next = (*curr)->next;

            void* ptr = (char*)(*curr) + sizeof(block_t);

            // Replace current block in free list with new_block
            *curr = new_block;

            return ptr;
        } else {
            void* ptr = (char*)(*curr) + sizeof(block_t);
            *curr = (*curr)->next;
            return ptr;
        }
        curr = &(*curr)->next;
    }

    DEBUG_PRINT("[malloc] failed to allocate buffer of real size %u bytes, size %u bytes\n", real_size, size);
    return NULL; // Out of memory
}

void free(void* ptr) {
    if (!ptr) return;
    block_t* block = (block_t*)((char*)ptr - sizeof(block_t));
    DEBUG_PRINT("[malloc] freed %u byte big buffer\n", block->size);
    block->next = free_list;
    free_list = block;
}

void* calloc(size_t num, size_t size) {
    size_t total = num * size;
    void* ptr = malloc(total);
    if (!ptr) return NULL;
    memset(ptr, 0, total);
    return ptr;
}

void* realloc(void* ptr, size_t new_size) {
    if (!ptr) return malloc(new_size);   // behave like malloc
    if (new_size == 0) {                 // behave like free
        free(ptr);
        return NULL;
    }

    void* new_ptr = malloc(new_size);
    if (!new_ptr) return NULL;

    memcpy(new_ptr, ptr, new_size);
    free(ptr);
    return new_ptr;
}