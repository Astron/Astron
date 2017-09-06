# - Find UVW
# Find uvw.hpp
# This module defines
#  UVW_INCLUDE_DIR

find_path(
    UVW_INCLUDE_DIR
    NAMES uvw.hpp
    PATHS ${CMAKE_SOURCE_DIR}/dependencies/uvw/src
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(UVW DEFAULT_MSG UVW_INCLUDE_DIR)
