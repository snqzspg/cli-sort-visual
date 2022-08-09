#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "terminal_visualiser.h"

#include "helper/precise_and_acc_time.h"
#include "helper/beep_log.h"
#include "helper/anti_quicksort.h"
#include "helper/ioext.h"
#include "helper/triangular_input.h"
#include "helper/smooth_heapify.h"

#ifdef PA_INSTALLED
#include "helper/sound_player.h"
#endif

#ifndef DEFAULT_DIS_DELAY
#define DEFAULT_DIS_DELAY 50
#endif

#ifndef DEFAULT_ARR_LEN
#define DEFAULT_ARR_LEN 32
#endif

#ifndef BAR_CHAR
#define BAR_CHAR ':'
#endif

#ifndef ACTIVE_BAR_CHAR
#define ACTIVE_BAR_CHAR '+'
#endif

#ifndef COMPARE_BAR_CHAR
#define COMPARE_BAR_CHAR '#'
#endif

#ifndef VIS_LOWER_CENT_BOUND
#define VIS_LOWER_CENT_BOUND -2100
#endif
#ifndef VIS_LOWER_SEMITONE_BOUND
#define VIS_LOWER_SEMITONE_BOUND -21
#endif
#ifndef VIS_HIGHER_CENT_BOUND
#define VIS_HIGHER_CENT_BOUND 2700
#endif
#ifndef VIS_HIGHER_SEMITONE_BOUND
#define VIS_HIGHER_SEMITONE_BOUND 27
#endif

#ifndef CHORD_START_SEMITONE
#define CHORD_START_SEMITONE -21
#endif
#ifndef CHORD_OCTAVE_RANGE
#define CHORD_OCTAVE_RANGE 3
#endif

#ifndef IDEAL_CROTCHET_DUR_MS
#define IDEAL_CROTCHET_DUR_MS 500
#endif

#ifndef MIN_SOUND_LEN
#define MIN_SOUND_LEN 40
#endif

#ifndef MAX_AMPLITUDE
#define MAX_AMPLITUDE 0.1
#endif

#ifndef SHUFFLE_DELAY_FACTOR
#define SHUFFLE_DELAY_FACTOR 1.0
#endif

#ifndef VERIFY_DELAY_FACTOR
#define VERIFY_DELAY_FACTOR 1.0
#endif

#ifndef PRINT_PRECISE_TIME
#define PRINT_PRECISE_TIME 0
#endif

#ifndef PRINT_TIME_W_O_DELAY
#define PRINT_TIME_W_O_DELAY 1
#endif

#ifndef TUNING_FREQ
#define TUNING_FREQ 440.0
#endif

#ifndef N_CROTCHETS_IN_BAR
#define N_CROTCHETS_IN_BAR 4
#endif

#ifndef SETTINGS_FILE
#define SETTINGS_FILE "settings.txt"
#endif

#ifndef CHORD_PROGRESS_FILE
#define CHORD_PROGRESS_FILE "chord_progression.txt"
#endif

#ifndef USE_EQ_TEMP
#define USE_EQ_TEMP 1
#endif

#ifndef DISPLAY_FULL_BAR
#define DISPLAY_FULL_BAR 1
#endif

#ifndef ADDITIVE_FREQ_FILE
#define ADDITIVE_FREQ_FILE "sound_frequencies.txt"
#endif

#ifndef SHOW_VER
#define SHOW_VER 1
#endif

#ifndef SHOW_BUILD
#define SHOW_BUILD 0
#endif

#ifndef DISPLAY_JUPITERBJY_NOTATION
#define DISPLAY_JUPITERBJY_NOTATION 1
#endif

#ifndef DEF_N_SIMILAR_ITEMS
#define DEF_N_SIMILAR_ITEMS 1
#endif

static int bar_display_delay = DEFAULT_DIS_DELAY;

static vis_int_t *display_array = NULL;
static int display_array_len = 0;
static int non_array_lines = 15;

static char normal_bar_char = BAR_CHAR;
static char active_bar_char = ACTIVE_BAR_CHAR;
static char compare_bar_char = COMPARE_BAR_CHAR;

static const int chords[] = {
	I_chord, ii_chord, iii_chord, IV_chord, V_chord, vi_chord, vii_chord,
	i_min_chord, ii_dim_chord, III_maj_chord, iv_min_chord, v_min_chord, VI_maj_chord, VII_maj_chord,
	aug_chord_i, dim_chord_i,
	dim7_chord, min7_flat_5_chord, min7_chord, m_maj7_chord, dom7_chord, maj7_chord, aug7_chord, aug_maj7_chord,
	dom9_chord, dom11_chord, dom13_chord,
	sus2_chord, sus4_chord, jz_9sus4_chord,
	major_scale, neutral_minor_scale, melodic_minor_scale, harmonic_minor_scale
};
static const int chord_progression[8] = {I_chord, IV_chord, vi_chord, V_chord, I_chord, IV_chord, V_chord, I_chord};
static const int chord_progression_len = 8;
static int *custom_chord_progression = (int*) chord_progression;
static int custom_chord_progression_len = 8;
static int custom_chord_progression_alloc = 8;

static double max_sound_amplitude = MAX_AMPLITUDE;
static int tone_lower_cent_bound = VIS_LOWER_CENT_BOUND;
static int tone_upper_cent_bound = VIS_HIGHER_CENT_BOUND;
static int chord_start_semitone = CHORD_START_SEMITONE;
static int chord_octave_range = CHORD_OCTAVE_RANGE;
static int ideal_crotchet_dur = IDEAL_CROTCHET_DUR_MS;
static int min_sound_len = MIN_SOUND_LEN;
static double shuffle_delay_factor = SHUFFLE_DELAY_FACTOR;
static double verify_delay_factor = VERIFY_DELAY_FACTOR;
static char print_precise_time = PRINT_PRECISE_TIME;
static char print_estimated_real_time = PRINT_TIME_W_O_DELAY;
static double tuning_freq = TUNING_FREQ;
static char use_eq_temp = USE_EQ_TEMP;
static char display_full_bar = DISPLAY_FULL_BAR;
static char show_ver_no = SHOW_VER;
static char show_build_no = SHOW_BUILD;
static char display_jupiterbjy_numbers = DISPLAY_JUPITERBJY_NOTATION;
static char display_stability_test = 0;
static int stability_test_highest_tag = -1;

#define CHORD_MODE_OFF 0
#define CHORD_MODE_NOTE 1
#define CHORD_MODE_SINGLE 2
#define CHORD_MODE_SIMPLE_PROGRESSION 3

static int chord_mode = CHORD_MODE_OFF;
static int chord_no = I_chord;
static int chord_progression_timing_ms = 2000;
static int n_crotchets_in_bar = N_CROTCHETS_IN_BAR;

#define ERROR_HANDLE_GO_AHEAD 0
#define ERROR_HANDLE_WAIT 1
#define ERROR_HANDLE_USER_IN 2

#define error_pause(error_mode) switch (error_mode) { \
case ERROR_HANDLE_WAIT: \
	sleep_ms(5000); \
	break; \
case ERROR_HANDLE_USER_IN: \
	printf("Press <enter> to continue... "); while (getchar() != '\n'); \
	break; \
case ERROR_HANDLE_GO_AHEAD: \
default: \
	break; \
}

static int error_handle_mode = ERROR_HANDLE_USER_IN;

static const char* display_name = NULL;
static time_t start_time = 0;
static time_t vis_start_time = 0;
static int highest_item = 0;
static char include_sleep_time = 0;
static char* shuffle_name = NULL;
static size_t radix_base;
static mstime_t start_time_ms = 0;
static char log_sound = 1;
static char play_sound = 0;
static char is_sorting = 0;
static double delay_factor = 1.0;

static int last_nrows = 80;
static int last_ncols = 25;

static unsigned long comparisons = 0;
static char comparisons_overflow = 0;
static unsigned long writes = 0;
static char writes_overflow = 0;
static unsigned long aux_writes = 0;
static char aux_writes_overflow = 0;
static clock_t delay_secs = 0;
static char delay_secs_overflow = 0;

// ========== Helper functions breadcrumbs ==========

static void print_time(double secs, int ncols);
static void print_mstime(mstime_t msecs, int ncols);
static void print_time_long(time_t secs, int ncols);
static int find_max_int(vis_int_t* arr, size_t len);
void sleep_ms(int milliseconds);
static char should_include_sleep_time(int test_delay);
static void clear_screen_(char complete);
static int larger_int_back(const void* a, const void* b);
static int pgcg_get_console_rows();
static int pgcg_get_console_cols();
static size_t get_concat_sort_name_len(const char* prefix);
static void cpy_concat_sort_name(char* __restrict__ concat_buffer, const char* prefix);
static void print_version_top_right(const char *prefix, int ncols);
static char jupiterbjy_number_representation(int i);
static char jupiterbjy_number_representation2(int i);
static char pointer_in_array(const void* p);
static char is_help_arg(const char* arg);
static void print_usage_info(const char* arg0, size_t nvis_args, vis_arg_t *vis_args);
static size_t int_num_digits(const void* a, size_t b);
static size_t base_n_modulo(const void* a, size_t b);
static int get_chord_mode(const char *arg, int* __restrict__ chord_id);
static int get_chord_timing(int delay_ms);
static double get_freq_by_pointer(const vis_int_t *p);
static void set_options_from_file(int* __restrict__ delay, int* __restrict__ arr_len, char** __restrict__ shuffle_type, char** __restrict__ chord_arg, char** __restrict__ wave_shape, char* __restrict__ use_optimization/*, int *__restrict__ n_similar_items*/);
static int read_chord_progression_from_file();
static void custom_progression_cleanup();

#ifdef _WIN32
static void clear_screen_win32_init();
#endif

// ========== End helper functions breadcrumbs ==========

// ========== Optimization functions breadcrumbs ==========

static char *print_stdout_buffer = NULL;
static int print_buffer_size = 0;

static int buffer_set_sizing(int rows, int cols, unsigned char opt_level);
static int buffer_optimize_init(unsigned char opt_level);
static int buffer_optimize_cleanup(char force);

// ========== End optimization functions breadcrumbs ==========

// ========== Visulizer start and cleanup functions breadcrumbs ==========

static void prepare_visual(vis_int_t *d_array, int *d_array_clone, int delay, const int arr_len, const char* shuffle_type, const char* chord_arg, double maxamp, char* wave_type, const char optimization_on, const int n_similar_items, const char* algo_name);
static int get_sort_status(int *d_array_clone, int array_len, const char optimization_on);
static int perform_sort_visual(int delay, const int arr_len, const char* shuffle_type, const char* chord_arg, double maxamp, char* wave_type, const char optimization_on, const int n_similar_items, const char* algo_name, call_sort_t call_sort, size_t nvis_args, vis_arg_t *vis_args);
// static int perform_radix_sort_visual(int delay, int arr_len, const char* shuffle_type, size_t unsigned_base, const char* chord_arg, double maxamp, char* wave_type, void (*sort_algo)(void*, size_t, size_t, const size_t, size_t (*num_digits)(const void*, size_t), size_t (*get_nth_digit)(const void*, size_t)), const char* algo_name);
/**
 * Returns 1 if program should exit
 */
static int process_args(int* __restrict__ delay, int* __restrict__ arr_len, char** __restrict__ shuffle/*, int *__restrict__ n_similar_items*/, int argc, char** argv, int* __restrict__ radix_arg_id, size_t nvis_args, vis_arg_t *vis_args);

/**
 * Returns 1 if program should exit
 */
static int process_args_2(char** __restrict__ chord_arg, double* __restrict__ amp, char** __restrict__ wave_type, int argc, char** argv, int* __restrict__ arg_id);

// ========== End visulizer start and cleanup functions breadcrumbs ==========

static char perform_verify_sort(int *not_sorted_index, int* idendical_array, int actual_len);
static void perform_selected_shuffle(const char* term, int* idendical_array, int actual_len);
static char* get_shuffle_display_name(const char* term);

// ========== Main helper functions ==========

void (*compr_algo)(void* list, size_t nitems, size_t size, int (*compr)(const void*, const void*));

int call_comparison_sort(void* list, size_t nitems, size_t size, int (*compr)(const void*, const void*), size_t nvis_args, vis_arg_t *vis_args) {
	assert(compr != NULL);
	compr_algo(list, nitems, size, compr);
	return 0;
}

typedef size_t (*radix_num_digits_t)(const void*, size_t);
typedef size_t (*radix_get_nth_digit_t)(const void*, size_t);
void (*radix_algo)(void*, size_t, size_t, const size_t, size_t (*num_digits)(const void*, size_t), size_t (*get_nth_digit)(const void*, size_t));

int call_radix_sort(void* list, size_t nitems, size_t size, int (*compr)(const void*, const void*), size_t nvis_args, vis_arg_t *vis_args) {
	assert(vis_args != NULL);
	assert(nvis_args >= 3);
	
	assert(vis_args[0].size == sizeof(size_t));
	assert(vis_args[0].buffer != NULL);
	assert(vis_args[1].size == sizeof(radix_num_digits_t));
	assert(vis_args[1].buffer != NULL);
	assert(vis_args[2].size == sizeof(radix_get_nth_digit_t));
	assert(vis_args[2].buffer != NULL);

	radix_algo(list, nitems, size, *((size_t *) vis_args[0].buffer), (radix_num_digits_t) vis_args[1].buffer, (radix_get_nth_digit_t) vis_args[2].buffer);
	return 0;
}

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
vis_arg_t set_vis_arg(char should_parse, char *name, char *info, void *buffer, void *default_value, size_t size, int (*parse_arg) (void*, const char*, const void*, size_t)) {
	vis_arg_t r;
	r.should_parse = should_parse;
	r.name = name;
	r.info = info;
	r.buffer = buffer;
	r.default_value = default_value;
	r.parse_arg = parse_arg;
	r.size = size;
	return r;
}

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
int vis_main_visargs(int argc, char** argv, const char* algo_name, call_sort_t call_sort, size_t nvis_args, vis_arg_t *vis_args) {
	int delay = DEFAULT_DIS_DELAY;
	int arr_len = DEFAULT_ARR_LEN;
	char *shuffle = NULL;
	char *chord_arg = NULL;
	char default_wave[] = "square";
	char* wave_type = default_wave;
	char use_optimization = 1;
	int n_similar_items = DEF_N_SIMILAR_ITEMS;
	int arg_id = 0;
	set_options_from_file(&delay, &arr_len, &shuffle, &chord_arg, &wave_type, &use_optimization/*, &n_similar_items*/);
	if (process_args(&delay, &arr_len, &shuffle/*, &n_similar_items*/, argc, argv, &arg_id, nvis_args, vis_args)) {
		return 0;
	}
	for (size_t i = 0; i < nvis_args; i++) {
		assert(vis_args != NULL);
		vis_arg_t *arg_obj = vis_args + i;
		if (arg_obj -> should_parse) {
			assert(arg_obj -> buffer != NULL);
			assert(arg_obj -> default_value != NULL);
			memcpy(arg_obj -> buffer, arg_obj -> default_value, arg_obj -> size);
			if (argc >= arg_id + 1) {
				arg_obj -> parse_arg(arg_obj -> buffer, argv[arg_id], arg_obj -> default_value, arg_obj -> size);
				arg_id++;
			}
		}
	}
	if (process_args_2(&chord_arg, &max_sound_amplitude, &wave_type, argc, argv, &arg_id)) {
		return 0;
	}
	return perform_sort_visual(delay, arr_len, shuffle, chord_arg, max_sound_amplitude, wave_type, use_optimization, n_similar_items, algo_name, call_sort, nvis_args, vis_args);
}

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
int vis_main_vargs(int argc, char** argv, const char* algo_name, call_sort_t call_sort, size_t nvis_args, va_list vis_args) {
	vis_arg_t vargs_list[nvis_args];
	for (size_t i = 0; i < nvis_args; i++) {
		vis_arg_t *arg_obj;
		arg_obj = va_arg(vis_args, vis_arg_t *);
		assert(arg_obj != NULL);
		vargs_list[i] = *(arg_obj);
	}
	return vis_main_visargs(argc, argv, algo_name, call_sort, nvis_args, vargs_list);
}

