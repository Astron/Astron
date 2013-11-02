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
Production  = production_name "=" [ Expression ] "." .
Expression  = Alternative { "|" Alternative } .
Alternative = Term { Term } .
Term        = production_name | token [ "…" token ] | Group | Option | Repetition .
Group       = "(" Expression ")" .
Option      = "[" Expression "]" .
Repetition  = "{" Expression "}" .
```

Productions are expressions constructed from terms and the following operators, in increasing precedence:

```
|   alternation
()  grouping
[]  option (0 or 1 times)
{}  repetition (0 to n times)
```

Lower-case production names are used to identify lexical tokens. Non-terminals are in CamelCase. Lexical tokens are enclosed in double quotes "" or back quotes ``.

The form a … b represents the set of characters from a through b as alternatives.

### Lexical Elements ###


### Grammer ###
