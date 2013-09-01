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
find_package (Boost COMPONENTS system)
if (Boost_FOUND)
	include_directories(${Boost_INCLUDE_DIR})
	link_directories(${Boost_LIBRARY_DIRS})
	message("${Boost_LIBRARIES}")
else ()
	message (FATAL_ERROR "You need boost to build this, set the BOOST_ROOT or BOOSTROOT env variables, or pass them to cmake")
endif()
add_definitions(-DBOOST_ALL_NO_LIB)

set (BUILD_TESTS CACHE BOOL "If set to true, test files will be compiled in")

if(BUILD_TESTS)
	set (TEST_FILES src/tests/participant_test.cpp)
endif()

add_executable (openotpd
	src/core/main.cpp 
	src/core/config.cpp 
	src/core/logger.cpp 
	src/core/config.h 
	src/core/global.h
	src/core/global.cpp
	src/core/logger.h
	src/messagedirector/messagedirector.cpp 
	src/messagedirector/messagedirector.h
	src/messagedirector/MDNetworkParticipant.cpp
	src/messagedirector/MDNetworkParticipant.h
	src/core/datagram.h
	src/core/datagramIterator.h
	${TEST_FILES}
)

source_group ("Header Files" FILES src/core/config.h src/core/global.h src/core/logger.h src/messagedirector/messagedirector.h)

add_library (dcparser 
	src/dcparser/p3dcparser_composite1.cxx 
	src/dcparser/p3dcparser_composite2.cxx 
	src/dcparser/dcLexer.cxx
	src/dcparser/dcParser.cxx
)

add_dependencies (openotpd dcparser)
target_link_libraries (openotpd dcparser ${YAMLCPP_LIBNAME} ${Boost_LIBRARIES})
