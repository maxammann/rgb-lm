include(LibAVFindComponent)

libav_find_component("Util")

list(APPEND AVUtil_DEFINITIONS "-D__STDC_CONSTANT_MACROS")

# Handle arguments
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(AVUtil
	DEFAULT_MSG
	AVUtil_LIBRARIES
	AVUtil_INCLUDE_DIRS
)

mark_as_advanced(AVUtil_LIBRARIES AVUtil_INCLUDE_DIRS)
