#ifndef BLOCK_ALLOC_H
#define BLOCK_ALLOC_H
#include "../common.h"
#include <stdint.h>

struct alloc_type {
    void *(*malloc)(struct alloc_type *, size_t size);
    void *(*malloc_hint)(struct alloc_type *, void *hint, size_t size);
    void (*free)(struct alloc_type *, void *);
};

struct unfixed_block {
    struct slab *partial, *full;
    size_t alloc_num;
    size_t data_size;
    size_t unit_num;
    struct alloc_type *allocator;
};

extern struct alloc_type *default_alloc;

void *block_alloc(struct unfixed_block *inblock);
void block_free(struct unfixed_block *inblock, void *ptr);

struct unfixed_block create_unfixed_block(size_t unit_size, size_t unit_num);
struct unfixed_block create_unfixed_block_with(size_t unit_size, size_t unit_num, struct alloc_type *alloc);
void destroy_unfixed_block(struct unfixed_block *blk);
#endif
