#ifndef BLOCK_ALLOC_H
#define BLOCK_ALLOC_H
#include "common.h"
#include <stdint.h>
/*
 struct mytype {
 union {
 type_dat next;
 void * next
 }
 uint16_t block_index
 }
 
 */
#define GET_BLOB_DATA(x) (x)
#define SET_NEXT_BLOB(x, newb) *(void **)x = newb;
#define GET_NEXT_BLOB(x) (*(void **)x)
#define SET_BLOCK(x, data_size, inb)  *((char *)x + data_size - sizeof(uint16_t)) = inb;
#define GET_BLOCK(x, data_size) (*((char *)x + data_size - sizeof(uint16_t)))

typedef struct unfixed_block {
    void **slabs;
    size_t data_size;
    
    alloc_fn_type alloc;
    void *alloc_params;

    uint32_t full_end, slab_count;

    void *first_open;
} unfixed_block;

void *swap_slabs(unfixed_block *inblock);

inline void *block_alloc(unfixed_block *inblock) {
    if (FAST_ALLOC_PREDICT_NOT(inblock->first_open == NULL)) {
        return swap_slabs(inblock);
    }
    void *data = GET_BLOB_DATA(inblock->first_open);
    inblock->first_open = GET_NEXT_BLOB(inblock->first_open);
    return data;
}

#ifndef FAST_ALLOC_IMPL
#undef GET_NEXT_BLOCK
#undef SET_NEXT_BLOCK
#undef GET_BLOCK_DATA
#include "undefs.h"
#endif
#endif
