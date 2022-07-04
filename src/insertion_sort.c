#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "terminal_visualiser.h"

void isort(void *list, size_t nitems, size_t size, int (*compr)(const void*, const void*)) {
	for (size_t i = 1; i < nitems; i++) {
		char to_insert[size];
		memcpy(to_insert, list + i * size, size);
		print_array_bars_not_compare(list + i * size, NULL);
		void* ins_ptr = list + (i - 1) * size;
		for (; ins_ptr >= list && compr(ins_ptr, to_insert) > 0; ins_ptr -= size) {
			memcpy(ins_ptr + size, ins_ptr, size);
			mark_array_write();
			print_array_bars_not_compare(ins_ptr, ins_ptr + size);
		}
		if (ins_ptr != list + (i - 1) * size) {
			memcpy(ins_ptr + size, to_insert, size);
			mark_array_write();
			print_array_bars_not_compare(ins_ptr + size, NULL);
		}
	}
}

int main(int argc, char** argv) {
	return perform_sort_visual_args(argc, argv, isort, "insertion sort");
}
