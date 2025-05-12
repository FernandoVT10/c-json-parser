#ifndef ARENA_H
#define ARENA_H

#define ARENA_REGION_CAPACITY (8*1024)

typedef struct Region Region;

struct Region {
    void *data;
    size_t count;
    size_t capacity;
    Region *next;
};

typedef struct {
    Region *head, *tail;
} Arena;

void *arena_alloc(Arena *arena, size_t bytes);
void arena_free(Arena *arena);

#endif // ARENA_H
