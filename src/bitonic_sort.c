#include <assert.h>
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
	// print_display_array_bars("Sorting"); // NOT PART OF IMPLEMENTATION
}

static void bitonicmerge(void *list, size_t nitems, const size_t size, int (*compr)(const void*, const void*), char reverse) {
	// Base case
	if (nitems <= 1) {
		return;
	}
	void *left_ptr = list;
	void *right_ptr = list + (nitems / 2) * size;
	void *stop_ptr = list + nitems * size;
	// Comparing the leftmost element with the middle element, and the middle element with the last element. 
	// All three being equal means that the entire array given is all identical.
	// No further swaps are necessary.
	if (compr(left_ptr, right_ptr) == 0 && compr(right_ptr, stop_ptr - size) == 0) {
		return;
	}
	// The array given can be in two arrangements:
	//   1 - The array is given in a triangle-like arrangement, ascending on left half and decending on right half.
	//       The middle element will be the highest. This is one bitonic sequence. 
	//   2 - The array is given in a "V"-shaped format, decending on the left half and ascending on the right hald.
	//       The middle element will be the lowest. This comprises of two unidirectional bitonic sequences.
	// The following variable checks which arrangement the array is in, by accessing if the middle element is the 
	// highest or the lowest.
	// Because this variable is only needed for an array with odd numbers later, it doesn't matter if the variable
	// produces inaccurate results for even-numbered arrays.
	char is_middle_min = compr(left_ptr, right_ptr) > 0 || compr(right_ptr, stop_ptr - size) < 0;
	// For odd-numbered arrays, the right pointer is increased by one position.
	// This prevents the "sweep" below from touching the middle element.
	if (nitems % 2) right_ptr += size;
	// Perform the "sweep". It compares the left pointer with the right pointer and swap if necessary.
	for (; right_ptr < stop_ptr; left_ptr += size, right_ptr += size) {
		if (reverse && compr(left_ptr, right_ptr) < 0) {
			swap(left_ptr, right_ptr, size);
		} else if (!reverse && compr(left_ptr, right_ptr) > 0) {
			swap(left_ptr, right_ptr, size);
		}
	}
	// After the "sweep", the array will have a lower half and a upper half. 
	// The lower half will be positioned at the left for forward sorting, right for reverse sorting. 
	// Vice versa for the upper half.
	// The lower half will be in a single bitonic sequence (triangle-like arrangement), while the upper half will
	// be in a "V"-shaped arrangement of two bitonic sequences (one pure decending and one pure ascending).

	// The next step is to recursively "sweep" each half. Because each "sweep" will create two partitions with all
	// elements in it greater that all elements in the other one (similarly to quick sort), recursive sweeps will
	// create smaller such high-low partitions until the array sizes reaches two elements, where the single-element
	// "partitions" will be ordered correctly, resulting in a sorted array.

	// For even-numbered sizes the splitting is straightforward.
	if (nitems % 2 == 0) {
		bitonicmerge(list, nitems / 2, size, compr, reverse);
		bitonicmerge(list + (nitems / 2) * size, nitems / 2, size, compr, reverse);
		return;
	}

	assert(reverse == 0 || reverse == 1);
	assert(is_middle_min == 0 || is_middle_min == 1);

	// For odd-numbered sizes, the middle element will be the odd one out. Because the resulting partition will be
	// one high and one low, and the middle element will either be the highest in the array or the lowest, the
	// element will join the upper one if it is a max, and the lower one if it is a min.
	if (reverse != is_middle_min) {
		bitonicmerge(list, nitems / 2 + 1, size, compr, reverse);
		bitonicmerge(list + (nitems / 2 + 1) * size, nitems / 2, size, compr, reverse);
		return;
	}
	bitonicmerge(list, nitems / 2, size, compr, reverse);
	bitonicmerge(list + (nitems / 2) * size, nitems / 2 + 1, size, compr, reverse);
}

static void insert_last_element(void *list, size_t nitems, size_t size, int (*compr)(const void*, const void*)) {
	char to_insert[size];
	memcpy(to_insert, list + (nitems - 1) * size, size);
	void* ins_ptr = list + (nitems - 2) * size;
	for (; ins_ptr >= list && compr(ins_ptr, to_insert) < 0; ins_ptr -= size) {
		memcpy(ins_ptr + size, ins_ptr, size);
		mark_array_write();
		print_array_bars_not_compare(ins_ptr, ins_ptr + size);
		// print_display_array_bars("Sorting"); // NOT PART OF IMPLEMENTATION
	}
	memcpy(ins_ptr + size, to_insert, size);
	mark_array_write();
	print_array_bars_not_compare(to_insert, NULL);
	// print_display_array_bars("Sorting"); // NOT PART OF IMPLEMENTATION
}

void bitonicsort_dir(void *list, size_t nitems, size_t size, int (*compr)(const void*, const void*), char reverse) {
	if (nitems <= 1) {
		return;
	}
	void* left_half = list;
	void* right_half = list + nitems / 2 * size;
	size_t left_half_len = nitems / 2;
	size_t right_half_len = nitems - left_half_len;

	assert(left_half_len == right_half_len || right_half_len - left_half_len == 1);

	// In this bitonic sort, the array will be divided into half, and the left half will have the smaller number of
	// elements, if the division is not even.
	// Each half will be recursively bitonic-sorted.

	bitonicsort_dir(left_half, left_half_len, size, compr, 1);
	bitonicsort_dir(right_half, right_half_len, size, compr, 0);

	// The halves will be sorted in opposite order: in this case first half will be in reverse, while second half
	// will be in proper order. This will create a "V" shape arrangement of two bitonic sequences (one pure decending,
	// another ascending).

	// For odd-number number of elements, the middle will have to be the smallest element of the entire array. 
	// Because the middle element is sorted with the second half of the array, it should now be the lowest value
	// among the second half. However, it may not be the lowest among the first half.
	if (nitems % 2) {
		// To ensure the smallest element must be in the middle of an odd-number number of items, the middle element
		// will be inserted in a similar manner to insertion sort: compare and shift the elements backwards until
		// a suitable position for that middle element is found. 
		// (The insertion will be completed in a reverse order: shift if next element is smaller, stop if greater.)
		insert_last_element(list, nitems / 2 + 1, size, compr);
	}

	// After the sorting halves process, the resulting array will be in a "V"-shaped format, with the lowest numbers
	// in the middle. 

	bitonicmerge(list, nitems, size, compr, reverse);
}

void bitonicsort(void* list, size_t nitems, size_t size, int (*compr)(const void*, const void*)) {
	bitonicsort_dir(list, nitems, size, compr, 0);
}

int main(int argc, char** argv) {
	return perform_sort_visual_args(argc, argv, bitonicsort, "bitonic sort");
}