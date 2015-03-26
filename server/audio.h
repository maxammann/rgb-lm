#pragma once

typedef int (*brake)();

int play(char *file_path, double max_vol, brake brake_fn);
