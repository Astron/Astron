# OpenOTP C++ Style Guide
Failing to adhere to this guide will result in a paddling and your
pull-request being instantly rejected. Feel free to fix the issues
and re-submit the pull request though.
- - -
## Whitespace
Tabs shall be used to indent, spaces shall be used to align.

## Variable Names
Variables shall have a descriptive lowercase name, where words are seperated by underscores.
Member variables shall start with m_

Example: `field_count` or

	class Foo
	{
		private:
			int m_my_number;
	};

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
The `auto` keyword shall be used to avoid typing out long iterator types, and only for that.

Example:

	std::list<DistributedObjects> my_objects;
	auto it = my_objects.begin();

## Template specifiers
There shall be no space between the identifier and the <>

Good example: `std::list<channel_t> my_channels;`

Bad example: `std::list <channel_t> my_channels;`

## Access specifiers
Class/struct access specifiers shall be indented.

Good example:

	class Foo
	{
		public:
			Foo();
	};

Bad examples:

	class Foo
	{
	public:
		Foo();
	};
	
	class Foo
	{
		public:
		Foo();
	};

## Switches
Case statements inside switches shall be indented. If the case does not fall through, it shall have it's own scope.
The braces for the scope shall be on the same indentation level as the case statement itself.
At the end of the scope, a `break;` shall be placed on the same indentation level as the case statement.

Example:

	int temp = some_int;
	switch(temp)
	{
		case 0:
		{
			//code
		}
		break;
		default:
			//code
	}

## Character Limit
Each line shall be no longer than 100 characters, each tab counting as 4 characters.
This is not a hard-limit, exceptions may be made.

## Initializer lists
The first variable inside of the initializer list shall be on the same line as the function header
if the character limit allows.
The following variables will be on the next line, with another level of indentation. 
That line will continue until it hits the character limit, once that occurs a new line will be created,
with the same level of indenation.

Example:

	class Foo
	{
		private:
			int m_number;
			int m_number2;
			
		public:
			Foo() : m_number(0),
				m_number2(0)
			{
			}
	};