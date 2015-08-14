#include "rand.h"
#include "tests/tree.h"
#include "block_alloc.h"

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>
#include <assert.h>

void printf_f(FILE *outf, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
    va_start(args, fmt);
    vfprintf(outf, fmt, args);
}
//time difference in seconds
double timediff(clock_t s, clock_t e) {
    return ((e - s) * 1.0) / CLOCKS_PER_SEC;
}
volatile size_t accval = 0; //go away compiler optimizations

void clear_cache() {
    //My largest cache (shared L2) is 3mb. 5mb should clear it
    static const size_t num_bytes = 5000000; 
    //hopefully intermittent malloc/free of big stuff
    //gets new pages and does some tlb clearing each time
    char *mybytes = malloc(num_bytes);
    memset(mybytes, accval >> 24, num_bytes);
    rand_s rstate;
    seed_rand(&rstate, num_bytes);
    for(size_t i = 0; i < num_bytes; i++) {
        uint32_t val = next_rand_to(&rstate, num_bytes);
        mybytes[val] = (val ^ accval) & 0xff;
        accval += mybytes[val];
    }
    memset(mybytes, mybytes[0], num_bytes);
    free(mybytes);
}

void shuffle(uint32_t *nums, size_t size, rand_s* randstate) {
    if (size > 1) {
        size_t i;
        for (i = 0; i < size - 1; i++) {
            size_t j = i + next_rand_to(randstate, size - i);
            j = (j < size ? j : size - 1);
            int t = nums[j];
            nums[j] = nums[i];
            nums[i] = t;
            //mix up the last one
            t = nums[i];
            nums[i] = nums[size - 1];
            nums[size - 1] = t;
        }
    } 
}

//inserts numbers in random order and then deallocates in different order
//for free list, has the effect of making allocations disjoint
//for malloc/free, maybe helps to scramble the quicklists? idk if will have serious effect
void shuffle_tree_mem(tree *to_shuffle, size_t num_elem, size_t num_shuffle, rand_s *randstate) {
    destroy_tree(to_shuffle);
    uint32_t *nums = malloc(num_elem * sizeof(uint32_t));
    for(size_t i = 0; i < num_elem; i++) {
        nums[i] = i;
    }
    shuffle(nums, num_elem, randstate);
    shuffle(nums, num_elem, randstate);
    shuffle(nums, num_elem, randstate);
    for(size_t i = 0; i < num_shuffle; i++) {
        shuffle(nums, num_elem, randstate);
        for(size_t j = 0; j < num_elem; j++) {
            add_tree(to_shuffle, nums[j]);
        }
        destroy_tree(to_shuffle);
    }
    destroy_tree(to_shuffle);
    free(nums);
}

double _time_access(tree *lookup, size_t num_lookup, uint32_t mask, rand_s *startstate) {
    clear_cache();
    clock_t ctime = clock();
    for(size_t i = 0; i < num_lookup; i++)
        contains(lookup, next_rand_to(startstate, mask));
    return 1000000 * timediff(ctime, clock()) / num_lookup;
}

double time_access(tree *lookup, size_t num_lookup, uint32_t mask, rand_s startstate) {
    double tsum = 0;
    for (size_t i = 0; i < 8.0; i++) {
        tsum += _time_access(lookup, num_lookup, mask, &startstate);
    }
    return tsum / 8.0;
}

static void (*fncs[])(tree *, uint32_t) = {remove_tree, change_tree, add_tree};

void build_random_tree(tree *tobuild, size_t num_iter, uint32_t mask, rand_s *startstate) {
    size_t numgo = num_iter/10;
    numgo = numgo < 10 ? 10 : numgo;
    numgo = numgo > 50000 ? 50000 : numgo;
    for (size_t i = 0; i < num_iter; i++) {
        int myval = next_rand(startstate) % 3;
        size_t limit = next_rand(startstate) % numgo;
        for(size_t j = 0; j < limit; j++) {
            fncs[myval](tobuild, next_rand_to(startstate, mask));
        }
    }
}

void add_up_to(tree *tobuild, size_t mask, rand_s *startstate) {
    for(size_t i = 0; i < mask/2; i++) {
        add_tree(tobuild, next_rand_to(startstate, mask));
    }
}

void free_some(tree *toclear, size_t maxv) {
    for(size_t i = 0; i < maxv; i += 5) {
        remove_tree(toclear, i);
        remove_tree(toclear, i+3);
        remove_tree(toclear, i+4);
    }
}

typedef struct {
    struct alloc_type alloc;
    struct unfixed_block *blk;
} block_alloc_class;

static void *alloc_block(struct alloc_type *myalloc, size_t size) {
    block_alloc_class *act_alloc = (block_alloc_class *)myalloc;
    return block_alloc(act_alloc->blk);
}

