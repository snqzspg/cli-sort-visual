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

static int s32_rand() {
	int rng = rand() & 0xff;
	rng |= (rand() & 0xff) << 8;
	rng |= (rand() & 0xff) << 16;
	rng |= (rand() & 0x7f) << 24;
	return rng;
}

void shuffle(void *list, size_t nitems, size_t size) {
	for (size_t i = 0; i < nitems; i++) {
		int rng = s32_rand() % (nitems - i) + i;
		swap(list + i * size, list + rng * size, size);
	}
}

char is_sorted(void *list, size_t nitems, size_t size, int (*compr)(const void*, const void*)) {
	void *list_end = list + nitems * size;
	for (void* ptr = list + size; ptr < list_end; ptr += size) {
		if (compr(ptr - size, ptr) > 0) {
			return 0;
		}
	}
	return 1;
}

void bozosort(void *list, size_t nitems, size_t size, int (*compr)(const void*, const void*)) {
	while (!is_sorted(list, nitems, size, compr)) {
		swap(list + (s32_rand() % nitems) * size, list + (s32_rand() % nitems) * size, size);
		// shuffle(list, nitems, size);
	}
}

int main(int argc, char** argv) {
	return perform_sort_visual_args(argc, argv, bozosort, "bozo sort");
}
