#include "mem_pool.h"

#define MP_INIT_MEMORY_LIST(ml, mem_size)           \
    do {                                            \
        ml->mem_pool_size = mem_size;               \
        ml->alloc_mem = 0;                          \
        ml->alloc_prog_mem = 0;                     \
        ml->free_list = (_MemoryChunk *) ml->start; \
        ml->free_list->is_free = 1;                 \
        ml->free_list->alloc_mem = mem_size;        \
        ml->free_list->prev = NULL;                 \
        ml->free_list->next = NULL;                 \
        ml->alloc_list = NULL;                      \
    } while (0)

MemoryPool *MemoryPoolInit(mem_size_t maxMemPoolSize, mem_size_t memPoolSize)
{
    if (memPoolSize > maxMemPoolSize) {
        return NULL;
    }

    MemoryPool *mp = (MemoryPool *) malloc(sizeof(MemoryPool));
    if (!mp)
        return NULL;

    mp->last_id = 0;
    if (memPoolSize < maxMemPoolSize)
        mp->auto_extend = 1;
    mp->max_mem_pool_size = maxMemPoolSize;
    mp->total_mem_pool_size = mp->mem_pool_size = memPoolSize;

    char *s =
        (char *) malloc(sizeof(_MemoryList) + sizeof(char) * mp->mem_pool_size);
    if (!s)
        return NULL;

    mp->mlist = (_MemoryList *) s;
    mp->mlist->start = s + sizeof(_MemoryList);
    MP_INIT_MEMORY_LIST(mp->mlist, mp->mem_pool_size);
    mp->mlist->next = NULL;
    mp->mlist->id = mp->last_id++;

    return mp;
}