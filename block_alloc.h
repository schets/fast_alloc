#ifndef BLOCK_ALLOC_H
#define BLOCK_ALLOC_H
#include "common.h"
#include <stdint.h>
/*
 struct mytype {
 union {
 type_dat next;
 void *next;
 }
 slab *block_index;
 }
 
 */

struct slab;

typedef struct unfixed_block {
    struct slab *partial, *full, *empty;
    size_t data_size;
    size_t unit_num;
    
    alloc_fn_type alloc;
    void *alloc_params;

} unfixed_block;

void *block_alloc(unfixed_block *inblock);
void block_free(unfixed_block *inblock, void *ptr);

unfixed_block create_unfixed_block(size_t unit_size, size_t unit_num);
void destroy_unfixed_block(unfixed_block *blk);
#endif
