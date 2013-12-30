/* A Bison parser, made by GNU Bison 2.7.12-4996.  */

/* Bison implementation for Yacc-like parsers in C
   
      Copyright (C) 1984, 1989-1990, 2000-2013 Free Software Foundation, Inc.
   
   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.
   
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

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

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Bison version.  */
#define YYBISON_VERSION "2.7.12-4996"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1




/* Copy the first part of user declarations.  */
/* Line 371 of yacc.c  */
#line 5 "Parser.ypp"

#include "LexerDefs.h"
#include "ParserDefs.h"
#include "../File.h"
#include "../Class.h"
#include "../AtomicField.h"
#include "../MolecularField.h"
#include "../ClassParameter.h"
#include "../ArrayParameter.h"
#include "../SimpleParameter.h"
#include "../Typedef.h"
#include "../Keyword.h"
#include "../NumericRange.h"
#include "../DataType.h"


#include <assert.h>
#include <unistd.h>

using namespace dclass;
#define yyparse dcyyparse
#define yylex dcyylex
#define yyerror dcyyerror
#define yywarning dcyywarning
#define yylval dcyylval
#define yychar dcyychar
#define yydebug dcyydebug
#define yynerrs dcyynerrs

// Because our token type contains objects of type std::string, which
// require correct copy construction (and not simply memcpying), we
// cannot use bison's built-in auto-stack-grow feature.  As an easy
// solution, we ensure here that we have enough yacc stack to start
// with, and that it doesn't ever try to grow.
#define YYINITDEPTH 1000
#define YYMAXDEPTH 1000

	File *dc_file = (File*)NULL;

	static Class *current_class = (Class*)NULL;
	static AtomicField *current_atomic = (AtomicField*)NULL;
	static MolecularField *current_molecular = (MolecularField*)NULL;
	static Parameter *current_parameter = (Parameter*)NULL;
	static KeywordList current_keyword_list;

	/* Helper functions */
	static Parameter* param_with_modulus(Parameter* p, double mod);
	static Parameter* param_with_divisor(Parameter* p, uint32_t div);

////////////////////////////////////////////////////////////////////
// Defining the interface to the parser.
////////////////////////////////////////////////////////////////////

	void dc_init_parser(std::istream & in, const std::string & filename, File & file)
	{
		dc_file = &file;
		dc_init_lexer(in, filename);
	}

	void dc_cleanup_parser()
	{
		dc_file = (File *)NULL;
	}


/* Line 371 of yacc.c  */
#line 134 "Parser.cpp"

# ifndef YY_NULL
#  if defined __cplusplus && 201103L <= __cplusplus
#   define YY_NULL nullptr
#  else
#   define YY_NULL 0
#  endif
# endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 0
#endif

/* In a future release of Bison, this section will be replaced
   by #include "Parser.h".  */
#ifndef YY_YY_PARSER_H_INCLUDED
# define YY_YY_PARSER_H_INCLUDED
/* Enabling traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int yydebug;
#endif

/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     UNSIGNED_INTEGER = 258,
     SIGNED_INTEGER = 259,
     REAL = 260,
     STRING = 261,
     HEX_STRING = 262,
     IDENTIFIER = 263,
     KEYWORD = 264,
     START_DC = 265,
     KW_DCLASS = 266,
     KW_STRUCT = 267,
     KW_FROM = 268,
     KW_IMPORT = 269,
     KW_TYPEDEF = 270,
     KW_KEYWORD = 271,
     KW_INT8 = 272,
     KW_INT16 = 273,
     KW_INT32 = 274,
     KW_INT64 = 275,
     KW_UINT8 = 276,
     KW_UINT16 = 277,
     KW_UINT32 = 278,
     KW_UINT64 = 279,
     KW_FLOAT32 = 280,
     KW_FLOAT64 = 281,
     KW_STRING = 282,
     KW_BLOB = 283,
     KW_CHAR = 284
   };
#endif


#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED

# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
#endif

extern YYSTYPE yylval;

#ifdef YYPARSE_PARAM
#if defined __STDC__ || defined __cplusplus
int yyparse (void *YYPARSE_PARAM);
#else
int yyparse ();
#endif
#else /* ! YYPARSE_PARAM */
#if defined __STDC__ || defined __cplusplus
int yyparse (void);
#else
int yyparse ();
#endif
#endif /* ! YYPARSE_PARAM */

#endif /* !YY_YY_PARSER_H_INCLUDED  */

/* Copy the second part of user declarations.  */

/* Line 390 of yacc.c  */
#line 228 "Parser.cpp"

#ifdef short
# undef short
#endif

#ifdef YYTYPE_UINT8
typedef YYTYPE_UINT8 yytype_uint8;
#else
typedef unsigned char yytype_uint8;
#endif

#ifdef YYTYPE_INT8
typedef YYTYPE_INT8 yytype_int8;
#elif (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
typedef signed char yytype_int8;
#else
typedef short int yytype_int8;
#endif

#ifdef YYTYPE_UINT16
typedef YYTYPE_UINT16 yytype_uint16;
#else
typedef unsigned short int yytype_uint16;
#endif

#ifdef YYTYPE_INT16
typedef YYTYPE_INT16 yytype_int16;
#else
typedef short int yytype_int16;
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif ! defined YYSIZE_T && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned int
# endif
#endif

#define YYSIZE_MAXIMUM ((YYSIZE_T) -1)

#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(Msgid) dgettext ("bison-runtime", Msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(Msgid) Msgid
# endif
#endif

#ifndef __attribute__
/* This feature is available in gcc versions 2.5 and later.  */
# if (! defined __GNUC__ || __GNUC__ < 2 \
      || (__GNUC__ == 2 && __GNUC_MINOR__ < 5))
#  define __attribute__(Spec) /* empty */
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YYUSE(E) ((void) (E))
#else
# define YYUSE(E) /* empty */
#endif


/* Identity function, used to suppress warnings about constant conditions.  */
#ifndef lint
# define YYID(N) (N)
#else
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static int
YYID (int yyi)
#else
static int
YYID (yyi)
    int yyi;
#endif
{
  return yyi;
}
#endif

#if ! defined yyoverflow || YYERROR_VERBOSE

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined EXIT_SUCCESS && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
      /* Use EXIT_SUCCESS as a witness for stdlib.h.  */
#     ifndef EXIT_SUCCESS
#      define EXIT_SUCCESS 0
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's `empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (YYID (0))
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined EXIT_SUCCESS \
       && ! ((defined YYMALLOC || defined malloc) \
	     && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef EXIT_SUCCESS
#    define EXIT_SUCCESS 0
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined EXIT_SUCCESS && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined EXIT_SUCCESS && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* ! defined yyoverflow || YYERROR_VERBOSE */


#if (! defined yyoverflow \
     && (! defined __cplusplus \
	 || (defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yytype_int16 yyss_alloc;
  YYSTYPE yyvs_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (yytype_int16) + sizeof (YYSTYPE)) \
      + YYSTACK_GAP_MAXIMUM)

# define YYCOPY_NEEDED 1

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack_alloc, Stack)				\
    do									\
      {									\
	YYSIZE_T yynewbytes;						\
	YYCOPY (&yyptr->Stack_alloc, Stack, yysize);			\
	Stack = &yyptr->Stack_alloc;					\
	yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
	yyptr += yynewbytes / sizeof (*yyptr);				\
      }									\
    while (YYID (0))

#endif

