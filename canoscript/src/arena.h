#ifndef ARENA_H
#define ARENA_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>

#ifndef ARENA_INIT_SIZE
#define ARENA_INIT_SIZE 128
#endif

typedef struct Arena {
    struct Arena *next;
    size_t capacity;
    size_t size;
    uint8_t *data;
} Arena;
	

Arena arena_init(size_t capacity);
void *arena_alloc(Arena *arena, size_t size);
void *arena_realloc(Arena *arena, void *old_ptr, size_t old_size, size_t new_size);
void arena_reset(Arena *arena);
void arena_free(Arena *arena);
void arena_print(const Arena *arena);
	
#endif // ARENA_H

#ifdef ARENA_IMPLEMENTATION
#define ARENA_IMPLEMENTATION

#ifndef ARENA_ALLOC
void *custom_malloc(size_t size) {
    void *ptr = malloc(sizeof(uint8_t) * size);
    if(ptr == NULL) {
        fprintf(stderr, "error: buy more ram");
        exit(1);
    }
    return ptr;
}
#define ARENA_ALLOC custom_malloc
#endif // ARENA_ALLOC


Arena arena_init(size_t capacity) {
    void *data = ARENA_ALLOC(sizeof(uint8_t) * capacity);
    Arena arena = {
        .capacity = capacity,
        .size = 0,
        .data = data,
        .next = NULL,
    };
    return arena;
}

void *arena_alloc(Arena *arena, size_t size) {
	if(arena == NULL) return NULL;
    Arena *current = arena;
    while(!(current->size + size <= current->capacity)) {
        if(current->next == NULL) {
            Arena *next = ARENA_ALLOC(sizeof(Arena));
            Arena initted = arena_init(arena->capacity+size);
            memcpy(next, &initted, sizeof(Arena));
            current->next = next;
        }
        current = current->next;
    }

    uint8_t *data = &current->data[current->size];
    current->size += size;
    return data;
}

void *arena_realloc(Arena *arena, void *old_ptr, size_t old_size, size_t new_size) {
    if (new_size <= old_size) return old_ptr;

    void *new_ptr = arena_alloc(arena, new_size);
	if(old_ptr == NULL) return new_ptr;
    uint8_t *new_ptr_char = new_ptr;
    uint8_t *old_ptr_char = old_ptr;

	memcpy(new_ptr_char, old_ptr_char, sizeof(uint8_t)*old_size);

    return new_ptr;
}

void arena_reset(Arena *arena) {
    Arena *current = arena;
    while(current != NULL) {
        current->size = 0;
        current = current->next;
    }
}

void arena_free(Arena *arena) {
    free(arena->data);
    arena->capacity = 0;
    arena->size = 0;
    Arena *current = arena->next;
    while(current != NULL) {
        Arena *tmp = current->next;
        free(current->data);
        free(current);
        current = tmp;
    }
    arena->next = NULL;
}

void arena_print(const Arena *arena) {
    const Arena *current = arena;
    while(current != NULL) {
        printf("capacity: %zu, size: %zu, data ptr: %p -> ", current->capacity, current->size, current->data);
        current = current->next;
    }
    printf("NULL\n");
}
	
#endif // ARENA_IMPEMENATION