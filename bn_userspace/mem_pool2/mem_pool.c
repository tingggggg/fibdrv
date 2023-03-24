#include <stdio.h>
#include <stdlib.h>


typedef struct node {
    int val;
    struct node *next;
} node_t;

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


void show(node_t *n)
{
    while (n) {
        printf("%d, ", n->val);
        n = n->next;
    }
    printf("\n");
}

int main()
{
    node_t *n;
    alloc_node(n, node_t, 4);

    show(n);


    return 0;
}