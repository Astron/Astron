cmake_minimum_required (VERSION 2.6)
set_property ( GLOBAL PROPERTY USE_FOLDERS ON)
project (OOTP)

set (P3D_DCPARSER_DIR "dependencies/panda3d/direct/src/dcparser" CACHE PATH "Path to dcparser source")
include_directories (${CMAKE_CURRENT_LIST_DIR}/${P3D_DCPARSER_DIR})
set (P3D_DTOOLBASE_DIR "dependencies/panda3d/dtool/src/dtoolbase" CACHE PATH "Path to dtoolbase source")
include_directories (${CMAKE_CURRENT_LIST_DIR}/${P3D_DTOOLBASE_DIR})

set (YAMLCPP_INCLUDES "dependencies/yaml-cpp-0.5.1/include" CACHE PATH "Path to yamlcpp includes")
set (YAMLCPP_LIBRARIES "dependencies/yaml-cpp-0.5.1/lib" CACHE PATH "Path to yamlcpp libs")
include_directories (${CMAKE_CURRENT_LIST_DIR}/${YAMLCPP_INCLUDES})
link_directories (${CMAKE_CURRENT_LIST_DIR}/${YAMLCPP_LIBRARIES})

#fucking yamlcpp needs boost
set (BOOST_INCLUDES "FILLME" CACHE PATH "Path to boost includes folder")
include_directories (${BOOST_INCLUDES})

add_executable (core src/core/main.cpp src/core/config.cpp src/core/logger.cpp src/core/config.h src/core/global.h src/core/logger.h)
source_group ("Header Files" FILES src/core/config.h src/core/global.h src/core/logger.h)

add_library (MessageDirector src/messagedirector/messagedirector.cpp src/messagedirector/messagedirector.h)
source_group ("Header Files" FILES src/messagedirector/messagedirector.h)

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

add_dependencies (dcparser dtoolbase)
target_link_libraries (dcparser dtoolbase)
add_dependencies (core MessageDirector dcparser)
target_link_libraries (core MessageDirector dcparser libyaml-cppmdd)