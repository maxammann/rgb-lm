#include "led-matrix.h"
//#include <ft2build.h>
#include <ft2build.h>
#include <freetype.h>

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
            10 * 64, 0,
            100, 0
    );

    return face;
}

static inline FT_Bitmap create_bitmap(FT_Face face, char c) {
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

    for (j = bitmap.rows - 1; j >= 0; j--) {
        for (i = 0; i < bitmap.width; i++) {
            if (i < 0 || j < 0 || i >= 32 || j >= 32)
                continue;

            // For non-monochrome: unsigned char value = bitmap.buffer[bitmap.width * y + x ];

            unsigned char *row = &bitmap.buffer[bitmap.pitch * j];
            char value = row[(i >> 3)];

            if ((value & (128 >> (i & 7))) == 0) {
                continue;
            }

            lm_matrix_set_pixel(matrix, i + x, (j - bitmap.rows - 1) + y, red, green, blue);
        }
    }
}

void lm_matrix_print_string(lmLedMatrix *matrix, char *s, char *font,
        uint16_t x, uint16_t y,
        uint8_t red, uint8_t green, uint8_t blue) {
    FT_Face face = create_font(font);

    int i;

    size_t length = strlen(s);

    FT_GlyphSlot slot = face->glyph;

    int pen_x = x;
    for (i = 0; i < length; ++i) {
        FT_Bitmap bitmap = create_bitmap(face, s[i]);

        print_wchar(matrix, bitmap, pen_x + slot->bitmap_left, y, red, green, blue);

        pen_x += slot->advance.x >> 6;
    }
}