static void *alloc_block_hint(struct alloc_type *myalloc, void *hint, size_t size) {
        block_alloc_class *act_alloc = (block_alloc_class *)myalloc;
    return block_alloc(act_alloc->blk);
}

static void free_block(struct alloc_type *myalloc, void *inptr) {
    block_alloc_class *act_alloc = (block_alloc_class *)myalloc;
    block_free(act_alloc->blk, inptr);
}

static struct alloc_type block_alloc_base = {alloc_block, alloc_block_hint, free_block};

void bench_a_copy(uint32_t maxv, size_t accesses, FILE *outf) {
    static size_t counter = 0; //ensures randomness across same max value;
    printf("Building tree for %d\n", maxv);
    tree mallocs, mallocc, mallocsh; //malloc, copy, shuffle
    tree freel, freelc, freels;
    struct unfixed_block blk1, blk2, blk3;
    rand_s malr, freelr;
    seed_rand(&malr, 0xdeadbeef + maxv + counter);
    seed_rand(&freelr, 0xdeadbeef + maxv + counter);
    counter += 100;
    mallocs = mallocc = mallocsh = create_tree(default_alloc);
    blk1 = create_unfixed_block(24, 500);
    blk2 = create_unfixed_block(24, 500);
    blk3 = create_unfixed_block(24, 500);
    block_alloc_class myclass1 = {block_alloc_base, &blk1};
    block_alloc_class myclass2 = {block_alloc_base, &blk2};
    block_alloc_class myclass3 = {block_alloc_base, &blk3};
    freel = create_tree((struct alloc_type *)&myclass1);
    freelc = create_tree((struct alloc_type *)&myclass2);
    freels = create_tree((struct alloc_type *)&myclass3);

    //build base trees
    add_up_to(&mallocs, maxv, &malr);
    build_random_tree(&mallocs, 1000, maxv, &malr);
    free_some(&mallocs, maxv);

    if (!mallocs.elems)
        return;
    //benchmark!
    printf_f(outf, "treesize %ld\n", mallocs.elems);

    printf_f(outf, "malloc %f\n", time_access(&mallocs, accesses, maxv, malr));

    copy_tree(&mallocs, &mallocc);
    destroy_tree(&mallocs);
    printf_f(outf, "malloc_copy %f\n", time_access(&mallocc, accesses, maxv, malr));
    
    add_up_to(&freel, maxv, &freelr);
    build_random_tree(&freel, 1000, maxv, &freelr);
    free_some(&freel, maxv);
    printf_f(outf, "free_list %f\n", time_access(&freel, accesses, maxv, freelr));

    copy_tree(&freel, &freelc);
    destroy_unfixed_block(&blk1);
    printf_f(outf, "free_list_copy %f\n", time_access(&freelc, accesses, maxv, freelr));

    shuffle_tree_mem(&freels, freel.elems, 10, &freelr);
    copy_tree(&freelc, &freels);
    destroy_unfixed_block(&blk2);
    printf_f(outf, "free_list_shuffle %f\n", time_access(&freels, accesses, maxv, freelr));
    destroy_unfixed_block(&blk3);
}

void _bench_copy(FILE *outf) {
    size_t accesses_per_tree = 30000;
    for (size_t i = 400; i < 2000; i += 200) {
        for (size_t j = 0; j < 2; j++) {
            bench_a_copy(i, accesses_per_tree, outf);
        }
    }

    accesses_per_tree = 30000;
    for (size_t i = 2000; i < 20000; i += 1000) {
        for (size_t j = 0; j < 2; j++) {
            bench_a_copy(i, accesses_per_tree, outf);
        }
    }

    accesses_per_tree = 100000;
    for (size_t i = 20000; i < 100000; i += 5000) {
        for (size_t j = 0; j < 2; j++) {
            bench_a_copy(i, accesses_per_tree, outf);
        }
    }

    accesses_per_tree = 200000;
    for (size_t i = 100000; i <= 400000; i += 10000) {
        for (size_t j = 0; j < 2; j++) {
            bench_a_copy(i, accesses_per_tree, outf);
        }
    }
}

void bench_copy(FILE *outf) {
    for(size_t i = 0; i < 3; i++) {
        _bench_copy(outf);
    }
}

double _time_many(tree *lookup, size_t numt, uint32_t mask, rand_s *startstate) {
    clear_cache();
    clock_t ctime = clock();
    for(size_t i = 0; i < numt * 200; i++) {
        size_t lookn = next_rand_to(startstate, numt);
        for(size_t j = 0; j < 5; j++)
            contains(lookup + lookn, next_rand_to(startstate, mask));
    }
    return 1000000000 * timediff(ctime, clock()) / (numt * 200 * 5);
}

