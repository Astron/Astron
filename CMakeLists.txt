cmake_minimum_required (VERSION 2.6)
set_property ( GLOBAL PROPERTY USE_FOLDERS ON)
project (OOTP)

if (WIN32)
	add_definitions(-D_WIN32_WINDOWS)
endif()

if(NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)
	SET(CMAKE_BUILD_TYPE RelWithDebInfo)
endif()

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2 -Wall")
	set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} -O2 -g")

	### Release Flags -- warnings are errors, release code should not have warnings ###
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O2 -Wall -Werror")
	
	### Debug flags, Wall
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall")
endif()

### Debug Flags -- compile in spam/debug log messages ###
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDEBUG_MESSAGES")

### YAML Depedency -- required by config ###
set(YAMLCPP_INCLUDES "dependencies/yaml-cpp-0.5.1/include" CACHE PATH "Path to yamlcpp includes")
if(EXISTS "${PROJECT_SOURCE_DIR}/dependencies/yaml-cpp-0.5.1/lib")
	set(YAMLCPP_LIBRARIES "dependencies/yaml-cpp-0.5.1/lib" CACHE PATH "Path to yamlcpp libs")
else()
	set(YAMLCPP_LIBRARIES "dependencies/yaml-cpp-0.5.1" CACHE PATH "Path to yamlcpp libs")
endif()
if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	set (YAMLCPP_LIBNAME "yaml-cpp" CACHE STRING "Name of YAML library")
	set (CMAKE_CXX_FLAGS "-pthread")
else ()
	set (YAMLCPP_LIBNAME "libyaml-cppmd" CACHE STRING "Name of YAML library")
	set (YAMLCPP_LIBNAME optimized ${YAMLCPP_LIBNAME} debug ${YAMLCPP_LIBNAME}d)
	message("${YAMLCPP_LIBNAME}")
endif()
include_directories (${CMAKE_CURRENT_LIST_DIR}/${YAMLCPP_INCLUDES})
link_directories (${CMAKE_CURRENT_LIST_DIR}/${YAMLCPP_LIBRARIES})

### BOOST Dependency -- required by yaml-cpp, and networking ###
set(Boost_USE_STATIC_LIBS ON CACHE BOOL "Test")
find_package (Boost COMPONENTS system)
if (NOT Boost_FOUND)
	set(Boost_USE_STATIC_LIBS OFF CACHE BOOL "Test")
	find_package (Boost COMPONENTS system)
endif()
if (Boost_FOUND)
	include_directories(${Boost_INCLUDE_DIR})
	link_directories(${Boost_LIBRARY_DIRS})
	message("${Boost_LIBRARIES}")
else ()
	message (FATAL_ERROR "You need boost to build this, set the BOOST_ROOT or BOOSTROOT env variables, or pass them to cmake")
endif()
add_definitions(-DBOOST_ALL_NO_LIB -D_SCL_SECURE_NO_WARNINGS)

set (BUILD_TESTS CACHE BOOL "If set to true, test files will be compiled in")

if(BUILD_TESTS)
	set (TEST_FILES
		src/tests/participant_test.cpp
		src/tests/performance_test.cpp
	)
endif()

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
	execute_process(COMMAND ${CMAKE_C_COMPILER} -dumpversion OUTPUT_VARIABLE GCC_VERSION)
	message("${GCC_VERSION}")
	if(GCC_VERSION VERSION_GREATER 4.7 OR GCC_VERSION VERSION_EQUAL 4.7)
		add_definitions(-std=c++11) # Enable the new C++ standard
	else()
		add_definitions(-std=c++0x) # Enable the new C++ standard
	endif()
endif()

include_directories (src)
add_executable (openotpd
	src/core/main.cpp 
	src/core/config.cpp 
	src/core/logger.cpp 
	src/core/config.h 
	src/core/global.h
	src/core/global.cpp
	src/core/logger.h
	src/core/RoleFactory.cpp
	src/core/RoleFactory.h
	src/messagedirector/MessageDirector.cpp
	src/messagedirector/MessageDirector.h
	src/messagedirector/MDNetworkParticipant.cpp
	src/messagedirector/MDNetworkParticipant.h
	src/util/Datagram.h
	src/util/DatagramIterator.h
	src/util/Role.h
	src/util/NetworkClient.cpp
	src/util/NetworkClient.h
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
