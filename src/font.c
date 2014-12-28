#include "led-matrix.h"

#pragma clang diagnostic push
#pragma ide diagnostic ignored "UnusedImportStatement"

#include <ft2build.h>

#pragma clang diagnostic pop

#include <freetype.h>
#include <ftglyph.h>
#include <fttrigon.h>

#include <math.h>

#define TO_FT_STRING(text, len) int i; FT_ULong ft_text[length]; for (i = 0; i < len; i++) {ft_text[i] = (FT_ULong) text[i];}

typedef struct TGlyph_ {
    FT_Vector pos;
    FT_Glyph image;

} TGlyph, *PGlyph;


static void render_bitmap(lmLedMatrix *matrix, FT_Bitmap bitmap,
        FT_Int x, FT_Int y,
        uint8_t red, uint8_t green, uint8_t blue) {
    FT_Int i, j;

    for (j = 0; j < bitmap.rows; j++) {
        for (i = 0; i < bitmap.width; i++) {
            if (i < 0 || j < 0 || i >= 32 || j >= 32)
                continue;

            // For non-monochrome: unsigned char value = bitmap.buffer[bitmap.width * y + x ];

            unsigned char *row = &bitmap.buffer[bitmap.pitch * j];
            char value = row[(i >> 3)];

            if ((value & (128 >> (i & 7))) == 0) {
                continue;
            }

            lm_matrix_set_pixel(matrix, (uint16_t) (i + x), (uint16_t) (j + y), red, green, blue);
        }
    }
}

