#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>

#include "precise_and_acc_time.h"

#include "beep_log.h"

#ifdef _WIN32
#include <windows.h>
// #define mkdir(dir, mode) _mkdir(dir)
// #define stat(pathname, statbuf) _stat(pathname, statbuf)
// #else
#endif // _WIN32
#include <sys/stat.h>
#include <sys/types.h>

static char* log_file_name = NULL;
static FILE* log_file_stream = NULL;
static mstime_t start_time = 0;
static mstime_t pause_start = 0;
static char is_paused = 0;

struct sbeep_header_bytes {
	int8_t fmt_define[16]; // The first 15 bytes should be tested to determine if the file is a valid sbeep version 1 file.
	// The bytes is an ASCII encoding of the text "angelin22071997". The 16th byte is not checked but should be a NUL byte.
	uint32_t time_size; // First 4 bytes describing how time is stored in bytes. Should be 4.
	uint32_t freq_size; // Next 4 bytes describing how frequency is stored in bytes. Should be 8.
	int32_t accuracy; // Next 4 bytes is currently an arbitary value about the accuracy in timing. Higher is better. Currently there's two levels: 3 and 5.
	// The data will be written right after without seperation. For sbeep file v1, the header is a fixed 28 bytes.
};

#define SB_TIME_SIZE 4
#define SB_FREQ_SIZE 8
#define SB_ACCURACY_LOW 3
#define SB_ACCURACY_OKAY 5

struct sbeep_data_time_bytes_4 {
	int32_t start_ms; // The first eight bytes (depends on the time_size definition in the header) is the time bytes. Four for the start time.
	int32_t end_ms;   // another four for the end. The unit is in milliseconds.
	// The frequencies for this time log will be in eight bytes sequences after that.
	// A sequence of 8 bytes of 0 will determine the end of this beep log. It is not useful if the frequency is 0 so...
};

#define SB_FREQ_LIST_END_BYTE 0

static size_t num_char_int(int n) {
	size_t d = 1;
	while (n >= 10){
		n /= 10;
		d++;
	}
	if (n < 1) {
		d++;
	}
	return d;
}

static void strip_invalid_chars(char* fname) {
	for (; *fname != '\0'; fname++) {
		if (strchr("<>:\"/\\|?* ", *fname) != NULL) {
			*fname = '_';
		}
	}
}

static char* create_log_filename(const char* prefix) {
	const size_t pflen = strlen(prefix);
	time_t now = time(NULL);
	struct tm* timeinfo = localtime(&now);
	size_t year_len = num_char_int(timeinfo -> tm_year + 1900);
	char* ret = malloc(sizeof(char) * (pflen + year_len + 27));
	strcpy(ret, prefix);
	ret[pflen] = '-';
	strftime(ret + pflen + 1, year_len + 22, "%Y-%m-%d_%H.%M.%S.sbeep", timeinfo);
	strip_invalid_chars(ret);
	return ret;
}

static void delete_log_filename(char* log_file) {
	free(log_file);
}

void start_beep_logging(const char* prefix) {
	if (log_file_name != NULL) {
		printf("[beep_log] Previous log has not been stopped. Cannot do >= two logs at once!\n");
		return;
	}
	char default_prefix[] = "sbeep";
	if (prefix == NULL) {
		prefix = default_prefix;
	}
	start_time = mstime();
	log_file_name = create_log_filename(prefix);
	struct stat fi;
	if (stat("sounds", &fi) == -1) {
		#ifdef _WIN32
		mkdir("sounds");
		#else
		mkdir("sounds", 0700);
		#endif
	}
	char fname[strlen("sounds") + strlen(log_file_name) + 1];
	strcpy(fname, "sounds/");
	strcat(fname, log_file_name);
	log_file_stream = fopen(fname, "wb");

	// Writing header data
	struct sbeep_header_bytes headers;
	headers.fmt_define[0] = 'a';
	headers.fmt_define[1] = 'n';
	headers.fmt_define[2] = 'g';
	headers.fmt_define[3] = 'e';
	headers.fmt_define[4] = 'l';
	headers.fmt_define[5] = 'i';
	headers.fmt_define[6] = 'n';
	headers.fmt_define[7] = '2';
	headers.fmt_define[8] = '2';
	headers.fmt_define[9] = '0';
	headers.fmt_define[10] = '7';
	headers.fmt_define[11] = '1';
	headers.fmt_define[12] = '9';
	headers.fmt_define[13] = '9';
	headers.fmt_define[14] = '7';
	headers.fmt_define[15] = '\0';

	headers.time_size = SB_TIME_SIZE;
	headers.freq_size = SB_FREQ_SIZE;
	headers.accuracy = is_accurate_time ? SB_ACCURACY_OKAY : SB_ACCURACY_LOW;

	fwrite(&headers, sizeof(struct sbeep_header_bytes), 1, log_file_stream);
}

