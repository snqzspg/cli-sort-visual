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
	memcpy(a, tmp, size);
	mark_array_write();
	print_array_bars_not_compare(a, b);
}

void cycle_sort(void *list, size_t nitems, size_t size, int (*compr)(const void*, const void*)) {
	for (size_t cycle_start = 0; cycle_start < nitems - 1; cycle_start++) {
		char holding_item[size];
		memcpy_v(holding_item, list + cycle_start * size, size);
		size_t pos = cycle_start;
		char sorted = compr(list + (pos + 1) * size, list + pos * size)>= 0;
		for (size_t i = cycle_start + 1; i < nitems; i++) {
			if (compr(list + i * size, holding_item) < 0) {
				pos++;
			}
			if (i < nitems - 1 && compr(list + (i + 1) * size, list + i * size) < 0) {
				sorted = 0;
			}
		}
		if (sorted) {
			break;
		}
		if (pos == cycle_start) {
			continue;
		}
		while (compr(holding_item, list + pos * size) == 0) {
			pos++;
		}
		swap(list + pos * size, holding_item, size);

		while (pos != cycle_start) {
			pos = cycle_start;
			for (size_t i = cycle_start + 1; i < nitems; i++) {
				if (compr(list + i * size, holding_item) < 0) {
					pos++;
				}
			}
			while (compr(holding_item, list + pos * size) == 0) {
				pos++;
			}
			swap(list + pos * size, holding_item, size);
		}
	}
}

int main(int argc, char** argv) {
	return perform_sort_visual_args(argc, argv, cycle_sort, "better cycle sort");
}
