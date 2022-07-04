#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <limits.h>

// ========== Helper functions breadcrumbs ==========

double saw_wave_fx(double relative_phase);
double sine_wave_fx(double relative_phase);
double square_wave_fx(double relative_phase);
double triangle_wave_fx(double relative_phase);

/**
 * The header of a wav file Based on:
 * https://ccrma.stanford.edu/courses/422/projects/WaveFormat/
 */
typedef struct wavfile_header_s {
	char    ChunkID[4];     /*  4   */
	int32_t ChunkSize;      /*  4   */
	char    Format[4];      /*  4   */
	
	char    Subchunk1ID[4]; /*  4   */
	int32_t Subchunk1Size;  /*  4   */
	int16_t AudioFormat;    /*  2   */
	int16_t NumChannels;    /*  2   */
	int32_t SampleRate;     /*  4   */
	int32_t ByteRate;       /*  4   */
	int16_t BlockAlign;     /*  2   */
	int16_t BitsPerSample;  /*  2   */
	
	char    Subchunk2ID[4];
	int32_t Subchunk2Size;
} wavfile_header_t;

int write_PCM16_stereo_header(FILE *file_p, int32_t sample_rate, int32_t total_audio_samples);

/**
 * Data structure to hold a single frame with two channels
 */
typedef struct PCM16_stereo_s {
	int16_t left;
	int16_t right;
} PCM16_stereo_t;

PCM16_stereo_t *PCM16_stereo_create(int32_t samples);
void PCM16_stereo_superposition_add(PCM16_stereo_t* __restrict__ add_to, const PCM16_stereo_t *value_to_add, double normalize_amplitude);
void PCM16_stereo_delete(PCM16_stereo_t *stereo_samples);

typedef struct tone_s {
	double frequency;
	double amplitude;
} tone_t;

static void normalize_doubles(double *arr, size_t nitems, const double new_max_amplitude);
int generate_superpositioned_wave(tone_t *tones, size_t ntones, double normalize_amplitude, int32_t duration_in_samples, int32_t sample_rate, double (*wave_fx)(double), char soft_start, PCM16_stereo_t* __restrict__ buffer);
// size_t write_PCM16wav_data (FILE* wav_file, int32_t start_at_ms, int32_t sample_rate, double normalize_amplitude, PCM16_stereo_t *stereo_data, int32_t duration_in_samples);
size_t write_PCM16wav_data (FILE* wav_file, int32_t start_sample_p, double normalize_amplitude, PCM16_stereo_t *stereo_data, int32_t duration_in_samples);
int clean_wav_clicks(FILE* wav_file, int32_t sample_rate, double lowest_freq, double highest_freq, double normalize_amplitude, double (*wave_fx)(double));

// ========== End helper functions breadcrumbs ==========

// ========== Standard values for CD-quality audio ==========

#define SUBCHUNK1SIZE   (16)
#define AUDIO_FORMAT    (1) /*For PCM*/
#define NUM_CHANNELS    (2) // Stereo
#define SAMPLE_RATE     (44100)

#define BITS_PER_SAMPLE (16)

#define BYTE_RATE       (SAMPLE_RATE * NUM_CHANNELS * BITS_PER_SAMPLE / 8)
#define BLOCK_ALIGN     (NUM_CHANNELS * BITS_PER_SAMPLE / 8)

// ========== End CD audio macros ==========

// ========== Helper functions breadcrumbs ==========

#define INI_DYN_LIST_SIZE 16
#define DEFAULT_TONE_AMPLITUDE 0.1

#define DECAY_CONSTANT 2

static char report_and_verify_header(FILE* sbeep_file);
typedef struct sbeep_data_time_bytes_4 {
	int32_t start_ms; // The first eight bytes (depends on the time_size definition in the header) is the time bytes. Four for the start time.
	int32_t end_ms;   // another four for the end. The unit is in milliseconds.
	// The frequencies for this time log will be in eight bytes sequences after that.
	// A sequence of 8 bytes of 0 will determine the end of this beep log. It is not useful if the frequency is 0 so...
} beep_time_t;
typedef struct freq_list_s {
	tone_t *items;
	size_t nitems;
	size_t allocated;
} freq_list_t;

freq_list_t *freq_list_t_create();
int freq_list_t_append(freq_list_t *flist, tone_t value);
void freq_list_t_delete(freq_list_t *flist);
// typedef struct conflict_list_s conflict_list_t;
// conflict_list_t *conflict_list_t_create();
// int conflict_list_t_append(conflict_list_t *clist, int value);
// void conflict_list_t_delete(conflict_list_t *clist);

static size_t int_num_digits(int n);
static size_t get_suggested_out_name_len(const char *in_name);
static void cpy_suggested_out_name(char* __restrict__ buffer, const char *in_name);
static size_t get_overflow_out_name_len(const char *original_name, int n_overflows);
static void cpy_overflow_out_name(char* __restrict__ buffer, const char *original_name, int n_overflows);

typedef double (*wavefx_t)(double);
static void process_arguments(char* __restrict__ nolog, wavefx_t* __restrict__ wavefx, char* __restrict__ soft_start, double* __restrict__ amplitude, int* __restrict__ delay_ms, int* __restrict__ argid, int argc, char** argv);
static void print_usage_info(const char* arg0);

// ========== End helper functions breadcrumbs ==========

// ========== Wav timing handler breadcrumbs ==========

typedef struct {
	uint8_t freqcount;
	int start;
	int dur;
	double freq1;
	double freq2;
} ms_slot_t;

void ms_slot_reset(ms_slot_t* __restrict__ slot) {
	slot -> freqcount = 0;
	slot -> start = 0;
	slot -> dur = 0;
	slot -> freq1 = 0.0;
	slot -> freq2 = 0.0;
}

void ms_slot_init(ms_slot_t* __restrict__ slots, size_t nslots) {
	while (nslots--) {
		ms_slot_reset(slots);
		slots++;
	}
}