mstime_t pause_beep_logging() {
	if (log_file_name == NULL) {
		printf("[beep_log] Log has not started yet.\n");
		return 0;
	}
	if (is_paused) {
		printf("[beep_log] Log is already paused.\n");
		return 0;
	}
	pause_start = mstime();
	is_paused = 1;
	return pause_start;
}

mstime_t resume_beep_logging() {
	if (log_file_name == NULL) {
		printf("[beep_log] Log has not started yet.\n");
		return 0;
	}
	if (!is_paused) {
		printf("[beep_log] Log is not paused.\n");
		return 0;
	}
	mstime_t paused_time = mstime() - pause_start;
	start_time += paused_time;
	pause_start = 0;
	is_paused = 0;
	return paused_time;
}

mstime_t log_beep(int duration_ms, double* freq_arr, size_t nfreqs) {
	if (log_file_name == NULL) {
		printf("[beep_log] Log has not started yet.\n");
		return 0;
	}
	if (is_paused) {
		printf("[beep_log] Log is paused.\n");
		return 0;
	}
	mstime_t time_mark = mstime() - start_time;
	struct sbeep_data_time_bytes_4 time_data;
	time_data.start_ms = time_mark;
	time_data.end_ms = time_mark + (mstime_t) duration_ms;

	// Debug print (human-readable)
	// fprintf(log_file_stream, "%d %d ", time_data.start_ms, time_data.end_ms);
	// for (int i = 0; i < nfreqs; i++) {
	// 	fprintf(log_file_stream, "%f ", freq_arr[i]);
	// }
	// fprintf(log_file_stream, "\n");

	// Actual print (bytes)
	fwrite(&time_data, sizeof(struct sbeep_data_time_bytes_4), 1, log_file_stream);
	fwrite(freq_arr, SB_FREQ_SIZE, nfreqs, log_file_stream);
	double end_mark = SB_FREQ_LIST_END_BYTE;
	fwrite(&end_mark, SB_FREQ_SIZE, 1, log_file_stream);
	return time_mark;
}

mstime_t stop_beep_logging() {
	if (log_file_name == NULL) {
		printf("[beep_log] Log has not started yet.\n");
		return 0;
	}
	mstime_t r = mstime() - start_time;
	start_time = 0;
	fclose(log_file_stream);
	log_file_stream = NULL;
	delete_log_filename(log_file_name);
	log_file_name = NULL;
	return r;
}

#define TUNING_A_FREQ 440.0
#define TUNER_CENT_FACTOR 1.000578

/**
 * @brief This gives the frequency which corresponds to the number of cents (100 cents make up a semitone) from the tuning A (nearest A to the middle C on a piano).
 * 
 * @param cents_offset The cents (semitones x 100) offset from the tuning A (nearest A to the middle C on a piano).
 */
double get_freq_from_cents_offset(int cents_offset, double tuning_freq) {
	if (cents_offset == 0) return tuning_freq;
	if (cents_offset > 0) {
		return tuning_freq * pow(TUNER_CENT_FACTOR, cents_offset);
	}
	return tuning_freq / pow(TUNER_CENT_FACTOR, cents_offset * -1);
}

