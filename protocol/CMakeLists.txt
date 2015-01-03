find_package (ProtobufC)
find_package (ProtobufCRPC)

include_directories(${PROTOBUFC_INCLUDE_DIRS})
include_directories(${PROTOBUFCRPC_INCLUDE_DIRS})
include_directories ("${PROJECT_SOURCE_DIR}/src")


set(SOURCE_HEADERS)

include_directories(${CMAKE_CURRENT_BINARY_DIR})
PROTOBUFC_GENERATE_C(PROTO_SRCS PROTO_HDRS test.proto)

set(SOURCE_FILES ${SOURCE_HEADERS} ${PROTO_SRCS} ${PROTO_HDRS})

add_executable(client ${SOURCE_FILES} client.c)
target_link_libraries (client ${PROTOBUFC_LIBRARY} ${PROTOBUFCRPC_LIBRARY} rgb_led_matrix)

add_executable(server ${SOURCE_FILES} server.c)
target_link_libraries (server ${PROTOBUFC_LIBRARY} ${PROTOBUFCRPC_LIBRARY} rgb_led_matrix)


#install (TARGETS protocol DESTINATION lib)
#install (FILES ${SOURCE_HEADERS} DESTINATION include/protocol)
