cmake_minimum_required(VERSION 2.8.9 FATAL_ERROR) # Required for FindBoost
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
project(Astron)

find_package(PythonLibs)
find_package(PythonInterp)
if(PYTHONINTERP_FOUND)
	enable_testing()
	if(PYTHON_VERSION_MAJOR EQUAL 2)
		set(PYTHON2_EXECUTABLE "${PYTHON_EXECUTABLE}")
	else()
		set(PYTHON2_EXECUTABLE python2)
	endif()
endif()

### Configure CMake to use our extra Find modules ###
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/modules/")


### Define extra windows defines ###
if(WIN32)
	add_definitions(-DBOOST_USE_WINDOWS_H)
	add_definitions(-DWIN32_LEAN_AND_MEAN)

	add_definitions(-D_WIN32_WINDOWS)
	add_definitions(-D_WIN32_WINNT=0x0600)
	add_definitions(-DNOMINMAX) # min() and max() macros conflict with <limits>
	add_definitions(-D_CRT_SECURE_NO_WARNINGS)
	add_definitions(-D_WINSOCK_DEPRECATED_NO_WARNINGS)
	if(MSVC)
		add_definitions("/wd4267") # 64-bit size_t complaints
	endif()
endif()

### Set build type to Release with Debug Info by default ###
if(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING "Build type.")
endif()
set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS
	RelWithDebInfo Release Debug)

### Define astron’s git aha for use in program ###
if(EXISTS "${CMAKE_SOURCE_DIR}/.git")
	find_package(Git)
	if(GIT_FOUND)
		exec_program(
			"${GIT_EXECUTABLE}"
			"${CMAKE_CURRENT_SOURCE_DIR}"
			ARGS "describe --abbrev=8 --dirty --always"
			OUTPUT_VARIABLE GIT_SHA_VERSION
		)

		add_definitions(-DGIT_SHA1="${GIT_SHA_VERSION}")
	endif()
endif()


### Add per-compiler flags ###
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR
   CMAKE_CXX_COMPILER_ID MATCHES "GNU")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Werror")
endif()

if(CMAKE_GENERATOR STREQUAL Xcode)
	add_definitions(-std=c++0x -stdlib=libc++)
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
	add_definitions(-std=c++11)
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
	execute_process(COMMAND "${CMAKE_C_COMPILER}" -dumpversion OUTPUT_VARIABLE GCC_VERSION)
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
if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")  ### Windows requires winsock
	list(APPEND EXTRA_LIBS wsock32 ws2_32)
endif()

if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")		### Linux requires pthreads
	list(APPEND EXTRA_LIBS pthread rt)
endif()

### YAML Depedency -- required by config, used by yaml database ###
set(YAMLCPP_USE_STATIC_LIBS ON CACHE BOOL "If true, will try to find static YamlCpp first instead of dynamic.")
find_package(YamlCpp REQUIRED)
include_directories(${YAMLCPP_INCLUDE_DIR})
link_directories(${YAMLCPP_LIBRARY_DIR})
message(STATUS "Found the following YamlCpp library:")
message("  ${YAMLCPP_LIBRARY}\n")

set(BUILD_YAML ON) # ON CACHE BOOL "Stuff"
if(BUILD_YAML)
	add_definitions(-DASTRON_WITH_YAML)
endif()

### BOOST Dependency -- required by yaml-cpp and networking ###
set(Boost_USE_STATIC_LIBS ON CACHE BOOL "If true, will try to find static Boost first instead of dynamic.")
find_package(Boost REQUIRED COMPONENTS system filesystem thread regex)
include_directories(${Boost_INCLUDE_DIR})
link_directories(${Boost_LIBRARY_DIRS})

add_definitions(
	-DBOOST_ALL_NO_LIB
	-D_SCL_SECURE_NO_WARNINGS
)

### SSL Dependency -- required by Boost::ASIO's SSL library
find_package(OpenSSL REQUIRED)
include_directories(${OPENSSL_INCLUDE_DIR})