#if defined YYCOPY_NEEDED && YYCOPY_NEEDED
/* Copy COUNT objects from SRC to DST.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(Dst, Src, Count) \
      __builtin_memcpy (Dst, Src, (Count) * sizeof (*(Src)))
#  else
#   define YYCOPY(Dst, Src, Count)              \
      do                                        \
        {                                       \
          YYSIZE_T yyi;                         \
          for (yyi = 0; yyi < (Count); yyi++)   \
            (Dst)[yyi] = (Src)[yyi];            \
        }                                       \
      while (YYID (0))
#  endif
# endif
#endif /* !YYCOPY_NEEDED */

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  5
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   229

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  45
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  61
/* YYNRULES -- Number of rules.  */
#define YYNRULES  154
/* YYNRULES -- Number of states.  */
#define YYNSTATES  238

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   284

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,    40,     2,     2,
      38,    39,    33,     2,    34,    44,    31,    32,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    37,    30,
       2,    43,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    41,     2,    42,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    35,     2,    36,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const yytype_uint16 yyprhs[] =
{
       0,     0,     3,     6,     8,    11,    14,    17,    20,    23,
      26,    27,    33,    35,    39,    41,    45,    47,    49,    51,
      55,    58,    61,    63,    66,    69,    71,    73,    74,    82,
      84,    86,    89,    91,    95,    97,   100,   104,   107,   110,
     113,   116,   117,   125,   127,   129,   132,   134,   138,   140,
     143,   147,   150,   153,   156,   159,   160,   166,   168,   170,
     172,   176,   178,   180,   182,   184,   186,   188,   190,   192,
     194,   196,   198,   200,   202,   204,   209,   213,   217,   221,
     225,   229,   232,   235,   238,   241,   246,   251,   256,   261,
     266,   270,   274,   278,   282,   286,   290,   294,   296,   298,
     302,   305,   309,   315,   320,   322,   324,   328,   331,   335,
     341,   346,   348,   350,   352,   354,   356,   358,   360,   362,
     364,   366,   368,   370,   372,   374,   378,   382,   384,   388,
     392,   396,   400,   404,   406,   408,   410,   412,   414,   416,
     418,   420,   422,   424,   426,   428,   430,   432,   435,   437,
     438,   443,   445,   447,   451
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int8 yyrhs[] =
{
      46,     0,    -1,    10,    47,    -1,   105,    -1,    47,    30,
      -1,    47,    57,    -1,    47,    48,    -1,    47,    54,    -1,
      47,    55,    -1,    14,    50,    -1,    -1,    13,    50,    14,
      49,    52,    -1,    51,    -1,    50,    31,    51,    -1,     8,
      -1,    51,    32,     8,    -1,    53,    -1,    33,    -1,    51,
      -1,    53,    34,    51,    -1,    15,    79,    -1,    16,    56,
      -1,   105,    -1,    56,     8,    -1,    56,     9,    -1,    58,
      -1,    65,    -1,    -1,    11,     8,    59,    61,    35,    63,
      36,    -1,     8,    -1,   105,    -1,    37,    62,    -1,    60,
      -1,    62,    34,    60,    -1,   105,    -1,    63,    30,    -1,
      63,    64,    30,    -1,    72,    99,    -1,   101,   100,    -1,
      78,    99,    -1,    77,    99,    -1,    -1,    12,     8,    66,
      68,    35,    70,    36,    -1,     8,    -1,   105,    -1,    37,
      69,    -1,    67,    -1,    69,    34,    67,    -1,   105,    -1,
      70,    30,    -1,    70,    71,    30,    -1,    72,   100,    -1,
     101,   100,    -1,    78,   100,    -1,    77,   100,    -1,    -1,
       8,    38,    73,    74,    39,    -1,   105,    -1,    75,    -1,
      76,    -1,    75,    34,    76,    -1,    79,    -1,    84,    -1,
      86,    -1,    88,    -1,    80,    -1,    81,    -1,    82,    -1,
      83,    -1,    85,    -1,    87,    -1,    77,    -1,    78,    -1,
      98,    -1,     8,    -1,    98,    38,    89,    39,    -1,    80,
      40,    94,    -1,    81,    40,    94,    -1,    80,    32,    92,
      -1,    81,    32,    92,    -1,    82,    32,    92,    -1,    80,
       8,    -1,    81,     8,    -1,    82,     8,    -1,    83,     8,
      -1,    80,    41,    90,    42,    -1,    81,    41,    90,    42,
      -1,    82,    41,    90,    42,    -1,    83,    41,    90,    42,
      -1,    84,    41,    90,    42,    -1,    80,    43,    96,    -1,
      81,    43,    96,    -1,    82,    43,    96,    -1,    83,    43,
      96,    -1,    85,    43,    96,    -1,    84,    43,    96,    -1,
      86,    43,    96,    -1,   105,    -1,    95,    -1,    95,    44,
      95,    -1,    95,    94,    -1,    89,    34,    95,    -1,    89,
      34,    95,    44,    95,    -1,    89,    34,    95,    94,    -1,
     105,    -1,    91,    -1,    91,    44,    91,    -1,    91,    93,
      -1,    90,    34,    91,    -1,    90,    34,    91,    44,    91,
      -1,    90,    34,    91,    93,    -1,     6,    -1,    92,    -1,
       3,    -1,     4,    -1,     3,    -1,     4,    -1,     5,    -1,
       6,    -1,    94,    -1,     4,    -1,     3,    -1,     5,    -1,
       6,    -1,     7,    -1,    41,    97,    42,    -1,    35,    97,
      36,    -1,    96,    -1,     4,    33,    92,    -1,     3,    33,
      92,    -1,     5,    33,    92,    -1,     7,    33,    92,    -1,
      97,    34,    96,    -1,    17,    -1,    18,    -1,    19,    -1,
      20,    -1,    21,    -1,    22,    -1,    23,    -1,    24,    -1,
      25,    -1,    26,    -1,    27,    -1,    28,    -1,    29,    -1,
     105,    -1,    99,     9,    -1,    99,    -1,    -1,     8,    37,
     102,   104,    -1,     8,    -1,   103,    -1,   104,    34,   103,
      -1,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   153,   153,   157,   158,   159,   174,   175,   176,   180,
     185,   184,   193,   194,   201,   202,   210,   211,   215,   216,
     220,   243,   247,   248,   252,   263,   264,   269,   268,   280,
     307,   308,   312,   319,   329,   330,   331,   349,   361,   362,
     371,   383,   382,   394,   422,   423,   427,   434,   444,   445,
     446,   460,   468,   469,   470,   475,   474,   494,   495,   499,
     500,   504,   514,   515,   516,   520,   521,   522,   523,   524,
     525,   529,   530,   534,   535,   561,   579,   580,   584,   585,
     586,   590,   591,   592,   593,   597,   598,   599,   600,   604,
     608,   609,   610,   611,   612,   616,   617,   621,   622,   631,
     640,   653,   661,   669,   684,   685,   694,   703,   712,   720,
     728,   739,   750,   754,   766,   782,   783,   784,   788,   800,
     804,   809,   814,   819,   838,   843,   857,   884,   889,   899,
     909,   919,   929,   937,   938,   939,   940,   941,   942,   943,
     944,   945,   946,   947,   948,   949,   953,   954,   958,   968,
     968,   973,  1006,  1013,  1028
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || 0
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "UNSIGNED_INTEGER", "SIGNED_INTEGER",
  "REAL", "STRING", "HEX_STRING", "IDENTIFIER", "KEYWORD", "START_DC",
  "KW_DCLASS", "KW_STRUCT", "KW_FROM", "KW_IMPORT", "KW_TYPEDEF",
  "KW_KEYWORD", "KW_INT8", "KW_INT16", "KW_INT32", "KW_INT64", "KW_UINT8",
  "KW_UINT16", "KW_UINT32", "KW_UINT64", "KW_FLOAT32", "KW_FLOAT64",
  "KW_STRING", "KW_BLOB", "KW_CHAR", "';'", "'.'", "'/'", "'*'", "','",
  "'{'", "'}'", "':'", "'('", "')'", "'%'", "'['", "']'", "'='", "'-'",
  "$accept", "grammar", "dc", "import", "$@1", "import_identifier",
  "import_path", "import_symbol_list_or_star", "import_symbol_list",
  "typedef_decl", "keyword_decl", "keyword_decl_list", "dclass_or_struct",
  "dclass", "@2", "dclass_name", "dclass_derivation", "dclass_base_list",
  "dclass_fields", "dclass_field", "struct", "@3", "struct_name",
  "struct_derivation", "struct_base_list", "struct_fields", "struct_field",
  "atomic_field", "@4", "parameter_list", "nonempty_parameter_list",
  "atomic_element", "named_parameter", "unnamed_parameter", "parameter",
  "param_w_typ", "param_w_rng", "param_w_mod", "param_w_div",
  "param_w_nam", "param_u_arr", "param_n_arr", "param_u_def",
  "param_n_def", "double_range", "uint_range", "char_or_uint",
  "small_unsigned_integer", "small_negative_integer", "number",
  "char_or_number", "parameter_value", "array_value", "type_token",
  "keyword_list", "no_keyword_list", "molecular_field", "$@5",
  "atomic_name", "molecular_atom_list", "empty", YY_NULL
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[YYLEX-NUM] -- Internal token number corresponding to
   token YYLEX-NUM.  */
static const yytype_uint16 yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
      59,    46,    47,    42,    44,   123,   125,    58,    40,    41,
      37,    91,    93,    61,    45
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    45,    46,    47,    47,    47,    47,    47,    47,    48,
      49,    48,    50,    50,    51,    51,    52,    52,    53,    53,
      54,    55,    56,    56,    56,    57,    57,    59,    58,    60,
      61,    61,    62,    62,    63,    63,    63,    64,    64,    64,
      64,    66,    65,    67,    68,    68,    69,    69,    70,    70,
      70,    71,    71,    71,    71,    73,    72,    74,    74,    75,
      75,    76,    77,    77,    77,    78,    78,    78,    78,    78,
      78,    79,    79,    80,    80,    81,    82,    82,    83,    83,
      83,    84,    84,    84,    84,    85,    85,    85,    85,    86,
      87,    87,    87,    87,    87,    88,    88,    89,    89,    89,
      89,    89,    89,    89,    90,    90,    90,    90,    90,    90,
      90,    91,    91,    92,    93,    94,    94,    94,    95,    95,
      96,    96,    96,    96,    96,    96,    96,    97,    97,    97,
      97,    97,    97,    98,    98,    98,    98,    98,    98,    98,
      98,    98,    98,    98,    98,    98,    99,    99,   100,   102,
     101,   103,   104,   104,   105
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     2,     1,     2,     2,     2,     2,     2,     2,
       0,     5,     1,     3,     1,     3,     1,     1,     1,     3,
       2,     2,     1,     2,     2,     1,     1,     0,     7,     1,
       1,     2,     1,     3,     1,     2,     3,     2,     2,     2,
       2,     0,     7,     1,     1,     2,     1,     3,     1,     2,
       3,     2,     2,     2,     2,     0,     5,     1,     1,     1,
       3,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     4,     3,     3,     3,     3,
       3,     2,     2,     2,     2,     4,     4,     4,     4,     4,
       3,     3,     3,     3,     3,     3,     3,     1,     1,     3,
       2,     3,     5,     4,     1,     1,     3,     2,     3,     5,
       4,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     3,     3,     1,     3,     3,
       3,     3,     3,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     2,     1,     0,
       4,     1,     1,     3,     0
};

/* YYDEFACT[STATE-NAME] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       0,   154,     0,     2,     3,     1,     0,     0,     0,     0,
       0,   154,     4,     6,     7,     8,     5,    25,    26,    27,
      41,    14,     0,    12,     9,    74,   133,   134,   135,   136,
     137,   138,   139,   140,   141,   142,   143,   144,   145,    71,
      72,    20,    65,    66,    67,    68,    62,    69,    63,    70,
      64,    73,    21,    22,   154,   154,    10,     0,     0,    81,
       0,     0,   154,     0,    82,     0,     0,   154,     0,    83,
       0,   154,     0,    84,   154,     0,   154,     0,     0,     0,
     154,    23,    24,     0,     0,    30,     0,     0,    44,     0,
      13,    15,   113,    78,   115,   116,   117,    76,   111,     0,
     105,   112,   104,   121,   120,   122,   123,   124,     0,     0,
      90,    79,    77,     0,    91,    80,     0,    92,     0,    93,
       0,    95,    94,    96,   118,     0,   119,    98,    97,    29,
      32,    31,   154,    43,    46,    45,   154,    17,    18,    11,
      16,     0,    85,   114,     0,   107,   121,   120,   122,   124,
     127,     0,     0,    86,    87,    88,    89,     0,    75,     0,
     100,     0,     0,    34,     0,     0,    48,     0,   108,   106,
       0,     0,     0,     0,     0,   126,   125,   101,    99,    33,
      74,    35,    28,     0,   154,   154,   154,   154,    47,    49,
      42,     0,   154,   154,   154,   154,    19,     0,   110,   129,
     128,   130,   131,   132,     0,   103,   149,    55,    36,    37,
     146,    40,    39,   148,    38,    50,    51,    54,    53,    52,
     109,   102,     0,   154,   147,   151,   152,   150,     0,    58,
      59,    61,    57,     0,    56,     0,   153,    60
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     2,     3,    13,    89,    22,    23,   139,   140,    14,
      15,    52,    16,    17,    54,   130,    84,   131,   162,   183,
      18,    55,   134,    87,   135,   165,   191,   184,   223,   228,
     229,   230,    39,    40,   231,    42,    43,    44,    45,    46,
      47,    48,    49,    50,   125,    99,   100,   101,   145,   126,
     127,   150,   151,    51,   213,   214,   187,   222,   226,   227,
     210
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -155
static const yytype_int16 yypact[] =
{
      32,  -155,    51,    88,  -155,  -155,    55,    56,    76,    76,
     165,  -155,  -155,  -155,  -155,  -155,  -155,  -155,  -155,  -155,
    -155,  -155,     7,   101,    65,  -155,  -155,  -155,  -155,  -155,
    -155,  -155,  -155,  -155,  -155,  -155,  -155,  -155,  -155,  -155,
    -155,  -155,    37,    54,    33,    40,    87,   113,   114,  -155,
    -155,   120,    27,  -155,   143,   144,  -155,    76,   171,  -155,
     194,   106,    13,    52,  -155,   194,   106,    13,    52,  -155,
     194,    13,    52,  -155,    13,    52,    13,    52,    52,    52,
     148,  -155,  -155,   190,   164,  -155,   192,   166,  -155,     6,
     101,  -155,  -155,  -155,  -155,  -155,  -155,  -155,  -155,    -8,
       0,  -155,  -155,  -155,  -155,  -155,  -155,  -155,    85,    85,
    -155,  -155,  -155,    18,  -155,  -155,    64,  -155,    74,  -155,
     161,  -155,  -155,  -155,  -155,    -9,  -155,    24,  -155,  -155,
    -155,   168,  -155,  -155,  -155,   172,  -155,  -155,   101,  -155,
     173,    13,  -155,  -155,    13,  -155,   175,   176,   177,   178,
    -155,    71,   162,  -155,  -155,  -155,  -155,   148,  -155,   148,
    -155,   190,   119,  -155,   192,   142,  -155,    76,     5,  -155,
     194,   194,   194,   194,    52,  -155,  -155,    28,  -155,  -155,
       9,  -155,  -155,   182,  -155,  -155,  -155,  -155,  -155,  -155,
    -155,   183,  -155,  -155,  -155,  -155,   101,    13,  -155,  -155,
    -155,  -155,  -155,  -155,   148,  -155,  -155,  -155,  -155,   196,
    -155,   196,   196,   196,  -155,  -155,  -155,  -155,  -155,  -155,
    -155,  -155,   206,   165,  -155,  -155,  -155,   181,   179,   185,
    -155,  -155,  -155,   206,  -155,   165,  -155,  -155
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -155,  -155,  -155,  -155,  -155,   207,   -46,  -155,  -155,  -155,
    -155,  -155,  -155,  -155,  -155,    59,  -155,  -155,  -155,  -155,
    -155,  -155,    53,  -155,  -155,  -155,  -155,    60,  -155,  -155,
    -155,   -14,  -125,   -80,   213,  -155,  -155,  -155,  -155,  -155,
    -155,  -155,  -155,  -155,  -155,    58,  -126,   -58,    61,   -60,
    -154,   -55,   115,  -155,   -62,   -18,    62,  -155,    -7,  -155,
      -1
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -1
static const yytype_uint8 yytable[] =
{
       4,    97,    93,   177,   143,   178,   112,   111,   110,   143,
      53,    90,   115,   114,    21,   168,    92,   117,   169,    98,
     119,    56,   121,   122,   123,   157,   141,    94,    95,    96,
     158,    94,    95,    96,   142,    81,    82,   185,    57,   137,
     193,    69,     1,   138,   144,    59,   206,   207,    73,   197,
     221,     5,   141,    85,    88,   103,   104,   105,   106,   107,
     153,   102,    64,    19,    20,    70,   102,   160,   159,    60,
     102,   220,   204,   102,    71,   102,    72,    61,    62,   128,
      63,    74,   186,    75,    21,   194,    65,   108,   146,   147,
     148,   106,   149,   109,    66,    67,    57,    68,   141,     6,
       7,     8,     9,    10,    11,   174,   154,   175,   141,    94,
      95,    96,   199,   200,   201,   202,   155,   205,    12,   203,
     108,   196,   209,   211,   212,   113,   109,   180,    76,   116,
      77,   163,   118,    58,   120,   166,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,   181,
     180,    94,    95,    96,   124,   182,    78,    79,    80,    26,
      27,    28,    29,    30,    31,    32,    33,    34,    35,    36,
      37,    38,   189,    25,   216,   217,   218,   219,   190,    91,
      83,    86,    26,    27,    28,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    38,   141,   174,    92,   129,   132,
     133,   136,   161,   156,   176,   224,   164,   167,   170,   171,
     172,   173,   208,   215,   225,   233,    24,   188,   234,   235,
     179,   237,   232,    41,   152,   192,   236,   195,     0,   198
};

#define yypact_value_is_default(Yystate) \
  (!!((Yystate) == (-155)))

#define yytable_value_is_error(Yytable_value) \
  YYID (0)

static const yytype_int16 yycheck[] =
{
       1,    61,    60,   157,     4,   159,    66,    65,    63,     4,
      11,    57,    70,    68,     8,   141,     3,    72,   144,     6,
      75,    14,    77,    78,    79,    34,    34,     3,     4,     5,
      39,     3,     4,     5,    42,     8,     9,   162,    31,    33,
     165,     8,    10,    89,    44,     8,    37,    38,     8,    44,
     204,     0,    34,    54,    55,     3,     4,     5,     6,     7,
      42,    62,     8,     8,     8,    32,    67,   127,    44,    32,
      71,   197,    44,    74,    41,    76,    43,    40,    41,    80,
      43,    41,   162,    43,     8,   165,    32,    35,     3,     4,
       5,     6,     7,    41,    40,    41,    31,    43,    34,    11,
      12,    13,    14,    15,    16,    34,    42,    36,    34,     3,
       4,     5,   170,   171,   172,   173,    42,   177,    30,   174,
      35,   167,   184,   185,   186,    67,    41,     8,    41,    71,
      43,   132,    74,    32,    76,   136,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    30,
       8,     3,     4,     5,     6,    36,    43,    43,    38,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,     8,   192,   193,   194,   195,    36,     8,
      37,    37,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    34,    34,     3,     8,    35,
       8,    35,    34,    42,    42,     9,    34,    34,    33,    33,
      33,    33,    30,    30,     8,    34,     9,   164,    39,    34,
     161,   235,   223,    10,   109,   165,   233,   165,    -1,   168
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,    10,    46,    47,   105,     0,    11,    12,    13,    14,
      15,    16,    30,    48,    54,    55,    57,    58,    65,     8,
       8,     8,    50,    51,    50,     8,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    77,
      78,    79,    80,    81,    82,    83,    84,    85,    86,    87,
      88,    98,    56,   105,    59,    66,    14,    31,    32,     8,
      32,    40,    41,    43,     8,    32,    40,    41,    43,     8,
      32,    41,    43,     8,    41,    43,    41,    43,    43,    43,
      38,     8,     9,    37,    61,   105,    37,    68,   105,    49,
      51,     8,     3,    92,     3,     4,     5,    94,     6,    90,
      91,    92,   105,     3,     4,     5,     6,     7,    35,    41,
      96,    92,    94,    90,    96,    92,    90,    96,    90,    96,
      90,    96,    96,    96,     6,    89,    94,    95,   105,     8,
      60,    62,    35,     8,    67,    69,    35,    33,    51,    52,
      53,    34,    42,     4,    44,    93,     3,     4,     5,     7,
      96,    97,    97,    42,    42,    42,    42,    34,    39,    44,
      94,    34,    63,   105,    34,    70,   105,    34,    91,    91,
      33,    33,    33,    33,    34,    36,    42,    95,    95,    60,
       8,    30,    36,    64,    72,    77,    78,   101,    67,    30,
      36,    71,    72,    77,    78,   101,    51,    44,    93,    92,
      92,    92,    92,    96,    44,    94,    37,    38,    30,    99,
     105,    99,    99,    99,   100,    30,   100,   100,   100,   100,
      91,    95,   102,    73,     9,     8,   103,   104,    74,    75,
      76,    79,   105,    34,    39,    34,   103,    76
};

#define yyerrok		(yyerrstatus = 0)
#define yyclearin	(yychar = YYEMPTY)
#define YYEMPTY		(-2)
#define YYEOF		0

#define YYACCEPT	goto yyacceptlab
#define YYABORT		goto yyabortlab
#define YYERROR		goto yyerrorlab


/* Like YYERROR except do call yyerror.  This remains here temporarily
   to ease the transition to the new meaning of YYERROR, for GCC.
   Once GCC version 2 has supplanted version 1, this can go.  However,
   YYFAIL appears to be in use.  Nevertheless, it is formally deprecated
   in Bison 2.4.2's NEWS entry, where a plan to phase it out is
   discussed.  */

#define YYFAIL		goto yyerrlab
#if defined YYFAIL
  /* This is here to suppress warnings from the GCC cpp's
     -Wunused-macros.  Normally we don't worry about that warning, but
     some users do, and we want to make it easy for users to remove
     YYFAIL uses, which will produce warnings from Bison 2.5.  */
#endif

#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)                                  \
do                                                              \
  if (yychar == YYEMPTY)                                        \
    {                                                           \
      yychar = (Token);                                         \
      yylval = (Value);                                         \
      YYPOPSTACK (yylen);                                       \
      yystate = *yyssp;                                         \
      goto yybackup;                                            \
    }                                                           \
  else                                                          \
    {                                                           \
      yyerror (YY_("syntax error: cannot back up")); \
      YYERROR;							\
    }								\
while (YYID (0))

/* Error token number */
#define YYTERROR	1
#define YYERRCODE	256


/* This macro is provided for backward compatibility. */
#ifndef YY_LOCATION_PRINT
# define YY_LOCATION_PRINT(File, Loc) ((void) 0)
#endif


/* YYLEX -- calling `yylex' with the right arguments.  */
#ifdef YYLEX_PARAM
# define YYLEX yylex (YYLEX_PARAM)
#else
# define YYLEX yylex ()
#endif

/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)			\
do {						\
  if (yydebug)					\
    YYFPRINTF Args;				\
} while (YYID (0))

