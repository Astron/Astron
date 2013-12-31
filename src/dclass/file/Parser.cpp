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
#include "../StructParameter.h"
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

	static Struct *current_class = (Struct*)NULL;
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
#define YYLAST   216

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  45
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  57
/* YYNRULES -- Number of rules.  */
#define YYNRULES  140
/* YYNRULES -- Number of states.  */
#define YYNSTATES  218

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
      26,    29,    30,    36,    38,    42,    44,    48,    50,    52,
      54,    58,    61,    64,    66,    69,    72,    73,    81,    83,
      85,    88,    90,    94,    96,    99,   103,   106,   109,   112,
     115,   116,   123,   125,   128,   131,   134,   137,   138,   144,
     146,   148,   150,   154,   156,   158,   160,   162,   164,   166,
     168,   170,   172,   174,   176,   178,   180,   182,   187,   191,
     195,   199,   203,   207,   210,   213,   216,   219,   224,   229,
     234,   239,   244,   248,   252,   256,   260,   264,   268,   272,
     274,   276,   280,   283,   285,   287,   291,   294,   296,   298,
     300,   302,   304,   306,   308,   310,   312,   314,   316,   318,
     320,   322,   326,   330,   332,   336,   340,   344,   348,   352,
     354,   356,   358,   360,   362,   364,   366,   368,   370,   372,
     374,   376,   378,   380,   383,   385,   386,   391,   393,   395,
     399
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int8 yyrhs[] =
{
      46,     0,    -1,    10,    47,    -1,   101,    -1,    47,    30,
      -1,    47,    57,    -1,    47,    64,    -1,    47,    48,    -1,
      47,    54,    -1,    47,    55,    -1,    14,    50,    -1,    -1,
      13,    50,    14,    49,    52,    -1,    51,    -1,    50,    31,
      51,    -1,     8,    -1,    51,    32,     8,    -1,    53,    -1,
      33,    -1,    51,    -1,    53,    34,    51,    -1,    15,    73,
      -1,    16,    56,    -1,   101,    -1,    56,     8,    -1,    56,
       9,    -1,    -1,    11,     8,    58,    60,    35,    62,    36,
      -1,     8,    -1,   101,    -1,    37,    61,    -1,    59,    -1,
      61,    34,    59,    -1,   101,    -1,    62,    30,    -1,    62,
      63,    30,    -1,    68,    95,    -1,    97,    96,    -1,    74,
      95,    -1,    73,    95,    -1,    -1,    12,     8,    65,    35,
      66,    36,    -1,   101,    -1,    66,    30,    -1,    66,    67,
      -1,    74,    30,    -1,    73,    30,    -1,    -1,     8,    38,
      69,    70,    39,    -1,   101,    -1,    71,    -1,    72,    -1,
      71,    34,    72,    -1,    75,    -1,    80,    -1,    82,    -1,
      84,    -1,    76,    -1,    77,    -1,    78,    -1,    79,    -1,
      81,    -1,    83,    -1,    73,    -1,    74,    -1,    94,    -1,
       8,    -1,    94,    38,    85,    39,    -1,    76,    40,    90,
      -1,    77,    40,    90,    -1,    76,    32,    88,    -1,    77,
      32,    88,    -1,    78,    32,    88,    -1,    76,     8,    -1,
      77,     8,    -1,    78,     8,    -1,    79,     8,    -1,    76,
      41,    86,    42,    -1,    77,    41,    86,    42,    -1,    78,
      41,    86,    42,    -1,    79,    41,    86,    42,    -1,    80,
      41,    86,    42,    -1,    76,    43,    92,    -1,    77,    43,
      92,    -1,    78,    43,    92,    -1,    79,    43,    92,    -1,
      81,    43,    92,    -1,    80,    43,    92,    -1,    82,    43,
      92,    -1,   101,    -1,    91,    -1,    91,    44,    91,    -1,
      91,    90,    -1,   101,    -1,    87,    -1,    87,    44,    87,
      -1,    87,    89,    -1,     6,    -1,    88,    -1,     3,    -1,
       4,    -1,     3,    -1,     4,    -1,     5,    -1,     6,    -1,
      90,    -1,     4,    -1,     3,    -1,     5,    -1,     6,    -1,
       7,    -1,    41,    93,    42,    -1,    35,    93,    36,    -1,
      92,    -1,     4,    33,    88,    -1,     3,    33,    88,    -1,
       5,    33,    88,    -1,     7,    33,    88,    -1,    93,    34,
      92,    -1,    17,    -1,    18,    -1,    19,    -1,    20,    -1,
      21,    -1,    22,    -1,    23,    -1,    24,    -1,    25,    -1,
      26,    -1,    27,    -1,    28,    -1,    29,    -1,   101,    -1,
      95,     9,    -1,    95,    -1,    -1,     8,    37,    98,   100,
      -1,     8,    -1,    99,    -1,   100,    34,    99,    -1,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   151,   151,   155,   156,   157,   164,   171,   172,   173,
     177,   182,   181,   190,   191,   198,   199,   207,   208,   212,
     213,   217,   231,   235,   236,   240,   252,   251,   263,   291,
     292,   296,   303,   313,   314,   315,   329,   341,   342,   351,
     363,   362,   373,   374,   375,   389,   390,   395,   394,   413,
     414,   418,   419,   423,   433,   434,   435,   439,   440,   441,
     442,   443,   444,   448,   449,   453,   454,   479,   497,   498,
     502,   503,   504,   508,   509,   510,   511,   515,   516,   517,
     518,   522,   526,   527,   528,   529,   530,   534,   535,   539,
     540,   541,   542,   546,   547,   548,   549,   553,   564,   568,
     580,   596,   597,   598,   602,   614,   618,   623,   628,   633,
     652,   657,   671,   698,   703,   713,   723,   733,   743,   751,
     752,   753,   754,   755,   756,   757,   758,   759,   760,   761,
     762,   763,   767,   768,   772,   782,   782,   787,   807,   814,
     829
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
  "typedef_decl", "keyword_decl", "keyword_decl_list", "dclass", "@2",
  "dclass_name", "dclass_inheritance", "dclass_parents", "dclass_fields",
  "dclass_field", "dstruct", "$@3", "struct_fields", "struct_field",
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
       0,    45,    46,    47,    47,    47,    47,    47,    47,    47,
      48,    49,    48,    50,    50,    51,    51,    52,    52,    53,
      53,    54,    55,    56,    56,    56,    58,    57,    59,    60,
      60,    61,    61,    62,    62,    62,    63,    63,    63,    63,
      65,    64,    66,    66,    66,    67,    67,    69,    68,    70,
      70,    71,    71,    72,    73,    73,    73,    74,    74,    74,
      74,    74,    74,    75,    75,    76,    76,    77,    78,    78,
      79,    79,    79,    80,    80,    80,    80,    81,    81,    81,
      81,    82,    83,    83,    83,    83,    83,    84,    84,    85,
      85,    85,    85,    86,    86,    86,    86,    87,    87,    88,
      89,    90,    90,    90,    91,    91,    92,    92,    92,    92,
      92,    92,    92,    93,    93,    93,    93,    93,    93,    94,
      94,    94,    94,    94,    94,    94,    94,    94,    94,    94,
      94,    94,    95,    95,    96,    98,    97,    99,   100,   100,
     101
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     2,     1,     2,     2,     2,     2,     2,     2,
       2,     0,     5,     1,     3,     1,     3,     1,     1,     1,
       3,     2,     2,     1,     2,     2,     0,     7,     1,     1,
       2,     1,     3,     1,     2,     3,     2,     2,     2,     2,
       0,     6,     1,     2,     2,     2,     2,     0,     5,     1,
       1,     1,     3,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     4,     3,     3,
       3,     3,     3,     2,     2,     2,     2,     4,     4,     4,
       4,     4,     3,     3,     3,     3,     3,     3,     3,     1,
       1,     3,     2,     1,     1,     3,     2,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     3,     3,     1,     3,     3,     3,     3,     3,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     2,     1,     0,     4,     1,     1,     3,
       0
};

