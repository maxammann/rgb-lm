#pragma once

typedef int (*brake)();

static const double VOLUME_FINISHED = 1.0;

int play(char *file_path, double max_vol, brake brake_fn);