/**
 * @brief This is a main function that kick start the sorting visualizer.
 *        This can be wrapped with functions that take in varadic arguments.
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
int vis_main(int argc, char** argv, const char* algo_name, call_sort_t call_sort, size_t nvis_args, ...) {
	va_list vis_args;
	va_start(vis_args, nvis_args);
	int ret = vis_main_vargs(argc, argv, algo_name, call_sort, nvis_args, vis_args);
	va_end(vis_args);
	return ret;
}

/**
 * @brief This is a main function that kick start the sorting visualizer.
 *        This is specially designed for comparison sorts, which is the vast majority of sorts.
 * 
 * @param argc This is the argument count from your main function.
 * @param argv This is the argument vector from your main function.
 * @param algo_name The name of the algorithm to be displayed on the top left.
 * @param sort_algo A function pointer that points to the sorting function. The function must take in the same parameters as stdlib.h's qsort.
 */
int vis_main_comparison(int argc, char** argv, const char* algo_name, void (*sort_algo)(void* list, size_t nitems, size_t size, int (*compr)(const void*, const void*))) {
	compr_algo = sort_algo;
	return vis_main_visargs(argc, argv, algo_name, call_comparison_sort, 0, NULL);
}

static int parse_radix_base(void* buffer, const char* arg, const void* default_value, size_t size) {
	assert(size == sizeof(size_t));
	assert(arg != NULL);

	int n = atoi(arg);
	if (n > 1) {
		*((int *) buffer) = (size_t) n;
		radix_base = (size_t) n;
	}

	return 0;
}

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
int vis_main_radix(int argc, char** argv, size_t default_radix_base, const char* algo_name, void (*sort_algo)(void*, size_t, size_t, const size_t, size_t (*num_digits)(const void*, size_t), size_t (*get_nth_digit)(const void*, size_t))) {
	char radix_base_name[] = "radix_base";
	char radix_base_info[64];
	snprintf(radix_base_info, 64, "[Optional] The base for the radix sort. Default is base-%lu.\n", (unsigned long) default_radix_base);
	radix_base_info[63] = '\0';
	radix_base_info[62] = '\n';
	size_t radix_base_user = default_radix_base;
	radix_base = default_radix_base;
	vis_arg_t vis_args[3];
	vis_args[0] = set_vis_arg(1, radix_base_name, radix_base_info, &radix_base_user, &default_radix_base, sizeof(size_t), parse_radix_base);

	vis_args[1] = set_vis_arg(0, NULL, NULL, int_num_digits, NULL, sizeof(radix_num_digits_t), NULL);
	vis_args[2] = set_vis_arg(0, NULL, NULL, base_n_modulo, NULL, sizeof(radix_get_nth_digit_t), NULL);

	radix_algo = sort_algo;

	return vis_main_visargs(argc, argv, algo_name, call_radix_sort, 3, vis_args);
}

/**
 * @deprecated Use vis_main_comparison instead.
 */
int perform_sort_visual_args(int argc, char** argv, void (*sort_algo)(void* list, size_t nitems, size_t size, int (*compr)(const void*, const void*)), const char* algo_name) {
	return vis_main_comparison(argc, argv, algo_name, sort_algo);
}

/**
 * @deprecated Use vis_main_radix instead.
 */
int perform_radix_sort_visual_args(int argc, char** argv, size_t default_radix_base, void (*sort_algo)(void*, size_t, size_t, const size_t, size_t (*num_digits)(const void*, size_t), size_t (*get_nth_digit)(const void*, size_t)), const char* algo_name) {
	return vis_main_radix(argc, argv, default_radix_base, algo_name, sort_algo);
}

// ========== End main functions ==========

// ========== Visulizer start and cleanup functions ==========

static void prepare_visual(vis_int_t *d_array, int *d_array_clone, int delay, const int arr_len, const char* shuffle_type, const char* chord_arg, double maxamp, char* wave_type, const char optimization_on, const int n_similar_items, const char* algo_name) {
	#ifdef _WIN32
	clear_screen_win32_init();
	#endif
	if (log_sound) {
		play_sound = 1;
		start_beep_logging(algo_name);
	}
	#ifdef PA_INSTALLED
	if (maxamp > 0.0) {
		play_sound = 1;
		if (sound_player_init(maxamp, wave_type) != 0) {
			error_pause(error_handle_mode);
		}
	}
	#endif
	if (n_similar_items > 1) {
		for (int i = 0; i < arr_len; i++) {
			d_array[i].num = i / n_similar_items + 1;
			d_array[i].tag = -1;
			d_array_clone[i] = i / n_similar_items + 1;
		}
		non_array_lines += 3;
		display_stability_test = 1;
	} else {
		for (int i = 0; i < arr_len; i++) {
			d_array[i].num = i + 1;
			d_array_clone[i] = i + 1;
		}
	}

	display_array = d_array;
	display_array_len = arr_len;
	bar_display_delay = delay;
	display_name = algo_name;
	highest_item = find_max_int(display_array, display_array_len);
	if (chord_arg != NULL) {
		chord_mode = get_chord_mode(chord_arg, &chord_no);
	}
	chord_progression_timing_ms = get_chord_timing(delay);
	read_chord_progression_from_file();
	if (optimization_on) {
		if (buffer_optimize_init((unsigned char) optimization_on) != 0) {
			error_pause(error_handle_mode);
		}
	}

	last_ncols = pgcg_get_console_cols();
	last_nrows = pgcg_get_console_rows();

	ioext_init(last_ncols);

	clear_screen_(1);

	srand(time(NULL));
	include_sleep_time = should_include_sleep_time(100);

	reset_counters();
	shuffle_name = get_shuffle_display_name(shuffle_type);
	delay_factor = shuffle_delay_factor;
	perform_selected_shuffle(shuffle_type, d_array_clone, arr_len);
	delay_factor = 1.0;
	reset_counters();
	clear_screen_(1);
	print_array_bars("Shuffling Done! :D", NULL, 0, NULL, 0, 1);
}

static void tag_similar_items(vis_int_t *d_array, const int arr_len, const int highest_number, const int n_similar_items) {
	int counts[highest_number + 1];
	for (int i = 0; i <= highest_number; i++) {
		counts[i] = 0;
	}
	stability_test_highest_tag = n_similar_items - 1;
	int highest_tag = -1; // Should not be needed, but just in case
	clear_screen_(1);
	for (int i = 0; i < arr_len; i++) {
		d_array[i].tag = counts[d_array[i].num];
		counts[d_array[i].num]++;
		print_array_bars("Tagging similar items", d_array + i, 0, NULL, 0, 1);
		if (highest_tag < d_array[i].tag) { // Should not be needed, but just in case
			highest_tag = d_array[i].tag;   // Should not be needed, but just in case
		}                                   // Should not be needed, but just in case
	}
	stability_test_highest_tag = highest_tag; // Should not be needed, but just in case
	clear_screen_(1);
	print_array_bars("Tagging Done! :D", NULL, 0, NULL, 0, 1);
}

static int get_sort_status(int *d_array_clone, int array_len, const char optimization_on) {
	int incorrect_index = 0;

	delay_factor = verify_delay_factor;
	char sort_correct = perform_verify_sort(&incorrect_index, d_array_clone, array_len);
	delay_factor = 1.0;

	clear_screen_(1);
	char prefixtxt_success[] = "Sorting Done! :D";
	char prefixtxt_fail[] = "Sorting Failed! D:";
	char catprefix[get_concat_sort_name_len(sort_correct ? prefixtxt_success : prefixtxt_fail)];
	cpy_concat_sort_name(catprefix, sort_correct ? prefixtxt_success : prefixtxt_fail);
	print_array_bars(catprefix, sort_correct ? NULL : display_array + incorrect_index, !sort_correct, sort_correct ? NULL : display_array + incorrect_index - 1, !sort_correct, 0);
	if (optimization_on) {
		if (buffer_optimize_cleanup(1) != 0) {
			error_pause(error_handle_mode);
		}
	}
	ioext_cleanup();
	custom_progression_cleanup();
	display_name = NULL;
	display_array = NULL;
	if (log_sound) {
		stop_beep_logging();
	}
	#ifdef PA_INSTALLED
	if (max_sound_amplitude > 0.0) {
		if (sound_player_cleanup() != 0) {
			error_pause(error_handle_mode);
		}
	}
	#endif
	return !sort_correct;
}

static int perform_sort_visual(int delay, const int arr_len, const char* shuffle_type, const char* chord_arg, double maxamp, char* wave_type, const char optimization_on, const int n_similar_items, const char* algo_name, call_sort_t call_sort, size_t nvis_args, vis_arg_t *vis_args) {
	vis_int_t d_array[arr_len];
	int d_array_clone[arr_len];

	prepare_visual(d_array, d_array_clone, delay, arr_len, shuffle_type, chord_arg, maxamp, wave_type, optimization_on, n_similar_items, algo_name);

	if (n_similar_items > 1) {
		tag_similar_items(d_array, arr_len, highest_item, n_similar_items);
	}

	include_sleep_time = should_include_sleep_time(1000);

	reset_counters();
	is_sorting = 1;
	if (call_sort(display_array, display_array_len, sizeof(vis_int_t), larger_int_back, nvis_args, vis_args) != 0) {
		error_pause(error_handle_mode);
	}
	is_sorting = 0;

	return get_sort_status(d_array_clone, arr_len, optimization_on);
}

/**
 * Returns 1 if program should exit
 */
static int process_args(int* __restrict__ delay, int* __restrict__ arr_len, char** __restrict__ shuffle/*, int *__restrict__ n_similar_items*/, int argc, char** argv, int* __restrict__ radix_arg_id, size_t nvis_args, vis_arg_t *vis_args) {
	int arg_id = 1;
	if (argc >= arg_id + 1) {
		if (is_help_arg(argv[arg_id])) {
			print_usage_info(argv[0], nvis_args, vis_args);
			return 1;
		}
		if (strcmp(argv[arg_id], "nologsound") == 0) {
			log_sound = 0;
			arg_id++;
			if (argc <= arg_id) {
				print_usage_info(argv[0], nvis_args, vis_args);
				return 1;
			}
		}
		*delay = atoi(argv[arg_id]);
	}
	arg_id++;
	if (argc >= arg_id + 1) {
		*arr_len = atoi(argv[arg_id]);
	}
	if (*arr_len < 2) {
		*arr_len = 2;
	}
	if (*arr_len > pgcg_get_console_cols() - 1) {
		*arr_len = pgcg_get_console_cols() - 1;
	}
	arg_id++;
	if (argc >= arg_id + 1) {
		*shuffle = argv[arg_id];
	}/*
	arg_id++;
	if (argc >= arg_id + 1) {
		int val = atoi(argv[arg_id]);
		*n_similar_items = val > 1 ? val : 1;
	}*/
	arg_id++;
	if (radix_arg_id != NULL) {
		*radix_arg_id = arg_id;
	}
	return 0;
}

/**
 * Returns 1 if program should exit
 */
static int process_args_2(char** __restrict__ chord_arg, double* __restrict__ amp, char** __restrict__ wave_type, int argc, char** argv, int* __restrict__ arg_id) {
	if (argc >= *arg_id + 1) {
		*chord_arg = argv[*arg_id];
	}
	(*arg_id)++;
	if (argc >= *arg_id + 1) {
		*wave_type = argv[*arg_id];
	}
	(*arg_id)++;
	if (argc >= *arg_id + 1) {
		*amp = atof(argv[*arg_id]);
		*amp = *amp < 0.0 ? 0.0 : *amp;
		*amp = *amp > 1.0 ? 1.0 : *amp;
	}
	(*arg_id)++;
	return 0;
}

// ========== End visulizer start and cleanup functions ==========

// ========== Visualisation functions ==========

#include <limits.h>
#include <float.h>

#define pnt_trunc(fmt, limit) print_truncated_f(fmt, limit)
#define pnt_trunc_f(fmt, limit, ...) print_truncated_f(fmt, limit, __VA_ARGS__)

void print_array_bars(const char *prefix, const void *p1, char is_p1_comparison, const void *p2, char is_p2_comparison, char in_progress) {
	clock_t vt0 = clock();
	int ncols = pgcg_get_console_cols();
	int nrows = pgcg_get_console_rows();
	if (last_ncols == ncols && last_nrows == nrows) {
		clear_screen_(0);
	} else {
		last_ncols = ncols;
		last_nrows = nrows;
		clear_screen_(1);
	}
	int i1 = (p1 - (void*) display_array) / sizeof(vis_int_t);
	int i2 = (p2 - (void*) display_array) / sizeof(vis_int_t);
	int vislen = display_array_len < ncols - 1 ? display_array_len : ncols - 1;
	if (prefix != NULL) {
		print_version_top_right(prefix, ncols);
	}
	int allowed_h = nrows - non_array_lines;
	allowed_h = highest_item < allowed_h ? highest_item : allowed_h;
	/*if (display_jupiterbjy_numbers && highest_item / 10 < allowed_h) {
		allowed_h = highest_item / 10;
		if (highest_item % 10 != 0) {
			allowed_h++;
		}
	}*/
	for (int i = allowed_h; i > 0; i--) {
		for (int j = 0; j < vislen; j++) {
			size_t elem_row_no = /*display_jupiterbjy_numbers ? ((display_array[j].num - 1) / 10 + 1) :*/ (display_array[j].num * allowed_h / highest_item);
			if (display_array[j].num != 0 && (display_full_bar ? (i == 1 || elem_row_no >= i) : (elem_row_no == i || (i == 1 && elem_row_no == 0)))) {
				if ((p1 != NULL && j == i1) || (p2 != NULL && j == i2)) {
					if ((j == i1 && is_p1_comparison) || (j == i2 && is_p2_comparison)) {
						fputc(compare_bar_char, stdout);
					} else {
						fputc(active_bar_char, stdout);
					}
				} else {
					if (display_jupiterbjy_numbers && (elem_row_no == i || (i == 1 && elem_row_no == 0))) {
						fputc(jupiterbjy_number_representation(display_array[j].num), stdout);
					} else {
						fputc(normal_bar_char, stdout);
					}
				}
			} else {
				fputc(' ', stdout);
			}
		}
		fputc('\n', stdout);
	}

	if (display_stability_test) {
		for (int i = 0; i < vislen; i++) {
			fputc(jupiterbjy_number_representation2(display_array[i].tag), stdout);
		}
		pnt_trunc("\n", ncols);
	}
	pnt_trunc("\n", ncols);

	int delay_to_use = delay_factor == 1.0 ? bar_display_delay : (int) ((double) bar_display_delay * delay_factor);
	pnt_trunc_f("Number of items: %d, delay: %dms \n", ncols, display_array_len, delay_to_use);
	pnt_trunc("[Enter <sort_name> help for more info to adjust the above]\n", ncols);
	pnt_trunc_f("Comparisons:%s %lu\n", ncols, comparisons_overflow ? " more than" : "", comparisons);
	pnt_trunc_f("Writes to main array:%s %lu\n", ncols, writes_overflow ? " more than" : "", writes);
	pnt_trunc_f("Writes to additional array(s):%s %lu\n", ncols, aux_writes_overflow ? " more than" : "", aux_writes);

	time_t t1 = clock() - start_time;
	pnt_trunc("Time taken: ", ncols);
	if (print_precise_time) {
		mstime_t mst1 = mstime() - start_time_ms;
		print_mstime(mst1, ncols);
	} else {
		time_t vt1 = time(NULL) - vis_start_time;
		print_time_long(vt1, ncols);
	}

	pnt_trunc("\n", ncols);
	if (print_estimated_real_time) {
		pnt_trunc("Estimated time w/o delay: ", ncols);
		if (include_sleep_time) {
			print_time((double)t1 / CLOCKS_PER_SEC, ncols);
		} else {
			if (delay_secs_overflow) {
				pnt_trunc("too long!", ncols);
			} else {
				print_time((double)(t1 - delay_secs) / CLOCKS_PER_SEC, ncols);
			}
		}
		pnt_trunc(" (may be inaccurate)\n", ncols);
	}
	pnt_trunc_f("Comparisons are marked with '%c'\nOther array accesses are marked '%c'.\n", ncols, compare_bar_char, active_bar_char);
	if (display_stability_test) {
		pnt_trunc("If the original order of similar items are preserved,\nthe numbers at the bottom will be in ascending clusters separated by spaces.\n", ncols);
	}
	if (in_progress) pnt_trunc("Press / hold / spam <ctrl-c> to cancel.\n", ncols);
	pnt_trunc("\n", ncols);
	fflush(stdout);
	if (play_sound && (pointer_in_array(p1) || pointer_in_array(p2))) {
		double freqs[2];
		freqs[0] = get_freq_by_pointer((const vis_int_t *) p1);
		freqs[1] = get_freq_by_pointer((const vis_int_t *) p2);
		double minfreq = pointer_in_array(p1) ? freqs[0] : freqs[1];
		if (pointer_in_array(p1) && pointer_in_array(p2)) {
			minfreq = freqs[0] < freqs[1] ? freqs[0] : freqs[1];
		}
		int minfreq_period = 1000 / (int) minfreq;
		#ifdef ACCOUNT_PRINT_LEN
		int sound_len = clock() - vt0 + (delay_to_use > min_sound_len ? delay_to_use : min_sound_len);
		#else
		int sound_len = min_sound_len;
		#endif
		if (delay_to_use > sound_len) {
			sound_len = delay_to_use;
		}
		if (sound_len < minfreq_period) {
			sound_len = minfreq_period;
		}
		if (log_sound) log_beep(sound_len, (double*)freqs + (pointer_in_array(p2) && !pointer_in_array(p1) ? 1 : 0), (pointer_in_array(p1) && pointer_in_array(p2)) ? 2 : 1);
		#ifdef PA_INSTALLED
		sound_player_put_tone(sound_len, pointer_in_array(p1) ? freqs[0] : 0.0, pointer_in_array(p2) ? freqs[1] : 0.0);
		#endif
	}
	// if (!include_sleep_time) {
		if (delay_secs == INT_MAX) {
			delay_secs_overflow = 1;
		} else {
			delay_secs += clock() - vt0;
		}
	// }
	sleep_ms(delay_to_use);
}

