#include <pthread.h>
#include <stdlib.h>

#pragma clang diagnostic push
#pragma ide diagnostic ignored "UnusedImportStatement"

#include <string.h>
//#include <ft2build.h>
#include <ft2build.h>
#include <freetype.h>
#include <wchar.h>

#include "stdint.h"
#include "led-matrix.h"

#ifdef INVERSE_RGB_DISPLAY_COLORS
#  define COLOR_OUT_BITS(x) (x) ^ 0xffff
#else
#  define COLOR_OUT_BITS(x) (x)
#endif

#define COLOR_SHIFT MAX_BITPLANES - 8

struct lmLedMatrix_ {
    uint16_t columns, rows;
    uint8_t row_mask;

    uint8_t pwm_bits;
    io_bits *bitplane_buffer;

    pthread_mutex_t buffer_mutex;
};

lmLedMatrix *lm_matrix_new(uint16_t columns, uint16_t rows, uint8_t pwm_bits) {
    lmLedMatrix *matrix = malloc(sizeof(lmLedMatrix));

    matrix->columns = columns;
    matrix->rows = rows;
    uint8_t double_rows = lm_matrix_double_rows(matrix);
    matrix->row_mask = double_rows - (uint8_t) 1;
    matrix->pwm_bits = pwm_bits;
    matrix->bitplane_buffer = calloc(1, sizeof(io_bits) * double_rows * columns * MAX_BITPLANES);
    pthread_mutex_init(&matrix->buffer_mutex, NULL);

    return matrix;
}

void lm_matrix_free(lmLedMatrix *matrix) {
    pthread_mutex_destroy(&matrix->buffer_mutex);
    free(matrix->bitplane_buffer);
    free(matrix);
}

uint16_t lm_matrix_columns(lmLedMatrix *matrix) {
    return matrix->columns;
}

uint16_t lm_matrix_rows(lmLedMatrix *matrix) {
    return matrix->rows;
}

void lm_matrix_lock(lmLedMatrix *matrix) {
    pthread_mutex_lock(&matrix->buffer_mutex);
}

void lm_matrix_unlock(lmLedMatrix *matrix) {
    pthread_mutex_unlock(&matrix->buffer_mutex);
}

uint16_t lm_matrix_pwm_bits(lmLedMatrix *matrix) {
    return matrix->pwm_bits;
}

inline uint8_t lm_matrix_double_rows(lmLedMatrix *matrix) {
    return (uint8_t) (matrix->rows / 2);
}

io_bits *lm_matrix_bit_plane(lmLedMatrix *matrix) {
    return matrix->bitplane_buffer;
}

static uint16_t map_color(uint16_t color) {
    return COLOR_OUT_BITS((COLOR_SHIFT > 0) ? (color << COLOR_SHIFT) : (color >> -COLOR_SHIFT));
}

//bitplanes code took from hzeller! https://github.com/hzeller/rpi-rgb-led-matrix/blob/440549553d58157cd3355b92fb791bf25f526fbd/lib/framebuffer.cc#L150
void lm_matrix_fill(lmLedMatrix *matrix, uint8_t r, uint8_t g, uint8_t b) {
    int i, row, col;
    io_bits *bitplane = matrix->bitplane_buffer;
    uint16_t columns = matrix->columns;

    const uint16_t red = map_color(r);
    const uint16_t green = map_color(g);
    const uint16_t blue = map_color(b);

    uint8_t double_rows = lm_matrix_double_rows(matrix);

    for (i = MAX_BITPLANES - matrix->pwm_bits; i < MAX_BITPLANES; ++i) {
        int mask = 1 << i;
        io_bits plane_bits;
        plane_bits.raw = 0;
        plane_bits.bits.r1 = plane_bits.bits.r2 = (bits_t) ((red & mask) == mask);
        plane_bits.bits.g1 = plane_bits.bits.g2 = (bits_t) ((green & mask) == mask);
        plane_bits.bits.b1 = plane_bits.bits.b2 = (bits_t) ((blue & mask) == mask);
        for (row = 0; row < double_rows; ++row) {
            lm_matrix_lock(matrix);
            io_bits *row_data = lm_io_bits_value_at(bitplane, columns, row, 0, i);
            for (col = 0; col < columns; ++col) {
                (row_data++)->raw = plane_bits.raw;
            }
            lm_matrix_unlock(matrix);
        }
    }
}