double get_freq_between_bounds(size_t i, size_t last_index, int lower_cent_bound, int upper_cent_bound, double tuning_freq) {
	if (lower_cent_bound > upper_cent_bound) {
		int tmp = upper_cent_bound;
		upper_cent_bound = lower_cent_bound;
		lower_cent_bound = tmp;
	}
	int len = upper_cent_bound - lower_cent_bound;
	int pos = len * i / last_index;
	return get_freq_from_cents_offset(lower_cent_bound + pos, tuning_freq);
}

double get_freq_between_bounds_semitones(size_t i, size_t last_index, int lower_semitone_bound, int upper_semitone_bound, double tuning_freq) {
	if (lower_semitone_bound > upper_semitone_bound) {
		int tmp = upper_semitone_bound;
		upper_semitone_bound = lower_semitone_bound;
		lower_semitone_bound = tmp;
	}
	int len = upper_semitone_bound - lower_semitone_bound;
	int pos = len * i / last_index;
	return get_freq_from_cents_offset((lower_semitone_bound + pos) * 100, tuning_freq);
}

typedef struct interval_s {
	int numerator;
	int denominator;
	char is_irrational;
	double irrational_ratio;
	int eq_temp_cents;
} interval_t; 

typedef struct {
	interval_t intervals[8]; // Include padding arpeggios if desired. Expand buffer size if necessary.
	size_t nintervals;
	size_t n_octaves;
	size_t actual_nintervals; // Actual number of intervals in this chord. Note that there should not be any padding arpeggios among the chord intervals.
	// All padding arpeggios should be after.
} chord_t;

#define UNISON {.numerator = 1, .denominator = 1, .is_irrational = 0, .irrational_ratio = 1.0, .eq_temp_cents = 0}
#define PERFECT_FIFTH {.numerator = 3, .denominator = 2, .is_irrational = 0, .irrational_ratio = 1.5, .eq_temp_cents = 700}
#define PERFECT_FOURTH {.numerator = 4, .denominator = 3, .is_irrational = 0, .irrational_ratio = 1.333333, .eq_temp_cents = 500}
#define MAJOR_THIRD {.numerator = 5, .denominator = 4, .is_irrational = 0, .irrational_ratio = 1.25, .eq_temp_cents = 400}
#define MINOR_THIRD {.numerator = 6, .denominator = 5, .is_irrational = 0, .irrational_ratio = 1.2, .eq_temp_cents = 300}
#define MAJOR_SIXTH {.numerator = 5, .denominator = 3, .is_irrational = 0, .irrational_ratio = 1.666667, .eq_temp_cents = 900}
#define MINOR_SIXTH {.numerator = 8, .denominator = 5, .is_irrational = 0, .irrational_ratio = 1.6, .eq_temp_cents = 800}
#define WHOLE_STEP {.numerator = 10, .denominator = 9, .is_irrational = 0, .irrational_ratio = 1.111111, .eq_temp_cents = 200}
#define MAJOR_SEVENTH {.numerator = 15, .denominator = 8, .is_irrational = 0, .irrational_ratio = 1.875, .eq_temp_cents = 1100}
#define MINOR_SEVENTH {.numerator = 16, .denominator = 9, .is_irrational = 0, .irrational_ratio = 1.777778, .eq_temp_cents = 1000}
#define SEMITONE {.numerator = 16, .denominator = 15, .is_irrational = 0, .irrational_ratio = 1.066667, .eq_temp_cents = 100}

#define AUGMENTED_FIFTH {.numerator = 25, .denominator = 16, .is_irrational = 0, .irrational_ratio = 1.5625, .eq_temp_cents = 800}
/** aka tritone (The tone interval should be sqrt(2)) */
#define DIMINISHED_FIFTH {.numerator = 10, .denominator = 7, .is_irrational = 1, .irrational_ratio = 1.414213, .eq_temp_cents = 600}