void print_array_bars_not_compare(const void *p1, const void *p2) {
	char prefixtxt[] = "Sorting";
	char catprefix[get_concat_sort_name_len(prefixtxt)];
	cpy_concat_sort_name(catprefix, prefixtxt);
	print_array_bars(catprefix, p1, 0, p2, 0, 1);
}

void mark_comparison() {
	if (comparisons == ULONG_MAX) {
		comparisons_overflow = 1;
	} else {
		comparisons++;
	}
}

void mark_array_write() {
	if (writes == ULONG_MAX) {
		writes_overflow = 1;
	} else {
		writes++;
	}
}

void mark_aux_array_write() {
	if (aux_writes == ULONG_MAX) {
		aux_writes_overflow = 1;
	} else {
		aux_writes++;
	}
}

void reset_comparisons() {
	comparisons = 0;
	comparisons_overflow = 0;
}

void reset_writes() {
	writes = 0;
	aux_writes = 0;
	writes_overflow = 0;
	aux_writes_overflow = 0;
}

void reset_delays() {
	delay_secs = 0;
	delay_secs_overflow = 0;
}

void reset_time() {
	reset_delays();
	start_time = clock();
	vis_start_time = time(NULL);
	start_time_ms = mstime();
}

void reset_counters() {
	reset_comparisons();
	reset_writes();
	reset_time();
}

void memcpy_v(void *dest, const void *src, const size_t n) {
	memcpy(dest, src, n);
	if (pointer_in_array(dest)) {
		mark_array_write();
	}
	print_array_bars_not_compare(pointer_in_array(dest) ? dest : NULL, pointer_in_array(src) ? src : NULL);
}

void memmove_v(void *dest, const void *src, const size_t n) {
	memmove(dest, src, n);
	if (pointer_in_array(dest)) {
		mark_array_write();
	}
	print_array_bars_not_compare(pointer_in_array(dest) ? dest : NULL, pointer_in_array(src) ? src : NULL);
}

void remove_varray_item(vis_int_t *ptr) {
	if (!pointer_in_array(ptr)) {
		return;
	}
	vis_int_t* end = display_array + display_array_len;
	for (vis_int_t *p = ptr + 1; p < end; p++) {
		*(p - 1) = *p;
		mark_array_write();
		print_array_bars_not_compare(p - 1, p);
	}
	(end - 1) -> num = 0;
	(end - 1) -> tag = -1;
	print_array_bars_not_compare(end - 1, NULL);
	display_array_len--;
}

void set_v_array_len(int n) {
	int ncol = pgcg_get_console_cols();
	int new_n = n < ncol ? n : ncol - 1;
	if (new_n == display_array_len) {
		return;
	}
	if (new_n < display_array_len) {
		for (int i = n; i < display_array_len; i++) {
			display_array[i].num = 0;
			display_array[i].tag = -1;
			// mark_array_write();
			print_array_bars_not_compare(display_array + i, NULL);
		}
	}
	display_array_len = n < ncol ? n : ncol - 1;
}

struct verify_sort_cache {
	long comparisons;
	char comparisons_overflow;
	long writes;
	char writes_overflow;
	long aux_writes;
	char aux_writes_overflow;
	clock_t delay_secs;
	char delay_secs_overflow;
	time_t start_time;
	time_t vis_start_time;
	mstime_t start_time_ms;
};

static char perform_verify_sort(int *not_sorted_index, int* idendical_array, int actual_len) {
	struct verify_sort_cache cache;
	cache.comparisons = comparisons;
	cache.comparisons_overflow = comparisons_overflow;
	cache.writes = writes;
	cache.writes_overflow = writes_overflow;
	cache.aux_writes = aux_writes;
	cache.aux_writes_overflow = aux_writes_overflow;
	cache.delay_secs = delay_secs;
	cache.delay_secs_overflow = delay_secs_overflow;
	cache.start_time = start_time;
	cache.vis_start_time = vis_start_time;
	cache.start_time_ms = start_time_ms;
	reset_counters();
	clear_screen_(1);
	char is_sorted = 1;
	for (int i = 0; i < display_array_len; i++) {
		mark_comparison();
		if (display_array[i].num != idendical_array[i]) {
			is_sorted = 0;
			*not_sorted_index = i;
			break;
		}
		print_array_bars("Verifying Sort", &(display_array[i]), 1, &(display_array[i - 1]), 1, 1);
	}
	if (is_sorted && display_array_len != actual_len) {
		is_sorted = 0;
		*not_sorted_index = display_array_len;
	}
	cache.start_time += clock() - start_time;
	cache.vis_start_time += time(NULL) - vis_start_time;
	cache.start_time_ms += mstime() - start_time_ms;
	comparisons = cache.comparisons;
	comparisons_overflow = cache.comparisons_overflow;
	writes = cache.writes;
	writes_overflow = cache.writes_overflow;
	aux_writes = cache.aux_writes;
	aux_writes_overflow = cache.aux_writes_overflow;
	delay_secs = cache.delay_secs;
	delay_secs_overflow = cache.delay_secs_overflow;
	start_time = cache.start_time;
	vis_start_time = cache.vis_start_time;
	start_time_ms = cache.start_time_ms;
	return is_sorted;
}

// ========== End visualisation functions ==========

// ========== Shuffle functions ==========

static void shuffle_display_array() {
	for (int i = 0; i < display_array_len; i++) {
		int rng = rand() & 0xff;
		rng |= (rand() & 0xff) << 8;
		rng |= (rand() & 0xff) << 16;
		rng |= (rand() & 0x7f) << 24;
		rng %= display_array_len - i;
		rng += i;
		vis_int_t tmp = display_array[rng];
		display_array[rng] = display_array[i];
		mark_array_write();
		display_array[i] = tmp;
		mark_array_write();
		print_array_bars("Shuffling", &(display_array[i]), 0, &(display_array[rng]), 0, 1);
	}
}

static void reverse_display_array() {
	for (int i = 0, j = display_array_len - 1; i < j; i++, j--) {
		vis_int_t tmp = display_array[j];
		display_array[j] = display_array[i];
		mark_array_write();
		display_array[i] = tmp;
		mark_array_write();
		print_array_bars("Reversing", &(display_array[i]), 0, &(display_array[j]), 0, 1);
	}
}

static void slightly_shuffle_display_array() {
	int shuffle_count = display_array_len < 20 ? 1 : display_array_len / 20;
	for (int i = 0; i < shuffle_count; i++) {
		int rng1 = rand() & 0xff;
		rng1 |= (rand() & 0xff) << 8;
		rng1 |= (rand() & 0xff) << 16;
		rng1 |= (rand() & 0x7f) << 24;
		rng1 %= display_array_len;
		int rng2 = rng1;
		while (rng2 == rng1) {
			rng2 = rand() & 0xff;
			rng2 |= (rand() & 0xff) << 8;
			rng2 |= (rand() & 0xff) << 16;
			rng2 |= (rand() & 0x7f) << 24;
			rng2 %= display_array_len;
		}
		vis_int_t tmp = display_array[rng1];
		display_array[rng1] = display_array[rng2];
		mark_array_write();
		display_array[rng2] = tmp;
		mark_array_write();
		print_array_bars("Shuffling", &(display_array[rng2]), 0, &(display_array[rng1]), 0, 1);
	}
}

static void circle_pass(vis_int_t* arr, int nitems) {
	if (nitems <= 1) return;
	size_t l, r;

	for (l = 0, r = nitems - 1; l < r; l++, r--) {
		mark_comparison();
		if (arr[l].num > arr[r].num) {
			vis_int_t tmp = arr[l];
			arr[l] = arr[r];
			mark_array_write();
			arr[r] = tmp;
			mark_array_write();
			print_array_bars("Performing circle pass", &(arr[l]), 0, &(arr[r]), 0, 1);
		}
	}
	mark_comparison();

	mark_comparison();
	if (l == r && arr[l].num > arr[r + 1].num) {
		vis_int_t tmp = arr[l];
		arr[l] = arr[r + 1];
		mark_array_write();
		arr[r + 1] = tmp;
		mark_array_write();
		print_array_bars("Performing circle pass", &(arr[l]), 0, &(arr[r + 1]), 0, 1);
	}

	size_t middle = (nitems - 1) / 2;
	circle_pass(arr, middle + 1);
	circle_pass(arr + middle + 1, nitems - middle - 1);
}

static void first_circle_pass() {
	shuffle_display_array();
	circle_pass(display_array, display_array_len);
}

static void shuffle_tail() {
	vis_int_t tmp_arr[display_array_len];
	for (int i = 0; i < display_array_len; i++) {
		tmp_arr[i] = display_array[i];
		mark_aux_array_write();
		print_array_bars("Performing 3:1 split", &(display_array[i]), 0, NULL, 0, 1);
	}
	int i1 = 0;
	for (int i = 0; i < display_array_len; i++) {
		if (i % 4 != 0) {
			display_array[i1] = tmp_arr[i];
			mark_array_write();
			print_array_bars("Performing 3:1 split", &(display_array[i1]), 0, NULL, 0, 1);
			i1++;
		}
	}
	int shuffle_start = i1;
	for (int i = 0; i < display_array_len; i++) {
		if (i % 4 == 0) {
			display_array[i1] = tmp_arr[i];
			mark_array_write();
			print_array_bars("Performing 3:1 split", &(display_array[i1]), 0, NULL, 0, 1);
			i1++;
		}
	}
	for (int i = shuffle_start; i < display_array_len; i++) {
		int rng = rand() & 0xff;
		rng |= (rand() & 0xff) << 8;
		rng |= (rand() & 0xff) << 16;
		rng |= (rand() & 0x7f) << 24;
		rng %= display_array_len - i;
		rng += i;
		vis_int_t tmp = display_array[rng];
		display_array[rng] = display_array[i];
		mark_array_write();
		display_array[i] = tmp;
		mark_array_write();
		print_array_bars("Shuffling", &(display_array[i]), 0, &(display_array[rng]), 0, 1);
	}
}

static void shuffle_head() {
	vis_int_t tmp_arr[display_array_len];
	for (int i = 0; i < display_array_len; i++) {
		tmp_arr[i] = display_array[i];
		mark_aux_array_write();
		print_array_bars("Performing 1:3 split", &(display_array[i]), 0, NULL, 0, 1);
	}
	int i1 = 0;
	for (int i = 0; i < display_array_len; i++) {
		if (i % 4 == 0) {
			display_array[i1] = tmp_arr[i];
			mark_array_write();
			print_array_bars("Performing 1:3 split", &(display_array[i1]), 0, NULL, 0, 1);
			i1++;
		}
	}
	int shuffle_end = i1;
	for (int i = 0; i < display_array_len; i++) {
		if (i % 4 != 0) {
			display_array[i1] = tmp_arr[i];
			mark_array_write();
			print_array_bars("Performing 1:3 split", &(display_array[i1]), 0, NULL, 0, 1);
			i1++;
		}
	}
	for (int i = 0; i < shuffle_end; i++) {
		int rng = rand() & 0xff;
		rng |= (rand() & 0xff) << 8;
		rng |= (rand() & 0xff) << 16;
		rng |= (rand() & 0x7f) << 24;
		rng %= shuffle_end - i;
		rng += i;
		vis_int_t tmp = display_array[rng];
		display_array[rng] = display_array[i];
		mark_array_write();
		display_array[i] = tmp;
		mark_array_write();
		print_array_bars("Shuffling", &(display_array[i]), 0, &(display_array[rng]), 0, 1);
	}
}

static void final_merge_pass() {
	vis_int_t tmp_arr[display_array_len];
	for (int i = 0; i < display_array_len; i++) {
		tmp_arr[i] = display_array[i];
		mark_aux_array_write();
		print_array_bars("Performing 1:1 split", &(display_array[i]), 0, NULL, 0, 1);
	}
	int i1 = 0;
	for (int i = 0; i < display_array_len; i++) {
		if (i % 2 == 0) {
			display_array[i1] = tmp_arr[i];
			mark_array_write();
			print_array_bars("Performing 1:1 split", &(display_array[i1]), 0, NULL, 0, 1);
			i1++;
		}
	}
	for (int i = 0; i < display_array_len; i++) {
		if (i % 2 != 0) {
			display_array[i1] = tmp_arr[i];
			mark_array_write();
			print_array_bars("Performing 1:1 split", &(display_array[i1]), 0, NULL, 0, 1);
			i1++;
		}
	}
}

static void reverse_final_merge_pass() {
	final_merge_pass();
	clear_screen_(1);
	reverse_display_array();
}

static void sift_down_display_array(size_t heaplen, size_t idx) {
	size_t i = idx;
	while (idx < heaplen) {
		size_t children[2];
		children[0] = 2 * idx + 1;
		children[1] = 2 * (idx + 1);
		size_t ccount = 2;
		if (children[0] >= heaplen) {
			break;
		}
		if (children[1] >= heaplen) {
			ccount = 1;
		}
		i = children[0];
		if (ccount == 2) {
			mark_comparison();
			if (ccount == 2 && display_array[i].num < display_array[children[1]].num) {
				i = children[1];
			}
			print_array_bars("Performing heapify", &(display_array[children[0]]), 1, &(display_array[children[1]]), 1, 1);
		}
		
		mark_comparison();
		print_array_bars("Performing heapify", &(display_array[idx]), 1, &(display_array[i]), 1, 1);
		if (display_array[idx].num >= display_array[i].num) {
			break;
		}
		vis_int_t tmp = display_array[idx];
		display_array[idx] = display_array[i];
		mark_array_write();
		display_array[i] = tmp;
		mark_array_write();
		print_array_bars("Performing heapify", &(display_array[idx]), 0, &(display_array[i]), 0, 1);
		idx = i;
	}
}

static void heapify_display_array(size_t heaplen) {
	int parent = (heaplen - 2) / 2;
	while (1) {
		sift_down_display_array(heaplen, parent);
		if (parent == 0) {
			break;
		}
		parent--;
	}
}

#define get_last_child(x) (2 * (x + 1))

