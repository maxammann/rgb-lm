package lm;

/**
 * Represents a FontLibrary
 */
public class FontLibrary {
    private static LmLibrary INSTANCE = LmLibrary.INSTANCE;

    private final LmLibrary.lmFontLibrary library = INSTANCE.lm_fonts_init();

    public void free() {
        INSTANCE.lm_fonts_free(library);
    }

    public Font newFont(String font, int size) {
        return new Font(font, size);
    }

    public class Font {

        private final LmLibrary.lmFont lmFont;

        private Font(String font, int size) {
            lmFont = INSTANCE.lm_fonts_font_new(library, font, size);
        }

        public void free() {
            INSTANCE.lm_fonts_font_free(library, lmFont);
        }

        LmLibrary.lmFontLibrary getLibrary() {
            return library;
        }

        public LmLibrary.lmFont getNative() {
            return lmFont;
        }
    }
}
