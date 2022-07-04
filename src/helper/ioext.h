#ifndef IOEXT_H_INCLUDED
#define IOEXT_H_INCLUDED

#include <stdio.h>

void vfprint_truncated_f(FILE* f, const char* fmt, size_t limit, va_list args);
void fprint_truncated_f(FILE* f, const char* fmt, size_t limit, ...);
void print_truncated_f(const char* fmt, size_t limit, ...);
void vfprint_wraped_f(FILE* f, const char* line, size_t limit, int indentation, int indent_size, va_list args);
void vfprint_wraped_linef(FILE* f, const char* line, size_t limit, int indentation, int indent_size, va_list args);
void print_wraped_linef(const char* line, size_t limit, int indentation, int indent_size, ...);
void ioext_init(size_t memsize);
void ioext_cleanup();

#endif // IOEXT_H_INCLUDED