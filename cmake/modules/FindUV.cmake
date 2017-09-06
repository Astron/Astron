# - Find UV
# Find the libuv includes and library
# This module defines
#  UV_INCLUDE_DIR
#  UV_LIBRARY

find_path(UV_DIR
    NAMES include/uv.h
    PATHS ${PROJECT_SOURCE_DIR}/dependencies/uvw/deps/libuv/src
)

if (NOT UV_DIR)
    message(FATAL_ERROR "libuv not found!")
endif()

set(UV_INCLUDE_DIR "${UV_DIR}/include")

find_library(UV_LIBRARY NAMES libuv.a libuv PATHS ${UV_DIR} PATH_SUFFIXES .libs)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(UV DEFAULT_MSG UV_LIBRARY UV_INCLUDE_DIR)
