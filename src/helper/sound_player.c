#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "portaudio.h"
#include "sound_player.h"

#ifndef ADDITIVE_FREQ_FILE
#define ADDITIVE_FREQ_FILE "sound_frequencies.txt"
#endif

#ifndef SETTINGS_FILE
#define SETTINGS_FILE "settings.txt"
#endif

#define SAMPLE_RATE (44100)

// ========== Sound player breadcrumbs ==========

typedef struct {
	float serving_fperiod_1;
	int waves_left_1;
	float request_fperiod_1;
	int request_wave_count_1;
	char is_sending_1;
	char is_receiving_1;
	char request_received_1;
	float phase_1;
	float serving_fperiod_2;
	int waves_left_2;
	float request_fperiod_2;
	int request_wave_count_2;
	char is_sending_2;
	char is_receiving_2;
	char request_received_2;
	float phase_2;
	char last_reqed_2;
	float sample_period;
	float max_amplitude;
	double (*wave_fx)(double);
} audreq_handle_t;

static void audreq_handle_init(audreq_handle_t* __restrict__ handler, int sample_rate, float max_amplitude, double (*wave_fx)(double));
static void audreq_handle_request_freq_1(audreq_handle_t* __restrict__ handler, double frequency, int duration_ms, int sample_rate);
static void audreq_handle_request_freq_2(audreq_handle_t* __restrict__ handler, double frequency, int duration_ms, int sample_rate);
static void audreq_handle_request_freq(audreq_handle_t* __restrict__ handler, double frequency, int duration_ms, int sample_rate);
static int audreq_handle_tick(const void *input_buffer, void *output_buffer, unsigned long frames_per_buffer, const PaStreamCallbackTimeInfo *time_info, PaStreamCallbackFlags status_flags, void *user_data);

// ========== End sound player breadcrumbs ==========

// ========== Wave functions breadcrumbs ==========

double square_wave_fx(double relative_phase);
double sine_wave_fx(double relative_phase);
double saw_wave_fx(double relative_phase);

// ========== End wave functions breadcrumbs ==========

// ========== Additive wave functions breadcrumbs ==========

#define ADD_SINE 0
#define ADD_COSINE 1
#define ADD_TRIANGLE 2
#define ADD_SQUARE 3
#define ADD_SAW 4

typedef struct {
	double rel_freq;
	double amp;
	int type;
} harmonic_t;

static harmonic_t standard_additive_sine[5] = {
	{.rel_freq = 1.0, .amp = 0.2, .type = ADD_SINE},
	{.rel_freq = 2.0, .amp = 0.2, .type = ADD_SINE},
	{.rel_freq = 3.0, .amp = 0.2, .type = ADD_SINE},
	{.rel_freq = 4.0, .amp = 0.2, .type = ADD_SINE},
	{.rel_freq = 5.0, .amp = 0.2, .type = ADD_SINE}
};
static harmonic_t *additive_sines = standard_additive_sine;
static size_t additive_sines_count = 5;
static size_t additive_sines_allocated = 5;

static harmonic_t violin[8] = {
	{.rel_freq = 1.0, .amp = 0.291, .type = ADD_SINE},
	{.rel_freq = 2.0, .amp = 0.274854, .type = ADD_COSINE},
	{.rel_freq = 3.0, .amp = 0.124269, .type = ADD_SINE},
	{.rel_freq = 4.0, .amp = 0.140351, .type = ADD_COSINE},
	{.rel_freq = 6.0, .amp = 0.106725, .type = ADD_COSINE},
	{.rel_freq = 7.0, .amp = 0.011696, .type = ADD_SINE},
	{.rel_freq = 8.0, .amp = 0.024854, .type = ADD_COSINE},
	{.rel_freq = 10.0, .amp = 0.026316, .type = ADD_COSINE}
};
static size_t nviolin_harmonics = 8;

