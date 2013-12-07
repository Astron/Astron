cmake_minimum_required(VERSION 2.6)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
project(Astron)
enable_testing()

### Configure CMake to use our extra Find modules ###
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/modules/")


### Define extra windows defines ###
if(WIN32)
	add_definitions(-D_WIN32_WINDOWS)
endif()


### Set build type to Release with Debug Info by default ###
if(NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE RelWithDebInfo)
endif()



### Add per-compiler flags ###
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR
   CMAKE_CXX_COMPILER_ID MATCHES "GNU")

	### Release with Debugging Info
	set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} -O2 -g")

	### Release Flags -- warnings are errors, release code should not have warnings ###
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O2 -Wall -Werror")
	
	### Debug flags, Wall
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall")
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
	add_definitions(-std=c++11)
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
	execute_process(COMMAND ${CMAKE_C_COMPILER} -dumpversion OUTPUT_VARIABLE GCC_VERSION)
	message(STATUS "GCC Version: ${GCC_VERSION}")
	if(GCC_VERSION VERSION_GREATER 4.7 OR GCC_VERSION VERSION_EQUAL 4.7)
		add_definitions(-std=c++11) # Enable the new C++ standard
	else()
		add_definitions(-std=c++0x) # Enable the new C++ standard
	endif()
endif()


### Debug Flags -- compile in spam/debug log messages ###
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DASTRON_DEBUG_MESSAGES")


### OS Specific Settings ###
if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")		### Set Yaml libary name for Windows
	set(YAMLCPP_LIBNAME "libyaml-cppmd" CACHE STRING "Name of YAML library")
	set(YAMLCPP_LIBNAME optimized ${YAMLCPP_LIBNAME} debug ${YAMLCPP_LIBNAME}d)

else()											### Set Yaml libary name for Unix, Linux, OS X, etc
	set(YAMLCPP_LIBNAME "yaml-cpp" CACHE STRING "Name of YAML library")
endif()

if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")		### Linux requires pthreads
	set(CMAKE_CXX_FLAGS "-pthread")
endif()

### YAML Depedency -- required by config, used by yaml database ###
set(YAMLCPP_USE_STATIC_LIBS ON CACHE BOOL "If true, will try to find static YamlCpp first instead of dynamic.")
find_package(YamlCpp)
if(NOT YAMLCPP_FOUND)
	set(YAMLCPP_USE_STATIC_LIBS OFF CACHE BOOL "If true, will try to find static YamlCpp first instead of dynamic.")
	find_package(YamlCpp)
endif()
if(YAMLCPP_FOUND)
	include_directories(${YAMLCPP_INCLUDE_DIR})
	link_directories(${YAMLCPP_LIBRARY_DIR})
	message(STATUS "Found the following YamlCpp library:")
	message("  ${YAMLCPP_LIBRARY}\n")
else()
	message(FATAL_ERROR "You need yamlcpp to build this, build yamlcpp in the 'dependencies/' directory, or install it as a system package")
endif()


### BOOST Dependency -- required by yaml-cpp and networking ###
set(Boost_USE_STATIC_LIBS ON CACHE BOOL "If true, will try to find static Boost first instead of dynamic.")
find_package(Boost COMPONENTS system filesystem)
if(NOT Boost_FOUND)
	unset(Boost_USE_STATIC_LIBS CACHE)
	set (Boost_USE_STATIC_LIBS OFF CACHE BOOL "If true, will try to find static Boost first instead of dynamic.")
	find_package(Boost COMPONENTS system filesystem)
	if(NOT Boost_FOUND)
		unset(Boost_USE_STATIC_LIBS CACHE)
	endif()
endif()
if(Boost_FOUND)
	include_directories(${Boost_INCLUDE_DIR})
	link_directories(${Boost_LIBRARY_DIRS})
	message("  ${Boost_LIBRARIES}\n")
else()
	message(FATAL_ERROR "You need boost to build this, set the BOOST_ROOT or BOOSTROOT env variables, or pass them to cmake")
endif()

add_definitions(
	-DBOOST_ALL_NO_LIB
	-D_SCL_SECURE_NO_WARNINGS
)

set(BUILD_TESTS OFF CACHE BOOL "If set to true, test files will be compiled in")

if(BUILD_TESTS)
	set(TEST_FILES
		src/tests/MDParticipantTest.cpp
		src/tests/MDPerformanceTest.cpp
	)
endif()