double time_many(tree *lookup, size_t numt, uint32_t mask, rand_s startstate) {
    double curmean = 0;
    for (size_t i = 0; i < 5; i++) {
        curmean += _time_many(lookup, numt, mask, &startstate);
    }
    return curmean / 5.0;
}
typedef struct block_tree {
    block_alloc_class myblk;
    struct unfixed_block blk;
    tree ctree;
} block_tree;

void create_block_tree(block_tree *rval, size_t dsize, size_t blksize) {
    rval->blk = create_unfixed_block(24, 500);
    rval->myblk.alloc = block_alloc_base;
    rval->myblk.blk = &rval->blk;
    rval->ctree = create_tree(&rval->myblk.alloc);
}

void destroy_block_tree(block_tree *det) {
    destroy_unfixed_block(&det->blk);
}

void bench_a_small_copy(uint32_t numt, FILE *outf) {
    static size_t counter = 0;
    static const size_t maxv = 5e2;
    printf("Building tree for %d\n", numt);
    tree *mallocs, *mallocc; //malloc, copy, shuffle
    block_tree *freel, *freelc;
    mallocs = malloc(sizeof(tree)*numt);
    mallocc = malloc(sizeof(tree)*numt);
    freel = malloc(sizeof(block_tree)*numt);
    freelc = malloc(sizeof(block_tree)*numt);
    rand_s malr, freelr;
    seed_rand(&malr, 0xdeadbeef + numt + counter);
    seed_rand(&freelr, 0xdeadbeef + numt + counter);
    counter += 100;

    //build base trees
    size_t meansize = 0;
    for (size_t i = 0; i < numt; i++) {
        mallocs[i] = create_tree(default_alloc);
        add_up_to(mallocs + i, maxv, &malr);
        build_random_tree(mallocs + i, 100, maxv, &malr);
        free_some(mallocs + i, maxv);
        meansize += mallocs[i].elems;
    }

    //benchmark!
    printf_f(outf, "numsize %ld\n", numt);

    printf_f(outf, "malloc %f\n", time_many(mallocs, numt, maxv, malr));

    for (size_t i = 0; i < numt; i++) {
        mallocc[i] = create_tree(default_alloc);
        copy_tree(mallocs + i, mallocc + i);
        destroy_tree(mallocs + i);
    }
    printf_f(outf, "malloc_copy %f\n", time_many(mallocc, numt, maxv, malr));

    for (size_t i = 0; i < numt; i++) {
        destroy_tree(mallocc + i);
    }
    
    for (size_t i = 0; i < numt; i++) {
        create_block_tree(freel + i, 24, 100);
        add_up_to(&freel[i].ctree, maxv, &freelr);
        build_random_tree(&freel[i].ctree, 1000, maxv, &freelr);
        free_some(&freel[i].ctree, maxv);
        mallocs[i] = freel[i].ctree;
    }
    printf_f(outf, "free_list %f\n", time_many(mallocs, numt, maxv, freelr));

    for (size_t i = 0; i < numt; i++) {
        create_block_tree(freelc + i, 24, 500);
        copy_tree(&freel[i].ctree, &freelc[i].ctree);
        destroy_unfixed_block(&freel[i].blk);
        mallocs[i] = freelc[i].ctree;
    }
    printf_f(outf, "free_list_copy %f\n", time_many(mallocs, numt, maxv, freelr));

    for (size_t i = 0; i < numt; i++) {
        create_block_tree(freel + i, 24, 500);
        shuffle_tree_mem(&freel[i].ctree, freelc[i].ctree.elems, 10, &freelr);
        copy_tree(&freelc[i].ctree, &freel[i].ctree);
        destroy_unfixed_block(&freelc[i].blk);
        mallocs[i] = freel[i].ctree;
    }
    printf_f(outf, "free_list_shuffle %f\n", time_many(mallocs, numt, maxv, freelr));
    for (size_t i = 0; i < numt; i++) {
        destroy_unfixed_block(&freel[i].blk);
    }
    free(mallocs);
    free(mallocc);
    free(freel);
    free(freelc);
}

void bench_small(FILE *outf) {
    for (size_t i = 0; i < 70; i++) {
        for (size_t j = 1; j < 20; j++)
            bench_a_small_copy(j, outf);
        for (size_t j = 20; j <= 50; j += 2)
            bench_a_small_copy(j, outf);
        for (size_t j = 50; j < 200; j += 5)
            bench_a_small_copy(j, outf);
        for (size_t j = 200; j < 500; j += 10)
            bench_a_small_copy(j, outf);
        for (size_t j = 500; j < 1000; j += 50)
            bench_a_small_copy(j, outf);
    }
}
