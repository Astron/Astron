/* A Bison parser, made by GNU Bison 3.8.2.  */

/* Bison interface for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018-2021 Free Software Foundation,
   Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
   especially those whose name start with YY_ or yy_.  They are
   private implementation details that can be changed or removed.  */

#ifndef YY_YY_PARSER_H_INCLUDED
# define YY_YY_PARSER_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 1
#endif
#if YYDEBUG
extern int yydebug;
#endif

/* Token kinds.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    YYEMPTY = -2,
    YYEOF = 0,                     /* "end of file"  */
    YYerror = 256,                 /* error  */
    YYUNDEF = 257,                 /* "invalid token"  */
    UNSIGNED_INTEGER = 258,        /* UNSIGNED_INTEGER  */
    REAL = 259,                    /* REAL  */
    STRING = 260,                  /* STRING  */
    HEX_STRING = 261,              /* HEX_STRING  */
    IDENTIFIER = 262,              /* IDENTIFIER  */
    CHAR = 263,                    /* CHAR  */
    START_DC_FILE = 264,           /* START_DC_FILE  */
    START_DC_VALUE = 265,          /* START_DC_VALUE  */
    KW_DCLASS = 266,               /* KW_DCLASS  */
    KW_STRUCT = 267,               /* KW_STRUCT  */
    KW_FROM = 268,                 /* KW_FROM  */
    KW_IMPORT = 269,               /* KW_IMPORT  */
    KW_TYPEDEF = 270,              /* KW_TYPEDEF  */
    KW_KEYWORD = 271,              /* KW_KEYWORD  */
    KW_INT8 = 272,                 /* KW_INT8  */
    KW_INT16 = 273,                /* KW_INT16  */
    KW_INT32 = 274,                /* KW_INT32  */
    KW_INT64 = 275,                /* KW_INT64  */
    KW_UINT8 = 276,                /* KW_UINT8  */
    KW_UINT16 = 277,               /* KW_UINT16  */
    KW_UINT32 = 278,               /* KW_UINT32  */
    KW_UINT64 = 279,               /* KW_UINT64  */
    KW_FLOAT32 = 280,              /* KW_FLOAT32  */
    KW_FLOAT64 = 281,              /* KW_FLOAT64  */
    KW_STRING = 282,               /* KW_STRING  */
    KW_BLOB = 283,                 /* KW_BLOB  */
    KW_CHAR = 284                  /* KW_CHAR  */
  };
  typedef enum yytokentype yytoken_kind_t;
#endif

/* Value type.  */


extern YYSTYPE yylval;


int yyparse (void);


#endif /* !YY_YY_PARSER_H_INCLUDED  */
