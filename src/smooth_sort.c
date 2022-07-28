/**
 * This smoothsort algorithm is adopted from the implementation that was formerly available on Wikipedia.
 * The algorithm was eventually removed for unknown reasons. Someone managed to cache the implementation
 * on a StackOverflow forum.
 * https://stackoverflow.com/questions/1390832/how-to-sort-nearly-sorted-array-in-the-fastest-time-possible-java/28352545#28352545
 *
 * The author who made this implementation is unknown, and also whether did they demanded retraction of
 * code due to a change in their policy of copyright for this code.
 *
 * Similar code can also be found in existing sorting visualizer projects like Timo Bingmann's "Sound of Sorting", written in C++.
 * https://panthema.net/2013/sound-of-sorting/
 *
 * In this project, Snqzs' PG had made two modifications to the implementation found on the Internet:
 *     1 - To eliminate the need of a pre-defined array of Leonardo's numbers. (This negatively impacted the performance)
 *     2 - Reducing the number of comparisons needed for sifting down from three to two.
 *
 * -- 1. Eliminate the need of a pre-defined array --
 * An attempt was made to remove the need of a pre-defined array of values
 * that is in the adopted code (formerly) from Wikipedia. It took a painstaking amount
 * of hours to work out a system to shift not just the "pshift" values, but the
 * Leonardo's numbers to match the "pshift"-th term in the sequence. 
 *
 * The idea is to introduce a struct that contains the pshift value, along with the
 * values of LP[pshift], LP[pshift - 1] and LP[pshift - 2]. When pshift is shifted,
 * the three Leonardo's numbers can be "shifted" thanks to their inductive nature.
 * The process can be seen in the shift_up and shift_down functions below.
 *
 * While eventually it managed to do away the need for that list of Leonardo numbers,
 * it did impact the performance relatively heavily.
 *
 * -- 2. Sifting down --
 *
 * While looking at the code from the implementation, the author had made three
 * comparisons for the sifting down procedure. The first two comparisons is to check
 * if the current top element needs to be swapped with each of its two children, one
 * comparison for each child. Then the last comparison is to determine which child
 * to swap with. Note that this doesn't always take three comparisons, sometimes it
 * only takes two.
 *
 * The second modification changes the order of comparisons during sift-down. The
 * children are compared first to determine the larger child, then the larger child
 * is compared to the parent. Because we are swapping only with the larger child, 
 * the comparison with the smaller child is not needed. Compared to the original
 * implementation, this method consistently takes two comparisons.
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "terminal_visualiser.h"

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

static void sift(void *list, size_t nitems, size_t size, pshift_t pshift, size_t head, int (*compr)(const void*, const void*)) {
	/* 
	 * we do not use Floyd's improvements to the heapsort sift, because we
	 * are not doing what heapsort does - always moving nodes from near
	 * the bottom of the tree to the root.
	 */

	assert(head < nitems);

	char val[size];
	memcpy_v(val, list + head * size, size);

	while (pshift.pshift > 1) {
		size_t rt = head - 1;
		size_t lf = head - 1 - pshift.lpsm2;

		assert(rt < nitems);
		assert(lf < nitems);

		char left_leq = compr(list + lf * size, list + rt * size) >= 0;

		if (left_leq ? (compr(val, list + lf * size) >= 0) : (compr(val, list + rt * size) >= 0)) {
			break;
		}
		if (left_leq) {
			if (head != lf) {
				memcpy_v(list + head * size, list + lf * size, size);
			}
			head = lf;
			shift_down(&pshift, 1);
		} else {
			if (head != rt) {
				memcpy_v(list + head * size, list + rt * size, size);
			}
			head = rt;
			shift_down(&pshift, 2);
		}
	}
	memcpy_v(list + head * size, val, size);
}

static void trinkle(void *list, size_t nitems, size_t size, size_t p, pshift_t pshift, size_t head, char is_trusty, int (*compr)(const void*, const void*)) {
	assert(head < nitems);

	char val[size];
	memcpy_v(val, list + head * size, size);

	while (p != 1) {
		size_t stepson = head - pshift.lps;
		if (compr(list + stepson * size, val) <= 0) {
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

			assert(rt < nitems);
			assert(lf < nitems);

			if (compr(list + rt * size, list + stepson * size) >= 0 || compr(list + lf * size, list + stepson * size) >= 0) {
				break;
			}
		}

		memcpy_v(list + head * size, list + stepson * size, size);

		head = stepson;
		size_t trail = number_of_trailing_zeros(p & ~1);
		p >>= trail;
		shift_up(&pshift, trail);
		is_trusty = 0;
	}

	if (!is_trusty) {
		memcpy_v(list + head * size, val, size);
		sift(list, nitems, size, pshift, head, compr);
	}
}

void ssort(void *list, size_t nitems, size_t size, int (*compr)(const void*, const void*)) {
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

	size_t hi = nitems - 1;
	size_t head = 0;

	while (head < hi) {
		if ((p & 3) == 3) {
			// Add 1 by merging the first two blocks into a larger one.
			// The next Leonardo number is one bigger.
			sift(list, nitems, size, pshift, head, compr);
			p >>= 2;
			shift_up(&pshift, 2);
		} else {
			// adding a new block of length 1
			if (pshift.lpsm1 >= hi - head) {
				// this block is its final size.
				trinkle(list, nitems, size, p, pshift, head, 0, compr);
			} else {
				// this block will get merged. Just make it trusty.
				sift(list, nitems, size, pshift, head, compr);
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

	trinkle(list, nitems, size, p, pshift, head, 0, compr);

	while (pshift.pshift != 1 || p != 1) {
		if (pshift.pshift <= 1) {
			// block of length 1. No fiddling needed
			size_t trail = number_of_trailing_zeros(p & ~1);
			p >>= trail;
			shift_up(&pshift, trail);
		} else {
			p <<= 2;
			p ^= 7;
			shift_down(&pshift, 2);

			// This block gets broken into three bits. The rightmost
			// bit is a block of length 1. The left hand part is split into
			// two, a block of length LP[pshift+1] and one of LP[pshift].
			// Both these two are appropriately heapified, but the root
			// nodes are not necessarily in order. We therefore semitrinkle
			// both of them

			pshift_t pshift1 = pshift;
			shift_up(&pshift1, 1);
			trinkle(list, nitems, size, p >> 1, pshift1, head - pshift.lps - 1, 1, compr);
			trinkle(list, nitems, size, p, pshift, head - 1, 1, compr);
		}
		head--;
	}
}

int main(int argc, char** argv) {
	return vis_main_comparison(argc, argv, "smooth sort", ssort);
}
