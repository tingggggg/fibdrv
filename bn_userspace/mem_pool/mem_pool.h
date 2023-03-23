#ifndef _MEMORYPOOL_H_
#define _MEMORYPOOL_H_

#include <stdlib.h>

#define mem_size_t unsigned long long

typedef struct _mp_memory_chunk {
    mem_size_t alloc_mem;
    struct _mp_memory_chunk *prev, *next;
    int is_free;
} _MemoryChunk;

typedef struct _mp_memory_list {
    char *start;
    unsigned int id;
    mem_size_t mem_pool_size;
    mem_size_t alloc_mem;
    mem_size_t alloc_prog_mem;
    _MemoryChunk *free_list, *alloc_list;
    struct _mp_memory_list *next;
} _MemoryList;

typedef struct _mp_memory_pool {
    unsigned int last_id;
    int auto_extend;
    mem_size_t mem_pool_size;
    mem_size_t total_mem_pool_size;
    mem_size_t max_mem_pool_size;
    _MemoryList *mlist;
} MemoryPool;

MemoryPool *MemoryPoolInit(mem_size_t maxMemPoolSize, mem_size_t memPoolSize)

#endif /* _MEMORYPOOL_H_ */