#include <stdio.h>
#include <stdlib.h>
#include "bn.h"

#define ITH 1000
#define ITER_TIMES 2000000

int main()
{
    bn *fib = bn_alloc(1);
    for (int i = 0; i < ITER_TIMES; i++) {
        bn_fib_fdoubling(fib, 1000);
        // bn_fib(fib, i);
        char *p = bn_to_string(fib);
        printf("Fib(%d): %s\n", i, p);
        free(p);
    }

    bn_free(fib);

    return 0;
}