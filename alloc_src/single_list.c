#define FAST_ALLOC_IMPL

#include "single_list.h"
#include "stdlib.h"
#include <stdio.h>

#define GET_BLOB_DATA(x) (x)
#define SET_NEXT_BLOB(x, newb) *((void **)x) = newb;
#define GET_NEXT_BLOB(x) (*((void **)x))

static void *mymalloc(struct alloc_type *myalloc, size_t size) {
    return malloc(size);
}

static void *mymalloc_hint(struct alloc_type *myalloc, void *hint, size_t size) {
    return malloc(size);
}

static void myfree(struct alloc_type *myalloc, void *tofree) {
    free(tofree);
}

static struct alloc_type _default_alloc = {mymalloc, mymalloc_hint, myfree};

struct alloc_type *default_alloc = &_default_alloc;

//only used internally, I'll typedef it for simplicity
typedef struct slab {
    struct slab *next;
    void *data;
} slab;

static size_t get_proper_size(size_t init_size) {
    size_t blob_size = init_size < sizeof(void *) ? sizeof(void *) : init_size; 
    return pad_size(blob_size);
}

static slab* create_slab_data(size_t blob_size,
                              uint32_t num_blobs,
                              struct alloc_type * alloc) {
    size_t slab_size = pad_size(sizeof(slab));
    void *full_blob = alloc->malloc(alloc, blob_size * num_blobs + slab_size);
    if (full_blob != NULL) {
        void *data_blob = (slab *)full_blob + 1;
        void *cur_blob = data_blob;
        for(uint32_t i = 0; i < num_blobs - 1; i++) {
            SET_NEXT_BLOB(cur_blob, ((char *) cur_blob + blob_size));
            cur_blob = (char *)cur_blob + blob_size;
        }
        SET_NEXT_BLOB(cur_blob, NULL);
        ((slab *) full_blob)->data = data_blob;
    }

    return (slab *)full_blob;
}

static inline void *unchecked_alloc(struct unfixed_block *inblock) {
    void *first_open = inblock->first_open;
    void *data = GET_BLOB_DATA(first_open);
    inblock->first_open = GET_NEXT_BLOB(first_open);
    return data;
}

static void *alloc_slab(struct unfixed_block *inblock) {
    slab *newslab = create_slab_data(inblock->data_size,
                                     inblock->unit_num,
                                     inblock->allocator);
    if (newslab == NULL)
        return NULL;
    inblock->alloc_num++;
    newslab->next = inblock->partial;
    inblock->partial = newslab;
    inblock->first_open = newslab->data;
    return unchecked_alloc(inblock);
}

void *block_alloc(struct unfixed_block *inblock) {
    if (FAST_ALLOC_PREDICT_NOT (!inblock->first_open))
        return alloc_slab(inblock);
    
    return unchecked_alloc(inblock);
}

void block_free(struct unfixed_block *inblock, void *ptr) {
    if (FAST_ALLOC_PREDICT_NOT(ptr == NULL))
        return;

    SET_NEXT_BLOB(ptr, inblock->first_open);
    inblock->first_open = ptr;
}

struct unfixed_block create_unfixed_block_with(size_t unit_size, size_t unit_num, struct alloc_type *alloc) {
    struct unfixed_block blk;
    blk.partial = NULL;
    blk.data_size = get_proper_size(unit_size);
    blk.unit_num = unit_num < 2 ? 2 : unit_num;
    blk.allocator = alloc;
    blk.alloc_num = 0;
    return blk;
}

struct unfixed_block create_unfixed_block(size_t unit_size, size_t unit_num) {
    return create_unfixed_block_with(unit_size, unit_num, default_alloc);
}

static size_t free_slab_ring(struct alloc_type *alloc, slab *inslab) {
    if (!inslab)
        return 0;
    size_t num = 0;
    while(inslab) {
        num++;
        void *free_ptr = inslab;
        inslab = inslab->next;
        alloc->free(alloc, free_ptr);
    } 
    return num;
}

void destroy_unfixed_block(struct unfixed_block* blk) {
    size_t part = free_slab_ring(blk->allocator, blk->partial);
    blk->partial =  NULL;
    printf("%ld blocks counted, %ld blocks allocated\n", part, blk->alloc_num);
}
