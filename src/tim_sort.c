#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "terminal_visualiser.h"

#define MAX_MERGE_PENDING 85
#define MIN_GALLOP 7

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
}

/**
 * Find the position to insert the value of valptr. If there are identical values, it will find the back of the values.
 */
static void* pos_insert_back(void* valptr, void* list, size_t nitems, const size_t size, int (*compr)(const void*, const void*)) {
	size_t start = 0, end = nitems;
	size_t middle;
	while (end > start) {
		middle = ((start + end) / 2);
		if (compr(valptr, list + middle * size) < 0) {
			end = middle;
		} else {
			start = middle + 1;
		}
	}
	return list + start * size;
}

void bisort(void *list, size_t nitems, size_t start, size_t size, int (*compr)(const void*, const void*)) {
	if (start == 0) {
		start++;
	}
	for (size_t i = start; i < nitems; i++) {
		char to_insert[size];
		memcpy_v(to_insert, list + i * size, size);
		void* pos_to_insert = pos_insert_back(to_insert, list, i, size, compr);
		void* ins_ptr = list + (i) * size;
		for (; ins_ptr > pos_to_insert; ins_ptr -= size) {
			memcpy_v(ins_ptr, ins_ptr - size, size);
		}
		if (ins_ptr != list + (i) * size) {
			memcpy_v(ins_ptr, to_insert, size);
		}
	}
}

typedef struct {
	void* start_ptr;
	size_t nitems;
	size_t size;
} array_run;

static void array_run_advance(array_run *run, size_t n) {
	run -> start_ptr += run -> size * n;
	run -> nitems -= n;
}

typedef struct {
	// array_run* runs_buffer;
	// size_t runs_buffer_length;
	size_t min_gallop;
	array_run pending_stack[85];
	size_t stack_capacity;
	size_t num_items;
	void* aux_array;
	size_t aux_array_capacity;
} merge_state;

static size_t compute_minrun(size_t n) {
	size_t r = 0;
	assert(n >= 0);
	while (n >= 64) {
		r |= n & 1;
		n >>= 1;
	}
	return n + r;
}

static void reverse_arr(void *arr, size_t nitems, size_t size) {
	void* end_ptr = arr + (nitems - 1) * size;
	for (; arr < end_ptr; arr += size, end_ptr -= size) {
		swap(arr, end_ptr, size);
	}
}

static size_t count_run(void* start, void* end, const size_t size, char* __restrict__ descending, int (*compr)(const void*, const void*)) {
	*descending = 0;
	start += size;
	if (start == end) {
		return 1;
	}
	size_t ret = 2;
	if (compr(start, start - size) < 0) {
		*descending = 1;
		for (start = start + size; start < end; start += size, ret++) {
			if (compr(start, start - size) >= 0) {
				break;
			}
		}
	} else {
		for (start = start + size; start < end; start += size, ret++) {
			if (compr(start, start - size) < 0) {
				break;
			}
		}
	}
	return ret;
}

