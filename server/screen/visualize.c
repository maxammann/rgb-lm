#include "visualize.h"
#include <math.h>
#include "screen.h"
#include <libavcodec/avfft.h>
#include <string.h>
#include "stdlib.h"

static int16_t left_bands[32];
static int16_t right_bands[32];

static RDFTContext *ctx;

static int N, samples;

void visualize_init(int samples_) {
    samples = samples_;
    N = samples_ / 2; // left/right channels
    ctx = av_rdft_init((int) log2(N), DFT_R2C);
}

void detrend(float array[], int n) {
    double x, y, a, b;
    double sy = 0.0,
            sxy = 0.0,
            sxx = 0.0;
    int i;

    for (i = 0, x = (-n / 2.0 + 0.5); i < n; i++, x += 1.0) {
        y = array[i];
        sy += y;
        sxy += x * y;
        sxx += x * x;
    }
    b = sxy / sxx;
    a = sy / n;

    for (i = 0, x = (-n / 2.0 + 0.5); i < n; i++, x += 1.0)
        array[i] -= (a + b * x);
}

void buffer_visualize(int16_t *data) {
    int i, tight_index;


    float left_data[sizeof(float) * N * 3];
    float right_data[sizeof(float) * N * 3];

    for (i = 0, tight_index = 0; i < samples; i += 2) {
        int16_t left = data[i];
        int16_t right = data[i + 1];

        double window_modifier = (0.5 * (1 - cos(2 * M_PI * tight_index / N)));
        float value = (float) (window_modifier * ((left) / 32768.0f));

        if (value > 1.0) {
            value = 1;
        } else if (value < -1.0) {
            value = -1;
        }

        left_data[tight_index] = value;
        value = (float) (window_modifier * ((right) / 32768.0f));

        if (value > 1.0) {
            value = 1;
        } else if (value < -1.0) {
            value = -1;
        }

        right_data[tight_index] = value;

        tight_index++;
    }


    av_rdft_calc(ctx, left_data);
    av_rdft_calc(ctx, right_data);

    int size = N / 2 * 2; // half is usable, but we have re and im


    for (i = 0, tight_index = 0; i < size; i += size / 32) {

        float im = left_data[i];
        float re = left_data[i + 1];
        double mag = sqrt(im * im + re * re);

        left_bands[tight_index] = (int16_t) (mag * 16.0f);

        tight_index++;
    }

    for (i = 0, tight_index = 0; i < size; i += size / 32) {

        float im = right_data[i];
        float re = right_data[i + 1];
        double mag = sqrt(im * im + re * re);

        right_bands[tight_index] = (int16_t) (mag * 16.0f);

        tight_index++;
    }
};

void screen_draw(lmLedMatrix *matrix, double elapsed) {
    lm_matrix_clear(matrix);

    int16_t x, y;

    rgb c = {200, 160, 20};

    for (x = 0; x < 32; ++x) {
        for (y = 0; y < left_bands[x]; ++y) {
            lm_matrix_set_pixel(matrix, x, 32 - y, &c);
        }

        for (y = 0; y < right_bands[x]; ++y) {
            lm_matrix_set_pixel(matrix, x, 32 - (y + 16), &c);
        }
    }
    lm_matrix_swap_buffers(matrix);
}

void register_visualize_screen() {
    register_screen("visualize", (screen_t) &screen_draw);
}