# define YY_SYMBOL_PRINT(Title, Type, Value, Location)			  \
do {									  \
  if (yydebug)								  \
    {									  \
      YYFPRINTF (stderr, "%s ", Title);					  \
      yy_symbol_print (stderr,						  \
		  Type, Value); \
      YYFPRINTF (stderr, "\n");						  \
    }									  \
} while (YYID (0))


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_value_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
#else
static void
yy_symbol_value_print (yyoutput, yytype, yyvaluep)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
#endif
{
  FILE *yyo = yyoutput;
  YYUSE (yyo);
  if (!yyvaluep)
    return;
# ifdef YYPRINT
  if (yytype < YYNTOKENS)
    YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# else
  YYUSE (yyoutput);
# endif
  YYUSE (yytype);
}


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
#else
static void
yy_symbol_print (yyoutput, yytype, yyvaluep)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
#endif
{
  if (yytype < YYNTOKENS)
    YYFPRINTF (yyoutput, "token %s (", yytname[yytype]);
  else
    YYFPRINTF (yyoutput, "nterm %s (", yytname[yytype]);

  yy_symbol_value_print (yyoutput, yytype, yyvaluep);
  YYFPRINTF (yyoutput, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_stack_print (yytype_int16 *yybottom, yytype_int16 *yytop)
#else
static void
yy_stack_print (yybottom, yytop)
    yytype_int16 *yybottom;
    yytype_int16 *yytop;
#endif
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)				\
do {								\
  if (yydebug)							\
    yy_stack_print ((Bottom), (Top));				\
} while (YYID (0))


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_reduce_print (YYSTYPE *yyvsp, int yyrule)
#else
static void
yy_reduce_print (yyvsp, yyrule)
    YYSTYPE *yyvsp;
    int yyrule;
#endif
{
  int yynrhs = yyr2[yyrule];
  int yyi;
  unsigned long int yylno = yyrline[yyrule];
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %lu):\n",
	     yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr, yyrhs[yyprhs[yyrule] + yyi],
		       &(yyvsp[(yyi + 1) - (yynrhs)])
		       		       );
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)		\
do {					\
  if (yydebug)				\
    yy_reduce_print (yyvsp, Rule); \
} while (YYID (0))

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args)
# define YY_SYMBOL_PRINT(Title, Type, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef	YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif


#if YYERROR_VERBOSE

# ifndef yystrlen
#  if defined __GLIBC__ && defined _STRING_H
#   define yystrlen strlen
#  else
/* Return the length of YYSTR.  */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static YYSIZE_T
yystrlen (const char *yystr)
#else
static YYSIZE_T
yystrlen (yystr)
    const char *yystr;
#endif
{
  YYSIZE_T yylen;
  for (yylen = 0; yystr[yylen]; yylen++)
    continue;
  return yylen;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined __GLIBC__ && defined _STRING_H && defined _GNU_SOURCE
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static char *
yystpcpy (char *yydest, const char *yysrc)
#else
static char *
yystpcpy (yydest, yysrc)
    char *yydest;
    const char *yysrc;
#endif
{
  char *yyd = yydest;
  const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
#  endif
# endif

# ifndef yytnamerr
/* Copy to YYRES the contents of YYSTR after stripping away unnecessary
   quotes and backslashes, so that it's suitable for yyerror.  The
   heuristic is that double-quoting is unnecessary unless the string
   contains an apostrophe, a comma, or backslash (other than
   backslash-backslash).  YYSTR is taken from yytname.  If YYRES is
   null, do not copy; instead, return the length of what the result
   would have been.  */
static YYSIZE_T
yytnamerr (char *yyres, const char *yystr)
{
  if (*yystr == '"')
    {
      YYSIZE_T yyn = 0;
      char const *yyp = yystr;

      for (;;)
	switch (*++yyp)
	  {
	  case '\'':
	  case ',':
	    goto do_not_strip_quotes;

	  case '\\':
	    if (*++yyp != '\\')
	      goto do_not_strip_quotes;
	    /* Fall through.  */
	  default:
	    if (yyres)
	      yyres[yyn] = *yyp;
	    yyn++;
	    break;

	  case '"':
	    if (yyres)
	      yyres[yyn] = '\0';
	    return yyn;
	  }
    do_not_strip_quotes: ;
    }

  if (! yyres)
    return yystrlen (yystr);

  return yystpcpy (yyres, yystr) - yyres;
}
# endif

/* Copy into *YYMSG, which is of size *YYMSG_ALLOC, an error message
   about the unexpected token YYTOKEN for the state stack whose top is
   YYSSP.

   Return 0 if *YYMSG was successfully written.  Return 1 if *YYMSG is
   not large enough to hold the message.  In that case, also set
   *YYMSG_ALLOC to the required number of bytes.  Return 2 if the
   required number of bytes is too large to store.  */
static int
yysyntax_error (YYSIZE_T *yymsg_alloc, char **yymsg,
                yytype_int16 *yyssp, int yytoken)
{
  YYSIZE_T yysize0 = yytnamerr (YY_NULL, yytname[yytoken]);
  YYSIZE_T yysize = yysize0;
  enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
  /* Internationalized format string. */
  const char *yyformat = YY_NULL;
  /* Arguments of yyformat. */
  char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
  /* Number of reported tokens (one for the "unexpected", one per
     "expected"). */
  int yycount = 0;

  /* There are many possibilities here to consider:
     - Assume YYFAIL is not used.  It's too flawed to consider.  See
       <http://lists.gnu.org/archive/html/bison-patches/2009-12/msg00024.html>
       for details.  YYERROR is fine as it does not invoke this
       function.
     - If this state is a consistent state with a default action, then
       the only way this function was invoked is if the default action
       is an error action.  In that case, don't check for expected
       tokens because there are none.
     - The only way there can be no lookahead present (in yychar) is if
       this state is a consistent state with a default action.  Thus,
       detecting the absence of a lookahead is sufficient to determine
       that there is no unexpected or expected token to report.  In that
       case, just report a simple "syntax error".
     - Don't assume there isn't a lookahead just because this state is a
       consistent state with a default action.  There might have been a
       previous inconsistent state, consistent state with a non-default
       action, or user semantic action that manipulated yychar.
     - Of course, the expected token list depends on states to have
       correct lookahead information, and it depends on the parser not
       to perform extra reductions after fetching a lookahead from the
       scanner and before detecting a syntax error.  Thus, state merging
       (from LALR or IELR) and default reductions corrupt the expected
       token list.  However, the list is correct for canonical LR with
       one exception: it will still contain any token that will not be
       accepted due to an error action in a later state.
  */
  if (yytoken != YYEMPTY)
    {
      int yyn = yypact[*yyssp];
      yyarg[yycount++] = yytname[yytoken];
      if (!yypact_value_is_default (yyn))
        {
          /* Start YYX at -YYN if negative to avoid negative indexes in
             YYCHECK.  In other words, skip the first -YYN actions for
             this state because they are default actions.  */
          int yyxbegin = yyn < 0 ? -yyn : 0;
          /* Stay within bounds of both yycheck and yytname.  */
          int yychecklim = YYLAST - yyn + 1;
          int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
          int yyx;

          for (yyx = yyxbegin; yyx < yyxend; ++yyx)
            if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR
                && !yytable_value_is_error (yytable[yyx + yyn]))
              {
                if (yycount == YYERROR_VERBOSE_ARGS_MAXIMUM)
                  {
                    yycount = 1;
                    yysize = yysize0;
                    break;
                  }
                yyarg[yycount++] = yytname[yyx];
                {
                  YYSIZE_T yysize1 = yysize + yytnamerr (YY_NULL, yytname[yyx]);
                  if (! (yysize <= yysize1
                         && yysize1 <= YYSTACK_ALLOC_MAXIMUM))
                    return 2;
                  yysize = yysize1;
                }
              }
        }
    }

  switch (yycount)
    {
# define YYCASE_(N, S)                      \
      case N:                               \
        yyformat = S;                       \
      break
      YYCASE_(0, YY_("syntax error"));
      YYCASE_(1, YY_("syntax error, unexpected %s"));
      YYCASE_(2, YY_("syntax error, unexpected %s, expecting %s"));
      YYCASE_(3, YY_("syntax error, unexpected %s, expecting %s or %s"));
      YYCASE_(4, YY_("syntax error, unexpected %s, expecting %s or %s or %s"));
      YYCASE_(5, YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s"));
# undef YYCASE_
    }

  {
    YYSIZE_T yysize1 = yysize + yystrlen (yyformat);
    if (! (yysize <= yysize1 && yysize1 <= YYSTACK_ALLOC_MAXIMUM))
      return 2;
    yysize = yysize1;
  }

  if (*yymsg_alloc < yysize)
    {
      *yymsg_alloc = 2 * yysize;
      if (! (yysize <= *yymsg_alloc
             && *yymsg_alloc <= YYSTACK_ALLOC_MAXIMUM))
        *yymsg_alloc = YYSTACK_ALLOC_MAXIMUM;
      return 1;
    }

  /* Avoid sprintf, as that infringes on the user's name space.
     Don't have undefined behavior even if the translation
     produced a string with the wrong number of "%s"s.  */
  {
    char *yyp = *yymsg;
    int yyi = 0;
    while ((*yyp = *yyformat) != '\0')
      if (*yyp == '%' && yyformat[1] == 's' && yyi < yycount)
        {
          yyp += yytnamerr (yyp, yyarg[yyi++]);
          yyformat += 2;
        }
      else
        {
          yyp++;
          yyformat++;
        }
  }
  return 0;
}
#endif /* YYERROR_VERBOSE */

/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep)
#else
static void
yydestruct (yymsg, yytype, yyvaluep)
    const char *yymsg;
    int yytype;
    YYSTYPE *yyvaluep;
#endif
{
  YYUSE (yyvaluep);

  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

  YYUSE (yytype);
}




/* The lookahead symbol.  */
int yychar;


#ifndef YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_END
#endif
#ifndef YY_INITIAL_VALUE
# define YY_INITIAL_VALUE(Value) /* Nothing. */
#endif

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval YY_INITIAL_VALUE(yyval_default);

/* Number of syntax errors so far.  */
int yynerrs;


/*----------.
| yyparse.  |
`----------*/

#ifdef YYPARSE_PARAM
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yyparse (void *YYPARSE_PARAM)
#else
int
yyparse (YYPARSE_PARAM)
    void *YYPARSE_PARAM;
#endif
#else /* ! YYPARSE_PARAM */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yyparse (void)
#else
int
yyparse ()

#endif
#endif
{
    int yystate;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus;

    /* The stacks and their tools:
       `yyss': related to states.
       `yyvs': related to semantic values.

       Refer to the stacks through separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* The state stack.  */
    yytype_int16 yyssa[YYINITDEPTH];
    yytype_int16 *yyss;
    yytype_int16 *yyssp;

    /* The semantic value stack.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs;
    YYSTYPE *yyvsp;

    YYSIZE_T yystacksize;

  int yyn;
  int yyresult;
  /* Lookahead token as an internal (translated) token number.  */
  int yytoken = 0;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;

#if YYERROR_VERBOSE
  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYSIZE_T yymsg_alloc = sizeof yymsgbuf;
#endif

#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  yyssp = yyss = yyssa;
  yyvsp = yyvs = yyvsa;
  yystacksize = YYINITDEPTH;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY; /* Cause a token to be read.  */
  goto yysetstate;

/*------------------------------------------------------------.
| yynewstate -- Push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
 yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;

 yysetstate:
  *yyssp = yystate;

  if (yyss + yystacksize - 1 <= yyssp)
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = yyssp - yyss + 1;

#ifdef yyoverflow
      {
	/* Give user a chance to reallocate the stack.  Use copies of
	   these so that the &'s don't force the real ones into
	   memory.  */
	YYSTYPE *yyvs1 = yyvs;
	yytype_int16 *yyss1 = yyss;

	/* Each stack pointer address is followed by the size of the
	   data in use in that stack, in bytes.  This used to be a
	   conditional around just the two extra args, but that might
	   be undefined if yyoverflow is a macro.  */
	yyoverflow (YY_("memory exhausted"),
		    &yyss1, yysize * sizeof (*yyssp),
		    &yyvs1, yysize * sizeof (*yyvsp),
		    &yystacksize);

	yyss = yyss1;
	yyvs = yyvs1;
      }
#else /* no yyoverflow */
# ifndef YYSTACK_RELOCATE
      goto yyexhaustedlab;
# else
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
	goto yyexhaustedlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
	yystacksize = YYMAXDEPTH;

      {
	yytype_int16 *yyss1 = yyss;
	union yyalloc *yyptr =
	  (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
	if (! yyptr)
	  goto yyexhaustedlab;
	YYSTACK_RELOCATE (yyss_alloc, yyss);
	YYSTACK_RELOCATE (yyvs_alloc, yyvs);
#  undef YYSTACK_RELOCATE
	if (yyss1 != yyssa)
	  YYSTACK_FREE (yyss1);
      }
# endif
#endif /* no yyoverflow */

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;

      YYDPRINTF ((stderr, "Stack size increased to %lu\n",
		  (unsigned long int) yystacksize));

      if (yyss + yystacksize - 1 <= yyssp)
	YYABORT;
    }

  YYDPRINTF ((stderr, "Entering state %d\n", yystate));

  if (yystate == YYFINAL)
    YYACCEPT;

  goto yybackup;

/*-----------.
| yybackup.  |
`-----------*/
yybackup:

  /* Do appropriate processing given the current state.  Read a
     lookahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to lookahead token.  */
  yyn = yypact[yystate];
  if (yypact_value_is_default (yyn))
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid lookahead symbol.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = YYLEX;
    }

  if (yychar <= YYEOF)
    {
      yychar = yytoken = YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yytable_value_is_error (yyn))
        goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the lookahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);

  /* Discard the shifted token.  */
  yychar = YYEMPTY;

  yystate = yyn;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END

  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- Do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     `$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
        case 5:
/* Line 1787 of yacc.c  */
#line 160 "Parser.ypp"
    {
		if(!dc_file->add_class((yyvsp[(2) - (2)].u.dclass)))
		{
			Class *old_class = dc_file->get_class_by_name((yyvsp[(2) - (2)].u.dclass)->get_name());
			if(old_class != (Class *)NULL && old_class->is_bogus_class())
			{
				yyerror("Base class defined after its first reference: " + (yyvsp[(2) - (2)].u.dclass)->get_name());
			}
			else
			{
				yyerror("Duplicate class name: " + (yyvsp[(2) - (2)].u.dclass)->get_name());
			}
		}
	}
    break;

  case 9:
/* Line 1787 of yacc.c  */
#line 181 "Parser.ypp"
    {
		dc_file->add_import_module((yyvsp[(2) - (2)].str));
	}
    break;

  case 10:
/* Line 1787 of yacc.c  */
#line 185 "Parser.ypp"
    {
		dc_file->add_import_module((yyvsp[(2) - (3)].str));
	}
    break;

  case 13:
/* Line 1787 of yacc.c  */
#line 195 "Parser.ypp"
    {
		(yyval.str) = (yyvsp[(1) - (3)].str) + std::string(".") + (yyvsp[(3) - (3)].str);
	}
    break;

  case 15:
/* Line 1787 of yacc.c  */
#line 203 "Parser.ypp"
    {
		(yyval.str) = (yyvsp[(1) - (3)].str) + std::string("/") + (yyvsp[(3) - (3)].str);
	}
    break;

  case 17:
/* Line 1787 of yacc.c  */
#line 211 "Parser.ypp"
    { dc_file->add_import_symbol("*"); }
    break;

  case 18:
/* Line 1787 of yacc.c  */
#line 215 "Parser.ypp"
    { dc_file->add_import_symbol((yyvsp[(1) - (1)].str)); }
    break;

  case 19:
/* Line 1787 of yacc.c  */
#line 216 "Parser.ypp"
    { dc_file->add_import_symbol((yyvsp[(3) - (3)].str)); }
    break;

  case 20:
/* Line 1787 of yacc.c  */
#line 221 "Parser.ypp"
    {
	if((yyvsp[(2) - (2)].u.parameter) != (Parameter *)NULL)
	{
		Typedef *dtypedef = new Typedef((yyvsp[(2) - (2)].u.parameter));

		if(!dc_file->add_typedef(dtypedef))
		{
			Typedef *old_typedef = dc_file->get_typedef_by_name(dtypedef->get_name());
			if(old_typedef->is_bogus_typedef())
			{
				yyerror("typedef defined after its first reference: " + dtypedef->get_name());
			}
			else
			{
				yyerror("Duplicate typedef name: " + dtypedef->get_name());
			}
		}
	}
}
    break;

  case 23:
/* Line 1787 of yacc.c  */
#line 249 "Parser.ypp"
    {
	dc_file->add_keyword((yyvsp[(2) - (2)].str));
}
    break;

  case 24:
/* Line 1787 of yacc.c  */
#line 253 "Parser.ypp"
    {
	// This keyword has already been defined.  But since we are now
	// explicitly defining it, clear its bitmask, so that we will have a
	// new hash code--doing this will allow us to phase out the
	// historical hash code support later.
	((Keyword *)(yyvsp[(2) - (2)].u.keyword))->clear_historical_flag();
}
    break;

  case 27:
/* Line 1787 of yacc.c  */
#line 269 "Parser.ypp"
    {
		current_class = new Class(dc_file, (yyvsp[(2) - (2)].str), false, false);
	}
    break;

  case 28:
/* Line 1787 of yacc.c  */
#line 273 "Parser.ypp"
    {
		(yyval.u.dclass) = current_class;
		current_class = (yyvsp[(3) - (7)].u.dclass);
	}
    break;

  case 29:
/* Line 1787 of yacc.c  */
#line 281 "Parser.ypp"
    {
	if(dc_file == (File *)NULL)
	{
		yyerror("No File available, so no class names are predefined.");
		(yyval.u.dclass) = NULL;

	}
	else {
		Class *dclass = dc_file->get_class_by_name((yyvsp[(1) - (1)].str));
		if(dclass == (Class *)NULL)
		{
			// Create a bogus class as a forward reference.
			dclass = new Class(dc_file, (yyvsp[(1) - (1)].str), false, true);
			dc_file->add_class(dclass);
		}
		if(dclass->is_struct())
		{
			yyerror("struct name not allowed");
		}

		(yyval.u.dclass) = dclass;
	}
}
    break;

  case 32:
/* Line 1787 of yacc.c  */
#line 313 "Parser.ypp"
    {
	if((yyvsp[(1) - (1)].u.dclass) != (Class *)NULL)
	{
		current_class->add_parent((yyvsp[(1) - (1)].u.dclass));
	}
}
    break;

  case 33:
/* Line 1787 of yacc.c  */
#line 320 "Parser.ypp"
    {
	if((yyvsp[(3) - (3)].u.dclass) != (Class *)NULL)
	{
			current_class->add_parent((yyvsp[(3) - (3)].u.dclass));
	}
}
    break;

  case 36:
/* Line 1787 of yacc.c  */
#line 332 "Parser.ypp"
    {
	if((yyvsp[(2) - (3)].u.field) == (Field *)NULL)
	{
		// Pass this error up.
	}
	else if(!current_class->add_field((yyvsp[(2) - (3)].u.field)))
	{
		yyerror("Duplicate field name: " + (yyvsp[(2) - (3)].u.field)->get_name());
	}
	else if((yyvsp[(2) - (3)].u.field)->get_number() < 0)
	{
		yyerror("A non-network field cannot be stored on a dclass");
	}
}
    break;

  case 37:
/* Line 1787 of yacc.c  */
#line 350 "Parser.ypp"
    {
	if((yyvsp[(1) - (2)].u.field) != (Field *)NULL)
	{
		if((yyvsp[(1) - (2)].u.field)->get_name().empty())
		{
			yyerror("Field name required.");
		}
		(yyvsp[(1) - (2)].u.field)->copy_keywords(current_keyword_list);
	}
	(yyval.u.field) = (yyvsp[(1) - (2)].u.field);
}
    break;

  case 39:
/* Line 1787 of yacc.c  */
#line 363 "Parser.ypp"
    {
	yyerror("Unnamed parameters are not allowed on a dclass");
	if((yyvsp[(1) - (2)].u.parameter) != (Field *)NULL)
	{
		(yyvsp[(1) - (2)].u.parameter)->copy_keywords(current_keyword_list);
	}
	(yyval.u.field) = (yyvsp[(1) - (2)].u.parameter);
}
    break;

  case 40:
/* Line 1787 of yacc.c  */
#line 372 "Parser.ypp"
    {
	if((yyvsp[(1) - (2)].u.parameter) != (Field *)NULL)
	{
		(yyvsp[(1) - (2)].u.parameter)->copy_keywords(current_keyword_list);
	}
	(yyval.u.field) = (yyvsp[(1) - (2)].u.parameter);
}
    break;

  case 41:
/* Line 1787 of yacc.c  */
#line 383 "Parser.ypp"
    {
	current_class = new Class(dc_file, (yyvsp[(2) - (2)].str), true, false);
}
    break;

  case 42:
/* Line 1787 of yacc.c  */
#line 387 "Parser.ypp"
    {
	(yyval.u.dclass) = current_class;
	current_class = (yyvsp[(3) - (7)].u.dclass);
}
    break;

  case 43:
/* Line 1787 of yacc.c  */
#line 395 "Parser.ypp"
    {
	if(dc_file == (File *)NULL)
	{
		yyerror("No File available, so no struct names are predefined.");
		(yyval.u.dclass) = NULL;

	}
	else
	{
		Class *dstruct = dc_file->get_class_by_name((yyvsp[(1) - (1)].str));
		if(dstruct == (Class *)NULL)
		{
			// Create a bogus class as a forward reference.
			dstruct = new Class(dc_file, (yyvsp[(1) - (1)].str), false, true);
			dc_file->add_class(dstruct);
		}
		if(!dstruct->is_struct())
		{
			yyerror("struct name required");
		}

		(yyval.u.dclass) = dstruct;
	}
}
    break;

  case 46:
/* Line 1787 of yacc.c  */
#line 428 "Parser.ypp"
    {
		if((yyvsp[(1) - (1)].u.dclass) != (Class *)NULL)
		{
			current_class->add_parent((yyvsp[(1) - (1)].u.dclass));
		}
	}
    break;

  case 47:
/* Line 1787 of yacc.c  */
#line 435 "Parser.ypp"
    {
		if((yyvsp[(3) - (3)].u.dclass) != (Class *)NULL)
		{
			current_class->add_parent((yyvsp[(3) - (3)].u.dclass));
		}
	}
    break;

  case 50:
/* Line 1787 of yacc.c  */
#line 447 "Parser.ypp"
    {
		if((yyvsp[(2) - (3)].u.field) == (Field *)NULL)
		{
			// Pass this error up.
		}
		else if(!current_class->add_field((yyvsp[(2) - (3)].u.field)))
		{
			yyerror("Duplicate field name: " + (yyvsp[(2) - (3)].u.field)->get_name());
		}
	}
    break;

  case 51:
/* Line 1787 of yacc.c  */
#line 461 "Parser.ypp"
    {
		if((yyvsp[(1) - (2)].u.field)->get_name().empty())
		{
			yyerror("Field name required.");
		}
		(yyval.u.field) = (yyvsp[(1) - (2)].u.field);
	}
    break;

  case 53:
/* Line 1787 of yacc.c  */
#line 469 "Parser.ypp"
    { (yyval.u.field) = (yyvsp[(1) - (2)].u.parameter); }
    break;

  case 54:
/* Line 1787 of yacc.c  */
#line 470 "Parser.ypp"
    { (yyval.u.field) = (yyvsp[(1) - (2)].u.parameter); }
    break;

  case 55:
/* Line 1787 of yacc.c  */
#line 475 "Parser.ypp"
    {
		if(current_class == (Class *)NULL)
		{
			yyerror("Cannot define a method outside of a struct or class.");
			Class *temp_class = new Class(dc_file, "temp", false, false);  // memory leak.
			current_atomic = new AtomicField((yyvsp[(1) - (2)].str), temp_class, false);
		}
		else {
			current_atomic = new AtomicField((yyvsp[(1) - (2)].str), current_class, false);
		}
	}
    break;

  case 56:
/* Line 1787 of yacc.c  */
#line 487 "Parser.ypp"
    {
		(yyval.u.field) = current_atomic;
		current_atomic = (yyvsp[(3) - (5)].u.atomic);
	}
    break;

  case 61:
/* Line 1787 of yacc.c  */
#line 505 "Parser.ypp"
    {
		if((yyvsp[(1) - (1)].u.parameter) != (Parameter *)NULL)
		{
			current_atomic->add_element((yyvsp[(1) - (1)].u.parameter));
		}
	}
    break;

  case 62:
/* Line 1787 of yacc.c  */
#line 514 "Parser.ypp"
    { (yyval.u.parameter) = (yyvsp[(1) - (1)].u.parameter); }
    break;

  case 63:
/* Line 1787 of yacc.c  */
#line 515 "Parser.ypp"
    { (yyval.u.parameter) = (yyvsp[(1) - (1)].u.parameter); }
    break;

  case 64:
/* Line 1787 of yacc.c  */
#line 516 "Parser.ypp"
    { (yyval.u.parameter) = (yyvsp[(1) - (1)].u.parameter); }
    break;

  case 65:
/* Line 1787 of yacc.c  */
#line 520 "Parser.ypp"
    { (yyval.u.parameter) = (yyvsp[(1) - (1)].u.parameter); }
    break;

  case 66:
/* Line 1787 of yacc.c  */
#line 521 "Parser.ypp"
    { (yyval.u.parameter) = (yyvsp[(1) - (1)].u.parameter); }
    break;

  case 67:
/* Line 1787 of yacc.c  */
#line 522 "Parser.ypp"
    { (yyval.u.parameter) = (yyvsp[(1) - (1)].u.parameter); }
    break;

  case 68:
/* Line 1787 of yacc.c  */
#line 523 "Parser.ypp"
    { (yyval.u.parameter) = (yyvsp[(1) - (1)].u.parameter); }
    break;

  case 69:
/* Line 1787 of yacc.c  */
#line 524 "Parser.ypp"
    { (yyval.u.parameter) = (yyvsp[(1) - (1)].u.parameter); }
    break;

  case 70:
/* Line 1787 of yacc.c  */
#line 525 "Parser.ypp"
    { (yyval.u.parameter) = (yyvsp[(1) - (1)].u.parameter); }
    break;

  case 71:
/* Line 1787 of yacc.c  */
#line 529 "Parser.ypp"
    { (yyval.u.parameter) = (yyvsp[(1) - (1)].u.parameter); }
    break;

  case 72:
/* Line 1787 of yacc.c  */
#line 530 "Parser.ypp"
    { (yyval.u.parameter) = (yyvsp[(1) - (1)].u.parameter); }
    break;

  case 73:
/* Line 1787 of yacc.c  */
#line 534 "Parser.ypp"
    { (yyval.u.parameter) = current_parameter = new SimpleParameter((yyvsp[(1) - (1)].u.datatype));   }
    break;

  case 74:
/* Line 1787 of yacc.c  */
#line 536 "Parser.ypp"
    {
		Typedef *dtypedef = dc_file->get_typedef_by_name((yyvsp[(1) - (1)].str));
		if(dtypedef == (Typedef *)NULL)
		{
			// Maybe it's a class name.
			Class *dclass = dc_file->get_class_by_name((yyvsp[(1) - (1)].str));
			if(dclass != (Class *)NULL)
			{
				// Create an implicit typedef for this.
				dtypedef = new Typedef(new ClassParameter(dclass), true);
			}
			else
			{
				// It's an undefined typedef.  Create a bogus forward reference.
				dtypedef = new Typedef((yyvsp[(1) - (1)].str));
			}

			dc_file->add_typedef(dtypedef);
		}

		(yyval.u.parameter) = current_parameter = dtypedef->make_new_parameter();
	}
    break;

  case 75:
/* Line 1787 of yacc.c  */
#line 562 "Parser.ypp"
    {
		SimpleParameter *simple_param = new SimpleParameter((yyvsp[(1) - (4)].u.datatype));
		if(simple_param == NULL
		|| simple_param->get_typedef() != (Typedef*)NULL)
		{
			yyerror("Ranges are only valid for numeric, string, or blob types.");
		}
		if(!simple_param->set_range((yyvsp[(3) - (4)].double_range)))
		{
			yyerror("Inappropriate range for type.");
		}

		(yyval.u.parameter) = current_parameter = simple_param;
	}
    break;

  case 76:
/* Line 1787 of yacc.c  */
#line 579 "Parser.ypp"
    { (yyval.u.parameter) = param_with_modulus((yyvsp[(1) - (3)].u.parameter), (yyvsp[(3) - (3)].u.real)); }
    break;

  case 77:
/* Line 1787 of yacc.c  */
#line 580 "Parser.ypp"
    { (yyval.u.parameter) = param_with_modulus((yyvsp[(1) - (3)].u.parameter), (yyvsp[(3) - (3)].u.real)); }
    break;

  case 78:
/* Line 1787 of yacc.c  */
#line 584 "Parser.ypp"
    { (yyval.u.parameter) = param_with_divisor((yyvsp[(1) - (3)].u.parameter), (yyvsp[(3) - (3)].u.uint32)); }
    break;

  case 79:
/* Line 1787 of yacc.c  */
#line 585 "Parser.ypp"
    { (yyval.u.parameter) = param_with_divisor((yyvsp[(1) - (3)].u.parameter), (yyvsp[(3) - (3)].u.uint32)); }
    break;

  case 80:
/* Line 1787 of yacc.c  */
#line 586 "Parser.ypp"
    { (yyval.u.parameter) = param_with_divisor((yyvsp[(1) - (3)].u.parameter), (yyvsp[(3) - (3)].u.uint32)); }
    break;

  case 81:
/* Line 1787 of yacc.c  */
#line 590 "Parser.ypp"
    { (yyval.u.parameter) = (yyvsp[(1) - (2)].u.parameter); (yyvsp[(1) - (2)].u.parameter)->set_name((yyvsp[(2) - (2)].str)); }
    break;

  case 82:
/* Line 1787 of yacc.c  */
#line 591 "Parser.ypp"
    { (yyval.u.parameter) = (yyvsp[(1) - (2)].u.parameter); (yyvsp[(1) - (2)].u.parameter)->set_name((yyvsp[(2) - (2)].str)); }
    break;

  case 83:
/* Line 1787 of yacc.c  */
#line 592 "Parser.ypp"
    { (yyval.u.parameter) = (yyvsp[(1) - (2)].u.parameter); (yyvsp[(1) - (2)].u.parameter)->set_name((yyvsp[(2) - (2)].str)); }
    break;

  case 84:
/* Line 1787 of yacc.c  */
#line 593 "Parser.ypp"
    { (yyval.u.parameter) = (yyvsp[(1) - (2)].u.parameter); (yyvsp[(1) - (2)].u.parameter)->set_name((yyvsp[(2) - (2)].str)); }
    break;

  case 85:
/* Line 1787 of yacc.c  */
#line 597 "Parser.ypp"
    { (yyval.u.parameter) = (yyvsp[(1) - (4)].u.parameter)->append_array_specification((yyvsp[(3) - (4)].uint_range)); }
    break;

  case 86:
/* Line 1787 of yacc.c  */
#line 598 "Parser.ypp"
    { (yyval.u.parameter) = (yyvsp[(1) - (4)].u.parameter)->append_array_specification((yyvsp[(3) - (4)].uint_range)); }
    break;

  case 87:
/* Line 1787 of yacc.c  */
#line 599 "Parser.ypp"
    { (yyval.u.parameter) = (yyvsp[(1) - (4)].u.parameter)->append_array_specification((yyvsp[(3) - (4)].uint_range)); }
    break;

  case 88:
/* Line 1787 of yacc.c  */
#line 600 "Parser.ypp"
    { (yyval.u.parameter) = (yyvsp[(1) - (4)].u.parameter)->append_array_specification((yyvsp[(3) - (4)].uint_range)); }
    break;

  case 89:
/* Line 1787 of yacc.c  */
#line 604 "Parser.ypp"
    { (yyval.u.parameter) = (yyvsp[(1) - (4)].u.parameter)->append_array_specification((yyvsp[(3) - (4)].uint_range)); }
    break;

  case 90:
/* Line 1787 of yacc.c  */
#line 608 "Parser.ypp"
    { (yyval.u.parameter) = (yyvsp[(1) - (3)].u.parameter); (yyvsp[(1) - (3)].u.parameter)->set_default_value((yyvsp[(3) - (3)].str)); }
    break;

  case 91:
/* Line 1787 of yacc.c  */
#line 609 "Parser.ypp"
    { (yyval.u.parameter) = (yyvsp[(1) - (3)].u.parameter); (yyvsp[(1) - (3)].u.parameter)->set_default_value((yyvsp[(3) - (3)].str)); }
    break;

  case 92:
/* Line 1787 of yacc.c  */
#line 610 "Parser.ypp"
    { (yyval.u.parameter) = (yyvsp[(1) - (3)].u.parameter); (yyvsp[(1) - (3)].u.parameter)->set_default_value((yyvsp[(3) - (3)].str)); }
    break;

  case 93:
/* Line 1787 of yacc.c  */
#line 611 "Parser.ypp"
    { (yyval.u.parameter) = (yyvsp[(1) - (3)].u.parameter); (yyvsp[(1) - (3)].u.parameter)->set_default_value((yyvsp[(3) - (3)].str)); }
    break;

  case 94:
/* Line 1787 of yacc.c  */
#line 612 "Parser.ypp"
    { (yyval.u.parameter) = (yyvsp[(1) - (3)].u.parameter); (yyvsp[(1) - (3)].u.parameter)->set_default_value((yyvsp[(3) - (3)].str)); }
    break;

  case 95:
/* Line 1787 of yacc.c  */
#line 616 "Parser.ypp"
    { (yyval.u.parameter) = (yyvsp[(1) - (3)].u.parameter); (yyvsp[(1) - (3)].u.parameter)->set_default_value((yyvsp[(3) - (3)].str)); }
    break;

  case 96:
/* Line 1787 of yacc.c  */
#line 617 "Parser.ypp"
    { (yyval.u.parameter) = (yyvsp[(1) - (3)].u.parameter); (yyvsp[(1) - (3)].u.parameter)->set_default_value((yyvsp[(3) - (3)].str)); }
    break;

  case 97:
/* Line 1787 of yacc.c  */
#line 621 "Parser.ypp"
    { (yyval.double_range) = DoubleRange(); }
    break;

  case 98:
/* Line 1787 of yacc.c  */
#line 623 "Parser.ypp"
    {
		DoubleRange double_range;
		if(!double_range.add_range((yyvsp[(1) - (1)].u.real), (yyvsp[(1) - (1)].u.real)))
		{
			yyerror("Overlapping range");
		}
		(yyval.double_range) = double_range;
	}
    break;

  case 99:
/* Line 1787 of yacc.c  */
#line 632 "Parser.ypp"
    {
		DoubleRange double_range;
		if(!double_range.add_range((yyvsp[(1) - (3)].u.real), (yyvsp[(3) - (3)].u.real)))
		{
			yyerror("Overlapping range");
		}
		(yyval.double_range) = double_range;
	}
    break;

  case 100:
/* Line 1787 of yacc.c  */
#line 641 "Parser.ypp"
    {
		DoubleRange double_range;
		if((yyvsp[(2) - (2)].u.real) >= 0)
		{
			yyerror("Syntax error");
		}
		else if(!double_range.add_range((yyvsp[(1) - (2)].u.real), -(yyvsp[(2) - (2)].u.real)))
		{
			yyerror("Overlapping range");
		}
		(yyval.double_range) = double_range;
	}
    break;

  case 101:
/* Line 1787 of yacc.c  */
#line 654 "Parser.ypp"
    {
		if(!(yyvsp[(1) - (3)].double_range).add_range((yyvsp[(3) - (3)].u.real), (yyvsp[(3) - (3)].u.real)))
		{
			yyerror("Overlapping range");
		}
		(yyval.double_range) = (yyvsp[(1) - (3)].double_range);
	}
    break;

  case 102:
/* Line 1787 of yacc.c  */
#line 662 "Parser.ypp"
    {
		if(!(yyvsp[(1) - (5)].double_range).add_range((yyvsp[(3) - (5)].u.real), (yyvsp[(5) - (5)].u.real)))
		{
			yyerror("Overlapping range");
		}
		(yyval.double_range) = (yyvsp[(1) - (5)].double_range);
	}
    break;

  case 103:
/* Line 1787 of yacc.c  */
#line 670 "Parser.ypp"
    {
		if((yyvsp[(4) - (4)].u.real) >= 0)
		{
			yyerror("Syntax error");
		}
		else if(!(yyvsp[(1) - (4)].double_range).add_range((yyvsp[(3) - (4)].u.real), -(yyvsp[(4) - (4)].u.real)))
		{
			yyerror("Overlapping range");
		}
		(yyval.double_range) = (yyvsp[(1) - (4)].double_range);
	}
    break;

  case 104:
/* Line 1787 of yacc.c  */
#line 684 "Parser.ypp"
    { (yyval.uint_range) = UnsignedIntRange(); }
    break;

  case 105:
/* Line 1787 of yacc.c  */
#line 686 "Parser.ypp"
    {
		UnsignedIntRange uint_range;
		if(!uint_range.add_range((yyvsp[(1) - (1)].u.uint32), (yyvsp[(1) - (1)].u.uint32)))
		{
			yyerror("Overlapping range");
		}
		(yyval.uint_range) = uint_range;
	}
    break;

  case 106:
/* Line 1787 of yacc.c  */
#line 695 "Parser.ypp"
    {
		UnsignedIntRange uint_range;
		if(!uint_range.add_range((yyvsp[(1) - (3)].u.uint32), (yyvsp[(3) - (3)].u.uint32)))
		{
			yyerror("Overlapping range");
		}
		(yyval.uint_range) = uint_range;
	}
    break;

  case 107:
/* Line 1787 of yacc.c  */
#line 704 "Parser.ypp"
    {
		UnsignedIntRange uint_range;
		if(!uint_range.add_range((yyvsp[(1) - (2)].u.uint32), (yyvsp[(2) - (2)].u.uint32)))
		{
			yyerror("Overlapping range");
		}
		(yyval.uint_range) = uint_range;
	}
    break;

  case 108:
/* Line 1787 of yacc.c  */
#line 713 "Parser.ypp"
    {
		if(!(yyvsp[(1) - (3)].uint_range).add_range((yyvsp[(3) - (3)].u.uint32), (yyvsp[(3) - (3)].u.uint32)))
		{
			yyerror("Overlapping range");
		}
		(yyval.uint_range) = (yyvsp[(1) - (3)].uint_range);
	}
    break;

  case 109:
/* Line 1787 of yacc.c  */
#line 721 "Parser.ypp"
    {
		if(!(yyvsp[(1) - (5)].uint_range).add_range((yyvsp[(3) - (5)].u.uint32), (yyvsp[(5) - (5)].u.uint32)))
		{
			yyerror("Overlapping range");
		}
		(yyval.uint_range) = (yyvsp[(1) - (5)].uint_range);
	}
    break;

  case 110:
/* Line 1787 of yacc.c  */
#line 729 "Parser.ypp"
    {
		if(!(yyvsp[(1) - (4)].uint_range).add_range((yyvsp[(3) - (4)].u.uint32), (yyvsp[(4) - (4)].u.uint32)))
		{
			yyerror("Overlapping range");
		}
		(yyval.uint_range) = (yyvsp[(1) - (4)].uint_range);
	}
    break;

  case 111:
/* Line 1787 of yacc.c  */
#line 740 "Parser.ypp"
    {
		if((yyvsp[(1) - (1)].str).length() != 1)
		{
			yyerror("Single character required.");
			(yyval.u.uint32) = 0;
		}
		else {
			(yyval.u.uint32) = (unsigned char)(yyvsp[(1) - (1)].str)[0];
		}
	}
    break;

  case 113:
/* Line 1787 of yacc.c  */
#line 755 "Parser.ypp"
    {
		(yyval.u.uint32) = (unsigned int)(yyvsp[(1) - (1)].u.uint64);
		if((yyval.u.uint32) != (yyvsp[(1) - (1)].u.uint64))
		{
			yyerror("Number out of range.");
			(yyval.u.uint32) = 1;
		}
	}
    break;

  case 114:
/* Line 1787 of yacc.c  */
#line 767 "Parser.ypp"
    {
		(yyval.u.uint32) = (unsigned int) - (yyvsp[(1) - (1)].u.int64);
		if((yyvsp[(1) - (1)].u.int64) >= 0)
		{
			yyerror("Syntax error while parsing small_negative_integer.");
		}
		else if((yyval.u.uint32) != -(yyvsp[(1) - (1)].u.int64))
		{
			yyerror("Number out of range.");
			(yyval.u.uint32) = 1;
		}
	}
    break;

  case 115:
/* Line 1787 of yacc.c  */
#line 782 "Parser.ypp"
    { (yyval.u.real) = (double)(yyvsp[(1) - (1)].u.uint64); }
    break;

  case 116:
/* Line 1787 of yacc.c  */
#line 783 "Parser.ypp"
    { (yyval.u.real) = (double)(yyvsp[(1) - (1)].u.int64); }
    break;

  case 118:
/* Line 1787 of yacc.c  */
#line 789 "Parser.ypp"
    {
		if((yyvsp[(1) - (1)].str).length() != 1)
		{
			yyerror("Single character required.");
			(yyval.u.real) = 0;
		}
		else
		{
			(yyval.u.real) = (double)(unsigned char)(yyvsp[(1) - (1)].str)[0];
		}
	}
    break;

  case 120:
/* Line 1787 of yacc.c  */
#line 805 "Parser.ypp"
    {
		// TODO: check for range limits
		(yyval.str) = std::string((char*)&(yyvsp[(1) - (1)].u.int64), sizeof(int64_t));
	}
    break;

  case 121:
/* Line 1787 of yacc.c  */
#line 810 "Parser.ypp"
    {
		// TODO: check for range limits
		(yyval.str) = std::string((char*)&(yyvsp[(1) - (1)].u.uint64), sizeof(uint64_t));
	}
    break;

  case 122:
/* Line 1787 of yacc.c  */
#line 815 "Parser.ypp"
    {
		// TODO: check for range limits
		(yyval.str) = std::string((char*)&(yyvsp[(1) - (1)].u.real), sizeof(double));
	}
    break;

  case 123:
/* Line 1787 of yacc.c  */
#line 820 "Parser.ypp"
    {
		DataType type = current_parameter->get_type();
		if(type == DT_string)
		{
			if((yyvsp[(1) - (1)].str).length() != current_parameter->get_size())
			{
				yyerror("Default value for fixed-length string has incorrect length.");
		 		(yyval.str) = (yyvsp[(1) - (1)].str);
			}
		}
		else // DT_varstring
		{
			// TODO: Check for range limits
			// Prepend length tag
			sizetag_t length = (yyvsp[(1) - (1)].str).length();
			(yyval.str) = std::string((char*)&length, sizeof(sizetag_t)) + (yyvsp[(1) - (1)].str);
		}
	}
    break;

  case 124:
/* Line 1787 of yacc.c  */
#line 839 "Parser.ypp"
    {
		// TODO: check for range limits... maybe?
		(yyval.str) = (yyvsp[(1) - (1)].str);
	}
    break;

  case 125:
/* Line 1787 of yacc.c  */
#line 844 "Parser.ypp"
    {
		DataType type = current_parameter->get_type();
		if(type == DT_vararray)
		{
			sizetag_t length = (yyvsp[(2) - (3)].str).length();
			(yyval.str) = std::string((char*)&length, sizeof(sizetag_t)) + (yyvsp[(2) - (3)].str);
		}
		else // DT_array
		{
			// TODO: Check range limits
			(yyval.str) = (yyvsp[(2) - (3)].str);
		}
	}
    break;

  case 126:
/* Line 1787 of yacc.c  */
#line 858 "Parser.ypp"
    {
		DataType type = current_parameter->get_type();
		if(type == DT_vararray)
		{
			sizetag_t length = (yyvsp[(2) - (3)].str).length();
			(yyval.str) = std::string((char*)&length, sizeof(sizetag_t)) + (yyvsp[(2) - (3)].str);
		}
		else if(type == DT_struct)
		{
			yywarning("The {val} format is still parsed as an array value, values may not be properly validated or packed for a struct.");
			(yyval.str) = (yyvsp[(2) - (3)].str);
		}
		else // DT_array
		{
			// TODO: Check range limits
			(yyval.str) = (yyvsp[(2) - (3)].str);
		}
	}
    break;

  case 127:
/* Line 1787 of yacc.c  */
#line 885 "Parser.ypp"
    {
		// TODO: Check array type matches parameter type
		(yyval.str) = (yyvsp[(1) - (1)].str);
	}
    break;

  case 128:
/* Line 1787 of yacc.c  */
#line 890 "Parser.ypp"
    {
		// TODO: Check array type compatible with SIGNED_INTEGER
		std::string val;
		for(unsigned int i = 0; i < (yyvsp[(3) - (3)].u.uint32); i++)
		{
			val.append((char*)&(yyvsp[(1) - (3)].u.int64), sizeof(int64_t));
		}
		(yyval.str) = val;
	}
    break;

  case 129:
/* Line 1787 of yacc.c  */
#line 900 "Parser.ypp"
    {
		// TODO: Check array type compatible with UNSIGNED_INTEGER
		std::string val;
		for(unsigned int i = 0; i < (yyvsp[(3) - (3)].u.uint32); i++)
		{
			val.append((char*)&(yyvsp[(1) - (3)].u.uint64), sizeof(uint64_t));
		}
		(yyval.str) = val;
	}
    break;

  case 130:
/* Line 1787 of yacc.c  */
#line 910 "Parser.ypp"
    {
		// TODO: Check array type compatible with REAL
		std::string val;
		for(unsigned int i = 0; i < (yyvsp[(3) - (3)].u.uint32); i++)
		{
			val.append((char*)&(yyvsp[(1) - (3)].u.real), sizeof(double));
		}
		(yyval.str) = val;
	}
    break;

  case 131:
/* Line 1787 of yacc.c  */
#line 920 "Parser.ypp"
    {
		// TODO: Check array type compatible with HEX_STRING
		std::string val;
		for(unsigned int i = 0; i < (yyvsp[(3) - (3)].u.uint32); i++)
		{
			val += (yyvsp[(1) - (3)].str);
		}
		(yyval.str) = val;
	}
    break;

  case 132:
/* Line 1787 of yacc.c  */
#line 930 "Parser.ypp"
    {
		// TODO: Check array type matches parameter type
		(yyval.str) = (yyvsp[(1) - (3)].str) + (yyvsp[(3) - (3)].str);
	}
    break;

  case 133:
/* Line 1787 of yacc.c  */
#line 937 "Parser.ypp"
    { (yyval.u.datatype) = DT_int8; }
    break;

  case 134:
/* Line 1787 of yacc.c  */
#line 938 "Parser.ypp"
    { (yyval.u.datatype) = DT_int16; }
    break;

  case 135:
/* Line 1787 of yacc.c  */
#line 939 "Parser.ypp"
    { (yyval.u.datatype) = DT_int32; }
    break;

  case 136:
/* Line 1787 of yacc.c  */
#line 940 "Parser.ypp"
    { (yyval.u.datatype) = DT_int64; }
    break;

  case 137:
/* Line 1787 of yacc.c  */
#line 941 "Parser.ypp"
    { (yyval.u.datatype) = DT_uint8; }
    break;

  case 138:
/* Line 1787 of yacc.c  */
#line 942 "Parser.ypp"
    { (yyval.u.datatype) = DT_uint16; }
    break;

  case 139:
/* Line 1787 of yacc.c  */
#line 943 "Parser.ypp"
    { (yyval.u.datatype) = DT_uint32; }
    break;

  case 140:
/* Line 1787 of yacc.c  */
#line 944 "Parser.ypp"
    { (yyval.u.datatype) = DT_uint64; }
    break;

  case 141:
/* Line 1787 of yacc.c  */
#line 945 "Parser.ypp"
    { (yyval.u.datatype) = DT_float32; }
    break;

  case 142:
/* Line 1787 of yacc.c  */
#line 946 "Parser.ypp"
    { (yyval.u.datatype) = DT_float64; }
    break;

  case 143:
/* Line 1787 of yacc.c  */
#line 947 "Parser.ypp"
    { (yyval.u.datatype) = DT_string; }
    break;

  case 144:
/* Line 1787 of yacc.c  */
#line 948 "Parser.ypp"
    { (yyval.u.datatype) = DT_blob; }
    break;

  case 145:
/* Line 1787 of yacc.c  */
#line 949 "Parser.ypp"
    { (yyval.u.datatype) = DT_char; }
    break;

  case 146:
/* Line 1787 of yacc.c  */
#line 953 "Parser.ypp"
    { current_keyword_list.clear_keywords(); }
    break;

  case 147:
/* Line 1787 of yacc.c  */
#line 954 "Parser.ypp"
    { current_keyword_list.add_keyword((yyvsp[(2) - (2)].u.keyword)); }
    break;

  case 148:
/* Line 1787 of yacc.c  */
#line 959 "Parser.ypp"
    {
		if(current_keyword_list.get_num_keywords() != 0)
		{
			yyerror("Keywords are not allowed here.");
		}
	}
    break;

  case 149:
/* Line 1787 of yacc.c  */
#line 968 "Parser.ypp"
    { current_molecular = new MolecularField((yyvsp[(1) - (2)].str), current_class); }
    break;

  case 150:
/* Line 1787 of yacc.c  */
#line 969 "Parser.ypp"
    { (yyval.u.field) = current_molecular; }
    break;

  case 151:
/* Line 1787 of yacc.c  */
#line 974 "Parser.ypp"
    {
		Field *field = current_class->get_field_by_name((yyvsp[(1) - (1)].str));
		(yyval.u.atomic) = (AtomicField *)NULL;
		if(field == (Field *)NULL)
		{
			// Maybe the field is unknown because the class is partially
			// bogus.  In that case, allow it for now; create a bogus field as
			// a placeholder.
			if(current_class->inherits_from_bogus_class())
			{
				(yyval.u.atomic) = new AtomicField((yyvsp[(1) - (1)].str), current_class, true);
				current_class->add_field((yyval.u.atomic));

			}
			else
			{
				// Nope, it's a fully-defined class, so this is a real error.
				yyerror("Unknown field: " + (yyvsp[(1) - (1)].str));
			}

		}
		else {
			(yyval.u.atomic) = field->as_atomic_field();
			if((yyval.u.atomic) == (AtomicField *)NULL)
			{
				yyerror("Not an atomic field: " + (yyvsp[(1) - (1)].str));
			}
		}
	}
    break;

  case 152:
/* Line 1787 of yacc.c  */
#line 1007 "Parser.ypp"
    {
		if((yyvsp[(1) - (1)].u.atomic) != (AtomicField *)NULL)
		{
			current_molecular->add_atomic((yyvsp[(1) - (1)].u.atomic));
		}
	}
    break;

  case 153:
/* Line 1787 of yacc.c  */
#line 1014 "Parser.ypp"
    {
		if((yyvsp[(3) - (3)].u.atomic) != (AtomicField *)NULL)
		{
			current_molecular->add_atomic((yyvsp[(3) - (3)].u.atomic));
			if(!(yyvsp[(3) - (3)].u.atomic)->is_bogus_field() && !current_molecular->compare_keywords(*(yyvsp[(3) - (3)].u.atomic)))
			{
				yyerror("Mismatched keywords in molecule between " +
				current_molecular->get_atomic(0)->get_name() + " and " +
				(yyvsp[(3) - (3)].u.atomic)->get_name());
			}
		}
	}
    break;


/* Line 1787 of yacc.c  */
#line 2879 "Parser.cpp"
      default: break;
    }
  /* User semantic actions sometimes alter yychar, and that requires
     that yytoken be updated with the new translation.  We take the
     approach of translating immediately before every use of yytoken.
     One alternative is translating here after every semantic action,
     but that translation would be missed if the semantic action invokes
     YYABORT, YYACCEPT, or YYERROR immediately after altering yychar or
     if it invokes YYBACKUP.  In the case of YYABORT or YYACCEPT, an
     incorrect destructor might then be invoked immediately.  In the
     case of YYERROR or YYBACKUP, subsequent parser actions might lead
     to an incorrect destructor call or verbose syntax error message
     before the lookahead is translated.  */
  YY_SYMBOL_PRINT ("-> $$ =", yyr1[yyn], &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);

  *++yyvsp = yyval;

  /* Now `shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTOKENS] + *yyssp;
  if (0 <= yystate && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTOKENS];

  goto yynewstate;


