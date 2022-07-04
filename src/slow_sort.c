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

void slowsort(void *start, void *end, const size_t size, int (*compr)(const void*, const void*)) {
	if (start >= end) {
		return;
	}
	void* middle = start + ((end - start) / size / 2) * size;
	slowsort(start, middle, size, compr);
	slowsort(middle + size, end, size, compr);
	if (compr(end, middle) < 0) {
		swap(end, middle, size);
	}
	slowsort(start, end - size, size, compr);
}

void slsort(void *list, size_t nitems, size_t size, int (*compr)(const void*, const void*)) {
	slowsort(list, list + (nitems - 1) * size, size, compr);
}

int main(int argc, char** argv) {
	return perform_sort_visual_args(argc, argv, slsort, "slow sort");
}