set(BUILD_DBSERVER ON CACHE BOOL "If on, Database Server will be built into binary")
if(BUILD_DBSERVER)
	set(DBSERVER_FILES
		src/database/DatabaseServer.cpp
		src/database/DatabaseBackend.h
		src/database/DBBackendFactory.h
		src/database/DBBackendFactory.cpp
	)

	set(BUILD_DB_YAML ON CACHE BOOL "If on, will support a YAML-based database (for development)")
	if(BUILD_DB_YAML)
		set(DBSERVER_FILES
			${DBSERVER_FILES}
			src/database/YAMLDatabase.cpp
		)
		add_test(db_yaml ${CMAKE_SOURCE_DIR}/test/test_dbserver_yaml.py)
	endif()

	### Check for soci and if available, compile SQL database support ###
	find_package(Soci COMPONENTS postgresql mysql sqlite3)

	if(SOCI_FOUND)
		message(STATUS "Found the following Soci library:")
		message("  ${SOCI_LIBRARY}\n")

		message(STATUS "Found the following Soci plugins:")
		### PostgreSQL support ##
		if(SOCI_postgresql_FOUND)
			set(BUILD_DB_POSTGRESQL ON CACHE BOOL "If on, will support PostgreSQL.")
			message(STATUS "  PostgreSQL found")
			message("    ${SOCI_postgresql_PLUGIN}")
		else()
			unset(BUILD_DB_POSTGRESQL CACHE)
			message(STATUS "  PostgreSQL not found")
			message("    won't add PostgreSQl support")
		endif()

		### MySQL support ###
		if(SOCI_mysql_FOUND)
			set(BUILD_DB_MYSQL ON CACHE BOOL "If on, will support MySQL.")
			message(STATUS "  MySQL found")
			message("    ${SOCI_mysql_PLUGIN}")
		else()
			unset(BUILD_DB_MYSQL CACHE)
			message(STATUS "  MySQL not found")
			message("    won't add MySQL support")
		endif()

		### SQLite3 support ###
		if(SOCI_sqlite3_FOUND)
			set(BUILD_DB_SQLITE ON CACHE BOOL "If on, will support SQLite3.")
			message(STATUS "  SQLite3 found")
			message("    ${SOCI_sqlite3_PLUGIN}")
		else()
			unset(BUILD_DB_SQLITE CACHE)
			message(STATUS "  SQLite3 not found")
			message("    won't add SQLite3 support")
		endif()

		message("")

		### Add files to compiler and linker ###
		if(BUILD_DB_SQLITE OR BUILD_DB_MYSQL OR BUILD_DB_POSTGRESQL)
			include_directories(${SOCI_INCLUDE_DIRS})
			link_directories(${SOCI_LIBRARY_DIR})
			set(DBSERVER_FILES
				${DBSERVER_FILES}
				src/database/SociSQLDatabase.cpp
			)
			set(SOCI_LIBRARY_NAMES soci_core dl)
			if(BUILD_DB_POSTGRESQL)
				set(SOCI_LIBRARY_NAMES ${SOCI_LIBRARY_NAMES} pq)
				add_test(db_pgsql ${CMAKE_SOURCE_DIR}/test/test_dbserver_posgres.py)

			endif()
		endif()
	else()
		message(STATUS "Soci not found")
		message("  won't add SQL support")
		unset(BUILD_DB_MYSQL CACHE)
		unset(BUILD_DB_POSTGRESQL CACHE)
		unset(BUILD_DB_SQLITE CACHE)
	endif()
else()
	unset(BUILD_DB_FILESYSTEM CACHE)
	unset(BUILD_DB_YAML CACHE)
	unset(BUILD_DB_MYSQL CACHE)
	unset(BUILD_DB_POSTGRESQL CACHE)
	unset(BUILD_DB_SQLITE CACHE)
endif()

set(BUILD_STATESERVER ON CACHE BOOL "If on, State Server will be built into binary")
if(BUILD_STATESERVER)
	set(STATESERVER_FILES
		src/stateserver/StateServer.cpp
		src/stateserver/StateServer.h
		src/stateserver/DistributedObject.cpp
		src/stateserver/DistributedObject.h
	)
	add_test(stateserver ${CMAKE_SOURCE_DIR}/test/test_stateserver.py)

	set(BUILD_STATESERVER_DBSS ON CACHE BOOL "If on, the Database-State Server will be built into binary")
	if(BUILD_STATESERVER_DBSS)
		set(STATESERVER_FILES
			${STATESERVER_FILES}
			src/stateserver/DBStateServer.h
			src/stateserver/DBStateServer.cpp
			src/stateserver/LoadingObject.h
			src/stateserver/LoadingObject.cpp
		)
		add_test(dbss ${CMAKE_SOURCE_DIR}/test/test_dbss.py)
	endif()
