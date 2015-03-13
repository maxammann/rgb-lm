cmake_minimum_required(VERSION 2.8.4)
project(rgb_led_matrix)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu99")

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake-modules )

add_definitions(-DREV2)

add_definitions(-DRT)

set(CMAKE_BUILD_TYPE Release)
#set(CMAKE_BUILD_TYPE Debug)

find_package (Threads)
find_library(REAL_TIME rt)

set(THREADING ${REAL_TIME} ${CMAKE_THREAD_LIBS_INIT})

add_subdirectory(src/lm)
add_subdirectory(examples)
add_subdirectory(server)


add_executable(measure measure.c)
target_link_libraries (measure ${THREADING})
set_target_properties(measure
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)
