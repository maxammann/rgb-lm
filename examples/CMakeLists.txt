find_package (Threads)
include_directories ("${PROJECT_SOURCE_DIR}/src")

add_executable(simple main_test.c)
target_link_libraries (simple rgb_led_matrix ${CMAKE_THREAD_LIBS_INIT})
set_target_properties(simple
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

add_executable(pulsing pulsing.c)
target_link_libraries (pulsing rgb_led_matrix ${CMAKE_THREAD_LIBS_INIT})
set_target_properties(pulsing
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)