static void build_sorted_heap() {
	vis_int_t tmp_arr[display_array_len];
	for (int i = 0; i < display_array_len; i++) {
		tmp_arr[i] = display_array[i];
		// The following assertion was meant to ensure that the array given is already sorted, which it by default should be.
		// assert(tmp_arr[i].num == i + 1);  // CHECK tmp_arr[i].num with clone_array[i] instead! Need to add new parameter.
		mark_aux_array_write();
		print_array_bars("Generating heap input", &(display_array[i]), 0, NULL, 0, 1);
	}
	display_array[0] = tmp_arr[display_array_len - 1];
	mark_array_write();
	print_array_bars("Generating heap input", display_array, 0, NULL, 0, 1);
	int j = display_array_len - 1;
	int last_parent = 0;
	int last_child = get_last_child(last_parent);
	while (j > 0) {
		assert(last_parent < last_child);
		for (int i = last_child; i > last_parent; i--) {
			display_array[i] = tmp_arr[j - 1];
			j--;
			mark_array_write();
			print_array_bars("Generating heap input", &(display_array[i]), 0, NULL, 0, 1);
		}
		last_parent = last_child;
		last_child = get_last_child(last_parent);
		if (last_child >= display_array_len) {
			last_child = display_array_len - 1;
		}
	}
}

struct used_tag {
	vis_int_t x;
	char used;
};

#define get_first_child(x) (2 * x + 1)

static void build_bst(vis_int_t* dest, int i, struct used_tag *tmp_arr, int start, int end, int* __restrict__ max_i_reach, int recurse_depth) {
	if (recurse_depth <= 0) {
		return;
	}
	if (i >= display_array_len) {
		return;
	}
	if (start > end) {
		return;
	}
	int mid = (start + end) / 2;
	dest[i] = tmp_arr[mid].x;
	tmp_arr[mid].used = 1;
	if (*max_i_reach < i) {
		*max_i_reach = i;
	}

	mark_array_write();
	print_array_bars("Generating binary search tree", &(dest[i]), 0, NULL, 0, 1);

	build_bst(dest, get_first_child(i), tmp_arr, start, mid - 1, max_i_reach, recurse_depth - 1);
	build_bst(dest, get_last_child(i), tmp_arr, mid + 1, end, max_i_reach, recurse_depth - 1);
}

static int find_last_full_hirachy(int nitems) {
	int previous_i = 0;
	int i = get_last_child(previous_i);
	int r = 1;
	while (i < nitems) {
		previous_i = i;
		i = get_last_child(i);
		r++;
	}
	return r;
}

static void build_binary_search_tree() {
	struct used_tag tmp_arr[display_array_len];
	for (int i = 0; i < display_array_len; i++) {
		tmp_arr[i].x = display_array[i];
		tmp_arr[i].used = 0;
		// The following assertion was meant to ensure that the array given is already sorted, which it by default should be.
		// assert(tmp_arr[i].x.num == i + 1);  // CHECK tmp_arr[i].num with clone_array[i] instead! Need to add new parameter.
		mark_aux_array_write();
		print_array_bars("Generating binary search tree", &(display_array[i]), 0, NULL, 0, 1);
	}
	int full_hirachies_count = find_last_full_hirachy(display_array_len);
	int max_index_reached = 0;
	build_bst(display_array, 0, tmp_arr, 0, display_array_len - 1, &max_index_reached, full_hirachies_count);
	int i = max_index_reached + 1;
	for (int j = 0; j < display_array_len; j++) {
		if (!tmp_arr[j].used) {
			assert(i < display_array_len);
			display_array[i] = tmp_arr[j].x;
			mark_array_write();
			print_array_bars("Generating binary search tree", &(display_array[i]), 0, NULL, 0, 1);
			i++;
		}
	}
}

static void split_quarters() {
	vis_int_t tmp_arr[display_array_len];
	for (int i = 0; i < display_array_len; i++) {
		tmp_arr[i] = display_array[i];
		mark_aux_array_write();
		print_array_bars("Performing 1:1:1:1 split", &(display_array[i]), 0, NULL, 0, 1);
	}
	int i1 = 0;
	for (int i = 0; i < display_array_len; i++) {
		if (i % 4 == 0) {
			display_array[i1] = tmp_arr[i];
			mark_array_write();
			print_array_bars("Performing 1:1:1:1 split", &(display_array[i1]), 0, NULL, 0, 1);
			i1++;
		}
	}
	for (int i = 0; i < display_array_len; i++) {
		if (i % 4 == 1) {
			display_array[i1] = tmp_arr[i];
			mark_array_write();
			print_array_bars("Performing 1:1:1:1 split", &(display_array[i1]), 0, NULL, 0, 1);
			i1++;
		}
	}
	for (int i = 0; i < display_array_len; i++) {
		if (i % 4 == 2) {
			display_array[i1] = tmp_arr[i];
			mark_array_write();
			print_array_bars("Performing 1:1:1:1 split", &(display_array[i1]), 0, NULL, 0, 1);
			i1++;
		}
	}
	for (int i = 0; i < display_array_len; i++) {
		if (i % 4 == 3) {
			display_array[i1] = tmp_arr[i];
			mark_array_write();
			print_array_bars("Performing 1:1:1:1 split", &(display_array[i1]), 0, NULL, 0, 1);
			i1++;
		}
	}
}

static void reverse_split_quarters() {
	split_quarters();
	clear_screen_(1);
	reverse_display_array();
}

static void pipe_organ_input() {
	vis_int_t tmp_arr[display_array_len];
	for (int i = 0; i < display_array_len; i++) {
		tmp_arr[i] = display_array[i];
		mark_aux_array_write();
		print_array_bars("Performing 1:1 split", &(display_array[i]), 0, NULL, 0, 1);
	}
	int i1 = 0;
	for (int i = 0; i < display_array_len; i++) {
		if (i % 2 == 0) {
			display_array[i1] = tmp_arr[i];
			mark_array_write();
			print_array_bars("Performing 1:1 split", &(display_array[i1]), 0, NULL, 0, 1);
			i1++;
		}
	}
	i1 = display_array_len - 1;
	for (int i = 0; i < display_array_len; i++) {
		if (i % 2 != 0) {
			display_array[i1] = tmp_arr[i];
			mark_array_write();
			print_array_bars("Performing 1:1 split", &(display_array[i1]), 0, NULL, 0, 1);
			i1--;
		}
	}
}

static void half_reverse_display_array() {
	for (int i = 0, j = display_array_len - 1; i < (display_array_len - 1) / 4; i++, j--) {
		vis_int_t tmp = display_array[j];
		display_array[j] = display_array[i];
		mark_array_write();
		display_array[i] = tmp;
		mark_array_write();
		print_array_bars("Reversing", &(display_array[i]), 0, &(display_array[j]), 0, 1);
	}
}

// BUG and dilemma: Do we consider shuffle odds as in odd numbered-positions? or odd numbers in the array?
static void reverse_odds() {
	vis_int_t *start = display_array, *end = display_array + display_array_len - 1;
	if (start -> num % 2 == 0) {
		start++;
	}
	if (end -> num % 2 == 0) {
		end--;
	}
	for (; start < end; start += 2, end -= 2) {
		vis_int_t tmp = *end;
		*end = *start;
		mark_array_write();
		*start = tmp;
		mark_array_write();
		print_array_bars("Reversing odd numbers", start, 0, end, 0, 1);
	}
}

// BUG and dilemma: Do we consider shuffle evens as in even numbered-positions? or even numbers in the array?
static void reverse_evens() {
	vis_int_t *start = display_array, *end = display_array + display_array_len - 1;
	if (start -> num % 2 != 0) {
		start++;
	}
	if (end -> num % 2 != 0) {
		end--;
	}
	for (; start < end; start += 2, end -= 2) {
		vis_int_t tmp = *end;
		*end = *start;
		mark_array_write();
		*start = tmp;
		mark_array_write();
		print_array_bars("Reversing even numbers", start, 0, end, 0, 1);
	}
}

static void shuffle_odds() {
	// BUG and dilemma: Do we consider shuffle odds as in odd numbered-positions? or odd numbers in the array?
	// assert(display_array[0].num % 2 == 1); // For odd positions instead of numbers just comment this out.
	int odds_len = display_array_len / 2;
	if (display_array_len % 2 != 0) {
		odds_len++;
	}
	for (int i = 0; i < odds_len; i++) {
		int rng = rand() & 0xff;
		rng |= (rand() & 0xff) << 8;
		rng |= (rand() & 0xff) << 16;
		rng |= (rand() & 0x7f) << 24;
		rng %= odds_len - i;
		rng += i;
		rng *= 2;
		int i1 = i * 2;
		vis_int_t tmp = display_array[rng];
		assert(tmp.num % 2 == 1);
		// BUG and dilemma: Do we consider shuffle odds as in odd numbered-positions? or odd numbers in the array?
		assert(display_array[i1].num % 2 == 1);
		display_array[rng] = display_array[i1];
		mark_array_write();
		display_array[i1] = tmp;
		mark_array_write();
		print_array_bars("Shuffling", &(display_array[i1]), 0, &(display_array[rng]), 0, 1);
	}
}

static void shuffle_evens() {
	// BUG and dilemma: Do we consider shuffle evens as in even numbered-positions? or even numbers in the array?
	// assert(display_array[1].num % 2 == 0); // For even positions instead of numbers just comment this out.
	int evens_len = display_array_len / 2;
	for (int i = 0; i < evens_len; i++) {
		int rng = rand() & 0xff;
		rng |= (rand() & 0xff) << 8;
		rng |= (rand() & 0xff) << 16;
		rng |= (rand() & 0x7f) << 24;
		rng %= evens_len - i;
		rng += i;
		rng *= 2;
		rng++;
		int i1 = i * 2 + 1;
		vis_int_t tmp = display_array[rng];
		assert(tmp.num % 2 == 0);
		// BUG and dilemma: Do we consider shuffle evens as in even numbered-positions? or even numbers in the array?
		assert(display_array[i1].num % 2 == 0);
		display_array[rng] = display_array[i1];
		mark_array_write();
		display_array[i1] = tmp;
		mark_array_write();
		print_array_bars("Shuffling", &(display_array[i1]), 0, &(display_array[rng]), 0, 1);
	}
}

static void final_bitonic_pass() {
	vis_int_t tmp_arr[display_array_len];
	for (int i = 0; i < display_array_len; i++) {
		tmp_arr[i] = display_array[i];
		mark_aux_array_write();
		print_array_bars("Performing 1:1 split", &(display_array[i]), 0, NULL, 0, 1);
	}
	int i1 = 0;
	for (int i = display_array_len; i > 0; i--) {
		if ((i - 1) % 2 == 0) {
			display_array[i1] = tmp_arr[i - 1];
			mark_array_write();
			print_array_bars("Performing 1:1 split", &(display_array[i1]), 0, NULL, 0, 1);
			i1++;
		}
	}
	i1 = display_array_len - 1;
	for (int i = display_array_len; i > 0; i--) {
		if ((i - 1) % 2 != 0) {
			display_array[i1] = tmp_arr[i - 1];
			mark_array_write();
			print_array_bars("Performing 1:1 split", &(display_array[i1]), 0, NULL, 0, 1);
			i1--;
		}
	}
}

static void final_radix_pass() {
	int len1 = display_array_len / 2 + 1;
	int len2 = display_array_len - len1;
	vis_int_t tmp_arr1[len1];
	vis_int_t tmp_arr2[len2];
	assert(len2 <= len1);
	for (int i = 0; i < len1; i++) {
		tmp_arr1[i] = display_array[i];
		mark_aux_array_write();
		print_array_bars("Weaving", &(display_array[i]), 0, NULL, 0, 1);
	}
	for (int i = 0; i < len2; i++) {
		tmp_arr2[i] = display_array[i + len1];
		mark_aux_array_write();
		print_array_bars("Weaving", &(display_array[i + len1]), 0, NULL, 0, 1);
	}
	int i1 = 0;
	for (int i = 0; i < len1; i++) {
		display_array[i1] = tmp_arr1[i];
		mark_array_write();
		print_array_bars("Weaving", &(display_array[i1]), 0, NULL, 0, 1);
		i1++;
		if (i < len2) {
			display_array[i1] = tmp_arr2[i];
			mark_array_write();
			print_array_bars("Weaving", &(display_array[i1]), 0, NULL, 0, 1);
			i1++;
		}
	}
}

static void cross_weave_1() {
	int len1 = display_array_len / 2 + 1;
	int len2 = display_array_len - len1;
	vis_int_t tmp_arr1[len1];
	vis_int_t tmp_arr2[len2];
	assert(len2 <= len1);
	for (int i = 0; i < len1; i++) {
		tmp_arr1[i] = display_array[i];
		mark_aux_array_write();
		print_array_bars("Weaving", &(display_array[i]), 0, NULL, 0, 1);
	}
	for (int i = 0, j = len2 - 1; i < len2; i++, j--) {
		tmp_arr2[i] = display_array[j + len1];
		mark_aux_array_write();
		print_array_bars("Weaving", &(display_array[j + len1]), 0, NULL, 0, 1);
		assert(j >= 0);
		assert(j < len2);
	}
	int i1 = 0;
	for (int i = 0; i < len1; i++) {
		display_array[i1] = tmp_arr1[i];
		mark_array_write();
		print_array_bars("Weaving", &(display_array[i1]), 0, NULL, 0, 1);
		i1++;
		if (i < len2) {
			display_array[i1] = tmp_arr2[i];
			mark_array_write();
			print_array_bars("Weaving", &(display_array[i1]), 0, NULL, 0, 1);
			i1++;
		}
	}
}

static void cross_weave_2() {
	int len1 = display_array_len / 2 + 1;
	int len2 = display_array_len - len1;
	vis_int_t tmp_arr1[len1];
	vis_int_t tmp_arr2[len2];
	assert(len2 <= len1);
	for (int i = 0, j = len1 - 1; i < len1; i++, j--) {
		tmp_arr1[i] = display_array[j];
		mark_aux_array_write();
		print_array_bars("Weaving", &(display_array[j]), 0, NULL, 0, 1);
		assert(j >= 0);
		assert(j < len1);
	}
	for (int i = 0; i < len2; i++) {
		tmp_arr2[i] = display_array[i + len1];
		mark_aux_array_write();
		print_array_bars("Weaving", &(display_array[i + len1]), 0, NULL, 0, 1);
	}
	int i1 = 0;
	for (int i = 0; i < len1; i++) {
		display_array[i1] = tmp_arr1[i];
		mark_array_write();
		print_array_bars("Weaving", &(display_array[i1]), 0, NULL, 0, 1);
		i1++;
		if (i < len2) {
			display_array[i1] = tmp_arr2[i];
			mark_array_write();
			print_array_bars("Weaving", &(display_array[i1]), 0, NULL, 0, 1);
			i1++;
		}
	}
}

static void perform_selected_shuffle(const char* term, int* idendical_array, int actual_len) {
	if (term == NULL || strcmp(term, "random") == 0) {
		shuffle_display_array();
		return;
	}
	if (strcmp(term, "reverse") == 0) {
		reverse_display_array();
		return;
	}
	if (strcmp(term, "noshuffle") == 0) {
		return;
	}
	if (strcmp(term, "slightshuffle") == 0) {
		slightly_shuffle_display_array();
		return;
	}
	if (strcmp(term, "circlepass") == 0) {
		first_circle_pass();
		return;
	}
	if (strcmp(term, "shuffletail") == 0) {
		shuffle_tail();
		return;
	}
	if (strcmp(term, "shufflehead") == 0) {
		shuffle_head();
		return;
	}
	if (strcmp(term, "mergepass") == 0) {
		final_merge_pass();
		return;
	}
	if (strcmp(term, "reversemergepass") == 0) {
		reverse_final_merge_pass();
		return;
	}
	if (strcmp(term, "heapified") == 0) {
		heapify_display_array(display_array_len);
		return;
	}
	if (strcmp(term, "sortedheapinput") == 0) {
		build_sorted_heap();
		return;
	}
	if (strcmp(term, "binarysearchtree") == 0) {
		build_binary_search_tree();
		return;
	}
	if (strcmp(term, "quarters") == 0) {
		split_quarters();
		return;
	}
	if (strcmp(term, "reversequarters") == 0) {
		reverse_split_quarters();
		return;
	}
	if (strcmp(term, "mountain") == 0) {
		pipe_organ_input();
		return;
	}
	if (strcmp(term, "antiquicksort") == 0) {
		highest_item = display_array_len - 1;
		if (antiqsort_v(display_array, display_array_len, idendical_array, actual_len) < 0) {
			error_pause(error_handle_mode);
		}
		return;
	}
	if (strcmp(term, "halfreverse") == 0) {
		half_reverse_display_array();
		return;
	}
	if (strcmp(term, "reverseodds") == 0) {
		reverse_odds();
		return;
	}
	if (strcmp(term, "reverseevens") == 0) {
		reverse_evens();
		return;
	}
	if (strcmp(term, "shuffleodds") == 0) {
		shuffle_odds();
		return;
	}
	if (strcmp(term, "shuffleevens") == 0) {
		shuffle_evens();
		return;
	}
	if (strcmp(term, "bitonicpass") == 0) {
		final_bitonic_pass();
		return;
	}
	if (strcmp(term, "radixpass") == 0) {
		final_radix_pass();
		return;
	}
	if (strcmp(term, "crossweave1") == 0) {
		cross_weave_1();
		return;
	}
	if (strcmp(term, "crossweave2") == 0) {
		cross_weave_2();
		return;
	}
	if (strcmp(term, "triangular") == 0) {
		triangular_shuffle(display_array, display_array_len);
		return;
	}
	if (strcmp(term, "smoothheapified") == 0) {
		reverse_display_array();
		clear_screen_(1);
		smooth_heapify(display_array, display_array_len);
		return;
	}
}