set(USE_32BIT_DATAGRAMS OFF CACHE BOOL
	"If on, datagrams and dclass fields will use 32-bit length tags instead of 16-bit.")
if(USE_32BIT_DATAGRAMS)
	add_definitions(-DASTRON_32BIT_DATAGRAMS)
	add_definitions(-DDCLASS_32BIT_SIZETAG)
endif()
set(USE_128BIT_CHANNELS OFF CACHE BOOL
	"If on, channels will be 128-bit and doids and zones will be 64-bit (instead of 64/32).")
if(USE_128BIT_CHANNELS)
	add_definitions(-DASTRON_128BIT_CHANNELS)
endif()

set(BUILD_TESTS OFF CACHE BOOL "If set to true, test files will be compiled in")
if(BUILD_TESTS)
	set(TEST_FILES
		src/tests/MDParticipantTest.cpp
		src/tests/MDPerformanceTest.cpp
	)
endif()

# DB backends can add libraries here as needed:
set(DB_LIBRARY_NAMES)

set(BUILD_DBSERVER ON CACHE BOOL "If on, Database Server will be built into binary")
if(BUILD_DBSERVER)
	add_definitions(-DBUILD_DBSERVER)
	set(DBSERVER_FILES
		src/database/DatabaseServer.h
		src/database/DatabaseServer.cpp
		src/database/DatabaseBackend.h
		src/database/DatabaseBackend.cpp
		src/database/DBOperation.h
		src/database/DBOperation.cpp
		src/database/DBOperationQueue.h
		src/database/DBOperationQueue.cpp
		src/database/OldDatabaseBackend.h
		src/database/OldDatabaseBackend.cpp
		src/database/DBBackendFactory.h
		src/database/DBBackendFactory.cpp
	)

	set(BUILD_DB_YAML ON CACHE BOOL "If on, will support a YAML-based database (for development)")
	if(BUILD_DB_YAML)
		add_definitions(-DBUILD_DB_YAML)
		set(DBSERVER_FILES
			${DBSERVER_FILES}
			src/database/YAMLDatabase.cpp
		)
		add_test(db_yaml "${PYTHON2_EXECUTABLE}" "${CMAKE_SOURCE_DIR}/test/test_dbserver_yaml.py")
		add_test(validate_config_dbyaml "${PYTHON2_EXECUTABLE}" "${CMAKE_SOURCE_DIR}/test/test_config_dbyaml.py")
		set(PYTHON_TESTS ${PYTHON_TESTS} db_yaml validate_config_dbyaml)
	endif()

	### Check for the presence of the MongoDB client library ###
	find_package(libmongocxx QUIET)
	find_package(libbsoncxx QUIET)

	if(LIBMONGOCXX_LIBRARIES)
		set(BUILD_DB_MONGO ON CACHE BOOL "If on, will support MongoDB.")
		message(STATUS "MongoDB client library found")
	else()
		unset(BUILD_DB_MONGO CACHE)
		message(STATUS "MongoDB client library not found")
	endif()
	if(BUILD_DB_MONGO)
		list(APPEND DBSERVER_FILES
		     src/database/MongoDatabase.cpp)
		include_directories(${LIBMONGOCXX_INCLUDE_DIRS} ${LIBBSONCXX_INCLUDE_DIRS})
		link_directories(${LIBMONGOCXX_LIBRARY_DIRS} ${LIBBSONCXX_LIBRARY_DIRS})
		list(APPEND DB_LIBRARY_NAMES ${LIBMONGOCXX_LIBRARIES} ${LIBBSONCXX_LIBRARIES})

		add_test(db_mongo "${PYTHON2_EXECUTABLE}" "${CMAKE_SOURCE_DIR}/test/test_dbserver_mongodb.py")
		add_test(validate_config_dbmongo "${PYTHON2_EXECUTABLE}" "${CMAKE_SOURCE_DIR}/test/test_config_dbmongo.py")
		# TODO: Config validation test
		list(APPEND PYTHON_TESTS db_mongo)
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
			add_definitions(-DBUILD_DB_SQL)
			include_directories(${SOCI_INCLUDE_DIRS})
			link_directories(${SOCI_LIBRARY_DIR})
			set(DBSERVER_FILES
				${DBSERVER_FILES}
				src/database/SociSQLDatabase.cpp
			)
    list(APPEND DB_LIBRARY_NAMES soci_core dl)
			if(BUILD_DB_POSTGRESQL)
        list(APPEND DB_LIBRARY_NAMES pq)
				add_test(db_pgsql "${PYTHON2_EXECUTABLE}" "${CMAKE_SOURCE_DIR}/test/test_dbserver_postgres.py")
				add_test(validate_config_dbpostgres
					 "${PYTHON2_EXECUTABLE}" "${CMAKE_SOURCE_DIR}/test/test_config_dbpostgres.py")
				set(PYTHON_TESTS ${PYTHON_TESTS} db_pgsql validate_config_dbpostgres)
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
	unset(BUILD_DB_MONGO CACHE)
