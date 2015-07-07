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
    void *first_open;
    struct slab *partial;
    size_t data_size;
    size_t unit_num;
};

extern struct alloc_type *default_alloc;

void *block_alloc(struct unfixed_block *inblock);
void *block_alloc_hint(struct unfixed_block *inblock, void *hint);
void block_free(struct unfixed_block *inblock, void *ptr);

struct unfixed_block create_unfixed_block(size_t unit_size, size_t unit_num);
void destroy_unfixed_block(struct unfixed_block *blk);
#endif
