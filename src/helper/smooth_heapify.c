/**
 * Smooth heapify from https://stackoverflow.com/questions/1390832/how-to-sort-nearly-sorted-array-in-the-fastest-time-possible-java/28352545#28352545
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../terminal_visualiser.h"

#include "smooth_heapify.h"

static size_t number_of_trailing_zeros(size_t i) {
	size_t y;
	size_t n = sizeof(size_t) * 8;
	if (i == 0) {
		return n;
	}
	n--;
	for (size_t d = (n + 1) / 2; d > 1; d /= 2) {
		y = i << d;
		if (y != 0) {
			n -= d;
			i = y;
		}
		assert(d % 2 == 0);
	}
	return n - ((i << 1) >> (sizeof(size_t) * 8 - 1));
}

typedef struct {
	size_t pshift;
	size_t lps;   // L(pshift)
	size_t lpsm1; // L(pshift - 1)
	size_t lpsm2; // L(pshift - 2)
} pshift_t;

#define new_pshift_t_inst {.pshift = 1, .lps = 1, .lpsm1 = 1, .lpsm2 = 1}

static void shift_up(pshift_t *pshift, size_t byn) {
	size_t ini_pshift = pshift -> pshift;
	pshift -> pshift += byn;
	if (ini_pshift == 0 && byn > 0) {
		byn--;
	}
	for (size_t i = 0; i < byn; i++) {
		pshift -> lpsm2 = pshift -> lpsm1;
		pshift -> lpsm1 = pshift -> lps;
		pshift -> lps = pshift -> lpsm1 + pshift -> lpsm2 + 1;
	}
}

static void shift_down(pshift_t *pshift, size_t byn) {
	size_t ini_pshift = pshift -> pshift;
	assert(ini_pshift >= byn);
	pshift -> pshift -= byn;
	for (size_t i = 0; i < byn; i++) {
		pshift -> lps = pshift -> lpsm1;
		pshift -> lpsm1 = pshift -> lpsm2;
		pshift -> lpsm2 = pshift -> lps <= 1 ? 1 : (pshift -> lps - pshift -> lpsm1 - 1);
	}
	assert(pshift -> lpsm2 <= pshift -> lpsm1);
}

static void sift(int *display_array, int display_array_len, pshift_t pshift, size_t head) {
	/* 
	 * we do not use Floyd's improvements to the heapsort sift, because we
	 * are not doing what heapsort does - always moving nodes from near
	 * the bottom of the tree to the root.
	 */

	assert(head < display_array_len);

	int val = display_array[head];
	print_array_bars("Performing smooth heapify", display_array + head, 0, NULL, 0, 1);

	while (pshift.pshift > 1) {
		size_t rt = head - 1;
		size_t lf = head - 1 - pshift.lpsm2;

		assert(rt < display_array_len);
		assert(lf < display_array_len);

		char left_leq = display_array[lf] >= display_array[rt];
		mark_comparison();
		print_array_bars("Performing smooth heapify", display_array + lf, 1, display_array + rt, 1, 1);

		if (left_leq) {
			mark_comparison();
			print_array_bars("Performing smooth heapify", display_array + lf, 1, NULL, 0, 1);
			if (display_array[lf] < val) {
				break;
			}
			if (head != lf) {
				display_array[head] = display_array[lf];
				mark_array_write();
				print_array_bars("Performing smooth heapify", display_array + head, 1, display_array + lf, 1, 1);
			}
			head = lf;
			shift_down(&pshift, 1);
		} else {
			mark_comparison();
			print_array_bars("Performing smooth heapify", display_array + rt, 1, NULL, 0, 1);
			if (display_array[rt] < val) {
				break;
			}
			if (head != rt) {
				display_array[head] = display_array[rt];
				mark_array_write();
				print_array_bars("Performing smooth heapify", display_array + head, 1, display_array + lf, 1, 1);
			}
			head = rt;
			shift_down(&pshift, 2);
		}
	}
	display_array[head] = val;
	mark_array_write();
	print_array_bars("Performing smooth heapify", display_array + head, 1, NULL, 0, 1);
}