endif()

set(BUILD_STATESERVER ON CACHE BOOL "If on, State Server will be built into binary")
if(BUILD_STATESERVER)
	set(STATESERVER_FILES
		src/stateserver/StateServer.cpp
		src/stateserver/StateServer.h
		src/stateserver/DistributedObject.cpp
		src/stateserver/DistributedObject.h
	)
	add_test(stateserver "${PYTHON2_EXECUTABLE}" "${CMAKE_SOURCE_DIR}/test/test_stateserver.py")
	add_test(validate_config_stateserver "${PYTHON2_EXECUTABLE}" "${CMAKE_SOURCE_DIR}/test/test_config_stateserver.py")
	set(PYTHON_TESTS ${PYTHON_TESTS} stateserver validate_config_stateserver)

	set(BUILD_STATESERVER_DBSS ON CACHE BOOL "If on, the Database-State Server will be built into binary")
	if(BUILD_STATESERVER_DBSS)
		set(STATESERVER_FILES
			${STATESERVER_FILES}
			src/stateserver/DBStateServer.h
			src/stateserver/DBStateServer.cpp
			src/stateserver/LoadingObject.h
			src/stateserver/LoadingObject.cpp
		)
		add_test(dbss "${PYTHON2_EXECUTABLE}" "${CMAKE_SOURCE_DIR}/test/test_dbss.py")
		add_test(validate_config_dbss "${PYTHON2_EXECUTABLE}" "${CMAKE_SOURCE_DIR}/test/test_config_dbss.py")
		set(PYTHON_TESTS ${PYTHON_TESTS} dbss validate_config_dbss)
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
	add_test(eventlogger "${PYTHON2_EXECUTABLE}" "${CMAKE_SOURCE_DIR}/test/test_eventlogger.py")
	add_test(validate_config_eventlogger "${PYTHON2_EXECUTABLE}" "${CMAKE_SOURCE_DIR}/test/test_config_eventlogger.py")
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
	add_test(clientagent "${PYTHON2_EXECUTABLE}" "${CMAKE_SOURCE_DIR}/test/test_clientagent.py")
	add_test(validate_config_clientagent "${PYTHON2_EXECUTABLE}" "${CMAKE_SOURCE_DIR}/test/test_config_clientagent.py")
	set(PYTHON_TESTS ${PYTHON_TESTS} clientagent validate_config_clientagent)
endif()

### Define core components ###
set(CORE_FILES
	src/core/global.h
	src/core/global.cpp
	src/core/main.cpp
	src/core/Logger.h
	src/core/Logger.cpp
	src/core/Role.h
	src/core/Role.cpp
	src/core/RoleFactory.h
	src/core/RoleFactory.cpp
	src/core/shutdown.h
	src/core/shutdown.cpp
	src/core/types.h
	src/core/msgtypes.h
)

set(CONFIG_FILES
	src/config/ConfigGroup.h
	src/config/ConfigGroup.cpp
	src/config/ConfigVariable.h
	src/config/ConfigVariable.cpp
	src/config/constraints.cpp
)

