#include "fixed_stack_alloc.h"


void destroy_fixed_stack(fixed_stack instack) {
    destroy_fixed_stack_with(instack, fast_alloc_free, NULL);
}

void destroy_fixed_stack_with(fixed_stack instack, free_fn_type freefn, void *params) {
    if (instack.data_block != NULL) {
        freefn(instack.data_block, params);
    }
}

fixed_stack create_fixed_stack(size_t unit_size, size_t unit_num) {
    return create_fixed_stack_with(unit_size,
                                   unit_num,
                                   fast_alloc_malloc,
                                   NULL);
}

fixed_stack create_fixed_stack_with(size_t unit_size,
                                    size_t unit_num,
                                    alloc_fn_type allocfn,
                                    void *params) {
    fixed_stack retstack;
    retstack.data_size = pad_size(unit_size);
    retstack.data_block = allocfn(retstack.data_size * unit_num, params);
    retstack.data_end = (char *)retstack.data_block + retstack.data_size * unit_num;
    retstack.curdata = retstack.data_block;
    return retstack;
}
