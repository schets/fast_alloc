#define FAST_ALLOC_IMPL

#include "fixed_block_alloc.h"
#include <stdlib.h>

//!assures block size is aligned to sizeof(void *)
static inline size_t block_size(size_t initial) {
    if (initial <= sizeof(void *))
        return sizeof (void *);
    if ((initial % sizeof(void *)) == 0)
        return initial;
    return sizeof(void *) - (initial % sizeof(void *));
}

    
fixed_block create_fixed_block(size_t unit_size,
                               size_t num_units) {
    return create_fixed_block_with(unit_size, num_units, fast_alloc_malloc, 0);
}

void destoy_fixed_block(fixed_block inblock) {
    destroy_fixed_block_with(inblock, fast_alloc_free, 0);
}

fixed_block create_fixed_block_with(size_t unit_size,
                                    size_t num_units,
                                    alloc_fn_type alloc,
                                    void *params) {

    size_t data_size = block_size(unit_size);
    void *block = alloc(data_size * num_units, params);
    
    fixed_block data;
    data.data_blocks = block;
    data.first_open = data.data_blocks;
    data.data_size = data_size;

    if (block != NULL) {

#ifdef FAST_ALLOC_DEBUG
        fixed_block.num_elements = num_units;
#endif

        void *start = block;
        for(size_t i = 0; i < num_units - 1; i++) {
            void *next_open = GET_NEXT_BLOCK(start, data_size);
            SET_NEXT_BLOCK(start, next_open);
            start = next_open;
        }
        SET_NEXT_BLOCK(start, NULL);
    }
    return data;
}

void destroy_fixed_block_with(fixed_block inblock, free_fn_type freefn, void *params) {
    if(inblock.data_blocks != NULL) {
        freefn(inblock.data_blocks, params);
    }
}
