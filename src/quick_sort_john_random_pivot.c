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
static void* partition(void* pivot, void* left_ptr, void* right_ptr, const size_t size, int (*compr)(const void*, const void*)) {
	// printf("DEBUG right_ptr %p\n", right_ptr);
	if (pivot != right_ptr) {
		swap(pivot, right_ptr, size);
		pivot = right_ptr;
	}
	right_ptr -= size;
	while (left_ptr < right_ptr) {
		while (compr(left_ptr, pivot) <= 0 && left_ptr < right_ptr) {
			left_ptr += size;
		}
		while (compr(right_ptr, pivot) >= 0 && left_ptr < right_ptr) {
			right_ptr -= size;
		}
		swap(left_ptr, right_ptr, size);
	}
	if (compr(left_ptr, pivot) >= 0) {
		swap(left_ptr, pivot, size);
		return left_ptr;
	}
	return pivot;
}

void quicksort(void *list, size_t nitems, size_t size, int (*compr)(const void*, const void*)) {
	if (nitems <= 1) {
		return;
	}
	void* last_item = list + (nitems - 1) * size;
	void* pivot = list + (rand() % nitems) * size;

	void *partition_ptr = partition(pivot, list, last_item, size, compr);

	size_t left_len = (size_t)(partition_ptr - list) / size;
	quicksort(list, left_len, size, compr);
	quicksort(partition_ptr + size, nitems - left_len - 1, size, compr);
}

int main(int argc, char** argv) {
	return perform_sort_visual_args(argc, argv, quicksort, "quick sort (john's implementation) (random pivot)");
}