static void trinkle(int *display_array, int display_array_len, size_t p, pshift_t pshift, size_t head, char is_trusty) {
	assert(head < display_array_len);

	int val = display_array[head];
	print_array_bars("Performing smooth heapify", display_array + head, 0, NULL, 0, 1);

	while (p != 1) {
		size_t stepson = head - pshift.lps;
		mark_comparison();
		print_array_bars("Performing smooth heapify", display_array + stepson, 1, NULL, 0, 1);
		if (display_array[stepson] <= val) {
			break; // current node is greater than head. Sift.
		}

		/*
		 * no need to check this if we know the current node is trusty,
		 * because we just checked the head (which is val, in the first
		 * iteration)
		 */
		if (!is_trusty && pshift.pshift > 1) {
			size_t rt = head - 1;
			size_t lf = head - 1 - pshift.lpsm2;

			assert(rt < display_array_len);
			assert(lf < display_array_len);

			mark_comparison();
			print_array_bars("Performing smooth heapify", display_array + stepson, 1, display_array + rt, 1, 1);
			if (display_array[rt] >= display_array[stepson]) {
				break;
			}

			mark_comparison();
			print_array_bars("Performing smooth heapify", display_array + stepson, 1, display_array + lf, 1, 1);
			if (display_array[lf] >= display_array[stepson]) {
				break;
			}
		}

		display_array[head] = display_array[stepson];
		mark_array_write();
		print_array_bars("Performing smooth heapify", display_array + head, 1, display_array + stepson, 1, 1);

		head = stepson;
		size_t trail = number_of_trailing_zeros(p & ~1);
		p >>= trail;
		shift_up(&pshift, trail);
		is_trusty = 0;
	}

	if (!is_trusty) {
		display_array[head] = val;
		mark_array_write();
		print_array_bars("Performing smooth heapify", display_array + head, 1, NULL, 0, 1);
	}
}

void smooth_heapify(int *display_array, int display_array_len) {
	/*
	 * These variables need a little explaining. If our string of heaps
	 * is of length 38, then the heaps will be of size 25+9+3+1, which are
	 * Leonardo numbers 6, 4, 2, 1.
	 * Turning this into a binary number by setting the seventh last bit (the
	 * rightmost bit is 0, so the seventh bit index is 6), fifth, third and
	 * second, we get b01010110 = 0x56.
	 *
	 *  0000 0000 0101 0110 <- last bit is index 0, second-last is 1 and 
	 *  ^          ^ ^  ^^     so on...
	 * 15          6 4  21
	 *
	 * We represent this number as a pair of numbers by right-shifting all 
	 * the zeros and storing the mantissa and exponent as "p" and "pshift".
	 * This is handy, because the exponent is the index into L[] giving the
	 * size of the rightmost heap, and because we can instantly find out if
	 * the rightmost two heaps are consecutive Leonardo numbers by checking
	 * (p&3)==3
	 */
	
	size_t p = 1; // the bitmap of the current standard concatenation >> pshift
	pshift_t pshift = new_pshift_t_inst;

	size_t hi = display_array_len - 1;
	size_t head = 0;

	while (head < hi) {
		if ((p & 3) == 3) {
			// Add 1 by merging the first two blocks into a larger one.
			// The next Leonardo number is one bigger.
			sift(display_array, display_array_len, pshift, head);
			p >>= 2;
			shift_up(&pshift, 2);
		} else {
			// adding a new block of length 1
			if (pshift.lpsm1 >= hi - head) {
				// this block is its final size.
				trinkle(display_array, display_array_len, p, pshift, head, 0);
			} else {
				// this block will get merged. Just make it trusty.
				sift(display_array, display_array_len, pshift, head);
			}

			if (pshift.pshift == 1) {
				// LP[1] is being used, so we add use LP[0]
				p <<= 1;
				pshift.pshift--;
				assert(pshift.lps == 1);
				assert(pshift.lpsm1 == 1);
				assert(pshift.lpsm2 == 1);
			} else {
				// shift out to position 1, add LP[1]
				p <<= (pshift.pshift - 1);
				pshift.pshift = 1;
				pshift.lps = 1;
				pshift.lpsm1 = 1;
				pshift.lpsm2 = 1;
			}
		}
		p |= 1;
		head++;
	}
}
