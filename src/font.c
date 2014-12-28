#include "led-matrix.h"
//#include <ft2build.h>
#include <freetype.h>
#include <ftglyph.h>


//            "bin/6x12.bdf",
//            "/usr/share/fonts/truetype/msttcorefonts/arial.ttf",
//            "../arial_uni.ttf",

typedef struct TGlyph_ {
    FT_UInt index;
    /* glyph index                  */
    FT_Vector pos;
    /* glyph origin on the baseline */
    FT_Glyph image;  /* glyph image                  */

} TGlyph, *PGlyph;


static void render_bitmap(lmLedMatrix *matrix, FT_Bitmap bitmap,
        uint16_t x, uint16_t y,
        uint8_t red, uint8_t green, uint8_t blue) {
    int i, j;

    for (j = 0; j < bitmap.rows; j++) {
        for (i = 0; i < bitmap.width; i++) {
            if (i < 0 || j < 0 || i >= 32 || j >= 32)
                continue;

            // For non-monochrome:
//            unsigned char value = bitmap.buffer[bitmap.width * y + x ];

            unsigned char *row = &bitmap.buffer[bitmap.pitch * j];
            char value = row[(i >> 3)];

            if ((value & (128 >> (i & 7))) == 0) {
                continue;
            }

            lm_matrix_set_pixel(matrix, i + x, j + y, red, green, blue);
        }
    }
}

void lm_matrix_print_string(lmLedMatrix *matrix, char *text, char *font,
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
            12 * 64, 0,
            100, 0
    );

    FT_GlyphSlot slot = face->glyph;   /* a small shortcut */
    FT_UInt glyph_index;
    FT_Bool use_kerning;
    FT_UInt previous;
    int pen_x, pen_y, n;

    FT_Glyph glyphs[50];   /* glyph image    */
    FT_Vector pos[50];   /* glyph position */
    FT_UInt num_glyphs;

    size_t num_chars = strlen(text);


    pen_x = 0;   /* start at (0,0) */
    pen_y = 0;

    num_glyphs = 0;
    use_kerning = FT_HAS_KERNING(face);
    previous = 0;

    for (n = 0; n < num_chars; n++) {
        /* convert character code to glyph index */
        glyph_index = FT_Get_Char_Index(face, text[n]);

        /* retrieve kerning distance and move pen position */
        if (use_kerning && previous && glyph_index) {
            FT_Vector delta;


            FT_Get_Kerning(face, previous, glyph_index,
                    FT_KERNING_DEFAULT, &delta);

            pen_x += delta.x >> 6;
        }

        /* store current pen position */
        pos[num_glyphs].x = pen_x;    //1: 0 2: 10
        pos[num_glyphs].y = pen_y;

        /* load glyph image into the slot without rendering */
        error = FT_Load_Glyph(face, glyph_index, FT_LOAD_DEFAULT);
        if (error) {
            printf("load glyph error");
            continue;  /* ignore errors, jump to next glyph */
        }


        /* extract glyph image and store it in our table */
        error = FT_Get_Glyph(face->glyph, &glyphs[num_glyphs]);
        if (error) {
            printf("get glyph error");
            continue;  /* ignore errors, jump to next glyph */
        }

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
            FT_Glyph_Get_CBox(glyphs[n], ft_glyph_bbox_pixels,
                    &glyph_bbox);

            glyph_bbox.xMin += pos[n].x;
            glyph_bbox.xMax += pos[n].x;
            glyph_bbox.yMin += pos[n].y;
            glyph_bbox.yMax += pos[n].y;

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

    /* compute string dimensions in integer pixels */
    FT_Pos string_width = string_bbox.xMax - string_bbox.xMin;
    FT_Pos string_height = string_bbox.yMax - string_bbox.yMin;

/* compute start pen position in 26.6 Cartesian pixels */
//    int start_x = ((my_target_width - string_width) / 2) * 64;
//    int start_y = ((my_target_height - string_height) / 2) * 64;

    int start_x = 0 * 64;
    int start_y = 0 * 64;

    for (n = 0; n < num_glyphs; n++) {
        FT_Glyph image;
        FT_Vector pen;


        image = glyphs[n];

        pen.x = start_x + pos[n].x * 64;
        pen.y = start_y + pos[n].y * 64;

        error = FT_Glyph_To_Bitmap(&image, FT_RENDER_MODE_MONO,
                &pen, 0);
        if (!error) {
            FT_BitmapGlyph bit = (FT_BitmapGlyph) image;


            render_bitmap(matrix, bit->bitmap,
                    bit->left + x,
                    string_height - bit->top + y, red, green, blue);

            FT_Done_Glyph(image);
        }
    }
}
