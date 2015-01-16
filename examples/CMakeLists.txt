find_package (Threads)
include_directories ("${PROJECT_SOURCE_DIR}/src")

add_executable(simple_fill simple_fill.c)
target_link_libraries (simple_fill rgb_led_matrix)
set_target_properties(simple_fill
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

add_executable(simple_pixels simple_pixels.c)
target_link_libraries (simple_pixels rgb_led_matrix)
set_target_properties(simple_pixels
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)


add_executable(pulsing pulsing.c)
target_link_libraries (pulsing rgb_led_matrix)
set_target_properties(pulsing
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)


add_executable(simple_font simple_font.c)
target_link_libraries (simple_font rgb_led_matrix)
set_target_properties(simple_font
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

add_executable(transform_fonts transform_fonts.c)
target_link_libraries (transform_fonts rgb_led_matrix)
target_link_libraries (transform_fonts ${MATH_LIBRARIES})
set_target_properties(transform_fonts
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)



add_executable(mesmerizing_screen mesmerizing.c)
target_link_libraries (mesmerizing_screen rgb_led_matrix)
target_link_libraries (mesmerizing_screen ${MATH_LIBRARIES})
set_target_properties(mesmerizing_screen
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)
