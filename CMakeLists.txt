cmake_minimum_required(VERSION 2.8.4)
project(rgb_led_matrix)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu99")

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake-modules )

add_definitions(-DREV2)

#set(CMAKE_BUILD_TYPE RELWITHDEBINFO)
set(CMAKE_BUILD_TYPE Debug)

add_subdirectory(src/lm)
add_subdirectory(examples)
add_subdirectory(controller)