ms_slot_t* ms_slot_create(size_t nslots) {
	ms_slot_t* r = (ms_slot_t *) malloc(sizeof(ms_slot_t) * nslots);
	ms_slot_init(r, nslots);
	return r;
}

void ms_slot_copy_to(ms_slot_t* __restrict__ dest, const ms_slot_t *src) {
	dest -> freqcount = src -> freqcount;
	dest -> dur = src -> dur;
	dest -> start = src -> start;
	dest -> freq1 = src -> freq1;
	dest -> freq2 = src -> freq2;
}

void ms_slot_delete(ms_slot_t* slots) {
	free(slots);
}

/**
 *  Return 1 if end of file is reached or some errors that prevented the reading of the file.
 */
int read_next_ms(int nextms, ms_slot_t* __restrict__ slots_buffer, int first_slot_ms, ms_slot_t* on_hold, FILE* sbeep) {
	int ret = 0;
	int offset_i = on_hold -> start - first_slot_ms;
	if (on_hold -> freqcount > 0) {
		if (offset_i >= nextms) {
			return ret;
		}
		ms_slot_copy_to(slots_buffer + offset_i, on_hold);
		ms_slot_reset(on_hold);
	}
	int read;
	beep_time_t startendtime;
	do {
		read = fread(&startendtime, sizeof(beep_time_t), 1, sbeep);
		if (read != 1) {
			ret = 1;
			return ret;
		}
		offset_i = startendtime.start_ms - first_slot_ms;
		on_hold -> start = startendtime.start_ms;
		on_hold -> dur = startendtime.end_ms - startendtime.start_ms;
		double freq1;
		double freq2;
		read = fread(&freq1, sizeof(double), 1, sbeep);
		if (read != 1) {
			ret = 1;
			return ret;
		}
		if (freq1 == 0.0) {
			ms_slot_reset(on_hold);
			continue;
		}
		on_hold -> freqcount = 1;
		on_hold -> freq1 = freq1;
		read = fread(&freq2, sizeof(double), 1, sbeep);
		if (read != 1) {
			ret = 1;
			goto copy_if_within_range;
		}
		if (freq2 == 0.0) {
			goto copy_if_within_range;
		}
		on_hold -> freq2 = freq2;
	copy_if_within_range:
		if (offset_i < nextms) {
			ms_slot_copy_to(slots_buffer + offset_i, on_hold);
			ms_slot_reset(on_hold);
		}
	} while (on_hold -> freqcount > 0 && ret == 0);
	return ret;
}

// ========== End wav timing handler breadcrumbs ==========

// #define MAX_WAV_BYTE_LEN 2147483
#define MAX_WAV_BYTE_LEN 2147483647
#define samples_per_millisecond 44

#define samples_from_ms(ms) ((ms) * samples_per_millisecond + (ms) / 10)

#define SQUARE_WAVE 0
#define SINE_WAVE 1
#define SAW_WAVE 2
#define TIRANGLE_WAVE 3