static harmonic_t smu_choir_voice_attempt_1[55] = {
	{.rel_freq = 0.649519, .amp = 0.003056, .type = ADD_SINE},
	{.rel_freq = 0.767308, .amp = 0.004584, .type = ADD_SINE},
	{.rel_freq = 0.988999, .amp = 0.013710, .type = ADD_SINE},
	{.rel_freq = 0.995915, .amp = 0.013452, .type = ADD_SINE},
	{.rel_freq = 1.002831, .amp = 0.031798, .type = ADD_SINE},
	{.rel_freq = 1.009747, .amp = 0.014274, .type = ADD_SINE},
	{.rel_freq = 1.016663, .amp = 0.030486, .type = ADD_SINE},
	{.rel_freq = 1.023579, .amp = 0.013271, .type = ADD_SINE},
	{.rel_freq = 1.030495, .amp = 0.033351, .type = ADD_SINE},
	{.rel_freq = 1.037411, .amp = 0.016819, .type = ADD_SINE},
	{.rel_freq = 1.044328, .amp = 0.007251, .type = ADD_SINE},
	{.rel_freq = 1.051244, .amp = 0.009671, .type = ADD_SINE},
	{.rel_freq = 1.480040, .amp = 0.007814, .type = ADD_SINE},
	{.rel_freq = 1.486956, .amp = 0.014778, .type = ADD_SINE},
	{.rel_freq = 1.493872, .amp = 0.025100, .type = ADD_SINE},
	{.rel_freq = 1.500789, .amp = 0.007324, .type = ADD_SINE},
	{.rel_freq = 1.507705, .amp = 0.036106, .type = ADD_SINE},
	{.rel_freq = 1.514621, .amp = 0.020659, .type = ADD_SINE},
	{.rel_freq = 1.521537, .amp = 0.046488, .type = ADD_SINE},
	{.rel_freq = 1.528453, .amp = 0.053091, .type = ADD_SINE},
	{.rel_freq = 1.535369, .amp = 0.051171, .type = ADD_SINE},
	{.rel_freq = 1.542285, .amp = 0.069092, .type = ADD_SINE},
	{.rel_freq = 1.549201, .amp = 0.026946, .type = ADD_SINE},
	{.rel_freq = 1.556117, .amp = 0.007201, .type = ADD_SINE},
	{.rel_freq = 1.563033, .amp = 0.011751, .type = ADD_SINE},
	{.rel_freq = 1.569949, .amp = 0.008829, .type = ADD_SINE},
	{.rel_freq = 1.950333, .amp = 0.007087, .type = ADD_SINE},
	{.rel_freq = 1.957250, .amp = 0.007817, .type = ADD_SINE},
	{.rel_freq = 1.964166, .amp = 0.013938, .type = ADD_SINE},
	{.rel_freq = 1.971082, .amp = 0.012078, .type = ADD_SINE},
	{.rel_freq = 1.977998, .amp = 0.010420, .type = ADD_SINE},
	{.rel_freq = 1.984914, .amp = 0.025996, .type = ADD_SINE},
	{.rel_freq = 1.991830, .amp = 0.027465, .type = ADD_SINE},
	{.rel_freq = 1.998746, .amp = 0.021018, .type = ADD_SINE},
	{.rel_freq = 2.005662, .amp = 0.018810, .type = ADD_SINE},
	{.rel_freq = 2.012578, .amp = 0.008683, .type = ADD_SINE},
	{.rel_freq = 2.019494, .amp = 0.024113, .type = ADD_SINE},
	{.rel_freq = 2.026410, .amp = 0.031529, .type = ADD_SINE},
	{.rel_freq = 2.033326, .amp = 0.029438, .type = ADD_SINE},
	{.rel_freq = 2.040242, .amp = 0.012434, .type = ADD_SINE},
	{.rel_freq = 2.047159, .amp = 0.014449, .type = ADD_SINE},
	{.rel_freq = 2.054075, .amp = 0.032149, .type = ADD_SINE},
	{.rel_freq = 2.067907, .amp = 0.012434, .type = ADD_SINE},
	{.rel_freq = 2.074823, .amp = 0.010978, .type = ADD_SINE},
	{.rel_freq = 2.081739, .amp = 0.015143, .type = ADD_SINE},
	{.rel_freq = 2.088655, .amp = 0.016774, .type = ADD_SINE},
	{.rel_freq = 3.015409, .amp = 0.008014, .type = ADD_SINE},
	{.rel_freq = 3.063822, .amp = 0.007132, .type = ADD_SINE},
	{.rel_freq = 4.025156, .amp = 0.009734, .type = ADD_SINE},
	{.rel_freq = 4.032072, .amp = 0.007943, .type = ADD_SINE},
	{.rel_freq = 4.038988, .amp = 0.012105, .type = ADD_SINE},
	{.rel_freq = 4.045905, .amp = 0.014873, .type = ADD_SINE},
	{.rel_freq = 4.066653, .amp = 0.007045, .type = ADD_SINE},
	{.rel_freq = 4.971154, .amp = 0.001129, .type = ADD_SINE},
	{.rel_freq = 4.9375, .amp = 0.001196, .type = ADD_SINE}
};
static size_t n_smu_choir_voice_attempt_1_harmonics = 55;

