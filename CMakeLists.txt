cmake_minimum_required (VERSION 2.6)
set_property ( GLOBAL PROPERTY USE_FOLDERS ON)
project (OOTP)

set (PANDA3D_INCLUDES "FILLME" CACHE PATH "Path to p3d includes directory")
set (PANDA3D_LIBRARIES "FILLME" CACHE PATH "Path to p3d libraries folder")
include_directories (${PANDA3D_INCLUDES})
link_directories (${PANDA3D_LIBRARIES})

add_executable (core src/core/main.cpp src/core/config.cpp src/core/logger.cpp src/core/config.h src/core/global.h src/core/logger.h)
source_group ("Header Files" FILES src/core/config.h src/core/global.h src/core/logger.h)

add_library (MessageDirector src/messagedirector/messagedirector.cpp src/messagedirector/messagedirector.h)
source_group ("Header Files" FILES src/messagedirector/messagedirector.h)

add_dependencies (core MessageDirector)
target_link_libraries (core MessageDirector libp3direct)