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

void gsort(void *list, size_t nitems, size_t size, int (*compr)(const void*, const void*)) {
	void* pos = list;
	void* goal = list + nitems * size - size;
	while (pos < goal) {
		if (compr(pos, pos + size) <= 0) {
			pos += size;
			continue;
		}
		swap(pos, pos + size, size);
		if (pos == list) {
			pos += size;
			continue;
		}
		pos -= size;
	}
}

int main(int argc, char** argv) {
	return perform_sort_visual_args(argc, argv, gsort, "gnome sort");
}
