#include <stdarg.h>
#include <stdio.h>

typedef struct {
    const int *array;
    int array_len;
    int n_references;
} int_pool_item_t;

const int *