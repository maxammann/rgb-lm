package lm;

/**
 * Represents a Main
 */
public class Main {

    public static void main(String[] args) {
        LmLibrary lm = LmLibrary.INSTANCE;

        lm.lm_gpio_init();

        lm.lm_gpio_init_output(lm.lm_io_bits_new());


        LmLibrary.lmLedMatrix matrix = lm.lm_matrix_new((short) 32, (short) 32, (byte) 1);

        lm.lm_matrix_clear(matrix);
        lm.lm_matrix_fill(matrix, (byte) 255, (byte) 0, (byte) 0);

        LmLibrary.lmThread thread = lm.lm_thread_new(matrix);
        lm.lm_thread_start(thread);

        lm.lm_thread_wait(thread);

        lm.lm_matrix_free(matrix);
        lm.lm_thread_free(thread);
    }
}