double additive_sine_wave_fx(double relative_phase, const harmonic_t *harmonics, size_t nharmonics);
double custom_additive_sine_wave_fx(double relative_phase);
static int process_harmonic_type(const char *s);

/**
 *  Return 0 if successful, 1 if not.
 */
static int process_harmonic(harmonic_t* __restrict__ harmonic, const char* optln);
static void create_wave_file();
static void read_wave_from_file();
static void custom_add_sine_cleanup();

// ========== End additive wave functions breadcrumbs ==========

// ========== Wave functions ==========

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

double violin_wave_fx(double relative_phase);
double smu_choir_voice_attempt_1_wave_fx(double relative_phase);

typedef double (*wavefx_t)(double);

wavefx_t get_wavefx(const char* arg_str) {
	if (strcmp(arg_str, "square") == 0) {
		return square_wave_fx;
	}
	if (strcmp(arg_str, "sine") == 0) {
		return sine_wave_fx;
	}
	if (strcmp(arg_str, "saw") == 0) {
		return saw_wave_fx;
	}
	if (strcmp(arg_str, "triangle") == 0) {
		return triangle_wave_fx;
	}
	if (strcmp(arg_str, "violin") == 0) {
		return violin_wave_fx;
	}
	if (strcmp(arg_str, "electronicvoice1") == 0) {
		return smu_choir_voice_attempt_1_wave_fx;
	}
	if (strcmp(arg_str, "customised") == 0 || strcmp(arg_str, "customized") == 0) {
		return custom_additive_sine_wave_fx;
	}
	return square_wave_fx;
}

// ========== End wave functions ==========

// ========== Sound player functions ==========

static audreq_handle_t aud_handler;
static char inited = 0;
static char stream_started = 0;
PaStream *sstream = NULL;

int sound_player_init(double max_amp, const char* wave_arg) {
	PaError err;

	read_wave_from_file();

	audreq_handle_init(&aud_handler, SAMPLE_RATE, max_amp, get_wavefx(wave_arg)); // Tentative

	err = Pa_Initialize();
	if (err != paNoError) {
		goto error;
	}

	err = Pa_OpenDefaultStream(&sstream, 0, 2, paFloat32, SAMPLE_RATE, 256, audreq_handle_tick, &aud_handler);
	if (err != paNoError) {
		goto error;
	}
	inited = 1;
	return err;
error:
	Pa_Terminate();
	fprintf( stderr, "An error occurred while using the portaudio stream\n" );
	fprintf( stderr, "Error number: %d\n", err );
	fprintf( stderr, "Error message: %s\n", Pa_GetErrorText( err ) );
	return err;
}

int start_stream() {
	PaError err;
	if (!stream_started) {
		err = Pa_StartStream(sstream);
		if (err != paNoError) {
			fprintf( stderr, "An error occurred while using the portaudio stream\n" );
			fprintf( stderr, "Error number: %d\n", err );
			fprintf( stderr, "Error message: %s\n", Pa_GetErrorText( err ) );
			return err;
		}
		stream_started = 1;
		return err;
	}
	return paNoError;
}

int stop_stream() {
	if (stream_started) {
		PaError err;
		// while (aud_handler.waves_left_1 > 0 || aud_handler.waves_left_2 > 0);
		err = Pa_StopStream(sstream);
		if (err != paNoError) {
			fprintf( stderr, "An error occurred while using the portaudio stream\n" );
			fprintf( stderr, "Error number: %d\n", err );
			fprintf( stderr, "Error message: %s\n", Pa_GetErrorText( err ) );
			return err;
		}
		stream_started = 0;
		return err;
	}
	return paNoError;
}

int sound_player_cleanup() {
	PaError err = paNoError;

	custom_add_sine_cleanup();

	if (stream_started) {
		err = stop_stream();
	}

	if (inited) {
		err = Pa_CloseStream(sstream);
		sstream = NULL;
		if (err != paNoError) {
			fprintf( stderr, "An error occurred while using the portaudio stream\n" );
			fprintf( stderr, "Error number: %d\n", err );
			fprintf( stderr, "Error message: %s\n", Pa_GetErrorText( err ) );
		}
		Pa_Terminate();
		inited = 0;
		return err;
	}

	return err;
}

