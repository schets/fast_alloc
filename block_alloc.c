#define BLOCK_IMPL

#include "block_alloc.h"
#include <stdlib.h>

void *fast_alloc_malloc(size_t size, void* params) {
    params = 0; //remove unused variable compiler warnings
    return malloc(size);
}

void fast_alloc_free(void* tofree, void * params) {
    params = 0; //remove unused variable compiler warnings
    free(tofree);
}
    
fixed_block alloc_fixed_block(size_t unit_size,
                              size_t num_units) {
    return alloc_fixed_block_with(unit_size, num_units, fast_alloc_malloc, 0);
}

void free_fixed_block(fixed_block inblock) {
    free_fixed_block_with(inblock, fast_alloc_free, 0);
}

fixed_block alloc_fixed_block_with(size_t unit_size,
                                   size_t num_units,
                                   alloc_fn_type alloc,
                                   void *params) {

    void *block = alloc(FIXED_BLOCK_SIZE(unit_size) * num_units, params);
    
    fixed_block data;
    data.data_blocks = block;
    data.first_open = data.data_blocks;
    data.data_size = unit_size;

    if (FAST_ALLOC_PREDICT(block)) {

#ifdef FAST_ALLOC_DEBUG
        fixed_block.num_elements = num_units;
#endif

        void *start = block;
        for(size_t i = 0; i < num_units - 1; i++) {
            void *next_open = GET_NEXT_BLOCK(start, unit_size);
            SET_NEXT_BLOCK(start, next_open);
            start = next_open;
        }
    }
    return data;
}

void free_fixed_block_with(fixed_block inblock, free_fn_type freefn, void *params) {
    if(inblock.data_blocks) {
        freefn(inblock.data_blocks, params);
    }
}
