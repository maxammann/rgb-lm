find_package (Threads)

set(SOURCE_FILES led-matrix.c gpio.c thread.c io_bits.c io_bits.h lm.h)

add_library(rgb_led_matrix ${SOURCE_FILES})
target_link_libraries (rgb_led_matrix ${CMAKE_THREAD_LIBS_INIT})
