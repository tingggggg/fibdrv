#include <stdio.h>
#include <stdlib.h>

#define alloc_node(n, n_type, size)                                      \
    do {                                                                 \
        n = malloc(sizeof(n_type) * size);                               \
        ((n_type *) n)->val = 0;                                         \
        for (int i = 1; i < size; i++) {                                 \
            ((n_type *) ((char *) n + sizeof(n_type) * (i - 1)))->next = \
                (char *) n + sizeof(n_type) * i;                         \
            ((n_type *) ((char *) n + sizeof(n_type) * i))->val = i;     \
        }                                                                \
    } while (0);

typedef struct node {
    int val;
    struct node *next;
} node_t;


#define BLOCK_INS_NODE(b, n)                      \
    do {                                          \
        block_t **indirect = &b;                  \
        block_t *new_b = malloc(sizeof(block_t)); \
        new_b->node_list_head = n;                \
        new_b->next = NULL;                       \
        while (*indirect) {                       \
            indirect = &(*indirect)->next;        \
        }                                         \
        *indirect = new_b;                        \
    } while (0);

typedef struct block {
    node_t *node_list_head;
    node_t *next;
} block_t;

void show_node_list(node_t *n)
{
    while (n) {
        printf("%d (addr: %p), ", n->val, n);
        n = n->next;
    }
    printf("\n");
}

int main()
{
    printf("Sizeof(node_t): %zu\n", sizeof(node_t));

    node_t *n, *n2;

    alloc_node(n, node_t, 6);
    show_node_list(n);

    alloc_node(n2, node_t, 4);
    show_node_list(n2);

    block_t *b = NULL;
    BLOCK_INS_NODE(b, n);
    BLOCK_INS_NODE(b, n2);

    while (b) {
        show_node_list(b->node_list_head);
        b = b->next;
    }

    return 0;
}