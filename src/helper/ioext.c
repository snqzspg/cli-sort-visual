#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ioext.h"

/**
 * Returns the number of chars copied (not including newlines).
 */
static size_t cpy_to_next_eof(char* dest, const char* line, int limit) {
	size_t i;
	for (i = 0; *line != '\n' && *line != '\0' && limit; line++, dest++, limit--, i++) {
		*dest = *line;
	}
	*dest = '\0';
	return i;
}

//static static count_to_next_space(const char* line) {
//	size_t count = 0;
//	for (; strchr(" \t\r\n\v\f", *line) == NULL && *line != '\0'; line++) {
//		count++;
//	}
//	return count;
//}

/**
 * The following macro is not necessary for GCC, but it is a solution for MSVC compilers.
 * https://stackoverflow.com/questions/558223/va-copy-porting-to-visual-c
 */
#ifndef va_copy
#define va_copy(d, s) ((d) = (s))
#endif // va_copy

static size_t numchars = 0;

static char* fmtbuf = NULL;
static int fmtbuf_alloc = 0;

/**
 * Delete using free()
 */
static char* get_fmttd_line(const char* fmt, va_list args) {
	// size_t buffer = strlen(fmt) + 1;
	size_t len = 0;
	va_list stash;
	va_copy(stash, args);
	// char* ret = NULL;
	if (fmtbuf != NULL) {
		vsnprintf(fmtbuf, fmtbuf_alloc, fmt, args);
		fmtbuf[fmtbuf_alloc - 1] = '\0';
		len = strlen(fmtbuf);
		va_end(args);
	}
	while (len >= fmtbuf_alloc - 1) {
		va_copy(args, stash);
		if (fmtbuf_alloc == 0) {
			fmtbuf_alloc = 128;
		} else {
			fmtbuf_alloc *= 2;
		}
		fmtbuf = realloc(fmtbuf, sizeof(char) * fmtbuf_alloc);
		vsnprintf(fmtbuf, fmtbuf_alloc, fmt, args);
		fmtbuf[fmtbuf_alloc - 1] = '\0';
		len = strlen(fmtbuf);
		va_end(args);
	}
	va_end(stash);
	return fmtbuf;
}

void vfprint_truncated_f(FILE* f, const char* fmt, size_t limit, va_list args) {
	char* fmttd = get_fmttd_line(fmt, args);
	char* ptr = fmttd;
	size_t cols = limit - 1;
	char tmp[cols + 1];
	tmp[cols] = '\0';
	size_t copied = 0;
	for (; *ptr != '\0'; ptr += copied) {
		if (*ptr == '\n') {
			copied = 1;
			fputs("\n", f);
			numchars = 0;
			continue;
		}
		copied = cpy_to_next_eof(tmp, ptr, cols);
		numchars += copied;
		if (numchars <= limit) {
			fputs(tmp, f);
		}
	}
	// free(fmttd);
}

void fprint_truncated_f(FILE* f, const char* fmt, size_t limit, ...) {
	va_list args;
	va_start(args, limit);
	vfprint_truncated_f(f, fmt, limit, args);
	va_end(args);
}

void print_truncated_f(const char* fmt, size_t limit, ...) {
	va_list args;
	va_start(args, limit);
	// vfprint_truncated_f(stdout, fmt, limit, args);
	vprintf(fmt, args);
	va_end(args);
}

void vfprint_wraped_f(FILE* f, const char* line, size_t limit, int indentation, int indent_size, va_list args) {
	char* fmttd = get_fmttd_line(line, args);
	char* ptr = fmttd;
	indentation *= indent_size;
	const int cols = limit - 1;
	const int space = cols - indentation;
	char tmpstack[cols + 1];
	tmpstack[cols] = '\0';
	size_t copied = 0;
	for (; *ptr != '\0'; ptr += copied) {
		if (*ptr == '\n') {
			copied = 1;
			fprintf(f, "\n");
			continue;
		}
		for (size_t i = 0; i < indentation; i++) {
			tmpstack[i] = ' ';
		}
		copied = cpy_to_next_eof(&(tmpstack[indentation]), ptr, space);
		fprintf(f, "%s%s", tmpstack, copied < space ? "" : "\n");
	}
	// free(fmttd);
}

void vfprint_wraped_linef(FILE* f, const char* line, size_t limit, int indentation, int indent_size, va_list args) {
	vfprint_wraped_f(f, line, limit, indentation, indent_size, args);
	fprintf(f, "\n");
}

void print_wraped_linef(const char* line, size_t limit, int indentation, int indent_size, ...) {
	va_list args;
	va_start(args, indent_size);
	vfprint_wraped_linef(stdout, line, limit, indentation, indent_size, args);
	va_end(args);
}

void ioext_init(size_t memsize) {
	fmtbuf = (char *) malloc(sizeof(char) * memsize);
	fmtbuf_alloc = memsize;
}

void ioext_cleanup() {
	free(fmtbuf);
	fmtbuf = NULL;
	fmtbuf_alloc = 0;
}
