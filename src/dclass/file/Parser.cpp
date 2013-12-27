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
#define yylex dcyylex
#define yyerror dcyyerror

// Because our token type contains objects of type std::string, which
// require correct copy construction (and not simply memcpying), we
// cannot use bison's built-in auto-stack-grow feature.  As an easy
// solution, we ensure here that we have enough yacc stack to start
// with, and that it doesn't ever try to grow.
#define YYINITDEPTH 1000
#define YYMAXDEPTH 1000

	File *dc_file = (File *)NULL;
	static Class *current_class = (Class *)NULL;
	static AtomicField *current_atomic = (AtomicField *)NULL;
	static MolecularField *current_molecular = (MolecularField *)NULL;
	static Parameter *current_parameter = (Parameter *)NULL;
	static KeywordList current_keyword_list;
	static DoubleRange double_range;
	static Field *parameter_description = (Field *)NULL;

	/* Helper functions */
	static Parameter* param_with_modulus(Parameter* p, double mod);
	static Parameter* param_with_divisor(Parameter* p, uint32_t div);

////////////////////////////////////////////////////////////////////
// Defining the interface to the parser.
////////////////////////////////////////////////////////////////////

	void
	dc_init_parser(std::istream & in, const std::string & filename, File & file)
	{
		dc_file = &file;
		dc_init_lexer(in, filename);
	}

	void
	dc_init_parser_parameter_value(std::istream & in, const std::string & filename)
	{
		dc_file = NULL;
		dc_init_lexer(in, filename);
		dc_start_parameter_value();
	}

	void
	dc_init_parser_parameter_description(std::istream & in, const std::string & filename,
	File * file)
	{
		dc_file = file;
		dc_init_lexer(in, filename);
		parameter_description = NULL;
		dc_start_parameter_description();
	}

	Field *
	dc_get_parameter_description()
	{
		return parameter_description;
	}

	void
	dc_cleanup_parser()
	{
		dc_file = (File *)NULL;
	}


