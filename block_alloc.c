#define FAST_ALLOC_IMPL

#include "block_alloc.h"
#include "stdlib.h"
#include <stdio.h>

#define GET_BLOB_DATA(x) (x)
#define SET_NEXT_BLOB(x, newb) *((void **)x) = newb;
#define GET_NEXT_BLOB(x) (*((void **)x))
#define SET_BLOCK(x, data_size, inb)  *(void **)((char *)(x) + data_size - sizeof(void *)) = inb;
#define GET_BLOCK(x, data_size) (*(void **)((char *)(x) + data_size - sizeof(void *)))

void *mymalloc(struct alloc_type *myalloc, size_t size) {
    return malloc(size);
}

void myfree(struct alloc_type *myalloc, void *tofree) {
    free(tofree);
}

static struct alloc_type _default_alloc = {mymalloc, myfree};
struct alloc_type *default_alloc = &_default_alloc;

//only used internally, I'll typedef it for simplicity
typedef struct slab {
    void *first_open;
    size_t num_alloc;
    struct slab *next, *prev;
    void *data;
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
                              struct alloc_type * alloc) {
    size_t slab_size = pad_size(sizeof(slab));
    void *full_blob = alloc->malloc(alloc, blob_size * num_blobs + slab_size);
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

static void swap_partial(struct unfixed_block *inblock) {
    slab *newfull = inblock->partial;
    inblock->partial = remove_slab(newfull);
    inblock->full = add_slab(inblock->full, newfull);
}

static inline void *unchecked_alloc(struct unfixed_block *inblock) {
    slab *inslab = inblock->partial;
    void *first_open = inslab->first_open;
    //  inslab->num_alloc++;
    void *data = GET_BLOB_DATA(first_open);
    inslab->first_open = GET_NEXT_BLOB(first_open);
    if (FAST_ALLOC_PREDICT_NOT(inblock->partial->first_open == NULL))
        swap_partial(inblock);
    return data;
}

static void *alloc_slab(struct unfixed_block *inblock) {
    slab *newslab = create_slab_data(inblock->data_size,
                                     inblock->unit_num,
                                     inblock->allocator);
    if (newslab == NULL)
        return NULL;

    newslab->num_alloc = 0;
    newslab->first_open = newslab->data;
    inblock->partial = add_slab(inblock->partial, newslab);
    return unchecked_alloc(inblock);
}

static void *get_partial(struct unfixed_block *inblock) {
//    if(!inblock->empty)
        return alloc_slab(inblock);
//    else {
//        slab *newpartial = inblock->empty;
//        inblock->empty = remove_slab(newpartial);
//        inblock->partial = add_slab(inblock->partial, newpartial);
//        return unchecked_alloc(inblock);
//    }
}

void *block_alloc(struct unfixed_block *inblock) {
    if (FAST_ALLOC_PREDICT_NOT (!inblock->partial))
        return get_partial(inblock);
    
    return unchecked_alloc(inblock);
}

void *block_alloc_hint(struct unfixed_block *inblock, void *hint) {
    if (FAST_ALLOC_PREDICT_NOT(!hint))
        return block_alloc(inblock);

    slab *const hintslab = GET_BLOCK(hint, inblock->data_size);
    if (hintslab->first_open) {
        void *data = GET_BLOB_DATA(hintslab->first_open);
        hintslab->first_open = GET_NEXT_BLOB(hintslab-> first_open);
        if (FAST_ALLOC_PREDICT_NOT(!hintslab->first_open)) {
                slab *newslab = remove_slab(hintslab);
                inblock->partial = (hintslab == inblock->partial ? newslab : inblock->partial);
                inblock->full = add_slab(inblock->full, hintslab);
        }
        //       hintslab->num_alloc++;
        return data;
    }
    else
        return block_alloc(inblock);
}

void block_free(struct unfixed_block *inblock, void *ptr) {
    if (FAST_ALLOC_PREDICT_NOT(ptr == NULL))
        return;

    slab *curslab = GET_BLOCK(ptr, inblock->data_size);
    char is_full = curslab->first_open == NULL;
    SET_NEXT_BLOB(ptr, curslab->first_open);
    curslab->first_open = ptr;
//    curslab->num_alloc--;

    if(FAST_ALLOC_PREDICT_NOT(is_full)) {
        slab *newptr = remove_slab(curslab);
        inblock->full = (curslab == inblock->full ? newptr : inblock->full);
        inblock->partial = add_slab(inblock->partial, curslab);
    }
/*    else if (FAST_ALLOC_PREDICT_NOT(curslab->num_alloc == 0)) {
        slab *newptr = remove_slab(curslab);
        inblock->partial = (curslab == inblock->partial ? newptr : inblock->partial);
        inblock->empty = add_slab(inblock->empty, curslab);
        }*/
}

struct unfixed_block create_unfixed_block_with(size_t unit_size, size_t unit_num, struct alloc_type *alloc) {
    struct unfixed_block blk;
    blk.partial = blk.full = blk.empty = NULL;
    blk.data_size = get_proper_size(unit_size);
    blk.unit_num = unit_num < 2 ? 2 : unit_num;
    blk.allocator = alloc;
    return blk;
}

struct unfixed_block create_unfixed_block(size_t unit_size, size_t unit_num) {
    return create_unfixed_block_with(unit_size, unit_num, default_alloc);
}

static void free_slab_ring(struct alloc_type *alloc, slab *inslab) {
    if (!inslab)
        return;
    inslab->prev->next = NULL;
    while(inslab) {
        void *free_ptr = inslab;
        inslab = inslab->next;
        alloc->free(alloc, free_ptr);
    } 
}

void destroy_unfixed_block(struct unfixed_block* blk) {
    free_slab_ring(blk->allocator, blk->partial);
    free_slab_ring(blk->allocator, blk->full);
    free_slab_ring(blk->allocator, blk->empty);
    blk->partial = blk->full = blk->empty = NULL;
}
