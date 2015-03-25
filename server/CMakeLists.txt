find_package (ProtobufC REQUIRED)
find_package (WiringPi REQUIRED)
find_package (PkgConfig REQUIRED)
find_package (AO REQUIRED)
find_package (libsndfile REQUIRED)
find_package (AVCodec REQUIRED)
find_package (AVFormat REQUIRED)
find_package (AVUtil REQUIRED)
find_package (AVResample REQUIRED)

pkg_check_modules (GLIB2   glib-2.0>=2.10)


include_directories(${PROTOBUFC_INCLUDE_DIRS})
include_directories("${PROJECT_SOURCE_DIR}/src")
include_directories("${PROJECT_SOURCE_DIR}/m3u")
include_directories(${GLIB2_INCLUDE_DIRS})
include_directories(${WIRINGPI_INCLUDE_DIRS})
include_directories(${LIBAO_INCLUDE_DIR})
include_directories(${LIBSNDFILE_INCLUDE_DIR})

include_directories(${AVFormat_INCLUDE_DIRS})
include_directories(${AVCodec_INCLUDE_DIRS})
include_directories(${AVResample_INCLUDE_DIRS})


set(SOURCE_HEADERS)

include_directories(${CMAKE_CURRENT_BINARY_DIR})

PROTOBUFC_GENERATE_C(PROTO_SRCS PROTO_HDRS ../lm.proto)

set(SOURCE_FILES ${SOURCE_HEADERS} ${PROTO_SRCS} ${PROTO_HDRS})

option(FFMPEG_COMPAT "FFMPEG_COMPAT" ON)

if (${FFMPEG_COMPAT})
	set(SOURCE_FILES ${SOURCE_FILES} ffmpeg/ffmpeg_compat.c)
endif (${FFMPEG_COMPAT})

add_executable(server ${SOURCE_FILES}
	main.c
	ffmpeg/audio_ffmpeg.c
 	controller.c
	server.c
 	alarms.c
 	discovery/discovery_server.c
 	screen/screen.c
 	screen/menu.c screen/menu.h
  	screen/alarms.c screen/alarms.h
  	screen/example.c screen/example.h
  	rotary_encoder.c rotary_encoder.h audio.h)

  	message(${AVResample_LIBRARIES})

target_link_libraries (server m3u)
target_link_libraries (server rgb_led_matrix
${PROTOBUFC_LIBRARY}
${GLIB2_LIBRARIES}
${THREADING}
${WIRINGPI_LIBRARIES}
${LIBAO_LIBRARIES} ${LIBSNDFILE_LIBRARY}
${AVCodec_LIBRARIES}
${AVFormat_LIBRARIES} ${AVUtil_LIBRARIES} ${AVResample_LIBRARIES}
)


#add_executable(test pc_main.c)
#target_link_libraries (test rgb_led_matrix ${LIBAO_LIBRARIES} ${LIBSNDFILE_LIBRARY})

#install (TARGETS protocol DESTINATION lib)
#install (FILES ${SOURCE_HEADERS} DESTINATION include/protocol)