#define DIMINISHED_SIXTH {.numerator = 192, .denominator = 125, .is_irrational = 0, .irrational_ratio = 1.536, .eq_temp_cents = 700}

#define DIMINISHED_SEVENTH {.numerator = 128, .denominator = 75, .is_irrational = 1, .irrational_ratio = 1.681793, .eq_temp_cents = 900}

#define MAJOR_NINTH {.numerator = 9, .denominator = 4, .is_irrational = 0, .irrational_ratio = 2.25, .eq_temp_cents = 1400}
#define PERFECT_ELEVENTH {.numerator = 8, .denominator = 3, .is_irrational = 0, .irrational_ratio = 2.666667, .eq_temp_cents = 1700}
#define MAJOR_THIRTEENTH {.numerator = 10, .denominator = 3, .is_irrational = 0, .irrational_ratio = 3.333333, .eq_temp_cents = 2100}

static const chord_t chords[] = {
	{
		.intervals = {UNISON, MAJOR_THIRD, PERFECT_FIFTH}, // I Chord
		.nintervals = 3,
		.n_octaves = 1,
		.actual_nintervals = 3
	},
	{
		.intervals = {WHOLE_STEP, PERFECT_FOURTH, MAJOR_SIXTH}, // ii Chord
		.nintervals = 3,
		.n_octaves = 1,
		.actual_nintervals = 3
	},
	{
		.intervals = {MAJOR_THIRD, PERFECT_FIFTH, MAJOR_SEVENTH}, // iii Chord
		.nintervals = 3,
		.n_octaves = 1,
		.actual_nintervals = 3
	},
	{
		.intervals = {UNISON, PERFECT_FOURTH, MAJOR_SIXTH}, // IV Chord
		.nintervals = 3,
		.n_octaves = 1,
		.actual_nintervals = 3
	},
	{
		.intervals = {WHOLE_STEP, PERFECT_FIFTH, MAJOR_SEVENTH}, // V Chord
		.nintervals = 3,
		.n_octaves = 1,
		.actual_nintervals = 3
	},
	{
		.intervals = {UNISON, MAJOR_THIRD, MAJOR_SIXTH}, // vi Chord
		.nintervals = 3,
		.n_octaves = 1,
		.actual_nintervals = 3
	},
	{
		.intervals = {WHOLE_STEP, PERFECT_FOURTH, MAJOR_SEVENTH}, // vii (dim) Chord
		.nintervals = 3,
		.n_octaves = 1,
		.actual_nintervals = 3
	},
	{
		.intervals = {UNISON, MINOR_THIRD, PERFECT_FIFTH}, // i (minor) Chord
		.nintervals = 3,
		.n_octaves = 1,
		.actual_nintervals = 3
	},
	{
		.intervals = {WHOLE_STEP, PERFECT_FOURTH, MINOR_SIXTH}, // ii (dim) Chord
		.nintervals = 3,
		.n_octaves = 1,
		.actual_nintervals = 3
	},
	{
		.intervals = {MINOR_THIRD, PERFECT_FIFTH, MINOR_SEVENTH}, // III (major) Chord
		.nintervals = 3,
		.n_octaves = 1,
		.actual_nintervals = 3
	},
	{
		.intervals = {UNISON, PERFECT_FOURTH, MINOR_SIXTH}, // iv (minor) Chord
		.nintervals = 3,
		.n_octaves = 1,
		.actual_nintervals = 3
	},
	{
		.intervals = {WHOLE_STEP, PERFECT_FIFTH, MINOR_SEVENTH}, // v (minor) Chord
		.nintervals = 3,
		.n_octaves = 1,
		.actual_nintervals = 3
	},
	{
		.intervals = {UNISON, MINOR_THIRD, MINOR_SIXTH}, // VI (major) Chord
		.nintervals = 3,
		.n_octaves = 1,
		.actual_nintervals = 3
	},
	{
		.intervals = {WHOLE_STEP, PERFECT_FOURTH, MINOR_SEVENTH}, // VII (major) Chord
		.nintervals = 3,
		.n_octaves = 1,
		.actual_nintervals = 3
	},
	{
		.intervals = {UNISON, MAJOR_THIRD, AUGMENTED_FIFTH}, // aug Chord
		.nintervals = 3,
		.n_octaves = 1,
		.actual_nintervals = 3
	},
	{
		.intervals = {UNISON, MINOR_THIRD, DIMINISHED_FIFTH}, // dim Chord
		.nintervals = 3,
		.n_octaves = 1,
		.actual_nintervals = 3
	},
	{
		.intervals = {UNISON, MINOR_THIRD, DIMINISHED_FIFTH, DIMINISHED_SEVENTH}, // dim7 Chord
		.nintervals = 4,
		.n_octaves = 1,
		.actual_nintervals = 4
	},
	{
		.intervals = {UNISON, MINOR_THIRD, DIMINISHED_FIFTH, MINOR_SEVENTH}, // half dim7 Chord
		.nintervals = 4,
		.n_octaves = 1,
		.actual_nintervals = 4
	},
	{
		.intervals = {UNISON, MINOR_THIRD, PERFECT_FIFTH, MINOR_SEVENTH}, // min7 Chord
		.nintervals = 4,
		.n_octaves = 1,
		.actual_nintervals = 4
	},
	{
		.intervals = {UNISON, MINOR_THIRD, PERFECT_FIFTH, MAJOR_SEVENTH}, // min maj7 Chord
		.nintervals = 4,
		.n_octaves = 1,
		.actual_nintervals = 4
	},
	{
		.intervals = {UNISON, MAJOR_THIRD, PERFECT_FIFTH, MINOR_SEVENTH}, // dominant 7th Chord
		.nintervals = 4,
		.n_octaves = 1,
		.actual_nintervals = 4
	},
	{
		.intervals = {UNISON, MAJOR_THIRD, PERFECT_FIFTH, MAJOR_SEVENTH}, // maj7 Chord
		.nintervals = 4,
		.n_octaves = 1,
		.actual_nintervals = 4
	},
	{
		.intervals = {UNISON, MAJOR_THIRD, AUGMENTED_FIFTH, MINOR_SEVENTH}, // aug7 Chord
		.nintervals = 4,
		.n_octaves = 1,
		.actual_nintervals = 4
	},
	{
		.intervals = {UNISON, MAJOR_THIRD, AUGMENTED_FIFTH, MAJOR_SEVENTH}, // aug maj7 Chord
		.nintervals = 4,
		.n_octaves = 1,
		.actual_nintervals = 4
	},
	{
		.intervals = {UNISON, MAJOR_THIRD, PERFECT_FIFTH, MINOR_SEVENTH, MAJOR_NINTH}, // dom9 Chord
		.nintervals = 5,
		.n_octaves = 2,
		.actual_nintervals = 5
	},
	{
		.intervals = {UNISON, MAJOR_THIRD, PERFECT_FIFTH, MINOR_SEVENTH, MAJOR_NINTH, PERFECT_ELEVENTH}, // dom11 Chord
		.nintervals = 6,
		.n_octaves = 2,
		.actual_nintervals = 6
	},
	{
		.intervals = {UNISON, MAJOR_THIRD, PERFECT_FIFTH, MINOR_SEVENTH, MAJOR_NINTH, PERFECT_ELEVENTH, MAJOR_THIRTEENTH}, // dom13 Chord
		.nintervals = 7,
		.n_octaves = 2,
		.actual_nintervals = 7
	},
	{
		.intervals = {UNISON, WHOLE_STEP, PERFECT_FIFTH}, // sus2 Chord
		.nintervals = 3,
		.n_octaves = 1,
		.actual_nintervals = 3
	},
	{
		.intervals = {UNISON, PERFECT_FOURTH, PERFECT_FIFTH}, // sus4 Chord
		.nintervals = 3,
		.n_octaves = 1,
		.actual_nintervals = 3
	},
	{
		.intervals = {UNISON, PERFECT_FOURTH, PERFECT_FIFTH, MINOR_SEVENTH, MAJOR_NINTH}, // 9sus4 Chord
		.nintervals = 5,
		.n_octaves = 2,
		.actual_nintervals = 5
	},
	{
		.intervals = {UNISON, WHOLE_STEP, MAJOR_THIRD, PERFECT_FOURTH, PERFECT_FIFTH, MAJOR_SIXTH, MAJOR_SEVENTH}, // Major Scale
		.nintervals = 7,
		.n_octaves = 1,
		.actual_nintervals = 7
	},
	{
		.intervals = {UNISON, WHOLE_STEP, MINOR_THIRD, PERFECT_FOURTH, PERFECT_FIFTH, MINOR_SIXTH, MINOR_SEVENTH}, // Minor Scale
		.nintervals = 7,
		.n_octaves = 1,
		.actual_nintervals = 7
	},
	{
		.intervals = {UNISON, WHOLE_STEP, MINOR_THIRD, PERFECT_FOURTH, PERFECT_FIFTH, MAJOR_SIXTH, MAJOR_SEVENTH}, // Melodic minor Scale
		.nintervals = 7,
		.n_octaves = 1,
		.actual_nintervals = 7
	},
	{
		.intervals = {UNISON, WHOLE_STEP, MINOR_THIRD, PERFECT_FOURTH, PERFECT_FIFTH, MINOR_SIXTH, MAJOR_SEVENTH}, // Harmonic minor Scale
		.nintervals = 7,
		.n_octaves = 1,
		.actual_nintervals = 7
	}
};

