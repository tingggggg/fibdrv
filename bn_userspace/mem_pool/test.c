#include <stdio.h>

#include "mem_pool.h"

struct TAT {
    int T_T;
};

int main()
{
    printf("Test Memory Pool\n");

    MemoryPool *mp = MemoryPoolInit(1024, 512);

    struct TAT *tat = (struct TAT *) MemoryPoolAlloc(mp, sizeof(struct TAT));

    free(tat);

    free(mp);

    return 0;
}