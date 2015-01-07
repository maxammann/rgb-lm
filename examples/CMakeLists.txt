find_package (Threads)
include_directories ("${PROJECT_SOURCE_DIR}/src")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

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
target_link_libraries (simple_font lm)
set_target_properties(simple_font
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)



add_executable(mesmerizing mesmerizing.c)
target_link_libraries (mesmerizing lm m)
set_target_properties(mesmerizing
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)
