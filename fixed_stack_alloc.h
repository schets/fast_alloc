#ifndef FIXED_STACK_ALLOC_H
#define FIXED_STACK_ALLOC_H
#include "common.h"

#include <stddef.h>

typedef struct fixed_stack {
    void *data_block;
    void *data_end;
    void *curdata;
    size_t data_size;
} fixed_stack;

/**
 * Creates a fixed size stack allocator containing unit_num of
 * unit_size sized units.
 *
 * @param unit_size The size of each element of the stack
 * @param unit_num The number of units held in the stack
 *
 * @return A fixed_stack allocator object
 */ 
fixed_stack create_fixed_stack(size_t unit_size, size_t unit_num);

/**
 * Creates a fixed size stack allocator containing unit_num of
 * unit_size sized units. Memory is allocated using the specified
 * allocator
 *
 * @param unit_size The size of each element of the stack
 * @param unit_num The number of units held in the stack
 * @param allocfn The function used to allocate memory
 * @param params The parameters passed to the allocation function 
 *
 * @return A fixed_stack allocator object
 */ 
fixed_stack create_fixed_stack_with(size_t unit_size,
                                    size_t unit_num,
                                    alloc_fn_type allocfn,
                                    void *params);

void destroy_fixed_stack_with(fixed_stack instack, free_fn_type freefn, void *params);
void destroy_fixed_stack(fixed_stack instack);

void *fixed_stack_alloc(fixed_stack *instack);

void fixed_stack_pop(fixed_stack *instack);

#endif
