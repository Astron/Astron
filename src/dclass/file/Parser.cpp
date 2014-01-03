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
	std::string *output_value = (std::string*)NULL;

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

	void dc_init_parser(std::istream& in, const std::string& filename, File& file)
	{
		dc_file = &file;
		dc_init_lexer(in, filename);
	}

	void dc_init_value_parser(std::istream& in, const std::string& source, const Parameter* parameter, std::string& output)
	{
		output_value = &output;
		dc_init_lexer(in, source);
		dc_start_parameter_value();
	}

	void dc_cleanup_parser()
	{
		dc_file = (File*)NULL;
		output_value = (std::string*)NULL;
	}


/* Line 371 of yacc.c  */
#line 143 "Parser.cpp"

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
     START_PARAMETER_VALUE = 266,
     KW_DCLASS = 267,
     KW_STRUCT = 268,
     KW_FROM = 269,
     KW_IMPORT = 270,
     KW_TYPEDEF = 271,
     KW_KEYWORD = 272,
     KW_INT8 = 273,
     KW_INT16 = 274,
     KW_INT32 = 275,
     KW_INT64 = 276,
     KW_UINT8 = 277,
     KW_UINT16 = 278,
     KW_UINT32 = 279,
     KW_UINT64 = 280,
     KW_FLOAT32 = 281,
     KW_FLOAT64 = 282,
     KW_STRING = 283,
     KW_BLOB = 284,
     KW_CHAR = 285
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
#line 238 "Parser.cpp"

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
#define YYFINAL  15
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   219

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  46
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  58
/* YYNRULES -- Number of rules.  */
#define YYNRULES  142
/* YYNRULES -- Number of states.  */
#define YYNSTATES  221

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   285

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,    41,     2,     2,
      39,    40,    34,     2,    35,    45,    32,    33,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    38,    31,
       2,    44,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    42,     2,    43,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    36,     2,    37,     2,     2,     2,     2,
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
      25,    26,    27,    28,    29,    30
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const yytype_uint16 yyprhs[] =
{
       0,     0,     3,     6,     9,    11,    14,    17,    20,    23,
      26,    29,    31,    34,    35,    41,    43,    47,    49,    53,
      55,    57,    59,    63,    66,    69,    71,    74,    77,    78,
      86,    88,    90,    93,    95,    99,   101,   104,   108,   111,
     114,   117,   120,   121,   128,   130,   133,   136,   139,   142,
     143,   149,   151,   153,   155,   159,   161,   163,   165,   167,
     169,   171,   173,   175,   177,   179,   181,   183,   185,   187,
     192,   196,   200,   204,   208,   212,   215,   218,   221,   224,
     229,   234,   239,   244,   249,   253,   257,   261,   265,   269,
     273,   277,   279,   281,   285,   288,   290,   292,   296,   299,
     301,   303,   305,   307,   309,   311,   313,   315,   317,   319,
     321,   323,   325,   327,   331,   335,   337,   341,   345,   349,
     353,   357,   359,   361,   363,   365,   367,   369,   371,   373,
     375,   377,   379,   381,   383,   385,   388,   390,   391,   396,
     398,   400,   404
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int8 yyrhs[] =
{
      47,     0,    -1,    10,    48,    -1,    11,    49,    -1,   103,
      -1,    48,    31,    -1,    48,    59,    -1,    48,    66,    -1,
      48,    50,    -1,    48,    56,    -1,    48,    57,    -1,    94,
      -1,    15,    52,    -1,    -1,    14,    52,    15,    51,    54,
      -1,    53,    -1,    52,    32,    53,    -1,     8,    -1,    53,
      33,     8,    -1,    55,    -1,    34,    -1,    53,    -1,    55,
      35,    53,    -1,    16,    75,    -1,    17,    58,    -1,   103,
      -1,    58,     8,    -1,    58,     9,    -1,    -1,    12,     8,
      60,    62,    36,    64,    37,    -1,     8,    -1,   103,    -1,
      38,    63,    -1,    61,    -1,    63,    35,    61,    -1,   103,
      -1,    64,    31,    -1,    64,    65,    31,    -1,    70,    97,
      -1,    99,    98,    -1,    76,    97,    -1,    75,    97,    -1,
      -1,    13,     8,    67,    36,    68,    37,    -1,   103,    -1,
      68,    31,    -1,    68,    69,    -1,    76,    31,    -1,    75,
      31,    -1,    -1,     8,    39,    71,    72,    40,    -1,   103,
      -1,    73,    -1,    74,    -1,    73,    35,    74,    -1,    77,
      -1,    82,    -1,    84,    -1,    86,    -1,    78,    -1,    79,
      -1,    80,    -1,    81,    -1,    83,    -1,    85,    -1,    75,
      -1,    76,    -1,    96,    -1,     8,    -1,    96,    39,    87,
      40,    -1,    78,    41,    92,    -1,    79,    41,    92,    -1,
      78,    33,    90,    -1,    79,    33,    90,    -1,    80,    33,
      90,    -1,    78,     8,    -1,    79,     8,    -1,    80,     8,
      -1,    81,     8,    -1,    78,    42,    88,    43,    -1,    79,
      42,    88,    43,    -1,    80,    42,    88,    43,    -1,    81,
      42,    88,    43,    -1,    82,    42,    88,    43,    -1,    78,
      44,    94,    -1,    79,    44,    94,    -1,    80,    44,    94,
      -1,    81,    44,    94,    -1,    83,    44,    94,    -1,    82,
      44,    94,    -1,    84,    44,    94,    -1,   103,    -1,    93,
      -1,    93,    45,    93,    -1,    93,    92,    -1,   103,    -1,
      89,    -1,    89,    45,    89,    -1,    89,    91,    -1,     6,
      -1,    90,    -1,     3,    -1,     4,    -1,     3,    -1,     4,
      -1,     5,    -1,     6,    -1,    92,    -1,     4,    -1,     3,
      -1,     5,    -1,     6,    -1,     7,    -1,    42,    95,    43,
      -1,    36,    95,    37,    -1,    94,    -1,     4,    34,    90,
      -1,     3,    34,    90,    -1,     5,    34,    90,    -1,     7,
      34,    90,    -1,    95,    35,    94,    -1,    18,    -1,    19,
      -1,    20,    -1,    21,    -1,    22,    -1,    23,    -1,    24,
      -1,    25,    -1,    26,    -1,    27,    -1,    28,    -1,    29,
      -1,    30,    -1,   103,    -1,    97,     9,    -1,    97,    -1,
      -1,     8,    38,   100,   102,    -1,     8,    -1,   101,    -1,
     102,    35,   101,    -1,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   161,   161,   162,   166,   167,   168,   175,   182,   183,
     184,   188,   195,   200,   199,   208,   209,   216,   217,   225,
     226,   230,   231,   235,   249,   253,   254,   258,   270,   269,
     282,   309,   310,   314,   321,   331,   332,   333,   347,   359,
     360,   369,   381,   380,   391,   392,   393,   407,   408,   413,
     412,   431,   432,   436,   437,   441,   451,   452,   453,   457,
     458,   459,   460,   461,   462,   466,   467,   471,   472,   504,
     522,   523,   527,   528,   529,   533,   534,   535,   536,   540,
     541,   542,   543,   547,   551,   552,   553,   554,   555,   559,
     560,   564,   565,   566,   567,   571,   572,   573,   574,   578,
     589,   593,   605,   621,   622,   623,   627,   639,   643,   648,
     653,   658,   677,   682,   696,   723,   728,   738,   748,   758,
     768,   776,   777,   778,   779,   780,   781,   782,   783,   784,
     785,   786,   787,   788,   792,   793,   797,   807,   807,   812,
     832,   839,   854
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || 0
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "UNSIGNED_INTEGER", "SIGNED_INTEGER",
  "REAL", "STRING", "HEX_STRING", "IDENTIFIER", "KEYWORD", "START_DC",
  "START_PARAMETER_VALUE", "KW_DCLASS", "KW_STRUCT", "KW_FROM",
  "KW_IMPORT", "KW_TYPEDEF", "KW_KEYWORD", "KW_INT8", "KW_INT16",
  "KW_INT32", "KW_INT64", "KW_UINT8", "KW_UINT16", "KW_UINT32",
  "KW_UINT64", "KW_FLOAT32", "KW_FLOAT64", "KW_STRING", "KW_BLOB",
  "KW_CHAR", "';'", "'.'", "'/'", "'*'", "','", "'{'", "'}'", "':'", "'('",
  "')'", "'%'", "'['", "']'", "'='", "'-'", "$accept", "grammar", "dc",
  "output_parameter_value", "import", "$@1", "import_identifier",
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
     285,    59,    46,    47,    42,    44,   123,   125,    58,    40,
      41,    37,    91,    93,    61,    45
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    46,    47,    47,    48,    48,    48,    48,    48,    48,
      48,    49,    50,    51,    50,    52,    52,    53,    53,    54,
      54,    55,    55,    56,    57,    58,    58,    58,    60,    59,
      61,    62,    62,    63,    63,    64,    64,    64,    65,    65,
      65,    65,    67,    66,    68,    68,    68,    69,    69,    71,
      70,    72,    72,    73,    73,    74,    75,    75,    75,    76,
      76,    76,    76,    76,    76,    77,    77,    78,    78,    79,
      80,    80,    81,    81,    81,    82,    82,    82,    82,    83,
      83,    83,    83,    84,    85,    85,    85,    85,    85,    86,
      86,    87,    87,    87,    87,    88,    88,    88,    88,    89,
      89,    90,    91,    92,    92,    92,    93,    93,    94,    94,
      94,    94,    94,    94,    94,    95,    95,    95,    95,    95,
      95,    96,    96,    96,    96,    96,    96,    96,    96,    96,
      96,    96,    96,    96,    97,    97,    98,   100,    99,   101,
     102,   102,   103
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     2,     2,     1,     2,     2,     2,     2,     2,
       2,     1,     2,     0,     5,     1,     3,     1,     3,     1,
       1,     1,     3,     2,     2,     1,     2,     2,     0,     7,
       1,     1,     2,     1,     3,     1,     2,     3,     2,     2,
       2,     2,     0,     6,     1,     2,     2,     2,     2,     0,
       5,     1,     1,     1,     3,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     4,
       3,     3,     3,     3,     3,     2,     2,     2,     2,     4,
       4,     4,     4,     4,     3,     3,     3,     3,     3,     3,
       3,     1,     1,     3,     2,     1,     1,     3,     2,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     3,     3,     1,     3,     3,     3,     3,
       3,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     2,     1,     0,     4,     1,
       1,     3,     0
};

/* YYDEFACT[STATE-NAME] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       0,   142,     0,     0,     2,     4,   109,   108,   110,   111,
     112,     0,     0,     3,    11,     1,     0,     0,     0,     0,
       0,   142,     5,     8,     9,    10,     6,     7,   109,   108,
     110,   112,   115,     0,     0,    28,    42,    17,     0,    15,
      12,    68,   121,   122,   123,   124,   125,   126,   127,   128,
     129,   130,   131,   132,   133,    23,     0,     0,     0,     0,
      56,    57,    58,    67,    24,    25,     0,     0,     0,     0,
       0,   114,   113,   142,     0,    13,     0,     0,    75,     0,
       0,    76,     0,     0,    77,     0,    78,   142,     0,     0,
     142,    26,    27,   101,   117,   116,   118,   119,   120,     0,
       0,    31,   142,     0,    16,    18,    72,   103,   104,   105,
      70,    73,    71,    74,    99,     0,    96,   100,    95,    89,
      90,   106,     0,   107,    92,    91,    30,    33,    32,   142,
       0,    44,    20,    21,    14,    19,    83,   102,     0,    98,
      69,     0,    94,     0,     0,    35,    45,    43,    46,     0,
       0,    59,    60,    61,    62,    63,    64,     0,    97,    93,
      34,    68,    36,    29,     0,   142,   142,   142,   142,    48,
      47,   142,     0,   142,     0,   142,     0,   142,     0,     0,
      22,   137,    49,    37,    38,   134,    41,    40,   136,    39,
       0,    84,     0,    85,     0,    86,     0,    87,    88,     0,
     142,   135,    79,    80,    81,    82,   139,   140,   138,     0,
      52,    53,    65,    66,    55,    51,     0,    50,     0,   141,
      54
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     3,     4,    13,    23,   103,    38,    39,   134,   135,
      24,    25,    64,    26,    73,   127,   100,   128,   144,   164,
      27,    74,   130,   148,   165,   200,   209,   210,   211,   212,
     213,   214,   151,   152,   153,   154,    60,   155,    61,   156,
      62,   122,   115,   116,   117,   139,   123,   124,    32,    33,
      63,   184,   189,   168,   199,   207,   208,   118
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -104
static const yytype_int16 yypact[] =
{
      60,  -104,     8,    34,    81,  -104,  -104,  -104,  -104,  -104,
    -104,    18,    18,  -104,  -104,  -104,    38,    48,    51,    51,
     189,  -104,  -104,  -104,  -104,  -104,  -104,  -104,    41,    79,
      80,    83,  -104,    44,   -17,  -104,  -104,  -104,    16,    19,
      84,  -104,  -104,  -104,  -104,  -104,  -104,  -104,  -104,  -104,
    -104,  -104,  -104,  -104,  -104,  -104,     2,    22,     9,   112,
      46,    78,  -104,    85,    91,  -104,   120,   120,   120,   120,
       8,  -104,  -104,    88,   107,  -104,    51,   119,  -104,   120,
      82,  -104,   120,    82,  -104,   120,  -104,    13,     8,     8,
      33,  -104,  -104,  -104,  -104,  -104,  -104,  -104,  -104,   136,
     109,  -104,  -104,    43,    19,  -104,  -104,  -104,  -104,  -104,
    -104,  -104,  -104,  -104,  -104,   103,     0,  -104,  -104,  -104,
    -104,  -104,   110,  -104,     4,  -104,  -104,  -104,   114,  -104,
     111,  -104,  -104,    19,  -104,   116,  -104,  -104,    13,  -104,
    -104,    33,  -104,   136,   165,  -104,  -104,  -104,  -104,   121,
     122,    20,    24,    25,    32,   113,  -104,    51,  -104,  -104,
    -104,    71,  -104,  -104,   123,  -104,  -104,  -104,  -104,  -104,
    -104,    13,     8,    13,     8,    13,     8,    13,     8,     8,
      19,  -104,  -104,  -104,   138,  -104,   138,   138,   138,  -104,
     115,  -104,   117,  -104,   118,  -104,   125,  -104,  -104,   147,
     189,  -104,  -104,  -104,  -104,  -104,  -104,  -104,   124,   129,
     127,  -104,  -104,  -104,  -104,  -104,   147,  -104,   189,  -104,
    -104
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -104,  -104,  -104,  -104,  -104,  -104,   137,   -74,  -104,  -104,
    -104,  -104,  -104,  -104,  -104,    27,  -104,  -104,  -104,  -104,
    -104,  -104,  -104,  -104,  -104,  -104,  -104,  -104,   -55,   -19,
    -103,  -104,   151,   152,   154,   156,  -104,  -104,  -104,  -104,
    -104,  -104,   -93,    40,    36,  -104,   -77,    39,     3,   186,
    -104,   -60,  -104,  -104,  -104,   -16,  -104,    -1
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -1
static const yytype_uint8 yytable[] =
{
       5,    55,   104,   110,   137,    14,   112,   107,   108,   109,
      78,     6,     7,     8,     9,    10,    93,    84,    70,   114,
      65,    28,    29,    30,     9,    31,    72,   150,    78,   133,
      81,    75,    81,    84,    15,    79,   107,   108,   109,   121,
      86,   167,    85,    80,    11,   138,    35,   142,    76,   141,
      12,    37,    77,    79,    11,    82,    36,    82,    85,    37,
      12,    80,   171,    83,   172,    83,   173,   175,   174,   176,
       1,     2,   101,    98,   177,    66,   178,   132,   190,    70,
     192,    71,   194,   180,   196,   107,   108,   109,    87,   125,
      88,   119,   120,    16,    17,    18,    19,    20,    21,    91,
      92,   131,    94,    95,    96,    97,   186,   187,   188,   181,
     182,   149,    22,    67,    68,   106,    76,    69,   111,    41,
      86,   113,    89,    93,    90,   166,    99,   105,   145,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,   146,   102,   126,   129,   136,   201,   147,   143,
     140,   157,   169,   170,   183,   206,    40,   179,   202,   216,
     203,   204,   218,   220,   185,   185,   185,   185,   205,   217,
     160,    56,    57,   161,    58,   191,    59,   193,   158,   195,
     159,   197,   198,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,   162,    41,    34,   215,
     219,     0,   163,     0,     0,     0,     0,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54
};

#define yypact_value_is_default(Yystate) \
  (!!((Yystate) == (-104)))

#define yytable_value_is_error(Yytable_value) \
  YYID (0)

static const yytype_int16 yycheck[] =
{
       1,    20,    76,    80,     4,     2,    83,     3,     4,     5,
       8,     3,     4,     5,     6,     7,     3,     8,    35,     6,
      21,     3,     4,     5,     6,     7,    43,   130,     8,   103,
       8,    15,     8,     8,     0,    33,     3,     4,     5,     6,
       8,   144,    33,    41,    36,    45,     8,   124,    32,    45,
      42,     8,    33,    33,    36,    33,     8,    33,    33,     8,
      42,    41,    42,    41,    44,    41,    42,    42,    44,    44,
      10,    11,    73,    70,    42,    34,    44,    34,   171,    35,
     173,    37,   175,   157,   177,     3,     4,     5,    42,    90,
      44,    88,    89,    12,    13,    14,    15,    16,    17,     8,
       9,   102,    66,    67,    68,    69,   166,   167,   168,    38,
      39,   130,    31,    34,    34,    79,    32,    34,    82,     8,
       8,    85,    44,     3,    39,   144,    38,     8,   129,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    36,     8,    36,    43,     9,    37,    35,
      40,    35,    31,    31,    31,     8,    19,    44,    43,    35,
      43,    43,    35,   218,   165,   166,   167,   168,    43,    40,
     143,    20,    20,     8,    20,   172,    20,   174,   138,   176,
     141,   178,   179,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,     8,    12,   200,
     216,    -1,    37,    -1,    -1,    -1,    -1,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    30
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,    10,    11,    47,    48,   103,     3,     4,     5,     6,
       7,    36,    42,    49,    94,     0,    12,    13,    14,    15,
      16,    17,    31,    50,    56,    57,    59,    66,     3,     4,
       5,     7,    94,    95,    95,     8,     8,     8,    52,    53,
      52,     8,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,    30,    75,    78,    79,    80,    81,
      82,    84,    86,    96,    58,   103,    34,    34,    34,    34,
      35,    37,    43,    60,    67,    15,    32,    33,     8,    33,
      41,     8,    33,    41,     8,    33,     8,    42,    44,    44,
      39,     8,     9,     3,    90,    90,    90,    90,    94,    38,
      62,   103,    36,    51,    53,     8,    90,     3,     4,     5,
      92,    90,    92,    90,     6,    88,    89,    90,   103,    94,
      94,     6,    87,    92,    93,   103,     8,    61,    63,    36,
      68,   103,    34,    53,    54,    55,    43,     4,    45,    91,
      40,    45,    92,    35,    64,   103,    31,    37,    69,    75,
      76,    78,    79,    80,    81,    83,    85,    35,    89,    93,
      61,     8,    31,    37,    65,    70,    75,    76,    99,    31,
      31,    42,    44,    42,    44,    42,    44,    42,    44,    44,
      53,    38,    39,    31,    97,   103,    97,    97,    97,    98,
      88,    94,    88,    94,    88,    94,    88,    94,    94,   100,
      71,     9,    43,    43,    43,    43,     8,   101,   102,    72,
      73,    74,    75,    76,    77,   103,    35,    40,    35,   101,
      74
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
        case 6:
/* Line 1787 of yacc.c  */
#line 169 "Parser.ypp"
    {
		if(!dc_file->add_class((yyvsp[(2) - (2)].u.dclass)))
		{
			yyerror("Duplicate class name: " + (yyvsp[(2) - (2)].u.dclass)->get_name());
		}
	}
    break;

  case 7:
/* Line 1787 of yacc.c  */
#line 176 "Parser.ypp"
    {
		if(!dc_file->add_struct((yyvsp[(2) - (2)].u.dstruct)))
		{
			yyerror("Duplicate class name: " + (yyvsp[(2) - (2)].u.dstruct)->get_name());
		}
	}
    break;

  case 11:
/* Line 1787 of yacc.c  */
#line 189 "Parser.ypp"
    {
		output_value->assign((yyvsp[(1) - (1)].str));
	}
    break;

  case 12:
/* Line 1787 of yacc.c  */
#line 196 "Parser.ypp"
    {
		dc_file->add_import_module((yyvsp[(2) - (2)].str));
	}
    break;

  case 13:
/* Line 1787 of yacc.c  */
#line 200 "Parser.ypp"
    {
		dc_file->add_import_module((yyvsp[(2) - (3)].str));
	}
    break;

  case 16:
/* Line 1787 of yacc.c  */
#line 210 "Parser.ypp"
    {
		(yyval.str) = (yyvsp[(1) - (3)].str) + std::string(".") + (yyvsp[(3) - (3)].str);
	}
    break;

  case 18:
/* Line 1787 of yacc.c  */
#line 218 "Parser.ypp"
    {
		(yyval.str) = (yyvsp[(1) - (3)].str) + std::string("/") + (yyvsp[(3) - (3)].str);
	}
    break;

  case 20:
/* Line 1787 of yacc.c  */
#line 226 "Parser.ypp"
    { dc_file->add_import_symbol("*"); }
    break;

  case 21:
/* Line 1787 of yacc.c  */
#line 230 "Parser.ypp"
    { dc_file->add_import_symbol((yyvsp[(1) - (1)].str)); }
    break;

  case 22:
/* Line 1787 of yacc.c  */
#line 231 "Parser.ypp"
    { dc_file->add_import_symbol((yyvsp[(3) - (3)].str)); }
    break;

  case 23:
/* Line 1787 of yacc.c  */
#line 236 "Parser.ypp"
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

  case 26:
/* Line 1787 of yacc.c  */
#line 255 "Parser.ypp"
    {
	dc_file->add_keyword((yyvsp[(2) - (2)].str));
}
    break;

  case 27:
/* Line 1787 of yacc.c  */
#line 259 "Parser.ypp"
    {
	// This keyword has already been defined.  But since we are now
	// explicitly defining it, clear its bitmask, so that we will have a
	// new hash code--doing this will allow us to phase out the
	// historical hash code support later.
	((Keyword *)(yyvsp[(2) - (2)].u.keyword))->clear_historical_flag();
}
    break;

  case 28:
/* Line 1787 of yacc.c  */
#line 270 "Parser.ypp"
    {
		current_class = new Class(dc_file, (yyvsp[(2) - (2)].str));
	}
    break;

  case 29:
/* Line 1787 of yacc.c  */
#line 274 "Parser.ypp"
    {
		current_class->as_class()->rebuild_fields();
		(yyval.u.dclass) = current_class->as_class();
		current_class = (yyvsp[(3) - (7)].u.dclass);
	}
    break;

  case 30:
/* Line 1787 of yacc.c  */
#line 283 "Parser.ypp"
    {
		if(dc_file == (File *)NULL)
		{
			yyerror("No File available, so no class names are predefined.");
			(yyval.u.dclass) = NULL;
		}
		else
		{
			Struct *dc_struct = dc_file->get_class_by_name((yyvsp[(1) - (1)].str));
			if(dc_struct == (Class*)NULL)
			{
				yyerror("dclass '" + std::string((yyvsp[(1) - (1)].str)) + "' has not been declared.");
			}

			Class* dc_class = dc_struct->as_class();
			if(dc_class == (Class*)NULL)
			{
				yyerror("dclass cannot inherit from a struct");
			}

			(yyval.u.dclass) = dc_class;
		}
	}
    break;

  case 33:
/* Line 1787 of yacc.c  */
#line 315 "Parser.ypp"
    {
		if((yyvsp[(1) - (1)].u.dclass) != (Class*)NULL)
		{
			current_class->as_class()->add_parent((yyvsp[(1) - (1)].u.dclass));
		}
	}
    break;

  case 34:
/* Line 1787 of yacc.c  */
#line 322 "Parser.ypp"
    {
		if((yyvsp[(3) - (3)].u.dclass) != (Class*)NULL)
		{
			current_class->as_class()->add_parent((yyvsp[(3) - (3)].u.dclass));
		}
	}
    break;

  case 37:
/* Line 1787 of yacc.c  */
#line 334 "Parser.ypp"
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

  case 38:
/* Line 1787 of yacc.c  */
#line 348 "Parser.ypp"
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

  case 40:
/* Line 1787 of yacc.c  */
#line 361 "Parser.ypp"
    {
		yyerror("Unnamed parameters are not allowed on a dclass");
		if((yyvsp[(1) - (2)].u.parameter) != (Field *)NULL)
		{
			(yyvsp[(1) - (2)].u.parameter)->copy_keywords(current_keyword_list);
		}
		(yyval.u.field) = (yyvsp[(1) - (2)].u.parameter);
	}
    break;

  case 41:
/* Line 1787 of yacc.c  */
#line 370 "Parser.ypp"
    {
		if((yyvsp[(1) - (2)].u.parameter) != (Field *)NULL)
		{
			(yyvsp[(1) - (2)].u.parameter)->copy_keywords(current_keyword_list);
		}
		(yyval.u.field) = (yyvsp[(1) - (2)].u.parameter);
	}
    break;

  case 42:
/* Line 1787 of yacc.c  */
#line 381 "Parser.ypp"
    {
		current_class = new Struct(dc_file, (yyvsp[(2) - (2)].str));
	}
    break;

  case 43:
/* Line 1787 of yacc.c  */
#line 385 "Parser.ypp"
    {
		(yyval.u.dstruct) = current_class;
	}
    break;

  case 46:
/* Line 1787 of yacc.c  */
#line 394 "Parser.ypp"
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

  case 47:
/* Line 1787 of yacc.c  */
#line 407 "Parser.ypp"
    { (yyval.u.field) = (yyvsp[(1) - (2)].u.parameter); }
    break;

  case 48:
/* Line 1787 of yacc.c  */
#line 408 "Parser.ypp"
    { (yyval.u.field) = (yyvsp[(1) - (2)].u.parameter); }
    break;

  case 49:
/* Line 1787 of yacc.c  */
#line 413 "Parser.ypp"
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

  case 50:
/* Line 1787 of yacc.c  */
#line 424 "Parser.ypp"
    {
		(yyval.u.field) = current_atomic;
		current_atomic = (yyvsp[(3) - (5)].u.atomic);
	}
    break;

  case 55:
/* Line 1787 of yacc.c  */
#line 442 "Parser.ypp"
    {
		if((yyvsp[(1) - (1)].u.parameter) != (Parameter *)NULL)
		{
			current_atomic->add_element((yyvsp[(1) - (1)].u.parameter));
		}
	}
    break;

  case 56:
/* Line 1787 of yacc.c  */
#line 451 "Parser.ypp"
    { (yyval.u.parameter) = (yyvsp[(1) - (1)].u.parameter); }
    break;

  case 57:
/* Line 1787 of yacc.c  */
#line 452 "Parser.ypp"
    { (yyval.u.parameter) = (yyvsp[(1) - (1)].u.parameter); }
    break;

  case 58:
/* Line 1787 of yacc.c  */
#line 453 "Parser.ypp"
    { (yyval.u.parameter) = (yyvsp[(1) - (1)].u.parameter); }
    break;

  case 59:
/* Line 1787 of yacc.c  */
#line 457 "Parser.ypp"
    { (yyval.u.parameter) = (yyvsp[(1) - (1)].u.parameter); }
    break;

  case 60:
/* Line 1787 of yacc.c  */
#line 458 "Parser.ypp"
    { (yyval.u.parameter) = (yyvsp[(1) - (1)].u.parameter); }
    break;

  case 61:
/* Line 1787 of yacc.c  */
#line 459 "Parser.ypp"
    { (yyval.u.parameter) = (yyvsp[(1) - (1)].u.parameter); }
    break;

  case 62:
/* Line 1787 of yacc.c  */
#line 460 "Parser.ypp"
    { (yyval.u.parameter) = (yyvsp[(1) - (1)].u.parameter); }
    break;

  case 63:
/* Line 1787 of yacc.c  */
#line 461 "Parser.ypp"
    { (yyval.u.parameter) = (yyvsp[(1) - (1)].u.parameter); }
    break;

  case 64:
/* Line 1787 of yacc.c  */
#line 462 "Parser.ypp"
    { (yyval.u.parameter) = (yyvsp[(1) - (1)].u.parameter); }
    break;

  case 65:
/* Line 1787 of yacc.c  */
#line 466 "Parser.ypp"
    { (yyval.u.parameter) = (yyvsp[(1) - (1)].u.parameter); }
    break;

  case 66:
/* Line 1787 of yacc.c  */
#line 467 "Parser.ypp"
    { (yyval.u.parameter) = (yyvsp[(1) - (1)].u.parameter); }
    break;

  case 67:
/* Line 1787 of yacc.c  */
#line 471 "Parser.ypp"
    { (yyval.u.parameter) = current_parameter = new SimpleParameter((yyvsp[(1) - (1)].u.datatype));   }
    break;

  case 68:
/* Line 1787 of yacc.c  */
#line 473 "Parser.ypp"
    {
		Typedef *dtypedef = dc_file->get_typedef_by_name((yyvsp[(1) - (1)].str));
		if(dtypedef == (Typedef*)NULL)
		{
			// Maybe it's a class name.
			Struct *dc_struct = dc_file->get_class_by_name((yyvsp[(1) - (1)].str));
			if(dc_struct != (Struct*)NULL)
			{
				if(dc_struct->as_class() != (Class*)NULL)
				{
					yyerror("Cannot use distributed class '" + (yyvsp[(1) - (1)].str) + "' as parameter type.");
				}
				else
				{
					// Create an implicit typedef for this.
					dtypedef = new Typedef(new StructParameter(dc_struct), true);
				}
			}
			else
			{
				yyerror("Cannot use undefined type '" + (yyvsp[(1) - (1)].str) + "' as parameter type.");
			}

			dc_file->add_typedef(dtypedef);
		}

		(yyval.u.parameter) = current_parameter = dtypedef->new_parameter();
	}
    break;

  case 69:
/* Line 1787 of yacc.c  */
#line 505 "Parser.ypp"
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

  case 70:
/* Line 1787 of yacc.c  */
#line 522 "Parser.ypp"
    { (yyval.u.parameter) = param_with_modulus((yyvsp[(1) - (3)].u.parameter), (yyvsp[(3) - (3)].u.real)); }
    break;

  case 71:
/* Line 1787 of yacc.c  */
#line 523 "Parser.ypp"
    { (yyval.u.parameter) = param_with_modulus((yyvsp[(1) - (3)].u.parameter), (yyvsp[(3) - (3)].u.real)); }
    break;

  case 72:
/* Line 1787 of yacc.c  */
#line 527 "Parser.ypp"
    { (yyval.u.parameter) = param_with_divisor((yyvsp[(1) - (3)].u.parameter), (yyvsp[(3) - (3)].u.uint32)); }
    break;

  case 73:
/* Line 1787 of yacc.c  */
#line 528 "Parser.ypp"
    { (yyval.u.parameter) = param_with_divisor((yyvsp[(1) - (3)].u.parameter), (yyvsp[(3) - (3)].u.uint32)); }
    break;

  case 74:
/* Line 1787 of yacc.c  */
#line 529 "Parser.ypp"
    { (yyval.u.parameter) = param_with_divisor((yyvsp[(1) - (3)].u.parameter), (yyvsp[(3) - (3)].u.uint32)); }
    break;

  case 75:
/* Line 1787 of yacc.c  */
#line 533 "Parser.ypp"
    { (yyval.u.parameter) = (yyvsp[(1) - (2)].u.parameter); (yyvsp[(1) - (2)].u.parameter)->set_name((yyvsp[(2) - (2)].str)); }
    break;

  case 76:
/* Line 1787 of yacc.c  */
#line 534 "Parser.ypp"
    { (yyval.u.parameter) = (yyvsp[(1) - (2)].u.parameter); (yyvsp[(1) - (2)].u.parameter)->set_name((yyvsp[(2) - (2)].str)); }
    break;

  case 77:
/* Line 1787 of yacc.c  */
#line 535 "Parser.ypp"
    { (yyval.u.parameter) = (yyvsp[(1) - (2)].u.parameter); (yyvsp[(1) - (2)].u.parameter)->set_name((yyvsp[(2) - (2)].str)); }
    break;

  case 78:
/* Line 1787 of yacc.c  */
#line 536 "Parser.ypp"
    { (yyval.u.parameter) = (yyvsp[(1) - (2)].u.parameter); (yyvsp[(1) - (2)].u.parameter)->set_name((yyvsp[(2) - (2)].str)); }
    break;

  case 79:
/* Line 1787 of yacc.c  */
#line 540 "Parser.ypp"
    { (yyval.u.parameter) = (yyvsp[(1) - (4)].u.parameter)->append_array_specification((yyvsp[(3) - (4)].range)); }
    break;

  case 80:
/* Line 1787 of yacc.c  */
#line 541 "Parser.ypp"
    { (yyval.u.parameter) = (yyvsp[(1) - (4)].u.parameter)->append_array_specification((yyvsp[(3) - (4)].range)); }
    break;

  case 81:
/* Line 1787 of yacc.c  */
#line 542 "Parser.ypp"
    { (yyval.u.parameter) = (yyvsp[(1) - (4)].u.parameter)->append_array_specification((yyvsp[(3) - (4)].range)); }
    break;

  case 82:
/* Line 1787 of yacc.c  */
#line 543 "Parser.ypp"
    { (yyval.u.parameter) = (yyvsp[(1) - (4)].u.parameter)->append_array_specification((yyvsp[(3) - (4)].range)); }
    break;

  case 83:
/* Line 1787 of yacc.c  */
#line 547 "Parser.ypp"
    { (yyval.u.parameter) = (yyvsp[(1) - (4)].u.parameter)->append_array_specification((yyvsp[(3) - (4)].range)); }
    break;

  case 84:
/* Line 1787 of yacc.c  */
#line 551 "Parser.ypp"
    { (yyval.u.parameter) = (yyvsp[(1) - (3)].u.parameter); (yyvsp[(1) - (3)].u.parameter)->set_default_value((yyvsp[(3) - (3)].str)); }
    break;

  case 85:
/* Line 1787 of yacc.c  */
#line 552 "Parser.ypp"
    { (yyval.u.parameter) = (yyvsp[(1) - (3)].u.parameter); (yyvsp[(1) - (3)].u.parameter)->set_default_value((yyvsp[(3) - (3)].str)); }
    break;

  case 86:
/* Line 1787 of yacc.c  */
#line 553 "Parser.ypp"
    { (yyval.u.parameter) = (yyvsp[(1) - (3)].u.parameter); (yyvsp[(1) - (3)].u.parameter)->set_default_value((yyvsp[(3) - (3)].str)); }
    break;

  case 87:
/* Line 1787 of yacc.c  */
#line 554 "Parser.ypp"
    { (yyval.u.parameter) = (yyvsp[(1) - (3)].u.parameter); (yyvsp[(1) - (3)].u.parameter)->set_default_value((yyvsp[(3) - (3)].str)); }
    break;

  case 88:
/* Line 1787 of yacc.c  */
#line 555 "Parser.ypp"
    { (yyval.u.parameter) = (yyvsp[(1) - (3)].u.parameter); (yyvsp[(1) - (3)].u.parameter)->set_default_value((yyvsp[(3) - (3)].str)); }
    break;

  case 89:
/* Line 1787 of yacc.c  */
#line 559 "Parser.ypp"
    { (yyval.u.parameter) = (yyvsp[(1) - (3)].u.parameter); (yyvsp[(1) - (3)].u.parameter)->set_default_value((yyvsp[(3) - (3)].str)); }
    break;

  case 90:
/* Line 1787 of yacc.c  */
#line 560 "Parser.ypp"
    { (yyval.u.parameter) = (yyvsp[(1) - (3)].u.parameter); (yyvsp[(1) - (3)].u.parameter)->set_default_value((yyvsp[(3) - (3)].str)); }
    break;

  case 91:
/* Line 1787 of yacc.c  */
#line 564 "Parser.ypp"
    { (yyval.range) = NumericRange(); }
    break;

  case 92:
/* Line 1787 of yacc.c  */
#line 565 "Parser.ypp"
    { (yyval.range) = NumericRange((yyvsp[(1) - (1)].u.real), (yyvsp[(1) - (1)].u.real)); }
    break;

  case 93:
/* Line 1787 of yacc.c  */
#line 566 "Parser.ypp"
    { (yyval.range) = NumericRange((yyvsp[(1) - (3)].u.real), (yyvsp[(3) - (3)].u.real)); }
    break;

  case 94:
/* Line 1787 of yacc.c  */
#line 567 "Parser.ypp"
    { (yyval.range) = NumericRange((yyvsp[(1) - (2)].u.real), (yyvsp[(2) - (2)].u.real)); }
    break;

  case 95:
/* Line 1787 of yacc.c  */
#line 571 "Parser.ypp"
    { (yyval.range) = NumericRange(); }
    break;

  case 96:
/* Line 1787 of yacc.c  */
#line 572 "Parser.ypp"
    { (yyval.range) = NumericRange((yyvsp[(1) - (1)].u.uint32), (yyvsp[(1) - (1)].u.uint32)); }
    break;

  case 97:
/* Line 1787 of yacc.c  */
#line 573 "Parser.ypp"
    { (yyval.range) = NumericRange((yyvsp[(1) - (3)].u.uint32), (yyvsp[(3) - (3)].u.uint32)); }
    break;

  case 98:
/* Line 1787 of yacc.c  */
#line 574 "Parser.ypp"
    { (yyval.range) = NumericRange((yyvsp[(1) - (2)].u.uint32), (yyvsp[(2) - (2)].u.uint32)); }
    break;

  case 99:
/* Line 1787 of yacc.c  */
#line 579 "Parser.ypp"
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

  case 101:
/* Line 1787 of yacc.c  */
#line 594 "Parser.ypp"
    {
		(yyval.u.uint32) = (unsigned int)(yyvsp[(1) - (1)].u.uint64);
		if((yyval.u.uint32) != (yyvsp[(1) - (1)].u.uint64))
		{
			yyerror("Number out of range.");
			(yyval.u.uint32) = 1;
		}
	}
    break;

  case 102:
/* Line 1787 of yacc.c  */
#line 606 "Parser.ypp"
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

  case 103:
/* Line 1787 of yacc.c  */
#line 621 "Parser.ypp"
    { (yyval.u.real) = (double)(yyvsp[(1) - (1)].u.uint64); }
    break;

  case 104:
/* Line 1787 of yacc.c  */
#line 622 "Parser.ypp"
    { (yyval.u.real) = (double)(yyvsp[(1) - (1)].u.int64); }
    break;

  case 106:
/* Line 1787 of yacc.c  */
#line 628 "Parser.ypp"
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

  case 108:
/* Line 1787 of yacc.c  */
#line 644 "Parser.ypp"
    {
		// TODO: check for range limits
		(yyval.str) = std::string((char*)&(yyvsp[(1) - (1)].u.int64), sizeof(int64_t));
	}
    break;

  case 109:
/* Line 1787 of yacc.c  */
#line 649 "Parser.ypp"
    {
		// TODO: check for range limits
		(yyval.str) = std::string((char*)&(yyvsp[(1) - (1)].u.uint64), sizeof(uint64_t));
	}
    break;

  case 110:
/* Line 1787 of yacc.c  */
#line 654 "Parser.ypp"
    {
		// TODO: check for range limits
		(yyval.str) = std::string((char*)&(yyvsp[(1) - (1)].u.real), sizeof(double));
	}
    break;

  case 111:
/* Line 1787 of yacc.c  */
#line 659 "Parser.ypp"
    {
		DataType type = current_parameter->get_datatype();
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

  case 112:
/* Line 1787 of yacc.c  */
#line 678 "Parser.ypp"
    {
		// TODO: check for range limits... maybe?
		(yyval.str) = (yyvsp[(1) - (1)].str);
	}
    break;

  case 113:
/* Line 1787 of yacc.c  */
#line 683 "Parser.ypp"
    {
		DataType type = current_parameter->get_datatype();
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

  case 114:
/* Line 1787 of yacc.c  */
#line 697 "Parser.ypp"
    {
		DataType type = current_parameter->get_datatype();
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

  case 115:
/* Line 1787 of yacc.c  */
#line 724 "Parser.ypp"
    {
		// TODO: Check array type matches parameter type
		(yyval.str) = (yyvsp[(1) - (1)].str);
	}
    break;

  case 116:
/* Line 1787 of yacc.c  */
#line 729 "Parser.ypp"
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

  case 117:
/* Line 1787 of yacc.c  */
#line 739 "Parser.ypp"
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

  case 118:
/* Line 1787 of yacc.c  */
#line 749 "Parser.ypp"
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

  case 119:
/* Line 1787 of yacc.c  */
#line 759 "Parser.ypp"
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

  case 120:
/* Line 1787 of yacc.c  */
#line 769 "Parser.ypp"
    {
		// TODO: Check array type matches parameter type
		(yyval.str) = (yyvsp[(1) - (3)].str) + (yyvsp[(3) - (3)].str);
	}
    break;

  case 121:
/* Line 1787 of yacc.c  */
#line 776 "Parser.ypp"
    { (yyval.u.datatype) = DT_int8; }
    break;

  case 122:
/* Line 1787 of yacc.c  */
#line 777 "Parser.ypp"
    { (yyval.u.datatype) = DT_int16; }
    break;

  case 123:
/* Line 1787 of yacc.c  */
#line 778 "Parser.ypp"
    { (yyval.u.datatype) = DT_int32; }
    break;

  case 124:
/* Line 1787 of yacc.c  */
#line 779 "Parser.ypp"
    { (yyval.u.datatype) = DT_int64; }
    break;

  case 125:
/* Line 1787 of yacc.c  */
#line 780 "Parser.ypp"
    { (yyval.u.datatype) = DT_uint8; }
    break;

  case 126:
/* Line 1787 of yacc.c  */
#line 781 "Parser.ypp"
    { (yyval.u.datatype) = DT_uint16; }
    break;

  case 127:
/* Line 1787 of yacc.c  */
#line 782 "Parser.ypp"
    { (yyval.u.datatype) = DT_uint32; }
    break;

  case 128:
/* Line 1787 of yacc.c  */
#line 783 "Parser.ypp"
    { (yyval.u.datatype) = DT_uint64; }
    break;

  case 129:
/* Line 1787 of yacc.c  */
#line 784 "Parser.ypp"
    { (yyval.u.datatype) = DT_float32; }
    break;

  case 130:
/* Line 1787 of yacc.c  */
#line 785 "Parser.ypp"
    { (yyval.u.datatype) = DT_float64; }
    break;

  case 131:
/* Line 1787 of yacc.c  */
#line 786 "Parser.ypp"
    { (yyval.u.datatype) = DT_string; }
    break;

  case 132:
/* Line 1787 of yacc.c  */
#line 787 "Parser.ypp"
    { (yyval.u.datatype) = DT_blob; }
    break;

  case 133:
/* Line 1787 of yacc.c  */
#line 788 "Parser.ypp"
    { (yyval.u.datatype) = DT_char; }
    break;

  case 134:
/* Line 1787 of yacc.c  */
#line 792 "Parser.ypp"
    { current_keyword_list.clear_keywords(); }
    break;

  case 135:
/* Line 1787 of yacc.c  */
#line 793 "Parser.ypp"
    { current_keyword_list.add_keyword((yyvsp[(2) - (2)].u.keyword)); }
    break;

  case 136:
/* Line 1787 of yacc.c  */
#line 798 "Parser.ypp"
    {
		if(current_keyword_list.get_num_keywords() != 0)
		{
			yyerror("Keywords are not allowed here.");
		}
	}
    break;

  case 137:
/* Line 1787 of yacc.c  */
#line 807 "Parser.ypp"
    { current_molecular = new MolecularField((yyvsp[(1) - (2)].str), current_class); }
    break;

  case 138:
/* Line 1787 of yacc.c  */
#line 808 "Parser.ypp"
    { (yyval.u.field) = current_molecular; }
    break;

  case 139:
/* Line 1787 of yacc.c  */
#line 813 "Parser.ypp"
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

  case 140:
/* Line 1787 of yacc.c  */
#line 833 "Parser.ypp"
    {
		if((yyvsp[(1) - (1)].u.atomic) != (AtomicField *)NULL)
		{
			current_molecular->add_atomic((yyvsp[(1) - (1)].u.atomic));
		}
	}
    break;

  case 141:
/* Line 1787 of yacc.c  */
#line 840 "Parser.ypp"
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
#line 2678 "Parser.cpp"
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
#line 859 "Parser.ypp"
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
