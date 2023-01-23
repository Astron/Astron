#!/bin/bash
# NOTE: Run from repository root directory!

cat > ./.git/info/exclude << EOF
## Compilation Artifacts ##
astrond
*.[oa]
CMakeCache.txt
CMakeFiles/
CTestTestfile.cmake
cmake_install.cmake
Makefile
EOF

BUILD_TYPE=$1
if [[ -z $BUILD_TYPE ]];
then
    echo "No parameter passed; Building Astron release."
    cmake -DCMAKE_BUILD_TYPE=Release . && make
else
    if [ $BUILD_TYPE == "release" ]; then
        cmake -DCMAKE_BUILD_TYPE=Release . && make
	exit 0
    fi
    if [ $BUILD_TYPE == "debug" ]; then
        cmake -DCMAKE_BUILD_TYPE=Release . && make
	exit 0
    fi
    if [ $BUILD_TYPE == "dev" ]; then
        cmake . && make
    fi
    echo "Invalid build type parameter passed. ('release', 'debug', 'dev')"
fi
