#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include "tests/tree.h"

void bench_tree(size_t num, size_t alloc_size, void **storage); 
void bench_mem(size_t num, size_t alloc_size);

int main() {
    srand(time(NULL));
    bench_tree(500000, 10, 0);
}
