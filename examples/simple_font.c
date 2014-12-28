#include <stdio.h>
#include <assert.h>
#include <lm.h>


//int draw_font(lmLedMatrix *matrix) {
//    FT_Library library;
//    FT_Face face;
//
//    FT_Error error = FT_Init_FreeType(&library);
//
//
//    error = FT_New_Face(library,
////            "bin/6x12.bdf",
////            "/usr/share/fonts/truetype/msttcorefonts/arial.ttf",
//            "../arial_uni.ttf",
//            0,
//            &face);
//
//    FT_Select_Charmap(face, FT_ENCODING_MS_SYMBOL);
//
//    if (error == FT_Err_Unknown_File_Format) {
//        return -1;
//    }
//
//    error = FT_Set_Char_Size(face,
//            60 * 64, 0,
//            32, 0
//    );
//
//    FT_UInt glyph_index = FT_Get_Char_Index(face, L'☁');
//
//    error = FT_Load_Glyph(face, glyph_index, FT_LOAD_DEFAULT);
//    error = FT_Render_Glyph(face->glyph, FT_RENDER_MODE_MONO);
//
//    FT_Bitmap bitmap = face->glyph->bitmap;
//
////    printf("Rows: %d\n", bitmap.rows);
////    printf("Width: %d\n", bitmap.width);
////    printf("Mode: %d\n", bitmap.pixel_mode);
////    printf("Grays: %d\n", bitmap.num_grays);
////    printf("pitch: %d\n", bitmap.pitch);
//
//    int x, y;
//
//    FT_Int x_max = bitmap.width;
//    FT_Int y_max = bitmap.rows;
//
//    for (x = 0; x < x_max; x++) {
//        for (y = 0; y < y_max; y++) {
//            if (x < 0 || y < 0 || x >= 32 || y >= 32)
//                continue;
//
//            // For non-monochrome: unsigned char value = bitmap.buffer[bitmap.width * y + x ];
//
//            unsigned char *row = &bitmap.buffer[bitmap.pitch * y];
//            char value = row[(x >> 3)];
//
//
////            printf("%d\n", value);
//
//            if ((value & (128 >> (x & 7))) == 0) {
//                continue;
//            }
//
//            lm_matrix_set_pixel(matrix, x, y, 255, 0, 0);
//        }
//    }
//
//    return error;
//}


int main() {
    printf("Starting fonts\n");
    lm_gpio_init();

    uint32_t raw = lm_io_bits_new();
    uint32_t outputs = lm_gpio_init_output(raw);
    assert(outputs == raw);

    lmLedMatrix *matrix = lm_matrix_new(32, 32, 11);
    lm_matrix_clear(matrix);

//    lm_matrix_print_wchar(matrix, 'A', "/usr/share/fonts/truetype/msttcorefonts/arial.ttf", 0, 0, 0, 255, 255);

    lm_matrix_print_string(matrix, "Fu", "../arial_uni.ttf", 16, 16, 255, 0, 0);
//    lm_matrix_print_string(matrix, "❤", "../arial_uni.ttf", 11, 31, 255, 0, 0);

//    lm_matrix_print_wstring(matrix, L"ist scheiße!", "/usr/share/fonts/truetype/msttcorefonts/arial.ttf", 0, 16, 0, 255, 255);
//    printf("%d\n", draw_font(matrix));

    lmThread *thread = lm_thread_new(matrix);
    lm_thread_start(thread);

    lm_thread_wait(thread);

    lm_matrix_free(matrix);
    lm_thread_free(thread);
    return 0;
}