static size_t gallop_left(merge_state* state, void* item, void* sorted_list, size_t nitems, size_t hint, const size_t size, int (*compr)(const void*, const void*)) {
	size_t offset;
	size_t last_offset;
	size_t k;

	assert(item != NULL);
	assert(sorted_list != NULL);
	assert(nitems > 0);
	// assert(hint >= 0);
	assert(hint < nitems);

	void *item_at_hint = sorted_list + hint * size;
	last_offset = 0;
	offset = 1;
	if (compr(item_at_hint, item) < 0) {
		const size_t maxoffset = nitems - hint;
		while (offset < maxoffset) {
			if (compr(item_at_hint + offset * size, item) < 0) {
				last_offset = offset;
				offset = (offset << 1) + 1;
			} else {
				break;
			}
		}
		if (offset > maxoffset) {
			offset = maxoffset;
		}
		last_offset += hint;
		offset += hint;
	} else {
		const size_t maxoffset = hint + 1;
		while (offset < maxoffset) {
			if (compr(item_at_hint - offset * size, item) < 0) {
				break;
			}
			last_offset = offset;
			offset = (offset << 1) + 1;
		}
		if (offset > maxoffset) {
			offset = maxoffset;
		}
		k = last_offset;
		last_offset = hint - offset; // Negative unsigned overflow ???? >*o*<
		offset = hint - k;
	}

	last_offset++;
	// assert(-1 <= last_offset); // Negative unsigned overflow ???? >*o*<
	assert(last_offset <= offset && offset <= nitems);

	// Because the only "negative" last_offset value is "-1" (The largest possible value),
	// the above statement will cause it to overflow back in all cases!
	// So no need to worry about overflow.
	while (last_offset < offset) {
		size_t m = last_offset + ((offset - last_offset) >> 1);
		if (compr(sorted_list + m * size, item) < 0) {
			last_offset = m + 1;
		} else {
			offset = m;
		}
	}
	assert(last_offset == offset);
	return offset;
}

static size_t gallop_right(merge_state* state, void* item, void* sorted_list, size_t nitems, size_t hint, const size_t size, int (*compr)(const void*, const void*)) {
	size_t offset;
	size_t last_offset;
	size_t k;

	assert(item != NULL);
	assert(sorted_list != NULL);
	assert(nitems > 0);
	// assert(hint >= 0);
	assert(hint < nitems);

	void *item_at_hint = sorted_list + hint * size;
	last_offset = 0;
	offset = 1;
	if (compr(item, item_at_hint) < 0) {
		const size_t maxoffset = hint + 1;    // &sorted_list[0] is lowest
		while (offset < maxoffset) {
			if (compr(item, item_at_hint - offset * size) < 0) {
				last_offset = offset;
				offset = (offset << 1) + 1;
			} else {                    // sorted_list[hint - ofs] <= key
				break;
			}
		}
		if (offset > maxoffset) {
			offset = maxoffset;
		}
		k = last_offset;
		last_offset = hint - offset; // Negative unsigned overflow ???? >*o*<
		offset = hint - k;
	} else {
		// sorted_list[hint] <= key -- gallop right, until
		// sorted_list[hint + lastofs] <= key < sorted_list[hint + ofs]
		const size_t maxoffset = nitems - hint;
		while (offset < maxoffset) {
			if (compr(item, item_at_hint + offset * size) < 0) {
				break;
			}
			last_offset = offset;
			offset = (offset << 1) + 1;
		}
		if (offset > maxoffset) {
			offset = maxoffset;
		}
		last_offset += hint;
		offset += hint;
	}
	// item_at_hint -= hint * size;

	last_offset++;
	// Because the only "negative" last_offset value is "-1" (The largest possible value),
	// the above statement will cause it to overflow back in all cases!
	// So no need to worry about overflow.
	// assert(-1 <= last_offset); // Negative unsigned overflow ???? >*o*<
	assert(last_offset <= offset && offset <= nitems);

	while (last_offset < offset) {
		size_t m = last_offset + ((offset - last_offset) >> 1);
		if (compr(item, sorted_list + m * size) < 0) {
			offset = m;
		} else {
			last_offset = m + 1;
		}
	}
	assert(last_offset == offset);
	return offset;
}

