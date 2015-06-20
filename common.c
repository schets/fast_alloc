#include "common.h"
#include <stdlib.h>
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
size_t pad_size_to(size_t initial, size_t align) {
    if (initial <= align)
        return align;
    if ((initial % align) == 0)
        return initial;
    return initial + align - (initial % align);
}

size_t pad_size(size_t initial) {
    return pad_size_to(initial, 8);
}
