package lm;

import com.sun.jna.NativeLong;

import java.io.IOException;

/**
 * Represents a Main
 */
public class Main {

    //        BufferedImage read = ImageIO.read(new File("img.png"));
//        BufferedImage img = new BufferedImage(32, 32, BufferedImage.TYPE_INT_RGB);
//        AffineTransform at = new AffineTransform();
//        at.scale(0.16, 0.16);
//        AffineTransformOp scaleOp = new AffineTransformOp(at, AffineTransformOp.TYPE_BILINEAR);
//        img = scaleOp.filter(read, img);
//
//        for (int x = 0; x < img.getHeight(); x++) {
//            for (int y = 0; y < img.getWidth(); y++) {
//                int[] rgb = getPixelData(img, x, y);
//
//                rgb_.ByValue color = new rgb_.ByValue();
//
//                color.r = (byte) rgb[0];
//                color.g = (byte) rgb[1];
//                color.b = (byte) rgb[2];
////
//                lm.lm_matrix_set_pixel(matrix, (short) x, (short) y, RED);
//            }
//        }


    public static void main(String[] args) throws IOException {
        LmLibrary lm = LmLibrary.INSTANCE;

        System.out.println("Font: " + args[0]);
        LmLibrary.rgb_ RED = new LmLibrary.rgb_();
        RED.r = (byte) 128;

        lm.lm_gpio_init();

        lm.lm_gpio_init_output(lm.lm_io_bits_new());

        LmLibrary.lmLedMatrix matrix = lm.lm_matrix_new((short) 32, (short) 32, (byte) 1);
        lm.lm_matrix_clear(matrix);

        LmLibrary.lmFontLibrary library = lm.lm_fonts_init();
        LmLibrary.lmFont font = lm.lm_fonts_font_new(library, args[0], 20);

        for (int x = 0; x < 32; x++) {
            for (int y = 0; y < 32; y++) {
//                Structure.newInstance(rgb_.class);
                LmLibrary.rgb_ rgb = new LmLibrary.rgb_();


//                rgb.r = rgb.b = rgb.g = 0;
                lm.lm_matrix_set_pixel(matrix, (short) x, (short) y, new LmLibrary.rgb_((byte) 55, (byte) 55, (byte) 55));

//                System.out.println(rgb.toString(true));
//                rgb.clear();
            }
        }

        lm.lm_fonts_print_string(library, matrix, "test", font, (short) 0, (short) 2, RED);
        lm.lm_fonts_font_free(library, font);

        lm.lm_matrix_swap_buffers(matrix);

        LmLibrary.lmThread thread = lm.lm_thread_new(matrix, new NativeLong(LmLibrary.DEFAULT_BASE_TIME_NANOS));
        lm.lm_thread_start(thread);

        try {
            Thread.sleep(5000);
        } catch (InterruptedException e) {
            e.printStackTrace();
        }

        lm.lm_thread_free(thread);
        lm.lm_matrix_free(matrix);

        lm.lm_fonts_free(library);
    }

//    private static int[] getPixelData(BufferedImage img, int x, int y) {
//        int argb = img.getRGB(x, y);
//
//        return new int[]{
//                (argb >> 16) & 0xff, //red
//                (argb >> 8) & 0xff, //green
//                (argb) & 0xff  //blue
//        };
//    }
}