char shuffle_names[][38] = {
	"randomly shuffled",
	"reversed",
	"no shuffle",
	"slightly shuffled",
	"shuffled tail",
	"shuffled head",
	"final merge pass",
	"reverse final merge pass",
	"heapified",
	"first circle pass",
	"sorted heap input",
	"anti-quicksort",
	"binary search tree",
	"quarters",
	"reverse quarters",
	"mountain",
	"half reversed",
	"odds reversed",
	"evens reversed",
	"shuffled odds",
	"shuffled evens",
	"final bitonic pass",
	"final radix pass",
	"cross weave - higher half descending",
	"cross weave - lower half descending",
	"triangular input",
	"smooth heapified"
};

static char* get_shuffle_display_name(const char* term) {
	if (term == NULL || strcmp(term, "random") == 0) {
		return shuffle_names[0];
	}
	if (strcmp(term, "reverse") == 0) {
		return shuffle_names[1];
	}
	if (strcmp(term, "noshuffle") == 0) {
		return shuffle_names[2];
	}
	if (strcmp(term, "slightshuffle") == 0) {
		return shuffle_names[3];
	}
	if (strcmp(term, "circlepass") == 0) {
		return shuffle_names[9];
	}
	if (strcmp(term, "shuffletail") == 0) {
		return shuffle_names[4];
	}
	if (strcmp(term, "shufflehead") == 0) {
		return shuffle_names[5];
	}
	if (strcmp(term, "mergepass") == 0) {
		return shuffle_names[6];
	}
	if (strcmp(term, "reversemergepass") == 0) {
		return shuffle_names[7];
	}
	if (strcmp(term, "heapified") == 0) {
		return shuffle_names[8];
	}
	if (strcmp(term, "sortedheapinput") == 0) {
		return shuffle_names[10];
	}
	if (strcmp(term, "antiquicksort") == 0) {
		return shuffle_names[11];
	}
	if (strcmp(term, "binarysearchtree") == 0) {
		return shuffle_names[12];
	}
	if (strcmp(term, "quarters") == 0) {
		return shuffle_names[13];
	}
	if (strcmp(term, "reversequarters") == 0) {
		return shuffle_names[14];
	}
	if (strcmp(term, "mountain") == 0) {
		return shuffle_names[15];
	}
	if (strcmp(term, "halfreverse") == 0) {
		return shuffle_names[16];
	}
	if (strcmp(term, "reverseodds") == 0) {
		return shuffle_names[17];
	}
	if (strcmp(term, "reverseevens") == 0) {
		return shuffle_names[18];
	}
	if (strcmp(term, "shuffleodds") == 0) {
		return shuffle_names[19];
	}
	if (strcmp(term, "shuffleevens") == 0) {
		return shuffle_names[20];
	}
	if (strcmp(term, "bitonicpass") == 0) {
		return shuffle_names[21];
	}
	if (strcmp(term, "radixpass") == 0) {
		return shuffle_names[22];
	}
	if (strcmp(term, "crossweave1") == 0) {
		return shuffle_names[23];
	}
	if (strcmp(term, "crossweave2") == 0) {
		return shuffle_names[24];
	}
	if (strcmp(term, "triangular") == 0) {
		return shuffle_names[25];
	}
	if (strcmp(term, "smoothheapified") == 0) {
		return shuffle_names[26];
	}
	return shuffle_names[2];
}

// ========== End shuffle functions ==========

// ========== Usage info functions ==========

static char is_help_arg(const char* arg) {
	return strcmp(arg, "h") == 0 || strcmp(arg, "-h") == 0 || strcmp(arg, "help") == 0 || strcmp(arg, "-help") == 0 || strcmp(arg, "--help") == 0;
}

static void print_usage_info(const char* arg0, size_t nvis_args, vis_arg_t *vis_args) {
	char sep = '/';
	#ifdef _WIN32
	sep = '\\';
	#endif
	const char* basename = strrchr(arg0, sep);
	if (basename == NULL) {
		basename = arg0;
	}

	const int current_longest_option = 13;
	int extra_space_needed = 0;

	printf("\n");
	printf("usage: %s [nologsound] [delay_ms] [nitems] [shuffle_mode]", basename);
	for (size_t i = 0; i < nvis_args; i++) {
		if (vis_args[i].should_parse) {
			assert(vis_args[i].name != NULL);

			size_t argn_len = strlen(vis_args[i].name);
			int extra_space = argn_len > current_longest_option ? argn_len - current_longest_option : 0;
			if (extra_space_needed < extra_space) {
				extra_space_needed = extra_space;
			}

			printf(" [%s]", vis_args[i].name);
		}
	}
	printf(" [tone_interval] [wave_shape] [max_amplidude]\n");
	printf("       %s h|help\n\n", basename);
	printf("       nologsound    [Optional] Enter this term for this app not to write sound data.\n");
	printf("                     (Sound is located in .sbeep files in sounds folder and can be converted using make_wav)\n");
	printf("                     [NOTE: this is the only argument that does not need to be written before anything else!]\n");
	printf("       delay_ms      [Optional] The delay for each print in milliseconds(ms)\n");
	printf("                     Note that for some command line interfaces there may be additional delay due to time taken to print.\n");
	printf("                     [1 second = 1000ms] Default is %dms\n", DEFAULT_DIS_DELAY);
	printf("       nitems        [Optional] The number of items in the array. Default is %d.\n", DEFAULT_ARR_LEN);
	printf("       shuffle_mode  [Optional] The shuffle before sorting. Default is 'random'.\n");
	printf("                     The possible values are:\n");
	printf("                       'random'           Shuffles randomly. This is default.\n");
	printf("                       'reverse'          Reverses the array.\n");
	printf("                       'halfreverse'      Reverses the array halfway.\n");
	printf("                       'reverseodds'      Reverses the odd numbers.\n");
	printf("                       'reverseevens'     Reverses the even numbers.\n");
	printf("                       'noshuffle'        The array is already sorted.\n");
	printf("                       'slightshuffle'    Shuffles array slightly.\n");
	printf("                       'shuffletail'      Divides the array into 3:1 section and shuffles the smaller section at the back.\n");
	printf("                       'shufflehead'      Divides the array into 1:3 section and shuffles the smaller section at the front.\n");
	printf("                       'mergepass'        Divides the array into 1:1 section.\n");
	printf("                       'reversemergepass' Divides the array into 1:1 section and reverses the array.\n");
	printf("                       'heapified'        Performs heapify on the array.\n");
	printf("                       'smoothheapified'  Reverses and performs Leonardo heapify on the array.\n");
	printf("                       'sortedheapinput'  Similar to heapified, but each hirachy is sorted.\n");
	printf("                       'circlepass'       Shuffles randomly, then performs a first pass of circle sort.\n");
	printf("                       'antiquicksort'    Quick sort killer adversary input.\n");
	printf("                       'quarters'         Divites the array into 1:1:1:1 sections.\n");
	printf("                       'reversequarters'  Divites the array into 1:1:1:1 sections and reverses the array.\n");
	printf("                       'binarysearchtree' Generates a binary search tree from the input, and places remaining elements at the back.\n");
	printf("                       'mountain'         Front half ascending, back half descending.\n");
	printf("                       'bitonicpass'      Front half descending, back half ascending.\n");
	printf("                       'shuffleodds'      Shuffles only the odd numbers.\n");
	printf("                       'shuffleevens'     Shuffles only the even numbers.\n");
	printf("                       'radixpass'        Put the first half and second half elements next to each other.\n");
	printf("                       'crossweave1'      Put the first half and second half elements next to each other, with the second half in reverse.\n");
	printf("                       'crossweave2'      Put the first half and second half elements next to each other, with the first half in reverse.\n");
	printf("                       'triangular'       Generates a \"triangular\" input.\n");
	for (size_t i = 0; i < nvis_args; i++) {
		if (vis_args[i].should_parse) {
			assert(vis_args[i].name != NULL);
			assert(vis_args[i].info != NULL);

			printf("       %s ", vis_args[i].name);

			size_t namelen = strlen(vis_args[i].name);
			if (namelen < current_longest_option + extra_space_needed) {
				int padding = current_longest_option + extra_space_needed - namelen;
				char spaces[padding + 1];
				spaces[padding] = '\0';
				for (int i = 0; i < padding; i++) {
					spaces[i] = ' ';
				}
				printf("%s", spaces);
			}
			fputs(vis_args[i].info, stdout);
		}
	}
	printf("       tone_interval [Optional] The interval between different elements. Default is 'cent'.\n");
	printf("                     The possible values are:\n");
	printf("                       'cent'                The tone interval increases by cents. 1 semitone = 100 cents. This is default.\n");
	printf("                       'semitone'            The tone interval increases by semitones or half notes.\n");
	printf("                       'majorscale'          The tone interval increases according to a major scale (Key set in chord_start_semitone in settings.txt).\n");
	printf("                       'minorscale'          The tone interval increases according to a neutral minor scale (Key set in chord_start_semitone in settings.txt).\n");
	printf("                       'melodicminorscale'   The tone interval increases according to a melodic minor scale (Key set in chord_start_semitone in settings.txt).\n");
	printf("                       'harmonicminorscale'  The tone interval increases according to a harmonic minor scale (Key set in chord_start_semitone in settings.txt).\n");
	printf("                       'chord'               Defaults to 'chord_i' below.\n");
	printf("                       'chord_i'-'chord_vii' The tone interval increases by arpeggios of the respective major chord interval.\n");
	printf("                       'chord_min_i' -       The tone interval increases by arpeggios of the respective minor chord interval.\n");
	printf("                       'chord_min_vii'\n");
	printf("                       'chord_progression'   The tone interval increases by arpeggios of chords in a simple progression.\n");
	printf("                       [More possible values are generated in the '%s' file. Remember to add 'chord_' before each of the values.]\n", CHORD_PROGRESS_FILE);
	printf("       wave_shape    [Optional] The texture of the sound. Default is 'square'.\n");
	printf("                     The possible values are:\n");
	printf("                       'square'           Gives some pixe-ly feel. This is default.\n");
	printf("                       'sine'             Standard synth organ-like timbre.\n");
	printf("                       'saw'              Rich but rough sounding.\n");
	printf("                       'triangle'         Rough texture but weak.\n");
	printf("                       'violin'           A violin-like tone.\n");
	printf("                       'electronicvoice1' An attempt to synthesize a choir-CCA voice.\n");
	printf("                       'customised'       Customised wave. More information in '%s' file\n", ADDITIVE_FREQ_FILE);
	printf("       h | help      Displays this page and exits.\n\n");
	printf("Note that a '%s' is also generated amongst the executables that contains some adjustable values.\n", SETTINGS_FILE);
	printf("This file is read by all of the sorting executables.\n");
	printf("Some of the settings in the files are repeated here, and they take the same values as stated above.\n");
	printf("Note that the arguments passed in will override the settings stated in the '%s' file.\n\n", SETTINGS_FILE);
	printf("Can't see everything? You can try logging the message into a file using './<sort_name> help>log.txt'. Then open with your favourite text editor (like VIM)!\n\n");
}

// ========== End usage info functions ==========

// ========== Optimization functions ==========

static int buffer_set_sizing(int rows, int cols, unsigned char opt_level) {
	fflush(stdout);
	assert(opt_level != 0);
	int ret = 0;
	char *new_buffer = (char *) malloc(sizeof(char) * rows * cols * opt_level);
	if (new_buffer == NULL) {
		perror("Set stdout buffer sizing failed");
		error_pause(error_handle_mode);
		return -1;
	}
	ret = setvbuf(stdout, new_buffer, _IOFBF, rows * cols * opt_level);
	if (ret != 0) {
		free(new_buffer);
		perror("Set stdout buffer sizing failed");
		error_pause(error_handle_mode);
		return ret;
	}
	print_buffer_size = rows * cols * opt_level;
	free(print_stdout_buffer);
	print_stdout_buffer = new_buffer;
	return ret;
}

static int buffer_optimize_init(unsigned char opt_level) {
	int rows = highest_item + non_array_lines + 1;
	int cols = display_array_len + 1;
	return buffer_set_sizing(rows, cols, opt_level);
}

static int buffer_optimize_cleanup(char force) {
	int r = setvbuf(stdout, NULL, _IOLBF, 65536);
	if (r != 0) {
		perror("Cleanup failed");
		error_pause(error_handle_mode);
		if (!force) {
			return r;
		}
	}
	free(print_stdout_buffer);
	print_stdout_buffer = NULL;
	print_buffer_size = 0;
	return 0;
}

// ========== End optimization functions ==========

// ========== Graphic functions ==========

#define GRAPH_BAR 1
#define GRAPH_PLOTS 0
#define GRAPH_JUPITERBJY_DIGITS 2

static int get_display_mode(const char* arg) {
	if (arg == NULL) {
		return GRAPH_BAR;
	}
	if (strcmp(arg, "bar") == 0) {
		return GRAPH_BAR;
	}
	if (strcmp(arg, "plot") == 0) {
		return GRAPH_PLOTS;
	}
	if (strcmp(arg, "digits") == 0) {
		return GRAPH_JUPITERBJY_DIGITS;
	}
	return GRAPH_BAR;
}

// ========== End graphic functions ==========

// ========== Helper Functions ==========

static int find_max_int(vis_int_t* arr, size_t len) {
	int r = INT_MIN;
	for (; len > 0; len--, arr++) {
		if (arr -> num > r) {
			r = arr -> num;
		}
	}
	return r;
}

/**
 * Cross-platform sleep function for C
 * https://gist.github.com/rafaelglikis/ee7275bf80956a5308af5accb4871135
 * @param int milliseconds
 */
#ifdef WIN32
	#include <synchapi.h>
#elif _POSIX_C_SOURCE < 199309L
	#include <unistd.h>
#endif // WIN32

void sleep_ms(int milliseconds) {
	if (milliseconds == 0) {
		return;
	}
	clock_t t0 = clock();
	#ifdef WIN32
		if (milliseconds > 0 && milliseconds < 10) {
			mstime_t spin_lock_start = mstime();
			mstime_t t1 = mstime();
			while (t1 - spin_lock_start < milliseconds && t1 >= spin_lock_start) {
				t1 = mstime();
			}
		} else {
			Sleep(milliseconds);
		}
	#elif _POSIX_C_SOURCE >= 199309L
		struct timespec ts;
		ts.tv_sec = milliseconds / 1000;
		ts.tv_nsec = (milliseconds % 1000) * 1000000;
		nanosleep(&ts, NULL);
	#else
		usleep(milliseconds * 1000);
	#endif
	if (delay_secs == INT_MAX) {
		delay_secs_overflow = 1;
	} else {
		// delay_secs += milliseconds * CLOCKS_PER_SEC / 1000;
		delay_secs += clock() - t0;
	}
}

