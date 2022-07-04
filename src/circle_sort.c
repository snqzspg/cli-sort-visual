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

// static void print_int_list(const int* arr, size_t len) {
// 	printf("[");
// 	while (len--) {
// 		printf("%d", *arr);
// 		if (len) printf(", ");
// 		arr++;
// 	}
// 	printf("]");
// }

int circle_sorted(void *list, size_t nitems, size_t size, int (*compr)(const void*, const void*)) {
	if (nitems <= 1) {
		return 1;
	}

	// printf("Before: ");
	// print_int_list((int*) list, nitems);
	// printf("\n");

	char swapped = 0;

	size_t l, r;

	for (l = 0, r = nitems - 1; l < r; l++, r--) {
		if (compr(list + l * size, list + r * size) > 0) {
			swap(list + l * size, list + r * size, size);
			swapped = 1;
		}
	}

	if (l == r && compr(list + l * size, list + (r + 1) * size) > 0) {
		swap(list + l * size, list + (r + 1) * size, size);
		swapped = 1;
	}

	// printf("After: ");
	// print_int_list((int*) list, nitems);
	// printf("\n\n");

	size_t middle = (nitems - 1) / 2;
	int left_csorted = circle_sorted(list, middle + 1, size, compr);
	int right_csorted = circle_sorted(list + (middle + 1) * size, nitems - middle - 1, size, compr);

	return !swapped && left_csorted && right_csorted;
}

void csort(void *list, size_t nitems, size_t size, int (*compr)(const void*, const void*)) {
	while (!circle_sorted(list, nitems, size, compr));
}

int main(int argc, char** argv) {
	return perform_sort_visual_args(argc, argv, csort, "circle sort");
}
