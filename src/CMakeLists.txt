find_package (Threads)
find_package(Freetype)
find_library(MATH_LIB m)

include_directories(/usr/include/freetype2/)

set(SOURCE_FILES led-matrix.c font.c gpio.c thread.c io_bits.c io_bits.h lm.h)

add_library(rgb_led_matrix ${SOURCE_FILES})
target_link_libraries (rgb_led_matrix ${CMAKE_THREAD_LIBS_INIT} ${FREETYPE_LIBRARIES} ${MATH_LIB})
set_target_properties(rgb_led_matrix
    PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
)