int sound_player_put_tone(int duration_ms, double freq1, double freq2) {
	if (duration_ms > 0) {
		if ((freq1 < 0.001) != (freq2 < 0.001)) {
			duration_ms *= 2;
		} else {
			duration_ms += 10;
			duration_ms *= 3;
			duration_ms /= 2;
		}
	}
	PaError err = paNoError;
	if (!inited) {
		return paNoError;
	}
	if (!stream_started && duration_ms != 0) {
		err = start_stream();
		if (err != paNoError) {
			return err;
		}
	} /*else if (aud_handler.waves_left_1 <= 0 && aud_handler.waves_left_2 <= 0) {
	// 	err = stop_stream();
	// 	if (err != paNoError) {
	// 		return err;
	// 	}
	// }*/
	if (duration_ms > 0) {
		if (freq1 > 0.001) {
			audreq_handle_request_freq(&aud_handler, freq1, duration_ms, SAMPLE_RATE);
		}
		if (freq2 > 0.001) {
			audreq_handle_request_freq(&aud_handler, freq2, duration_ms, SAMPLE_RATE);
		}
	}
	return err;
}

// ========== End sound player functions ==========

// ========== Audio handler functions ==========

static void audreq_handle_init(audreq_handle_t* __restrict__ handler, int sample_rate, float max_amplitude, double (*wave_fx)(double)) {
	handler -> serving_fperiod_1 = 0.0;
	handler -> waves_left_1 = 0;
	handler -> request_fperiod_1 = 0.0;
	handler -> request_wave_count_1 = 0;
	handler -> is_receiving_1 = 0;
	handler -> is_sending_1 = 0;
	handler -> request_received_1 = 0;
	handler -> phase_1 = 0.0;
	handler -> serving_fperiod_2 = 0.0;
	handler -> waves_left_2 = 0;
	handler -> request_fperiod_2 = 0.0;
	handler -> request_wave_count_2 = 0;
	handler -> is_receiving_2 = 0;
	handler -> is_sending_2 = 0;
	handler -> request_received_2 = 0;
	handler -> phase_2 = 0.0;
	handler -> last_reqed_2 = 1;
	handler -> sample_period = 1.0 / (float) sample_rate;
	if (max_amplitude > 1.0) {
		max_amplitude = 1.0;
	}
	if (max_amplitude < 0.0) {
		max_amplitude = 0.0;
	}
	handler -> max_amplitude = max_amplitude;
	handler -> wave_fx = wave_fx;
}

static void audreq_handle_request_freq_1(audreq_handle_t* __restrict__ handler, double frequency, int duration_ms, int sample_rate) {
	float period = (float) (1.0 / frequency);
	int wave_count = (int) ((float) duration_ms / period / 1000);
	handler -> request_fperiod_1 = period;
	handler -> request_wave_count_1 = wave_count;
	handler -> is_sending_1 = 1;
	clock_t start_waiting = clock();
	while (handler -> is_receiving_1 && ((double) (clock() - start_waiting) / CLOCKS_PER_SEC) < period * (double) wave_count); // {
	// 	printf("Waiting \r");
	// 	fflush(stdout);
	// }
	handler -> request_received_1 = 0;
	handler -> is_sending_1 = 0;
	handler -> last_reqed_2 = 0;
}

static void audreq_handle_request_freq_2(audreq_handle_t* __restrict__ handler, double frequency, int duration_ms, int sample_rate) {
	float period = (float) (1.0 / frequency);
	int wave_count = (int) ((float) duration_ms / period / 1000);
	handler -> request_fperiod_2 = period;
	handler -> request_wave_count_2 = wave_count;
	handler -> is_sending_2 = 1;
	clock_t start_waiting = clock();
	while (handler -> is_receiving_2 && ((double) (clock() - start_waiting) / CLOCKS_PER_SEC) < period * (double) wave_count); //{
	// 	printf("Waiting \r");
	// 	fflush(stdout);
	// }
	handler -> request_received_2 = 0;
	handler -> is_sending_2 = 0;
	handler -> last_reqed_2 = 1;
}

