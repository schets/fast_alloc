#include <stdio.h>
#include <time.h>
#include <stdlib.h>

const static size_t alloc_num = 5000;
const static size_t repeat = 5000;
const static size_t alloc_size = 4;
volatile size_t size_mod = 2;

void bench_malloc_batch(size_t num, size_t alloc_size, void **storage);
void bench_malloc_tog(size_t num, size_t alloc_size, void **storage);
void bench_slab_batch(size_t num, size_t alloc_size, void **storage);
void bench_slab_tog(size_t num, size_t alloc_size, void **storage);
void bench_slab_nof(size_t num, size_t alloc_size, void **storage);
void bench_stack_nof(size_t num, size_t alloc_size, void **storage);
void bench_tog(size_t num, size_t alloc_size, void **storage);
void bench_batch(size_t num, size_t alloc_size, void **storage);
void bench_ufslab_batch(size_t num, size_t alloc_size, void **storage); 


#define time_call(fnname, fstr)                                         \
    printf("Running benchmark %s\n", fstr);                             \
    {                                                                   \
        clock_t curtime = clock();                                      \
        for(i = 0; i < repeat; i++) {                   \
            fnname(alloc_num + size_mod, alloc_size, pointers);         \
        }                                                               \
        double gonetime = (double)clock() - curtime;                  \
        printf("Benchmark %s ran for  %.3f\n\n", fstr, gonetime/CLOCKS_PER_SEC); \
                                                                        \
    }                                                                   \

int main() {
    volatile size_t i = 0;
    void **pointers = (void**)malloc((1 + alloc_num) * sizeof (void *));
    time_call(bench_malloc_batch, "Batch malloc");
    time_call(bench_slab_batch, "Batch fslab");
    time_call(bench_ufslab_batch, "Batch ufslab");
    free(pointers);
}
