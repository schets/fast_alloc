#define FAST_ALLOC_IMPL

#include "block_alloc.h"
#include <stdio.h>

#define GET_BLOB_DATA(x) (x)
#define SET_NEXT_BLOB(x, newb) *((void **)x) = newb;
#define GET_NEXT_BLOB(x) (*((void **)x))
#define SET_BLOCK(x, data_size, inb)  *(void **)((char *)(x) + data_size - sizeof(void *)) = inb;
#define GET_BLOCK(x, data_size) (*(void **)((char *)(x) + data_size - sizeof(void *)))

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

static slab *add_slab(slab *list, slab *newslab) {
    if (!list) {
        newslab->next = newslab->prev = newslab;
        return newslab;
    }
    else {
        newslab->next = list;
        newslab->prev = list->prev;
        if (list->prev)
            list->prev->next = newslab;
        list->prev = newslab;
        return list;
    }
}

static size_t get_proper_size(size_t init_size) {
    size_t blob_size = init_size < sizeof(void *) ? sizeof(void *) : init_size; 
    blob_size = pad_size_to(blob_size, sizeof(void *));
    return pad_size(blob_size + sizeof(void *));
}

static slab* create_slab_data(size_t blob_size,
                              uint32_t num_blobs,
                              alloc_fn_type alloc,
                              void *params) {
    size_t slab_size = pad_size(sizeof(slab));
    void *full_blob = alloc(blob_size * num_blobs + slab_size, params);
    if (full_blob != NULL) {
        void *data_blob = (slab *)full_blob + 1;
        void *cur_blob = data_blob;
        for(uint32_t i = 0; i < num_blobs - 1; i++) {
            SET_NEXT_BLOB(cur_blob, ((char *) cur_blob + blob_size));
            SET_BLOCK(cur_blob, blob_size, full_blob);
            cur_blob = cur_blob + blob_size;
        }
        SET_NEXT_BLOB(cur_blob, NULL);
        SET_BLOCK(cur_blob, blob_size, full_blob);
        ((slab *) full_blob)->data = data_blob;
        ((slab *) full_blob)->first_open = data_blob;
    }

    return (slab *)full_blob;
}

static inline void *unchecked_alloc(slab *inslab) {
    //   inslab->num_alloc++;
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
    inblock->partial = add_slab(inblock->partial, newslab);
    return unchecked_alloc(inblock->partial);
}

static void *add_partial(unfixed_block *inblock) {
    if(1 || inblock->empty == NULL) {
        return alloc_slab(inblock);
    }
    else {
        slab *newpartial = inblock->empty;
        inblock->empty = remove_slab(newpartial);
        inblock->partial = add_slab(inblock->partial, newpartial);
        return unchecked_alloc(inblock->partial);
    }
}

static void *swap_slabs(unfixed_block *inblock) {
    // move the first partial block to full
    slab *newfull = inblock->partial;
    inblock->partial = remove_slab(newfull);
    inblock->full = add_slab(inblock->full, newfull);

    if (FAST_ALLOC_PREDICT(inblock->partial != NULL))
        return unchecked_alloc(inblock->partial);

    // if no partial blocks left, add another one
    return add_partial(inblock);

}

void *block_alloc(unfixed_block *inblock) {
    if (FAST_ALLOC_PREDICT_NOT (!inblock->partial))
        return add_partial(inblock);
    
    void *first_open = inblock->partial->first_open;

    if (FAST_ALLOC_PREDICT_NOT(!first_open))
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
    char is_full = (curslab->num_alloc == inblock->unit_num);
    SET_NEXT_BLOB(ptr, curslab->first_open);
    curslab->first_open = ptr;
    //  curslab->num_alloc--;

    if(FAST_ALLOC_PREDICT_NOT(is_full)) {
        slab *newptr = remove_slab(curslab);
        inblock->full = (curslab == inblock->full ? newptr : inblock->full);
        inblock->partial = add_slab(inblock->partial, curslab);
    }
    /*   else if (FAST_ALLOC_PREDICT_NOT(curslab->num_alloc == 0)) {
        slab *newptr = remove_slab(curslab);
        inblock->partial = (curslab == inblock->partial ? newptr : inblock->partial);
        inblock->empty = add_slab(inblock->empty, curslab);
        }*/
}

unfixed_block create_unfixed_block(size_t unit_size, size_t unit_num) {
    unfixed_block blk;
    blk.partial = blk.full = blk.empty = NULL;
    blk.data_size = get_proper_size(unit_size);
    blk.unit_num = unit_num;
    blk.alloc = fast_alloc_malloc;
    blk.alloc_params = 0;
    return blk;
}

static void free_slab_ring(slab *inslab) {
    if (!inslab)
        return;
    inslab->prev->next = NULL;
    while(inslab) {
        void *free_ptr = inslab;
        inslab = inslab->next;
        fast_alloc_free(free_ptr, 0);
    } 
}

void destroy_unfixed_block(unfixed_block* blk) {
    free_slab_ring(blk->partial);
    free_slab_ring(blk->full);
    free_slab_ring(blk->empty);
    blk->partial = blk->empty = blk->full = NULL;
}

void cleanup_block(unfixed_block *blk) {
    free_slab_ring(blk->empty);
    blk->empty = NULL;
}
