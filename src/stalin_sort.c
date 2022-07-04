#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "terminal_visualiser.h"

void stalinsort(void *list, size_t nitems, size_t size, int (*compr)(const void*, const void*)) {
	if (nitems < 1) {
		return;
	}
	void* left = list, *right = list + size, *end = list + nitems * size;
	for (; right < end; right += size) {
		if (compr(left, right) <= 0) {
			left += size;
			if (left != right) {
				memcpy_v(left, right, size);
			}
		}
	}
	left += size;
	set_v_array_len((left - list) / size);
}

int main(int argc, char** argv) {
	return perform_sort_visual_args(argc, argv, stalinsort, "stalin sort");
}
