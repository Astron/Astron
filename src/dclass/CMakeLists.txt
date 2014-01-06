cmake_minimum_required(VERSION 2.8)
project(DistributedClasses)
include_directories(.)
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

	file/lexer.cpp
	file/parser.cpp
	file/write.cpp
	value/format.cpp
	value/parse.cpp
)
