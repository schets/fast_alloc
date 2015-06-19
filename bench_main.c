#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include "fixed_block_alloc.h"
#include "fixed_stack_alloc.h"
#include "tests/tree.h"
#include "block_alloc.h"

void bench_malloc_batch(size_t num, size_t alloc_size, void **storage) {
    for(size_t i = 0; i < num; i++) {
        storage[i] = malloc(alloc_size);
    }
    for(size_t i = 0; i < num; i++) {
        free(storage[i]);
    }
}

void bench_malloc_tog(size_t num, size_t alloc_size, void **storage) {
    for(size_t i = 0; i < num; i++) {
        storage[i] = malloc(alloc_size);
        free(storage[i]);
    }
}

void bench_mem(size_t num, size_t alloc_size, void **storage) {
    memset(storage, 0, num * sizeof(void *));
    struct unfixed_block blk = create_unfixed_block(alloc_size, 5);
    srand(10);
    for(volatile size_t i = 0; i < num*200; i++) {
        size_t curind = rand() % num;
        if(storage[curind]) {
            block_free(&blk, storage[curind]);
            storage[curind] = 0;
        }
        else {
            storage[curind] = block_alloc(&blk);
        }
    }
    for(size_t i = 0; i < num; i++) {
        if (storage[i])
            block_free(&blk, storage[i]);
    }
    destroy_unfixed_block(&blk);
}

typedef struct my_alloc {
    struct alloc_type alloc;
    struct unfixed_block *blk;
} block_alloc_class;

void *alloc_block(struct alloc_type *myalloc, size_t size) {
    block_alloc_class *act_alloc = (block_alloc_class *)myalloc;
    return block_alloc(act_alloc->blk);
}

void free_block(struct alloc_type *myalloc, void *inptr) {
    block_alloc_class *act_alloc = (block_alloc_class *)myalloc;
    block_free(act_alloc->blk, inptr);
}

struct alloc_type block_alloc_base = {alloc_block, free_block};

void bench_tree(size_t num, size_t alloc_size, void **storage) {
    uint32_t mask =0xffff;
    size_t numiter = mask / 5;
    numiter = numiter < 20 ? 20 : numiter;
    numiter = 1000;
    void (*fncs[])(tree *, uint32_t) = {remove_tree, remove_tree, add_tree};
    srand(10);
    struct unfixed_block blk = create_unfixed_block(32, 10);
    block_alloc_class myclass = {block_alloc_base, &blk};
    tree mytree = create_tree((struct alloc_type *)&myclass);
    add_tree(&mytree, mask/2);
    for(size_t i = 0; i < mask/4; i++) {
        add_tree(&mytree, rand() & mask);
    }
    printf("Contructed base tree\n");
    for(size_t i = 0; i < num; i++) {
        int myval = ((rand() ^ rand()) % 4);
        size_t limit = rand() % 1000;
        for(size_t j = 0; j < limit; j++) {
            if (myval == 3) {
                if (contains(&mytree, rand() & mask))
                    rand(); //forcing a side effect so this is evaluated
            }
            else {
                fncs[myval](&mytree, rand() & mask);
            }
        }
    }
    printf("Performing Lookups\n");
    for(size_t i = 0; i < num * numiter / 100; i++) {
        if (contains(&mytree, rand() & mask))
            rand(); //forcing a side effect so this is evaluated
    }
    destroy_tree(&mytree);
    destroy_unfixed_block(&blk);
}

void bench_ufslab_batch(size_t num, size_t alloc_size, void **storage) {
    struct unfixed_block blk;
    blk = create_unfixed_block(alloc_size, (num/10));
    volatile size_t i = 0;
    for(; i < num; i++) {
        storage[i] = block_alloc(&blk);
    }
    for(size_t i = 0; i < num; i+= 2) {
        block_free(&blk, storage[i]);
    }
    destroy_unfixed_block(&blk);
}

void bench_slab_batch(size_t num, size_t alloc_size, void **storage) {
    fixed_block blk;
    blk = create_fixed_block(alloc_size, num + 1);
    volatile size_t i = 0;
    for(; i < num; i++) {
        storage[i] = fixed_block_alloc(&blk);
    }
    for(size_t i = 0; i < num; i++) {
        fixed_block_free(&blk, storage[i]);
    }
    destroy_fixed_block(&blk);
}

void bench_slab_tog(size_t num, size_t alloc_size, void **storage) {
    fixed_block blk = create_fixed_block(alloc_size, num + 1);
    volatile size_t i = 0;
    for(; i < num; i++) {
        storage[i] = fixed_block_alloc(&blk);
        fixed_block_free(&blk, storage[i]);
    }
    destroy_fixed_block(&blk);
}

void bench_slab_nof(size_t num, size_t alloc_size, void **storage) {
    fixed_block blk = create_fixed_block(alloc_size, num + 1);
    volatile size_t i = 0;
    for(; i < num; i++) {
        storage[i] = fixed_block_alloc(&blk);
    }
    destroy_fixed_block(&blk);
}

void bench_stack_nof(size_t num, size_t alloc_size, void **storage) {
    fixed_stack blk = create_fixed_stack(alloc_size, num + 1);
    volatile size_t i = 0;
    for(; i < num; i++) {
        storage[i] = fixed_stack_alloc(&blk);
    }
    destroy_fixed_stack(blk);
}


void bench_batch(size_t num, size_t alloc_size, void **storage) {
    volatile size_t i;
    for(i = 0; i < num; i++) {
    }
    for(i = 0; i < num; i++) {
    }
}

void bench_tog(size_t num, size_t alloc_size, void **storage) {
    volatile size_t i;
    for(i = 0; i < num; i++) {
    }
}

void bench_outline_batch(size_t num, void **storage,
                         void *(*allocfn)(), void (*freefn)(void *)) {
    for(size_t i = 0; i < num; i++) {
        storage[i] = allocfn();
    }
    for(size_t i = 0; i < num; i++) {
        freefn(storage[i]);
    }

}
