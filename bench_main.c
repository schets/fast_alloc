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
    unfixed_block blk = create_unfixed_block(alloc_size, 100);
    srand(10);
    for(volatile size_t i = 0; i < num*100; i++) {
        /*     if (storage[curval] ) {
            block_free(&blk, storage[curval]);
            storage[curval] = 0;
        }
        else {
            storage[curval] = block_alloc(&blk);
            }*/
        for(size_t i = 0; i < 90; i++) {
            storage[i] = block_alloc(&blk);
        }
        for(size_t i = 0; i < 90; i++) {
            block_free(&blk, storage[i]);
        }
    }
    destroy_unfixed_block(&blk);
}

void bench_tree(size_t num, size_t alloc_size, void **storage) {
    uint32_t mask = 0xffff;
    size_t numiter = mask / 5;
    numiter = numiter < 20 ? 20 : numiter;
    tree mytree = create_tree(0, numiter/10);
    srand(10);
    for(size_t i = 0; i < num; i++) {
        int myval = ((rand() ^ rand()) % 3);
        myval = 2;
        if (myval == 0)
            for(size_t j = 0; j < rand() % numiter; j++) {
                size_t val = rand() & mask;
                if (val > mask) {
                    printf("BOGUS!!!!");
                    return;
                }
                change_tree(&mytree, val);
            }
        else if (myval == 1)
            for(size_t j = 0; j < rand() % numiter; j++) {
                size_t val = rand() & mask;
                if (val > mask) {
                    printf("BOGUS!!!!");
                    return;
                }
                remove_tree(&mytree, val);
            }
        else
            for(size_t j = 0; j < rand() % numiter; j++) {
                size_t val = rand() & mask;
                if (val > mask) {
                    printf("BOGUS!!!!");
                    return;
                }
                add_tree(&mytree, val);
            }
    }
    destroy_tree(&mytree);
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
