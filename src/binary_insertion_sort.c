#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "terminal_visualiser.h"

/**
 * Find the position to insert the value of valptr. If there are identical values, it will find the back of the values.
 */
static void* pos_insert_back(void* valptr, void* list, size_t nitems, const size_t size, int (*compr)(const void*, const void*)) {
	size_t start = 0, end = nitems;
	size_t middle;
	while (end > start) {
		middle = ((start + end) / 2);
		if (compr(valptr, list + middle * size) < 0) {
			end = middle;
		} else {
			start = middle + 1;
		}
	}
	return list + start * size;
}

void bisort(void *list, size_t nitems, size_t size, int (*compr)(const void*, const void*)) {
	for (size_t i = 1; i < nitems; i++) {
		char to_insert[size];
		memcpy(to_insert, list + i * size, size);
		void* pos_to_insert = pos_insert_back(to_insert, list, i, size, compr);
		void* ins_ptr = list + (i) * size;
		for (; ins_ptr > pos_to_insert; ins_ptr -= size) {
			memcpy(ins_ptr, ins_ptr - size, size);
			mark_array_write();
			print_array_bars_not_compare(ins_ptr, ins_ptr - size);
		}
		if (ins_ptr != list + (i) * size) {
			memcpy(ins_ptr, to_insert, size);
			mark_array_write();
			print_array_bars_not_compare(ins_ptr, NULL);
		}
	}
}

int main(int argc, char** argv) {
	return perform_sort_visual_args(argc, argv, bisort, "binary insertion sort");
}