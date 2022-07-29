#ifndef TERMINAL_VISUALISER_H_INCLUDED
#define TERMINAL_VISUALISER_H_INCLUDED

// int perform_sort_visual(int delay, int arr_len, const char* shuffle_type, const char* chord_arg, double maxamp, char* wave_type, void (*sort_algo)(void* list, size_t nitems, size_t size, int (*compr)(const void*, const void*)), const char* algo_name);
// int perform_radix_sort_visual(int delay, int arr_len, const char* shuffle_type, size_t unsigned_base, const char* chord_arg, double maxamp, char* wave_type, void (*sort_algo)(void*, size_t, size_t, const size_t, size_t (*num_digits)(const void*, size_t), size_t (*get_nth_digit)(const void*, size_t)), const char* algo_name);

typedef struct {
	char should_parse;
	char *name;
	char *info;
	void *buffer;
	void *default_value;
	int (*parse_arg) (void*, const char*, const void*, size_t);
	size_t size;
} vis_arg_t;

/**
 * Return 0 to indicate sort success, otherwise to indicate failure.
 */
typedef int (*call_sort_t)(void* list, size_t nitems, size_t size, int (*compr)(const void*, const void*), size_t nvis_args, vis_arg_t *vis_args);

/**
 * This struct will be used as the item to sort instead of the primitive integers.
 * num stores the number the sorting algorithm should read to sort.
 * tag is an int that will follow the num when the struct array is sorted.
 */
typedef struct {
	int num;
	int tag;
} vis_int_t;

// ========== Main helper functions ==========

int call_comparison_sort(void* list, size_t nitems, size_t size, int (*compr)(const void*, const void*), size_t nvis_args, vis_arg_t *vis_args);
int call_radix_sort(void* list, size_t nitems, size_t size, int (*compr)(const void*, const void*), size_t nvis_args, vis_arg_t *vis_args);

// ========== End main helper functions ==========

// ========== Main functions (Call these functions in your main()) ==========

/**
 * @brief Creates a vis_arg_t object that can be passed into the vis_main functions later.
 * Each vis_arg_t object is equivilant to an extra parameter that a sorting algorithm takes, and it can also be set to take input from user.
 * If the object is to receive input from user, the should_parse argument should be 1.
 *     The name, info and default_value cannot be set to NULL.
 * If the object should not receive any input from user, the should_parse argument should be 0.
 *     The name, info and default_value can be set to NULL.
 * When should_parse is 0, the argument object will be left untouched when passed into the vis_main functions.
 * The same object will be passed as an array into your call_sorts method. (See vis_main_visargs).
 *
 * @param should_parse Whether this argument should be a command-line argument that can be set by the user. Can be set to 0 or 1. If 0, the argument will be untouched.
 * @param name A pointer to a string for the name of the argument to be displayed on the help page. If (and only if) should_parse is set to 0 then this can be set to NULL.
 * @param info A pointer to a string for the description of the argument to be displayed on the help page. If (and only if) should_parse is set to 0 then this can be set to NULL.
 * @param buffer A pointer to a variable that the user entered argument will be stored.
 * @param default_value A pointer to a variable that holds the default value should user choses to leave empty. If (and only if) should_parse is set to 0 then this can be set to NULL.
 * @param size The number of bytes both buffer and default_value takes.
 * @param parse_arg A function that converts a string argument into a desired value to be stored in the first argument. Return 0 if successful, 1 if failed.
 * @return A vis_arg_t object that can be passed into a vis_main function.
 */
vis_arg_t set_vis_arg(char should_parse, char *name, char *info, void *buffer, void *default_value, size_t size, int (*parse_arg) (void*, const char*, const void*, size_t));

/**
 * @brief This is the main function that kick start the sorting visualizer.
 *        There are many varients of this function beginning with 'vis_main' that starts the visualizer as well.
 * 
 * @param argc This is the argument count from your main function.
 * @param argv This is the argument vector from your main function.
 * @param algo_name The name of the algorithm to be displayed on the top left.
 * @param call_sort A function pointer that points to the function that calls the sorting algorithm. See call_comparison_sort above for an example.
 * @param nvis_args The number of extra parameters to pass to your sorts. 
 * @param vis_args An array of vis_arg_t objects that will be passed to the call_sort function. (See set_vis_arg above).
 *        Each object can be set to receive input from user, which will then be passed into the call_sort function.
 *        The order the objects given will be the same order passed into the call_sort function.
 */
int vis_main_visargs(int argc, char** argv, const char* algo_name, call_sort_t call_sort, size_t nvis_args, vis_arg_t *vis_args);

