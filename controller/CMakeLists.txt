find_package (ProtobufC REQUIRED)
find_package (PkgConfig REQUIRED)

find_library(RT_LIBRARIES rt)

pkg_check_modules (GLIB2   glib-2.0>=2.10)


include_directories(${PROTOBUFC_INCLUDE_DIRS})
include_directories("${PROJECT_SOURCE_DIR}/src")
include_directories(${GLIB2_INCLUDE_DIRS})


set(SOURCE_HEADERS)

include_directories(${CMAKE_CURRENT_BINARY_DIR})

PROTOBUFC_GENERATE_C(PROTO_SRCS PROTO_HDRS lm.proto)

set(SOURCE_FILES ${SOURCE_HEADERS} ${PROTO_SRCS} ${PROTO_HDRS})


add_executable(server ${SOURCE_FILES} main.c controller.c server.c screen/screen.c screen/screens.h)
target_link_libraries (server rgb_led_matrix ${PROTOBUFC_LIBRARY} ${GLIB2_LIBRARIES} ${RT_LIBRARIES})


#install (TARGETS protocol DESTINATION lib)
#install (FILES ${SOURCE_HEADERS} DESTINATION include/protocol)