else()
	unset(BUILD_STATESERVER_DBSS CACHE)
endif()

set(BUILD_EVENTLOGGER ON CACHE BOOL "If on, Event Logger will be built into binary")
if(BUILD_EVENTLOGGER)
	set(EVENTLOGGER_FILES
		src/eventlogger/EventLogger.cpp
		src/eventlogger/EventLogger.h
	)
endif()

set(BUILD_CLIENTAGENT ON CACHE BOOL "If on, Client Agent will be built into binary")
if(BUILD_CLIENTAGENT)
	set(CLIENTAGENT_FILES
		src/clientagent/Client.h
		src/clientagent/Client.cpp
		src/clientagent/ClientAgent.h
		src/clientagent/ClientAgent.cpp
		src/clientagent/ClientMessages.h
		src/clientagent/ClientFactory.cpp
		src/clientagent/ClientFactory.h
		src/clientagent/AstronClient.cpp
	)
	add_test(clientagent ${CMAKE_SOURCE_DIR}/test/test_clientagent.py)
endif()

set(USE_32BIT_DATAGRAMS OFF CACHE BOOL "If on, datagrams and dclass fields will use 32-bit length tags instead of 16-bit.")
if(USE_32BIT_DATAGRAMS)
	add_definitions(-DASTRON_32BIT_DATAGRAMS)
	add_definitions(-DDCPARSER_32BIT_LENGTH_TAG)
endif()


### Define core components ###
set(CORE_FILES
	src/core/main.cpp
	src/core/global.h
	src/core/global.cpp
	src/core/config.cpp
	src/core/config.h
	src/core/Logger.h
	src/core/Logger.cpp
	src/core/Role.h
	src/core/RoleFactory.cpp
	src/core/RoleFactory.h
	src/core/types.h
	src/core/msgtypes.h
)

set(MESSAGEDIRECTOR_FILES
	src/messagedirector/MessageDirector.cpp
	src/messagedirector/MessageDirector.h
	src/messagedirector/MDNetworkParticipant.cpp
	src/messagedirector/MDNetworkParticipant.h
)

set(UTIL_FILES
	src/util/Datagram.h
	src/util/DatagramIterator.h
	src/util/NetworkClient.cpp
	src/util/NetworkClient.h
	src/util/EventSender.cpp
	src/util/EventSender.h
)

include_directories(src)
add_test(messagedirector ${CMAKE_SOURCE_DIR}/test/test_messagedirector.py)
add_executable(astrond
	${CORE_FILES}
	${MESSAGEDIRECTOR_FILES}
	${UTIL_FILES}
	${STATESERVER_FILES}
	${EVENTLOGGER_FILES}
	${CLIENTAGENT_FILES}
	${TEST_FILES}
	${DBSERVER_FILES}
)

### Setup source groups ###
source_group("Core" FILES ${CORE_FILES})
source_group("Util" FILES ${UTIL_FILES})
source_group("MessageDirector" FILES ${MESSAGEDIRECTOR_FILES})
if(STATESERVER_FILES)
	source_group("StateServer" FILES ${STATESERVER_FILES})
endif()
if(EVENTLOGGER_FILES)
	source_group("EventLogger" FILES ${EVENTLOGGER_FILES})
endif()
if(CLIENTAGENT_FILES)
	source_group("ClientAgent" FILES ${CLIENTAGENT_FILES})
endif()
if(DBSERVER_FILES)
	source_group("DatabaseServer" FILES ${DBSERVER_FILES})
endif()
if(TEST_FILES)
	source_group("DaemonTests" FILES ${TEST_FILES})
endif()

### Header source group ###
#foreach(sourcefile IN LISTS ASTROND_SOURCES)
#	if(sourcefile MATCHES .*\\.h)
#		set(ASTROND_HEADERS "${ASTROND_HEADERS} ${sourcefile}")
#	endif()
#endforeach(sourcefile)
#source_group("Header Files" FILES ${ASTROND_HEADERS})

add_library(dcparser
	src/dcparser/p3dcparser_composite1.cxx 
	src/dcparser/p3dcparser_composite2.cxx 
	src/dcparser/dcLexer.cxx
	src/dcparser/dcParser.cxx
)

add_dependencies(astrond dcparser)
target_link_libraries(astrond dcparser ${YAMLCPP_LIBNAME} ${Boost_LIBRARIES} ${SOCI_LIBRARY_NAMES})
