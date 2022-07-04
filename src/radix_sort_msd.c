#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "terminal_visualiser.h"

static void msdrsort_w_aux_r(void *list, void *aux_array, size_t nitems, size_t size, const size_t base, size_t (*num_digits)(const void*, size_t), size_t (*get_last_nth_digit)(const void*, size_t), size_t passes_needed) {
	if (nitems <= 1 || passes_needed == 0) {
		return;
	}
	size_t bucket_counts[base];
	size_t prefix_sums[base];
	for (size_t i = 0; i < base; i++) {
		bucket_counts[i] = 0;
		prefix_sums[i] = 0;
	}
	for (size_t i = 0; i < nitems; i++) {
		size_t nth_digit = get_last_nth_digit(list + size * i, passes_needed);
		assert(nth_digit < base);
		mark_aux_array_write();
		memcpy_v(aux_array + size * i, list + size * i, size);
		bucket_counts[nth_digit]++;
		prefix_sums[nth_digit]++;
	}
	for (size_t i = 1; i < base; i++) {
		bucket_counts[i] += bucket_counts[i - 1];
		prefix_sums[i] += prefix_sums[i - 1];
	}
	for (size_t i = nitems; i > 0 ; i--) {
		size_t nth_digit = get_last_nth_digit(aux_array + size * (i - 1), passes_needed);
		assert(nth_digit < base);
		bucket_counts[nth_digit]--;
		memcpy_v(list + size * bucket_counts[nth_digit], aux_array + size * (i - 1), size);
	}
	msdrsort_w_aux_r(list, aux_array, prefix_sums[0], size, base, num_digits, get_last_nth_digit, passes_needed - 1);
	for (size_t i = 1; i < base; i++) {
		msdrsort_w_aux_r(list + prefix_sums[i - 1] * size, aux_array, prefix_sums[i] - prefix_sums[i - 1], size, base, num_digits, get_last_nth_digit, passes_needed - 1);
	}
}

void msdrsort_w_aux(void *list, void *aux_array, size_t nitems, size_t size, const size_t base, size_t (*num_digits)(const void*, size_t), size_t (*get_last_nth_digit)(const void*, size_t)) {
	if (nitems <= 1) {
		return;
	}
	size_t max_ndigits = 1;
	for (size_t i = 0; i < nitems; i++) {
		size_t ndigit = num_digits(list + size * i, base);
		if (max_ndigits < ndigit) {
			max_ndigits = ndigit;
		}
	}
	msdrsort_w_aux_r(list, aux_array, nitems, size, base, num_digits, get_last_nth_digit, max_ndigits);
}

/**
 * If you want to use negative numbers, the base need to double then subtract 1
 * In your get_last_nth_digit you need to shift the return value up so that all values >= 0
 */
void msdrsort(void* list, size_t nitems, size_t size, const size_t base, size_t (*num_digits)(const void*, size_t), size_t (*get_last_nth_digit)(const void*, size_t)) {
	void* aux_array = malloc(nitems * size);
	if (aux_array == NULL) {
		perror("ERROR [radix_sort_lsd] - Not sorting!\n");
		return;
	}
	msdrsort_w_aux(list, aux_array, nitems, size, base, num_digits, get_last_nth_digit);
	free(aux_array);
}

int main(int argc, char** argv) {
	return perform_radix_sort_visual_args(argc, argv, 4, msdrsort, "radix sort (msd)");
}
