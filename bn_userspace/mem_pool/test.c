#include <stdio.h>

#include "mem_pool.h"

int main()
{
    printf("Test Memory Pool\n");

    MemoryPool *mp = MemoryPoolInit(1024, 512);


    free(mp);

    return 0;
}