// static const interval_t chord_intervals[][3] = {
// 	{UNISON, MAJOR_THIRD, PERFECT_FIFTH}, // I Chord
// 	{WHOLE_STEP, PERFECT_FOURTH, MAJOR_SIXTH}, // ii Chord
// 	{MAJOR_THIRD, PERFECT_FIFTH, MAJOR_SEVENTH}, // iii Chord
// 	{UNISON, PERFECT_FOURTH, MAJOR_SIXTH}, // IV Chord
// 	{WHOLE_STEP, PERFECT_FIFTH, MAJOR_SEVENTH}, // V Chord
// 	{UNISON, MAJOR_THIRD, MAJOR_SIXTH}, // vi Chord
// 	{WHOLE_STEP, PERFECT_FOURTH, MAJOR_SEVENTH} // vii (dim) Chord
// };

// static char is_interval_larger_than_octave(const interval_t *interval) {
// 	return (double) (interval -> numerator) / (double) (interval -> denominator) > 2.0;
// }

double get_freq_between_bounds_arpeggio(size_t i, size_t last_index, int base_semitones_from_a4, int n_octaves, int chord_no, double tuning_freq, char use_eq_temp) {
	const chord_t *chord = chords + chord_no;
	int octave_pos = n_octaves * i / last_index / chord -> n_octaves;
	int pos_within_octave = ((n_octaves * chord -> nintervals) * i / last_index / chord -> n_octaves) % chord -> nintervals;
	if (((n_octaves * chord -> nintervals * i) % (last_index * chord -> n_octaves)) >= (last_index * chord -> n_octaves) / 2) {
		pos_within_octave++;
		if (pos_within_octave >= chord -> nintervals) {
			int oct_inc = pos_within_octave / chord -> nintervals;
			octave_pos += oct_inc;
			pos_within_octave -= oct_inc * chord -> nintervals;
		}
	}
	double r = pow(2, (double) octave_pos);
	const interval_t *interval = chord -> intervals;
	if (use_eq_temp) {
		r *= pow(2, (double) interval[pos_within_octave].eq_temp_cents / 1200.0);
	} else if (interval[pos_within_octave].is_irrational) {
		r *= interval[pos_within_octave].irrational_ratio;
	} else {
		r *= (double) interval[pos_within_octave].numerator;
		r /= (double) interval[pos_within_octave].denominator;
	}
	return tuning_freq * pow(2, (double) base_semitones_from_a4 / 12.0) * r;
}