static void audreq_handle_request_freq(audreq_handle_t* __restrict__ handler, double frequency, int duration_ms, int sample_rate) {
	if (handler -> last_reqed_2) {
		audreq_handle_request_freq_1(handler, frequency, duration_ms, sample_rate);
	} else {
		audreq_handle_request_freq_2(handler, frequency, duration_ms, sample_rate);
	}
}

static int audreq_handle_tick(const void *input_buffer, void *output_buffer, unsigned long frames_per_buffer, const PaStreamCallbackTimeInfo *time_info, PaStreamCallbackFlags status_flags, void *user_data) {
	audreq_handle_t *handler = (audreq_handle_t *) user_data;
	float *out = (float *) output_buffer;
	unsigned int i;
	(void) input_buffer; // Prevent unused variable warning.
	float single_ampli = handler -> max_amplitude / 2.0f;

	for (i = 0; i < frames_per_buffer; i++) {
		float fout = 0.0f;
		if (handler -> waves_left_1 > 0) {
			fout += single_ampli * handler -> wave_fx((double) (handler -> phase_1) / (double) (handler -> serving_fperiod_1));
			handler -> phase_1 += handler -> sample_period;
			if (handler -> phase_1 > handler -> serving_fperiod_1) {
				handler -> phase_1 -= handler -> serving_fperiod_1;
				if (!handler -> is_sending_1) handler -> waves_left_1--;
				if (!handler -> request_received_1 && handler -> waves_left_1 > 1) {
					handler -> waves_left_1 = 1;
				}
			}
		}
		if (!handler -> request_received_1 && handler -> waves_left_1 <= 0) {
			handler -> serving_fperiod_1 = handler -> request_fperiod_1;
			handler -> waves_left_1 = handler -> request_wave_count_1;
			handler -> is_receiving_1 = 1;
			handler -> request_received_1 = 1;
			handler -> is_receiving_1 = 0;
		}

		if (handler -> waves_left_2 > 0) {
			fout += single_ampli * handler -> wave_fx((double) (handler -> phase_2) / (double) (handler -> serving_fperiod_2));
			handler -> phase_2 += handler -> sample_period;
			if (handler -> phase_2 > handler -> serving_fperiod_2) {
				handler -> phase_2 -= handler -> serving_fperiod_2;
				if (!handler -> is_sending_2) handler -> waves_left_2--;
				if (!handler -> request_received_2 && handler -> waves_left_2 > 1) {
					handler -> waves_left_2 = 1;
				}
			}
		}
		if (!handler -> request_received_2 && handler -> waves_left_2 <= 0) {
			handler -> serving_fperiod_2 = handler -> request_fperiod_2;
			handler -> waves_left_2 = handler -> request_wave_count_2;
			handler -> is_receiving_2 = 1;
			handler -> request_received_2 = 1;
			handler -> is_receiving_2 = 0;
		}

		*out++ = fout;
		*out++ = fout;
	}
	return 0;
}

// ========== End audio handler functions ==========

// ========== Additive wave functions ==========

#define using_std_addsines (additive_sines == standard_additive_sine)

#define strip_int(x) ((x) - (double) ((int) (x)))

double additive_sine_wave_fx(double relative_phase, const harmonic_t *harmonics, size_t nharmonics) {
	double r = 0.0;
	// assert that sum of all amplitudes = 1.0
	for (size_t i = 0; i < nharmonics; i++) {
		switch (harmonics[i].type) {
			case ADD_COSINE:
				r += cos(relative_phase * (2 * 3.14159265 * harmonics[i].rel_freq)) * harmonics[i].amp;
				break;
			case ADD_TRIANGLE:
				r += triangle_wave_fx(strip_int(relative_phase * harmonics[i].rel_freq)) * harmonics[i].amp;
				break;
			case ADD_SQUARE:
				r += square_wave_fx(strip_int(relative_phase * harmonics[i].rel_freq)) * harmonics[i].amp;
				break;
			case ADD_SAW:
				r += saw_wave_fx(strip_int(relative_phase * harmonics[i].rel_freq)) * harmonics[i].amp;
				break;
			case ADD_SINE:
			default:
				r += sin(relative_phase * (2 * 3.14159265 * harmonics[i].rel_freq)) * harmonics[i].amp;
				break;
		}
	}
	if (r > 1.0) {
		r = 1.0;
	}
	if (r < -1.0) {
		r = -1.0;
	}
	return r;
}

double custom_additive_sine_wave_fx(double relative_phase) {
	return additive_sine_wave_fx(relative_phase, additive_sines, additive_sines_count);
}