static void merge_from_front(merge_state* state, array_run* a, array_run* b, const size_t size, int (*compr)(const void*, const void*)) {
	size_t k;
	void* left = a -> start_ptr, *right = b -> start_ptr;
	size_t left_nitems = a -> nitems, right_nitems = b -> nitems;
	void* dest;
	size_t min_gallop;

	assert(state != NULL);
	assert(left != NULL);
	assert(right != NULL);
	assert(left_nitems > 0);
	assert(right_nitems > 0);
	assert(left + left_nitems * size == right);

	// memcpy(state -> aux_array, left, left_nitems * size);
	for (size_t i = 0; i < left_nitems; i++) {
		memcpy_v(state -> aux_array + i * size, left + i * size, size);
		mark_aux_array_write();
	}
	dest = left;
	left = state -> aux_array;

	memcpy_v(dest, right, size);
	dest += size;
	right += size;
	right_nitems--;
	if (right_nitems == 0) {
		goto copy_leftover_left;
	}
	if (left_nitems == 1) {
		goto shift_lone_left;
	}
	min_gallop = state -> min_gallop;
	while (1) {
		size_t left_streak = 0;
		size_t right_streak = 0;
		/**
		 * Do the straightforward thing until (if ever) one run
		 * appears to win consistently.
		 */
		while (1) {
			// printf("DEBUG: [merge_from_front] left_nitems %lu\n", (unsigned long) left_nitems);
			// printf("DEBUG: [merge_from_front] right_nitems %lu\n", (unsigned long) right_nitems);
			assert(left_nitems > 1 && right_nitems > 0);
			if (compr(right, left) < 0) {
				memcpy_v(dest, right, size);
				dest += size;
				right += size;
				right_nitems--;
				if (right_nitems == 0) {
					goto copy_leftover_left;
				}
				right_streak++; // Right scored a point!
				left_streak = 0; // :( left lost it's streak. Better luck next time.
				if (right_streak >= min_gallop) { // right has won!
					break;
				}
			} else {
				memcpy_v(dest, left, size);
				dest += size;
				left += size;
				left_nitems--;
				left_streak++; // Left scored a point!
				right_streak = 0; // :( right lost it's streak. Better luck next time.
				if (left_nitems == 1) {
					goto shift_lone_left;
				}
				if (left_streak >= min_gallop) { // left has won!
					break;
				}
			}
		}
        /**
		 * One run is winning so consistently that galloping may
         * be a huge win. So try that, and continue galloping until
         * (if ever) neither run appears to be winning consistently
         * anymore.
         */
		min_gallop++;
		do {
			assert(left_nitems > 1 && right_nitems > 0);
			min_gallop -= min_gallop > 1; // You genius :<
			state -> min_gallop = min_gallop;
			k = gallop_right(state, right, left, left_nitems, 0, size, compr);
			left_streak = k;
			if (k) {
				// memcpy(dest, left, k * size);
				for (size_t i = 0; i < k; i++) {
					memcpy_v(dest + i * size, left + i * size, size);
				}
				dest += k * size;
				left += k * size;
				left_nitems -= k;
				if (left_nitems == 1) {
					goto shift_lone_left;
				}
				// From tim peter's comment:
				// left_nitems == 0 is impossible now if the comparison
				// function is consistent, but we can't assume that it is.
				//                         ------------------------------- <- huh ???
				if (left_nitems == 0) { // Just in case
					goto copy_leftover_left;
				}
			}
			memcpy_v(dest, right, size);
			right_nitems--;
			dest += size;
			right += size;
			if (right_nitems == 0) {
				goto copy_leftover_left;
			}
			k = gallop_left(state, left, right, right_nitems, 0, size, compr);
			right_streak = k;
			if (k) {
				// memmove(dest, right, k * size);
				for (size_t i = 0; i < k; i++) {
					memmove_v(dest + i * size, right + i * size, size);
				}
				dest += k * size;
				right += k * size;
				right_nitems -= k;
				if (right_nitems == 0) {
					goto copy_leftover_left;
				}
			}
			memcpy_v(dest, left, size);
			dest += size;
			left += size;
			left_nitems--;
			if (left_nitems == 1) {
				goto shift_lone_left;
			}
		} while (left_streak >= min_gallop || right_streak >= min_gallop);
		min_gallop++; // "penalize it for leaving galloping mode" -Tim Peters
		state -> min_gallop = min_gallop;
	}
copy_leftover_left:
	if (left_nitems) {
		// memcpy(dest, left, left_nitems * size);
		for (size_t i = 0; i < left_nitems; i++) {
			memcpy_v(dest + i * size, left + i * size, size);
		}
	}
	return;
shift_lone_left:
	assert(left_nitems == 1 && right_nitems > 0);
	// Shifts all the remaining data from right part to dest
	// memmove(dest, right, right_nitems * size);
	for (size_t i = 0; i < right_nitems; i++) {
		memmove_v(dest + i * size, right + i * size, size);
	}
	dest += size * right_nitems;
	memcpy_v(dest, left, size);
}

