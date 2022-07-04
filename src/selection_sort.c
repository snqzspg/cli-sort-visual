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

static void* find_minimum(void *list, size_t nitems, const size_t size, int (*compr)(const void*, const void*)) {
	void* r = list;
	list += size;
	nitems--;
	for (; nitems != 0; list += size, nitems--) {
		if (compr(list, r) < 0) {
			r = list;
		}
	}
	return r;
}

void selection_sort(void *list, size_t nitems, size_t size, int (*compr)(const void*, const void*)) {
	for (size_t i = 0; i < nitems; i++) {
		void *min = find_minimum(list + i * size, nitems - i, size, compr);
		swap(list + i * size, min, size);
	}
}

int main(int argc, char** argv) {
	return perform_sort_visual_args(argc, argv, selection_sort, "selection sort");
}