/* Line 371 of yacc.c  */
#line 154 "Parser.cpp"

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
     START_PARAMETER_DESCRIPTION = 267,
     KW_DCLASS = 268,
     KW_STRUCT = 269,
     KW_FROM = 270,
     KW_IMPORT = 271,
     KW_TYPEDEF = 272,
     KW_KEYWORD = 273,
     KW_INT8 = 274,
     KW_INT16 = 275,
     KW_INT32 = 276,
     KW_INT64 = 277,
     KW_UINT8 = 278,
     KW_UINT16 = 279,
     KW_UINT32 = 280,
     KW_UINT64 = 281,
     KW_FLOAT32 = 282,
     KW_FLOAT64 = 283,
     KW_STRING = 284,
     KW_BLOB = 285,
     KW_CHAR = 286
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
#line 250 "Parser.cpp"

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
#define YYFINAL  42
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   253

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  47
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  64
/* YYNRULES -- Number of rules.  */
#define YYNRULES  162
/* YYNRULES -- Number of states.  */
#define YYNSTATES  247

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   286

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,    42,     2,     2,
      40,    41,    35,     2,    36,    46,    34,    33,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    39,    32,
       2,    45,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    43,     2,    44,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    37,     2,    38,     2,     2,     2,     2,
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
      25,    26,    27,    28,    29,    30,    31
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const yytype_uint16 yyprhs[] =
{
       0,     0,     3,     6,     9,    12,    14,    17,    20,    23,
      26,    29,    31,    35,    37,    41,    44,    45,    51,    53,
      55,    57,    61,    64,    67,    69,    72,    75,    77,    79,
      80,    88,    90,    92,    95,    97,   101,   103,   106,   110,
     113,   116,   119,   122,   123,   131,   133,   135,   138,   140,
     144,   146,   149,   153,   156,   159,   162,   165,   166,   172,
     174,   176,   178,   182,   184,   186,   188,   190,   192,   194,
     196,   198,   200,   202,   204,   206,   209,   212,   215,   217,
     219,   224,   228,   232,   236,   240,   244,   247,   250,   253,
     256,   261,   266,   271,   276,   281,   285,   289,   293,   297,
     301,   305,   309,   311,   313,   317,   320,   324,   330,   335,
     337,   339,   343,   346,   350,   356,   361,   363,   365,   367,
     369,   371,   373,   375,   377,   379,   381,   383,   385,   387,
     389,   393,   397,   401,   405,   409,   411,   414,   416,   418,
     420,   424,   426,   428,   430,   432,   434,   436,   438,   440,
     442,   444,   446,   448,   450,   452,   455,   457,   458,   463,
     465,   467,   471
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int8 yyrhs[] =
{
      48,     0,    -1,    10,    49,    -1,    11,    99,    -1,    12,
      82,    -1,   110,    -1,    49,    32,    -1,    49,    59,    -1,
      49,    52,    -1,    49,    56,    -1,    49,    57,    -1,     8,
      -1,    50,    33,     8,    -1,    50,    -1,    51,    34,    50,
      -1,    16,    51,    -1,    -1,    15,    51,    16,    53,    54,
      -1,    55,    -1,    35,    -1,    50,    -1,    55,    36,    50,
      -1,    17,    81,    -1,    18,    58,    -1,   110,    -1,    58,
       8,    -1,    58,     9,    -1,    60,    -1,    67,    -1,    -1,
      13,     8,    61,    63,    37,    65,    38,    -1,     8,    -1,
     110,    -1,    39,    64,    -1,    62,    -1,    64,    36,    62,
      -1,   110,    -1,    65,    32,    -1,    65,    66,    32,    -1,
      74,   104,    -1,   106,   105,    -1,    80,   104,    -1,    79,
     104,    -1,    -1,    14,     8,    68,    70,    37,    72,    38,
      -1,     8,    -1,   110,    -1,    39,    71,    -1,    69,    -1,
      71,    36,    69,    -1,   110,    -1,    72,    32,    -1,    72,
      73,    32,    -1,    74,   105,    -1,   106,   105,    -1,    80,
     105,    -1,    79,   105,    -1,    -1,     8,    40,    75,    76,
      41,    -1,   110,    -1,    77,    -1,    78,    -1,    77,    36,
      78,    -1,    81,    -1,    87,    -1,    89,    -1,    91,    -1,
      83,    -1,    84,    -1,    85,    -1,    86,    -1,    88,    -1,
      90,    -1,    79,    -1,    80,    -1,    74,   105,    -1,    80,
     105,    -1,    79,   105,    -1,   103,    -1,     8,    -1,   103,
      40,    92,    41,    -1,    83,    42,    97,    -1,    84,    42,
      97,    -1,    83,    33,    95,    -1,    84,    33,    95,    -1,
      85,    33,    95,    -1,    83,     8,    -1,    84,     8,    -1,
      85,     8,    -1,    86,     8,    -1,    83,    43,    93,    44,
      -1,    84,    43,    93,    44,    -1,    85,    43,    93,    44,
      -1,    86,    43,    93,    44,    -1,    87,    43,    93,    44,
      -1,    83,    45,    99,    -1,    84,    45,    99,    -1,    85,
      45,    99,    -1,    86,    45,    99,    -1,    88,    45,    99,
      -1,    87,    45,    99,    -1,    89,    45,    99,    -1,   110,
      -1,    98,    -1,    98,    46,    98,    -1,    98,    97,    -1,
      92,    36,    98,    -1,    92,    36,    98,    46,    98,    -1,
      92,    36,    98,    97,    -1,   110,    -1,    94,    -1,    94,
      46,    94,    -1,    94,    96,    -1,    93,    36,    94,    -1,
      93,    36,    94,    46,    94,    -1,    93,    36,    94,    96,
      -1,     6,    -1,    95,    -1,     3,    -1,     4,    -1,     3,
      -1,     4,    -1,     5,    -1,     6,    -1,    97,    -1,     4,
      -1,     3,    -1,     5,    -1,     6,    -1,     7,    -1,    37,
     100,    38,    -1,     4,    35,    95,    -1,     3,    35,    95,
      -1,     5,    35,    95,    -1,     7,    35,    95,    -1,   101,
      -1,   102,   101,    -1,   110,    -1,    36,    -1,    99,    -1,
     102,    36,    99,    -1,    19,    -1,    20,    -1,    21,    -1,
      22,    -1,    23,    -1,    24,    -1,    25,    -1,    26,    -1,
      27,    -1,    28,    -1,    29,    -1,    30,    -1,    31,    -1,
     110,    -1,   104,     9,    -1,   104,    -1,    -1,     8,    39,
     107,   109,    -1,     8,    -1,   108,    -1,   109,    36,   108,
      -1,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   173,   173,   174,   175,   179,   180,   181,   196,   197,
     198,   202,   203,   210,   211,   218,   223,   222,   230,   231,
     238,   242,   249,   272,   276,   277,   281,   292,   293,   298,
     297,   309,   336,   337,   341,   348,   358,   359,   360,   378,
     390,   391,   400,   412,   411,   423,   451,   452,   456,   463,
     473,   474,   475,   489,   497,   498,   499,   504,   503,   523,
     524,   528,   529,   533,   543,   544,   545,   549,   550,   551,
     552,   553,   554,   558,   559,   563,   567,   571,   578,   579,
     605,   623,   624,   628,   629,   630,   634,   635,   636,   637,
     641,   642,   643,   644,   648,   652,   653,   654,   655,   656,
     660,   661,   665,   666,   674,   682,   694,   701,   708,   722,
     723,   732,   741,   750,   757,   764,   774,   785,   789,   801,
     817,   818,   819,   823,   834,   838,   842,   846,   850,   854,
     858,   862,   870,   878,   886,   897,   898,   902,   903,   907,
     908,   912,   913,   914,   915,   916,   917,   918,   919,   920,
     921,   922,   923,   924,   928,   929,   933,   943,   943,   948,
     981,   988,  1003
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || 0
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "UNSIGNED_INTEGER", "SIGNED_INTEGER",
  "REAL", "STRING", "HEX_STRING", "IDENTIFIER", "KEYWORD", "START_DC",
  "START_PARAMETER_VALUE", "START_PARAMETER_DESCRIPTION", "KW_DCLASS",
  "KW_STRUCT", "KW_FROM", "KW_IMPORT", "KW_TYPEDEF", "KW_KEYWORD",
  "KW_INT8", "KW_INT16", "KW_INT32", "KW_INT64", "KW_UINT8", "KW_UINT16",
  "KW_UINT32", "KW_UINT64", "KW_FLOAT32", "KW_FLOAT64", "KW_STRING",
  "KW_BLOB", "KW_CHAR", "';'", "'/'", "'.'", "'*'", "','", "'{'", "'}'",
  "':'", "'('", "')'", "'%'", "'['", "']'", "'='", "'-'", "$accept",
  "grammar", "dc", "slash_identifier", "import_identifier", "import",
  "$@1", "import_symbol_list_or_star", "import_symbol_list",
  "typedef_decl", "keyword_decl", "keyword_decl_list", "dclass_or_struct",
  "dclass", "@2", "dclass_name", "dclass_derivation", "dclass_base_list",
  "dclass_fields", "dclass_field", "struct", "@3", "struct_name",
  "struct_derivation", "struct_base_list", "struct_fields", "struct_field",
  "atomic_field", "@4", "parameter_list", "nonempty_parameter_list",
  "atomic_element", "named_parameter", "unnamed_parameter", "parameter",
  "parameter_description", "param_w_typ", "param_w_rng", "param_w_mod",
  "param_w_div", "param_w_nam", "param_u_arr", "param_n_arr",
  "param_u_def", "param_n_def", "double_range", "uint_range",
  "char_or_uint", "small_unsigned_integer", "small_negative_integer",
  "number", "char_or_number", "parameter_value", "array", "maybe_comma",
  "array_def", "type_token", "keyword_list", "no_keyword_list",
  "molecular_field", "$@5", "atomic_name", "molecular_atom_list", "empty", YY_NULL
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
     285,   286,    59,    47,    46,    42,    44,   123,   125,    58,
      40,    41,    37,    91,    93,    61,    45
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    47,    48,    48,    48,    49,    49,    49,    49,    49,
      49,    50,    50,    51,    51,    52,    53,    52,    54,    54,
      55,    55,    56,    57,    58,    58,    58,    59,    59,    61,
      60,    62,    63,    63,    64,    64,    65,    65,    65,    66,
      66,    66,    66,    68,    67,    69,    70,    70,    71,    71,
      72,    72,    72,    73,    73,    73,    73,    75,    74,    76,
      76,    77,    77,    78,    79,    79,    79,    80,    80,    80,
      80,    80,    80,    81,    81,    82,    82,    82,    83,    83,
      84,    85,    85,    86,    86,    86,    87,    87,    87,    87,
      88,    88,    88,    88,    89,    90,    90,    90,    90,    90,
      91,    91,    92,    92,    92,    92,    92,    92,    92,    93,
      93,    93,    93,    93,    93,    93,    94,    94,    95,    96,
      97,    97,    97,    98,    98,    99,    99,    99,    99,    99,
      99,    99,    99,    99,    99,   100,   100,   101,   101,   102,
     102,   103,   103,   103,   103,   103,   103,   103,   103,   103,
     103,   103,   103,   103,   104,   104,   105,   107,   106,   108,
     109,   109,   110
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     2,     2,     2,     1,     2,     2,     2,     2,
       2,     1,     3,     1,     3,     2,     0,     5,     1,     1,
       1,     3,     2,     2,     1,     2,     2,     1,     1,     0,
       7,     1,     1,     2,     1,     3,     1,     2,     3,     2,
       2,     2,     2,     0,     7,     1,     1,     2,     1,     3,
       1,     2,     3,     2,     2,     2,     2,     0,     5,     1,
       1,     1,     3,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     2,     2,     2,     1,     1,
       4,     3,     3,     3,     3,     3,     2,     2,     2,     2,
       4,     4,     4,     4,     4,     3,     3,     3,     3,     3,
       3,     3,     1,     1,     3,     2,     3,     5,     4,     1,
       1,     3,     2,     3,     5,     4,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       3,     3,     3,     3,     3,     1,     2,     1,     1,     1,
       3,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     2,     1,     0,     4,     1,
       1,     3,     0
};

/* YYDEFACT[STATE-NAME] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       0,   162,     0,     0,     0,     2,     5,   126,   125,   127,
     128,   129,   162,     3,    79,   141,   142,   143,   144,   145,
     146,   147,   148,   149,   150,   151,   152,   153,   162,   162,
     162,     4,    67,    68,    69,    70,    64,    71,    65,    72,
      66,    78,     1,     0,     0,     0,     0,     0,   162,     6,
       8,     9,    10,     7,    27,    28,     0,     0,     0,     0,
     138,   139,     0,   135,   162,   137,    57,   156,    75,   154,
      77,    76,    86,     0,     0,   162,     0,    87,     0,     0,
     162,     0,    88,     0,   162,     0,    89,   162,     0,   162,
       0,     0,     0,   162,    29,    43,    11,    13,     0,    15,
      79,    73,    74,    22,    23,    24,   118,   132,   131,   133,
     134,   130,   138,   136,   162,   155,    83,   120,   121,   122,
      81,   116,     0,   110,   117,   109,    95,    84,    82,     0,
      96,    85,     0,    97,     0,    98,     0,   100,    99,   101,
     123,     0,   124,   103,   102,   162,   162,     0,    16,     0,
      25,    26,   140,     0,    60,    61,    63,    59,     0,    90,
     119,     0,   112,    91,    92,    93,    94,     0,    80,     0,
     105,     0,     0,    32,     0,     0,    46,    12,     0,    14,
      58,     0,   113,   111,   106,   104,    31,    34,    33,   162,
      45,    48,    47,   162,    19,    20,    17,    18,    62,     0,
     115,     0,   108,     0,     0,    36,     0,     0,    50,     0,
     114,   107,    35,    79,    37,    30,     0,   162,   162,   162,
     162,    49,    51,    44,     0,   162,   162,   162,   162,    21,
     157,    38,    39,    42,    41,    40,    52,    53,    56,    55,
      54,     0,   159,   160,   158,     0,   161
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     4,     5,    97,    98,    50,   178,   196,   197,    51,
      52,   104,    53,    54,   145,   187,   172,   188,   204,   216,
      55,   146,   191,   175,   192,   207,   224,    28,   114,   153,
     154,   155,   101,   102,   156,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,   141,   122,   123,   124,   162,
     142,   143,    13,    62,    63,    64,    41,    67,    68,   220,
     241,   243,   244,    69
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -166
static const yytype_int16 yypact[] =
{
      82,  -166,    42,   198,    21,   156,  -166,    17,    35,    55,
    -166,    63,    97,  -166,   106,  -166,  -166,  -166,  -166,  -166,
    -166,  -166,  -166,  -166,  -166,  -166,  -166,  -166,  -166,  -166,
    -166,  -166,    30,    54,    11,    22,    92,   100,   122,  -166,
    -166,   135,  -166,    49,   168,   169,   169,   222,  -166,  -166,
    -166,  -166,  -166,  -166,  -166,  -166,   175,   175,   175,   175,
    -166,  -166,   141,  -166,   145,  -166,  -166,   173,  -166,  -166,
    -166,  -166,  -166,   175,   136,    31,    42,  -166,   175,   136,
      31,    42,  -166,   175,    31,    42,  -166,    31,    42,    31,
      42,    42,    42,   105,  -166,  -166,  -166,   150,   -10,   152,
    -166,  -166,  -166,  -166,   123,  -166,  -166,  -166,  -166,  -166,
    -166,  -166,    42,  -166,   222,  -166,  -166,  -166,  -166,  -166,
    -166,  -166,    -4,     7,  -166,  -166,  -166,  -166,  -166,    -1,
    -166,  -166,    32,  -166,    44,  -166,    71,  -166,  -166,  -166,
    -166,     0,  -166,     5,  -166,   148,   153,   176,  -166,   169,
    -166,  -166,  -166,   155,   157,  -166,  -166,  -166,    31,  -166,
    -166,    31,  -166,  -166,  -166,  -166,  -166,   105,  -166,   105,
    -166,   181,   160,  -166,   186,   161,  -166,  -166,    20,   150,
    -166,   222,    14,  -166,    12,  -166,  -166,  -166,   163,  -166,
    -166,  -166,   164,  -166,  -166,   150,  -166,   171,  -166,    31,
    -166,   105,  -166,   181,    98,  -166,   186,   130,  -166,   169,
    -166,  -166,  -166,    74,  -166,  -166,   180,  -166,  -166,  -166,
    -166,  -166,  -166,  -166,   182,  -166,  -166,  -166,  -166,   150,
    -166,  -166,   173,   173,   173,  -166,  -166,  -166,  -166,  -166,
    -166,   205,  -166,  -166,   179,   205,  -166
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -166,  -166,  -166,  -145,   170,  -166,  -166,  -166,  -166,  -166,
    -166,  -166,  -166,  -166,  -166,    28,  -166,  -166,  -166,  -166,
    -166,  -166,    26,  -166,  -166,  -166,  -166,  -165,  -166,  -166,
    -166,    52,    -3,    -2,   187,  -166,  -166,  -166,  -166,  -166,
    -166,  -166,  -166,  -166,  -166,  -166,   -58,  -138,   107,    53,
     -72,  -142,    -7,  -166,   172,  -166,  -166,   -75,   -17,    33,
    -166,    -8,  -166,     2
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -1
static const yytype_uint8 yytable[] =
{
      29,    30,   120,     6,   179,    61,   148,   128,   117,   118,
     119,   160,    70,    71,    65,   117,   118,   119,   160,    82,
     182,    42,   129,   183,   149,   184,   132,   185,    96,   134,
      86,   136,   158,   195,   106,   158,   167,   121,    72,   217,
     159,   168,   225,   163,    83,     7,     8,     9,    10,    11,
     105,   169,    56,   161,    84,   194,    85,    94,   201,   211,
     199,   210,    77,    73,   229,    87,    65,    88,   158,   126,
      57,   170,    74,    75,   130,    76,   164,   125,   133,    12,
     158,   135,   125,   137,   138,   139,   125,    78,   165,   125,
      58,   125,     1,     2,     3,   144,    79,    80,    59,    81,
       7,     8,     9,    10,    11,   152,   213,   158,   117,   118,
     119,   140,   202,   230,    66,   166,   157,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
     214,   150,   151,    60,    12,    89,   215,    90,   213,   117,
     118,   119,   232,   233,   234,    91,    66,   173,   176,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,   222,   107,   108,   109,   110,    92,   223,    43,
      44,    45,    46,    47,    48,    93,    95,    96,   106,   111,
     116,   112,   115,   147,   177,   127,   149,   171,    49,   186,
     131,   205,   174,   181,   190,   208,   180,   189,   193,   203,
     206,   218,   219,   235,   226,   227,    14,   209,   237,   238,
     239,   240,   231,   242,   236,   245,    99,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
     100,   212,   221,   198,   103,   200,   113,   246,     0,     0,
     228,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27
};

#define yypact_value_is_default(Yystate) \
  (!!((Yystate) == (-166)))

#define yytable_value_is_error(Yytable_value) \
  YYID (0)

static const yytype_int16 yycheck[] =
{
       3,     3,    74,     1,   149,    12,    16,    79,     3,     4,
       5,     4,    29,    30,    12,     3,     4,     5,     4,     8,
     158,     0,    80,   161,    34,   167,    84,   169,     8,    87,
       8,    89,    36,   178,     3,    36,    36,     6,     8,   204,
      44,    41,   207,    44,    33,     3,     4,     5,     6,     7,
      48,    46,    35,    46,    43,    35,    45,     8,    46,   201,
      46,   199,     8,    33,   209,    43,    64,    45,    36,    76,
      35,   143,    42,    43,    81,    45,    44,    75,    85,    37,
      36,    88,    80,    90,    91,    92,    84,    33,    44,    87,
      35,    89,    10,    11,    12,    93,    42,    43,    35,    45,
       3,     4,     5,     6,     7,   112,     8,    36,     3,     4,
       5,     6,   184,    39,    40,    44,   114,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      32,     8,     9,    36,    37,    43,    38,    45,     8,     3,
       4,     5,   217,   218,   219,    45,    40,   145,   146,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    56,    57,    58,    59,    45,    38,    13,
      14,    15,    16,    17,    18,    40,     8,     8,     3,    38,
      73,    36,     9,    33,     8,    78,    34,    39,    32,     8,
      83,   189,    39,    36,     8,   193,    41,    37,    37,    36,
      36,   204,   204,   220,   207,   207,     8,    36,   225,   226,
     227,   228,    32,     8,    32,    36,    46,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
       8,   203,   206,   181,    47,   182,    64,   245,    -1,    -1,
     207,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,    10,    11,    12,    48,    49,   110,     3,     4,     5,
       6,     7,    37,    99,     8,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    74,    79,
      80,    82,    83,    84,    85,    86,    87,    88,    89,    90,
      91,   103,     0,    13,    14,    15,    16,    17,    18,    32,
      52,    56,    57,    59,    60,    67,    35,    35,    35,    35,
      36,    99,   100,   101,   102,   110,    40,   104,   105,   110,
     105,   105,     8,    33,    42,    43,    45,     8,    33,    42,
      43,    45,     8,    33,    43,    45,     8,    43,    45,    43,
      45,    45,    45,    40,     8,     8,     8,    50,    51,    51,
       8,    79,    80,    81,    58,   110,     3,    95,    95,    95,
      95,    38,    36,   101,    75,     9,    95,     3,     4,     5,
      97,     6,    93,    94,    95,   110,    99,    95,    97,    93,
      99,    95,    93,    99,    93,    99,    93,    99,    99,    99,
       6,    92,    97,    98,   110,    61,    68,    33,    16,    34,
       8,     9,    99,    76,    77,    78,    81,   110,    36,    44,
       4,    46,    96,    44,    44,    44,    44,    36,    41,    46,
      97,    39,    63,   110,    39,    70,   110,     8,    53,    50,
      41,    36,    94,    94,    98,    98,     8,    62,    64,    37,
       8,    69,    71,    37,    35,    50,    54,    55,    78,    46,
      96,    46,    97,    36,    65,   110,    36,    72,   110,    36,
      94,    98,    62,     8,    32,    38,    66,    74,    79,    80,
     106,    69,    32,    38,    73,    74,    79,    80,   106,    50,
      39,    32,   104,   104,   104,   105,    32,   105,   105,   105,
     105,   107,     8,   108,   109,    36,   108
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
        case 4:
/* Line 1787 of yacc.c  */
#line 175 "Parser.ypp"
    { parameter_description = (yyvsp[(2) - (2)].u.field); }
    break;

  case 7:
/* Line 1787 of yacc.c  */
#line 182 "Parser.ypp"
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

  case 12:
/* Line 1787 of yacc.c  */
#line 204 "Parser.ypp"
    {
	(yyval.str) = (yyvsp[(1) - (3)].str) + std::string("/") + (yyvsp[(3) - (3)].str);
}
    break;

  case 14:
/* Line 1787 of yacc.c  */
#line 212 "Parser.ypp"
    {
	(yyval.str) = (yyvsp[(1) - (3)].str) + std::string(".") + (yyvsp[(3) - (3)].str);
}
    break;

  case 15:
/* Line 1787 of yacc.c  */
#line 219 "Parser.ypp"
    {
	dc_file->add_import_module((yyvsp[(2) - (2)].str));
}
    break;

  case 16:
/* Line 1787 of yacc.c  */
#line 223 "Parser.ypp"
    {
	dc_file->add_import_module((yyvsp[(2) - (3)].str));
}
    break;

  case 19:
/* Line 1787 of yacc.c  */
#line 232 "Parser.ypp"
    {
	dc_file->add_import_symbol("*");
}
    break;

  case 20:
/* Line 1787 of yacc.c  */
#line 239 "Parser.ypp"
    {
	dc_file->add_import_symbol((yyvsp[(1) - (1)].str));
}
    break;

  case 21:
/* Line 1787 of yacc.c  */
#line 243 "Parser.ypp"
    {
	dc_file->add_import_symbol((yyvsp[(3) - (3)].str));
}
    break;

  case 22:
/* Line 1787 of yacc.c  */
#line 250 "Parser.ypp"
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

  case 25:
/* Line 1787 of yacc.c  */
#line 278 "Parser.ypp"
    {
	dc_file->add_keyword((yyvsp[(2) - (2)].str));
}
    break;

  case 26:
/* Line 1787 of yacc.c  */
#line 282 "Parser.ypp"
    {
	// This keyword has already been defined.  But since we are now
	// explicitly defining it, clear its bitmask, so that we will have a
	// new hash code--doing this will allow us to phase out the
	// historical hash code support later.
	((Keyword *)(yyvsp[(2) - (2)].u.keyword))->clear_historical_flag();
}
    break;

  case 29:
/* Line 1787 of yacc.c  */
#line 298 "Parser.ypp"
    {
		current_class = new Class(dc_file, (yyvsp[(2) - (2)].str), false, false);
	}
    break;

  case 30:
/* Line 1787 of yacc.c  */
#line 302 "Parser.ypp"
    {
		(yyval.u.dclass) = current_class;
		current_class = (yyvsp[(3) - (7)].u.dclass);
	}
    break;

  case 31:
/* Line 1787 of yacc.c  */
#line 310 "Parser.ypp"
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

  case 34:
/* Line 1787 of yacc.c  */
#line 342 "Parser.ypp"
    {
	if((yyvsp[(1) - (1)].u.dclass) != (Class *)NULL)
	{
		current_class->add_parent((yyvsp[(1) - (1)].u.dclass));
	}
}
    break;

  case 35:
/* Line 1787 of yacc.c  */
#line 349 "Parser.ypp"
    {
	if((yyvsp[(3) - (3)].u.dclass) != (Class *)NULL)
	{
			current_class->add_parent((yyvsp[(3) - (3)].u.dclass));
	}
}
    break;

  case 38:
/* Line 1787 of yacc.c  */
#line 361 "Parser.ypp"
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

  case 39:
/* Line 1787 of yacc.c  */
#line 379 "Parser.ypp"
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

  case 41:
/* Line 1787 of yacc.c  */
#line 392 "Parser.ypp"
    {
	yyerror("Unnamed parameters are not allowed on a dclass");
	if((yyvsp[(1) - (2)].u.parameter) != (Field *)NULL)
	{
		(yyvsp[(1) - (2)].u.parameter)->copy_keywords(current_keyword_list);
	}
	(yyval.u.field) = (yyvsp[(1) - (2)].u.parameter);
}
    break;

  case 42:
/* Line 1787 of yacc.c  */
#line 401 "Parser.ypp"
    {
	if((yyvsp[(1) - (2)].u.parameter) != (Field *)NULL)
	{
		(yyvsp[(1) - (2)].u.parameter)->copy_keywords(current_keyword_list);
	}
	(yyval.u.field) = (yyvsp[(1) - (2)].u.parameter);
}
    break;

  case 43:
/* Line 1787 of yacc.c  */
#line 412 "Parser.ypp"
    {
	current_class = new Class(dc_file, (yyvsp[(2) - (2)].str), true, false);
}
    break;

  case 44:
/* Line 1787 of yacc.c  */
#line 416 "Parser.ypp"
    {
	(yyval.u.dclass) = current_class;
	current_class = (yyvsp[(3) - (7)].u.dclass);
}
    break;

  case 45:
/* Line 1787 of yacc.c  */
#line 424 "Parser.ypp"
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

  case 48:
/* Line 1787 of yacc.c  */
#line 457 "Parser.ypp"
    {
		if((yyvsp[(1) - (1)].u.dclass) != (Class *)NULL)
		{
			current_class->add_parent((yyvsp[(1) - (1)].u.dclass));
		}
	}
    break;

  case 49:
/* Line 1787 of yacc.c  */
#line 464 "Parser.ypp"
    {
		if((yyvsp[(3) - (3)].u.dclass) != (Class *)NULL)
		{
			current_class->add_parent((yyvsp[(3) - (3)].u.dclass));
		}
	}
    break;

  case 52:
/* Line 1787 of yacc.c  */
#line 476 "Parser.ypp"
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

  case 53:
/* Line 1787 of yacc.c  */
#line 490 "Parser.ypp"
    {
		if((yyvsp[(1) - (2)].u.field)->get_name().empty())
		{
			yyerror("Field name required.");
		}
		(yyval.u.field) = (yyvsp[(1) - (2)].u.field);
	}
    break;

  case 55:
/* Line 1787 of yacc.c  */
#line 498 "Parser.ypp"
    { (yyval.u.field) = (yyvsp[(1) - (2)].u.parameter); }
    break;

  case 56:
/* Line 1787 of yacc.c  */
#line 499 "Parser.ypp"
    { (yyval.u.field) = (yyvsp[(1) - (2)].u.parameter); }
    break;

  case 57:
/* Line 1787 of yacc.c  */
#line 504 "Parser.ypp"
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

  case 58:
/* Line 1787 of yacc.c  */
#line 516 "Parser.ypp"
    {
		(yyval.u.field) = current_atomic;
		current_atomic = (yyvsp[(3) - (5)].u.atomic);
	}
    break;

  case 63:
/* Line 1787 of yacc.c  */
#line 534 "Parser.ypp"
    {
		if((yyvsp[(1) - (1)].u.parameter) != (Parameter *)NULL)
		{
			current_atomic->add_element((yyvsp[(1) - (1)].u.parameter));
		}
	}
    break;

  case 64:
/* Line 1787 of yacc.c  */
#line 543 "Parser.ypp"
    { (yyval.u.parameter) = (yyvsp[(1) - (1)].u.parameter); }
    break;

  case 65:
/* Line 1787 of yacc.c  */
#line 544 "Parser.ypp"
    { (yyval.u.parameter) = (yyvsp[(1) - (1)].u.parameter); }
    break;

  case 66:
/* Line 1787 of yacc.c  */
#line 545 "Parser.ypp"
    { (yyval.u.parameter) = (yyvsp[(1) - (1)].u.parameter); }
    break;

  case 67:
/* Line 1787 of yacc.c  */
#line 549 "Parser.ypp"
    { (yyval.u.parameter) = (yyvsp[(1) - (1)].u.parameter); }
    break;

  case 68:
/* Line 1787 of yacc.c  */
#line 550 "Parser.ypp"
    { (yyval.u.parameter) = (yyvsp[(1) - (1)].u.parameter); }
    break;

  case 69:
/* Line 1787 of yacc.c  */
#line 551 "Parser.ypp"
    { (yyval.u.parameter) = (yyvsp[(1) - (1)].u.parameter); }
    break;

  case 70:
/* Line 1787 of yacc.c  */
#line 552 "Parser.ypp"
    { (yyval.u.parameter) = (yyvsp[(1) - (1)].u.parameter); }
    break;

  case 71:
/* Line 1787 of yacc.c  */
#line 553 "Parser.ypp"
    { (yyval.u.parameter) = (yyvsp[(1) - (1)].u.parameter); }
    break;

  case 72:
/* Line 1787 of yacc.c  */
#line 554 "Parser.ypp"
    { (yyval.u.parameter) = (yyvsp[(1) - (1)].u.parameter); }
    break;

  case 73:
/* Line 1787 of yacc.c  */
#line 558 "Parser.ypp"
    { (yyval.u.parameter) = (yyvsp[(1) - (1)].u.parameter); }
    break;

  case 74:
/* Line 1787 of yacc.c  */
#line 559 "Parser.ypp"
    { (yyval.u.parameter) = (yyvsp[(1) - (1)].u.parameter); }
    break;

  case 75:
/* Line 1787 of yacc.c  */
#line 564 "Parser.ypp"
    {
		(yyval.u.field) = (yyvsp[(1) - (2)].u.field);
	}
    break;

  case 76:
/* Line 1787 of yacc.c  */
#line 568 "Parser.ypp"
    {
		(yyval.u.field) = (yyvsp[(1) - (2)].u.parameter);
	}
    break;

  case 77:
/* Line 1787 of yacc.c  */
#line 572 "Parser.ypp"
    {
		(yyval.u.field) = (yyvsp[(1) - (2)].u.parameter);
	}
    break;

  case 78:
/* Line 1787 of yacc.c  */
#line 578 "Parser.ypp"
    { (yyval.u.parameter) = new SimpleParameter((yyvsp[(1) - (1)].u.datatype)); }
    break;

  case 79:
/* Line 1787 of yacc.c  */
#line 580 "Parser.ypp"
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

		(yyval.u.parameter) = dtypedef->make_new_parameter();
	}
    break;

  case 80:
/* Line 1787 of yacc.c  */
#line 606 "Parser.ypp"
    {
		SimpleParameter *simple_param = new SimpleParameter((yyvsp[(1) - (4)].u.datatype));
		if(simple_param == NULL
		|| simple_param->get_typedef() != (Typedef*)NULL)
		{
			yyerror("Ranges are only valid for numeric, string, or blob types.");
		}
		if(!simple_param->set_range(double_range))
		{
			yyerror("Inappropriate range for type.");
		}

		(yyval.u.parameter) = simple_param;
	}
    break;

  case 81:
/* Line 1787 of yacc.c  */
#line 623 "Parser.ypp"
    { (yyval.u.parameter) = param_with_modulus((yyvsp[(1) - (3)].u.parameter), (yyvsp[(3) - (3)].u.real)); }
    break;

  case 82:
/* Line 1787 of yacc.c  */
#line 624 "Parser.ypp"
    { (yyval.u.parameter) = param_with_modulus((yyvsp[(1) - (3)].u.parameter), (yyvsp[(3) - (3)].u.real)); }
    break;

  case 83:
/* Line 1787 of yacc.c  */
#line 628 "Parser.ypp"
    { (yyval.u.parameter) = param_with_divisor((yyvsp[(1) - (3)].u.parameter), (yyvsp[(3) - (3)].u.uint32)); }
    break;

  case 84:
/* Line 1787 of yacc.c  */
#line 629 "Parser.ypp"
    { (yyval.u.parameter) = param_with_divisor((yyvsp[(1) - (3)].u.parameter), (yyvsp[(3) - (3)].u.uint32)); }
    break;

  case 85:
/* Line 1787 of yacc.c  */
#line 630 "Parser.ypp"
    { (yyval.u.parameter) = param_with_divisor((yyvsp[(1) - (3)].u.parameter), (yyvsp[(3) - (3)].u.uint32)); }
    break;

  case 86:
/* Line 1787 of yacc.c  */
#line 634 "Parser.ypp"
    { (yyval.u.parameter) = (yyvsp[(1) - (2)].u.parameter); (yyvsp[(1) - (2)].u.parameter)->set_name((yyvsp[(2) - (2)].str)); }
    break;

  case 87:
/* Line 1787 of yacc.c  */
#line 635 "Parser.ypp"
    { (yyval.u.parameter) = (yyvsp[(1) - (2)].u.parameter); (yyvsp[(1) - (2)].u.parameter)->set_name((yyvsp[(2) - (2)].str)); }
    break;

  case 88:
/* Line 1787 of yacc.c  */
#line 636 "Parser.ypp"
    { (yyval.u.parameter) = (yyvsp[(1) - (2)].u.parameter); (yyvsp[(1) - (2)].u.parameter)->set_name((yyvsp[(2) - (2)].str)); }
    break;

  case 89:
/* Line 1787 of yacc.c  */
#line 637 "Parser.ypp"
    { (yyval.u.parameter) = (yyvsp[(1) - (2)].u.parameter); (yyvsp[(1) - (2)].u.parameter)->set_name((yyvsp[(2) - (2)].str)); }
    break;

  case 90:
/* Line 1787 of yacc.c  */
#line 641 "Parser.ypp"
    { (yyval.u.parameter) = (yyvsp[(1) - (4)].u.parameter)->append_array_specification((yyvsp[(3) - (4)].urange)); }
    break;

  case 91:
/* Line 1787 of yacc.c  */
#line 642 "Parser.ypp"
    { (yyval.u.parameter) = (yyvsp[(1) - (4)].u.parameter)->append_array_specification((yyvsp[(3) - (4)].urange)); }
    break;

  case 92:
/* Line 1787 of yacc.c  */
#line 643 "Parser.ypp"
    { (yyval.u.parameter) = (yyvsp[(1) - (4)].u.parameter)->append_array_specification((yyvsp[(3) - (4)].urange)); }
    break;

  case 93:
/* Line 1787 of yacc.c  */
#line 644 "Parser.ypp"
    { (yyval.u.parameter) = (yyvsp[(1) - (4)].u.parameter)->append_array_specification((yyvsp[(3) - (4)].urange)); }
    break;

  case 94:
/* Line 1787 of yacc.c  */
#line 648 "Parser.ypp"
    { (yyval.u.parameter) = (yyvsp[(1) - (4)].u.parameter)->append_array_specification((yyvsp[(3) - (4)].urange)); }
    break;

  case 95:
/* Line 1787 of yacc.c  */
#line 652 "Parser.ypp"
    { (yyval.u.parameter) = (yyvsp[(1) - (3)].u.parameter); (yyvsp[(1) - (3)].u.parameter)->set_default_value((yyvsp[(3) - (3)].str)); }
    break;

  case 96:
/* Line 1787 of yacc.c  */
#line 653 "Parser.ypp"
    { (yyval.u.parameter) = (yyvsp[(1) - (3)].u.parameter); (yyvsp[(1) - (3)].u.parameter)->set_default_value((yyvsp[(3) - (3)].str)); }
    break;

  case 97:
/* Line 1787 of yacc.c  */
#line 654 "Parser.ypp"
    { (yyval.u.parameter) = (yyvsp[(1) - (3)].u.parameter); (yyvsp[(1) - (3)].u.parameter)->set_default_value((yyvsp[(3) - (3)].str)); }
    break;

  case 98:
/* Line 1787 of yacc.c  */
#line 655 "Parser.ypp"
    { (yyval.u.parameter) = (yyvsp[(1) - (3)].u.parameter); (yyvsp[(1) - (3)].u.parameter)->set_default_value((yyvsp[(3) - (3)].str)); }
    break;

  case 99:
/* Line 1787 of yacc.c  */
#line 656 "Parser.ypp"
    { (yyval.u.parameter) = (yyvsp[(1) - (3)].u.parameter); (yyvsp[(1) - (3)].u.parameter)->set_default_value((yyvsp[(3) - (3)].str)); }
    break;

  case 100:
/* Line 1787 of yacc.c  */
#line 660 "Parser.ypp"
    { (yyval.u.parameter) = (yyvsp[(1) - (3)].u.parameter); (yyvsp[(1) - (3)].u.parameter)->set_default_value((yyvsp[(3) - (3)].str)); }
    break;

  case 101:
/* Line 1787 of yacc.c  */
#line 661 "Parser.ypp"
    { (yyval.u.parameter) = (yyvsp[(1) - (3)].u.parameter); (yyvsp[(1) - (3)].u.parameter)->set_default_value((yyvsp[(3) - (3)].str)); }
    break;

  case 102:
/* Line 1787 of yacc.c  */
#line 665 "Parser.ypp"
    { double_range.clear(); }
    break;

  case 103:
/* Line 1787 of yacc.c  */
#line 667 "Parser.ypp"
    {
		double_range.clear();
		if(!double_range.add_range((yyvsp[(1) - (1)].u.real), (yyvsp[(1) - (1)].u.real)))
		{
			yyerror("Overlapping range");
		}
	}
    break;

  case 104:
/* Line 1787 of yacc.c  */
#line 675 "Parser.ypp"
    {
		double_range.clear();
		if(!double_range.add_range((yyvsp[(1) - (3)].u.real), (yyvsp[(3) - (3)].u.real)))
		{
			yyerror("Overlapping range");
		}
	}
    break;

  case 105:
/* Line 1787 of yacc.c  */
#line 683 "Parser.ypp"
    {
		double_range.clear();
		if((yyvsp[(2) - (2)].u.real) >= 0)
		{
			yyerror("Syntax error");
		}
		else if(!double_range.add_range((yyvsp[(1) - (2)].u.real), -(yyvsp[(2) - (2)].u.real)))
		{
			yyerror("Overlapping range");
		}
	}
    break;

  case 106:
/* Line 1787 of yacc.c  */
#line 695 "Parser.ypp"
    {
		if(!double_range.add_range((yyvsp[(3) - (3)].u.real), (yyvsp[(3) - (3)].u.real)))
		{
			yyerror("Overlapping range");
		}
	}
    break;

  case 107:
/* Line 1787 of yacc.c  */
#line 702 "Parser.ypp"
    {
		if(!double_range.add_range((yyvsp[(3) - (5)].u.real), (yyvsp[(5) - (5)].u.real)))
		{
			yyerror("Overlapping range");
		}
	}
    break;

  case 108:
/* Line 1787 of yacc.c  */
#line 709 "Parser.ypp"
    {
		if((yyvsp[(4) - (4)].u.real) >= 0)
		{
			yyerror("Syntax error");
		}
		else if(!double_range.add_range((yyvsp[(3) - (4)].u.real), -(yyvsp[(4) - (4)].u.real)))
		{
			yyerror("Overlapping range");
		}
	}
    break;

  case 109:
/* Line 1787 of yacc.c  */
#line 722 "Parser.ypp"
    { (yyval.urange) = UnsignedIntRange(); }
    break;

  case 110:
/* Line 1787 of yacc.c  */
#line 724 "Parser.ypp"
    {
		UnsignedIntRange uint_range;
		if(!uint_range.add_range((yyvsp[(1) - (1)].u.uint32), (yyvsp[(1) - (1)].u.uint32)))
		{
			yyerror("Overlapping range");
		}
		(yyval.urange) = uint_range;
	}
    break;

  case 111:
/* Line 1787 of yacc.c  */
#line 733 "Parser.ypp"
    {
		UnsignedIntRange uint_range;
		if(!uint_range.add_range((yyvsp[(1) - (3)].u.uint32), (yyvsp[(3) - (3)].u.uint32)))
		{
			yyerror("Overlapping range");
		}
		(yyval.urange) = uint_range;
	}
    break;

  case 112:
/* Line 1787 of yacc.c  */
#line 742 "Parser.ypp"
    {
		UnsignedIntRange uint_range;
		if(!uint_range.add_range((yyvsp[(1) - (2)].u.uint32), (yyvsp[(2) - (2)].u.uint32)))
		{
			yyerror("Overlapping range");
		}
		(yyval.urange) = uint_range;
	}
    break;

  case 113:
/* Line 1787 of yacc.c  */
#line 751 "Parser.ypp"
    {
		if(!(yyvsp[(1) - (3)].urange).add_range((yyvsp[(3) - (3)].u.uint32), (yyvsp[(3) - (3)].u.uint32)))
		{
			yyerror("Overlapping range");
		}
	}
    break;

  case 114:
/* Line 1787 of yacc.c  */
#line 758 "Parser.ypp"
    {
		if(!(yyvsp[(1) - (5)].urange).add_range((yyvsp[(3) - (5)].u.uint32), (yyvsp[(5) - (5)].u.uint32)))
		{
			yyerror("Overlapping range");
		}
	}
    break;

  case 115:
/* Line 1787 of yacc.c  */
#line 765 "Parser.ypp"
    {
		if(!(yyvsp[(1) - (4)].urange).add_range((yyvsp[(3) - (4)].u.uint32), (yyvsp[(4) - (4)].u.uint32)))
		{
			yyerror("Overlapping range");
		}
	}
    break;

  case 116:
/* Line 1787 of yacc.c  */
#line 775 "Parser.ypp"
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

  case 118:
/* Line 1787 of yacc.c  */
#line 790 "Parser.ypp"
    {
		(yyval.u.uint32) = (unsigned int)(yyvsp[(1) - (1)].u.uint64);
		if((yyval.u.uint32) != (yyvsp[(1) - (1)].u.uint64))
		{
			yyerror("Number out of range.");
			(yyval.u.uint32) = 1;
		}
	}
    break;

  case 119:
/* Line 1787 of yacc.c  */
#line 802 "Parser.ypp"
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

  case 120:
/* Line 1787 of yacc.c  */
#line 817 "Parser.ypp"
    { (yyval.u.real) = (double)(yyvsp[(1) - (1)].u.uint64); }
    break;

  case 121:
/* Line 1787 of yacc.c  */
#line 818 "Parser.ypp"
    { (yyval.u.real) = (double)(yyvsp[(1) - (1)].u.int64); }
    break;

  case 123:
/* Line 1787 of yacc.c  */
#line 824 "Parser.ypp"
    {
		if((yyvsp[(1) - (1)].str).length() != 1)
		{
			yyerror("Single character required.");
			(yyval.u.real) = 0;
		}
		else {
			(yyval.u.real) = (double)(unsigned char)(yyvsp[(1) - (1)].str)[0];
		}
	}
    break;

  case 125:
/* Line 1787 of yacc.c  */
#line 839 "Parser.ypp"
    {
		(yyval.str) = std::string((char*)&(yyvsp[(1) - (1)].u.int64), sizeof(int64_t));
	}
    break;

  case 126:
/* Line 1787 of yacc.c  */
#line 843 "Parser.ypp"
    {
		(yyval.str) = std::string((char*)&(yyvsp[(1) - (1)].u.uint64), sizeof(uint64_t));
	}
    break;

  case 127:
/* Line 1787 of yacc.c  */
#line 847 "Parser.ypp"
    {
		(yyval.str) = std::string((char*)&(yyvsp[(1) - (1)].u.real), sizeof(double));
	}
    break;

  case 128:
/* Line 1787 of yacc.c  */
#line 851 "Parser.ypp"
    {
		(yyval.str) = (yyvsp[(1) - (1)].str);
	}
    break;

  case 129:
/* Line 1787 of yacc.c  */
#line 855 "Parser.ypp"
    {
		(yyval.str) = (yyvsp[(1) - (1)].str);
	}
    break;

  case 130:
/* Line 1787 of yacc.c  */
#line 859 "Parser.ypp"
    {
		// todo
	}
    break;

  case 131:
/* Line 1787 of yacc.c  */
#line 863 "Parser.ypp"
    {
		std::string val;
		for(unsigned int i = 0; i < (yyvsp[(3) - (3)].u.uint32); i++)
		{
			val.append((char*)&(yyvsp[(1) - (3)].u.int64), sizeof(int64_t));
		}
	}
    break;

  case 132:
/* Line 1787 of yacc.c  */
#line 871 "Parser.ypp"
    {
		std::string val;
		for(unsigned int i = 0; i < (yyvsp[(3) - (3)].u.uint32); i++)
		{
			val.append((char*)&(yyvsp[(1) - (3)].u.uint64), sizeof(uint64_t));
		}
	}
    break;

  case 133:
/* Line 1787 of yacc.c  */
#line 879 "Parser.ypp"
    {
		std::string val;
		for(unsigned int i = 0; i < (yyvsp[(3) - (3)].u.uint32); i++)
		{
			val.append((char*)&(yyvsp[(1) - (3)].u.real), sizeof(double));
		}
	}
    break;

  case 134:
/* Line 1787 of yacc.c  */
#line 887 "Parser.ypp"
    {
		std::string val;
		for(unsigned int i = 0; i < (yyvsp[(3) - (3)].u.uint32); i++)
		{
			val += (yyvsp[(1) - (3)].str);
		}
	}
    break;

  case 141:
/* Line 1787 of yacc.c  */
#line 912 "Parser.ypp"
    { (yyval.u.datatype) = DT_int8; }
    break;

  case 142:
/* Line 1787 of yacc.c  */
#line 913 "Parser.ypp"
    { (yyval.u.datatype) = DT_int16; }
    break;

  case 143:
/* Line 1787 of yacc.c  */
#line 914 "Parser.ypp"
    { (yyval.u.datatype) = DT_int32; }
    break;

  case 144:
/* Line 1787 of yacc.c  */
#line 915 "Parser.ypp"
    { (yyval.u.datatype) = DT_int64; }
    break;

  case 145:
/* Line 1787 of yacc.c  */
#line 916 "Parser.ypp"
    { (yyval.u.datatype) = DT_uint8; }
    break;

  case 146:
/* Line 1787 of yacc.c  */
#line 917 "Parser.ypp"
    { (yyval.u.datatype) = DT_uint16; }
    break;

  case 147:
/* Line 1787 of yacc.c  */
#line 918 "Parser.ypp"
    { (yyval.u.datatype) = DT_uint32; }
    break;

  case 148:
/* Line 1787 of yacc.c  */
#line 919 "Parser.ypp"
    { (yyval.u.datatype) = DT_uint64; }
    break;

  case 149:
/* Line 1787 of yacc.c  */
#line 920 "Parser.ypp"
    { (yyval.u.datatype) = DT_float32; }
    break;

  case 150:
/* Line 1787 of yacc.c  */
#line 921 "Parser.ypp"
    { (yyval.u.datatype) = DT_float64; }
    break;

  case 151:
/* Line 1787 of yacc.c  */
#line 922 "Parser.ypp"
    { (yyval.u.datatype) = DT_string; }
    break;

  case 152:
/* Line 1787 of yacc.c  */
#line 923 "Parser.ypp"
    { (yyval.u.datatype) = DT_blob; }
    break;

  case 153:
/* Line 1787 of yacc.c  */
#line 924 "Parser.ypp"
    { (yyval.u.datatype) = DT_char; }
    break;

  case 154:
/* Line 1787 of yacc.c  */
#line 928 "Parser.ypp"
    { current_keyword_list.clear_keywords(); }
    break;

  case 155:
/* Line 1787 of yacc.c  */
#line 929 "Parser.ypp"
    { current_keyword_list.add_keyword((yyvsp[(2) - (2)].u.keyword)); }
    break;

  case 156:
/* Line 1787 of yacc.c  */
#line 934 "Parser.ypp"
    {
		if(current_keyword_list.get_num_keywords() != 0)
		{
			yyerror("Keywords are not allowed here.");
		}
	}
    break;

  case 157:
/* Line 1787 of yacc.c  */
#line 943 "Parser.ypp"
    { current_molecular = new MolecularField((yyvsp[(1) - (2)].str), current_class); }
    break;

  case 158:
/* Line 1787 of yacc.c  */
#line 944 "Parser.ypp"
    { (yyval.u.field) = current_molecular; }
    break;

  case 159:
/* Line 1787 of yacc.c  */
#line 949 "Parser.ypp"
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

  case 160:
/* Line 1787 of yacc.c  */
#line 982 "Parser.ypp"
    {
		if((yyvsp[(1) - (1)].u.atomic) != (AtomicField *)NULL)
		{
			current_molecular->add_atomic((yyvsp[(1) - (1)].u.atomic));
		}
	}
    break;

  case 161:
/* Line 1787 of yacc.c  */
#line 989 "Parser.ypp"
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
#line 2865 "Parser.cpp"
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
#line 1008 "Parser.ypp"
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
