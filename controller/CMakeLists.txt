find_package (ProtobufC REQUIRED)



include_directories(${PROTOBUFC_INCLUDE_DIRS})
include_directories(${Boost_INCLUDE_DIRS})
include_directories ("${PROJECT_SOURCE_DIR}/src")


set(SOURCE_HEADERS)

include_directories(${CMAKE_CURRENT_BINARY_DIR})
PROTOBUFC_GENERATE_C(PROTO_SRCS PROTO_HDRS lm.proto)

set(SOURCE_FILES ${SOURCE_HEADERS} ${PROTO_SRCS} ${PROTO_HDRS})


add_executable(client ${SOURCE_FILES} client.c)
target_link_libraries (client rgb_led_matrix ${PROTOBUFC_LIBRARY})



add_executable(server ${SOURCE_FILES} server.c )
target_link_libraries (server rgb_led_matrix ${PROTOBUFC_LIBRARY})


#install (TARGETS protocol DESTINATION lib)
#install (FILES ${SOURCE_HEADERS} DESTINATION include/protocol)
