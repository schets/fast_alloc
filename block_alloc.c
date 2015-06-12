#define FAST_ALLOC_IMPL

#include "block_alloc.h"

static size_t get_proper_size(size_t init_size) {
    size_t blob_size = pad_size_to(init_size, sizeof(uint16_t));
    blob_size = blob_size < sizeof(void *) ? sizeof(void *) : blob_size; 
    return pad_size(blob_size + sizeof(uint16_t));
}

static void* create_slab_with(size_t blob_size,
                              uint32_t num_blobs,
                              uint16_t block,
                              alloc_fn_type alloc,
                              void *params) {
    void *data_blob = alloc(blob_size * num_blobs, params);
    if (FAST_ALLOC_PREDICT(data_blob != NULL)) {
        void *cur_blob = data_blob;
        for(uint32_t i = 0; i < num_blobs - 1; i++) {
            SET_BLOCK(cur_blob, blob_size, block);
            SET_NEXT_BLOB(cur_blob, (char *)cur_blob + blob_size);
            cur_blob = cur_blob + blob_size;
        }
        SET_NEXT_BLOB(cur_blob, NULL);
    }

    return data_blob;
}
