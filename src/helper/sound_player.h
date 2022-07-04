#ifndef SOUND_PLAYER_H_INCLUDED
#define SOUND_PLAYER_H_INCLUDED

int sound_player_init(double max_amp, const char* wave_arg);

int start_stream();

int stop_stream();

int sound_player_cleanup();

int sound_player_put_tone(int duration_ms, double freq1, double freq2);

#endif // SOUND_PLAYER_H_INCLUDED