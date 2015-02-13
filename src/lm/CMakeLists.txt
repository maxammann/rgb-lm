set(FREETYPE_INCLUDE_DIRS /usr/include/freetype2/)


find_package(Freetype)
find_library(MATH_LIBRARIES m)

include_directories(${FREETYPE_INCLUDE_DIRS})

set(SOURCE_HEADERS font.h led-matrix.h thread.h gpio.h io_bits.h lm.h)

set(SOURCE_FILES ${SOURCE_HEADERS} led-matrix.c font.c gpio.c thread.c io_bits.c)

add_library(rgb_led_matrix ${SOURCE_FILES})
target_link_libraries (rgb_led_matrix ${THREADING} ${FREETYPE_LIBRARIES} ${MATH_LIBRARIES})
set_target_properties(rgb_led_matrix
    PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
)



#add_library(lm SHARED ${SOURCE_FILES})
#target_link_libraries (lm ${CMAKE_THREAD_LIBS_INIT} ${FREETYPE_LIBRARIES} ${MATH_LIB})
#set_target_properties(lm
#    PROPERTIES
#    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
#)

#install (TARGETS lm DESTINATION lib)
#install (FILES ${SOURCE_HEADERS} DESTINATION include/lm)