/* YYDEFACT[STATE-NAME] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       0,   140,     0,     2,     3,     1,     0,     0,     0,     0,
       0,   140,     4,     7,     8,     9,     5,     6,    26,    40,
      15,     0,    13,    10,    66,   119,   120,   121,   122,   123,
     124,   125,   126,   127,   128,   129,   130,   131,    21,     0,
       0,     0,     0,    54,    55,    56,    65,    22,    23,   140,
       0,    11,     0,     0,    73,     0,     0,    74,     0,     0,
      75,     0,    76,   140,     0,     0,   140,    24,    25,     0,
       0,    29,   140,     0,    14,    16,    99,    70,   101,   102,
     103,    68,    71,    69,    72,    97,     0,    94,    98,    93,
     107,   106,   108,   109,   110,     0,     0,    87,    88,   104,
       0,   105,    90,    89,    28,    31,    30,   140,     0,    42,
      18,    19,    12,    17,    81,   100,     0,    96,   107,   106,
     108,   110,   113,     0,     0,    67,     0,    92,     0,     0,
      33,    43,    41,    44,     0,     0,    57,    58,    59,    60,
      61,    62,     0,    95,     0,     0,     0,     0,     0,   112,
     111,    91,    32,    66,    34,    27,     0,   140,   140,   140,
     140,    46,    45,   140,     0,   140,     0,   140,     0,   140,
       0,     0,    20,   115,   114,   116,   117,   118,   135,    47,
      35,    36,   132,    39,    38,   134,    37,     0,    82,     0,
      83,     0,    84,     0,    85,    86,     0,   140,   133,    77,
      78,    79,    80,   137,   138,   136,     0,    50,    51,    63,
      64,    53,    49,     0,    48,     0,   139,    52
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     2,     3,    13,    73,    21,    22,   112,   113,    14,
      15,    47,    16,    49,   105,    70,   106,   129,   156,    17,
      50,   108,   133,   157,   197,   206,   207,   208,   209,   210,
     211,   136,   137,   138,   139,    43,   140,    44,   141,    45,
     100,    86,    87,    88,   117,   101,   102,   122,   123,    46,
     181,   186,   160,   196,   204,   205,    89
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -86
static const yytype_int16 yypact[] =
{
       1,   -86,    41,   186,   -86,   -86,    49,    66,    71,    71,
     166,   -86,   -86,   -86,   -86,   -86,   -86,   -86,   -86,   -86,
     -86,    12,    57,    59,   -86,   -86,   -86,   -86,   -86,   -86,
     -86,   -86,   -86,   -86,   -86,   -86,   -86,   -86,   -86,    44,
      45,    26,    87,    -4,    54,   -86,    63,    16,   -86,    74,
      73,   -86,    71,   111,   -86,   132,    58,   -86,   132,    58,
     -86,   132,   -86,    30,    15,    15,   110,   -86,   -86,   128,
     102,   -86,   -86,     9,    57,   -86,   -86,   -86,   -86,   -86,
     -86,   -86,   -86,   -86,   -86,   -86,    96,     3,   -86,   -86,
     -86,   -86,   -86,   -86,   -86,    25,    25,   -86,   -86,   -86,
     100,   -86,    10,   -86,   -86,   -86,   107,   -86,   104,   -86,
     -86,    57,   -86,   108,   -86,   -86,    30,   -86,   112,   113,
     114,   115,   -86,    52,     4,   -86,   110,   -86,   128,   143,
     -86,   -86,   -86,   -86,   119,   120,    27,    32,    37,     8,
     101,   -86,    71,   -86,   132,   132,   132,   132,    15,   -86,
     -86,   -86,   -86,    80,   -86,   -86,   122,   -86,   -86,   -86,
     -86,   -86,   -86,    30,    15,    30,    15,    30,    15,    30,
      15,    15,    57,   -86,   -86,   -86,   -86,   -86,   -86,   -86,
     -86,   134,   -86,   134,   134,   134,   -86,   133,   -86,   135,
     -86,   136,   -86,   138,   -86,   -86,   145,   166,   -86,   -86,
     -86,   -86,   -86,   -86,   -86,   121,   137,   147,   -86,   -86,
     -86,   -86,   -86,   145,   -86,   166,   -86,   -86
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
     -86,   -86,   -86,   -86,   -86,   173,   -46,   -86,   -86,   -86,
     -86,   -86,   -86,   -86,    75,   -86,   -86,   -86,   -86,   -86,
     -86,   -86,   -86,   -86,   -86,   -86,   -86,   -11,    -9,   -85,
     -86,   144,   195,   196,   197,   -86,   -86,   -86,   -86,   -86,
     -86,   -65,    92,   -53,   -86,   -47,    83,   -61,   116,   -86,
     -77,   -86,   -86,   -86,    -3,   -86,    -1
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -1
static const yytype_uint8 yytable[] =
{
       4,    38,    77,    97,    98,    82,    74,   115,    84,    81,
      48,     1,    83,    78,    79,    80,    62,    20,    90,    91,
      92,    93,    94,   135,    67,    68,    51,   111,   118,   119,
     120,    93,   121,    76,    60,    54,    85,    63,   148,    64,
      57,     5,   110,    52,   159,    60,   150,   116,    71,   169,
      95,   170,    54,    57,   126,   127,    96,    18,    61,    55,
      95,    78,    79,    80,    58,   103,    96,    56,   163,    61,
     164,   109,    59,   165,    19,   166,    55,    58,   167,    20,
     168,   183,   184,   185,    56,    59,   148,   177,   149,    53,
      52,   173,   174,   175,   176,    62,   172,    65,   187,   134,
     189,    66,   191,   188,   193,   190,   130,   192,    72,   194,
     195,    69,    24,    78,    79,    80,    99,   178,   179,    75,
     158,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      34,    35,    36,    37,   131,    76,   104,   107,   114,   125,
     132,   128,   142,   198,   171,   144,   145,   146,   147,   161,
     162,   153,   180,   203,    39,   213,   182,   182,   182,   182,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,   154,    24,   199,   214,   200,   201,   155,
     202,   215,    23,    25,    26,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,   212,     6,     7,     8,
       9,    10,    11,   152,   217,    40,    41,    42,   143,   151,
     216,     0,   124,     0,     0,     0,    12
};

#define yypact_value_is_default(Yystate) \
  (!!((Yystate) == (-86)))

#define yytable_value_is_error(Yytable_value) \
  YYID (0)

static const yytype_int16 yycheck[] =
{
       1,    10,    55,    64,    65,    58,    52,     4,    61,    56,
      11,    10,    59,     3,     4,     5,     8,     8,     3,     4,
       5,     6,     7,   108,     8,     9,    14,    73,     3,     4,
       5,     6,     7,     3,     8,     8,     6,    41,    34,    43,
       8,     0,    33,    31,   129,     8,    42,    44,    49,    41,
      35,    43,     8,     8,    44,   102,    41,     8,    32,    32,
      35,     3,     4,     5,    32,    66,    41,    40,    41,    32,
      43,    72,    40,    41,     8,    43,    32,    32,    41,     8,
      43,   158,   159,   160,    40,    40,    34,   148,    36,    32,
      31,   144,   145,   146,   147,     8,   142,    43,   163,   108,
     165,    38,   167,   164,   169,   166,   107,   168,    35,   170,
     171,    37,     8,     3,     4,     5,     6,    37,    38,     8,
     129,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,    30,     3,     8,    35,    42,    39,
      36,    34,    34,     9,    43,    33,    33,    33,    33,    30,
      30,     8,    30,     8,    10,    34,   157,   158,   159,   160,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,    30,     8,    42,    39,    42,    42,    36,
      42,    34,     9,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,   197,    11,    12,    13,
      14,    15,    16,   128,   215,    10,    10,    10,   116,   126,
     213,    -1,    96,    -1,    -1,    -1,    30
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,    10,    46,    47,   101,     0,    11,    12,    13,    14,
      15,    16,    30,    48,    54,    55,    57,    64,     8,     8,
       8,    50,    51,    50,     8,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    73,    76,
      77,    78,    79,    80,    82,    84,    94,    56,   101,    58,
      65,    14,    31,    32,     8,    32,    40,     8,    32,    40,
       8,    32,     8,    41,    43,    43,    38,     8,     9,    37,
      60,   101,    35,    49,    51,     8,     3,    88,     3,     4,
       5,    90,    88,    90,    88,     6,    86,    87,    88,   101,
       3,     4,     5,     6,     7,    35,    41,    92,    92,     6,
      85,    90,    91,   101,     8,    59,    61,    35,    66,   101,
      33,    51,    52,    53,    42,     4,    44,    89,     3,     4,
       5,     7,    92,    93,    93,    39,    44,    90,    34,    62,
     101,    30,    36,    67,    73,    74,    76,    77,    78,    79,
      81,    83,    34,    87,    33,    33,    33,    33,    34,    36,
      42,    91,    59,     8,    30,    36,    63,    68,    73,    74,
      97,    30,    30,    41,    43,    41,    43,    41,    43,    41,
      43,    43,    51,    88,    88,    88,    88,    92,    37,    38,
      30,    95,   101,    95,    95,    95,    96,    86,    92,    86,
      92,    86,    92,    86,    92,    92,    98,    69,     9,    42,
      42,    42,    42,     8,    99,   100,    70,    71,    72,    73,
      74,    75,   101,    34,    39,    34,    99,    72
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
#line 158 "Parser.ypp"
    {
		if(!dc_file->add_class((yyvsp[(2) - (2)].u.dclass)))
		{
			yyerror("Duplicate class name: " + (yyvsp[(2) - (2)].u.dclass)->get_name());
		}
	}
    break;

  case 6:
/* Line 1787 of yacc.c  */
#line 165 "Parser.ypp"
    {
		if(!dc_file->add_struct((yyvsp[(2) - (2)].u.dstruct)))
		{
			yyerror("Duplicate class name: " + (yyvsp[(2) - (2)].u.dstruct)->get_name());
		}
	}
    break;

  case 10:
/* Line 1787 of yacc.c  */
#line 178 "Parser.ypp"
    {
		dc_file->add_import_module((yyvsp[(2) - (2)].str));
	}
    break;

  case 11:
/* Line 1787 of yacc.c  */
#line 182 "Parser.ypp"
    {
		dc_file->add_import_module((yyvsp[(2) - (3)].str));
	}
    break;

  case 14:
/* Line 1787 of yacc.c  */
#line 192 "Parser.ypp"
    {
		(yyval.str) = (yyvsp[(1) - (3)].str) + std::string(".") + (yyvsp[(3) - (3)].str);
	}
    break;

  case 16:
/* Line 1787 of yacc.c  */
#line 200 "Parser.ypp"
    {
		(yyval.str) = (yyvsp[(1) - (3)].str) + std::string("/") + (yyvsp[(3) - (3)].str);
	}
    break;

  case 18:
/* Line 1787 of yacc.c  */
#line 208 "Parser.ypp"
    { dc_file->add_import_symbol("*"); }
    break;

  case 19:
/* Line 1787 of yacc.c  */
#line 212 "Parser.ypp"
    { dc_file->add_import_symbol((yyvsp[(1) - (1)].str)); }
    break;

  case 20:
/* Line 1787 of yacc.c  */
#line 213 "Parser.ypp"
    { dc_file->add_import_symbol((yyvsp[(3) - (3)].str)); }
    break;

  case 21:
/* Line 1787 of yacc.c  */
#line 218 "Parser.ypp"
    {
		if((yyvsp[(2) - (2)].u.parameter) != (Parameter*)NULL)
		{
			Typedef *dtypedef = new Typedef((yyvsp[(2) - (2)].u.parameter));
			if(!dc_file->add_typedef(dtypedef))
			{
				yyerror("Duplicate typedef name: " + dtypedef->get_name());
			}
		}
	}
    break;

  case 24:
/* Line 1787 of yacc.c  */
#line 237 "Parser.ypp"
    {
	dc_file->add_keyword((yyvsp[(2) - (2)].str));
}
    break;

  case 25:
/* Line 1787 of yacc.c  */
#line 241 "Parser.ypp"
    {
	// This keyword has already been defined.  But since we are now
	// explicitly defining it, clear its bitmask, so that we will have a
	// new hash code--doing this will allow us to phase out the
	// historical hash code support later.
	((Keyword *)(yyvsp[(2) - (2)].u.keyword))->clear_historical_flag();
}
    break;

  case 26:
/* Line 1787 of yacc.c  */
#line 252 "Parser.ypp"
    {
		current_class = new Class(dc_file, (yyvsp[(2) - (2)].str));
	}
    break;

  case 27:
/* Line 1787 of yacc.c  */
#line 256 "Parser.ypp"
    {
		(yyval.u.dclass) = current_class->as_class();
		current_class = (yyvsp[(3) - (7)].u.dclass);
	}
    break;

  case 28:
/* Line 1787 of yacc.c  */
#line 264 "Parser.ypp"
    {
		if(dc_file == (File *)NULL)
		{
			yyerror("No File available, so no class names are predefined.");
			(yyval.u.dclass) = NULL;
		}
		else
		{
			Class *dclass = dc_file->get_class_by_name((yyvsp[(1) - (1)].str));
			if(dclass == (Class *)NULL)
			{
				yyerror("dclass '" + std::string((yyvsp[(1) - (1)].str)) + "' has not been declared.");
			}
			// TODO: Output this helpful info
			/*
			if(dc_file->get_struct_by_name($1))
			{
				yyerror("dclass cannot inherit from a struct");
			}
			*/

			(yyval.u.dclass) = dclass;
		}
	}
    break;

  case 31:
/* Line 1787 of yacc.c  */
#line 297 "Parser.ypp"
    {
		if((yyvsp[(1) - (1)].u.dclass) != (Class*)NULL)
		{
			current_class->as_class()->add_parent((yyvsp[(1) - (1)].u.dclass));
		}
	}
    break;

  case 32:
/* Line 1787 of yacc.c  */
#line 304 "Parser.ypp"
    {
		if((yyvsp[(3) - (3)].u.dclass) != (Class*)NULL)
		{
			current_class->as_class()->add_parent((yyvsp[(3) - (3)].u.dclass));
		}
	}
    break;

  case 35:
/* Line 1787 of yacc.c  */
#line 316 "Parser.ypp"
    {
		if((yyvsp[(2) - (3)].u.field) == (Field*)NULL)
		{
			// Pass this error up.
		}
		else if(!current_class->add_field((yyvsp[(2) - (3)].u.field)))
		{
			yyerror("Duplicate field name: " + (yyvsp[(2) - (3)].u.field)->get_name());
		}
	}
    break;

  case 36:
