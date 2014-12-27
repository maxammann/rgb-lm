package lm;

import javax.imageio.ImageIO;
import java.awt.geom.AffineTransform;
import java.awt.image.AffineTransformOp;
import java.awt.image.BufferedImage;
import java.io.File;
import java.io.IOException;

/**
 * Represents a Main
 */
public class Main {

    public static void main(String[] args) throws IOException {
        LmLibrary lm = LmLibrary.INSTANCE;

        lm.lm_gpio_init();
        lm.lm_gpio_init_output(lm.lm_io_bits_new());
        LmLibrary.lmLedMatrix matrix = lm.lm_matrix_new((short) 32, (short) 32, (byte) 1);
        lm.lm_matrix_clear(matrix);


        BufferedImage read = ImageIO.read(new File("img.png"));

        BufferedImage img = new BufferedImage(32, 32, BufferedImage.TYPE_INT_RGB);
        AffineTransform at = new AffineTransform();
        at.scale(0.16, 0.16);
        AffineTransformOp scaleOp = new AffineTransformOp(at, AffineTransformOp.TYPE_BILINEAR);
        img = scaleOp.filter(read, img);

        for (int x = 0; x < img.getHeight(); x++) {
            for (int y = 0; y < img.getWidth(); y++) {
                int[] rgb = getPixelData(img, x, y);

                lm.lm_matrix_set_pixel(matrix, (short) x, (short) y, (byte) rgb[0], (byte) rgb[1], (byte) rgb[2]);

            }
        }

        LmLibrary.lmThread thread = lm.lm_thread_new(matrix);
        lm.lm_thread_start(thread);

        lm.lm_thread_wait(thread);
    }

    private static int[] getPixelData(BufferedImage img, int x, int y) {
        int argb = img.getRGB(x, y);

        return new int[]{
                (argb >> 16) & 0xff, //red
                (argb >> 8) & 0xff, //green
                (argb) & 0xff  //blue
        };
    }

    public static void main1(String[] args) {
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