/*------------------------------------.
| yyerrlab -- here on detecting error |
`------------------------------------*/
yyerrlab:
  /* Make sure we have latest lookahead translation.  See comments at
     user semantic actions for why this is necessary.  */
  yytoken = yychar == YYEMPTY ? YYEMPTY : YYTRANSLATE (yychar);

  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if ! YYERROR_VERBOSE
      yyerror (YY_("syntax error"));
#else
# define YYSYNTAX_ERROR yysyntax_error (&yymsg_alloc, &yymsg, \
                                        yyssp, yytoken)
      {
        char const *yymsgp = YY_("syntax error");
        int yysyntax_error_status;
        yysyntax_error_status = YYSYNTAX_ERROR;
        if (yysyntax_error_status == 0)
          yymsgp = yymsg;
        else if (yysyntax_error_status == 1)
          {
            if (yymsg != yymsgbuf)
              YYSTACK_FREE (yymsg);
            yymsg = (char *) YYSTACK_ALLOC (yymsg_alloc);
            if (!yymsg)
              {
                yymsg = yymsgbuf;
                yymsg_alloc = sizeof yymsgbuf;
                yysyntax_error_status = 2;
              }
            else
              {
                yysyntax_error_status = YYSYNTAX_ERROR;
                yymsgp = yymsg;
              }
          }
        yyerror (yymsgp);
        if (yysyntax_error_status == 2)
          goto yyexhaustedlab;
      }
