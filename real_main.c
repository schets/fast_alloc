#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include "tests/tree.h"

void bench_tree(size_t num); 
void bench_mem(size_t num, size_t alloc_size);
void bench_copy(FILE *outf);

int main() {
    srand(time(NULL));
    FILE *outf = fopen("testout.txt", "w");
    bench_copy(outf);
    fclose(outf);
}