//bitplanes code took from hzeller! https://github.com/hzeller/rpi-rgb-led-matrix/blob/440549553d58157cd3355b92fb791bf25f526fbd/lib/framebuffer.cc#L171
void lm_matrix_set_pixel(lmLedMatrix *matrix,
        uint16_t x, uint16_t y,
        uint8_t r, uint8_t g, uint8_t b) {
    int i;

    uint16_t red = map_color(r);
    uint16_t green = map_color(g);
    uint16_t blue = map_color(b);

    uint8_t pwm = matrix->pwm_bits;
    uint16_t columns = matrix->columns;
    uint8_t double_rows = lm_matrix_double_rows(matrix);

    const int min_bit_plane = MAX_BITPLANES - pwm;

    lm_matrix_lock(matrix);
    io_bits *bits = lm_io_bits_value_at(matrix->bitplane_buffer, matrix->columns, y & matrix->row_mask, x, min_bit_plane);
    if (y < double_rows) {   // Upper sub-panel.
        for (i = min_bit_plane; i < MAX_BITPLANES; ++i) {
            int mask = 1 << i;

            bits->bits.r1 = (bits_t) ((red & mask) == mask);
            bits->bits.g1 = (bits_t) ((green & mask) == mask);
            bits->bits.b1 = (bits_t) ((blue & mask) == mask);
            bits += columns;
        }
    } else {
        for (i = min_bit_plane; i < MAX_BITPLANES; ++i) {
            int mask = 1 << i;
            bits->bits.r2 = (bits_t) ((red & mask) == mask);
            bits->bits.g2 = (bits_t) ((green & mask) == mask);
            bits->bits.b2 = (bits_t) ((blue & mask) == mask);
            bits += columns;
        }
    }
    lm_matrix_unlock(matrix);
}

static FT_Face create_font(char *font) {
    FT_Library library;
    FT_Face face;
    FT_Error error;

    FT_Init_FreeType(&library);

    FT_New_Face(library,
//            "bin/6x12.bdf",
//            "/usr/share/fonts/truetype/msttcorefonts/arial.ttf",
//            "../arial_uni.ttf",
            font,
            0,
            &face);

    error = FT_Select_Charmap(face, FT_ENCODING_MS_SYMBOL);

    if (error == FT_Err_Unknown_File_Format) {
        return NULL;
    }

    FT_Set_Char_Size(face,
            8 * 64, 0,
            100, 0
    );

    return face;
}

inline FT_Bitmap create_bitmap(FT_Face face, wchar_t c) {
    FT_UInt glyph_index = FT_Get_Char_Index(face, (FT_ULong) c);
    FT_Load_Glyph(face, glyph_index, FT_LOAD_DEFAULT);
    FT_Render_Glyph(face->glyph, FT_RENDER_MODE_MONO);

    //    printf("Rows: %d\n", bitmap.rows);
//    printf("Width: %d\n", bitmap.width);
//    printf("Mode: %d\n", bitmap.pixel_mode);
//    printf("Grays: %d\n", bitmap.num_grays);
//    printf("pitch: %d\n", bitmap.pitch);

    return face->glyph->bitmap;
}

static void print_wchar(lmLedMatrix *matrix, FT_Bitmap bitmap,
        uint16_t x, uint16_t y,
        uint8_t red, uint8_t green, uint8_t blue) {
    int i, j;

    for (i = 0; i < bitmap.width; i++) {
        for (j = 0; j < bitmap.rows; j++) {
            if (i < 0 || j < 0 || i >= 32 || j >= 32)
                continue;

            // For non-monochrome: unsigned char value = bitmap.buffer[bitmap.width * y + x ];

            unsigned char *row = &bitmap.buffer[bitmap.pitch * j];
            char value = row[(i >> 3)];

            if ((value & (128 >> (i & 7))) == 0) {
                continue;
            }

            lm_matrix_set_pixel(matrix, i + x, j + y, red, green, blue);
        }
    }
}


void lm_matrix_print_wchar(lmLedMatrix *matrix, wchar_t c, char *font,
        uint16_t x, uint16_t y,
        uint8_t red, uint8_t green, uint8_t blue) {

    FT_Face face = create_font(font);

    FT_Bitmap bitmap = create_bitmap(face, c);

    print_wchar(matrix, bitmap, x, y, red, green, blue);
}

void lm_matrix_print_wstring(lmLedMatrix *matrix, wchar_t *s, char *font,
        uint16_t x, uint16_t y,
        uint8_t red, uint8_t green, uint8_t blue) {
    FT_Face face = create_font(font);

    int i;

    size_t length = wcslen(s);

    FT_GlyphSlot slot = face->glyph;
//    FT_Vector pen;

    int pen_x = x;
    int pen_y = 0;

    for (i = 0; i < length; ++i) {
        FT_Bitmap bitmap = create_bitmap(face, s[i]);

//        FT_Set_Transform( face, 0, &pen );


        printf("top %d\n",(slot->advance.y));

        print_wchar(matrix, bitmap, pen_x + slot->bitmap_left,(slot->metrics.height - slot->bitmap_top), red, green, blue);

        pen_x += slot->advance.x >> 6;
    }
}

void lm_matrix_clear(lmLedMatrix *matrix) {
    io_bits *bits = matrix->bitplane_buffer;
    uint8_t double_rows = lm_matrix_double_rows(matrix);

    lm_matrix_lock(matrix);
    memset(bits, 0, sizeof(*bits) * double_rows * matrix->columns * MAX_BITPLANES);
    lm_matrix_unlock(matrix);
}


#pragma clang diagnostic pop
