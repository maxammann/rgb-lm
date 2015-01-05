package lm.jna;

import com.sun.jna.WString;

/**
 * Represents a LedMatrix
 */
public class LedMatrix {
    private static LmLibrary INSTANCE = LmLibrary.INSTANCE;
    private final LmLibrary.lmLedMatrix lmLedMatrix;


    public LedMatrix(int columns, int rows, int pwmBits) {
        lmLedMatrix = INSTANCE.lm_matrix_new((short) columns, (short) rows, (byte) pwmBits);
    }

    public void free() {
        INSTANCE.lm_matrix_free(lmLedMatrix);
    }

    public void lock() {
        INSTANCE.lm_matrix_lock(lmLedMatrix);
    }

    public void unlock() {
        INSTANCE.lm_matrix_unlock(lmLedMatrix);

    }

    public void fill(RGB rgb) {
        INSTANCE.lm_matrix_fill(lmLedMatrix, rgb);
    }

    public void setPixel(int x, int y, RGB rgb) {
        INSTANCE.lm_matrix_set_pixel(lmLedMatrix, (short) x, (short) y, rgb);
    }

    public void renderString(FontLibrary.Font font, String text, int x, int y, RGB rgb) {
        INSTANCE.lm_fonts_print_string(font.getLibrary(), lmLedMatrix, text, font
                .getNative(), (short) x, (short) y, rgb);
    }

    public void renderWideString(FontLibrary.Font font, String text, int x, int y, RGB rgb) {
        INSTANCE.lm_fonts_print_wstring(font.getLibrary(), lmLedMatrix, new WString(text), font
                .getNative(), (short) x, (short) y, rgb);
    }

    public void clear() {
        INSTANCE.lm_matrix_clear(lmLedMatrix);
    }

    public void swapBuffers() {
        INSTANCE.lm_matrix_swap_buffers(lmLedMatrix);
    }

    public int getColumns() {
        return INSTANCE.lm_matrix_columns(lmLedMatrix);
    }

    public int getRows() {
        return INSTANCE.lm_matrix_rows(lmLedMatrix);
    }

    public int getPWMBits() {
        return INSTANCE.lm_matrix_pwm_bits(lmLedMatrix);
    }

    LmLibrary.lmLedMatrix getNative() {
        return lmLedMatrix;
    }
}

