cmake_minimum_required(VERSION 2.8)
project(DistributedClasses)
include_directories(.)
include_directories(CMAKE_BINARY_DIR)

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
	HashGenerator.cpp

	DistributedType.cpp
	NumericType.cpp
	ArrayType.cpp
	Struct.cpp
	Method.cpp

	File.cpp
	Class.cpp

	KeywordList.cpp
	Parameter.cpp
	Field.cpp
	MolecularField.cpp

	file/parser.cpp
	file/lexer.cpp
	file/write.cpp
	file/read.cpp
	value/format.cpp
	value/parse.cpp
)
SET_TARGET_PROPERTIES(dclass PROPERTIES LINK_FLAGS -lfl)