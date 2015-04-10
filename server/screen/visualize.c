#include "visualize.h"
#include "../controller.h"
#include <fftw3.h>
#include <math.h>
#include <lm/led-matrix.h>
#include "screen.h"
#include <libavcodec/avfft.h>
#include "stdlib.h"

int16_t power[32];


void detrend(float array[], int n)
{
     double x, y, a, b;
     double sy = 0.0,
            sxy = 0.0,
            sxx = 0.0;
     int i;

     for (i=0, x=(-n/2.0+0.5); i<n; i++, x+=1.0)
     {
          y = array[i];
          sy += y;
          sxy += x * y;
          sxx += x * x;
     }
     b = sxy / sxx;
     a = sy / n;

     for (i=0, x=(-n/2.0+0.5); i<n; i++, x+=1.0)
          array[i] -= (a+b*x);
}

void buffer_visualize(uint16_t *data, int n) {
    /*fftw_complex *out;*/
    /*fftw_plan plan;*/
    int i, tight_index = 0;


    int in_size = n / 2;
    float *in = malloc(sizeof(float) * 2 * in_size);
    float *out = in;

    for (i = 0; i < n; i+=2) {
        uint16_t left = data[i];
        uint16_t right = data[i + 1];
        
        /*double window_modifier = (0.5 * ( 1 - cos(2 * M_PI * tight_index / in_size)));*/
        double window_modifier = 1;
        in[tight_index] = window_modifier  * ((left + right) / 65536.0f);
        tight_index++;
    }

    RDFTContext *ctx = av_rdft_init(log2(in_size), DFT_R2C);
    av_rdft_calc(ctx, in);
    /*out = (fftw_complex *) fftw_malloc(sizeof(fftw_complex) * in_size);*/

    /*plan = fftw_plan_dft_r2c_1d(in_size, in, out, FFTW_ESTIMATE);*/
    /*fftw_execute(plan);*/

    int size = in_size / 2;

    for (i = 0, tight_index = 0; i < size; i+= size / 32) {
        power[tight_index] = (int16_t) sqrt(out[i] * out[i]);
        ++tight_index;
    }

    /*fftw_destroy_plan(plan);*/
    /*fftw_free(out);*/
    av_rdft_end(ctx);
    free(in);
};

void screen_draw(lmLedMatrix *matrix, double elapsed) {
    lm_matrix_clear(matrix);

    int16_t x, y;

    rgb c = {255, 160, 20};

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
