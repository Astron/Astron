Astron C++ Style Guide
----------------------

Pull requests will not be merged until styling issues have been fixed.  
Pull requests with bad style are less likely to be accepted.


## Formatting

### Line Length
Each line of text should be at most 100 characters long.

### Indentation
Use only spaces, not tabs.  Indent with 4 spaces at a time.  
The contents of namespaces are not indented.  

Class/struct access specifiers should be indented 2 spaces.

    class Foo
    {
      public:
        Foo();
    };

Case statements should be at the same indentation as their switch, with indented logic.

    switch(day) {
    case Monday:
        return 1;
    case Tuesday:
        return 2;
    default:
        return 0;
    }

### Braces
Braces for control statements should be attached.  
Class and function braces should be on their own line (unless its for an empty block).  
Control statements should use consistent braces (ie. if the `if` uses braces so should the `else`.)

### Whitespace
There should be no space between a conditional and its condition (`if(foo)`).  
There should be a space between a control statement and its brace (`switch(val) {`).  
There should be no space between template specifiers and their template arguments.  
Nested template specifiers should have spaces between right angle-brackets.  
Prefer no spaces inside parentheses (`while(true)` vs `while( true )`).  

### Newlines
Source files should end with a newline.  
Short conditional statements may be written on one line if this enhances readability.  
When declaring nested namespaces, put each namespace on its own line.  
If statements should be broken before the logical operator.  


## Naming

### Variable Names
Variables should have a descriptive lowercase name, where words are seperated by underscores.  
Member variables should start with `m_`.  Global variables should start with `g_`.  

Example: `field_count` or

    class Foo
    {
      private:
        int m_my_number;
    };

### Function Names
Functions should have a descriptive lowercase name, where words are seperated by underscores.

### Class Names
Class names should be in CamelCase

Example: `DistributedObject`

### Typedefs
Typedefs should have a descriptive lower-case name, ending with `_t`.  
Non-standard/very-complicated typedefs should moonlight with class names.  

Example: `typedef unsigned int uint32_t`  
Example: `typedef std::unordered_set<const Foo*, CompareFooPtr> FooSet`  


## Header Files

### Header Guards
Astron uses `#pragma once` as a header guard.  All headers should have this line as the first
non-comment line in the file.  Guards **SHOULD NOT** use the `#ifndef` and `#define` semantics.

### Inline Functions
Functions should only be defined inline when they are small (~10 lines or less).

### Function Parameter Ordering
Functions parameters should be ordered with inputs preceding outputs.

### Names and Order of Includes
Include paths should include the full path from the `src/` directory, with the exception of the
header file corresponding to this file (ie. `foo.cxx` should use `#include "foo.h"`).
Internal includes should use quotations and external includes should use angle brackets.

Include order:
1. Header file corresponding to this source file (if exists)
2. C system files
3. C++ system files
4. External headers
5. Internal headers (ordered by logical dependency)


## Miscellaneous

### Function Arguments
Types from stl should be passed by constant reference `const ref&` whenever possible.

### Local Variables
Initialize local variables as soon as they are declared.  
Declare local variables as close to the first use as possible.  

### Member Variables
Simple member variables should use in-class member initializations.  
More complex member variables and variables set by arguments should be initialized in a constructor.  

### Explicit Constructors
Use the C++ keyword `explicit` for constructors with one argument.  
If a conversion is intended, it should be clearly marked with comments.  

### Copy Constructors
Disable copy and assignment operators with `= delete;`.  
If a copy constructor is necessary, provide both a copy constructor and assignment operator.  

### Operator Overloading
When implementing comparison operators, provide an implementation for all six of them.  
Comparison operators should be declared outside of a class (as non-member functions).  
Don't overload other operators -- if you need to, follow normal C++ semantics.  

### Iterators
Use prefix form (`++i`) of the increment and decrement operators with iterators.  
Complex iterator types should use the `auto` keyword. Never use `auto` for anything else.  

### Integer Types
Of the built-in C++ integer types, only use `int` and `unsigned int`.  
If a variable is always of a specific size, use precise-width integers.  

### Null Values
Use `nullptr` instead of `NULL`.