/* Line 1787 of yacc.c  */
#line 330 "Parser.ypp"
    {
		if((yyvsp[(1) - (2)].u.field) != (Field*)NULL)
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

  case 38:
/* Line 1787 of yacc.c  */
#line 343 "Parser.ypp"
    {
		yyerror("Unnamed parameters are not allowed on a dclass");
		if((yyvsp[(1) - (2)].u.parameter) != (Field *)NULL)
		{
			(yyvsp[(1) - (2)].u.parameter)->copy_keywords(current_keyword_list);
		}
		(yyval.u.field) = (yyvsp[(1) - (2)].u.parameter);
	}
    break;

  case 39:
/* Line 1787 of yacc.c  */
#line 352 "Parser.ypp"
    {
		if((yyvsp[(1) - (2)].u.parameter) != (Field *)NULL)
		{
			(yyvsp[(1) - (2)].u.parameter)->copy_keywords(current_keyword_list);
		}
		(yyval.u.field) = (yyvsp[(1) - (2)].u.parameter);
	}
    break;

  case 40:
/* Line 1787 of yacc.c  */
#line 363 "Parser.ypp"
    {
		current_class = new Struct(dc_file, (yyvsp[(2) - (2)].str));
	}
    break;

  case 41:
/* Line 1787 of yacc.c  */
#line 367 "Parser.ypp"
    {
		(yyval.u.dstruct) = current_class;
	}
    break;

  case 44:
/* Line 1787 of yacc.c  */
#line 376 "Parser.ypp"
    {
		if((yyvsp[(2) - (2)].u.field) == (Field *)NULL)
		{
			// Pass this error up.
		}
		else if(!current_class->add_field((yyvsp[(2) - (2)].u.field)))
		{
			yyerror("Duplicate field name: " + (yyvsp[(2) - (2)].u.field)->get_name());
		}
	}
    break;

  case 45:
/* Line 1787 of yacc.c  */
#line 389 "Parser.ypp"
    { (yyval.u.field) = (yyvsp[(1) - (2)].u.parameter); }
    break;

  case 46:
/* Line 1787 of yacc.c  */
#line 390 "Parser.ypp"
    { (yyval.u.field) = (yyvsp[(1) - (2)].u.parameter); }
    break;

  case 47:
/* Line 1787 of yacc.c  */
#line 395 "Parser.ypp"
    {
		if(current_class == (Class *)NULL)
		{
			yyerror("Cannot define a method outside of a struct or class.");
		}
		else
		{
			current_atomic = new AtomicField((yyvsp[(1) - (2)].str), current_class);
		}
	}
    break;

  case 48:
/* Line 1787 of yacc.c  */
#line 406 "Parser.ypp"
    {
		(yyval.u.field) = current_atomic;
		current_atomic = (yyvsp[(3) - (5)].u.atomic);
	}
    break;

  case 53:
/* Line 1787 of yacc.c  */
#line 424 "Parser.ypp"
    {
		if((yyvsp[(1) - (1)].u.parameter) != (Parameter *)NULL)
		{
			current_atomic->add_element((yyvsp[(1) - (1)].u.parameter));
		}
	}
    break;

  case 54:
/* Line 1787 of yacc.c  */
#line 433 "Parser.ypp"
    { (yyval.u.parameter) = (yyvsp[(1) - (1)].u.parameter); }
    break;

  case 55:
/* Line 1787 of yacc.c  */
#line 434 "Parser.ypp"
    { (yyval.u.parameter) = (yyvsp[(1) - (1)].u.parameter); }
    break;

  case 56:
/* Line 1787 of yacc.c  */
#line 435 "Parser.ypp"
    { (yyval.u.parameter) = (yyvsp[(1) - (1)].u.parameter); }
    break;

  case 57:
/* Line 1787 of yacc.c  */
#line 439 "Parser.ypp"
    { (yyval.u.parameter) = (yyvsp[(1) - (1)].u.parameter); }
    break;

  case 58:
/* Line 1787 of yacc.c  */
#line 440 "Parser.ypp"
    { (yyval.u.parameter) = (yyvsp[(1) - (1)].u.parameter); }
    break;

  case 59:
/* Line 1787 of yacc.c  */
#line 441 "Parser.ypp"
    { (yyval.u.parameter) = (yyvsp[(1) - (1)].u.parameter); }
    break;

  case 60:
/* Line 1787 of yacc.c  */
#line 442 "Parser.ypp"
    { (yyval.u.parameter) = (yyvsp[(1) - (1)].u.parameter); }
    break;

  case 61:
/* Line 1787 of yacc.c  */
#line 443 "Parser.ypp"
    { (yyval.u.parameter) = (yyvsp[(1) - (1)].u.parameter); }
    break;

  case 62:
/* Line 1787 of yacc.c  */
#line 444 "Parser.ypp"
    { (yyval.u.parameter) = (yyvsp[(1) - (1)].u.parameter); }
    break;

  case 63:
/* Line 1787 of yacc.c  */
#line 448 "Parser.ypp"
    { (yyval.u.parameter) = (yyvsp[(1) - (1)].u.parameter); }
    break;

  case 64:
/* Line 1787 of yacc.c  */
#line 449 "Parser.ypp"
    { (yyval.u.parameter) = (yyvsp[(1) - (1)].u.parameter); }
    break;

  case 65:
/* Line 1787 of yacc.c  */
#line 453 "Parser.ypp"
    { (yyval.u.parameter) = current_parameter = new SimpleParameter((yyvsp[(1) - (1)].u.datatype));   }
    break;

  case 66:
/* Line 1787 of yacc.c  */
#line 455 "Parser.ypp"
    {
		Typedef *dtypedef = dc_file->get_typedef_by_name((yyvsp[(1) - (1)].str));
		if(dtypedef == (Typedef *)NULL)
		{
			// Maybe it's a class name.
			Class *dclass = dc_file->get_class_by_name((yyvsp[(1) - (1)].str));
			if(dclass != (Class *)NULL)
			{
				// Create an implicit typedef for this.
				dtypedef = new Typedef(new StructParameter(dclass), true);
			}
			else
			{
				yyerror("Cannot use undefined type '" + (yyvsp[(1) - (1)].str) + "' as parameter type.");
			}

			dc_file->add_typedef(dtypedef);
		}

		(yyval.u.parameter) = current_parameter = dtypedef->make_new_parameter();
	}
    break;

  case 67:
/* Line 1787 of yacc.c  */
#line 480 "Parser.ypp"
    {
		SimpleParameter *simple_param = new SimpleParameter((yyvsp[(1) - (4)].u.datatype));
		if(simple_param == NULL
		|| simple_param->get_typedef() != (Typedef*)NULL)
		{
			yyerror("Ranges are only valid for numeric, string, or blob types.");
		}
		if(!simple_param->set_range((yyvsp[(3) - (4)].range)))
		{
			yyerror("Inappropriate range for type.");
		}

		(yyval.u.parameter) = current_parameter = simple_param;
	}
    break;

  case 68:
/* Line 1787 of yacc.c  */
#line 497 "Parser.ypp"
    { (yyval.u.parameter) = param_with_modulus((yyvsp[(1) - (3)].u.parameter), (yyvsp[(3) - (3)].u.real)); }
    break;

  case 69:
/* Line 1787 of yacc.c  */
#line 498 "Parser.ypp"
    { (yyval.u.parameter) = param_with_modulus((yyvsp[(1) - (3)].u.parameter), (yyvsp[(3) - (3)].u.real)); }
    break;

  case 70:
/* Line 1787 of yacc.c  */
#line 502 "Parser.ypp"
    { (yyval.u.parameter) = param_with_divisor((yyvsp[(1) - (3)].u.parameter), (yyvsp[(3) - (3)].u.uint32)); }
    break;

  case 71:
/* Line 1787 of yacc.c  */
#line 503 "Parser.ypp"
    { (yyval.u.parameter) = param_with_divisor((yyvsp[(1) - (3)].u.parameter), (yyvsp[(3) - (3)].u.uint32)); }
    break;

  case 72:
/* Line 1787 of yacc.c  */
#line 504 "Parser.ypp"
    { (yyval.u.parameter) = param_with_divisor((yyvsp[(1) - (3)].u.parameter), (yyvsp[(3) - (3)].u.uint32)); }
    break;

  case 73:
/* Line 1787 of yacc.c  */
#line 508 "Parser.ypp"
    { (yyval.u.parameter) = (yyvsp[(1) - (2)].u.parameter); (yyvsp[(1) - (2)].u.parameter)->set_name((yyvsp[(2) - (2)].str)); }
    break;

  case 74:
/* Line 1787 of yacc.c  */
#line 509 "Parser.ypp"
    { (yyval.u.parameter) = (yyvsp[(1) - (2)].u.parameter); (yyvsp[(1) - (2)].u.parameter)->set_name((yyvsp[(2) - (2)].str)); }
    break;

  case 75:
/* Line 1787 of yacc.c  */
#line 510 "Parser.ypp"
    { (yyval.u.parameter) = (yyvsp[(1) - (2)].u.parameter); (yyvsp[(1) - (2)].u.parameter)->set_name((yyvsp[(2) - (2)].str)); }
    break;

  case 76:
/* Line 1787 of yacc.c  */
#line 511 "Parser.ypp"
    { (yyval.u.parameter) = (yyvsp[(1) - (2)].u.parameter); (yyvsp[(1) - (2)].u.parameter)->set_name((yyvsp[(2) - (2)].str)); }
    break;

  case 77:
/* Line 1787 of yacc.c  */
#line 515 "Parser.ypp"
    { (yyval.u.parameter) = (yyvsp[(1) - (4)].u.parameter)->append_array_specification((yyvsp[(3) - (4)].range)); }
    break;

  case 78:
/* Line 1787 of yacc.c  */
#line 516 "Parser.ypp"
    { (yyval.u.parameter) = (yyvsp[(1) - (4)].u.parameter)->append_array_specification((yyvsp[(3) - (4)].range)); }
    break;

  case 79:
/* Line 1787 of yacc.c  */
#line 517 "Parser.ypp"
    { (yyval.u.parameter) = (yyvsp[(1) - (4)].u.parameter)->append_array_specification((yyvsp[(3) - (4)].range)); }
    break;

  case 80:
/* Line 1787 of yacc.c  */
#line 518 "Parser.ypp"
    { (yyval.u.parameter) = (yyvsp[(1) - (4)].u.parameter)->append_array_specification((yyvsp[(3) - (4)].range)); }
    break;

  case 81:
/* Line 1787 of yacc.c  */
#line 522 "Parser.ypp"
    { (yyval.u.parameter) = (yyvsp[(1) - (4)].u.parameter)->append_array_specification((yyvsp[(3) - (4)].range)); }
    break;

  case 82:
/* Line 1787 of yacc.c  */
#line 526 "Parser.ypp"
    { (yyval.u.parameter) = (yyvsp[(1) - (3)].u.parameter); (yyvsp[(1) - (3)].u.parameter)->set_default_value((yyvsp[(3) - (3)].str)); }
    break;

  case 83:
/* Line 1787 of yacc.c  */
#line 527 "Parser.ypp"
    { (yyval.u.parameter) = (yyvsp[(1) - (3)].u.parameter); (yyvsp[(1) - (3)].u.parameter)->set_default_value((yyvsp[(3) - (3)].str)); }
    break;

  case 84:
/* Line 1787 of yacc.c  */
#line 528 "Parser.ypp"
    { (yyval.u.parameter) = (yyvsp[(1) - (3)].u.parameter); (yyvsp[(1) - (3)].u.parameter)->set_default_value((yyvsp[(3) - (3)].str)); }
    break;

  case 85:
/* Line 1787 of yacc.c  */
#line 529 "Parser.ypp"
    { (yyval.u.parameter) = (yyvsp[(1) - (3)].u.parameter); (yyvsp[(1) - (3)].u.parameter)->set_default_value((yyvsp[(3) - (3)].str)); }
    break;

  case 86:
/* Line 1787 of yacc.c  */
#line 530 "Parser.ypp"
    { (yyval.u.parameter) = (yyvsp[(1) - (3)].u.parameter); (yyvsp[(1) - (3)].u.parameter)->set_default_value((yyvsp[(3) - (3)].str)); }
    break;

  case 87:
/* Line 1787 of yacc.c  */
#line 534 "Parser.ypp"
    { (yyval.u.parameter) = (yyvsp[(1) - (3)].u.parameter); (yyvsp[(1) - (3)].u.parameter)->set_default_value((yyvsp[(3) - (3)].str)); }
    break;

  case 88:
/* Line 1787 of yacc.c  */
#line 535 "Parser.ypp"
    { (yyval.u.parameter) = (yyvsp[(1) - (3)].u.parameter); (yyvsp[(1) - (3)].u.parameter)->set_default_value((yyvsp[(3) - (3)].str)); }
    break;

  case 89:
/* Line 1787 of yacc.c  */
#line 539 "Parser.ypp"
    { (yyval.range) = NumericRange(); }
    break;

  case 90:
/* Line 1787 of yacc.c  */
#line 540 "Parser.ypp"
    { (yyval.range) = NumericRange((yyvsp[(1) - (1)].u.real), (yyvsp[(1) - (1)].u.real)); }
    break;

  case 91:
/* Line 1787 of yacc.c  */
#line 541 "Parser.ypp"
    { (yyval.range) = NumericRange((yyvsp[(1) - (3)].u.real), (yyvsp[(3) - (3)].u.real)); }
    break;

  case 92:
/* Line 1787 of yacc.c  */
#line 542 "Parser.ypp"
    { (yyval.range) = NumericRange((yyvsp[(1) - (2)].u.real), (yyvsp[(2) - (2)].u.real)); }
    break;

  case 93:
/* Line 1787 of yacc.c  */
#line 546 "Parser.ypp"
    { (yyval.range) = NumericRange(); }
    break;

  case 94:
/* Line 1787 of yacc.c  */
#line 547 "Parser.ypp"
    { (yyval.range) = NumericRange((yyvsp[(1) - (1)].u.uint32), (yyvsp[(1) - (1)].u.uint32)); }
    break;

  case 95:
/* Line 1787 of yacc.c  */
#line 548 "Parser.ypp"
    { (yyval.range) = NumericRange((yyvsp[(1) - (3)].u.uint32), (yyvsp[(3) - (3)].u.uint32)); }
    break;

  case 96:
/* Line 1787 of yacc.c  */
#line 549 "Parser.ypp"
    { (yyval.range) = NumericRange((yyvsp[(1) - (2)].u.uint32), (yyvsp[(2) - (2)].u.uint32)); }
    break;

  case 97:
/* Line 1787 of yacc.c  */
#line 554 "Parser.ypp"
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

  case 99:
/* Line 1787 of yacc.c  */
#line 569 "Parser.ypp"
    {
		(yyval.u.uint32) = (unsigned int)(yyvsp[(1) - (1)].u.uint64);
		if((yyval.u.uint32) != (yyvsp[(1) - (1)].u.uint64))
		{
			yyerror("Number out of range.");
			(yyval.u.uint32) = 1;
		}
	}
    break;

  case 100:
/* Line 1787 of yacc.c  */
#line 581 "Parser.ypp"
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

  case 101:
/* Line 1787 of yacc.c  */
#line 596 "Parser.ypp"
    { (yyval.u.real) = (double)(yyvsp[(1) - (1)].u.uint64); }
    break;

  case 102:
/* Line 1787 of yacc.c  */
#line 597 "Parser.ypp"
    { (yyval.u.real) = (double)(yyvsp[(1) - (1)].u.int64); }
    break;

  case 104:
/* Line 1787 of yacc.c  */
#line 603 "Parser.ypp"
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

  case 106:
/* Line 1787 of yacc.c  */
#line 619 "Parser.ypp"
    {
		// TODO: check for range limits
		(yyval.str) = std::string((char*)&(yyvsp[(1) - (1)].u.int64), sizeof(int64_t));
	}
    break;

  case 107:
/* Line 1787 of yacc.c  */
#line 624 "Parser.ypp"
    {
		// TODO: check for range limits
		(yyval.str) = std::string((char*)&(yyvsp[(1) - (1)].u.uint64), sizeof(uint64_t));
	}
    break;

  case 108:
/* Line 1787 of yacc.c  */
#line 629 "Parser.ypp"
    {
		// TODO: check for range limits
		(yyval.str) = std::string((char*)&(yyvsp[(1) - (1)].u.real), sizeof(double));
	}
    break;

  case 109:
/* Line 1787 of yacc.c  */
#line 634 "Parser.ypp"
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

  case 110:
/* Line 1787 of yacc.c  */
#line 653 "Parser.ypp"
    {
		// TODO: check for range limits... maybe?
		(yyval.str) = (yyvsp[(1) - (1)].str);
	}
    break;

  case 111:
/* Line 1787 of yacc.c  */
#line 658 "Parser.ypp"
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

  case 112:
/* Line 1787 of yacc.c  */
#line 672 "Parser.ypp"
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

  case 113:
/* Line 1787 of yacc.c  */
#line 699 "Parser.ypp"
    {
		// TODO: Check array type matches parameter type
		(yyval.str) = (yyvsp[(1) - (1)].str);
	}
    break;

  case 114:
/* Line 1787 of yacc.c  */
#line 704 "Parser.ypp"
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

  case 115:
/* Line 1787 of yacc.c  */
#line 714 "Parser.ypp"
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

  case 116:
/* Line 1787 of yacc.c  */
#line 724 "Parser.ypp"
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

  case 117:
/* Line 1787 of yacc.c  */
#line 734 "Parser.ypp"
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

  case 118:
/* Line 1787 of yacc.c  */
#line 744 "Parser.ypp"
    {
		// TODO: Check array type matches parameter type
		(yyval.str) = (yyvsp[(1) - (3)].str) + (yyvsp[(3) - (3)].str);
	}
    break;

  case 119:
/* Line 1787 of yacc.c  */
#line 751 "Parser.ypp"
    { (yyval.u.datatype) = DT_int8; }
    break;

  case 120:
/* Line 1787 of yacc.c  */
#line 752 "Parser.ypp"
    { (yyval.u.datatype) = DT_int16; }
    break;

  case 121:
/* Line 1787 of yacc.c  */
#line 753 "Parser.ypp"
    { (yyval.u.datatype) = DT_int32; }
    break;

  case 122:
/* Line 1787 of yacc.c  */
#line 754 "Parser.ypp"
    { (yyval.u.datatype) = DT_int64; }
    break;

  case 123:
/* Line 1787 of yacc.c  */
#line 755 "Parser.ypp"
    { (yyval.u.datatype) = DT_uint8; }
    break;

  case 124:
/* Line 1787 of yacc.c  */
#line 756 "Parser.ypp"
    { (yyval.u.datatype) = DT_uint16; }
    break;

  case 125:
/* Line 1787 of yacc.c  */
#line 757 "Parser.ypp"
    { (yyval.u.datatype) = DT_uint32; }
    break;

  case 126:
/* Line 1787 of yacc.c  */
#line 758 "Parser.ypp"
    { (yyval.u.datatype) = DT_uint64; }
    break;

  case 127:
/* Line 1787 of yacc.c  */
#line 759 "Parser.ypp"
    { (yyval.u.datatype) = DT_float32; }
    break;

  case 128:
/* Line 1787 of yacc.c  */
#line 760 "Parser.ypp"
    { (yyval.u.datatype) = DT_float64; }
    break;

  case 129:
/* Line 1787 of yacc.c  */
#line 761 "Parser.ypp"
    { (yyval.u.datatype) = DT_string; }
    break;

  case 130:
/* Line 1787 of yacc.c  */
#line 762 "Parser.ypp"
    { (yyval.u.datatype) = DT_blob; }
    break;

  case 131:
/* Line 1787 of yacc.c  */
#line 763 "Parser.ypp"
    { (yyval.u.datatype) = DT_char; }
    break;

  case 132:
/* Line 1787 of yacc.c  */
#line 767 "Parser.ypp"
    { current_keyword_list.clear_keywords(); }
    break;

  case 133:
/* Line 1787 of yacc.c  */
#line 768 "Parser.ypp"
    { current_keyword_list.add_keyword((yyvsp[(2) - (2)].u.keyword)); }
    break;

  case 134:
/* Line 1787 of yacc.c  */
#line 773 "Parser.ypp"
    {
		if(current_keyword_list.get_num_keywords() != 0)
		{
			yyerror("Keywords are not allowed here.");
		}
	}
    break;

  case 135:
/* Line 1787 of yacc.c  */
#line 782 "Parser.ypp"
    { current_molecular = new MolecularField((yyvsp[(1) - (2)].str), current_class); }
    break;

  case 136:
/* Line 1787 of yacc.c  */
#line 783 "Parser.ypp"
    { (yyval.u.field) = current_molecular; }
    break;

  case 137:
/* Line 1787 of yacc.c  */
#line 788 "Parser.ypp"
    {
		Field *field = current_class->get_field_by_name((yyvsp[(1) - (1)].str));
		(yyval.u.atomic) = (AtomicField *)NULL;
		if(field == (Field *)NULL)
		{
			yyerror("Unknown field: " + (yyvsp[(1) - (1)].str));
		}
		else
		{
			(yyval.u.atomic) = field->as_atomic_field();
			if((yyval.u.atomic) == (AtomicField*)NULL)
			{
				yyerror("Not an atomic field: " + (yyvsp[(1) - (1)].str));
			}
		}
	}
    break;

  case 138:
/* Line 1787 of yacc.c  */
#line 808 "Parser.ypp"
    {
		if((yyvsp[(1) - (1)].u.atomic) != (AtomicField *)NULL)
		{
			current_molecular->add_atomic((yyvsp[(1) - (1)].u.atomic));
		}
	}
    break;

  case 139:
/* Line 1787 of yacc.c  */
#line 815 "Parser.ypp"
    {
		if((yyvsp[(3) - (3)].u.atomic) != (AtomicField *)NULL)
		{
			current_molecular->add_atomic((yyvsp[(3) - (3)].u.atomic));
			if(!current_molecular->compare_keywords(*(yyvsp[(3) - (3)].u.atomic)))
			{
				yyerror("Mismatched keywords in molecule between " +
				current_molecular->get_atomic(0)->get_name() + " and " +
				(yyvsp[(3) - (3)].u.atomic)->get_name());
			}
		}
	}
    break;


/* Line 1787 of yacc.c  */
#line 2648 "Parser.cpp"
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
#line 834 "Parser.ypp"
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
