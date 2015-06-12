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
    struct slab *partial, *empty, *full;
    size_t data_size;
    size_t unit_num;
    
    alloc_fn_type alloc;
    void *alloc_params;

} unfixed_block;

void *block_alloc(unfixed_block *inblock);

#endif
