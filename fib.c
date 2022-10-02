#include <stdio.h>
#include <stdlib.h>
#include "bn.h"

#define ITH 1000

int main()
{
    printf("Fib main\n");

    bn *fib = bn_alloc(1);
    for (int i = 0; i < ITH + 1; i++) {
        bn_fib_fdoubling(fib, i);
        // bn_fib(fib, i);
        char *p = bn_to_string(fib);
        printf("Fib(%d): %s\n", i, p);
        free(p);
    }

    bn_free(fib);

    return 0;
}