package lm;

/**
 * Represents a GPIO
 */
public class GPIO {
    private static LmLibrary INSTANCE = LmLibrary.INSTANCE;

    public static void init() {
        INSTANCE.lm_gpio_init();
        INSTANCE.lm_gpio_init_output(INSTANCE.lm_io_bits_new());
    }
}
