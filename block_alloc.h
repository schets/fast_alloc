#ifndef FIXED_BLOCK_ALLOC_H
#define FIXED_BLOCK_ALLOC_H
#include <stddef.h>
#include <stdint.h>
#include <assert.h>

//template<class T>
//block layout
//{
//    block_layout<T> * next_block;
//    T element_type;
//}

#define FAST_ALLOC_DEBUG_
#define FAST_ALLOC_PREDICT(x) x
#define FAST_ALLOC_PREDICT_NOT(x) !x

#define FIXED_BLOCK_SIZE(inval) (inval + sizeof(void *))
#define GET_BLOCK_DATA(inblock) (((char *)inblock) + sizeof(void *))
#define GET_NEXT_BLOCK(inblock, data_size) (((char *) inblock) +    \
                                           FIXED_BLOCK_SIZE(data_size))
#define SET_NEXT_BLOCK(inblock, next) *((void **)inblock) = (void *) next;

typedef void *(*alloc_fn_type)(size_t, void *);
typedef void (*free_fn_type)(void *, void *);

typedef struct fixed_block {
#ifdef FAST_ALLOC_DEBUG_
    size_t num_elements;
#endif
    void *data_blocks;
    void *first_open;
    size_t data_size;
} fixed_block;

fixed_block alloc_fixed_block(size_t unit_size, size_t num_units);
fixed_block alloc_fixed_block_with(size_t unit_size,
                                   size_t num_units,
                                   alloc_fn_type alloc,
                                   void *params);

void free_fixed_block(fixed_block inblock);
void free_fixed_block_with(fixed_block inblock, free_fn_type freefn, void *params);

inline void *fixed_block_alloc(fixed_block *inblock) {
    if (FAST_ALLOC_PREDICT_NOT(inblock->first_open))
        return NULL;
    void *ret_data = GET_BLOCK_DATA(inblock->first_open);
    inblock->first_open = GET_NEXT_BLOCK(inblock->first_open, inblock->data_size);
    return ret_data;
};

inline void fixed_block_free(void *data, fixed_block *inblock) {
    if (FAST_ALLOC_PREDICT(data)) {

#ifdef FAST_ALLOC_DEBUG_
        size_t data_size = FIXED_BLOCK_SIZE(inblock->data_size);
        assert((char*)data <
               ((char*)inblock->data_blocks + inblock->num_elements * data_size));
        assert(data >= inblock->data_blocks);
        assert(((ptrdiff_t)((char*)data - (char*)inblock->data_blocks)
                % (ptrdiff_t)data_size) == 0);
#endif
        SET_NEXT_BLOCK(data, inblock->first_open);
        inblock->first_open = data;
    }
}

#ifndef BLOCK_IMPL
#undef FIXED_BLOCK_SIZE
#undef SET_NEXT_BLOCK
#undef GET_NEXT_BLOCK
#undef GET_BLOCK_DATA
#undef FAST_ALLOC_PREDICT
#undef FAST_ALLOC_PREDICT_NOT
#endif

#endif