/**
 * @brief This is a main function that kick start the sorting visualizer.
 *        This can be wrapped with functions that take in varadic arguments.
 * 
 * @param argc This is the argument count from your main function.
 * @param argv This is the argument vector from your main function.
 * @param algo_name The name of the algorithm to be displayed on the top left.
 * @param call_sort A function pointer that points to the function that calls the sorting algorithm. See call_comparison_sort above for an example.
 * @param nvis_args The number of extra parameters to pass to your sorts. 
 * @param vis_args A va_list of POINTERS to vis_arg_t objects that will be passed to the call_sort function. (See set_vis_arg above).
 *        Each object can be set to receive input from user, which will then be passed into the call_sort function.
 *        The order the objects given will be the same order passed into the call_sort function.
 *        The number of vis_args objects given must match the count given in nvis_args.
 */
int vis_main_vargs(int argc, char** argv, const char* algo_name, call_sort_t call_sort, size_t nvis_args, va_list vis_args);

/**
 * @brief This is a main function that kick start the sorting visualizer.
 * 
 * @param argc This is the argument count from your main function.
 * @param argv This is the argument vector from your main function.
 * @param algo_name The name of the algorithm to be displayed on the top left.
 * @param call_sort A function pointer that points to the function that calls the sorting algorithm. See call_comparison_sort above for an example.
 * @param nvis_args The number of extra parameters to pass to your sorts. 
 * @param ... POINTERS to vis_arg_t objects that will be passed to the call_sort function. (See set_vis_arg above).
 *        Each object can be set to receive input from user, which will then be passed into the call_sort function.
 *        The order the objects given will be the same order passed into the call_sort function.
 *        The number of vis_args objects given must match the count given in nvis_args.
 */
int vis_main(int argc, char** argv, const char* algo_name, call_sort_t call_sort, size_t nvis_args, ...);

/**
 * @brief This is a main function that kick start the sorting visualizer.
 *        This is specially designed for comparison sorts, which is the vast majority of sorts.
 * 
 * @param argc This is the argument count from your main function.
 * @param argv This is the argument vector from your main function.
 * @param algo_name The name of the algorithm to be displayed on the top left.
 * @param sort_algo A function pointer that points to the sorting function. The function must take in the same parameters as stdlib.h's qsort.
 */
int vis_main_comparison(int argc, char** argv, const char* algo_name, void (*sort_algo)(void* list, size_t nitems, size_t size, int (*compr)(const void*, const void*)));

/**
 * @brief This is a main function that kick start the sorting visualizer.
 *        This is specially designed for unsigned radix sorts.
 * 
 * @param argc This is the argument count from your main function.
 * @param argv This is the argument vector from your main function.
 * @param default_radix_base The default base for the algorithm.
 * @param algo_name The name of the algorithm to be displayed on the top left.
 * @param sort_algo A function pointer that points to the sorting function. The function must take in a list as void pointer, nitems, size, an unsigned base, num_digits (see int_num_digits below) and get_nth_digit (see base_n_modulo below) functions.
 */
int vis_main_radix(int argc, char** argv, size_t default_radix_base, const char* algo_name, void (*sort_algo)(void*, size_t, size_t, const size_t, size_t (*num_digits)(const void*, size_t), size_t (*get_nth_digit)(const void*, size_t)));

/**
 * @deprecated Use vis_main_comparison instead.
 */
int perform_sort_visual_args(int argc, char** argv, void (*sort_algo)(void* list, size_t nitems, size_t size, int (*compr)(const void*, const void*)), const char* algo_name);

/**
 * @deprecated Use vis_main_radix instead.
 */
int perform_radix_sort_visual_args(int argc, char** argv, size_t default_radix_base, void (*sort_algo)(void*, size_t, size_t, const size_t, size_t (*num_digits)(const void*, size_t), size_t (*get_nth_digit)(const void*, size_t)), const char* algo_name);

// ========== End main functions ==========

void print_array_bars(const char *prefix, const void *p1, char is_p1_comparison, const void *p2, char is_p2_comparison, char in_progress);
void print_array_bars_not_compare(const void *p1, const void *p2);

void mark_comparison();
void mark_array_write();
void mark_aux_array_write();

void memcpy_v(void *dest, const void *src, const size_t n);
void memmove_v(void *dest, const void *src, const size_t n);
void remove_varray_item(int *ptr);
void set_v_array_len(int n);

void reset_comparisons();
void reset_writes();
void reset_delays();
void reset_time();
void reset_counters();

#endif // TERMINAL_VISUALISER_H_INCLUDED
