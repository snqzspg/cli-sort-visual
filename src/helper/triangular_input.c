#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "../terminal_visualiser.h"
#include "triangular_input.h"

static const int *da_start = NULL;
// static const int *da_end;
// static int *da_ptr;

// static int highest_number = 0;

static int *num_counts = NULL;
static int num_counts_len = 0;

#define record_int_write(n) mark_aux_array_write(); \
print_array_bars("Generating triangular input", da_start + (n), 0, NULL, 0, 1); //\
num_counts[n]++; \
if (highest_number < (n)) { \
	highest_number = n; \
}

static size_t get_n_numbers_in_sequence_i(int i) {
	return 1 << i;
}

static size_t write_permutations_n_bits_k_ones(int* __restrict__ arr, int n, int k) {
	assert(da_start != NULL);
	assert(num_counts != NULL);
	if (k == 0) {
		if (arr != NULL) *arr = 0;
		record_int_write(0);
		return 1;
	}
	if (n == k) {
		int i1 = (1 << k) - 1;
		if (arr != NULL) *arr = i1;
		record_int_write(i1);
		return 1;
	}
	size_t ret = write_permutations_n_bits_k_ones(arr, n - 1, k);
	arr += ret;
	size_t len = write_permutations_n_bits_k_ones(arr, n - 1, k - 1);
	for (size_t i = 0; i < len; i++) {
		arr[i] += 1 << (n - 1);
		record_int_write(arr[i]);
	}
	return ret + len;
}

static size_t write_triangular_sequence(int* __restrict__ arr, int n) {
	assert(da_start != NULL);
	assert(num_counts != NULL);
	if (n == 1) {
		*(arr++) = 0;
		record_int_write(0);
		*arr = 1;
		record_int_write(1);
		return 2;
	}
	size_t len = 0;
	for (size_t i = 0; i <= n; i++) {
		len += write_permutations_n_bits_k_ones(arr + len, n, i);
	}
	return len;
}

static int find_log2_n(int n) {
	int r = 0;
	while (n > 0) {
		r++;
		n >>= 1;
	}
	return r - 1;
}

int triangular_shuffle(int *display_array, int display_array_len, int *clone_arr, int clone_arr_len, int *highest_item) {
	assert(display_array_len == clone_arr_len);
	int sequence_no = find_log2_n(display_array_len);
	size_t triangular_sequence_size = get_n_numbers_in_sequence_i(sequence_no);
	if (triangular_sequence_size < display_array_len) {
		sequence_no++;
		triangular_sequence_size = get_n_numbers_in_sequence_i(sequence_no);
	}

	num_counts = (int *) malloc(sizeof(int) * (triangular_sequence_size + 1));
	if (num_counts == NULL) {
		perror("Triangular shuffle failed");
		return -1;
	}
	num_counts_len = triangular_sequence_size + 1;
	for (int i = 0; i < num_counts_len; i++) {
		num_counts[i] = 0;
	}

	assert(triangular_sequence_size >= display_array_len);

	da_start = display_array;

	int triangular_sequence[triangular_sequence_size];
	size_t actual_size = write_triangular_sequence(triangular_sequence, sequence_no);
	assert(actual_size == triangular_sequence_size);

	for (int i = 0; i < display_array_len; i++) {
		display_array[i] = triangular_sequence[i] + 1;
		mark_array_write();
		print_array_bars("Generating triangular input", display_array + i, 0, NULL, 0, 1);
		num_counts[display_array[i]]++;
		if (*highest_item < display_array[i]) {
			*highest_item = display_array[i];
		}
	}

	int i1 = 0;
	for (int i = 0; i < clone_arr_len; i++) {
		while (num_counts[i1] <= 0) {
			i1++;
		}
		clone_arr[i] = i1;
		num_counts[i1]--;
	}

	free(num_counts);
	num_counts = NULL;
	num_counts_len = 0;
	return 0;
}