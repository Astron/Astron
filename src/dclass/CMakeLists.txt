cmake_minimum_required(VERSION 2.8)
project(DistributedClasses)
add_library(dclass
	HashGenerator.cpp

	DistributedType.cpp
	NumericType.cpp
	ArrayType.cpp
	StructType.cpp
	MethodType.cpp

	File.cpp
	Class.cpp

	KeywordList.cpp
	Parameter.cpp
	Field.cpp
	MolecularField.cpp

	file/Lexer.cpp
	file/Parser.cpp
	value/format.cpp
	value/parse.cpp
)
