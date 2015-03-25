set(SOURCE_HEADERS m3u.c)

set(SOURCE_FILES ${SOURCE_HEADERS} m3u.c)

add_library(m3u ${SOURCE_FILES} m3u.h)
target_link_libraries (m3u ${THREADING} ${FREETYPE_LIBRARIES} ${MATH_LIBRARIES})
