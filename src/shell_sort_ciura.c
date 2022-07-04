#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "terminal_visualiser.h"

void ssort(void *list, size_t nitems, size_t size, int (*compr)(const void*, const void*)) {
	size_t gap_seq[] = {701, 301, 132, 57, 23, 10, 4, 1};
	size_t gap_seq_len = sizeof(gap_seq) / sizeof(size_t);
	for (size_t k = 0; k < gap_seq_len; k++) {
		size_t gap = gap_seq[k];
		if (gap > nitems) continue;
		for (size_t i = gap; i < nitems; i++) {
			char to_insert[size];
			memcpy_v(to_insert, list + i * size, size);
			void* ins_ptr = list + (i - gap) * size;
			for (; ins_ptr >= list && compr(ins_ptr, to_insert) > 0; ins_ptr -= size * gap) {
				memcpy_v(ins_ptr + size * gap, ins_ptr, size);
			}
			if (ins_ptr != list + (i - gap) * size) {
				memcpy_v(ins_ptr + size * gap, to_insert, size);
			}
		}
	}
}

int main(int argc, char** argv) {
	return perform_sort_visual_args(argc, argv, ssort, "shell sort (ciura's version)");
}