int main(int argc, char** argv) {
	int ret = 0;
	if (argc <= 1) {
		print_usage_info(argv[0]);
		return 0;
	}

	int argid = 1;
	wavefx_t wavefx = square_wave_fx;
	double amplitude_to_use = DEFAULT_TONE_AMPLITUDE;
	int offset_ms = 0;
	char nolog = 0;
	char soft_start = 0;

	process_arguments(&nolog, &wavefx, &soft_start, &amplitude_to_use, &offset_ms, &argid, argc, argv);

	int out_file_name_allocated = get_suggested_out_name_len(argv[1]) + 1;
	if (argc >= argid + 1) {
		out_file_name_allocated = strlen(argv[argid]) + 1;
	}
	char *original_out_file_name = (char *) malloc(sizeof(char) * out_file_name_allocated);
	if (original_out_file_name == NULL) {
		perror("original_out_file_name malloc failed in main");
		ret = -1;
		goto error0;
	}
	char *out_file_name = (char *) malloc(sizeof(char) * out_file_name_allocated);
	if (out_file_name == NULL) {
		perror("out_file_name malloc failed in main");
		ret = -1;
		goto error7;
	}
	if (argc >= argid + 1) {
		strcpy(original_out_file_name, argv[argid]);
		strcpy(out_file_name, argv[argid]);
	} else {
		cpy_suggested_out_name(original_out_file_name, argv[1]);
		cpy_suggested_out_name(out_file_name, argv[1]);
	}

	FILE* sbeep_file = fopen(argv[1], "rb");
	if (NULL == sbeep_file) {
		perror("fopen failed in main");
		ret = -1;
		goto error6;
    }
	char is_valid_sbeep = report_and_verify_header(sbeep_file);
	if (!is_valid_sbeep) {
		ret = 1;
		goto error1;
	}
	FILE* out_wav = fopen(out_file_name, "rb");
	if (out_wav != NULL) {
		fclose(out_wav);
		printf("File '%s' already exists - Exiting\n", out_file_name);
		ret = 0;
		goto error1;
	}
	out_wav = fopen(out_file_name, "wb+");
	if (NULL == out_wav) {
		perror("fopen failed in main");
		ret = -1;
		goto error1;
    }

	ret = write_PCM16_stereo_header(out_wav, SAMPLE_RATE, 1000);
	if (ret < 0) {
		fprintf(stderr, "write_PCM16_stereo_header failed in main\n");
		ret = -1;
		goto error2;
	}

	uint32_t previous_sample_end_p = 0;
	int previous_end_ms = 0;
	int overflow_count = 0;
	int ms_left_offset = -offset_ms;
	char reached_end = 0;
	int start_ms, end_ms;
	PCM16_stereo_t *audio_segment = NULL;
	beep_time_t l_time;
	size_t bytes_read;
	freq_list_t *flist;

	double lowest_freq = 20000;
	double highest_freq = 0;

	while (!reached_end) {
		bytes_read = fread(&l_time, sizeof(beep_time_t), 1, sbeep_file);
		if (bytes_read != 1) {
			reached_end = 1;
			break;
		}
		start_ms = l_time.start_ms * 5;
		end_ms = l_time.end_ms * 5;
		if (!nolog) printf("Segment %dms - %dms: ", start_ms, end_ms);
		flist = freq_list_t_create();
		if (flist == NULL) {
			if (!nolog) printf("\n");
			ret = 1;
			goto error2;
		}
		double read_freq = 0.0;
		bytes_read = fread(&read_freq, sizeof(double), 1, sbeep_file);
		if (bytes_read != 1) {
			reached_end = 1;
			read_freq = 0.0;
			freq_list_t_delete(flist);
			break;
		}
		while (read_freq < -0.0001 || read_freq > 0.0001) {
			tone_t new_tone;
			new_tone.frequency = read_freq;
			new_tone.amplitude = amplitude_to_use;
			if (read_freq < lowest_freq) {
				lowest_freq = read_freq;
			}
			if (read_freq > highest_freq) {
				highest_freq = read_freq;
			}
			freq_list_t_append(flist, new_tone);
			if (!nolog) printf("%fHz ", read_freq);
			bytes_read = fread(&read_freq, sizeof(double), 1, sbeep_file);
			if (bytes_read != 1) {
				reached_end = 1;
				read_freq = 0.0;
			}
		}
		int duration_ms = end_ms - start_ms;
		if (duration_ms == 0) {
			duration_ms = 5;
		}
		int duration_samples = duration_ms * SAMPLE_RATE / 1000;
		// double hf = flist -> items[0].frequency;
		// for (size_t i = 1; i < flist -> nitems; i++) {
		// 	if (hf < flist -> items[i].frequency) {
		// 		hf = flist -> items[i].frequency;
		// 	}
		// }
		// double lperiod = 1.0 / hf;
		// duration_samples += (int) (lperiod * (double) SAMPLE_RATE);
		audio_segment = PCM16_stereo_create(duration_samples);
		if (NULL == audio_segment) {
			perror("PCM16_stereo_create failed in main");
			ret = -1;
			goto error4;
		}
		ret = generate_superpositioned_wave(flist -> items, flist -> nitems, amplitude_to_use, duration_samples, SAMPLE_RATE, wavefx, soft_start, audio_segment);
		if (ret < 0) {
			fprintf(stderr, "generate_superpositioned_wave failed in main\n");
			ret = -1;
			goto error5;
		}
		// uint32_t current_sample_end_p = end_ms > ms_left_offset ? (uint32_t)(end_ms - ms_left_offset) * (uint32_t) SAMPLE_RATE / 1000 : 0;
		uint32_t current_sample_end_p = end_ms > ms_left_offset ? samples_from_ms((uint32_t)(end_ms - ms_left_offset)) : 0;
		// if (previous_sample_end_p >= MAX_WAV_BYTE_LEN) {
		// printf("\nDEBUG current_sample_end_p = %u\n", current_sample_end_p);
		// printf("DEBUG (uint32_t)previous_sample_end_p = %u\n", previous_sample_end_p);
		// printf("DEBUG MAX_WAV_BYTE_LEN = %d\n", MAX_WAV_BYTE_LEN);
		// printf("DEBUG ms_left_offset = %u\n", ms_left_offset);
		// printf("DEBUG (current_sample_end_p) > (uint32_t) MAX_WAV_BYTE_LEN = %d\n", (current_sample_end_p) > (uint32_t) MAX_WAV_BYTE_LEN);
		if ((current_sample_end_p) > (uint32_t) MAX_WAV_BYTE_LEN) {
			fseek(out_wav, 0, SEEK_SET);
			ret = write_PCM16_stereo_header(out_wav, SAMPLE_RATE, previous_sample_end_p);
			if (ret < 0) {
				fprintf(stderr, "write_PCM16_stereo_header failed in main\n");
				ret = -1;
				goto error5;
			}
			printf("Written audio duration: %dms\n", (previous_sample_end_p * 1000) / SAMPLE_RATE);
			fclose(out_wav);
			overflow_count++;

			// printf("Attempting to clean clicking noises... ");
			// if (clean_wav_clicks(out_wav, SAMPLE_RATE, lowest_freq, highest_freq, amplitude_to_use, wavefx) != 0) {
			// 	perror("clean_wav_clicks failed in main");
			// 	ret = -1;
			// 	goto error5;
			// }
			// printf("Done!\n");

			out_file_name_allocated = get_overflow_out_name_len(original_out_file_name, overflow_count) + 1;
			out_file_name = (char *) realloc(out_file_name, out_file_name_allocated);
			if (out_file_name == NULL) {
				perror("out_file_name malloc failed in main");
				ret = -1;
				goto error5;
			}
			cpy_overflow_out_name(out_file_name, original_out_file_name, overflow_count);
			out_wav = fopen(out_file_name, "wb+");
			if (NULL == out_wav) {
				perror("fopen failed in main");
				ret = -1;
				goto error5;
			}
			ret = write_PCM16_stereo_header(out_wav, SAMPLE_RATE, duration_samples);
			if (ret < 0) {
				fprintf(stderr, "write_PCM16_stereo_header failed in main\n");
				ret = -1;
				goto error5;
			}
			// previous_sample_end_p = duration_samples;
			ms_left_offset = previous_end_ms - offset_ms;
		}
		// Re-calculate as ms_left_offset might have been updated
		// previous_sample_end_p = (uint32_t)(end_ms - ms_left_offset) * (uint32_t)SAMPLE_RATE / 1000;
		previous_sample_end_p = samples_from_ms((uint32_t)(end_ms - ms_left_offset));
		previous_end_ms = end_ms;
		if (!nolog) printf("\nActual segment on audio file: %dms - %dms\n\n", start_ms - ms_left_offset, end_ms - ms_left_offset);
		if (start_ms - ms_left_offset > 0) {
			size_t written = write_PCM16wav_data(out_wav, samples_from_ms(start_ms - ms_left_offset), amplitude_to_use, audio_segment, duration_samples);
			if (written < duration_samples) {
				perror("write_PCM16wav_data failed in main");
				ret = -1;
				goto error5;
			}
		}

		PCM16_stereo_delete(audio_segment);
		audio_segment = NULL;
		freq_list_t_delete(flist);
		flist = NULL;
		continue;
	error5:
		PCM16_stereo_delete(audio_segment);
		audio_segment = NULL;
	error4:
		freq_list_t_delete(flist);
		flist = NULL;
		goto error2;
	}

	fseek(out_wav, 0, SEEK_SET);
	// ret = write_PCM16_stereo_header(out_wav, SAMPLE_RATE, l_time.end_ms * SAMPLE_RATE / 1000);
	ret = write_PCM16_stereo_header(out_wav, SAMPLE_RATE, previous_sample_end_p);
	if (ret < 0) {
		fprintf(stderr, "write_PCM16_stereo_header failed in main\n");
		ret = -1;
		goto error2;
	}
	printf("Written audio duration: %dms\n", (previous_sample_end_p * 1000) / SAMPLE_RATE);

	// TODO Remove clicking noises
	// fclose(out_wav);
	// out_wav = fopen(out_file_name, "rb+");
	// if (out_wav == NULL) {
	// 	perror("fopen failed in main");
	// 	ret = -1;
	// 	goto error1;
	// }
	// printf("Attempting to clean clicking noises... ");
	// if (clean_wav_clicks(out_wav, SAMPLE_RATE, lowest_freq, highest_freq, amplitude_to_use, wavefx) != 0) {
	// 	perror("clean_wav_clicks failed in main");
	// 	ret = -1;
	// 	goto error2;
	// }
	// printf("Done!\n");
error2:
	fclose(out_wav);
error1:
	fclose(sbeep_file);
error6:
	free(out_file_name);
error7:
	free(original_out_file_name);
error0:
	return ret;
}

