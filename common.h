#ifndef FAST_ALLOC_DEFINES_H
#define FAST_ALLOC_DEFINES_H
#include <stddef.h>

/**
 * A function prototype for an arbitrary allocator
 */
typedef void *(*alloc_fn_type)(size_t, void *);

/**
 * A function prototype for an arbitrary free function
 */
typedef void (*free_fn_type)(void *, void *);

//!Wrapper for free
void fast_alloc_free(void *, void *);

//!Wrapper for malloc
void *fast_alloc_malloc(size_t, void *);

//!Pads size to word boundary (sizeof(void *))
size_t pad_size(size_t initial);



#define FAST_ALLOC_PREDICT(x) (x)
#define FAST_ALLOC_PREDICT_NOT(x) !(x)

#endif
