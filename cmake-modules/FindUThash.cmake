FIND_PATH(UTHASH_INCLUDE_DIRS NAMES uthash.h
  PATHS
  /usr
  /usr/local
  /opt
  PATH_SUFFIXES
)

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(uthash DEFAULT_MSG UTHASH_INCLUDE_DIRS)