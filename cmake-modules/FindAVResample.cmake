include(LibAVFindComponent)

libav_find_component("Resample")

# Handle arguments
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(AVResample
	DEFAULT_MSG
	AVResample_LIBRARIES
	AVResample_INCLUDE_DIRS
)

mark_as_advanced(AVResample_LIBRARIES AVResample_INCLUDE_DIRS)
