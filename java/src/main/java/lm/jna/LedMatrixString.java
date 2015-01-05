package lm.jna;

import com.sun.jna.WString;

/**
 * Represents a lmString
 */
public class LedMatrixString {
    private static LmLibrary INSTANCE = LmLibrary.INSTANCE;
    private final LmLibrary.lmString lmString;

    public LedMatrixString() {
        lmString = INSTANCE.lm_fonts_string_new();
    }

    public void populate(String text, FontLibrary.Font font) {
        INSTANCE.lm_fonts_populate_string(font.getLibrary(), lmString, text, font.getNative());
    }

    public void populateWide(String text, FontLibrary.Font font) {
        INSTANCE.lm_fonts_populate_wstring(font.getLibrary(), lmString, new WString(text), font.getNative());
    }

    public void render(LedMatrix matrix, int x, int y, RGB rgb) {
        INSTANCE.lm_fonts_render_string(matrix.getNative(), lmString, ((short) x), ((short) y), rgb);
    }

    public void renderHorizontalCentered(LedMatrix matrix, int y, RGB rgb) {
        render(
                matrix,
                matrix.getColumns() / 2 - getWidth() / 2,
                y,
                rgb
        );
    }

    public void renderVerticalCentered(LedMatrix matrix, int x, RGB rgb) {
        render(
                matrix,
                x,
                matrix.getRows() / 2 - getHeight() / 2,
                rgb
        );
    }

    public void free() {
        INSTANCE.lm_fonts_string_free(lmString);
    }

    public int getWidth() {
        return INSTANCE.lm_fonts_string_width(lmString);
    }

    public int getHeight() {
        return INSTANCE.lm_fonts_string_height(lmString);
    }
}
