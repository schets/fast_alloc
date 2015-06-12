#include "common.h"
#include <jemalloc/jemalloc.h>

//!malloc wrapper
void *fast_alloc_malloc(size_t size, void *params) {
    params = 0; //remove unused variable compiler warnings
    return malloc(size);
}

//!free wrapper
void fast_alloc_free(void* tofree, void *params) {
    params = 0; //remove unused variable compiler warnings
    free(tofree);
}

//!assures block size is aligned to sizeof(void *)
size_t pad_size(size_t initial) {
    if (initial <= sizeof(void *))
        return sizeof (void *);
    if ((initial % sizeof(void *)) == 0)
        return initial;
    return sizeof(void *) - (initial % sizeof(void *));
}