// ========== Audio functions ==========

/**
 * Return 0 on success and -1 on failure
 */
int write_PCM16_stereo_header(FILE *file_p, int32_t sample_rate, int32_t total_audio_samples) {
	int ret;

	wavfile_header_t wav_header;
	int32_t subchunk2_size;
	int32_t chunk_size;

	size_t write_count;

	subchunk2_size  = total_audio_samples * NUM_CHANNELS * BITS_PER_SAMPLE / 8;
	chunk_size      = 4 + (8 + SUBCHUNK1SIZE) + (8 + subchunk2_size);

	wav_header.ChunkID[0] = 'R';
	wav_header.ChunkID[1] = 'I';
	wav_header.ChunkID[2] = 'F';
	wav_header.ChunkID[3] = 'F';
	
	wav_header.ChunkSize = chunk_size;
	
	wav_header.Format[0] = 'W';
	wav_header.Format[1] = 'A';
	wav_header.Format[2] = 'V';
	wav_header.Format[3] = 'E';
	
	wav_header.Subchunk1ID[0] = 'f';
	wav_header.Subchunk1ID[1] = 'm';
	wav_header.Subchunk1ID[2] = 't';
	wav_header.Subchunk1ID[3] = ' ';
	
	wav_header.Subchunk1Size = SUBCHUNK1SIZE;
	wav_header.AudioFormat = AUDIO_FORMAT;
	wav_header.NumChannels = NUM_CHANNELS;
	wav_header.SampleRate = sample_rate;
	wav_header.ByteRate = BYTE_RATE;
	wav_header.BlockAlign = BLOCK_ALIGN;
	wav_header.BitsPerSample = BITS_PER_SAMPLE;
	
	wav_header.Subchunk2ID[0] = 'd';
	wav_header.Subchunk2ID[1] = 'a';
	wav_header.Subchunk2ID[2] = 't';
	wav_header.Subchunk2ID[3] = 'a';
	wav_header.Subchunk2Size = subchunk2_size;
	
	write_count = fwrite(&wav_header, sizeof(wavfile_header_t), 1, file_p);
					
	ret = (1 != write_count) ? -1 : 0;
	
	return ret;
}

PCM16_stereo_t *PCM16_stereo_create(int32_t samples) {
	return (PCM16_stereo_t *) malloc(sizeof(PCM16_stereo_t) * samples);
}

PCM16_stereo_t *PCM16_stereo_resize(PCM16_stereo_t *stereo_samples, int32_t samples) {
	return (PCM16_stereo_t *) realloc(stereo_samples, sizeof(PCM16_stereo_t) * samples);
}

void PCM16_stereo_superposition_add(PCM16_stereo_t* __restrict__ add_to, const PCM16_stereo_t *value_to_add, double normalize_amplitude) {
	double d_left1 = (double) add_to -> left / (double) SHRT_MAX;
	double d_left2 = (double) value_to_add -> left / (double) SHRT_MAX;
	double d_right1 = (double) add_to -> right / (double) SHRT_MAX;
	double d_right2 = (double) value_to_add -> right / (double) SHRT_MAX;

	double d_newleft = d_left1 + d_left2;
	d_newleft *= normalize_amplitude / (fabs(d_left1) > fabs(d_left2) ? fabs(d_left1) : fabs(d_left2));
	double d_newright = d_right1 + d_right2;
	d_newright *= normalize_amplitude / (fabs(d_right1) > fabs(d_right2) ? fabs(d_right1) : fabs(d_right2));
	add_to -> left = d_newleft * (double) SHRT_MAX;
	add_to -> right = d_newright * (double) SHRT_MAX;
}

void PCM16_stereo_delete(PCM16_stereo_t *stereo_samples) {
	free(stereo_samples);
}

static void normalize_doubles(double *arr, size_t nitems, const double new_max_amplitude) {
	double maxval = *arr < 0 ? -1 * (*arr) : *arr;
	for (size_t i = 1; i < nitems; i++) {
		double abs_val = arr[i] < 0 ? -arr[i] : arr[i];
		if (abs_val > maxval) {
			maxval = abs_val;
		}
	}
	if (maxval < new_max_amplitude) {
		return;
	}
	for (size_t i = 0; i < nitems; i++) {
		arr[i] = arr[i] * new_max_amplitude / maxval;
	}
}