static inline void print_string(lmLedMatrix *matrix, FT_ULong *text, int length, char *font,
        uint16_t x, uint16_t y,
        uint8_t red, uint8_t green, uint8_t blue) {

    FT_Library library;
    FT_Face face;
    FT_Error error;

    error = FT_Init_FreeType(&library);

    if (error) {
        printf("Init error");
        return;
    }

    error = FT_New_Face(library,
            font,
            0,
            &face);

    if (error) {
        printf("new face error");
        return;
    }

    FT_Set_Char_Size(face,
            8 * 64, 0,
            100, 0
    );

    FT_GlyphSlot slot = face->glyph;   /* a small shortcut */
    FT_UInt glyph_index;
    FT_Long use_kerning;
    FT_UInt previous;
    int pen_x, pen_y, n;

    TGlyph glyphs[50];  /* glyphs table */

    FT_UInt num_glyphs;


    pen_x = 0;   /* start at (0,0) */
    pen_y = 0;

    num_glyphs = 0;
    use_kerning = FT_HAS_KERNING(face);
    previous = 0;

    for (n = 0; n < length; n++) {
        PGlyph glyph = &glyphs[n];

        glyph_index = FT_Get_Char_Index(face, text[n]);

        /* retrieve kerning distance and move pen position */
        if (use_kerning && previous && glyph_index) {
            FT_Vector delta;


            FT_Get_Kerning(face, previous, glyph_index,
                    FT_KERNING_DEFAULT, &delta);

            pen_x += delta.x >> 6;
        }

        /* store current pen position */
        glyph->pos.x = pen_x;
        glyph->pos.y = pen_y;

        /* load glyph image into the slot without rendering */
        error = FT_Load_Glyph(face, glyph_index, FT_LOAD_DEFAULT);
        if (error) {
            printf("load glyph error");
            continue;  /* ignore errors, jump to next glyph */
        }


        /* extract glyph image and store it in our table */
        error = FT_Get_Glyph(face->glyph, &glyph->image);
        if (error) {
            printf("get glyph error");
            continue;  /* ignore errors, jump to next glyph */
        }

        FT_Glyph_Transform(glyph->image, 0, &glyph->pos);

        /* increment pen position */
        pen_x += slot->advance.x >> 6;

        /* record current glyph index */
        previous = glyph_index;

        /* increment number of glyphs */
        num_glyphs++;
    }

    void compute_string_bbox(FT_BBox *abbox) {
        FT_BBox bbox;
        FT_BBox glyph_bbox;


        /* initialize string bbox to "empty" values */
        bbox.xMin = bbox.yMin = 32000;
        bbox.xMax = bbox.yMax = -32000;

        /* for each glyph image, compute its bounding box, */
        /* translate it, and grow the string bbox          */
        for (n = 0; n < num_glyphs; n++) {
            FT_Glyph_Get_CBox(glyphs[n].image, ft_glyph_bbox_pixels,
                    &glyph_bbox);

            if (glyph_bbox.xMin < bbox.xMin)
                bbox.xMin = glyph_bbox.xMin;

            if (glyph_bbox.yMin < bbox.yMin)
                bbox.yMin = glyph_bbox.yMin;

            if (glyph_bbox.xMax > bbox.xMax)
                bbox.xMax = glyph_bbox.xMax;

            if (glyph_bbox.yMax > bbox.yMax)
                bbox.yMax = glyph_bbox.yMax;
        }

        /* check that we really grew the string bbox */
        if (bbox.xMin > bbox.xMax) {
            bbox.xMin = 0;
            bbox.yMin = 0;
            bbox.xMax = 0;
            bbox.yMax = 0;
        }

        /* return string bbox */
        *abbox = bbox;
    }

    FT_BBox string_bbox;
    compute_string_bbox(&string_bbox);

    FT_Glyph image;
    FT_Vector pen;
    pen.x = 0;
    pen.y = 0;

    FT_Pos string_width = (string_bbox.xMax - string_bbox.xMin);
    FT_Pos string_height = (string_bbox.yMax - string_bbox.yMin);

    FT_Matrix trans_matrix;
    double angle = 30 * M_PI / 180.0;
    trans_matrix.xx = (FT_Fixed) (cos(angle) * 0x10000L);
    trans_matrix.xy = (FT_Fixed) (-sin(angle) * 0x10000L);
    trans_matrix.yx = (FT_Fixed) (sin(angle) * 0x10000L);
    trans_matrix.yy = (FT_Fixed) (cos(angle) * 0x10000L);

//    double scale = 1.5;
//    trans_matrix.xx = 0x10000L * scale;
//    trans_matrix.xy = 0;
//    trans_matrix.yx = 0;
//    trans_matrix.yy = 0x10000L * scale;

    for (n = 0; n < num_glyphs; n++) {
        error = FT_Glyph_Copy(glyphs[n].image, &image);
        if (error) continue;

        FT_Vector delta;
        delta.x = x * 64;
        delta.y = -y * 64;
        FT_Glyph_Transform(image, 0, &delta);
        FT_Glyph_Transform(image, 0, &pen);


        error = FT_Glyph_To_Bitmap(
                &image,
                FT_RENDER_MODE_MONO,
                0,
                1);

        if (!error) {
            FT_BitmapGlyph bit = (FT_BitmapGlyph) image;


            render_bitmap(matrix, bit->bitmap,
                    bit->left,
                    string_height - bit->top,
                    red, green, blue);


            /* increment pen position --                       */
            /* we don't have access to a slot structure,       */
            /* so we have to use advances from glyph structure */
            /* (which are in 16.16 fixed float format)         */
            pen.x += image->advance.x >> 10;
            pen.y += image->advance.y >> 10;

            FT_Done_Glyph(image);
        }
    }
}

void lm_matrix_print_string(lmLedMatrix *matrix, const char *text, char *font,
        uint16_t x, uint16_t y,
        uint8_t red, uint8_t green, uint8_t blue) {
    int length = (int) strlen(text);
    TO_FT_STRING(text, length);
    print_string(matrix, ft_text, length, font, x, y, red, green, blue);
}

void lm_matrix_print_wstring(lmLedMatrix *matrix, const wchar_t *text, char *font,
        uint16_t x, uint16_t y,
        uint8_t red, uint8_t green, uint8_t blue) {
    int length = (int) wcslen(text);
    TO_FT_STRING(text, length);
    print_string(matrix, ft_text, length, font, x, y, red, green, blue);
}


