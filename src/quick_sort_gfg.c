#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "terminal_visualiser.h"

/*
 * Thanks! https://stackoverflow.com/questions/2232706/swapping-objects-using-pointers
 */
static void swap(void* a, void* b, size_t size) {
	char tmp[size];
	memcpy(tmp, b, size);
	memcpy(b, a, size);
	mark_array_write();
	memcpy(a, tmp, size);
	mark_array_write();
	print_array_bars_not_compare(a, b);
}

/**
 * Thanks! https://youtu.be/h8eyY7dIiN4
 */
static void* partition(void* left_ptr, void* right_ptr, const size_t size, int (*compr)(const void*, const void*)) {
	assert(left_ptr < right_ptr);
	#ifndef NDEBUG
	void* debug_high_bound = right_ptr;
	#endif

	void* pivot = right_ptr;
	void* i = left_ptr - size;

	for (void* j = left_ptr; j <= right_ptr - 1; j += size) {
		if (compr(j, pivot) < 0) {
			i += size;
			swap(i, j, size);
		}
	}
	assert(i < debug_high_bound);
	i += size;
	swap(i, pivot, size);

	return i;
}

void quicksort(void *list, size_t nitems, size_t size, int (*compr)(const void*, const void*)) {
	if (nitems <= 1) {
		return;
	}
	void* last_item = list + (nitems - 1) * size;

	void *partition_ptr = partition(list, last_item, size, compr);

	size_t left_len = (size_t)(partition_ptr - list) / size;
	quicksort(list, left_len, size, compr);
	quicksort(partition_ptr + size, nitems - left_len - 1, size, compr);
}

int main(int argc, char** argv) {
	return perform_sort_visual_args(argc, argv, quicksort, "quick sort (geeksforgeeks)");
}