// Clear screen from https://cplusplus.com/articles/4z18T05o/

#ifdef _WIN32
#include <windows.h>

static HANDLE                     h_std_out;
static CONSOLE_SCREEN_BUFFER_INFO csbi;
static DWORD                      count;
static DWORD                      cell_count;
static COORD                      home_coords = {0, 0};

static void clear_screen_win32_init() {
	h_std_out = GetStdHandle(STD_OUTPUT_HANDLE);
}

static void clear_screen_win32(char complete) {
	if (h_std_out == INVALID_HANDLE_VALUE) {
		printf("\n\n------------------------------\n\n");
		return;
	}

	// Get the number of cells in the current buffer
	if (!GetConsoleScreenBufferInfo(h_std_out, &csbi)) {
		printf("\n\n------------------------------\n\n");
		return;
	}
	cell_count = csbi.dwSize.X * csbi.dwSize.Y;

	// Fill the entire buffer with spaces
	if (complete && !FillConsoleOutputCharacter(
		h_std_out,
		(TCHAR) ' ',
		cell_count,
		home_coords,
		&count
	)) {
		printf("\n\n------------------------------\n\n");
		return;
	}

	// Fill the entire buffer with the current colors and attributes
	if (complete && !FillConsoleOutputAttribute(
		h_std_out,
		csbi.wAttributes,
		cell_count,
		home_coords,
		&count
	)) {
		printf("\n\n------------------------------\n\n");
		return;
	}

	/* Move the cursor home */
	SetConsoleCursorPosition(h_std_out, home_coords);
}
#endif // _WIN32

#include <unistd.h>
#if !defined(NO_NCURSES) && (defined (__unix__) || (defined (__APPLE__) && defined (__MACH__)))
#include <term.h>
#endif
static void clear_screen_(char complete) {
	clock_t t0 = clock();
	#ifdef USE_ANSI_CLEAR
		write(STDOUT_FILENO, "\e[1;1H\e[2J", 12);
	#elif defined _WIN32
		clear_screen_win32(complete);
	#elif defined (__unix__) || (defined (__APPLE__) && defined (__MACH__))
		if (isatty(STDOUT_FILENO)) {
			printf("\033[%d;%dH", 0, 0);
			if (complete) {
				int screen_char_count = pgcg_get_console_cols() * (pgcg_get_console_rows() - 1) - 1;
				char spaces[screen_char_count];
				for (int i = 0; i < screen_char_count; i++) {
					spaces[i] = ' ';
				}
				printf("%s", spaces);
				printf("\033[%d;%dH", 0, 0);
			}
		} else {
		#ifndef NO_NCURSES
			if (!cur_term) {
				int result;
				setupterm(NULL, STDOUT_FILENO, &result);
				if (result <= 0) {
		#endif // NO_NCURSES
					printf("\n\n------------------------------\n\n");
		#ifndef NO_NCURSES
					return;
				}
			}
			putp(tigetstr("clear"));
		#endif // NO_NCURSES
		}
	#else
	printf("\n\n------------------------------\n\n");
	#endif // _WIN32
	if (delay_secs == INT_MAX) {
		delay_secs_overflow = 1;
	} else {
		delay_secs += clock() - t0;
	}
}

#ifndef _WIN32
#include <sys/ioctl.h>
#endif

static int pgcg_get_console_rows() {
	#ifdef _WIN32
	HANDLE console_handle = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	GetConsoleScreenBufferInfo(console_handle, &csbi);
	return (int) csbi.srWindow.Bottom - (int) csbi.srWindow.Top + 1;
	#else
	struct winsize ws;
	ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws);
	return ws.ws_row;
	#endif // _WIN32
}

static int pgcg_get_console_cols() {
	#ifdef _WIN32
	HANDLE console_handle = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	GetConsoleScreenBufferInfo(console_handle, &csbi);
	return (int) csbi.dwSize.X;
	#else
	struct winsize ws;
	ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws);
	return ws.ws_col;
	#endif // _WIN32
}

static char should_include_sleep_time(int test_delay) {
	time_t t0 = clock();
	sleep_ms(test_delay);
	return (double) ((clock() - t0) * 1000 / CLOCKS_PER_SEC) < (double) (test_delay);
}

static int larger_int_back(const void* a, const void* b) {
	mark_comparison();
	char prefixtxt[] = "Sorting";
	char catprefix[get_concat_sort_name_len(prefixtxt)];
	cpy_concat_sort_name(catprefix, prefixtxt);
	print_array_bars(catprefix, a, 1, b, 1, 1);
	return (((vis_int_t*)a) -> num) - (((vis_int_t*)b) -> num);
}

#include <math.h>
static void print_time(double secs, int ncols) {
	int hour = ((int)secs / 3600) % 24;
	int min = ((int)secs / 60) % 60;
	double s = fmod(secs, 60);
	if (secs >= 86400) {
		pnt_trunc_f("%dd %s", ncols, (int)secs / 86400, hour < 10 ? "0" : "");
	}
	if (secs >= 3600) {
		pnt_trunc_f("%dh %s", ncols, hour, min < 10 ? "0" : "");
	}
	if (secs >= 60) {
		pnt_trunc_f("%dm %s", ncols, min, s < 10 ? "0" : "");
	}
	pnt_trunc_f("%fs", ncols, s);
}

static void print_mstime(mstime_t msecs, int ncols) {
	unsigned long hour = ((unsigned long) msecs / 3600000) % 24;
	unsigned long min = ((unsigned long) msecs / 60000) % 60;
	double s = fmod((double) msecs / 1000.0, 60.0);
	if (msecs >= 86400000) {
		pnt_trunc_f("%lud %s", ncols, msecs / 86400000, hour < 10 ? "0" : "");
	}
	if (msecs >= 3600000) {
		pnt_trunc_f("%luh %s", ncols, hour, min < 10 ? "0" : "");
	}
	if (msecs >= 60000) {
		pnt_trunc_f("%lum %s", ncols, min, s < 10 ? "0" : "");
	}
	pnt_trunc_f("%.3fs", ncols, s);
}

static void print_time_long(time_t secs, int ncols) {
	unsigned long hour = ((unsigned long) secs / 3600) % 24;
	unsigned long min = ((unsigned long) secs / 60) % 60;
	unsigned long s = (unsigned long) secs % 60;
	if (secs >= 86400) {
		pnt_trunc_f("%lud %s", ncols, secs / 86400, hour < 10 ? "0" : "");
	}
	if (secs >= 3600) {
		pnt_trunc_f("%luh %s", ncols, hour, min < 10 ? "0" : "");
	}
	if (secs >= 60) {
		pnt_trunc_f("%lum %s", ncols, min, s < 10 ? "0" : "");
	}
	pnt_trunc_f("%lus", ncols, s);
}

static size_t get_concat_sort_name_len(const char* prefix) {
	return strlen(display_name) + strlen(prefix) + strlen(shuffle_name) + 7;
}

static void cpy_concat_sort_name(char* __restrict__ concat_buffer, const char* prefix) {
	strcpy(concat_buffer, prefix);
	strcat(concat_buffer, " - ");
	strcat(concat_buffer, display_name);
	strcat(concat_buffer, " - ");
	strcat(concat_buffer, shuffle_name);
}

static void print_version_top_right(const char *prefix, int ncols) {
	pnt_trunc_f("%s", ncols, prefix);
	if (show_ver_no) {
	#if defined(IS_SNAPSHOT) && defined(VERSION) && defined(BUILD)
		char is_snapshot = IS_SNAPSHOT;
		char version[] = VERSION;
		char build[] = BUILD;
		char ver_pref[strlen(version) + 1 + 8 + strlen(build) + 2];
		strcpy(ver_pref, "v");
		strcat(ver_pref, version);
		if (show_build_no) {
			strcat(ver_pref, " (Build ");
			strcat(ver_pref, build);
			strcat(ver_pref, ")");
		}
		char build_pref[strlen(build) + 7];
		strcpy(build_pref, "Build ");
		strcat(build_pref, build);
		char *print_ver = is_snapshot ? build_pref : ver_pref;
		size_t verlen = strlen(print_ver);
		size_t prefix_len = strlen(prefix);
		if (verlen + prefix_len <= ncols - 1) {
			size_t nspaces = (size_t) ncols - 1 - strlen(print_ver) - strlen(prefix);
			char spaces[nspaces + 1];
			for (int i = 0; i < nspaces; i++) {
				spaces[i] = ' ';
			}
			spaces[nspaces] = '\0';
			pnt_trunc_f("%s%s", ncols, spaces, print_ver);
		}
	#endif
	}
	pnt_trunc("\n\n", ncols);
}

static char jupiterbjy_number_representation(int i) {
	i %= 10;
	if (i == 0) {
		return 'X';
	}
	return '0' + i;
}

static char jupiterbjy_number_representation2(int i) {
	if (i < 0) {
		return ' ';
	}
	if (i > -9 * (stability_test_highest_tag + 1) / 10 && i < (stability_test_highest_tag + 1) * 9 / 10) {
		return '1' + i * 10 / (stability_test_highest_tag + 1);
	}
	return i == stability_test_highest_tag ? ' ' : 'X';
}

static char pointer_in_array(const void* p) {
	return p >= (void*) display_array && p < ((void*) display_array) + sizeof(vis_int_t) * display_array_len;
}

/**
 * @brief Thanks https://stackoverflow.com/questions/101439/the-most-efficient-way-to-implement-an-integer-based-power-function-powint-int
 *
 * @param base a
 * @param exp n
 * @return int a to the power of n
 */
static size_t stpow(size_t base, size_t exp) {
	size_t result = 1;
	for (;;) {
		if (exp & 1)
			result *= base;
		exp >>= 1;
		if (!exp)
			break;
		base *= base;
	}
	return result;
}

static size_t int_num_digits(const void* a, size_t b) {
	int n = ((vis_int_t*)a) -> num;
	if (n == 0) {
		return 1;
	}
	size_t r = 0;
	while (n > 0) {
		r++;
		n /= b;
	}
	return r;
}

static size_t base_n_modulo(const void* a, size_t b) {
	return ((((vis_int_t*)a) -> num) / stpow(radix_base, b - 1)) % radix_base;
}

/**
 * @brief Generates a signed 32-bit sdbm hash from a string.
 */
static int dwhash(const char* s) {
	int hash = 0;
	while (*s) {
		hash = *s + (hash << 6) + (hash << 16) - hash;
		s++;
	}
	return hash;
}

/** String "cent" */
#define cent_hash 0x304cf348
/** String "semitones" */
#define semitones_hash 0x9d263c93
/** String "chordprogress" */
#define chordprogress_hash 0xe93534c9
/** String "chord" */
#define chord_hash 0x6248605c
/** String "chord_i" */
#define chord_i_hash 0x31c50a26
/** String "chord_ii" */
#define chord_ii_hash 0x49a37fc3
/** String "chord_iii" */
#define chord_iii_hash 0x9eff7166
/** String "chord_iv" */
#define chord_iv_hash 0x49a37fd0
/** String "chord_v" */
#define chord_v_hash 0x31c50a33
/** String "chord_vi" */
#define chord_vi_hash 0x49b082f6
/** String "chord_vii" */
#define chord_vii_hash 0xa5663af3
/** String "chord_min_i" */
#define chord_min_i_hash 0xf917ffb9
/** String "chord_min_ii" */
#define chord_min_ii_hash 0x4ca0eef0
/** String "chord_min_iii" */
#define chord_min_iii_hash 0xca8acd79
/** String "chord_min_iv" */
#define chord_min_iv_hash 0x4ca0eefd
/** String "chord_min_v" */
#define chord_min_v_hash 0xf917ffc6
/** String "chord_min_vi" */
#define chord_min_vi_hash 0x4cadf223
/** String "chord_min_vii" */
#define chord_min_vii_hash 0xd0f19706
/** String "chord_aug" */
#define chord_aug_hash 0x9b1af850
/** String "chord_dim" */
#define chord_dim_hash 0x9c8923e5
/** String "chord_dim7" */
#define chord_dim7_hash 0xa9a4d592
/** String "chord_min7_flat_5" */
#define chord_min7_flat_5_hash 0xeeb8cd46
/** String "chord_min7" */
#define chord_min7_hash 0x4c632c88
/** String "chord_min_maj7" */
#define chord_min_maj7_hash 0x5bcd5831
/** String "chord_dom7" */
#define chord_dom7_hash 0xac993298
/** String "chord_maj7" */
#define chord_maj7_hash 0x486eaf84
/** String "chord_aug7" */
#define chord_aug7_hash 0x23f31be7
/** String "chord_aug_maj7" */
#define chord_aug_maj7_hash 0x25c58890
/** String "chord_dom9" */
#define chord_dom9_hash 0xac99329a
/** String "chord_dom11" */
#define chord_dom11_hash 0xac45721f
/** String "chord_dom13" */
#define chord_dom13_hash 0xac457221
/** String "chord_sus2" */
#define chord_sus2_hash 0x6979cc44
/** String "chord_sus4" */
#define chord_sus4_hash 0x6979cc46
/** String "chord_9sus4" */
#define chord_9sus4_hash 0x01df4c39
/** String "major_scale" */
#define major_scale_hash 0x71e76671
/** String "minor_scale" */
#define minor_scale_hash 0x96fc1d75
/** String "melodicminorscale" */
#define melodic_minor_scale_hash 0xdfdba098
/** String "harmonicminorscale" */
#define harmonic_minor_scale_hash 0x7d2a3962

static int get_chord_mode(const char *arg, int* __restrict__ chord_id) {
	int dhash = dwhash(arg);
	switch (dhash) {
	case cent_hash:
		return CHORD_MODE_OFF;
	case semitones_hash:
		return CHORD_MODE_NOTE;
	case chordprogress_hash:
		return CHORD_MODE_SIMPLE_PROGRESSION;
	case chord_hash:
	case chord_i_hash:
		*chord_id = I_chord;
		return CHORD_MODE_SINGLE;
	case chord_ii_hash:
		*chord_id = ii_chord;
		return CHORD_MODE_SINGLE;
	case chord_iii_hash:
		*chord_id = iii_chord;
		return CHORD_MODE_SINGLE;
	case chord_iv_hash:
		*chord_id = IV_chord;
		return CHORD_MODE_SINGLE;
	case chord_v_hash:
		*chord_id = V_chord;
		return CHORD_MODE_SINGLE;
	case chord_vi_hash:
		*chord_id = vi_chord;
		return CHORD_MODE_SINGLE;
	case chord_vii_hash:
		*chord_id = vii_chord;
		return CHORD_MODE_SINGLE;
	case chord_min_i_hash:
		*chord_id = i_min_chord;
		return CHORD_MODE_SINGLE;
	case chord_min_ii_hash:
		*chord_id = ii_dim_chord;
		return CHORD_MODE_SINGLE;
	case chord_min_iii_hash:
		*chord_id = III_maj_chord;
		return CHORD_MODE_SINGLE;
	case chord_min_iv_hash:
		*chord_id = iv_min_chord;
		return CHORD_MODE_SINGLE;
	case chord_min_v_hash:
		*chord_id = v_min_chord;
		return CHORD_MODE_SINGLE;
	case chord_min_vi_hash:
		*chord_id = VI_maj_chord;
		return CHORD_MODE_SINGLE;
	case chord_min_vii_hash:
		*chord_id = VII_maj_chord;
		return CHORD_MODE_SINGLE;
	case chord_aug_hash:
		*chord_id = aug_chord_i;
		return CHORD_MODE_SINGLE;
	case chord_dim_hash:
		*chord_id = dim_chord_i;
		return CHORD_MODE_SINGLE;
	case chord_dim7_hash:
		*chord_id = dim7_chord;
		return CHORD_MODE_SINGLE;
	case chord_min7_flat_5_hash:
		*chord_id = min7_flat_5_chord;
		return CHORD_MODE_SINGLE;
	case chord_min7_hash:
		*chord_id = min7_chord;
		return CHORD_MODE_SINGLE;
	case chord_min_maj7_hash:
		*chord_id = m_maj7_chord;
		return CHORD_MODE_SINGLE;
	case chord_dom7_hash:
		*chord_id = dom7_chord;
		return CHORD_MODE_SINGLE;
	case chord_maj7_hash:
		*chord_id = maj7_chord;
		return CHORD_MODE_SINGLE;
	case chord_aug7_hash:
		*chord_id = aug7_chord;
		return CHORD_MODE_SINGLE;
	case chord_aug_maj7_hash:
		*chord_id = aug_maj7_chord;
		return CHORD_MODE_SINGLE;
	case chord_dom9_hash:
		*chord_id = dom9_chord;
		return CHORD_MODE_SINGLE;
	case chord_dom11_hash:
		*chord_id = dom11_chord;
		return CHORD_MODE_SINGLE;
	case chord_dom13_hash:
		*chord_id = dom13_chord;
		return CHORD_MODE_SINGLE;
	case chord_sus2_hash:
		*chord_id = sus2_chord;
		return CHORD_MODE_SINGLE;
	case chord_sus4_hash:
		*chord_id = sus4_chord;
		return CHORD_MODE_SINGLE;
	case chord_9sus4_hash:
		*chord_id = jz_9sus4_chord;
		return CHORD_MODE_SINGLE;
	case major_scale_hash:
		*chord_id = major_scale;
		return CHORD_MODE_SINGLE;
	case minor_scale_hash:
		*chord_id = neutral_minor_scale;
		return CHORD_MODE_SINGLE;
	case melodic_minor_scale_hash:
		*chord_id = melodic_minor_scale;
		return CHORD_MODE_SINGLE;
	case harmonic_minor_scale_hash:
		*chord_id = harmonic_minor_scale;
		return CHORD_MODE_SINGLE;
	}
	return CHORD_MODE_OFF;
}

