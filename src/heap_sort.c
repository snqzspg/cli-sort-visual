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

int main(int argc, char** argv) {
	return perform_sort_visual_args(argc, argv, hsort, "heap sort");
}
