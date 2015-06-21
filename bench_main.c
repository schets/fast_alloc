#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include "tests/tree.h"
#include "block_alloc.h"

uint32_t next_rand(uint64_t *state) {
    *state = 6364136223846793005 * *state + 1442695040888963407;
    return *state >> 32;
}

void bench_mem(size_t num, size_t alloc_size) {
    void **storage = (void**)calloc((1 + alloc_size), sizeof(void *));
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

void *alloc_block_hint(struct alloc_type *myalloc, void *hint, size_t size) {
    block_alloc_class *act_alloc = (block_alloc_class *)myalloc;
    return block_alloc(act_alloc->blk);
}

void free_block(struct alloc_type *myalloc, void *inptr) {
    block_alloc_class *act_alloc = (block_alloc_class *)myalloc;
    block_free(act_alloc->blk, inptr);
}

void (*fncs[])(tree *, uint32_t) = {remove_tree, change_tree, add_tree};

struct alloc_type block_alloc_base = {alloc_block, alloc_block_hint, free_block};

void bench_tree(size_t num, size_t alloc_size, void **storage) {
    uint64_t randstate = 0xdeadbeef;
    uint32_t mask = 0xffffff;
    size_t numiter = mask / 100;
    numiter = numiter < 20 ? 20 : numiter;
    numiter = numiter > 10000 ? 10000 : numiter;
    struct unfixed_block blk = create_unfixed_block(32, 50000);
    block_alloc_class myclass = {block_alloc_base, &blk};
    tree mytree = create_tree((struct alloc_type *)default_alloc);
    add_tree(&mytree, mask/2);
    for(size_t i = 0; i < mask/2; i++) {
        add_tree(&mytree, next_rand(&randstate) & mask);
    }
    printf("Tree is %ld nodes deep\n", depth(&mytree));
    for(size_t i = 0; i < num; i++) {
        int myval = (next_rand(&randstate) % 4);
        size_t limit = next_rand(&randstate) % numiter;
        for(size_t j = 0; j < limit; j++) {
            if (myval == 3)
                contains(&mytree, next_rand(&randstate) & mask);
            else {
                fncs[myval](&mytree, next_rand(&randstate) & mask);
            }
        }
        if (!(i % 100)) {
            printf("Tree is %ld nodes deep\n", depth(&mytree));
        }
    }
    for(size_t i = 0; i < num; i++) {
        contains(&mytree, next_rand(&randstate) & mask);
    }
    // destroy_tree(&mytree);
    destroy_unfixed_block(&blk);
}
