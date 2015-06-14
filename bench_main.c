#include <stdio.h>
#include <time.h>
#include <stdlib.h>
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

void bench_tree(size_t num, size_t alloc_size, void **storage) {
    tree mytree = create_tree(alloc_size, num/10);
    uint32_t mask = 15;
    add_tree(&mytree, mask / 2);
    srand(10);
    for(size_t i = 0; i < num; i++) {
        int myval = ((rand() ^ rand()) % 3);
        if (myval == 0)
            for(size_t j = 0; j < rand() % num; j++) {
                change_tree(&mytree, rand() & mask);
            }
        else if (myval == 1)
            for(size_t j = 0; j < rand() % num; j++) {
                remove_tree(&mytree, rand() & mask);
            }
        else
            for(size_t j = 0; j < rand() % num; j++) {
                add_tree(&mytree, rand() & mask);
            }
    }
    print_tree(&mytree);
}

void bench_ufslab_batch(size_t num, size_t alloc_size, void **storage) {
    unfixed_block blk;
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
