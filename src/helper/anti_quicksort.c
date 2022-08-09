/**
 *  Anti-quick-sort generator adopted from an appendix from the paper 
 *     "A Killer Adversary for Quicksort"
 *  By M. D. MCILROY
 *  https://www.cs.dartmouth.edu/~doug/mdmspe.pdf
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../terminal_visualiser.h"

#include "anti_quicksort.h"

/*
 * Thanks! https://stackoverflow.com/questions/2232706/swapping-objects-using-pointers
 */
static void swap(void* a, void* b, size_t size) {
	char tmp[size];
	memcpy(tmp, b, size);
	memcpy(b, a, size);
	memcpy(a, tmp, size);
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

static void __qs(void *list, size_t nitems, size_t size, int (*compr)(const void*, const void*)) {
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
	__qs(list, left_len, size, compr);
	__qs(partition_ptr + size, nitems - left_len - 1, size, compr);
}

static vis_int_t *val;      // item values
static int ncmp;      // number of comparisons
static int nsolid;    // number of solid items
static int candidate; // pivot candidate
static int gas;       // gas value

static int *number_counts;

#define freeze(x) number_counts[val[x].num]--; val[x].num = nsolid++; number_counts[val[x].num]++; \
print_array_bars("Performing anti-quicksort", &(val[x]), 0, NULL, 0, 1)

static int cmp(const void *px, const void *py) { // per C standard
	const int x = *(const int*) px;
	const int y = *(const int*) py;
	ncmp++;
	if (val[x].num == gas && val[y].num == gas) {
		mark_array_write();
		if (x == candidate) {
			freeze(x);
		} else {
			freeze(y);
		}
	}
	if (val[x].num == gas) {
		candidate = x;
	} else if (val[y].num == gas) {
		candidate = y;
	}
	return val[x].num - val[y].num; // only the sign matters
}

int antiqsort_v(vis_int_t *a, int n, int *clone_a, int clone_n) {
	assert(n == clone_n);
	int i;
	int j;
	int *ptr = malloc(n * sizeof(*ptr));
	if (ptr == NULL) {
		ncmp = -1;
		goto err0;
	}
	number_counts = malloc(n * sizeof(int));
	if (number_counts == NULL) {
		ncmp = -1;
		goto err1;
	}
	for (i = 0; i < n; i++) {
		number_counts[i] = 0;
	}
	val = a;
	gas = n - 1;
	nsolid = ncmp = candidate = 0;
	for (i = 0; i < n; i++) {
		ptr[i] = i;
		val[i].num = gas;
		number_counts[val[i].num]++;
		mark_array_write();
		print_array_bars("Performing anti-quicksort", &(val[i]), 0, NULL, 0, 1);
	}
	__qs(ptr, n, sizeof(*ptr), cmp);

	j = 0;
	for (i = 0; i < clone_n; i++) {
		if (number_counts[j] <= 0) {
			j++;
		}
		clone_a[i] = j;
		number_counts[j]--;
	}
	free(number_counts);
err1:
	free(ptr);
err0:
	return ncmp;
}
