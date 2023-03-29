#include <stdio.h>
#include <stdlib.h>

#include "mem_pool.h"

static void *MEMORY;

#define INITIALIZE_MEMORY(size)               \
    do {                                      \
        MEMORY = malloc(sizeof(char) * size); \
    } while (0)

#define INITIALIZE_MEM_CHUNK(ck, mem_start, size) \
    do {                                          \
        ck = malloc(sizeof(_MP_Chunk));           \
        ck->start = mem_start;                    \
        ck->mem_size = size;                      \
        ck->next = NULL;                          \
        ck->is_free = 1;                          \
    } while (0)

#define INITIALIZE_MEM_POOL(p, size)                   \
    do {                                               \
        INITIALIZE_MEMORY(size);                       \
        p = (MemoryPool *) malloc(sizeof(MemoryPool)); \
        p->pool_size = size;                           \
        p->alloc_list = NULL;                          \
        _MP_Chunk *ck;                                 \
        INITIALIZE_MEM_CHUNK(ck, MEMORY, size);        \
        p->free_list = ck;                             \
    } while (0)

#define INSERT_TO_LIST(list, ck)      \
    do {                              \
        _MP_Chunk **indirect = &list; \
        while (*indirect && ck->start > (*indirect)->start) {   \
            indirect = &(*indirect)->next;                      \
        }   \
        ck->next = *indirect;         \
        *indirect = ck;               \
    } while (0)

#define REMOVE_FROM_LIST(list, ck, s)           \
    do {                                        \
        _MP_Chunk **indirect = &list;           \
        while (*indirect) {                     \
            if ((*indirect)->start == s) {      \
                ck = *indirect;          \
                *indirect = (*indirect)->next;  \
                break;                          \
            }                                   \
            indirect = &(*indirect)->next;      \
        }                                       \
    } while (0);

// Merge memory chunk
#define MERGE_CHUNK(target_list)                                   \
    do {                                                    \
        _MP_Chunk *list = target_list;                      \
        _MP_Chunk *tmp = NULL;                              \
        while (list && list->next) {                        \
            if ((void *)((char *) list->start + list->mem_size) >= list->next->start) {   \
                tmp = list->next;                        \
                list->mem_size += tmp->mem_size;            \
                list->next = tmp->next;                     \
                free(tmp);                                  \
                continue;                                   \
            }                                               \
            list = list->next;                              \
        }                                                   \
    } while (0)

void *my_malloc(MemoryPool *p, mem_size_t size)
{
    _MP_Chunk *free_list = p->free_list;

    while (free_list) {
        if (free_list->is_free && free_list->mem_size >= size) {
            _MP_Chunk *allocated_ck = NULL;
            if (size == free_list->mem_size) {
                // allocated_ck = free_list;
                REMOVE_FROM_LIST(p->free_list, allocated_ck, free_list->start);
                if (allocated_ck)
                    INSERT_TO_LIST(p->alloc_list, allocated_ck);
            } else {
                // Create new memory chunk
                INITIALIZE_MEM_CHUNK(allocated_ck, free_list->start, size);
                allocated_ck->is_free = 0;
                INSERT_TO_LIST(p->alloc_list, allocated_ck);

                free_list->start = (char *) free_list->start + size;
                free_list->mem_size -= size;
            }
            p->pool_size -= size;
            
            return allocated_ck->start;
        }

        free_list = free_list->next;
    }

    return (void *) 0;
}

void my_free(MemoryPool *pool, void *s)
{   
    _MP_Chunk *target_ck = NULL;

    // Remove chunk from allocated list
    REMOVE_FROM_LIST(pool->alloc_list, target_ck, s);

    // Insert chunk to free list if find out the target chunk
    if (target_ck != NULL) {
        target_ck->is_free = 1;
        pool->pool_size += target_ck->mem_size;
        INSERT_TO_LIST(pool->free_list, target_ck);

        MERGE_CHUNK(pool->free_list);
    } 
}

/* Test API */
void show_pool(MemoryPool *pool)
{
    printf(" ***** Show Pool ***** \n");
    printf("* free list [pool size: %llu]:\n", pool->pool_size);
    _MP_Chunk *list = pool->free_list;
    while (list) {
        printf("\tSize: %llu, start: %p, is_free: %d\n", list->mem_size,
                                                         list->start,
                                                         list->is_free);
        list = list->next;
    }

    printf("* alloc list:\n");
    list = pool->alloc_list;
    while (list) {
        printf("\tSize: %llu, start: %p, is_free: %d\n", list->mem_size,
                                                         list->start,
                                                         list->is_free);
        list = list->next;
    }
}

int main()
{
    MemoryPool *pool;
    INITIALIZE_MEM_POOL(pool, 32);

    show_pool(pool);
    // printf("Pool free_list start : %p\n", pool->free_list->start);
    // printf("Pool free_list mem size : %llu\n", pool->free_list->mem_size);

    int *a = (int *) my_malloc(pool, sizeof(int) * 3);
    int *b = (int *) my_malloc(pool, sizeof(int) * 2);
    printf("Integer a address : %p\n", a);
    printf("Integer b address : %p\n", b);

    show_pool(pool);

    // my_free(pool, a);
    // my_free(pool, b);

    // show_pool(pool);

    int *c = (int *) my_malloc(pool, sizeof(int) * 1);
    printf("Integer c address : %p\n", c);

    my_free(pool, b);
    show_pool(pool);

    int *d = (int *) my_malloc(pool, sizeof(int) * 3);
    printf("Integer d address : %p\n", d);

    return 0;
}