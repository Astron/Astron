cmake_minimum_required (VERSION 2.6)
set_property ( GLOBAL PROPERTY USE_FOLDERS ON)
project (OOTP)

if (WIN32)
	add_definitions(-D_WIN32_WINDOWS)
endif()

set (P3D_DIR "dependencies/panda3d" CACHE PATH "Path to Panda3D source")

set (P3D_DCPARSER_DIR ${P3D_DIR}/direct/src/dcparser CACHE PATH "Path to dcparser source")
include_directories (${CMAKE_CURRENT_LIST_DIR}/${P3D_DCPARSER_DIR})
set (P3D_DTOOLBASE_DIR ${P3D_DIR}/dtool/src/dtoolbase CACHE PATH "Path to dtoolbase source")
include_directories (${CMAKE_CURRENT_LIST_DIR}/${P3D_DTOOLBASE_DIR})
set (P3D_PANDAEXPRESS_DIR ${P3D_DIR}/panda/src/express CACHE PATH "Path to pandaexpress source")
include_directories (${CMAKE_CURRENT_LIST_DIR}/${P3D_PANDAEXPRESS_DIR})
include_directories (${CMAKE_CURRENT_LIST_DIR}/${P3D_DIR}/panda/src/pandabase)
include_directories (${CMAKE_CURRENT_LIST_DIR}/${P3D_DIR}/panda/src/express)
include_directories (${CMAKE_CURRENT_LIST_DIR}/${P3D_DIR}/dtool/src/prc)
include_directories (${CMAKE_CURRENT_LIST_DIR}/${P3D_DIR}/dtool/src/dtoolutil)
include_directories (${CMAKE_CURRENT_LIST_DIR}/${P3D_DIR}/dtool/src/dconfig)
include_directories (${CMAKE_CURRENT_LIST_DIR}/${P3D_DIR}/dtool/src/interrogatedb)

set (YAMLCPP_INCLUDES "dependencies/yaml-cpp-0.5.1/include" CACHE PATH "Path to yamlcpp includes")
set (YAMLCPP_LIBRARIES "dependencies/yaml-cpp-0.5.1/lib" CACHE PATH "Path to yamlcpp libs")
include_directories (${CMAKE_CURRENT_LIST_DIR}/${YAMLCPP_INCLUDES})
link_directories (${CMAKE_CURRENT_LIST_DIR}/${YAMLCPP_LIBRARIES})

#fucking yamlcpp needs boost
set (BOOST_INCLUDES "FILLME" CACHE PATH "Path to boost includes folder")
include_directories (${BOOST_INCLUDES})
set (BOOST_LIBS "FILLME" CACHE PATH "Path to boost includes folder")
link_directories (${BOOST_LIBS})

add_executable (core src/core/main.cpp src/core/config.cpp src/core/logger.cpp src/core/config.h src/core/global.h src/core/logger.h
 src/messagedirector/messagedirector.cpp src/messagedirector/messagedirector.h)
source_group ("Header Files" FILES src/core/config.h src/core/global.h src/core/logger.h src/messagedirector/messagedirector.h)

add_library (dcparser 
	${P3D_DCPARSER_DIR}/p3dcparser_composite1.cxx 
	${P3D_DCPARSER_DIR}/p3dcparser_composite2.cxx 
	${P3D_DCPARSER_DIR}/dcLexer.cxx.prebuilt
	${P3D_DCPARSER_DIR}/dcParser.cxx.prebuilt
)
set_source_files_properties (${P3D_DCPARSER_DIR}/dcLexer.cxx.prebuilt ${P3D_DCPARSER_DIR}/dcParser.cxx.prebuilt PROPERTIES LANGUAGE CXX)

add_library (dtoolbase 
	${P3D_DTOOLBASE_DIR}/p3dtoolbase_composite1.cxx 
	${P3D_DTOOLBASE_DIR}/p3dtoolbase_composite2.cxx 
	${P3D_DTOOLBASE_DIR}/lookup3.c
	${P3D_DTOOLBASE_DIR}/dtool_config.h
)
add_definitions (-DLINK_ALL_STATIC)

add_library (pandaexpress
	${P3D_PANDAEXPRESS_DIR}/p3express_composite1.cxx
	${P3D_PANDAEXPRESS_DIR}/p3express_composite2.cxx
)

add_dependencies (dcparser dtoolbase)
target_link_libraries (dcparser dtoolbase)
add_dependencies (core dcparser)
target_link_libraries (core dcparser libyaml-cppmdd)