static int get_chord_timing(int delay_ms) {
	if (delay_ms <= 0) {
		return 500 * n_crotchets_in_bar;
	}
	if (delay_ms >= 500) {
		return delay_ms * n_crotchets_in_bar;
	}
	int prev_delay_ms = 0;
	while (prev_delay_ms < ideal_crotchet_dur) {
		prev_delay_ms = delay_ms;
		delay_ms <<= 1;
		if (delay_ms < 0) {
			break;
		}
	}
	if (ideal_crotchet_dur - prev_delay_ms < delay_ms - ideal_crotchet_dur) {
		return prev_delay_ms * n_crotchets_in_bar;
	}
	return delay_ms * n_crotchets_in_bar;
}

static double get_freq_by_pointer(const vis_int_t *p) {
	if (!pointer_in_array(p)) {
		return tuning_freq;
	}
	mstime_t mst1 = mstime() - start_time_ms;
	switch (chord_mode) {
	case CHORD_MODE_OFF:
		return get_freq_between_bounds(p -> num, highest_item, tone_lower_cent_bound, tone_upper_cent_bound, tuning_freq);
	case CHORD_MODE_NOTE:
		return get_freq_between_bounds_semitones(p -> num, highest_item, tone_lower_cent_bound / 100, tone_upper_cent_bound / 100, tuning_freq);
	case CHORD_MODE_SINGLE:
		return get_freq_between_bounds_arpeggio(p -> num, highest_item, chord_start_semitone, chord_octave_range, chords[chord_no], tuning_freq, use_eq_temp);
	case CHORD_MODE_SIMPLE_PROGRESSION:
		return get_freq_between_bounds_arpeggio(p -> num, highest_item, chord_start_semitone, chord_octave_range, custom_chord_progression[is_sorting ? ((int) mst1 / chord_progression_timing_ms) % custom_chord_progression_len : 0], tuning_freq, use_eq_temp);
	}
	return get_freq_between_bounds(p -> num, highest_item, tone_lower_cent_bound, tone_upper_cent_bound, tuning_freq);
}

// struct opt_hash {
// 	char opt[30];
// 	const int hash;
// };

/*
 * NOTE: changing the strings below without updating the corresponding hashes macro will break the reading functionalities.
 */
static char options[][31] = {
	"log_sound",
	"default_delay",
	"default_nitems",
	"default_shuffle",
	"bar_char",
	"active_bar_char",
	"compare_bar_char",
	"display_style",
	"sound_texture",
	"max_sound_amplitude",
	"tone_lower_cent_bound",
	"tone_upper_cent_bound",
	"chord_start_semitone",
	"chord_octave_range",
	"ideal_crotchet_dur",
	"crotchets_in_bar",
	"tuning_frequency",
	"use_equal_temperament",
	"min_sound_len",
	"tone_interval",
	"shuffle_delay_factor",
	"verify_delay_factor",
	"use_precise_animation_time",
	"print_estimated_real_time",
	"show_version_build_number",
	"show_build_number",
	"optimisation_level",
	"when_error"
};
#define log_sound_hash 0x12f4a834
#define default_delay_hash 0xc909cb65
#define default_nitems_hash 0xa85e17d0
#define default_shuffle_hash 0x0e9be55b
#define bar_char_hash 0xd93f1802
#define active_bar_char_hash 0x62a849bb
#define compare_bar_char_hash 0xd7cd9c1c
#define display_style_hash 0x13add754
#define sound_texture_hash 0x91befb4b
#define max_sound_amplitude_hash 0x4311d058
#define tone_lower_cent_bound_hash 0x4b24de52
#define tone_upper_cent_bound_hash 0x7533a5d1
#define chord_start_semitone_hash 0x07d396e0
#define chord_octave_range_hash 0xd01571f1
#define ideal_crotchet_dur_hash 0xce849a94
#define crotchets_in_bar_hash 0xe8b2872d
#define tuning_frequency_hash 0x889e9092
#define use_equal_temperament_hash 0x928619bb
#define min_sound_len_hash 0x4efb59d8
#define tone_interval_hash 0x73a95972
#define shuffle_delay_factor_hash 0xc2929e51
#define verify_delay_factor_hash 0x0c85cad1
#define use_precise_animation_time_hash 0x671b5584
#define print_estimated_real_time_hash 0x4617ef79
#define show_version_build_number_hash 0x30d5c423
#define show_build_number_hash 0xba37a7fc
#define optimisation_level_hash 0x334bc139
#define when_error_hash 0x858ba183

#define SHUFFLE_BUFFER_SIZE 31
#define WAVE_SHAPE_BUFFER_SIZE 31
#define TONE_INT_BUFFER_SIZE 31

static char shuffle_mode_buffer[SHUFFLE_BUFFER_SIZE];
static char wave_shape_buffer[WAVE_SHAPE_BUFFER_SIZE];
static char tone_interval_buffer[TONE_INT_BUFFER_SIZE];

static double clamp_f(double d, double min, double max) {
	double r = d < min ? min : d;
	return r > max ? max : r;
}

static double change_if_nzero_f(double n, const double original) {
	if (n == 0.0) {
		return original;
	}
	return n;
}

static int change_if_nzero(int n, const int original) {
	if (n == 0) {
		return original;
	}
	return n;
}

static char is_valid_float(const char *s) {
	if (s == NULL) {
		return 0;
	}
	if (*s == 45) {
		s++;
	}
	if (*s == 46) {
		s++;
	}
	return *s >= 48 && *s < 58;
}

static char is_valid_int(const char *s) {
	if (s == NULL) {
		return 0;
	}
	if (*s == 45) {
		s++;
	}
	return *s >= 48 && *s < 58;
}

static int change_if_valid(const char* s, const int original) {
	if (!is_valid_int(s)) {
		return original;
	}
	return atoi(s);
}

static double change_if_valid_float(const char* s, const double original) {
	if (!is_valid_float(s)) {
		return original;
	}
	return atof(s);
}

static void parse_option(const char* option_name, const char* option_value, int* __restrict__ delay, int* __restrict__ arr_len, char** __restrict__ shuffle_type, char** __restrict__ chord_arg, char** __restrict__ wave_shape, char* __restrict__ use_optimization/*, int *__restrict__ n_similar_items*/) {
	int opt_hash = dwhash(option_name);
	switch (opt_hash) {
	case log_sound_hash:
		log_sound = change_if_valid(option_value, log_sound);
		return;
	case default_delay_hash:
		*delay = change_if_valid(option_value, *delay);
		return;
	case default_nitems_hash:
		*arr_len = change_if_nzero(atoi(option_value), *arr_len);
		return;
	case default_shuffle_hash:
		#pragma GCC diagnostic push
		#pragma GCC diagnostic ignored "-Wstringop-truncation"
		strncpy(shuffle_mode_buffer, option_value, SHUFFLE_BUFFER_SIZE);
		#pragma GCC diagnostic pop
		*shuffle_type = shuffle_mode_buffer;
		return;
	case bar_char_hash:
		normal_bar_char = *option_value == '\0' ? ' ' : *option_value;
		return;
	case active_bar_char_hash:
		active_bar_char = *option_value == '\0' ? ' ' : *option_value;
		return;
	case compare_bar_char_hash:
		compare_bar_char = *option_value == '\0' ? ' ' : *option_value;
		return;
	case display_style_hash:
		display_full_bar = get_display_mode(option_value);
		display_jupiterbjy_numbers = get_display_mode(option_value) == GRAPH_JUPITERBJY_DIGITS;
		return;
	case sound_texture_hash:
		#pragma GCC diagnostic push
		#pragma GCC diagnostic ignored "-Wstringop-truncation"
		strncpy(wave_shape_buffer, option_value, WAVE_SHAPE_BUFFER_SIZE);
		#pragma GCC diagnostic pop
		*wave_shape = wave_shape_buffer;
		return;
	case max_sound_amplitude_hash:
		max_sound_amplitude = clamp_f(change_if_valid_float(option_value, max_sound_amplitude), 0.0, 1.0);
		return;
	case tone_lower_cent_bound_hash:
		tone_lower_cent_bound = change_if_valid(option_value, tone_lower_cent_bound);
		return;
	case tone_upper_cent_bound_hash:
		tone_upper_cent_bound = change_if_valid(option_value, tone_upper_cent_bound);
		return;
	case chord_start_semitone_hash:
		chord_start_semitone = change_if_valid(option_value, chord_start_semitone);
		return;
	case chord_octave_range_hash:
		chord_octave_range = change_if_valid(option_value, chord_octave_range);
		return;
	case ideal_crotchet_dur_hash:
		ideal_crotchet_dur = change_if_valid(option_value, ideal_crotchet_dur);
		return;
	case crotchets_in_bar_hash:
		n_crotchets_in_bar = change_if_nzero(atoi(option_value), n_crotchets_in_bar);
		return;
	case tuning_frequency_hash:
		tuning_freq = change_if_nzero_f(atof(option_value), tuning_freq);
		return;
	case use_equal_temperament_hash:
		use_eq_temp = atoi(option_value);
		return;
	case min_sound_len_hash:
		min_sound_len = atoi(option_value);
		return;
	case tone_interval_hash:
		#pragma GCC diagnostic push
		#pragma GCC diagnostic ignored "-Wstringop-truncation"
		strncpy(tone_interval_buffer, option_value, TONE_INT_BUFFER_SIZE);
		#pragma GCC diagnostic pop
		*chord_arg = tone_interval_buffer;
		return;
	case shuffle_delay_factor_hash:
		shuffle_delay_factor = fabs(atof(option_value));
		return;
	case verify_delay_factor_hash:
		verify_delay_factor = fabs(atof(option_value));
		return;
	case use_precise_animation_time_hash:
		print_precise_time = atoi(option_value);
		return;
	case print_estimated_real_time_hash:
		print_estimated_real_time = atoi(option_value);
		return;
	case show_version_build_number_hash:
		show_ver_no = atoi(option_value);
		return;
	case show_build_number_hash:
		show_build_no = atoi(option_value);
		return;
	case optimisation_level_hash:
		*use_optimization = atoi(option_value);
		return;
	case when_error_hash:
		if (strcmp(option_value, "go_ahead") == 0) {
			error_handle_mode = ERROR_HANDLE_GO_AHEAD;
		}
		if (strcmp(option_value, "wait") == 0) {
			error_handle_mode = ERROR_HANDLE_WAIT;
		}
		if (strcmp(option_value, "pause") == 0) {
			error_handle_mode = ERROR_HANDLE_USER_IN;
		}
		return;
	}
}

static void strip_spaces(char* s) {
	char* to_write = s;
	for (;*s != '\0'; s++, to_write++) {
		if (*s == ' ' || *s == '\n' || *s == '\t') {
			to_write--;
		} else if (s != to_write) {
			*to_write = *s;
		}
	}
	*to_write = '\0';
}

#define print_spaces(n, f) for (size_t i = 0; i < (n); i++) {fputc(' ', (f));}

