#pragma once

typedef int (*brake)();

void audio_init();

int audio_play_default(char *file_path, double seconds, brake brake_fn);

int audio_play(char *file_path, double seconds, double max_vol, brake brake_fn);

void mute(int mute);
