#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "bn.h"

#define ITH 1000

int main()
{
    bn *fib = bn_alloc(1);

    for (int i = 0; i < ITH; i++)
        bn_fib_fdoubling(fib, i);

    for (int i = 0; i < ITH; i++) {
        struct timespec t1, t2;
        clock_gettime(CLOCK_MONOTONIC, &t1);
        bn_fib_fdoubling(fib, i);
        // bn_fib(fib, i);
        clock_gettime(CLOCK_MONOTONIC, &t2);
        long long ut = (long long) (t2.tv_sec * 1e9 + t2.tv_nsec) -
                       (t1.tv_sec * 1e9 + t1.tv_nsec);  // ns

        // char *p = bn_to_string(fib);
        printf("%d %lld\n", i, ut);
        // free(p);
    }

    bn_free(fib);

    return 0;
}