static void merge_from_back(merge_state* state, array_run* a, array_run* b, const size_t size, int (*compr)(const void*, const void*)) {
	size_t k;
	void* left = a -> start_ptr, *right = b -> start_ptr;
	size_t left_nitems = a -> nitems, right_nitems = b -> nitems;
	void* dest, *basea, *baseb;
	size_t min_gallop;

	assert(state != NULL);
	assert(left != NULL);
	assert(right != NULL);
	assert(left_nitems > 0);
	assert(right_nitems > 0);
	assert(left + size * left_nitems == right);

	dest = right;
	dest += (right_nitems - 1) * size;
	// memcpy(state -> aux_array, right, right_nitems * size);
	for (size_t i = 0; i < right_nitems; i++) {
		memcpy_v(state -> aux_array + i * size, right + i * size, size);
		mark_aux_array_write();
	}
	basea = left;
	baseb = state -> aux_array;
	right = state -> aux_array + (right_nitems - 1) * size;
	left += (left_nitems - 1) * size;

	memcpy_v(dest, left, size);
	dest -= size;
	left -= size;
	left_nitems--;
	if (left_nitems == 0) {
		goto copy_leftover_right;
	}
	if (right_nitems == 1) {
		goto copy_lone_right;
	}

	min_gallop = state -> min_gallop;
	while (1) {
		size_t left_streak = 0;
		size_t right_streak = 0;

		while (1) {
			assert(left_nitems > 0 && right_nitems > 1);
			if (compr(right, left) < 0) {
				memcpy_v(dest, left, size);
				dest -= size;
				left -= size;
				left_nitems--;
				left_streak++; // Left scored a point!
				right_streak = 0; // :( right lost it's streak. Better luck next time.
				if (left_nitems == 0) {
					goto copy_leftover_right;
				}
				if (left_streak >= min_gallop) { // left has won!
					break;
				}
			} else {
				memcpy_v(dest, right, size);
				dest -= size;
				right -= size;
				right_nitems--;
				right_streak++; // Right scored a point!
				left_streak = 0; // :( left lost it's streak. Better luck next time.
				if (right_nitems == 1) {
					goto copy_lone_right;
				}
				if (right_streak >= min_gallop) { // right has won!
					break;
				}
			}
		}

		min_gallop++;
		do {
			assert(left_nitems > 0 && right_nitems > 1);
			min_gallop -= min_gallop > 1; // You genius :<
			state -> min_gallop = min_gallop;
			k = gallop_right(state, right, basea, left_nitems, left_nitems - 1, size, compr);
			k = left_nitems - k;
			left_streak = k;
			if (k) {
				dest -= size * k;
				left -= size * k;
				// memmove(dest + size, left + size, k * size);
				for (size_t i = k; i > 0; i--) {
					memmove_v(dest + i * size, left + i * size, size);
				}
				left_nitems -= k;
				if (left_nitems == 0) {
					goto copy_leftover_right;
				}
			}
			memcpy_v(dest, right, size);
			dest -= size;
			right -= size;
			right_nitems--;
			if (right_nitems == 1) {
				goto copy_lone_right;
			}

			k = gallop_left(state, left, baseb, right_nitems, right_nitems - 1, size, compr);
			k = right_nitems - k;
			right_streak = k;
			if (k) {
				dest -= k * size;
				right -= k * size;
				// memcpy(dest + size, right + size, k * size);
				for (size_t i = k; i > 0; i--) {
					memcpy_v(dest + i * size, right + i * size, size);
				}
				right_nitems -= k;
				if (right_nitems == 1) {
					goto copy_lone_right;
				}
				// From tim peter's comment:
				// right_nitems == 0 is impossible now if the comparison
				// function is consistent, but we can't assume that it is.
				//                         ------------------------------- <- huh ???
				if (right_nitems == 0) {
					goto copy_leftover_right;
				}
			}
			memcpy_v(dest, left, size);
			dest -= size;
			left -= size;
			left_nitems--;
			if (left_nitems == 0) {
				goto copy_leftover_right;
			}
		} while (left_streak >= MIN_GALLOP || right_streak >= MIN_GALLOP);
		min_gallop++; // "penalize it for leaving galloping mode" -Tim Peters
		state -> min_gallop = min_gallop;
	}
copy_leftover_right:
	if (right_nitems) {
		// memcpy(dest - (right_nitems - 1) * size, baseb, right_nitems * size);
		for (size_t i = right_nitems; i > 0; i--) {
			memcpy_v(dest - (right_nitems - 1) * size + (i - 1) * size, baseb + (i - 1) * size, size);
		}
	}
	return;
copy_lone_right:
	assert(right_nitems == 1 && left_nitems > 0);
	// The first element of right belongs at the front of the merge.
	dest -= (left_nitems - 1) * size;
	left -= (left_nitems - 1) * size;
	// memmove(dest, left, left_nitems * size);
	for (size_t i = left_nitems; i > 0; i--) {
		memmove_v(dest + (i - 1) * size, left + (i - 1) * size, size);
	}
	dest -= size;
	left -= size;
	memcpy_v(dest, right, size);
}

