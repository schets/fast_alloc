#define FAST_ALLOC_IMPL

#include "fixed_block_alloc.h"
#include <stdlib.h>

#define GET_BLOCK_DATA(inblock) (inblock)
#define GET_NEXT_BLOCK(inblock) (*(void **)inblock)
#define SET_NEXT_BLOCK(inblock, next) *((void **)inblock) = (void *) next;

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

void destroy_fixed_block(fixed_block *inblock) {
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
            void *next_open = (char*)start + data_size;
            SET_NEXT_BLOCK(start, next_open);
            start = next_open;
        }
        SET_NEXT_BLOCK(start, NULL);
    }
    return data;
}

void destroy_fixed_block_with(fixed_block *inblock, free_fn_type freefn, void *params) {
    if(inblock->data_blocks != NULL) {
        freefn(inblock->data_blocks, params);
    }
}

void *fixed_block_alloc(fixed_block *inblock) {
    if (FAST_ALLOC_PREDICT_NOT(inblock->first_open == NULL))
        return NULL;
    void *ret_data = GET_BLOCK_DATA(inblock->first_open);
    inblock->first_open = GET_NEXT_BLOCK(inblock->first_open);
    return ret_data;
};

void fixed_block_free(fixed_block *inblock, void *data) {
    if (FAST_ALLOC_PREDICT(data != NULL)) {

#ifdef FAST_ALLOC_DEBUG_
        size_t data_size = inblock->data_size;
        assert((char*)data <
               ((char*)inblock->data_blocks + inblock->num_elements * data_size));
        assert(data >= inblock->data_blocks);
        assert(((ptrdiff_t)((char*)data - (char*)inblock->data_blocks)
                % (ptrdiff_t)data_size) == 0);
        assert(data != inblock->first_open);
#endif
        SET_NEXT_BLOCK(data, inblock->first_open);
        inblock->first_open = data;
    }
}
