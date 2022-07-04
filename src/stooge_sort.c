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

void stsort(void *list, size_t nitems, size_t size, int (*compr)(const void*, const void*)) {
	if (nitems <= 1) {
		return;
	}
	if (nitems == 2) {
		if (compr(list, list + size) > 0) {
			swap(list, list + size, size);
		}
		return;
	}
	// Round down
	size_t first_third = nitems / 3;
	// Round up
	size_t second_third = nitems * 2 / 3 + ((nitems % 3) ? 1 : 0);

	stsort(list, second_third, size, compr);
	stsort(list + first_third * size, nitems - first_third, size, compr);
	stsort(list, second_third, size, compr);
}

int main(int argc, char** argv) {
	return perform_sort_visual_args(argc, argv, stsort, "stooge sort");
}