static void merge_last_runs(merge_state* state, char trdandsec, const size_t size, int (*compr)(const void*, const void*)) {
	assert(state != NULL);
	assert(state -> num_items >= 2);

	size_t k;
	array_run *pending = state -> pending_stack;
	const size_t i = state -> num_items - (trdandsec ? 3 : 2);
	array_run a, b;
	a.start_ptr = pending[i].start_ptr;
	a.nitems = pending[i].nitems;
	a.size = pending[i].size;
	b.start_ptr = pending[i + 1].start_ptr;
	b.nitems = pending[i + 1].nitems;
	b.size = pending[i + 1].size;
	// array_run *a = &(pending[i]) , *b = &(pending[i + 1]);

	assert(a.start_ptr + a.nitems * a.size == b.start_ptr);
	
	pending[i].nitems = a.nitems + b.nitems;
	if (trdandsec) {
		pending[i + 1].start_ptr = pending[i + 2].start_ptr;
		pending[i + 1].nitems = pending[i + 2].nitems;
		pending[i + 1].size = pending[i + 2].size;
	}
	state -> num_items--;

	// Where does b start in a?  Elements in a before that can be ignored (already in place).
	k = gallop_right(state, b.start_ptr, a.start_ptr, a.nitems, 0, size, compr); // TOCHECK
	assert(k >= 0);
	array_run_advance(&a, k);
	if (a.nitems == 0) {
		return;
	}

	// Where does a end in b?  Elements in b after that can be ignored (already in place).
	b.nitems = gallop_left(state, a.start_ptr + (a.nitems - 1) * a.size, b.start_ptr, b.nitems, b.nitems - 1, size, compr); // TOCHECK
	if (b.nitems == 0) {
		return;
	}

	if (a.nitems <= b.nitems) {
		merge_from_front(state, &a, &b, size, compr); // TOCHECK
	} else {
		merge_from_back(state, &a, &b, size, compr); // TOCHECK
	}
}

