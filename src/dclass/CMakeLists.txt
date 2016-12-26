cmake_minimum_required(VERSION 2.8)
project(DistributedClasses)
include_directories(.)

find_package(BISON)
if(BISON_FOUND)
    add_custom_command(
      SOURCE ${CMAKE_SOURCE_DIR}/src/dclass/parser.ypp
      COMMAND ${BISON_EXECUTABLE}
      ARGS --defines=${CMAKE_SOURCE_DIR}/src/dclass/parser.h
           -o ${CMAKE_SOURCE_DIR}/src/dclass/parser.cpp
           ${CMAKE_SOURCE_DIR}/src/dclass/parser.ypp
      COMMENT "Generating dclass/parser.cpp"
      OUTPUT ${CMAKE_SOURCE_DIR}/src/dclass/parser.cpp
    )
endif(BISON_FOUND)

find_package(FLEX)
if(FLEX_FOUND)
    add_custom_command(
      SOURCE ${CMAKE_SOURCE_DIR}/src/dclass/lexer.lpp
      COMMAND ${FLEX_EXECUTABLE}
      ARGS -o${CMAKE_SOURCE_DIR}/src/dclass/lexer.cpp
           ${CMAKE_SOURCE_DIR}/src/dclass/lexer.lpp
      COMMENT "Generating dclass/lexer.cpp"
      OUTPUT ${CMAKE_SOURCE_DIR}/src/dclass/lexer.cpp
    )
endif(FLEX_FOUND)

add_library(dclass
	# Helper Classes
	util/PrimeNumberGenerator.cpp
	util/HashGenerator.cpp

	# Distributed Object Model
	dc/DistributedType.cpp
	dc/NumericType.cpp
	dc/ArrayType.cpp
	dc/Struct.cpp
	dc/Class.cpp
	dc/Method.cpp

	dc/KeywordList.cpp
	dc/File.cpp
	dc/Field.cpp
	dc/MolecularField.cpp
	dc/Parameter.cpp

	# File and Network I/O
	file/parser.cpp
	file/lexer.cpp
	file/write.cpp
	file/read.cpp
	file/hash_legacy.cpp
	value/format.cpp
	value/parse.cpp
	value/default.cpp
)
set_target_properties(dclass PROPERTIES LINK_FLAGS -lfl)

# Workaround for Flex bug #140:
set_source_files_properties(file/lexer.cpp PROPERTIES COMPILE_FLAGS -Wno-sign-compare)