double square_wave_fx(double relative_phase) {
	return relative_phase < 0.5 ? 1.0 : -1.0;
}

double sine_wave_fx(double relative_phase) {
	return sin(relative_phase * (2 * 3.14159265));
}

double saw_wave_fx(double relative_phase) {
	return -1.0 + relative_phase * 2.0;
}

double triangle_wave_fx(double relative_phase) {
	if (relative_phase < 0.25) {
		return relative_phase * 4;
	}
	if (relative_phase < 0.75) {
		return 1.0 - (relative_phase - 0.25) * 4;
	}
	return (relative_phase - 0.75) * -4;
}

double max_decay_amplitude(double original_amplitude, double relative_time, double decay_constant) {
	return relative_time > 0.3 ? original_amplitude * exp(-decay_constant * (relative_time - 0.3) / 0.7) : original_amplitude * exp(decay_constant * (relative_time - 0.3) / 0.3);
}

// double max_decay_amplitude_sharp_start(double original_amplitude, double relative_time, double decay_constant) {
// 	return original_amplitude * exp(-decay_constant * relative_time);
// }

int generate_superpositioned_wave(tone_t *tones, size_t ntones, double normalize_amplitude, int32_t duration_in_samples, int32_t sample_rate, double (*wave_fx)(double), char soft_start, PCM16_stereo_t* __restrict__ buffer) {
	int ret = 0;
	double sample_period = 1.0 / (double)sample_rate;
	double duration_in_secs = duration_in_samples * sample_period;

	// Check for the violation of the Nyquist limit
	for (size_t i = 0; i < ntones; i++) {
		if (tones[i].frequency * 2 >= (double) sample_rate) {
			ret = -1;
			return -1;
			// goto error0;
		}
	}

	double periods[ntones];
	for (size_t i = 0; i < ntones; i++) {
		periods[i] = 1.0 / tones[i].frequency;
	}

	double *cumulative_phase_points = (double *) malloc(sizeof(double) * duration_in_samples);
	if (cumulative_phase_points == NULL) {
		printf("[generate_superpositioned_tone] not enough memory!");
		ret = -1;
		free(cumulative_phase_points);
		return -1;
		// goto error0;
	}
	for (size_t i = 0; i < duration_in_samples; i++) {
		cumulative_phase_points[i] = 0.0;
	}

	for (size_t i = 0; i < ntones; i++) {
		int phases_left = (int) (duration_in_secs / periods[i]);
		double phase = 0.0;
		for (size_t j = 0; j < duration_in_samples; j++) {
			if (phases_left > 0) {
				cumulative_phase_points[j] += wave_fx(phase / periods[i]) * tones[i].amplitude;// * (phases_left == 3 ? 0.5 : (phases_left == 2 ? 0.15 : (phases_left == 1 ? 0.05 : 1.0)));
			}
			phase += sample_period;
			if (phase > periods[i]) {
				phases_left--;
				phase -= periods[i];
			}
		}
	}
	normalize_doubles(cumulative_phase_points, duration_in_samples, normalize_amplitude);
	for (size_t i = 0; i < duration_in_samples; i++) {
		double decay_factor =  soft_start ? max_decay_amplitude(normalize_amplitude, (double) i / (double) duration_in_samples, DECAY_CONSTANT) : 1.0;//max_decay_amplitude_sharp_start(normalize_amplitude, (double) i / (double) duration_in_samples, DECAY_CONSTANT);
		buffer[i].left = (int16_t) (decay_factor * cumulative_phase_points[i] * (double) SHRT_MAX);
		buffer[i].right = (int16_t) (decay_factor * cumulative_phase_points[i] * (double) SHRT_MAX);
	}
	free(cumulative_phase_points);
// error0:
	return ret;
}

// size_t write_PCM16wav_data (FILE* wav_file, int32_t start_at_ms, int32_t sample_rate, double normalize_amplitude, PCM16_stereo_t *stereo_data, int32_t duration_in_samples) {
size_t write_PCM16wav_data (FILE* wav_file, int32_t start_sample_p, double normalize_amplitude, PCM16_stereo_t *stereo_data, int32_t duration_in_samples) {
	// int start_byte = (start_at_ms * sample_rate / 1000) * sizeof(PCM16_stereo_t) + sizeof(wavfile_header_t);
	int start_byte = start_sample_p * sizeof(PCM16_stereo_t) + sizeof(wavfile_header_t);
	fseek(wav_file, 0, SEEK_END);
	int file_last_byte = (int) ftell(wav_file);
	if (file_last_byte > start_byte) {
		int diff = file_last_byte - start_byte;
		diff /= sizeof(PCM16_stereo_t);
		PCM16_stereo_t *existing_bytes = PCM16_stereo_create(diff);
		if (fread(existing_bytes, sizeof(PCM16_stereo_t), diff, wav_file) == diff) {
			for (size_t i = 0; i < diff; i++) {
				PCM16_stereo_superposition_add(stereo_data + i, &(existing_bytes[i]), normalize_amplitude);
			}
		}
		PCM16_stereo_delete(existing_bytes);
		// char zero = '\0';
		// fseek(wav_file, 1, SEEK_CUR);
		// fwrite(&zero, 1, start_byte - file_last_byte, wav_file);
	}
	fseek(wav_file, start_byte, SEEK_SET);
	size_t written = fwrite(stereo_data, sizeof(PCM16_stereo_t), duration_in_samples, wav_file);
	if (written != duration_in_samples) {
		perror("[write_PCM16wav_data] unable to write.");
		return written;
	}
	size_t fflush_status = fflush(wav_file);
	if (fflush_status != 0) {
		perror("[write_PCM16wav_data] unable to fflush.");
		return fflush_status;
	}
	return written;
}