static void merge_collapse(merge_state* state, const size_t size, int (*compr)(const void*, const void*)) {
	assert(state != NULL);
	array_run *pending = state -> pending_stack;
	while (state -> num_items > 1) {
		size_t n = state -> num_items - 2;

		if ((n > 0 && pending[n - 1].nitems <= pending[n].nitems + pending[n + 1].nitems) || (n > 1 && pending[n - 2].nitems <= pending[n - 1].nitems + pending[n].nitems)) {
			/*if (pending[n - 1].nitems < pending[n + 1].nitems) {
				n--;
			}*/
			merge_last_runs(state, pending[n - 1].nitems < pending[n + 1].nitems/* Merge 3rd and 2nd last */, size, compr); // TOCHECK
		} else if (pending[n].nitems <= pending[n + 1].nitems) {
			merge_last_runs(state, 0/* Merge 2nd and last */, size, compr); // TOCHECK
		} else {
			break;
		}
	}
}

static void merge_force_collapse(merge_state* state, const size_t size, int (*compr)(const void*, const void*)) {
	assert(state != NULL);
	array_run *pending = state -> pending_stack;
	while (state -> num_items > 1) {
		size_t n = state -> num_items - 2;
		merge_last_runs(state, n > 0 && pending[n - 1].nitems < pending[n + 1].nitems, size, compr);
	}
}

static void merge_state_init(merge_state *ms, void *aux_array, size_t nitems_aux) {
	assert(ms != NULL);
	ms -> num_items = 0;
	ms -> stack_capacity = MAX_MERGE_PENDING;
	ms -> min_gallop = MIN_GALLOP;
	ms -> aux_array = aux_array;
	ms -> aux_array_capacity = nitems_aux;
}

void tsort_w_aux(void *list, size_t nitems, void *aux_array, size_t nitems_aux, size_t size, int (*compr)(const void*, const void*)) {
	merge_state ms;
	size_t nremaining;
	size_t minrun;
	array_run lo;

	lo.start_ptr = list;
	lo.nitems = nitems;
	lo.size = size;

	merge_state_init(&ms, aux_array, nitems_aux);

	nremaining = nitems;
	if (nremaining < 2) {
		return;
	}

	// INFO: Look up on reverse sort stability.
	// In original timsort implementation:
	// Reverse sort stability achieved by initially reversing the list,
    // applying a stable forward sort, then reversing the final result.

	minrun = compute_minrun(nremaining);
	// minrun = 2;
	do {
		char descending;
		size_t n = count_run(lo.start_ptr, lo.start_ptr + lo.size * lo.nitems, lo.size, &descending, compr);
		assert(n >= 0);
		if (descending) {
			reverse_arr(lo.start_ptr, n, lo.size);
		}
		if (n < minrun) {
			const size_t force = nremaining <= minrun ? nremaining : minrun;
			bisort(lo.start_ptr, force, n, size, compr); // BUG
			n = force;
		}
		assert(ms.num_items < MAX_MERGE_PENDING);
		ms.pending_stack[ms.num_items].start_ptr = lo.start_ptr;
		ms.pending_stack[ms.num_items].nitems = n;
		ms.pending_stack[ms.num_items].size = size;
		ms.num_items++;
		merge_collapse(&ms, size, compr); // TOCHECK
		array_run_advance(&lo, n);
		nremaining -= n;
	} while (nremaining);

	merge_force_collapse(&ms, size, compr); // TODO
	assert(ms.num_items == 1);
	assert(ms.pending_stack[0].nitems == nitems);
}

/**
 * Stable sorting algorithm.
 * This is not thread safe!
 */
void tsort(void* list, size_t nitems, size_t size, int (*compr)(const void*, const void*)) {
	void* aux_array = malloc(nitems * size);
	tsort_w_aux(list, nitems, aux_array, nitems, size, compr);
	free(aux_array);
}

int main(int argc, char** argv) {
	return perform_sort_visual_args(argc, argv, tsort, "timsort (python's sorted function)");
}
