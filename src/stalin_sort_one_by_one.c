#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "terminal_visualiser.h"

void stalinsort(void *list, size_t nitems, size_t size, int (*compr)(const void*, const void*)) {
	for (size_t i = 1; i < nitems; i++) {
		if (compr(list + i * size, list + (i - 1) * size) < 0) {
			remove_varray_item(list + i * size);
			nitems--;
			i--;
		}
	}
}

int main(int argc, char** argv) {
	return perform_sort_visual_args(argc, argv, stalinsort, "stalin sort");
}
