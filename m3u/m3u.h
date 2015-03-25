#pragma once

enum TitleType_ {
    UNKNOWN = 0x00000000,
    PATH = 0x00000001,
    RELATIVE_PATH = PATH | 0x10000000,
    ABSOLUTE_PATH = PATH | 0x01000000,
    URL = 0x00000002
};


typedef enum TitleType_ TitleType;

struct Title_ {
    char *title_dest;
    TitleType type;
};

typedef struct Title_ Title;


Title *m3u_read(char *file_path, size_t *titles_size);

void m3u_free(Title *titles, size_t titles_size);

