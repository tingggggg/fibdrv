#include <stdio.h>
#include <stdlib.h>

#include "bn.h"
#include "mem_pool.h"

#define ITH 1000

extern MemoryPool *pool;

int main()
{
    initialize_mem_pool(&pool, 10000);

    bn *fib = bn_alloc(1);
    for (int i = 0; i < ITH + 1; i++) {
    // for (int i = 0; i < 374; i++) {
        bn_fib_fdoubling(fib, i);
        // bn_fib(fib, i);
        char *p = bn_to_string(fib);
        printf("%d,%s\n", i, p);
        free(p);
    }

    bn_free(fib);

    return 0;
}