int write_wave(FILE* wav_file, char left, int32_t duration_in_samples, double normalize_amplitude, double (*wave_fx)(double)) {
	PCM16_stereo_t sample;
	size_t read = 0;
	size_t written = 0;
	for (int32_t i = 0; i < duration_in_samples; i++) {
		read = fread(&sample, sizeof(PCM16_stereo_t), 1, wav_file);
		// printf("DEBUG sample before write %d\n", sample.left);
		if (read != 1) {
			perror("[write_wave] unable to read existing sample.");
			return -1;
		}
		if (left) {
			sample.left = (int16_t) (normalize_amplitude * wave_fx((double) i / (double) duration_in_samples) * (double) SHRT_MAX);
		} else {
			sample.right = (int16_t) (normalize_amplitude * wave_fx((double) i / (double) duration_in_samples) * (double) SHRT_MAX);
		}
		fseek(wav_file, -sizeof(PCM16_stereo_t), SEEK_CUR);
		written = fwrite(&sample, sizeof(PCM16_stereo_t), 1, wav_file);
		if (written != 1) {
			perror("[write_wave] unable to write new sample.");
			return -1;
		}
		written = fflush(wav_file);
		if (written != 0) {
			perror("[write_wave] unable to fflush.");
			return -1;
		}
		fseek(wav_file, -sizeof(PCM16_stereo_t), SEEK_CUR);
		// fread(&sample, sizeof(PCM16_stereo_t), 1, wav_file);
		// printf("DEBUG sample after write %d\n", sample.left);
	}
	return 0;
}

int clean_wav_clicks(FILE* wav_file, int32_t sample_rate, double lowest_freq, double highest_freq, double normalize_amplitude, double (*wave_fx)(double)) {
	int ret = 0;
	double lowest_period = 1.0 / highest_freq;
	double highest_period = 1.0 / lowest_freq;
	int consecutive_threshold = (int) (lowest_period * sample_rate);
	int consecutive_high_threshold = (int) (highest_period * sample_rate);

	// fseek(wav_file, 0, SEEK_END);
	// int debug_file_len = ftell(wav_file);
	// printf("DEBUG debug_file_len %d\n", debug_file_len);

	fseek(wav_file, sizeof(wavfile_header_t), SEEK_SET);
	int consecutive_left_zeros = 0;
	int consecutive_right_zeros = 0;
	PCM16_stereo_t sample;
	// size_t debug_nsamples = 1;
	size_t read_samples = fread(&sample, sizeof(PCM16_stereo_t), 1, wav_file);
	while (read_samples == 1) {
		if (sample.left == 0) {
			consecutive_left_zeros++;
		} else {
			if (consecutive_left_zeros >= consecutive_threshold && consecutive_left_zeros < consecutive_high_threshold) {
				int duration_in_samples = consecutive_left_zeros;
				fseek(wav_file, -duration_in_samples * sizeof(PCM16_stereo_t), SEEK_CUR);
				printf("DEBUG Written wave at %ld\n", (ftell(wav_file) - 44) / sizeof(PCM16_stereo_t));
				if (write_wave(wav_file, 1, duration_in_samples, normalize_amplitude, wave_fx) != 0) {
					ret = -1;
					goto error_rmclik0;
				}
			}
			consecutive_left_zeros = 0;
		}
		if (sample.right == 0) {
			consecutive_right_zeros++;
		} else {
			if (consecutive_right_zeros >= consecutive_threshold && consecutive_right_zeros < consecutive_high_threshold) {
				int duration_in_samples = consecutive_right_zeros;
				fseek(wav_file, -duration_in_samples * sizeof(PCM16_stereo_t), SEEK_CUR);
				if (write_wave(wav_file, 0, duration_in_samples, normalize_amplitude, wave_fx) != 0) {
					ret = -1;
					goto error_rmclik0;
				}
			}
			consecutive_right_zeros = 0;
		}
		// printf("DEBUG consecutive left zeros = %d\n", consecutive_left_zeros);
		// printf("DEBUG consecutive_threshold = %d\n", consecutive_threshold);
		// printf("DEBUG consecutive_high_threshold = %d\n", consecutive_high_threshold);
		read_samples = fread(&sample, sizeof(PCM16_stereo_t), 1, wav_file);
		// if (read_samples != 1) {
		// 	printf("DEBUG fread gave %lu\n", (long unsigned)read_samples);
		// 	perror("DEBUG");
		// }
		// debug_nsamples++;
	}
	// printf("DEBUG nsamples = %lu\n", (long unsigned)debug_nsamples);
	// printf("DEBUG ftell = %lu\n", (long unsigned)ftell(wav_file));
error_rmclik0:
	return ret;
}

// ========== End audio functions ==========

// ========== Sbeep functions ==========

struct sbeep_header_bytes {
	int8_t fmt_define[16]; // The first 15 bytes should be tested to determine if the file is a valid sbeep version 1 file.
	// The bytes is an ASCII encoding of the text "angelin22071997". The 16th byte is not checked but should be a NUL byte.
	uint32_t time_size; // First 4 bytes describing how time is stored in bytes. Should be 4.
	uint32_t freq_size; // Next 4 bytes describing how frequency is stored in bytes. Should be 8.
	int32_t accuracy; // Next 4 bytes is currently an arbitary value about the accuracy in timing. Higher is better. Currently there's two levels: 3 and 5.
	// The data will be written right after without seperation. For sbeep file v1, the header is a fixed 28 bytes.
};

static char report_and_verify_header(FILE* sbeep_file) {
	struct sbeep_header_bytes file_header;
	size_t read_size = fread(&file_header, sizeof(file_header), 1, sbeep_file);
	if (read_size != 1) {
		return 0;
	}
	file_header.fmt_define[15] = '\0';
	printf("Checking header... ");
	if (strcmp((char *)file_header.fmt_define, "angelin22071997") != 0) {
		printf("invalid!\n");
		return 0;
	}
	printf("valid!\n");
	printf("Time size: %u\n", (unsigned int) file_header.time_size);
	if (file_header.time_size != 4) {
		return 0;
	}
	printf("Frequency size: %u\n", (unsigned int) file_header.freq_size);
	if (file_header.freq_size != 8) {
		return 0;
	}
	printf("Arbitiary accuracy: %d\n", (int) file_header.accuracy);
	return 1;
}

