#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "terminal_visualiser.h"

/*
 * Thanks! https://stackoverflow.com/questions/2232706/swapping-objects-using-pointers
 */
static void swap(void* a, void* b, size_t size) {
	if (a == b) {
		return;
	}
	char tmp[size];
	memcpy(tmp, b, size);
	memcpy(b, a, size);
	mark_array_write();
	memcpy(a, tmp, size);
	mark_array_write();
	print_array_bars_not_compare(a, b);
}

void shakersort(void *list, size_t nitems, size_t size, int (*compr)(const void*, const void*)) {
	size_t leftmost_id = 0;
	size_t last_swapped_idx = 1;
	for (size_t len_left = nitems; len_left > leftmost_id; len_left--, leftmost_id++) {
		last_swapped_idx = 0;
		for (size_t i = leftmost_id + 1; i < len_left; i++) {
			if (compr(list + (i - 1) * size, list + i * size) > 0) {
				last_swapped_idx = i + 1;
				swap(list + (i - 1) * size, list + i * size, size);
			}
		}
		len_left = last_swapped_idx;
		if (last_swapped_idx == 0) {
			break;
		}
		last_swapped_idx = nitems - 1;
		for (size_t i = len_left - 1; i > leftmost_id; i--) {
			if (compr(list + (i - 1) * size, list + i * size) > 0) {
				last_swapped_idx = i - 2;
				swap(list + (i - 1) * size, list + i * size, size);
			}
		}
		leftmost_id = last_swapped_idx;
		if (last_swapped_idx == nitems - 1) {
			break;
		}
		// leftmost_id++;
	}
}

int main(int argc, char** argv) {
	return perform_sort_visual_args(argc, argv, shakersort, "even better cocktail shaker sort");
}
