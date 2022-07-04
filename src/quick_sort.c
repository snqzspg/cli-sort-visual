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

static void* find_median_pos(void* a, void* b, void* c, const size_t size, int (*compr)(const void*, const void*)) {
	if (compr(a, b) > 0) {
		if (compr(c, a) > 0) {
			return a;
		}
		if (compr(b, c) > 0) {
			return b;
		}
		return c;
	}
	if (compr(a, c) > 0) {
		return a;
	}
	if (compr(b, c) < 0) {
		return b;
	}
	return c;
}

/**
 * Thanks! https://youtu.be/h8eyY7dIiN4
 */
static void* partition(void* pivot, void* left_ptr, void* right_ptr, const size_t size, int (*compr)(const void*, const void*)) {
	// if (pivot != right_ptr) {
	// 	swap(pivot, right_ptr, size);
	// 	pivot = right_ptr;
	// }
	// right_ptr -= size;
	assert(left_ptr < right_ptr);
	assert(pivot >= left_ptr && pivot <= right_ptr);
	#ifndef NDEBUG
	void* debug_low_bound = left_ptr;
	void* debug_high_bound = right_ptr;
	#endif
	while (left_ptr < right_ptr) {
		while (compr(left_ptr, pivot) < 0 && left_ptr < right_ptr) {
			left_ptr += size;
		}
		while (compr(right_ptr, pivot) >= 0 && left_ptr < right_ptr) {
			right_ptr -= size;
		}
		assert(left_ptr <= debug_high_bound);
		assert(right_ptr >= debug_low_bound - size);
		if (right_ptr <= left_ptr) {
			break;
		}
		swap(left_ptr, right_ptr, size);
		if (left_ptr == pivot) {
			pivot = right_ptr;
		}
	}
	assert(left_ptr <= pivot);
	if (left_ptr != pivot) {
		swap(left_ptr, pivot, size);
	}
	// if (compr(left_ptr, pivot) >= 0) {
	// 	swap(left_ptr, pivot, size);
	// 	return left_ptr;
	// }
	// return pivot;
	return left_ptr;
}

void quicksort(void *list, size_t nitems, size_t size, int (*compr)(const void*, const void*)) {
	if (nitems <= 1) {
		return;
	}
	if (nitems == 2) {
		if (compr(list + size, list) < 0) {
			swap(list + size, list, size);
		}
		return;
	}
	void* last_item = list + (nitems - 1) * size;
	// void* pivot = list + (rand() % nitems) * size;
	void* pivot = find_median_pos(list, list + ((nitems - 1) / 2) * size, last_item, size, compr);

	void *partition_ptr = partition(pivot, list, last_item, size, compr);

	size_t left_len = (size_t)(partition_ptr - list) / size;
	quicksort(list, left_len, size, compr);
	quicksort(partition_ptr + size, nitems - left_len - 1, size, compr);
}

int main(int argc, char** argv) {
	return perform_sort_visual_args(argc, argv, quicksort, "quick sort");
}
