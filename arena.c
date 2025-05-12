#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include "arena.h"

static Region *create_region() {
    Region *r = malloc(sizeof(Region));
    assert(r != NULL && "Not enough ram :(");
    r->count = 0;
    r->capacity = ARENA_REGION_CAPACITY;
    r->data = malloc(ARENA_REGION_CAPACITY);
    r->next = NULL;
    assert(r->data != NULL && "Not enough ram :(");
    return r;
}

// returns current region
static Region *get_region(Arena *arena) {
    if(arena->tail == NULL) {
        arena->head = arena->tail = create_region();
    }

    return arena->tail;
}

void *arena_alloc(Arena *arena, size_t bytes) {
    Region *region = get_region(arena);

    if(region->count + bytes > region->capacity) {
        Region *new_region = create_region();
        arena->tail = new_region;
        region->next = new_region;
        region = new_region;
    }

    void *ptr = region->data + region->count;
    region->count += bytes;
    return ptr;
}

void arena_free(Arena *arena) {
    Region *region = arena->head;
    while(region != NULL) {
        free(region->data);
        Region *old_region = region;
        region = region->next;
        free(old_region);
    }
}