freq_list_t *freq_list_t_create() {
	freq_list_t *r = (freq_list_t *) malloc(sizeof(freq_list_t));
	if (r == NULL) {
		return NULL;
	}
	r -> items = (tone_t *) malloc(sizeof(tone_t) * INI_DYN_LIST_SIZE);
	if (r -> items == NULL) {
		free(r);
		return NULL;
	}
	r -> nitems = 0;
	r -> allocated = INI_DYN_LIST_SIZE;
	return r;
}

int freq_list_t_append(freq_list_t *flist, tone_t value) {
	if (flist -> allocated >= flist -> nitems) {
		flist -> allocated *= 2;
		flist -> items = (tone_t *) realloc(flist -> items, sizeof(tone_t) * flist -> allocated);
		if (flist -> items == NULL) {
			return 1;
		}
	}
	flist -> items[flist -> nitems].frequency = value.frequency;
	flist -> items[flist -> nitems].amplitude = value.amplitude;
	flist -> nitems++;
	return 0;
}

void freq_list_t_delete(freq_list_t *flist) {
	free(flist -> items);
	free(flist);
}

/*  ========== No longer needed code ==========

typedef struct conflict_list_s {
	int *items;
	size_t nitems;
	size_t allocated;
} conflict_list_t;

conflict_list_t *conflict_list_t_create() {
	conflict_list_t *r = (conflict_list_t *) malloc(sizeof(conflict_list_t));
	if (r == NULL) {
		return NULL;
	}
	r -> items = (int *) malloc(sizeof(int) * INI_DYN_LIST_SIZE);
	if (r -> items == NULL) {
		free(r);
		return NULL;
	}
	r -> nitems = 0;
	r -> allocated = INI_DYN_LIST_SIZE;
	return r;
}

int conflict_list_t_append(conflict_list_t *clist, int value) {
	if (clist -> allocated >= clist -> nitems) {
		clist -> allocated *= 2;
		clist -> items = (int *) realloc(clist -> items, sizeof(int) * clist -> allocated);
		if (clist -> items == NULL) {
			return 1;
		}
	}
	clist -> items[clist -> nitems] = value;
	clist -> nitems++;
	return 0;
}

void conflict_list_t_delete(conflict_list_t *clist) {
	free(clist -> items);
	free(clist);
}

*/ // ========== End no longer needed code ==========

// ========== End sbeep functions ==========

// ========== Helper functions ==========

static size_t int_num_digits(int n) {
	if (n == 0) {
		return 1;
	}
	size_t r = 0;
	while (n > 0) {
		r++;
		n /= 10;
	}
	return r;
}

static size_t get_suggested_out_name_len(const char *in_name) {
	size_t in_len = strlen(in_name);
	if (strcmp(in_name + in_len - 6, ".sbeep") == 0) {
		return in_len - 2;
	}
	return in_len + 4;
}

static void cpy_suggested_out_name(char* __restrict__ buffer, const char *in_name) {
	size_t in_len = strlen(in_name);
	if (strcmp(in_name + in_len - 6, ".sbeep") == 0) {
		// strncpy(buffer, in_name, in_len - 2);
		for (size_t i = 0; i < in_len - 2; i++) {
			buffer[i] = in_name[i];
		}
		buffer[in_len - 6] = '\0';
		strcat(buffer, ".wav");
		return;
	}
	strcpy(buffer, in_name);
	strcat(buffer, ".wav");
}

static size_t get_overflow_out_name_len(const char *original_name, int n_overflows) {
	return strlen(original_name) + int_num_digits(n_overflows) + 1;
}

static void cpy_overflow_out_name(char* __restrict__ buffer, const char *original_name, int n_overflows) {
	size_t ori_len = strlen(original_name);
	if (strcmp(original_name + ori_len - 4, ".wav") == 0) {
		strcpy(buffer, original_name);
		buffer[ori_len - 4] = '_';
		sprintf(buffer + ori_len - 3, "%d", n_overflows);
		strcat(buffer, ".wav");
		return;
	}
	strcpy(buffer, original_name);
	buffer[ori_len] = '_';
	sprintf(buffer + ori_len + 1, "%d", n_overflows);
}

/**
 * Returns 1 if the argument given is a nologs argument, 0 if not.
 */
static char process_nolog(char* __restrict__ nolog, const char* arg_str) {
	if (strcmp(arg_str, "lesstext") == 0) {
		*nolog = 1;
		return 1;
	}
	return 0;
}

/**
 * Returns 1 if the argument given is a wave type, 0 if not.
 */
static char process_wavefx(wavefx_t* __restrict__ wavefx, const char* arg_str) {
	if (strcmp(arg_str, "square") == 0) {
		*wavefx = square_wave_fx;
		return 1;
	}
	if (strcmp(arg_str, "sine") == 0) {
		*wavefx = sine_wave_fx;
		return 1;
	}
	if (strcmp(arg_str, "saw") == 0) {
		*wavefx = saw_wave_fx;
		return 1;
	}
	if (strcmp(arg_str, "triangle") == 0) {
		*wavefx = triangle_wave_fx;
		return 1;
	}
	return 0;
}

/**
 * Returns 1 if the argument given is a wave type, 0 if not.
 */
static char process_softstart(char* __restrict__ soft_start, const char* arg_str) {
	if (strcmp(arg_str, "hard") == 0) {
		*soft_start = 0;
		return 1;
	}
	if (strcmp(arg_str, "soft") == 0) {
		*soft_start = 1;
		return 1;
	}
	return 0;
}

/**
 * Returns 1 if the argument given is an amplitude value, 0 if not.
 */
static char process_amplitude(double* __restrict__ amplitude, const char* arg_str) {
	if (strchr(arg_str, '.') == NULL) {
		return 0;
	}
	int status = sscanf(arg_str, "%lf", amplitude);
	if (status != 1) {
		return 0;
	}
	if (*amplitude > 1) {
		*amplitude = 1;
	}
	if (*amplitude < 0) {
		*amplitude = 0;
	}
	return 1;
}

