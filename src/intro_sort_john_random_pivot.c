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
	char tmp[size];
	memcpy(tmp, b, size);
	memcpy(b, a, size);
	mark_array_write();
	memcpy(a, tmp, size);
	mark_array_write();
	print_array_bars_not_compare(a, b);
}

/**
 * Returns count of children, could be 2, 1 or 0.
 */
static size_t cpy_children_idx(size_t* __restrict__ children, const size_t i, const size_t heaplen) {
	size_t c1 = 2 * i + 1;
	size_t c2 = 2 * (i + 1);
	if (children != NULL) {
		children[0] = c1;
		children[1] = c2;
	}
	if (c1 >= heaplen) {
		return 0;
	}
	if (c2 >= heaplen) {
		return 1;
	}
	return 2;
}

static size_t parent_idx(const size_t i) {
	return (i - 1) / 2;
}

static void* get_ptr_by_i(void* list, size_t itemsize, size_t i) {
	return list + i * itemsize;
}

char hsort_partial_sift_down(void* list, size_t heaplen, size_t itemsize, int (*compr)(const void*, const void*), size_t idx, size_t* swpchildidx) {
	size_t children[2];
	size_t ccount = cpy_children_idx(children, idx, heaplen);
	if (ccount == 0) {
		return 0;
	}
	*swpchildidx = children[0];
	if (ccount == 2 && compr(get_ptr_by_i(list, itemsize, *swpchildidx), get_ptr_by_i(list, itemsize, children[1])) < 0) {
		*swpchildidx = children[1];
	}
	if (compr(get_ptr_by_i(list, itemsize, idx), get_ptr_by_i(list, itemsize, *swpchildidx)) < 0) {
		swap(get_ptr_by_i(list, itemsize, idx), get_ptr_by_i(list, itemsize, *swpchildidx), itemsize);
		return 1;
	}
	return 0;
}

void hsort_sift_down(void* list, size_t heaplen, size_t itemsize, int (*compr)(const void*, const void*), size_t idx) {
	size_t i = idx;
	while (idx < heaplen) {
		if (hsort_partial_sift_down(list, heaplen, itemsize, compr, idx, &i)) {
			idx = i;
		} else {
			break;
		}
	}
}

void hsort_heapify(void* list, size_t heaplen, size_t itemsize, int (*compr)(const void*, const void*)) {
	size_t parent = parent_idx(heaplen - 1);
	while (1) {
		hsort_sift_down(list, heaplen, itemsize, compr, parent);
		if (parent == 0) {
			break;
		}
		parent--;
	}
}

/**
 * Unstable sorting algorithm.
 */
void hsort(void* list, size_t nitems, size_t size, int (*compr)(const void*, const void*)) {
	if (nitems <= 1) {
		return;
	}
	size_t heaplen = nitems;
	hsort_heapify(list, heaplen, size, compr);
	while (heaplen > 1) {
		swap(list, get_ptr_by_i(list, size, heaplen - 1), size);
		heaplen--;
		hsort_sift_down(list, heaplen, size, compr, 0);
	}
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

static void introspective_quicksort(void *list, size_t nitems, size_t size, int (*compr)(const void*, const void*), const size_t heapsort_threshold, size_t recurse_limit) {
	if (nitems <= 1) {
		return;
	}
	if (nitems <= heapsort_threshold) {
		return;
	}
	if (recurse_limit == 0) {
		if (nitems > heapsort_threshold) {
			hsort(list, nitems, size, compr);
		}
		return;
	}
	void* last_item = list + (nitems - 1) * size;
	void* pivot = list + (rand() % nitems) * size;

	void *partition_ptr = partition(pivot, list, last_item, size, compr);

	size_t left_len = (size_t)(partition_ptr - list) / size;
	introspective_quicksort(partition_ptr + size, nitems - left_len - 1, size, compr, heapsort_threshold, recurse_limit - 1);
	introspective_quicksort(list, left_len, size, compr, heapsort_threshold, recurse_limit - 1);
}

static size_t find_log2_n(size_t n) {
	size_t r = 0;
	while (n > 0) {
		r++;
		n >>= 1;
	}
	return r - 1;
}

void introsort(void *list, size_t nitems, size_t size, int (*compr)(const void*, const void*)) {
	size_t threshold = find_log2_n(nitems) * 2;
	introspective_quicksort(list, nitems, size, compr, 16, threshold);
	isort(list, nitems, size, compr);
}

int main(int argc, char** argv) {
	return perform_sort_visual_args(argc, argv, introsort, "introspective sort (john's implementation)");
}
