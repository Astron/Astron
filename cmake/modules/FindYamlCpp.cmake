# Locate yaml-cpp
#
# This module defines
#  YAMLCPP_FOUND, if false, do not try to link to yaml-cpp
#  YAMLCPP_LIBRARY, where to find yaml-cpp
#  YAMLCPP_INCLUDE_DIR, where to find yaml.h
#  YAMLCPP_LIBRARY_DIR, the directory containing YAML_LIBRARY
#
# By default, the dynamic libraries of yaml-cpp will be found. To find the static ones instead,
# you must set the YAMLCPP_USE_STATIC_LIBS variable to TRUE before calling find_package(YamlCpp ...).

# attempt to find static library first if this is set
if(YAMLCPP_USE_STATIC_LIBS)
    set(YAMLCPP_STATIC libyaml-cpp.a)
endif()

# find the yaml-cpp include directory
find_path(YAMLCPP_INCLUDE_DIR yaml-cpp/yaml.h
          PATH_SUFFIXES include
          PATHS
          ~/Library/Frameworks/yaml-cpp/include/
          /Library/Frameworks/yaml-cpp/include/
          /usr/local/include/
          /usr/include/
          /sw/yaml-cpp/         # Fink
          /opt/local/yaml-cpp/  # DarwinPorts
          /opt/csw/yaml-cpp/    # Blastwave
          /opt/yaml-cpp/
          ${PROJECT_SOURCE_DIR}/dependencies/yaml-cpp-0.5.1/include)

# find the yaml-cpp library
find_library(YAMLCPP_LIBRARY
             NAMES ${YAMLCPP_STATIC} yaml-cpp libyaml-cppmd.lib
             PATH_SUFFIXES lib64 lib
             PATHS ~/Library/Frameworks
                    /Library/Frameworks
                    /usr/local
                    /usr
                    /sw
                    /opt/local
                    /opt/csw
                    /opt
                    ${PROJECT_SOURCE_DIR}/dependencies/yaml-cpp-0.5.1/)

get_filename_component(YAMLCPP_LIBRARY_DIR ${YAMLCPP_LIBRARY} PATH)

# handle the QUIETLY and REQUIRED arguments and set YAMLCPP_FOUND to TRUE if all listed variables are TRUE
include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(YamlCpp DEFAULT_MSG YAMLCPP_INCLUDE_DIR YAMLCPP_LIBRARY YAMLCPP_LIBRARY_DIR)
mark_as_advanced(YAMLCPP_INCLUDE_DIR YAMLCPP_LIBRARY)
