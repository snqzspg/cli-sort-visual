#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "terminal_visualiser.h"

void lsdrsort_w_aux(void *list, void *aux_array, size_t nitems, size_t size, const size_t base, size_t (*num_digits)(const void*, size_t), size_t (*get_last_nth_digit)(const void*, size_t)) {
	if (nitems <= 1) {
		return;
	}
	size_t bucket_counts[base];
	size_t passes_needed = 1;
	size_t pass_count = 1;
	while (passes_needed) {
		for (size_t i = 0; i < base; i++) {
			bucket_counts[i] = 0;
		}
		size_t max_ndigits = 1;
		for (size_t i = 0; i < nitems; i++) {
			if (pass_count == 1) {
				size_t ndigit = num_digits(list + size * i, base);
				if (max_ndigits < ndigit) {
					max_ndigits = ndigit;
				}
			}
			size_t nth_digit = get_last_nth_digit(list + size * i, pass_count);
			mark_aux_array_write();
			memcpy_v(aux_array + size * i, list + size * i, size);
			bucket_counts[nth_digit]++;
		}
		if (pass_count == 1) {
			passes_needed = max_ndigits;
		}
		for (size_t i = base - 1; i > 0; i--) {
			bucket_counts[i] = bucket_counts[i - 1];
		}
		bucket_counts[0] = 0;
		for (size_t i = 1; i < base; i++) {
			bucket_counts[i] += bucket_counts[i - 1];
		}
		for (size_t i = 0; i < nitems ; i++) {
			size_t nth_digit = get_last_nth_digit(aux_array + size * i, pass_count);
			memcpy_v(list + size * bucket_counts[nth_digit], aux_array + size * i, size);
			bucket_counts[nth_digit]++;
		}
		pass_count++;
		passes_needed--;
	}
}

/**
 * If you want to use negative numbers, the base need to double then subtract 1
 * In your get_last_nth_digit you need to shift the return value up so that all values >= 0
 */
void lsdrsort(void* list, size_t nitems, size_t size, const size_t base, size_t (*num_digits)(const void*, size_t), size_t (*get_last_nth_digit)(const void*, size_t)) {
	void* aux_array = malloc(nitems * size);
	if (aux_array == NULL) {
		printf("ERROR [radix_sort_lsd] not enough memory to create an auxillary array - Not sorting!\n");
		return;
	}
	lsdrsort_w_aux(list, aux_array, nitems, size, base, num_digits, get_last_nth_digit);
	free(aux_array);
}

int main(int argc, char** argv) {
	return perform_radix_sort_visual_args(argc, argv, 4, lsdrsort, "radix sort (lsd)");
}
