#include "visualize.h"
#include <math.h>
#include "screen.h"
#include <libavcodec/avfft.h>
#include <string.h>
#include "stdlib.h"

static int16_t power[32];

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

    for (i = 0, tight_index = 0; i < samples; i += 2) {
        int16_t left = data[i];
        int16_t right = data[i + 1];

        double window_modifier = (0.5 * (1 - cos(2 * M_PI * tight_index / N)));
//        double window_modifier = 1;
        float value = (float) (window_modifier * ((left) / 32768.0f));

        if (value > 1.0) {
            value = 1;
        } else if (value < -1.0) {
            value = -1;
        }

        left_data[tight_index] = value;
        tight_index++;
    }


    av_rdft_calc(ctx, left_data);
    detrend(left_data, N);

    int size = N / 2 * 2; // half is usable, but we have re and im




//    for (i = 0, tight_index = 0; i < size; i += size / 32) {
//        double sum = 0;
//        int i2;
//        for (i2 = (i == 0 ? 1 : 0); i2 < 32; i2 += 2) {
//            float im = left_data[i + i2];
//            float re = left_data[i + i2 + 1];
//            sum += sqrt(im * im + re * re);
//        }
//
//        power[tight_index] = (int16_t) (sum / 32.0);
//        tight_index++;
//    }

    int bands = 32;
    int band_index = 0;

    double avg = 0;
    int avg_count = N / 2 / bands;

//    double max_mag = -INFINITY;
//    int max_index = 0;

    for (i = 0; i < size; i += 2) {
//        if (i == 0) {
//            continue;
//        }

        float im = left_data[i];
        float re = left_data[i + 1];
        double mag = sqrt(im * im + re * re);

        avg += mag;

        if (avg_count == 0) {
            avg_count = N / 2 / bands;

            avg /= bands;

            power[band_index] = (int16_t) round(avg * 16.0f);

            band_index++;
        }

        --avg_count;


//        if (max_mag < mag) {
//            max_mag = mag;
//            max_index = i / 2;
//        }
    }

//    memset(power, 0, 32 * sizeof(int16_t));
//
//    power[max_index / 16] = 32;


    for (i = 0, tight_index = 0; i < size; i += size / 32) {

        float im = left_data[i];
        float re = left_data[i + 1];
        double mag = sqrt(im * im + re * re);

        power[tight_index] = (int16_t) (mag * 16.0f);

        tight_index++;
    }
};

void screen_draw(lmLedMatrix *matrix, double elapsed) {
    lm_matrix_clear(matrix);

    int16_t x, y;

    rgb c = {200, 160, 20};

    for (x = 0; x < 32; ++x) {
        for (y = 0; y < power[x]; ++y) {
            lm_matrix_set_pixel(matrix, x, 32 - y, &c);
        }
    }
    lm_matrix_swap_buffers(matrix);
}

void register_visualize_screen() {
    register_screen("visualize", (screen_t) &screen_draw);
}
