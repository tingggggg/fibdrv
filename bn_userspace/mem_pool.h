#ifndef _MEMORY_POOL_H
#define _MEMORY_POOL_H

#define mem_size_t unsigned long long

typedef struct _mp_chunk {
    void *start;
    mem_size_t mem_size;
    struct _mp_chunk *next;
    int is_free;
} _MP_Chunk;

typedef struct _mp_mem_pool {
    mem_size_t pool_size;
    _MP_Chunk *free_list;
    _MP_Chunk *alloc_list;
} MemoryPool;

void initialize_mem_pool(MemoryPool **pool, mem_size_t size);
void *my_malloc(MemoryPool *p, mem_size_t size);
void my_free(MemoryPool *pool, const void *s);
void *my_realloc(MemoryPool *pool, void *s, mem_size_t new_size);

#endif /* _MEMORY_POOL_H */