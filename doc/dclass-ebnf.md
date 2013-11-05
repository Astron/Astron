DistributedClass Language Specification
---------------------------------------
**Author:** Kevin "Kestred" Stenerson  
**Version:** 2013, November 2nd

### Introduction ###
This is a reference manual for the DistributedClass protocol-specification language.

The dc language provides a syntax for specifying a networked protocol based on object-oriented design
principles.  In essence, a distributed class specifies an "interface" for an object that is "shared"
between multiple processes or "exposed" in a remote process.

The object satisfying this interface is referred to as a "distributed object".



### Notation ###
The syntax is specified using Extended Backus-Naur Form (EBNF):

```
Production  ::= production_name "::=" [ Expression ]
Expression  ::= Alternative { "|" Alternative }
Alternative ::= Term { Term }
Term        ::= production_name | token [ "…" token ] | Group | Option | Repetition
Group       ::= "(" Expression ")"
Option      ::= "[" Expression "]"
Repetition  ::= "{" Expression "}"
```

Productions are expressions constructed from terms and the following operators, in increasing precedence:

```
|   alternation
()  grouping
[]  option (0 or 1 times)
{}  repetition (0 to n times)
```

Lower-case production names are used to identify lexical tokens. Non-terminals are in CamelCase.
Lexical tokens are enclosed in double quotes `""` or back quotes ``.

The form `a … b` represents the set of characters from a through b as alternatives.



### Lexical Elements ###
In a **.dc** file, spacing characters such as `0x20` and `\t` as well as newline characters
like `\n` and `\r` that exist between lexical elements are ignored. 
These should be used freely to make a file human readable.

#### Letters and Digits ###
```
letter   ::= "A" ... "z"
decDigit ::= "0" ... "9"
octDigit ::= "0" ... "7"
hexDigit ::= "0" ... "9" | "A" ... "F" | "a" ... "f"
binDigit ::= "0" | "1"
```

#### Operators ####
```
operator ::= "%" | "*" | "+" | "-" | "/"
```

#### Delimiters ####
Delimiters are used to seperate other lexical tokens, and may have additional special
meaning in **Grammar** productions.
```
delimiter ::= "(" | ")" | "{" | "}" | "[" | "]" | "," | ";" | "=" | ":"
              | <spaces or tabs> | operator
```

#### Number Literals ####
```
numLiteral ::= intLiteral | floatLiteral
```

**Integers**
```
intLiteral ::= decLiteral | octLiteral | hexLiteral | binLiteral
decLiteral ::= ( "1" … "9" ) { decDigit }
octLiteral ::= "0" { octDigit }
hexLiteral ::= "0" ( "x" | "X" ) hexDigit { hexDigit }
binLiteral ::= "0" ( "b" | "B" ) binDigit { binDigit }
```

**Floats**
```
floatLiteral ::= decimals "." [ decimals ] | "." [ decimals ]
decimals     ::= decDigit { decDigit }
```

#### Text Literals ####
```
charLiteral     ::= "'" ( nonSingleQuote | escapeCharacter ) "'" 
stringLiteral   ::= `"` { stringCharacter } `"`
stringCharacter ::= nonDoubleQuote | escapeSequence
nonSingleQuote  ::= <any printable character except "'" or newline>
nonDoubleQuote  ::= <any printable character except `"` or newline>
escapeSequence  ::= "\" ( <any character> | "x" hexDigit { hexDigit } )
```
The escape characters `\n`, `\r`, `\t` translate into newline, carriage-return, and tab.  
Other ascii characters can be specified via `\x<hex-code>`.

#### Identifiers ####
Identifiers are used to name arguments, fields, and types.
```
identifier ::= letter { letter | decDigit }
```

#### Keywords ####
The following identifiers are reserved as keywords and may not be used as identifiers.
```
keyword ::= "dclass" | "struct" | "keyword"
```

#### Data Types ####
The following identifiers are reserved for datatypes and may not be used as identifiers.
```
dataType  ::= charType | intType | floatType | sizedType
charType  ::= "char"
intType   ::= "int8" | "int16" | "int32" | "int64"
              | "uint8" | "uint16" | "uint32" | "uint64"
floatType ::= "float64"
sizedType ::= "string" | "blob"
```



### Grammar ###

#### DC File ####
The DCFile is the root production of the grammar.  A valid DistributedClass file
must satisify the "DCFile" production.
```
DCFile ::= TypeDecl { TypeDecl }
TypeDecl ::= KeywordType | StructType | ClassType
```

#### Keywords ####
```
KeywordType ::= "keyword" identifier
KeywordList ::= identifier { "," identifier }
```

#### Struct Type ####
```
StructType ::= "struct" identifier "{" Parameter ";" { Parameter ";" } "}" ";"
```

#### Class Type ####
```
ClassType ::= "dclass" identifier "{" { FieldDecl ";" } "}" ";"
```

#### Field Types ####
```
FieldDecl ::= MolecularField | AtomicField | ParameterField
```

```
MolecularField ::= identifier ":" ( AtomicField | ParameterField )
                                  { "," ( AtomicField | ParameterField ) }
AtomicField    ::= identifier "(" Parameter { "," Parameter } ")" [ KeywordList ]
ParameterField ::= Parameter [ KeywordList ]
```

#### Parameter Types ####
```
Parameter ::= CharParameter | IntParameter | FloatParameter | SizedParameter
              | StructParameter | ArrayParameter
```

**Char Parameter**
```
CharParameter ::= charType [ identifier ] [ "=" charLiteral ]
```

**Integer Parameter**
```
IntParameter ::= intType [ IntRange ] [ IntTransform ] [ identifier ] [ "=" IntConstant ] 
IntConstant  ::= intLiteral | "{" intLiteral IntTransform "}"
IntTransform ::= operator intLiteral { IntTransform } | "(" IntTransform ")"
IntRange     ::= "(" intLiteral "-" intLiteral ")"
```

**Float Parameter**
```
FloatParameter ::= floatType [ FloatRange ] [ FloatTransform ] [ identifier ] [ "=" FloatConstant ]
FloatConstant  ::= numLiteral | "{" numLiteral FloatTransform "}"
FloatTransform ::= operator numLiteral { FloatTransform } | "(" FloatTransform ")"
FloatRange     ::= "(" floatLiteral "-" floatLiteral ")"
```

**Sized Parameter**
```
SizedParameter ::= sizedType [ SizeConstraint ] [ identifier ] [ "=" stringLiteral ]
SizeConstraint ::= "(" intLiteral ")"
```

**Struct Parameter**
```
StructParameter ::= identifier [ identifier ]
```

**Array Parameter**
```
ArrayParameter ::= ( dataType | identifier ) [ identifier ] ArrayRange
ArrayRange     ::= "[" [ intLiteral [ "-" intLiteral ] ] "]"
```

Example array ranges are `[]`, `[5]`, `[0-11]`, or `[2 - 24]`.
