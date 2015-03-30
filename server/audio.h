#pragma once

typedef int (*brake)();

int play_default(char *file_path, double seconds, brake brake_fn);

int play(char *file_path, double seconds, double max_vol, brake brake_fn);
