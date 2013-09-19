# OpenOTP C++ Style Guide
- - -
## Whitespace
Tabs shall be used to indent, spaces shall be used to align.

## Variable Names
Variables shall have a descriptive lowercase name, where words are seperated by underscores.

Example: `field_count`

## Function Names
Functions shall be named the same way as variables

## Class Names
Class names shall be in CamelCase

Example: `DistributedObject`

## Braces
Each brace shall be on it's own line, even if it's for an empty member:

Example:

	void foo()
	{
	}

## Header Files
A class shall not have a header file if nothing else interacts with it or if nothing else will ever inherit from it. 
See DBServer for an example of this case

## Typedefs
Typedefs shall have a descriptive name and end with _t

Example: `typedef unsigned int uint32_t`

## Usage of std::string
Whenever possible, have function parameters be of type `const std::string&` and not `std::string`

Example:

	void foo(const std::string &name)
	{
		std::cout << "Hello " << name << "!" << std::endl;
	}