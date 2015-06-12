#ifndef FIXED_BLOCK_ALLOC_H
#define FIXED_BLOCK_ALLOC_H
#include "common.h"
#include <stddef.h>
#include <assert.h>

/**
 * Contains the data required for a fixed_block allocator
 */
typedef struct fixed_block {
#ifdef FAST_ALLOC_DEBUG_
    //! Number of elements in the block
    size_t num_elements;
#endif
    //! Pointer to the memory block used for allocating
    void *data_blocks;

    //! A pointer to the first open block
    void *first_open;
    size_t data_size;
} fixed_block;

/**
 * Creates a fixed_block allocator using malloc to allocate memory
 * @param unit_size The size of each item being allocated
 * @param num_units The number of units that the allocator can create
 * @return A fixed_block allocator using a malloc-allocated block
 */
fixed_block create_fixed_block(size_t unit_size, size_t num_units);

/**
 * Creates a fixed_block allocator using the passed allocator
 * and parameters to allocate memory
 * @param unit_size The size of each item being allocated
 * @param num_units The number of units that the allocator can create
 * @param alloc The function used to allocate data
 * @param params A pointer to parameters passed to the allocator
 * @return A fixed_block allocator using memory allocated by alloc
 */
fixed_block create_fixed_block_with(size_t unit_size,
                                    size_t num_units,
                                    alloc_fn_type alloc,
                                    void *params);

/**
 * Frees the memory associated with the block using free
 * @param inblock The fixed_block_allocator that is being destroyed
 */
void destroy_fixed_block(fixed_block inblock);

/**
 * Frees the memory associated with the block using the specified deallocator
 * @param inblock The fixed_block_allocator that is being destroyed
 * @param freefn The function used to deallocate the block's memory
 * @param params A pointer to parameter passed to the deallocator
 */
void destroy_fixed_block_with(fixed_block inblock, free_fn_type freefn, void *params);

/**
 * Allocates a unit of memory from the passed block. Return NULL if no
 * memory can be allocated.
 *
 * @param inblock A pointer to the block where memory is being allocated
 *
 * @return A pointer to a unit of memory
 */
void *fixed_block_alloc(fixed_block *inblock);

/**
 * Frees a block of memory that was allocated by the specified
 * allocator. When compiled with FAST_ALLOC_DEBUG defined, this will
 * perform a validity check on the passed pointer and assert if
 * the pointer is invalid. In addition, some basic checks are done
 * on doubly freed pointers in the debug case, although they are far
 * from comprehensive. Doubly freed pointers WILL result in data corruption!
 *
 * Passing a NULL pointer does nothing
 *
 * @params data The pointer to be released from the block
 * @params inblock The block to release the pointer from
 */
void fixed_block_free(fixed_block *inblock, void *data);

#endif