set(MESSAGEDIRECTOR_FILES
	src/messagedirector/ChannelMap.cpp
	src/messagedirector/ChannelMap.h
	src/messagedirector/MessageDirector.cpp
	src/messagedirector/MessageDirector.h
	src/messagedirector/MDNetworkParticipant.cpp
	src/messagedirector/MDNetworkParticipant.h
	src/messagedirector/MDNetworkUpstream.cpp
	src/messagedirector/MDNetworkUpstream.h
)

set(UTIL_FILES
	src/util/Datagram.h
	src/util/DatagramIterator.h
	src/util/EventSender.cpp
	src/util/EventSender.h
	src/util/password_prompt.cpp
	src/util/password_prompt.h
	src/util/Timeout.cpp
	src/util/Timeout.h
)

set(NET_FILES
	src/net/address_utils.cpp
	src/net/address_utils.h
	src/net/HAProxyHandler.cpp
	src/net/HAProxyHandler.h
	src/net/NetworkAcceptor.cpp
	src/net/NetworkAcceptor.h
	src/net/NetworkClient.cpp
	src/net/NetworkClient.h
	src/net/NetworkConnector.cpp
	src/net/NetworkConnector.h
	src/net/TcpAcceptor.cpp
	src/net/TcpAcceptor.h
	src/net/SslAcceptor.cpp
	src/net/SslAcceptor.h
)

include_directories(src)
add_test(validate_config_core "${PYTHON2_EXECUTABLE}" "${CMAKE_SOURCE_DIR}/test/test_config.py")
add_test(messagedirector "${PYTHON2_EXECUTABLE}" "${CMAKE_SOURCE_DIR}/test/test_messagedirector.py")
set(PYTHON_TESTS ${PYTHON_TESTS} messagedirector)
add_executable(astrond
	${CORE_FILES}
	${CONFIG_FILES}
	${MESSAGEDIRECTOR_FILES}
	${UTIL_FILES}
  ${NET_FILES}
	${STATESERVER_FILES}
	${EVENTLOGGER_FILES}
	${CLIENTAGENT_FILES}
	${TEST_FILES}
	${DBSERVER_FILES}
)

### Setup source groups and defines for configurables ###
source_group("Core" FILES ${CORE_FILES})
source_group("Config" FILES ${CONFIG_FILES})
source_group("Util" FILES ${UTIL_FILES})
source_group("MessageDirector" FILES ${MESSAGEDIRECTOR_FILES})
source_group("Net" FILES ${NET_FILES})
if(STATESERVER_FILES)
	add_definitions(-DBUILD_STATESERVER)
	source_group("StateServer" FILES ${STATESERVER_FILES})
endif()
if(EVENTLOGGER_FILES)
	add_definitions(-DBUILD_EVENTLOGGER)
	source_group("EventLogger" FILES ${EVENTLOGGER_FILES})
endif()
if(CLIENTAGENT_FILES)
	add_definitions(-DBUILD_CLIENTAGENT)
	source_group("ClientAgent" FILES ${CLIENTAGENT_FILES})
endif()
if(DBSERVER_FILES)
	add_definitions(-DBUILD_STATESERVER_DBSS)
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

add_subdirectory(src/dclass)

add_dependencies(astrond dclass)
target_link_libraries(astrond dclass ${OPENSSL_LIBRARIES} ${YAMLCPP_LIBRARY} ${DB_LIBRARY_NAMES} ${Boost_LIBRARIES} ${EXTRA_LIBS})

### Handle some final testing configuration ###
if(USE_32BIT_DATAGRAMS)
	set(PYTHON_TEST_ENV ${PYTHON_TEST_ENV} "USE_32BIT_DATAGRAMS=true")
endif()
if(USE_128BIT_CHANNELS)
	set(PYTHON_TEST_ENV ${PYTHON_TEST_ENV} "USE_128BIT_CHANNELS=true")
endif()
if(PYTHON_TEST_ENV)
	set_tests_properties(${PYTHON_TESTS} PROPERTIES ENVIRONMENT "${PYTHON_TEST_ENV}")
endif()
