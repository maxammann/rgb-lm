#include "visualize.h"
#include <math.h>
#include "screen.h"
#include <libavcodec/avfft.h>


#define HEIGHT 32
#define WIDTH 32

static rgb color = {0, 204, 204};

static int16_t left_bands[WIDTH]; // Left channel frequency bands
static int16_t right_bands[WIDTH]; // Right channel frequency bands

static RDFTContext *ctx;

static int N, samples; // N and number of samples to process each step

void visualize_init(int samples_) {
    samples = samples_;
    N = samples_ / 2; // left/right channels
    ctx = av_rdft_init((int) log2(N), DFT_R2C);
}

void buffer_visualize(int16_t *data) {
    int i, tight_index; // just some iterator indices


    float left_data[N * 2];
    float right_data[N * 2];

    for (i = 0, tight_index = 0; i < samples; i += 2) {
        int16_t left = data[i];
        int16_t right = data[i + 1];

        double window_modifier = (0.5 * (1 - cos(2 * M_PI * tight_index / (N - 1)))); // Hann (Hanning) window function
        float value = (float) (window_modifier * ((left) / 32768.0f)); // Convert to float and apply

        // cap values above 1 and below -1
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


    for (i = 0, tight_index = 0; i < size; i += size / WIDTH) {

        float im = left_data[i];
        float re = left_data[i + 1];
        double mag = sqrt(im * im +
                          re * re); // The magnitude in logarithmic scale, linear would be sqrt(im * im + re * re)

        // Visualize magnitude of i-th band
        left_bands[tight_index] = (int16_t) (mag * HEIGHT);


        tight_index++;
    }

    for (i = 0, tight_index = 0; i < size; i += size / WIDTH) {

        float im = right_data[i];
        float re = right_data[i + 1];
        double mag = sqrt(im * im + re * re);

        right_bands[tight_index] = (int16_t) (mag * HEIGHT);

        tight_index++;
    }
};

void screen_draw(lmLedMatrix *matrix, double elapsed) {
    lm_matrix_clear(matrix);

    int16_t x, y;

    for (x = 0; x < WIDTH; ++x) {
        for (y = 0; y < left_bands[x]; ++y) {
            lm_matrix_set_pixel(matrix, x, HEIGHT - y, &color);
        }

//        for (y = 0; y < right_bands[x]; ++y) {
//            lm_matrix_set_pixel(matrix, x, 32 - (y + 16), &color);
//        }
    }
    lm_matrix_swap_buffers(matrix);
}

void register_visualize_screen() {
    register_screen("visualize", (screen_t) &screen_draw);
}