static void create_options_file() {
	FILE* opt = fopen(SETTINGS_FILE, "wb");
	if (opt == NULL) {
		return;
	}
	size_t last_opt_char_count;
	fprintf(opt, "%s = %d ; ", options[0], 1);
	fprintf(opt, "0 = off, 1 = on. When on it will create a .sbeep file in the sounds folder.\n");
	fprintf(opt, "%s = %d ; ", options[1], DEFAULT_DIS_DELAY);
	fprintf(opt, "The delay time in milliseconds.\n");
	fprintf(opt, "; Note that for some command line interfaces there may be additional delay due to time taken to print. This is especially so for windows.\n");
	fprintf(opt, "%s = %d ; ", options[2], DEFAULT_ARR_LEN);
	fprintf(opt, "The number of items in the array. The value is limited by the number of bars that it can fit on the screen.\n");
	fprintf(opt, "%s = %s ; ", options[3], "random");
	fprintf(opt, "The shuffle type. Enter ./<sort_name> help for more values.\n");
	fprintf(opt, "\n");
	fprintf(opt, "; For the next three options, use '\\;' for semicolons.\n");
	fprintf(opt, "\n");
	fprintf(opt, "%s = %c ; ", options[4], BAR_CHAR);
	fprintf(opt, "The character to display for idle bars. Must be ASCII.\n");
	fprintf(opt, "%s = %c ; ", options[5], ACTIVE_BAR_CHAR);
	fprintf(opt, "The character to display for accessed bars. Must be ASCII.\n");
	fprintf(opt, "%s = %c ; ", options[6], COMPARE_BAR_CHAR);
	fprintf(opt, "The character to display for compared bars. Must be ASCII.\n");
	fprintf(opt, "%s = %s ; ", options[7], DISPLAY_FULL_BAR ? (DISPLAY_JUPITERBJY_NOTATION ? "digits" : "bar") : "plot");
	fprintf(opt, "The style of the display of numbers.\n");
	last_opt_char_count = strlen(options[7]) + 3 + (DISPLAY_FULL_BAR ? (DISPLAY_JUPITERBJY_NOTATION ? 6 :3) : 4) + 1;
	print_spaces(last_opt_char_count, opt);
	fprintf(opt, "; The possible values are:\n");
	print_spaces(last_opt_char_count, opt);
	fprintf(opt, ";  1. 'bar'  - Display integers as vertical bar lines.\n");
	print_spaces(last_opt_char_count, opt);
	fprintf(opt, ";  2. 'plot' - Display integers as plots.\n");
	print_spaces(last_opt_char_count, opt);
	fprintf(opt, ";  3. 'digits' - Display 123456789X on top of bars. (Currently incomplete) Inspired by https://github.com/jupiterbjy/Sorting_in_visual \n");
	fprintf(opt, "\n");
	fprintf(opt, "%s = %s ; ", options[8], "square");
	fprintf(opt, "Timbre of sound. Only have basic options (./<sort_name> help for more info)\n");
	fprintf(opt, "%s = %f ; ", options[9], MAX_AMPLITUDE);
	fprintf(opt, "Amplidude of the sound. You can somewhat think of this as the sound volume. Value between 0 and 1.\n");
	fprintf(opt, "\n");
	fprintf(opt, "; For the next two options, note than 100 cents makes up a semitone (half-note).\n");
	fprintf(opt, "; Also A4 = The A note right after the middle-C on a piano.\n");
	fprintf(opt, "\n");
	fprintf(opt, "%s = %d ; ", options[10], VIS_LOWER_CENT_BOUND);
	fprintf(opt, "The lowest tone, in cents offset from 440Hz (A4). Can be negative.\n");
	fprintf(opt, "%s = %d ; ", options[11], VIS_HIGHER_CENT_BOUND);
	fprintf(opt, "The highest tone, in cents offset from 440Hz (A4). Can be negative.\n");
	fprintf(opt, "\n");
	fprintf(opt, "%s = %d ; ", options[12], CHORD_START_SEMITONE);
	fprintf(opt, "The base note for chord mode, in SEMITONES offset from 440Hz (A4). Can be negative.\n");
	fprintf(opt, "%s = %d ; ", options[13], CHORD_OCTAVE_RANGE);
	fprintf(opt, "The sound range in octaves for chord mode.\n");
	fprintf(opt, "%s = %d ; ", options[14], IDEAL_CROTCHET_DUR_MS);
	fprintf(opt, "The ideal duration of quater-notes in milliseconds. ");
	fprintf(opt, "Note that the application can only try its best to follow the timing.\n");
	fprintf(opt, "; To convert from bpm take 60000/bpm.\n");
	fprintf(opt, "%s = %d ; ", options[15], N_CROTCHETS_IN_BAR);
	fprintf(opt, "Number of crotchets in a bar. This is used to time the chord progression mode.\n");
	fprintf(opt, "%s = %f ; ", options[16], TUNING_FREQ);
	fprintf(opt, "The tuning frequency the sound system is based on in Hz. Default is 440Hz, but another common value is 432Hz.\n");
	fprintf(opt, "; Adjusting this too far from 440Hz or 432Hz can have undesirable effects. YOU HAVE BEEN WARNED!\n");
	fprintf(opt, "%s = %d ; ", options[17], USE_EQ_TEMP);
	fprintf(opt, "0 = off, 1 = on. When on it uses equal temperament tuning to play the notes, as opposed to justonic when off. Most home instruments are tuned in equal temperament.\n");
	fprintf(opt, "%s = %d ; ", options[18], MIN_SOUND_LEN);
	fprintf(opt, "Minimum sound length in milliseconds. Adjusting this too far from the default value can have undesirable effects.\n");
	fprintf(opt, "%s = %s ; ", options[19], "cent");
	fprintf(opt, "The tone intervals. (./<sort_name> help for more info)\n");
	fprintf(opt, "\n");
	fprintf(opt, "; For the next two options, 1.0 = same as sorting delay, lower = faster.\n");
	fprintf(opt, "\n");
	fprintf(opt, "%s = %f ; ", options[20], SHUFFLE_DELAY_FACTOR);
	fprintf(opt, "The delay for the shuffling visualization relative to the sorting visualization delay.\n");
	fprintf(opt, "%s = %f ; ", options[21], VERIFY_DELAY_FACTOR);
	fprintf(opt, "The delay for the verify sort visualization relative to the sorting visualization delay.\n");
	fprintf(opt, "\n");
	fprintf(opt, "%s = %d ; ", options[22], PRINT_PRECISE_TIME);
	fprintf(opt, "0 = off, 1 = on. Displays 'Time taken' with millisecond precision.\n");
	fprintf(opt, "%s = %d ; ", options[23], PRINT_TIME_W_O_DELAY);
	fprintf(opt, "0 = off, 1 = on. When on it prints the estimated time without delay in the info below the visualization.\n");
	fprintf(opt, "; The \"real time\" is estimated by taking the time for sorting and subtracting the delay times and the time taken to print the items on the screen.\n");
	fprintf(opt, "; This is especially inaccurate for Windows. (We tried our best to make it accurate haish)\n");
	fprintf(opt, "\n");
	fprintf(opt, "%s = %d ; ", options[24], SHOW_VER);
	fprintf(opt, "0 = off, 1 = on. Whether you want the version / build number to show on the top right corner.\n");
	fprintf(opt, "%s = %d ; ", options[25], SHOW_BUILD);
	fprintf(opt, "0 = off, 1 = on. Whether you want the build number to show on the top right corner along with the release version.\n");
	fprintf(opt, "\n");
	fprintf(opt, "%s = %d ; ", options[26], 1);
	fprintf(opt, "0 = No optimisations, 1 = Use optimisations. Optimise the terminal printing by reducing write syscalls. (Just keep this on level 1)\n");
	last_opt_char_count = strlen(options[26]) + 4 + 1;
	print_spaces(last_opt_char_count, opt);
	fprintf(opt, "; This is done by collecting characters to print for each frame and then printing all of the collected characters in one print statement.\n");
	print_spaces(last_opt_char_count, opt);
	fprintf(opt, "; Level 2 - 127 is experimental, each level increases the memory usage.\n");
	fprintf(opt, "\n");
	fprintf(opt, "%s = %s ; ", options[27], "pause");
	fprintf(opt, "How should errors be handled.\n");
	last_opt_char_count = strlen(options[27]) + 8 + 1;
	print_spaces(last_opt_char_count, opt);
	fprintf(opt, "; 'go_ahead' - Ignorable errors will be ignored and the app will continue, giving you little to no time for reading the errors.\n");
	print_spaces(last_opt_char_count, opt);
	fprintf(opt, "; 'wait' - Error information will be displayed for 5 seconds before continuing.\n");
	print_spaces(last_opt_char_count, opt);
	fprintf(opt, "; 'pause' - Pause the execution of the program until you hit enter.\n");
	fclose(opt);
}

#define READ_FILE_BUFFER 40

static void set_options_from_file(int* __restrict__ delay, int* __restrict__ arr_len, char** __restrict__ shuffle_type, char** __restrict__ chord_arg, char** __restrict__ wave_shape, char* __restrict__ use_optimization/*, int *__restrict__ n_similar_items*/) {
	FILE* opt = fopen(SETTINGS_FILE, "r");
	if (opt == NULL) {
		create_options_file();
		return;
	}
	char optln[READ_FILE_BUFFER];
	char* fg_ret = fgets(optln, READ_FILE_BUFFER, opt);
	while (fg_ret) {
		char have_newline = strchr(optln, '\n') != NULL;
		char* comment_char = strchr(optln, ';');
		while (comment_char > fg_ret && *(comment_char - 1) == '\\') {
			char* read;
			char* to_write;
			for (to_write = comment_char - 1, read = comment_char; *read != '\0'; read++, to_write++) {
				*to_write = *read;
			}
			*to_write = '\0';
			comment_char = strchr(comment_char, ';');
		}
		if (comment_char != NULL) {
			*comment_char = '\0';
		}
		strip_spaces(optln);
		char* eq_chr = strchr(optln, '=');
		if (eq_chr != NULL) {
			*eq_chr = '\0';
			parse_option(optln, eq_chr + 1, delay, arr_len, shuffle_type, chord_arg, wave_shape, use_optimization/*, n_similar_items*/);
		}
		while (!have_newline) {
			fg_ret = fgets(optln, READ_FILE_BUFFER, opt);
			if (fg_ret == NULL) {
				break;
			}
			have_newline = strchr(optln, '\n') != NULL;
		}
		fg_ret = fgets(optln, READ_FILE_BUFFER, opt);
	}
	fclose(opt);
}

static int get_chord_id(const char *chord_name) {
	if (chord_name == NULL) {
		return -1;
	}
	size_t chord_name_l = strlen(chord_name);
	if (chord_name_l == 0) {
		return -1;
	}
	char arg_chord_name[chord_name_l + 6];
	strcpy(arg_chord_name, "chord_");
	strcat(arg_chord_name, chord_name);
	int dhash = dwhash(arg_chord_name);
	switch (dhash) {
	case chord_i_hash:
		return I_chord;
	case chord_ii_hash:
		return ii_chord;
	case chord_iii_hash:
		return iii_chord;
	case chord_iv_hash:
		return IV_chord;
	case chord_v_hash:
		return V_chord;
	case chord_vi_hash:
		return vi_chord;
	case chord_vii_hash:
		return vii_chord;
	case chord_min_i_hash:
		return i_min_chord;
	case chord_min_ii_hash:
		return ii_dim_chord;
	case chord_min_iii_hash:
		return III_maj_chord;
	case chord_min_iv_hash:
		return iv_min_chord;
	case chord_min_v_hash:
		return v_min_chord;
	case chord_min_vi_hash:
		return VI_maj_chord;
	case chord_min_vii_hash:
		return VII_maj_chord;
	case chord_aug_hash:
		return aug_chord_i;
	case chord_dim_hash:
		return dim_chord_i;
	case chord_dim7_hash:
		return dim7_chord;
	case chord_min7_flat_5_hash:
		return min7_flat_5_chord;
	case chord_min7_hash:
		return min7_chord;
	case chord_min_maj7_hash:
		return m_maj7_chord;
	case chord_dom7_hash:
		return dom7_chord;
	case chord_maj7_hash:
		return maj7_chord;
	case chord_aug7_hash:
		return aug7_chord;
	case chord_aug_maj7_hash:
		return aug_maj7_chord;
	case chord_dom9_hash:
		return dom9_chord;
	case chord_dom11_hash:
		return dom11_chord;
	case chord_dom13_hash:
		return dom13_chord;
	case chord_sus2_hash:
		return sus2_chord;
	case chord_sus4_hash:
		return sus4_chord;
	case chord_9sus4_hash:
		return jz_9sus4_chord;
	}
	return -1;
}

static void create_chord_progression_file() {
	FILE* opt = fopen(CHORD_PROGRESS_FILE, "wb");
	if (opt == NULL) {
		return;
	}
	fprintf(opt, "; This is a text file to specify the chord progression that will be used in the sorting visualizer.\n");
	fprintf(opt, "; Each line will contain one and only one chord name.\n");
	fprintf(opt, "; The chords will be read line by line from top to bottom.\n");
	fprintf(opt, "; Anything after semicolons in a line are comments and will not be processed.\n");
	fprintf(opt, "; Invalid chord names will be ignored.\n");
	fprintf(opt, "; The default chord progression is generated below (without semicolons at the front).\n");
	fprintf(opt, "; Note that you can add as many/few lines as you like (but not zero chords).\n\n");
	fprintf(opt, "i\niv\nvi\nv\ni\niv\nv\ni\n\n");
	fprintf(opt, "; Here are the possible chord names.\n");
	fprintf(opt, "; These are the only chords that this system can produce. Not all chord progressions are possible but you can try your best.\n");
	fprintf(opt, "; (NOTE: In the future when new chords are added, the list may need to be renamed to something else and you have to generate a new list to see the new values).\n");
	fprintf(opt, "\n; ========== Major scale chords ==========\n\n");
	fprintf(opt, ";     'i' - The first chord on a major scale. This is the I chord or the tonic chord.\n");
	fprintf(opt, ";     'ii' - The second chord on a major scale. This is the ii (minor) chord or the supertonic chord.\n");
	fprintf(opt, ";     'iii' - The third chord on a major scale. This is the iii (minor) chord or the mediant chord.\n");
	fprintf(opt, ";     'iv' - The fourth chord on a major scale. This is the IV (major) chord or the subdominant chord.\n");
	fprintf(opt, ";     'v' - The fifth chord on a major scale. This is the V (major) chord or the dominant chord.\n");
	fprintf(opt, ";     'vi' - The sixth chord on a major scale. This is the vi (minor) chord or the submediant chord.\n");
	fprintf(opt, ";     'vii' - The seventh chord on a major scale. This is the vii (diminished) chord or the leading tone / subtonic chord.\n");
	fprintf(opt, "\n; ========== Minor scale chords ==========\n\n");
	fprintf(opt, ";     'min_i' - The first chord on a minor scale. This is the i chord.\n");
	fprintf(opt, ";     'min_ii' - The second chord on a minor scale. This is the ii (diminished) chord.\n");
	fprintf(opt, ";     'min_iii' - The third chord on a minor scale. This is the III (major) chord.\n");
	fprintf(opt, ";     'min_iv' - The fourth chord on a minor scale. This is the iv (minor) chord.\n");
	fprintf(opt, ";     'min_v' - The fifth chord on a minor scale. This is the v (minor) chord.\n");
	fprintf(opt, ";     'min_vi' - The sixth chord on a minor scale. This is the VI (major) chord.\n");
	fprintf(opt, ";     'min_vii' - The seventh chord on a minor scale. This is the VII (major) chord.\n");
	fprintf(opt, "\n; ========== Augmented chords ==========\n\n");
	fprintf(opt, ";     'aug' - The first augmented fifth chord on a major scale.\n");
	fprintf(opt, "\n; ========== Diminished chords ==========\n\n");
	fprintf(opt, ";     'dim' - The first diminished chord.\n");
	fprintf(opt, "\n; ========== Seventh chords ==========\n\n");
	fprintf(opt, ";     'dim7' - The base diminished seventh chord.\n");
	fprintf(opt, ";     'min7_flat_5' - The half diminished seventh chord.\n");
	fprintf(opt, ";     'min7' - The minor seventh chord.\n");
	fprintf(opt, ";     'm_maj7' - The minor major seventh chord.\n");
	fprintf(opt, ";     'dom7' - The dominant seventh chord.\n");
	fprintf(opt, ";     'maj7' - The major seventh chord.\n");
	fprintf(opt, ";     'aug7' - The augmented seventh chord.\n");
	fprintf(opt, ";     'aug_maj7' - The augmented major seventh chord.\n");
	fprintf(opt, "\n; ========== Extended chords ==========\n\n");
	fprintf(opt, ";     'dom9' - The dominant ninth chord.\n");
	fprintf(opt, ";     'dom11' - The dominant eleventh chord.\n");
	fprintf(opt, ";     'dom13' - The dominant thriteenth chord.\n");
	fprintf(opt, "\n; ========== Suspended chords ==========\n\n");
	fprintf(opt, ";     'sus2' - The suspended second chord.\n");
	fprintf(opt, ";     'sus4' - The suspended fourth chord.\n");
	fprintf(opt, ";     '9sus4' - The jazz suspension chord.\n");
	fclose(opt);
}

static char using_heap = 0;

static int read_chord_progression_from_file() {
	FILE* opt = fopen(CHORD_PROGRESS_FILE, "r");
	if (opt == NULL) {
		create_chord_progression_file();
		return 0;
	}
	int prog_allocate = 8;
	int *new_progress = (int *) malloc(sizeof(int) * prog_allocate);
	int nchords = 0;
	if (new_progress == NULL) {
		goto read_chord_error;
	}
	char optln[READ_FILE_BUFFER];
	char* fg_ret = fgets(optln, READ_FILE_BUFFER, opt);
	while (fg_ret) {
		char have_newline = strchr(optln, '\n') != NULL;
		char* comment_char = strchr(optln, ';');
		while (comment_char > fg_ret && *(comment_char - 1) == '\\') {
			char* read;
			char* to_write;
			for (to_write = comment_char - 1, read = comment_char; *read != '\0'; read++, to_write++) {
				*to_write = *read;
			}
			*to_write = '\0';
			comment_char = strchr(comment_char, ';');
		}
		if (comment_char != NULL) {
			*comment_char = '\0';
		}
		strip_spaces(optln);

		if (nchords >= prog_allocate) {
			prog_allocate *= 2;
			new_progress = (int *) realloc(new_progress, sizeof(int) * prog_allocate);
			if (new_progress == NULL) {
				perror("Read chord file failed");
				nchords = 0;
				goto read_chord_error;
			}
		}

		int readid = get_chord_id(optln);
		if (readid >= 0) {
			new_progress[nchords] = get_chord_id(optln);
			nchords++;
		}

		while (!have_newline) {
			fg_ret = fgets(optln, READ_FILE_BUFFER, opt);
			if (fg_ret == NULL) {
				break;
			}
			have_newline = strchr(optln, '\n') != NULL;
		}
		fg_ret = fgets(optln, READ_FILE_BUFFER, opt);
	}
	fclose(opt);
	if (nchords > 0) {
		custom_chord_progression = new_progress;
		custom_chord_progression_alloc = prog_allocate;
		custom_chord_progression_len = nchords;
		using_heap = 1;
	} else {
		free(new_progress);
	}
	return 0;
read_chord_error:
	fclose(opt);
	error_pause(error_handle_mode);
	return 1;
}

static void custom_progression_cleanup() {
	if (using_heap) {
		free(custom_chord_progression);
	}
	custom_chord_progression = (int *) chord_progression;
	custom_chord_progression_len = chord_progression_len;
	custom_chord_progression_alloc = chord_progression_len;
}
