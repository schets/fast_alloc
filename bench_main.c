#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include "tests/tree.h"
#include "block_alloc.h"
#include "rand.h"
#define num_go 20
double _bench_rand_base(size_t num, size_t alloc_size) {
    rand_s randstate;
    size_t totalall = 0;
    seed_rand(&randstate, 0xdeadbeef);
    volatile void * volatile * volatile storage = calloc(num, sizeof(void *));
    struct unfixed_block blk = create_unfixed_block(alloc_size, 5);
    srand(10);
    clock_t clk = clock();
    for(volatile size_t i = 0; i < num*num_go; i++) {
        size_t curind = next_rand(&randstate) & num;
        if(storage[curind]) {
            storage[curind] = 0;
        }
        else {
            totalall++; //each alloc will be paired with a free
            storage[curind] = (void *)(curind | 1);
        }
    }
    for(size_t i = 0; i < num; i++) {
        if (storage[i])
            storage[i] = (void *)1; //just so don't remove this loop
    }
    double d = (clock() - clk) * 1.0 / CLOCKS_PER_SEC;
    destroy_unfixed_block(&blk);
    return d * 1e9 * 1.0;
}

double _bench_rand_block(size_t num, size_t alloc_size) {
    rand_s randstate;
    size_t totalall = 0;
    seed_rand(&randstate, 0xdeadbeef);
    volatile void * volatile * volatile storage = calloc(num, sizeof(void *));
    struct unfixed_block blk = create_unfixed_block(alloc_size, 100);
    srand(10);
    clock_t clk = clock();
    for(volatile size_t i = 0; i < num*num_go; i++) {
        size_t curind = next_rand(&randstate) & num;
        if(storage[curind]) {
            block_free(&blk, (void *)storage[curind]);
            storage[curind] = 0;
        }
        else {
            totalall++; //each alloc will be paired with a free
            storage[curind] = block_alloc(&blk);
        }
    }
    for(size_t i = 0; i < num; i++) {
        if (storage[i])
            block_free(&blk, (void *)storage[i]);
    }
    double d = (clock() - clk) * 1.0 / CLOCKS_PER_SEC;
    destroy_unfixed_block(&blk);
    return d * 1e9 * 1.0;
}

double _bench_rand_malloc(size_t num, size_t alloc_size) {
    rand_s randstate;
    size_t totalall = 0;
    seed_rand(&randstate, 0xdeadbeef);
    volatile void * volatile * volatile storage = calloc(num, sizeof(void *));
    srand(10);
    clock_t clk = clock();
    for(volatile size_t i = 0; i < num*num_go; i++) {
        size_t curind = next_rand(&randstate) & num;
        if(storage[curind]) {
            free((void *)storage[curind]);
            storage[curind] = 0;
        }
        else {
            totalall++; //each alloc will be paired with a free
            storage[curind] = malloc(alloc_size);
        }
    }
    for(size_t i = 0; i < num; i++) {
        if (storage[i])
            free((void *)storage[i]);
    }
    double d = (clock() - clk) * 1.0 / CLOCKS_PER_SEC;
    printf("%ld\n", totalall);
    return d * 1e9 * 1.0;
}

void bench_rand(size_t num, size_t alloc_size) {
    double block_m, block_b, block_l;
    block_b = block_l = block_m = 0;
    const size_t num_round = 50;
    for(size_t i = 0; i < num_round; i++) {
        block_b += _bench_rand_base(num, alloc_size);
    }
    block_b /= num_round;
    for(size_t i = 0; i < num_round; i++) {
        block_l += _bench_rand_block(num, alloc_size);
    }
    block_l /= num_round;
    block_l -= block_b;
    for(size_t i = 0; i < num_round; i++) {
        block_m += _bench_rand_malloc(num, alloc_size);
    }
    block_m /= num_round;
    block_m -= block_b;
    size_t num_alloc = 20000032;
    printf("base time is %f, block time is %f, malloc time is %f\n", block_b, block_l, block_m);
    printf("ns per malloc: %f, ns per list_alloc: %f\n", (block_m/num_alloc), (block_l/num_alloc));
}
