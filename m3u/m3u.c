#include <stdio.h>
#include <malloc.h>
#include <libzvbi.h>
#include <string.h>
#include "m3u.h"

Title *m3u_read(char *file_path, size_t *titles_size) {
    FILE *fp = fopen(file_path, "r");

    if (fp == NULL) {
        return NULL;
    }

    ssize_t read;
    char *line = NULL;
    size_t len = 0;

    size_t lines_ = 0;
    while ((getline(&line, &len, fp)) != -1) {
        if (line == '\0' || line[0] == '\n' || line[0] == '#') {
            continue;
        }

        lines_++;
    }
    rewind(fp);

    Title *titles = malloc(sizeof(Title) * lines_);

    int i = 0;
    while ((read = getline(&line, &len, fp)) != -1) {
        if (line == '\0' | line[0] == '\n' || line[0] == '#') {
            continue;
        }

        Title title;

        title.type = UNKNOWN;
        title.title_dest = malloc((size_t) read - 1);
        strncpy ( title.title_dest,  line,(size_t) read - 1);

        titles[i] = title;
        ++i;
    }

    *titles_size = lines_;

    free(line);

    return titles;
}


void m3u_free(Title *titles, size_t titles_size) {
    int i;

    for (i = 0; i < titles_size; ++i) {
        free(titles[i].title_dest);
    }

    free(titles);
}

