#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "terminal_visualiser.h"

#define COUNTING_SORT_SUCCESS 0
#define COUNTING_SORT_FAILURE 1

int counting_sort_w_aux(void *list, void *aux_array, size_t nitems, const size_t size, size_t *counts, size_t nitems_counts, size_t (*key) (void *item)) {
	for (size_t i = 0; i < nitems_counts; i++) {
		counts[i] = 0;
	}

	for (size_t i = 0; i < nitems; i++) {
		size_t j = key(list + i * size);
		if (j >= nitems_counts) {
			printf("ERROR [counting_sort_w_aux] the element at index %lu is mapped outside the bounds of the histogram buffer (largest %lu)\n", (unsigned long) i, (unsigned long) (nitems_counts - 1));
			puts("Ensure that the largest_key is indeed the item that maps to the largest number!");
			puts("Sort aborted!");
			return COUNTING_SORT_FAILURE;
		}
		counts[j]++;
		mark_aux_array_write();
		memcpy_v(aux_array + i * size, list + i * size, size);
	}

	for (size_t i = nitems - 1; i > 0; i--) {
		counts[i] = counts[i - 1];
	}
	counts[0] = 0;
	for (size_t i = 1; i < nitems_counts; i++) {
		counts[i] += counts[i - 1];
	}

	for (size_t i = 0; i < nitems; i++) {
		size_t j = key(aux_array + i * size);
		memcpy_v(list + counts[j] * size, aux_array + i * size, size);
		counts[j]++;
	}
	return COUNTING_SORT_SUCCESS;
}

/**
 * Only works with positive integers.
 */
int counting_sort(void *list, size_t nitems, size_t size, size_t largest_key, size_t (*key) (void *item)) {
	size_t *counts = calloc(largest_key + 1, sizeof(size_t));
	if (counts == NULL) {
		perror("ERROR [counting_sort] not enough memory to create a histogram array - Not sorting!");
		return COUNTING_SORT_FAILURE;
	}
	void *aux_array = malloc(nitems * size);
	if (aux_array == NULL) {
		perror("ERROR [counting_sort] not enough memory to create an auxillary array - Not sorting!");
		free(counts);
		return COUNTING_SORT_FAILURE;
	}
	if (counting_sort_w_aux(list, aux_array, nitems, size, counts, largest_key + 1, key) != COUNTING_SORT_SUCCESS) {
		free(aux_array);
		free(counts);
		return COUNTING_SORT_FAILURE;
	}
	free(aux_array);
	free(counts);
	return COUNTING_SORT_SUCCESS;
}

static size_t vis_int_key(void *item) {
	return (size_t) (*((int *)item));
}

static int call_counting_sort(void* list, size_t nitems, size_t size, int (*compr)(const void*, const void*), size_t nvis_args, vis_arg_t *vis_args) {
	return counting_sort(list, nitems, size, nitems, vis_int_key) != COUNTING_SORT_SUCCESS;
}

int main(int argc, char** argv) {
	return vis_main(argc, argv, "counting sort", call_counting_sort, 0);
}