/**
 * Returns 1 if the argument given is a millisecond value, 0 if not.
 */
static char process_delay_ms(int* __restrict__ delay_ms, const char* arg_str) {
	int status = sscanf(arg_str, "%d", delay_ms);
	if (status != 1) {
		return 0;
	}
	return 1;
}

static void process_arguments(char* __restrict__ nolog, wavefx_t* __restrict__ wavefx, char* __restrict__ soft_start, double* __restrict__ amplitude, int* __restrict__ delay_ms, int* __restrict__ argid, int argc, char** argv) {
	assert(argid != NULL);
	assert(*argid == 1);

	*argid = 2;
	if (argc >= *argid + 1 && process_nolog(nolog, argv[*argid])) {
		(*argid)++;
	}
	if (argc >= *argid + 1 && process_wavefx(wavefx, argv[*argid])) {
		(*argid)++;
	}
	if (argc >= *argid + 1 && process_softstart(soft_start, argv[*argid])) {
		(*argid)++;
	}
	if (argc >= *argid + 1 && process_amplitude(amplitude, argv[*argid])) {
		(*argid)++;
	}
	if (argc >= *argid + 1 && process_delay_ms(delay_ms, argv[*argid])) {
		(*argid)++;
	}
}

static void print_usage_info(const char* arg0) {
	char sep = '/';
	#ifdef _WIN32
	sep = '\\';
	#endif
	const char* basename = strrchr(arg0, sep);
	if (basename == NULL) {
		basename = arg0;
	}
	printf("\n");
	printf("usage: %s sbeep_file [lesstext] [wave_shape] [hard_soft_start] [amplitude] [offset_ms] [out_wav]\n", basename);
	printf("       sbeep_file The path to the .sbeep file.\n");
	printf("       lesstext        [Optional] Entering 'lesstext' as second argument produces less text.\n");
	printf("                       You can also log the text via redirection (example: %s file.sbeep > logfile.log)\n", basename);
	printf("       wave_shape      [Optional] The shape of the sound wave. This affects sound timbre. Default is 'square'.\n");
	printf("                       The other possible values are:\n");
	printf("                         'square'   Gives some pixe-ly feel. This is default.\n");
	printf("                         'sine'     Standard synth organ-like timbre.\n");
	printf("                         'saw'      Rich but rough sounding.\n");
	printf("                         'triangle' Rough texture but weak.\n");
	printf("       hard_soft_start [Optional] How the note is played. Default is 'hard'.\n");
	printf("                       The other possible values are:\n");
	printf("                         'hard'     Starts the note with high pressure.\n");
	printf("                                    Recommended for square, triangle and saw waves. This is default.\n");
	printf("                         'soft'     Gradually ramps up the note from low pressure.\n");
	printf("                                    Recommended for sine and triangle waves.\n");
	printf("       amplitude       [Optional] The max amplitude of the sound between 0 and 1. Default 0.1.\n");
	printf("       offset_ms       [Optional] The overall shift in timing of the generated audio in milliseconds.\n");
	printf("                       Positive number is to the right, negative to the left. Default is 0ms.\n");
	printf("                       (This may be needed because the app can only generate 48-second .wav segments\n");
	printf("                       and between the segments there may be gaps in the audio.)\n");
	printf("       out_wav         [Optional] The output path to write the .wav file.\n");
	printf("                       It will replace the .sbeep extension with .wav if any (or simply add .wav if no .sbeep).\n");
	printf("                       NOTE: This program will overwrite the existing wav file with matching names!\n\n");
}

// ========== End helper functions ==========

// int main(void) {
// 	int ret = 0;
// 	tone_t notes[5];
// 	notes[0].amplitude = 0.65;
// 	notes[0].frequency = 220.0;
// 	notes[1].amplitude = 0.65;
// 	notes[1].frequency = 440.0;
// 	notes[2].amplitude = 0.5;
// 	notes[2].frequency = 660.0;
// 	notes[3].amplitude = 0.5;
// 	notes[3].frequency = 880.0;
// 	notes[4].amplitude = 0.3;
// 	notes[4].frequency = 1100.0;
// 	notes[5].amplitude = 0.3;
// 	notes[5].frequency = 1320.0;

// 	int32_t totalduration_ms = 3000;
// 	int32_t toneduration_ms = 2000;
// 	int32_t totalduration_samples = totalduration_ms * SAMPLE_RATE / 1000;
// 	int32_t toneduration_samples = toneduration_ms * SAMPLE_RATE / 1000;

// 	printf("total_duration_samples: %d\n", totalduration_samples);

// 	FILE* wav_file = fopen("./test_square.wav", "wb+");
// 	if(NULL == wav_file) {
// 		perror("fopen failed in main");
// 		ret = -1;
// 		goto error0;
//     }

// 	PCM16_stereo_t *data = PCM16_stereo_create(toneduration_samples);
// 	if(NULL == data) {
// 		perror("PCM16_stereo_create failed in main");
// 		ret = -1;
// 		goto error1;
//     }

// 	ret = generate_superpositioned_wave(notes, 3, 0.5, toneduration_samples, SAMPLE_RATE, sine_wave_fx, data);
// 	if (ret < 0) {
// 		fprintf(stderr, "generate_superpositioned_wave failed in main\n");
// 		ret = -1;
// 		goto error2;
// 	}

// 	ret = write_PCM16_stereo_header(wav_file, SAMPLE_RATE, totalduration_samples);
// 	if (ret < 0) {
// 		fprintf(stderr, "write_PCM16_stereo_header failed in main\n");
// 		ret = -1;
// 		goto error2;
// 	}

// 	size_t written = write_PCM16wav_data(wav_file, 1000, SAMPLE_RATE, data, toneduration_samples, NULL, NULL);
// 	if (written < toneduration_samples) {
// 		perror("write_PCM16wav_data failed in main");
// 		ret = -1;
// 		goto error2;
// 	}
// 	// Free and close everything
// error2:
// 	PCM16_stereo_delete(data);
// error1:
// 	fclose(wav_file);
// error0:
// 	return ret;   
// }