double violin_wave_fx(double relative_phase) {
	return additive_sine_wave_fx(relative_phase, violin, nviolin_harmonics);
	// double r = 0.0;
	// // assert that sum of all amplitudes = 1.0
	// for (size_t i = 0; i < nviolin_harmonics; i++) {
	// 	switch (violin[i].type) {
	// 		case ADD_COSINE:
	// 			r += cos(relative_phase * (2 * 3.14159265 * violin[i].rel_freq)) * violin[i].amp;
	// 			break;
	// 		case ADD_SINE:
	// 		default:
	// 			r += sin(relative_phase * (2 * 3.14159265 * violin[i].rel_freq)) * violin[i].amp;
	// 			break;
	// 	}
	// }
	// if (r > 1.0) {
	// 	r = 1.0;
	// }
	// if (r < -1.0) {
	// 	r = -1.0;
	// }
	// return r;
}

double smu_choir_voice_attempt_1_wave_fx(double relative_phase) {
	return additive_sine_wave_fx(relative_phase, smu_choir_voice_attempt_1, n_smu_choir_voice_attempt_1_harmonics);
}

#define READ_FILE_BUFFER 40

static int process_harmonic_type(const char *s) {
	if (strcmp(s, "sine") == 0) {
		return ADD_SINE;
	}
	if (strcmp(s, "cosine") == 0) {
		return ADD_COSINE;
	}
	if (strcmp(s, "triangle") == 0) {
		return ADD_TRIANGLE;
	}
	if (strcmp(s, "square") == 0) {
		return ADD_SQUARE;
	}
	if (strcmp(s, "saw") == 0) {
		return ADD_SAW;
	}
	return -1;
}

/**
 *  Return 0 if successful, 1 if not.
 */
static int process_harmonic(harmonic_t* __restrict__ harmonic, const char* optln) {
	size_t olen = strlen(optln);
	if (olen == 0) {
		return 1;
	}
	char lnclone[olen + 1];
	strcpy(lnclone, optln);
	char sep[2] = ",";
	char *token = strtok(lnclone, sep);
	if (token == NULL) {
		return 1;
	}
	harmonic -> rel_freq = atof(token);
	if (harmonic -> rel_freq == 0.0) {
		return 1;
	}
	token = strtok(NULL, sep);
	if (token == NULL) {
		return 1;
	}
	harmonic -> amp = atof(token);
	token = strtok(NULL, sep);
	if (token == NULL) {
		return 1;
	}
	harmonic -> type = process_harmonic_type(token);
	if (harmonic -> type < 0) {
		return 1;
	}
	return 0;
}

