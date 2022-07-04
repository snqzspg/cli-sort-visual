#ifndef BEEP_LOG_H_INCLUDED
#define BEEP_LOG_H_INCLUDED

#include "precise_and_acc_time.h"

void start_beep_logging(const char* prefix);
mstime_t pause_beep_logging();
mstime_t resume_beep_logging();
mstime_t log_beep(int duration_ms, double* freq_arr, size_t nfreqs);
mstime_t stop_beep_logging();

/**
 * @brief This gives the frequency which corresponds to the number of cents (100 cents make up a semitone) from the tuning A (nearest A to the middle C on a piano).
 * 
 * @param cents_offset The cents (semitones x 100) offset from the tuning A (nearest A to the middle C on a piano).
 */
double get_freq_from_cents_offset(int cents_offset, double tuning_freq);
double get_freq_between_bounds(size_t i, size_t last_index, int lower_cent_bound, int upper_cent_bound, double tuning_freq);
double get_freq_between_bounds_semitones(size_t i, size_t last_index, int lower_semitone_bound, int upper_semitone_bound, double tuning_freq);

/**
 * Make sure to use these macros instead of the actual id as the ids for each chord may change over time.
 */

// ========== Major scale chords ==========

/** A major scale. Not a chord, but stored as one. */
#define major_scale 30

/** Aka tonic chord */
#define I_chord 0
/** Aka supertonic chord */
#define ii_chord 1
/** Aka mediant chord */
#define iii_chord 2
/** Aka subdominant chord */
#define IV_chord 3
/** Aka dominant chord */
#define V_chord 4
/** Aka submediant chord */
#define vi_chord 5
/** Aka leading tone / subtonic chord */
#define vii_chord 6

// ========== Minor scale chords ==========

/** A minor scale. Not a chord, but stored as one. */
#define neutral_minor_scale 31
/** A minor scale. Not a chord, but stored as one. */
#define melodic_minor_scale 32
/** A minor scale. Not a chord, but stored as one. */
#define harmonic_minor_scale 33

/** The first chord but in minor */
#define i_min_chord 7
/** The diminished second chord */
#define ii_dim_chord 8
/** The major third chord */
#define III_maj_chord 9
/** The minor fourth chord */
#define iv_min_chord 10
/** The minor fifth chord */
#define v_min_chord 11
/** The major sixth chord */
#define VI_maj_chord 12
/** The major seventh chord */
#define VII_maj_chord 13

// ========== Augmented and diminished chords ==========

/** The augmented-fifth chord */
#define aug_chord_i 14
/** The diminished chord */
#define dim_chord_i 15

// ========== Seventh chords ==========

/** The diminished seventh chord */
#define dim7_chord 16
/** The half diminished seventh chord */
#define min7_flat_5_chord 17
/** The minor seventh chord */
#define min7_chord 18
/** The minor major seventh chord */
#define m_maj7_chord 19
/** The dominant seventh chord */
#define dom7_chord 20
/** The major seventh chord */
#define maj7_chord 21
/** The augmented seventh chord */
#define aug7_chord 22
/** The augmented major seventh chord */
#define aug_maj7_chord 23

// ========== Extended chords ==========

/** The dominant ninth chord */
#define dom9_chord 24
/** The dominant eleventh chord */
#define dom11_chord 25
/** The dominant thriteenth chord */
#define dom13_chord 26

// ========== Suspended chords ==========

/** The suspended second chord */
#define sus2_chord 27
/** The suspended fourth chord */
#define sus4_chord 28
/** The jazz suspension chord */
#define jz_9sus4_chord 29

double get_freq_between_bounds_arpeggio(size_t i, size_t last_index, int base_semitones_from_a4, int n_octaves, int chord_no, double tuning_freq, char use_eq_temp);

#endif // BEEP_LOG_H_INCLUDED