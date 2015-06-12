#define FAST_ALLOC_IMPL

#include "block_alloc.h"

#define GET_BLOB_DATA(x) (x)
#define SET_NEXT_BLOB(x, newb) *(void **)x = newb;
#define GET_NEXT_BLOB(x) (*(void **)x)
#define SET_BLOCK(x, data_size, inb)  *(void **)((char *)x + data_size - sizeof(void *)) = inb;
#define GET_BLOCK(x, data_size) (*(void **)((char *)x + data_size - sizeof(void *)))

typedef struct slab {
    struct slab *next, *prev;
    void *data;
    void *first_open;
    size_t num_alloc;
} slab;

static slab *remove_slab (slab *inslab) {
    if (inslab->next == inslab) {
        inslab->next = inslab->prev = NULL;
        return NULL;
    }
    inslab->next->prev = inslab->prev;
    inslab->prev->next = inslab->next;
    return inslab->next;
}

static void add_slab(slab *list, slab *newslab) {
    if (!list)
        newslab->next = newslab->prev = newslab;
    else {
        newslab->next = list;
        newslab->prev = list->prev;
    }
}

static size_t get_proper_size(size_t init_size) {
    size_t blob_size = pad_size_to(init_size, sizeof(uint16_t));
    blob_size = blob_size < sizeof(void *) ? sizeof(void *) : blob_size; 
    return pad_size(blob_size + sizeof(uint16_t));
}

static slab* create_slab_data(size_t blob_size,
                              uint32_t num_blobs,
                              alloc_fn_type alloc,
                              void *params) {
    void *full_blob = alloc(blob_size * num_blobs + sizeof(alloc), params);
    if (FAST_ALLOC_PREDICT(full_blob != NULL)) {
        void *data_blob = (slab *)full_blob + 1;
        void *cur_blob = data_blob;
        for(uint32_t i = 0; i < num_blobs - 1; i++) {
            SET_NEXT_BLOB(cur_blob, (char *)cur_blob + blob_size);
            SET_BLOCK(cur_blob, blob_size, full_blob);
            cur_blob = cur_blob + blob_size;
        }
        SET_NEXT_BLOB(cur_blob, NULL);
        ((slab *) full_blob)->data = data_blob;
    }

    return (slab *)full_blob;
}

static inline void *unchecked_alloc(slab *inslab) {
    inslab->num_alloc++;
    void *first_open = inslab->first_open;
    void *data = GET_BLOB_DATA(first_open);
    inslab->first_open = GET_NEXT_BLOB(first_open);
    return data;
}

static void *alloc_slab(unfixed_block *inblock) {
    slab *newslab = create_slab_data(inblock->data_size,
                                     inblock->unit_num,
                                     inblock->alloc,
                                     inblock->alloc_params);
    if (newslab == NULL)
        return NULL;

    newslab->first_open = newslab->data;
    newslab->num_alloc = 1;
    add_slab(inblock->partial, newslab);
}

static void *add_partial(unfixed_block *inblock) {
    if(inblock->empty == NULL) {
        return alloc_slab(inblock);
    }
    else {
        slab *newpartial = inblock->empty;
        inblock->empty = remove_slab(newpartial);
        add_slab(inblock->partial, newpartial);
        return unchecked_alloc(inblock->partial);
    }
}

static void *swap_slabs(unfixed_block *inblock) {
    // move the first partial block to full
    slab *newfull = inblock->partial;
    inblock->partial = remove_slab(newfull);
    add_slab(inblock->full, newfull);

    // if no partial blocks left, add another one
    if (inblock->partial == NULL)
        return add_partial(inblock);

    return unchecked_alloc(inblock->partial);
}

void *block_alloc(unfixed_block *inblock) {
    void *first_open = inblock->partial->first_open;
    if (FAST_ALLOC_PREDICT_NOT(first_open == NULL))
        return swap_slabs(inblock);

    inblock->partial->num_alloc++;
    void *data = GET_BLOB_DATA(first_open);
    inblock->partial->first_open = GET_NEXT_BLOB(first_open);
    return data;
}

void block_free(unfixed_block *inblock, void *ptr) {
    if (FAST_ALLOC_PREDICT_NOT(ptr == NULL))
        return;
    slab *curslab = GET_BLOCK(ptr, inblock->data_size);
}