static void create_wave_file() {
	FILE* opt = fopen(ADDITIVE_FREQ_FILE, "wb");
	if (opt == NULL) {
		return;
	}
	fprintf(opt, "; This file allows you to (somewhat) customize the timbre of the sound produced using sine waves.\n");
	fprintf(opt, "; Musical instruments produces sounds that can be broken down into a sum of sine waves of different frequencies.\n");
	fprintf(opt, "; Each sound will have one base frequency and multiple harmonic frequencies.\n");
	fprintf(opt, "; The base frequency should have the lowest frequency and the highest amplitude.\n");
	fprintf(opt, "; If the harmonic frequencies harmonise with one another, the resulting sound is tonal, and you can tell which note it is playing (If you have perfect pitch maybe?)\n");
	fprintf(opt, "; If they don't harmonise with one another, the resulting sound is atonal, and you cannot tell which note it is playing.\n\n");
	fprintf(opt, "; In this file, each line is considered as a base frequency or a harmonic frequency.\n");
	fprintf(opt, "; For each line, there will be three values, seperated by commas:\n");
	fprintf(opt, ";  1. The first value is the relative frequency to the base frequency. If the base frequency is 100Hz, a value of 2.0 corresponds to 200Hz.\n");
	fprintf(opt, ";     There can only be one base frequency, and it's value is set to 1.0.\n");
	fprintf(opt, ";  2. The second value is the relative amplitude to the base frequency amplitude. It is to be kept between 0.0 and 1.0.\n");
	fprintf(opt, ";  3. The last value is the function for that harmonic. The values are 'sine', 'cosine', 'triangle', 'square' and 'saw'.\n");
	fprintf(opt, ";     Note that you will most likely to use 'sine' and 'cosine'. The system struggles to produce multiple triangle waves together.\n");
	fprintf(opt, "; You can specify comments by placing a semicolon at the beginning of the comment. (Like this one!)\n\n");
	fprintf(opt, "; Below is an example of a harmonic sound made by five different sine harmonics. The system will default to this combination should it fail to read this file.\n\n");
	fprintf(opt, "1.0, 0.2, sine\n");
	fprintf(opt, "2.0, 0.2, sine\n");
	fprintf(opt, "3.0, 0.2, sine\n");
	fprintf(opt, "4.0, 0.2, sine\n");
	fprintf(opt, "5.0, 0.2, sine\n\n");
	fprintf(opt, "; WARNING: Before you go ahead and run one of the sorts with your customization, you might want to consider lowering the upper bound of the tone or lowering your volume.\n");
	fprintf(opt, ";          This is because loud and high frequencies can be uncomfortable and can accelerate the damage to your hearing.\n");
	fprintf(opt, "; DISPLAIMER: Snqzs' PG will NOT be responsible for any hearing damage caused by the usage of our software. We have provided ample warning.\n\n");
	fprintf(opt, "; Below are some frequencies found on the internet that produces different sounds. Unfortunately these information is not easily found.\n");
	fprintf(opt, "; The frequencies are not perfect imitations, so they sound kinda \"chiptune-like\", which is kinda cool tbh.\n");
	fprintf(opt, "; You can uncomment them by removing the semicolons befor them.\n");
	fprintf(opt, "; Remember to comment out the example above as well! If not it will get mixed in.\n\n");
	fprintf(opt, "; The Violin-like tone used in the default settings (from https://meettechniek.info/additional/additive-synthesis.html)\n");
	fprintf(opt, "; 1.0, 0.995, sine\n");
	fprintf(opt, "; 2.0, 0.94, cosine\n");
	fprintf(opt, "; 3.0, 0.425, sine\n");
	fprintf(opt, "; 4.0, 0.480, cosine\n");
	fprintf(opt, "; 6.0, 0.365, cosine\n");
	fprintf(opt, "; 7.0, 0.040, sine\n");
	fprintf(opt, "; 8.0, 0.085, cosine\n");
	fprintf(opt, "; 10.0, 0.090, cosine\n\n");
	fprintf(opt, "; Organ tone\n");
	fprintf(opt, "; WARNING: Lower your volume before trying this one!\n");
	fprintf(opt, ";          You may also want to lower the lower and upper cent bound / chord start semitone setting\n");
	fprintf(opt, ";          in the '%s' file for a more comfortable experience.\n", SETTINGS_FILE);
	fprintf(opt, ";          Suggested: tone_lower_cent_bound = -3600\n");
	fprintf(opt, ";                     tone_upper_cent_bound = 0\n");
	fprintf(opt, ";                     chord_start_semitone = -36\n");
	fprintf(opt, ";                     chord_octave_range = 2\n");
	fprintf(opt, "; 1.0, 1.0, sine\n");
	fprintf(opt, "; 2.0, 0.9, sine\n");
	fprintf(opt, "; 4.0, 1.0, sine\n");
	fprintf(opt, "; 6.0, 0.6, sine\n");
	fprintf(opt, "; 16.0, 0.7, sine\n");
	fprintf(opt, "; 20.0, 0.5, sine\n");
	fprintf(opt, "; 24.0, 0.4, sine\n\n");
	fprintf(opt, "; (Supposedly) Marimbia-like tone\n");
	fprintf(opt, "; A lack of attack and release algorithm makes it difficult to tell.\n");
	fprintf(opt, "; WARNING: Lower your volume before trying this one!\n");
	fprintf(opt, ";          You may also want to lower the lower and upper cent bound / chord start semitone setting\n");
	fprintf(opt, ";          in the '%s' file for a more comfortable experience.\n", SETTINGS_FILE);
	fprintf(opt, ";          Suggested: tone_lower_cent_bound = -3600\n");
	fprintf(opt, ";                     tone_upper_cent_bound = 0\n");
	fprintf(opt, ";                     chord_start_semitone = -36\n");
	fprintf(opt, ";                     chord_octave_range = 2\n");
	fprintf(opt, "; 1.0, 1.0, sine\n");
	fprintf(opt, "; 10.0, 0.9, sine\n");
	fprintf(opt, "; 20.0, 0.4, sine\n");
	fprintf(opt, "; 30.0, 0.2, sine\n\n");
	fprintf(opt, "; Makeshift square wave\n");
	fprintf(opt, "; 1.0, 1.0, sine\n");
	fprintf(opt, "; 3.0, 0.333333, sine\n");
	fprintf(opt, "; 5.0, 0.2, sine\n");
	fprintf(opt, "; 7.0, 0.142857, sine\n");
	fprintf(opt, "; 9.0, 0.111111, sine\n");
	fprintf(opt, "; 11.0, 0.090909, sine\n");
	fprintf(opt, "; 13.0, 0.076923, sine\n");
	fprintf(opt, "; 15.0, 0.066667, sine\n\n");
	fprintf(opt, "; Is this supposed to be piano ??\n");
	fprintf(opt, "; 1.0, 1.0, sine\n");
	fprintf(opt, "; 2.0, 0.15625, sine\n");
	fprintf(opt, "; 3.0, 0.16667, sine\n");
	fprintf(opt, "; 4.0, 0.15625, sine\n");
	fprintf(opt, "; 5.0, 0.09375, sine\n");
	fprintf(opt, "; 6.0, 0.083333, sine\n");
	fprintf(opt, "; 7.0, 0.104167, sine\n");
	fprintf(opt, "; 8.0, 0.083333, sine\n");
	fprintf(opt, "; 9.0, 0.072917, sine\n\n");
	fprintf(opt, "; This is a trumpet-like sound\n");
	fprintf(opt, "; WARNING: Lower your volume before trying this one!\n");
	fprintf(opt, ";          You may also want to lower the lower and upper cent bound / chord start semitone setting\n");
	fprintf(opt, ";          in the '%s' file for a more comfortable experience.\n", SETTINGS_FILE);
	fprintf(opt, ";          Suggested: tone_lower_cent_bound = -3600\n");
	fprintf(opt, ";                     tone_upper_cent_bound = 0\n");
	fprintf(opt, ";                     chord_start_semitone = -36\n");
	fprintf(opt, ";                     chord_octave_range = 2\n");
	fprintf(opt, "; 1.0, 1.0, sine\n");
	fprintf(opt, "; 4.0, 0.3, sine\n");
	fprintf(opt, "; 6.0, 0.5, sine\n");
	fprintf(opt, "; 8.0, 0.5, sine\n");
	fprintf(opt, "; 10.0, 0.7, sine\n");
	fprintf(opt, "; 12.0, 0.7, sine\n");
	fprintf(opt, "; 14.0, 1.0, sine\n");
	fprintf(opt, "; 16.0, 0.95, sine\n");
	fprintf(opt, "; 18.0, 0.55, sine\n");
	fprintf(opt, "; 20.0, 0.3, sine\n");
	fclose(opt);
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

static void normalize_harmonics(harmonic_t* __restrict__ harmonics, size_t nharmonics) {
	double sum_amp = 0.0;
	for (size_t i = 0; i < nharmonics; i++) {
		sum_amp += harmonics[i].amp;
	}
	for (size_t i = 0; i < nharmonics; i++) {
		harmonics[i].amp /= sum_amp;
	}
}

static void read_wave_from_file() {
	FILE* opt = fopen(ADDITIVE_FREQ_FILE, "r");
	if (opt == NULL) {
		create_wave_file();
		return;
	}

	size_t har_alloc = 8;
	harmonic_t *custom_harmonics = (harmonic_t *) malloc(sizeof(harmonic_t) * har_alloc);
	size_t har_len = 0;

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

		if (har_len >= har_alloc) {
			har_alloc *= 2;
			custom_harmonics = (harmonic_t *) realloc(custom_harmonics, sizeof(harmonic_t) * har_alloc);
		}
		
		if (process_harmonic(custom_harmonics + har_len, optln) == 0) {
			har_len++;
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
	if (har_len > 0) {
		normalize_harmonics(custom_harmonics, har_len);
		additive_sines = custom_harmonics;
		additive_sines_allocated = har_alloc;
		additive_sines_count = har_len;
	} else {
		free(custom_harmonics);
	}
	fclose(opt);
}

static void custom_add_sine_cleanup() {
	if (!using_std_addsines) {
		free(additive_sines);
		additive_sines = standard_additive_sine;
		additive_sines_allocated = 5;
		additive_sines_count = 5;
	}
}

// ========== End additive wave functions ==========
