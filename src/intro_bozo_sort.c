#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "terminal_visualiser.h"

void isort(void *list, size_t nitems, size_t size, int (*compr)(const void*, const void*)) {
	for (size_t i = 1; i < nitems; i++) {
		char to_insert[size];
		memcpy_v(to_insert, list + i * size, size);
		void* ins_ptr = list + (i - 1) * size;
		for (; ins_ptr >= list && compr(ins_ptr, to_insert) > 0; ins_ptr -= size) {
			memcpy_v(ins_ptr + size, ins_ptr, size);
		}
		if (ins_ptr != list + (i - 1) * size) {
			memcpy_v(ins_ptr + size, to_insert, size);
		}
	}
}

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

char is_sorted(void *list, size_t nitems, size_t size, int (*compr)(const void*, const void*)) {
	void *list_end = list + nitems * size;
	for (void* ptr = list + size; ptr < list_end; ptr += size) {
		if (compr(ptr - size, ptr) > 0) {
			return 0;
		}
	}
	return 1;
}

static size_t find_log2_n(size_t n) {
	size_t r = 0;
	while (n > 0) {
		r++;
		n >>= 1;
	}
	return r - 1;
}

void introbozosort(void *list, size_t nitems, size_t size, int (*compr)(const void*, const void*)) {
	size_t threshold = nitems * find_log2_n(nitems) * find_log2_n(nitems);
	while (threshold--) {
		if (is_sorted(list, nitems, size, compr)) {
			return;
		}
		size_t i1 = s32_rand() % nitems;
		size_t i2 = s32_rand() % nitems;
		if (i1 > i2) {
			size_t i3 = i2;
			i2 = i1;
			i1 = i3;
		}
		if (compr(list + i1 * size, list + i2 * size) > 0) {
			swap(list + i1 * size, list + i2 * size, size);
		}
	}
	isort(list, nitems, size, compr);
}

int main(int argc, char** argv) {
	return perform_sort_visual_args(argc, argv, introbozosort, "introspective bozo sort");
}