# undef YYSYNTAX_ERROR
#endif
    }



  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
	 error, discard it.  */

      if (yychar <= YYEOF)
	{
	  /* Return failure if at end of input.  */
	  if (yychar == YYEOF)
	    YYABORT;
	}
      else
	{
	  yydestruct ("Error: discarding",
		      yytoken, &yylval);
	  yychar = YYEMPTY;
	}
    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:

  /* Pacify compilers like GCC when the user code never invokes
     YYERROR and the label yyerrorlab therefore never appears in user
     code.  */
  if (/*CONSTCOND*/ 0)
     goto yyerrorlab;

  /* Do not reclaim the symbols of the rule which action triggered
     this YYERROR.  */
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;	/* Each real token shifted decrements this.  */

  for (;;)
    {
      yyn = yypact[yystate];
      if (!yypact_value_is_default (yyn))
	{
	  yyn += YYTERROR;
	  if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYTERROR)
	    {
	      yyn = yytable[yyn];
	      if (0 < yyn)
		break;
	    }
	}

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
	YYABORT;


      yydestruct ("Error: popping",
		  yystos[yystate], yyvsp);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END


  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", yystos[yyn], yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturn;

/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturn;

#if !defined yyoverflow || YYERROR_VERBOSE
/*-------------------------------------------------.
| yyexhaustedlab -- memory exhaustion comes here.  |
`-------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
  if (yychar != YYEMPTY)
    {
      /* Make sure we have latest lookahead translation.  See comments at
         user semantic actions for why this is necessary.  */
      yytoken = YYTRANSLATE (yychar);
      yydestruct ("Cleanup: discarding lookahead",
                  yytoken, &yylval);
    }
  /* Do not reclaim the symbols of the rule which action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
		  yystos[*yyssp], yyvsp);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
#if YYERROR_VERBOSE
  if (yymsg != yymsgbuf)
    YYSTACK_FREE (yymsg);
#endif
  /* Make sure YYID is used.  */
  return YYID (yyresult);
}


/* Line 2050 of yacc.c  */
#line 1033 "Parser.ypp"
 /* Start helper function section */


Parameter* param_with_modulus(Parameter* p, double mod)
{
	SimpleParameter *simple_param = p->as_simple_parameter();
	if(simple_param == NULL
	|| simple_param->get_typedef() != (Typedef*)NULL
	|| !simple_param->is_numeric_type())
	{
		yyerror("A modulus is only valid on a numeric type.");
	}
	else if(!simple_param->set_modulus(mod))
	{
		yyerror("Invalid modulus.");
	}

	return simple_param;
}

Parameter* param_with_divisor(Parameter* p, uint32_t div)
{
	SimpleParameter *simple_param = p->as_simple_parameter();
	if(simple_param == NULL
	|| simple_param->get_typedef() != (Typedef*)NULL
	|| !simple_param->is_numeric_type())
	{
		yyerror("A divisor is only valid on a numeric type.");

	}
	else if(!simple_param->set_divisor(div))
	{
		yyerror("Invalid divisor.");
	}

	return simple_param;	
}
