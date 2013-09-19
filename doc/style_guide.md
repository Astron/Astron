# OpenOTP C++ Style Guide
Failing to adhere to this guide will result in a paddling and your
pull-request being instantlyrejected. Feel free to fix the issues
and re-submit the pull request though.
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
	
## Preprocessor Macros
Preproc macros shall be UPPER CASE and words shall be seperated by underlines.

Example: `#define CLIENT_HELLO 1`

## auto Keyword
The `auto` keyword shall be used to avoid typing out long-ass iterator types, and only for that.

Example:

	std::list<DistributedObjects> my_objects;
	auto it = my_objects.begin();

## Template specifiers
There shall be no space between the identifier and the <>

Good example: `std::list<channel_t> my_channels;`

Bad example: `std::list <channel_t> my_channels;`