#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

#define INSERT_TO_LIST(list, ck)                              \
    do {                                                      \
        _MP_Chunk **indirect = &list;                         \
        while (*indirect && ck->start > (*indirect)->start) { \
            indirect = &(*indirect)->next;                    \
        }                                                     \
        ck->next = *indirect;                                 \
        *indirect = ck;                                       \
    } while (0)

#define REMOVE_FROM_LIST(list, ck, s)          \
    do {                                       \
        _MP_Chunk **indirect = &list;          \
        while (*indirect) {                    \
            if ((*indirect)->start == s) {     \
                ck = *indirect;                \
                *indirect = (*indirect)->next; \
                break;                         \
            }                                  \
            indirect = &(*indirect)->next;     \
        }                                      \
    } while (0)

#define MERGE_TWO_CHUNK(ck_fir, ck_sec)       \
    do {                                      \
        _MP_Chunk *evict = ck_sec;            \
        ck_fir->mem_size += ck_sec->mem_size; \
        ck_fir->next = ck_sec->next;          \
        free(evict);                          \
    } while (0)

// Merge memory list
#define MERGE_LIST(target_list)                                     \
    do {                                                            \
        _MP_Chunk *list = target_list;                              \
        while (list && list->next) {                                \
            if ((void *) ((char *) list->start + list->mem_size) >= \
                list->next->start) {                                \
                MERGE_TWO_CHUNK(list, list->next);                  \
                continue;                                           \
            }                                                       \
            list = list->next;                                      \
        }                                                           \
    } while (0)

#define ALLOC_CHUNK(pool, alloc_ck, free_ck, size)                       \
    do {                                                                 \
        if (size == free_ck->mem_size) {                                 \
            REMOVE_FROM_LIST(pool->free_list, alloc_ck, free_ck->start); \
        } else {                                                         \
            INITIALIZE_MEM_CHUNK(alloc_ck, free_ck->start, size);        \
            free_ck->start = (char *) free_ck->start + size;             \
            free_ck->mem_size -= size;                                   \
        }                                                                \
        alloc_ck->is_free = 0;                                           \
        pool->pool_size -= size;                                         \
    } while (0)

void initialize_mem_pool(MemoryPool **pool, mem_size_t size)
{
    INITIALIZE_MEMORY(size);
    *pool = (MemoryPool *) malloc(sizeof(MemoryPool));
    (*pool)->pool_size = size;
    (*pool)->alloc_list = NULL;
    _MP_Chunk *ck;
    INITIALIZE_MEM_CHUNK(ck, MEMORY, size);
    (*pool)->free_list = ck;
}

void *my_malloc(MemoryPool *p, mem_size_t size)
{
    _MP_Chunk *free_list = p->free_list;

    while (free_list) {
        if (free_list->is_free && free_list->mem_size >= size) {
            _MP_Chunk *allocated_ck = NULL;

            ALLOC_CHUNK(p, allocated_ck, free_list, size);

            // INSERT_TO_LIST(p->alloc_list, allocated_ck);
            allocated_ck->next = p->alloc_list;
            p->alloc_list = allocated_ck;

            return allocated_ck->start;
        }
        free_list = free_list->next;
    }
    return (void *) 0;
}

void my_free(MemoryPool *pool, const void *s)
{
    _MP_Chunk *target_ck = NULL;

    // Remove chunk from allocated list
    REMOVE_FROM_LIST(pool->alloc_list, target_ck, s);

    // Insert chunk to free list if find out the target chunk
    if (target_ck != NULL) {
        target_ck->is_free = 1;
        pool->pool_size += target_ck->mem_size;
        INSERT_TO_LIST(pool->free_list, target_ck);

        MERGE_LIST(pool->free_list);
    }
}

void *my_realloc(MemoryPool *pool, void *s, mem_size_t new_size)
{
    // Find chunk
    _MP_Chunk *alloc_ck = pool->alloc_list;
    while (alloc_ck && alloc_ck->start != s) {
        alloc_ck = alloc_ck->next;
    }

    // Target chunk not found
    if (!alloc_ck)
        return (void *) 0;

    // Allocate the same size of space
    if (new_size == alloc_ck->mem_size)
        return s;

    if (new_size > alloc_ck->mem_size) {  // Allocate the bigger size of space
        void *end = (void *) ((char *) s + alloc_ck->mem_size);
        mem_size_t need_free_size = new_size - alloc_ck->mem_size;

        // Find connected chunk in free list
        _MP_Chunk *free_ck = pool->free_list;
        while (free_ck) {
            if (free_ck->start == end && free_ck->mem_size >= need_free_size)
                break;
            free_ck = free_ck->next;
        }
        if (free_ck) {
            _MP_Chunk *next_allocated_ck = NULL;
            ALLOC_CHUNK(pool, next_allocated_ck, free_ck, need_free_size);

            alloc_ck->mem_size += next_allocated_ck->mem_size;
            return alloc_ck->start;
        }

        // Find new chunk
        void *new_addr = my_malloc(pool, new_size);  // Alloc new space
        memcpy(new_addr, alloc_ck->start, alloc_ck->mem_size);
        my_free(pool, s);  // Free origin space
        return new_addr;

    } else {  // Allocate the smaller size of space
        // Add the free chunk to free list
        mem_size_t free_size = alloc_ck->mem_size - new_size;
        _MP_Chunk *free_ck;
        INITIALIZE_MEM_CHUNK(free_ck, (void *) ((char *) s + new_size),
                             free_size);
        INSERT_TO_LIST(pool->free_list, free_ck);
        pool->pool_size += free_size;

        alloc_ck->mem_size -= free_size;
        return alloc_ck->start;
    }

    // Unable to allocate the required space
    my_free(pool, s);
    return (void *) 0;
}

/* Test API */
// void show_pool(MemoryPool *pool)
// {
//     printf("\n");
//     printf(" ***** Show Pool ***** \n");
//     printf(" * free list [pool size: %llu]:\n", pool->pool_size);
//     _MP_Chunk *list = pool->free_list;
//     while (list) {
//         printf("\tSize: %llu, start: %p, is_free: %d\n", list->mem_size,
//                list->start, list->is_free);
//         list = list->next;
//     }

//     printf(" * alloc list:\n");
//     list = pool->alloc_list;
//     while (list) {
//         printf("\tSize: %llu, start: %p, is_free: %d\n", list->mem_size,
//                list->start, list->is_free);
//         list = list->next;
//     }
//     printf(" ********************* \n");
//     printf("\n");
// }

// int main()
// {
//     MemoryPool *pool;
//     INITIALIZE_MEM_POOL(pool, 28);

//     show_pool(pool);

//     int *a = (int *) my_malloc(pool, sizeof(int) * 1);
//     int *b = (int *) my_malloc(pool, sizeof(int) * 2);
//     printf("Integer a address : %p\n", a);
//     printf("Integer b address : %p\n", b);

//     // show_pool(pool);

//     int *c = (int *) my_malloc(pool, sizeof(int) * 2);
//     int *d = (int *) my_malloc(pool, sizeof(int) * 1);

//     printf("c address : %p\n", c);
//     printf("Integer d address : %p\n", d);

//     show_pool(pool);

//     my_free(pool, d);
//     c = my_realloc(pool, c, (sizeof(int) * 4));
//     printf("c address : %p\n", c);
//     show_pool(pool);

//     return 0;
// }