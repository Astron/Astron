cmake_minimum_required (VERSION 2.6)
set_property ( GLOBAL PROPERTY USE_FOLDERS ON)
project (OOTP)

if (WIN32)
	add_definitions(-D_WIN32_WINDOWS)
endif()

set (YAMLCPP_INCLUDES "dependencies/yaml-cpp-0.5.1/include" CACHE PATH "Path to yamlcpp includes")
set (YAMLCPP_LIBRARIES "dependencies/yaml-cpp-0.5.1/lib" CACHE PATH "Path to yamlcpp libs")
set (YAMLCPP_LIBNAME "libyaml-cppmdd" CACHE STRING "name of yaml lib")
include_directories (${CMAKE_CURRENT_LIST_DIR}/${YAMLCPP_INCLUDES})
link_directories (${CMAKE_CURRENT_LIST_DIR}/${YAMLCPP_LIBRARIES})

#fucking yamlcpp needs boost
find_package (Boost)
if (BOOST_FOUND)
	include_directories(${BOOST_INCLUDE_DIRS})
	link_directories(${Boost_LIBRARY_DIRS})
	target_link_libraries(core ${BOOST_LIBRARIES})
endif()
#set (BOOST_INCLUDES "FILLME" CACHE PATH "Path to boost includes folder")
#include_directories (${BOOST_INCLUDES})
#set (BOOST_LIBS "FILLME" CACHE PATH "Path to boost includes folder")
#link_directories (${BOOST_LIBS})

add_executable (core 
	src/core/main.cpp 
	src/core/config.cpp 
	src/core/logger.cpp 
	src/core/config.h 
	src/core/global.h 
	src/core/logger.h
	src/messagedirector/messagedirector.cpp 
	src/messagedirector/messagedirector.h
	src/core/datagram.h
	src/core/datagramIterator.h
)
source_group ("Header Files" FILES src/core/config.h src/core/global.h src/core/logger.h src/messagedirector/messagedirector.h)

add_library (dcparser 
	src/dcparser/p3dcparser_composite1.cxx 
	src/dcparser/p3dcparser_composite2.cxx 
	src/dcparser/dcLexer.cxx
	src/dcparser/dcParser.cxx
)

add_dependencies (core dcparser)
target_link_libraries (core dcparser ${YAMLCPP_LIBNAME})