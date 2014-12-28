#include "font.h"

#pragma clang diagnostic push
#pragma ide diagnostic ignored "UnusedImportStatement"

#include <ft2build.h>

#pragma clang diagnostic pop

#include <freetype.h>
#include <ftglyph.h>

#include <math.h>
#include <ftcache.h>

FT_Library library;
FTC_Manager manager;
FTC_ImageCache image_cache;

typedef struct CacheFace_ {
    const char *file_path;
    int face_index;

} CacheFace, *PCacheFace;

struct lmFont_ {
    FTC_ScalerRec scaler;
};

static FT_Error face_requester(FTC_FaceID face_id,
        FT_Library library,
        FT_Pointer req_data,
        FT_Face *aface) {
    PCacheFace face = (PCacheFace) face_id;

    FT_Error error = FT_New_Face(library, face->file_path, face->face_index, aface);
    return error;
}

int lm_matrix_font_init() {
    if (FT_Init_FreeType(&library)) {
        return -1;
    }

    if (FTC_Manager_New(library, 0, 0, 0, face_requester, NULL, &manager)) {
        return -2;
    }

    if (FTC_ImageCache_New(manager, &image_cache)) {
        return -2;
    }

    return 0;
}

lmFont *lm_matrix_font_create(char *font_file, font_size_t size) {
    lmFont *font = malloc(sizeof(lmFont));

    CacheFace *cache_face = malloc(sizeof(CacheFace)); // How to fuck do I clear this?
    cache_face->face_index = 0;
    cache_face->file_path = font_file;

    FTC_ScalerRec scaler;

    scaler.face_id = cache_face;
    scaler.width = size;
    scaler.height = size;
    scaler.pixel = 1;

    font->scaler = scaler;

    return font;
}

FT_Face get_font_face(FTC_ScalerRec scaler) {
    FT_Size size;
    FTC_Manager_LookupSize(manager, &scaler, &size);
    return size->face;
}

FT_Matrix rotation_matrix(double angle) {
    FT_Matrix rot_matrix;
    rot_matrix.xx = (FT_Fixed) (cos(angle) * 0x10000L);
    rot_matrix.xy = (FT_Fixed) (-sin(angle) * 0x10000L);
    rot_matrix.yx = (FT_Fixed) (sin(angle) * 0x10000L);
    rot_matrix.yy = (FT_Fixed) (cos(angle) * 0x10000L);
    return rot_matrix;
}

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

static void compute_string_bbox(int num_glyphs, FT_Glyph *glyphs, FT_BBox *abbox) {
    int n;
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

static inline void print_string(lmLedMatrix *matrix, FT_ULong *text, int length, lmFont *font,
        uint16_t x, uint16_t y,
        uint8_t red, uint8_t green, uint8_t blue) {
    // Get font face
    FT_Error error;
    FT_Face face = get_font_face(font->scaler);

    // Load glyphs

    FT_GlyphSlot slot = face->glyph;   // a small shortcut
    FT_UInt glyph_index;
    FT_Long use_kerning;
    FT_UInt previous;
    int pen_x, pen_y, n;
    FT_Glyph glyphs[length];  // glyphs table
    FT_UInt num_glyphs;

    // start at (0,0)
    pen_x = 0;
    pen_y = 0;

    num_glyphs = 0;
    use_kerning = FT_HAS_KERNING(face);
    previous = 0;

    for (n = 0; n < length; n++) {
        FT_Vector pos;

        glyph_index = FT_Get_Char_Index(face, text[n]);

        /* retrieve kerning distance and move pen position */
        if (use_kerning && previous && glyph_index) {
            FT_Vector delta;


            FT_Get_Kerning(face, previous, glyph_index,
                    FT_KERNING_DEFAULT, &delta);

            pen_x += delta.x >> 6;
        }

        /* store current pen position */
        pos.x = pen_x;
        pos.y = pen_y;

        FTC_Node node;
        FT_Glyph cachedGlyph;

        error = FTC_ImageCache_LookupScaler(image_cache, &font->scaler, FT_LOAD_DEFAULT, glyph_index, &cachedGlyph, &node);

        if (error) {
            continue;  /* ignore errors, jump to next glyph */
        }

        error = FT_Glyph_Copy(cachedGlyph, &glyphs[n]);
        FTC_Node_Unref(node, manager);

        if (error) {
            continue;  /* ignore errors, jump to next glyph */
        }

        FT_Glyph_Transform(glyphs[n], 0, &pos);

        /* increment pen position */
        pen_x += slot->advance.x >> 6;

        /* record current glyph index */
        previous = glyph_index;

        /* increment number of glyphs */
        num_glyphs++;
    }

    // Compute box

    FT_BBox string_bbox;
    compute_string_bbox(num_glyphs, glyphs, &string_bbox);

//    FT_Int string_width = (FT_Int) (string_bbox.xMax - string_bbox.xMin);
    FT_Int string_height = (FT_Int) (string_bbox.yMax - string_bbox.yMin);


    // Render font and apply transformations
    FT_Glyph image;
    FT_Vector pen;
    pen.x = 0;
    pen.y = 0;

    for (n = 0; n < num_glyphs; n++) {
        error = FT_Glyph_Copy(glyphs[n], &image);
        if (error) continue;

        FT_Vector delta;
        delta.x = x * 64;
        delta.y = -y * 64;

        FT_Glyph_Transform(image, 0, &delta);

        error = FT_Glyph_To_Bitmap(
                &image,
                FT_RENDER_MODE_MONO,
                &pen,     // Apply pen
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

    for (n = 0; n < num_glyphs; ++n) {
        FT_Done_Glyph(glyphs[n]);
    }
}

#define TO_FT_STRING(text, len) int i; FT_ULong ft_text[length]; for (i = 0; i < len; i++) {ft_text[i] = (FT_ULong) text[i];}

void lm_matrix_print_string(lmLedMatrix *matrix, const char *text, lmFont *font,
        uint16_t x, uint16_t y,
        uint8_t red, uint8_t green, uint8_t blue) {
    int length = (int) strlen(text);
    TO_FT_STRING(text, length);
    print_string(matrix, ft_text, length, font, x, y, red, green, blue);
}

void lm_matrix_print_wstring(lmLedMatrix *matrix, const wchar_t *text, lmFont *font,
        uint16_t x, uint16_t y,
        uint8_t red, uint8_t green, uint8_t blue) {
    int length = (int) wcslen(text);
    TO_FT_STRING(text, length);
    print_string(matrix, ft_text, length, font, x, y, red, green, blue);
}

void lm_matrix_font_destroy(lmFont *font) {
    FTC_FaceID faceID = font->scaler.face_id;
    FTC_Manager_RemoveFaceID(manager, faceID);
    free(faceID);
    free(font);
}

void lm_matrix_font_free() {
    FTC_Manager_Reset(manager);
    FTC_Manager_Done(manager);
    FT_Done_FreeType(library);
}


void lm_matrix_create_string(lmString *string, const char *text, lmFont *font) {

}

void lm_matrix_create_wstring(lmString *string, const wchar_t *text, lmFont *font) {

}

void lm_matrix_render_string(lmLedMatrix *matrix, lmString *string,
        uint16_t x, uint16_t y,
        uint8_t red, uint8_t green, uint8_t blue) {

}
