/* A Bison parser, made by GNU Bison 3.0.2.  */

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
#define YYBISON_VERSION "3.0.2"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1




/* Copy the first part of user declarations.  */
#line 4 "parser.ypp" /* yacc.c:339  */

	#include "file/lexerDefs.h"
	#include "file/parserDefs.h"
	#include "file/parser.h"
	#include "file/write.h" // format_type(Type);

	#include "dc/File.h"
	#include "dc/DistributedType.h"
	#include "dc/NumericRange.h"
	#include "dc/NumericType.h"
	#include "dc/ArrayType.h"
	#include "dc/Struct.h"
	#include "dc/Class.h"
	#include "dc/Field.h"
	#include "dc/Method.h"
	#include "dc/Parameter.h"
	#include "dc/MolecularField.h"

	#include <unistd.h>
	#include <stdint.h> // Fixed width integer limits
	#include <math.h>   // Float INFINITY
	#include <stack>    // std::stack

	#define yyparse run_parser
	#define yylex run_lexer
	#define yyerror parser_error
	#define yywarning parser_warning
	#define yychar dcyychar
	#define yydebug dcyydebug
	#define yynerrs dcyynerrs

	// Because our token type contains objects of type string, which
	// require correct copy construction (and not simply memcpying), we
	// cannot use bison's built-in auto-stack-grow feature.  As an easy
	// solution, we ensure here that we have enough yacc stack to start
	// with, and that it doesn't ever try to grow.
	#define YYINITDEPTH 1000
	#define YYMAXDEPTH 1000

	using namespace std;
	namespace dclass   // open namespace dclass
	{


	// Parser output
	static File* parsed_file = (File*)NULL;
	static string* parsed_value = (string*)NULL;

	// Parser state
	static Class* current_class = (Class*)NULL;
	static Struct* current_struct = (Struct*)NULL;

	// Stack of distributed types for parsing values
	struct TypeAndDepth
	{
		int depth;
		const DistributedType* type;
		TypeAndDepth(const DistributedType* t, int d) : depth(d), type(t) {}
	};
	static stack<TypeAndDepth> type_stack;
	static int current_depth;

	// These two types are really common types the parser doesn't need to make new
	//     duplicates of every time a string or blob is used.
	static ArrayType* basic_string = (ArrayType*)NULL;
	static ArrayType* basic_blob = (ArrayType*)NULL;

	/* Helper functions */
	static bool check_depth();
	static void depth_error(string what);
	static void depth_error(int depth, string what);
	static string number_value(Type type, double &number);
	static string number_value(Type type, int64_t &number);
	static string number_value(Type type, uint64_t &number);

////////////////////////////////////////////////////////////////////
// Defining the interface to the parser.
////////////////////////////////////////////////////////////////////

	void init_file_parser(istream& in, const string& filename, File& file)
	{
		parsed_file = &file;
		init_file_lexer(in, filename);
	}

	void init_value_parser(istream& in, const string& source,
	                       const DistributedType* type, string& output)
	{
		parsed_value = &output;
		current_depth = 0;
		type_stack.push(TypeAndDepth(type, 0));
		init_value_lexer(in, source);
	}

	void cleanup_parser()
	{
		current_depth = 0;
		type_stack = stack<TypeAndDepth>();
		parsed_file = (File*)NULL;
		parsed_value = (string*)NULL;
	}

	int parser_error_count()
	{
		return lexer_error_count();
	}
	int parser_warning_count()
	{
		return lexer_warning_count();
	}

	void parser_error(const string &msg)
	{
		lexer_error(msg);
	}
	void parser_warning(const string &msg)
	{
		lexer_warning(msg);
	}

#line 187 "parser.cpp" /* yacc.c:339  */

# ifndef YY_NULLPTR
#  if defined __cplusplus && 201103L <= __cplusplus
#   define YY_NULLPTR nullptr
#  else
#   define YY_NULLPTR 0
#  endif
# endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 1
#endif

/* In a future release of Bison, this section will be replaced
   by #include "parser.h".  */
#ifndef YY_YY_PARSER_H_INCLUDED
# define YY_YY_PARSER_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 1
#endif
#if YYDEBUG
extern int yydebug;
#endif

/* Token type.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    UNSIGNED_INTEGER = 258,
    SIGNED_INTEGER = 259,
    REAL = 260,
    STRING = 261,
    HEX_STRING = 262,
    IDENTIFIER = 263,
    CHAR = 264,
    START_DC_FILE = 265,
    START_DC_VALUE = 266,
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

/* Value type.  */


extern YYSTYPE yylval;

int yyparse (void);

#endif /* !YY_YY_PARSER_H_INCLUDED  */

/* Copy the second part of user declarations.  */

#line 264 "parser.cpp" /* yacc.c:358  */

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
#else
typedef signed char yytype_int8;
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
# elif ! defined YYSIZE_T
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

#ifndef YY_ATTRIBUTE
# if (defined __GNUC__                                               \
      && (2 < __GNUC__ || (__GNUC__ == 2 && 96 <= __GNUC_MINOR__)))  \
     || defined __SUNPRO_C && 0x5110 <= __SUNPRO_C
#  define YY_ATTRIBUTE(Spec) __attribute__(Spec)
# else
#  define YY_ATTRIBUTE(Spec) /* empty */
# endif
#endif

#ifndef YY_ATTRIBUTE_PURE
# define YY_ATTRIBUTE_PURE   YY_ATTRIBUTE ((__pure__))
#endif

#ifndef YY_ATTRIBUTE_UNUSED
# define YY_ATTRIBUTE_UNUSED YY_ATTRIBUTE ((__unused__))
#endif

#if !defined _Noreturn \
     && (!defined __STDC_VERSION__ || __STDC_VERSION__ < 201112)
# if defined _MSC_VER && 1200 <= _MSC_VER
#  define _Noreturn __declspec (noreturn)
# else
#  define _Noreturn YY_ATTRIBUTE ((__noreturn__))
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YYUSE(E) ((void) (E))
#else
# define YYUSE(E) /* empty */
#endif

#if defined __GNUC__ && 407 <= __GNUC__ * 100 + __GNUC_MINOR__
/* Suppress an incorrect diagnostic about yylval being uninitialized.  */
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN \
    _Pragma ("GCC diagnostic push") \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")\
    _Pragma ("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")
# define YY_IGNORE_MAYBE_UNINITIALIZED_END \
    _Pragma ("GCC diagnostic pop")
#else
# define YY_INITIAL_VALUE(Value) Value
#endif
#ifndef YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_END
#endif
#ifndef YY_INITIAL_VALUE
# define YY_INITIAL_VALUE(Value) /* Nothing. */
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
#    if ! defined _ALLOCA_H && ! defined EXIT_SUCCESS
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
   /* Pacify GCC's 'empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
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
#   if ! defined malloc && ! defined EXIT_SUCCESS
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined EXIT_SUCCESS
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
# define YYSTACK_RELOCATE(Stack_alloc, Stack)                           \
    do                                                                  \
      {                                                                 \
        YYSIZE_T yynewbytes;                                            \
        YYCOPY (&yyptr->Stack_alloc, Stack, yysize);                    \
        Stack = &yyptr->Stack_alloc;                                    \
        yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
        yyptr += yynewbytes / sizeof (*yyptr);                          \
      }                                                                 \
    while (0)

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
      while (0)
#  endif
# endif
#endif /* !YYCOPY_NEEDED */

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  20
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   255

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  46
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  79
/* YYNRULES -- Number of rules.  */
#define YYNRULES  171
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  264

/* YYTRANSLATE[YYX] -- Symbol number corresponding to YYX as returned
   by yylex, with out-of-bounds checking.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   285

#define YYTRANSLATE(YYX)                                                \
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, without out-of-bounds checking.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,    44,     2,     2,
      42,    43,    33,     2,    34,    45,    32,    35,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    40,    31,
       2,    41,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    36,     2,    37,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    38,     2,    39,     2,     2,     2,     2,
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
  /* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   229,   229,   230,   235,   236,   237,   238,   239,   240,
     244,   248,   252,   257,   265,   270,   279,   280,   287,   288,
     295,   299,   307,   308,   315,   360,   364,   372,   382,   397,
     401,   402,   410,   409,   449,   450,   454,   461,   471,   502,
     503,   504,   542,   566,   574,   573,   613,   614,   615,   649,
     650,   654,   655,   656,   657,   661,   666,   665,   680,   687,
     692,   701,   700,   712,   711,   723,   722,   736,   743,   744,
     748,   766,   770,   774,   778,   782,   786,   793,   819,   851,
     873,   901,   924,   925,   926,   927,   931,   935,   944,   953,
     965,   977,   984,   994,   998,  1005,  1015,  1028,  1029,  1030,
    1031,  1036,  1035,  1049,  1056,  1061,  1070,  1069,  1081,  1080,
    1094,  1095,  1096,  1097,  1101,  1102,  1103,  1104,  1108,  1120,
    1124,  1137,  1154,  1155,  1156,  1160,  1172,  1176,  1185,  1194,
    1203,  1233,  1263,  1264,  1269,  1268,  1305,  1306,  1314,  1313,
    1350,  1351,  1352,  1356,  1363,  1389,  1388,  1437,  1439,  1438,
    1453,  1458,  1474,  1490,  1506,  1556,  1610,  1611,  1615,  1616,
    1617,  1618,  1619,  1620,  1621,  1622,  1623,  1624,  1625,  1629,
    1633,  1646
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || 1
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "UNSIGNED_INTEGER", "SIGNED_INTEGER",
  "REAL", "STRING", "HEX_STRING", "IDENTIFIER", "CHAR", "START_DC_FILE",
  "START_DC_VALUE", "KW_DCLASS", "KW_STRUCT", "KW_FROM", "KW_IMPORT",
  "KW_TYPEDEF", "KW_KEYWORD", "KW_INT8", "KW_INT16", "KW_INT32",
  "KW_INT64", "KW_UINT8", "KW_UINT16", "KW_UINT32", "KW_UINT64",
  "KW_FLOAT32", "KW_FLOAT64", "KW_STRING", "KW_BLOB", "KW_CHAR", "';'",
  "'.'", "'*'", "','", "'/'", "'['", "']'", "'{'", "'}'", "':'", "'='",
  "'('", "')'", "'%'", "'-'", "$accept", "grammar", "file", "value",
  "import", "import_module", "import_symbols", "import_symbol_list",
  "import_alternatives", "typedef", "typedef_type",
  "nonmethod_type_with_name", "defined_type", "keyword_decl",
  "keyword_decl_list", "dclass", "$@1", "class_inheritance",
  "class_parents", "defined_class", "class_fields", "class_field",
  "dstruct", "$@2", "struct_fields", "struct_field", "named_field",
  "unnamed_field", "$@3", "field_with_name", "field_with_name_as_array",
  "field_with_name_and_default", "$@4", "$@5", "$@6", "method_as_field",
  "nonmethod_type", "nonmethod_type_no_array", "type_with_array",
  "molecular", "defined_field", "builtin_array_type", "numeric_type",
  "numeric_token_only", "numeric_with_range", "numeric_with_modulus",
  "numeric_with_divisor", "method", "method_body", "parameter", "$@7",
  "param_with_name", "param_with_name_as_array",
  "param_with_name_and_default", "$@8", "$@9", "numeric_range",
  "array_range", "char_or_uint", "small_unsigned_integer",
  "small_negative_integer", "number", "char_or_number", "type_value",
  "method_value", "$@10", "parameter_values", "struct_value", "$@11",
  "field_values", "array_value", "$@12", "element_values", "$@13",
  "array_expansion", "array_type_token", "numeric_type_token",
  "keyword_list", "empty", YY_NULLPTR
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[NUM] -- (External) token number corresponding to the
   (internal) symbol number NUM (which must be that of a token).  */
static const yytype_uint16 yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,    59,    46,    42,    44,    47,    91,    93,   123,   125,
      58,    61,    40,    41,    37,    45
};
# endif

#define YYPACT_NINF -188

#define yypact_value_is_default(Yystate) \
  (!!((Yystate) == (-188)))

#define YYTABLE_NINF -1

#define yytable_value_is_error(Yytable_value) \
  0

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int16 yypact[] =
{
      54,  -188,    17,    33,   129,  -188,  -188,  -188,  -188,  -188,
    -188,    59,  -188,  -188,  -188,  -188,  -188,  -188,  -188,  -188,
    -188,    89,    92,   115,   115,    85,  -188,  -188,  -188,  -188,
      25,  -188,  -188,  -188,  -188,    24,    17,    44,  -188,  -188,
    -188,    10,    91,   119,  -188,  -188,  -188,  -188,  -188,  -188,
    -188,  -188,  -188,  -188,  -188,  -188,  -188,  -188,  -188,   113,
     144,  -188,   123,   154,   157,     1,  -188,    53,   153,   155,
    -188,   147,  -188,    29,   163,   165,   166,   167,   168,  -188,
       3,  -188,  -188,  -188,    52,  -188,    20,   162,   169,     6,
     115,   195,    29,  -188,    29,    29,    29,   202,   180,   116,
     202,   180,   180,    29,  -188,  -188,  -188,   171,     7,  -188,
    -188,   202,   202,   202,   202,   202,  -188,  -188,    17,  -188,
      44,  -188,   198,   172,  -188,  -188,  -188,  -188,   175,    91,
      91,  -188,   174,   176,   177,   178,  -188,  -188,  -188,  -188,
    -188,   173,  -188,    13,  -188,  -188,  -188,   179,   181,   182,
    -188,  -188,    29,  -188,  -188,  -188,  -188,  -188,  -188,    24,
    -188,  -188,  -188,  -188,   183,  -188,  -188,   109,  -188,   115,
    -188,  -188,  -188,  -188,  -188,   180,  -188,  -188,  -188,  -188,
    -188,  -188,   198,   149,  -188,   170,  -188,  -188,  -188,   187,
    -188,  -188,    58,   117,  -188,   185,     5,    91,  -188,  -188,
     152,  -188,  -188,   188,  -188,   186,    49,  -188,    47,  -188,
      29,  -188,    29,  -188,   190,  -188,   213,  -188,   215,  -188,
     213,  -188,   189,   219,  -188,   146,   150,  -188,    85,  -188,
     191,    44,   197,    44,  -188,    44,  -188,  -188,  -188,  -188,
    -188,  -188,    29,  -188,    29,  -188,  -188,  -188,  -188,  -188,
    -188,    44,  -188,    44,   199,    44,   200,    44,  -188,  -188,
    -188,  -188,  -188,  -188
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       0,   171,   171,     0,     2,     4,   128,   127,   129,   130,
     131,   145,   138,   134,     3,    12,    13,   133,   132,    11,
       1,     0,     0,     0,     0,     0,   171,     5,     8,     9,
      24,    10,     6,     7,   144,     0,     0,     0,    32,    44,
      22,     0,    16,    14,    28,   159,   160,   161,   162,   163,
     164,   165,   166,   167,   168,   156,   157,   158,    25,    70,
       0,    68,    69,    72,    71,    82,    85,    83,    84,    80,
      86,    29,    30,   171,   128,   127,   129,   130,   131,   150,
       0,   147,   140,   141,     0,   136,     0,   171,     0,     0,
       0,     0,   171,    27,   171,   171,   171,     0,   171,     0,
       0,   171,   171,   171,    31,   120,   118,     0,   115,   119,
     114,     0,     0,     0,     0,     0,   148,   146,     0,   139,
       0,   135,     0,     0,    34,   171,    19,    15,    18,    20,
      17,    23,     0,     0,     0,     0,    91,   122,   123,   124,
     125,     0,   126,   111,   110,    90,    92,     0,     0,     0,
      26,   121,     0,   117,   152,   151,   153,   154,   155,     0,
     142,   143,   137,    38,    35,    36,   171,     0,    46,     0,
      74,    76,    75,    73,    87,     0,   113,    88,    89,    81,
     116,   149,     0,     0,    39,    28,    47,    45,    58,     0,
      49,    50,    51,    52,    53,    54,    55,    21,   112,    37,
      28,    40,    33,     0,   171,    43,     0,    67,     0,    48,
     171,    61,   171,    63,     0,    56,     0,    41,    42,   169,
       0,    93,   100,    68,    95,    97,    98,    99,     0,    94,
       0,     0,     0,     0,    65,     0,    79,    77,   170,    78,
     101,   103,   171,   106,   171,   108,    96,    59,    62,    60,
      64,     0,    57,     0,     0,     0,     0,     0,    66,   102,
     104,   107,   105,   109
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -188,  -188,  -188,  -188,  -188,   211,  -188,  -188,   -80,  -188,
    -188,   214,  -188,  -188,  -188,  -188,  -188,  -188,  -188,    56,
    -188,  -188,  -188,  -188,  -188,  -188,    57,  -188,  -188,  -188,
    -188,  -188,  -188,  -188,  -188,  -188,  -162,  -187,  -188,  -188,
      21,  -188,  -188,  -188,  -188,  -188,  -188,  -188,  -188,    14,
    -188,  -188,  -188,  -188,  -188,  -188,   -18,   -88,    93,    50,
    -188,   -87,    68,    -2,   -33,  -188,  -188,  -188,  -188,  -188,
    -188,  -188,  -188,  -188,    87,  -188,  -188,  -188,     0
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     3,     4,    14,    28,    41,   127,   128,    42,    29,
      30,   188,    59,    31,    71,    32,    87,   123,   164,   165,
     183,   203,    33,    88,   167,   189,   190,   191,   235,   192,
     193,   194,   231,   233,   251,   195,    60,    61,    62,   205,
     237,    63,    64,    65,    66,    67,    68,   207,   208,   224,
     253,   225,   226,   227,   255,   257,   141,   107,   108,   109,
     153,   142,   143,    79,    16,    37,    86,    17,    36,    84,
      18,    35,    80,   159,    81,    69,    70,   218,   110
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_uint16 yytable[] =
{
      15,     5,    19,    83,   132,   196,   133,   134,   135,   129,
     130,   151,   145,    93,    40,   149,   137,   138,   139,   223,
       6,     7,     8,     9,    10,    89,    72,    74,    75,    76,
      77,    78,   105,    20,    82,    85,    97,   116,   106,   126,
     117,   223,    90,    98,   222,    99,   215,     6,     7,     8,
       9,    10,   152,    11,   120,    12,   176,    44,   175,    13,
      11,    73,    12,   121,     1,     2,   222,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    56,    57,
      11,   228,    12,   147,   148,   161,   118,   124,   100,   197,
     229,   119,   221,    44,   210,   101,    34,    38,   144,   211,
      39,   144,   144,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    56,    57,   160,   185,   162,   137,
     138,   139,   230,    40,   232,   168,    91,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    56,    57,
     186,    21,    22,    23,    24,    25,    26,   136,   187,    92,
     146,    90,    93,   212,   254,   104,   256,   200,   213,    94,
      27,   154,   155,   156,   157,   158,   184,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    56,    57,
     201,   234,   242,   137,   138,   139,   244,   243,   202,   140,
      95,   245,   216,    96,   206,   102,   111,   103,   112,   113,
     114,   115,   122,   131,   219,   105,   163,   125,   150,   169,
     166,   170,   206,   171,   172,   173,   174,   182,   209,   217,
     220,   236,   177,   238,   178,   179,   214,   241,   247,   248,
     240,   250,    13,   252,   249,    43,   260,   262,   199,    58,
     204,   239,   246,   198,     0,   180,   181,     0,     0,   258,
       0,   259,     0,   261,     0,   263
};

static const yytype_int16 yycheck[] =
{
       2,     1,     2,    36,    92,   167,    94,    95,    96,    89,
      90,     4,    99,     8,     8,   103,     3,     4,     5,   206,
       3,     4,     5,     6,     7,    15,    26,     3,     4,     5,
       6,     7,     3,     0,    36,    37,    35,    34,     9,    33,
      37,   228,    32,    42,   206,    44,    41,     3,     4,     5,
       6,     7,    45,    36,    34,    38,   143,     8,    45,    42,
      36,    36,    38,    43,    10,    11,   228,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    30,
      36,    34,    38,   101,   102,   118,    34,    87,    35,   169,
      43,    39,    43,     8,    36,    42,    37,     8,    98,    41,
       8,   101,   102,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,   118,     8,   120,     3,
       4,     5,   210,     8,   212,   125,    35,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    30,
      31,    12,    13,    14,    15,    16,    17,    97,    39,    36,
     100,    32,     8,    36,   242,     8,   244,     8,    41,    36,
      31,   111,   112,   113,   114,   115,   166,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    30,
      31,   214,    36,     3,     4,     5,    36,    41,    39,     9,
      36,    41,    40,    36,    42,    42,    33,    42,    33,    33,
      33,    33,    40,     8,   204,     3,     8,    38,    37,    34,
      38,    37,    42,    37,    37,    37,    43,    34,    31,    31,
      34,     8,    43,     8,    43,    43,    41,     8,    37,   231,
      41,   233,    42,   235,    37,    24,    37,    37,   182,    25,
     183,   220,   228,   175,    -1,   152,   159,    -1,    -1,   251,
      -1,   253,    -1,   255,    -1,   257
};

  /* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,    10,    11,    47,    48,   124,     3,     4,     5,     6,
       7,    36,    38,    42,    49,   109,   110,   113,   116,   124,
       0,    12,    13,    14,    15,    16,    17,    31,    50,    55,
      56,    59,    61,    68,    37,   117,   114,   111,     8,     8,
       8,    51,    54,    51,     8,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    57,    58,
      82,    83,    84,    87,    88,    89,    90,    91,    92,   121,
     122,    60,   124,    36,     3,     4,     5,     6,     7,   109,
     118,   120,   109,   110,   115,   109,   112,    62,    69,    15,
      32,    35,    36,     8,    36,    36,    36,    35,    42,    44,
      35,    42,    42,    42,     8,     3,     9,   103,   104,   105,
     124,    33,    33,    33,    33,    33,    34,    37,    34,    39,
      34,    43,    40,    63,   124,    38,    33,    52,    53,    54,
      54,     8,   103,   103,   103,   103,   105,     3,     4,     5,
       9,   102,   107,   108,   124,   107,   105,   102,   102,   103,
      37,     4,    45,   106,   105,   105,   105,   105,   105,   119,
     109,   110,   109,     8,    64,    65,    38,    70,   124,    34,
      37,    37,    37,    37,    43,    45,   107,    43,    43,    43,
     104,   120,    34,    66,   124,     8,    31,    39,    57,    71,
      72,    73,    75,    76,    77,    81,    82,    54,   108,    65,
       8,    31,    39,    67,    72,    85,    42,    93,    94,    31,
      36,    41,    36,    41,    41,    41,    40,    31,   123,   124,
      34,    43,    82,    83,    95,    97,    98,    99,    34,    43,
     103,    78,   103,    79,   110,    74,     8,    86,     8,    86,
      41,     8,    36,    41,    36,    41,    95,    37,   109,    37,
     109,    80,   109,    96,   103,   100,   103,   101,   109,   109,
      37,   109,    37,   109
};

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    46,    47,    47,    48,    48,    48,    48,    48,    48,
      48,    49,    49,    49,    50,    50,    51,    51,    52,    52,
      53,    53,    54,    54,    55,    56,    56,    57,    58,    59,
      60,    60,    62,    61,    63,    63,    64,    64,    65,    66,
      66,    66,    67,    67,    69,    68,    70,    70,    70,    71,
      71,    72,    72,    72,    72,    73,    74,    73,    75,    76,
      76,    78,    77,    79,    77,    80,    77,    81,    82,    82,
      83,    83,    83,    84,    84,    84,    84,    85,    85,    86,
      87,    87,    88,    88,    88,    88,    89,    90,    90,    90,
      91,    92,    92,    93,    93,    94,    94,    95,    95,    95,
      95,    96,    95,    97,    98,    98,   100,    99,   101,    99,
     102,   102,   102,   102,   103,   103,   103,   103,   104,   104,
     105,   106,   107,   107,   107,   108,   108,   109,   109,   109,
     109,   109,   109,   109,   111,   110,   112,   112,   114,   113,
     115,   115,   115,   115,   116,   117,   116,   118,   119,   118,
     120,   120,   120,   120,   120,   120,   121,   121,   122,   122,
     122,   122,   122,   122,   122,   122,   122,   122,   122,   123,
     123,   124
};

  /* YYR2[YYN] -- Number of symbols on the right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     2,     2,     1,     2,     2,     2,     2,     2,
       2,     1,     1,     1,     2,     4,     1,     3,     1,     1,
       1,     3,     1,     3,     1,     2,     4,     2,     1,     2,
       1,     2,     0,     7,     1,     2,     1,     3,     1,     1,
       2,     3,     2,     1,     0,     6,     1,     2,     3,     1,
       1,     1,     1,     1,     1,     1,     0,     4,     1,     4,
       4,     0,     4,     0,     4,     0,     5,     2,     1,     1,
       1,     1,     1,     4,     4,     4,     4,     3,     3,     1,
       1,     4,     1,     1,     1,     1,     1,     4,     4,     4,
       3,     3,     3,     2,     2,     2,     3,     1,     1,     1,
       1,     0,     4,     2,     4,     4,     0,     4,     0,     4,
       1,     1,     3,     2,     1,     1,     3,     2,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     0,     4,     1,     3,     0,     4,
       1,     1,     3,     3,     2,     0,     4,     1,     0,     4,
       1,     3,     3,     3,     3,     3,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       2,     0
};


#define yyerrok         (yyerrstatus = 0)
#define yyclearin       (yychar = YYEMPTY)
#define YYEMPTY         (-2)
#define YYEOF           0

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab


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
      YYERROR;                                                  \
    }                                                           \
while (0)

/* Error token number */
#define YYTERROR        1
#define YYERRCODE       256



/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)                        \
do {                                            \
  if (yydebug)                                  \
    YYFPRINTF Args;                             \
} while (0)

/* This macro is provided for backward compatibility. */
#ifndef YY_LOCATION_PRINT
# define YY_LOCATION_PRINT(File, Loc) ((void) 0)
#endif


# define YY_SYMBOL_PRINT(Title, Type, Value, Location)                    \
do {                                                                      \
  if (yydebug)                                                            \
    {                                                                     \
      YYFPRINTF (stderr, "%s ", Title);                                   \
      yy_symbol_print (stderr,                                            \
                  Type, Value); \
      YYFPRINTF (stderr, "\n");                                           \
    }                                                                     \
} while (0)


/*----------------------------------------.
| Print this symbol's value on YYOUTPUT.  |
`----------------------------------------*/

static void
yy_symbol_value_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
{
  FILE *yyo = yyoutput;
  YYUSE (yyo);
  if (!yyvaluep)
    return;
# ifdef YYPRINT
  if (yytype < YYNTOKENS)
    YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# endif
  YYUSE (yytype);
}


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

static void
yy_symbol_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
{
  YYFPRINTF (yyoutput, "%s %s (",
             yytype < YYNTOKENS ? "token" : "nterm", yytname[yytype]);

  yy_symbol_value_print (yyoutput, yytype, yyvaluep);
  YYFPRINTF (yyoutput, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

static void
yy_stack_print (yytype_int16 *yybottom, yytype_int16 *yytop)
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)                            \
do {                                                            \
  if (yydebug)                                                  \
    yy_stack_print ((Bottom), (Top));                           \
} while (0)


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

static void
yy_reduce_print (yytype_int16 *yyssp, YYSTYPE *yyvsp, int yyrule)
{
  unsigned long int yylno = yyrline[yyrule];
  int yynrhs = yyr2[yyrule];
  int yyi;
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %lu):\n",
             yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr,
                       yystos[yyssp[yyi + 1 - yynrhs]],
                       &(yyvsp[(yyi + 1) - (yynrhs)])
                                              );
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)          \
do {                                    \
  if (yydebug)                          \
    yy_reduce_print (yyssp, yyvsp, Rule); \
} while (0)

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
#ifndef YYINITDEPTH
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
static YYSIZE_T
yystrlen (const char *yystr)
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
static char *
yystpcpy (char *yydest, const char *yysrc)
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
  YYSIZE_T yysize0 = yytnamerr (YY_NULLPTR, yytname[yytoken]);
  YYSIZE_T yysize = yysize0;
  enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
  /* Internationalized format string. */
  const char *yyformat = YY_NULLPTR;
  /* Arguments of yyformat. */
  char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
  /* Number of reported tokens (one for the "unexpected", one per
     "expected"). */
  int yycount = 0;

  /* There are many possibilities here to consider:
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
                  YYSIZE_T yysize1 = yysize + yytnamerr (YY_NULLPTR, yytname[yyx]);
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

static void
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep)
{
  YYUSE (yyvaluep);
  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YYUSE (yytype);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}




/* The lookahead symbol.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;
/* Number of syntax errors so far.  */
int yynerrs;


/*----------.
| yyparse.  |
`----------*/

int
yyparse (void)
{
    int yystate;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus;

    /* The stacks and their tools:
       'yyss': related to states.
       'yyvs': related to semantic values.

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
      yychar = yylex ();
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
     '$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
        case 9:
#line 241 "parser.ypp" /* yacc.c:1646  */
    {

	}
#line 1557 "parser.cpp" /* yacc.c:1646  */
    break;

  case 11:
#line 249 "parser.ypp" /* yacc.c:1646  */
    {
		parsed_value->clear();
	}
#line 1565 "parser.cpp" /* yacc.c:1646  */
    break;

  case 12:
#line 253 "parser.ypp" /* yacc.c:1646  */
    {
		parsed_value->assign((yyvsp[0].str));
		if(!type_stack.empty()) depth_error(0, "type");
	}
#line 1574 "parser.cpp" /* yacc.c:1646  */
    break;

  case 13:
#line 258 "parser.ypp" /* yacc.c:1646  */
    {
		parsed_value->assign((yyvsp[0].str));
		if(!type_stack.empty()) depth_error(0, "method");
	}
#line 1583 "parser.cpp" /* yacc.c:1646  */
    break;

  case 14:
#line 266 "parser.ypp" /* yacc.c:1646  */
    {
		Import* import = new Import((yyvsp[0].str));
		parsed_file->add_import(import);
	}
#line 1592 "parser.cpp" /* yacc.c:1646  */
    break;

  case 15:
#line 271 "parser.ypp" /* yacc.c:1646  */
    {
		Import* import = new Import((yyvsp[-2].str));
		import->symbols.assign((yyvsp[0].strings).begin(), (yyvsp[0].strings).end());
		parsed_file->add_import(import);
	}
#line 1602 "parser.cpp" /* yacc.c:1646  */
    break;

  case 16:
#line 279 "parser.ypp" /* yacc.c:1646  */
    { (yyval.str) = (yyvsp[0].str); }
#line 1608 "parser.cpp" /* yacc.c:1646  */
    break;

  case 17:
#line 281 "parser.ypp" /* yacc.c:1646  */
    {
		(yyval.str) = (yyvsp[-2].str) + string(".") + (yyvsp[0].str);
	}
#line 1616 "parser.cpp" /* yacc.c:1646  */
    break;

  case 18:
#line 287 "parser.ypp" /* yacc.c:1646  */
    { (yyval.strings) = (yyvsp[0].strings); }
#line 1622 "parser.cpp" /* yacc.c:1646  */
    break;

  case 19:
#line 289 "parser.ypp" /* yacc.c:1646  */
    {
		(yyval.strings) = vector<string>();
	}
#line 1630 "parser.cpp" /* yacc.c:1646  */
    break;

  case 20:
#line 296 "parser.ypp" /* yacc.c:1646  */
    {
		(yyval.strings) = vector<string>(1, (yyvsp[0].str));
	}
#line 1638 "parser.cpp" /* yacc.c:1646  */
    break;

  case 21:
#line 300 "parser.ypp" /* yacc.c:1646  */
    {
		(yyvsp[-2].strings).push_back((yyvsp[0].str));
		(yyval.strings) = (yyvsp[-2].strings);
	}
#line 1647 "parser.cpp" /* yacc.c:1646  */
    break;

  case 22:
#line 307 "parser.ypp" /* yacc.c:1646  */
    { (yyval.str) = (yyvsp[0].str); }
#line 1653 "parser.cpp" /* yacc.c:1646  */
    break;

  case 23:
#line 309 "parser.ypp" /* yacc.c:1646  */
    {
		(yyval.str) = (yyvsp[-2].str) + string("/") + (yyvsp[0].str);
	}
#line 1661 "parser.cpp" /* yacc.c:1646  */
    break;

  case 24:
#line 316 "parser.ypp" /* yacc.c:1646  */
    {
		if((yyvsp[0].nametype).type == (DistributedType*)NULL)
		{
			// Ignore this typedef, it should have already produced an error
			break;
		}

		// Set the type's typedef
		(yyvsp[0].nametype).type->set_alias((yyvsp[0].nametype).name);

		bool type_added	= parsed_file->add_typedef((yyvsp[0].nametype).name, (yyvsp[0].nametype).type);
		if(!type_added)
		{
			// Lets be really descriptive about why this failed
			DistributedType* dtype = parsed_file->get_type_by_name((yyvsp[0].nametype).name);
			if(dtype == (DistributedType*)NULL)
			{
				parser_error("Unknown error adding typedef to file.");
				break;
			}

			Struct* dstruct = dtype->as_struct();
			if(dstruct == (Struct*)NULL)
			{
				parser_error("Cannot add 'typedef " + (yyvsp[0].nametype).name
				             + "' to file because a typedef was already declared with that name.");
				break;
			}

			if(dstruct->as_class())
			{
				parser_error("Cannot add 'typedef " + (yyvsp[0].nametype).name
				             + "' to file because a class was already declared with that name.");
			}
			else
			{
				parser_error("Cannot add 'typedef " + (yyvsp[0].nametype).name
				             + "' to file because a struct was already declared with that name.");
			}
		}
	}
#line 1707 "parser.cpp" /* yacc.c:1646  */
    break;

  case 25:
#line 361 "parser.ypp" /* yacc.c:1646  */
    {
		(yyval.nametype) = (yyvsp[0].nametype);
	}
#line 1715 "parser.cpp" /* yacc.c:1646  */
    break;

  case 26:
#line 365 "parser.ypp" /* yacc.c:1646  */
    {
		(yyval.nametype) = (yyvsp[-3].nametype);
		(yyval.nametype).type = new ArrayType((yyvsp[-3].nametype).type, (yyvsp[-1].range));
	}
#line 1724 "parser.cpp" /* yacc.c:1646  */
    break;

  case 27:
#line 373 "parser.ypp" /* yacc.c:1646  */
    {
		TokenType::NameType nt;
		nt.type = (yyvsp[-1].u.dtype);
		nt.name = (yyvsp[0].str);
		(yyval.nametype) = nt;
	}
#line 1735 "parser.cpp" /* yacc.c:1646  */
    break;

  case 28:
#line 383 "parser.ypp" /* yacc.c:1646  */
    {
		DistributedType* dtype = parsed_file->get_type_by_name((yyvsp[0].str));
		if(dtype == (DistributedType*)NULL)
		{
			parser_error("Type '" + string((yyvsp[0].str)) + "' has not been declared.");
			(yyval.u.dtype) = NULL;
			break;
		}

		(yyval.u.dtype) = dtype;
	}
#line 1751 "parser.cpp" /* yacc.c:1646  */
    break;

  case 31:
#line 403 "parser.ypp" /* yacc.c:1646  */
    {
		parsed_file->add_keyword((yyvsp[0].str));
	}
#line 1759 "parser.cpp" /* yacc.c:1646  */
    break;

  case 32:
#line 410 "parser.ypp" /* yacc.c:1646  */
    {
		current_class = new Class(parsed_file, (yyvsp[0].str));
	}
#line 1767 "parser.cpp" /* yacc.c:1646  */
    break;

  case 33:
#line 414 "parser.ypp" /* yacc.c:1646  */
    {
		bool class_added = parsed_file->add_class(current_class);
		if(!class_added)
		{
			// Lets be really descriptive about why this failed
			DistributedType* dtype = parsed_file->get_type_by_name(current_class->get_name());
			if(dtype == (DistributedType*)NULL)
			{
				parser_error("Unknown error adding class to file.");
				break;
			}

			Struct* dstruct = dtype->as_struct();
			if(dstruct == (Struct*)NULL)
			{
				parser_error("Cannot add 'dclass " + current_class->get_name()
				             + "' to file because a typedef was already declared with that name.");
				break;
			}

			if(dstruct->as_class())
			{
				parser_error("Cannot add 'dclass " + current_class->get_name()
				             + "' to file because a class was already declared with that name.");
			}
			else
			{
				parser_error("Cannot add 'dclass " + current_class->get_name()
				             + "' to file because a struct was already declared with that name.");
			}
		}
	}
#line 1804 "parser.cpp" /* yacc.c:1646  */
    break;

  case 36:
#line 455 "parser.ypp" /* yacc.c:1646  */
    {
		if((yyvsp[0].u.dclass) != (Class*)NULL)
		{
			current_class->add_parent((yyvsp[0].u.dclass));
		}
	}
#line 1815 "parser.cpp" /* yacc.c:1646  */
    break;

  case 37:
#line 462 "parser.ypp" /* yacc.c:1646  */
    {
		if((yyvsp[0].u.dclass) != (Class*)NULL)
		{
			current_class->add_parent((yyvsp[0].u.dclass));
		}
	}
#line 1826 "parser.cpp" /* yacc.c:1646  */
    break;

  case 38:
#line 472 "parser.ypp" /* yacc.c:1646  */
    {
		DistributedType* dtype = parsed_file->get_type_by_name((yyvsp[0].str));
		if(dtype == (DistributedType*)NULL)
		{
			parser_error("'dclass " + string((yyvsp[0].str)) + "' has not been declared.");
			(yyval.u.dclass) = NULL;
			break;
		}

		Struct* dstruct = dtype->as_struct();
		if(dstruct == (Struct*)NULL)
		{
			parser_error("class cannot inherit from non-class type '" + string((yyvsp[0].str)) + "'.");
			(yyval.u.dclass) = NULL;
			break;
		}

		Class* dclass = dstruct->as_class();
		if(dclass == (Class*)NULL)
		{
			parser_error("class cannot inherit from struct type '" + string((yyvsp[0].str)) + "'.");
			(yyval.u.dclass) = NULL;
			break;
		}

		(yyval.u.dclass) = dclass;
	}
#line 1858 "parser.cpp" /* yacc.c:1646  */
    break;

  case 41:
#line 505 "parser.ypp" /* yacc.c:1646  */
    {
		if((yyvsp[-1].u.dfield) == (Field*)NULL)
		{
			// Ignore this field, it should have already generated a parser error
			break;
		}

		bool field_added = current_class->add_field((yyvsp[-1].u.dfield));
		if(!field_added)
		{
			// Lets be really descriptive about why this failed
			if(current_class->get_field_by_name((yyvsp[-1].u.dfield)->get_name()))
			{
				parser_error("Cannot add field '" + (yyvsp[-1].u.dfield)->get_name()
				             + "', a field with that name already exists in 'dclass "
				             + current_class->get_name() + "'.");
			}
			else if(current_class->get_name() == (yyvsp[-1].u.dfield)->get_name())
			{
				if((yyvsp[-1].u.dfield)->as_molecular())
				{
					parser_error("Cannot use a molecular field as a constructor.");
				}
				else
				{
					parser_error("The constructor must be the first field in the class.");
				}
			}
			else
			{
				parser_error("Unknown error adding field to class.");
			}
		}
	}
#line 1897 "parser.cpp" /* yacc.c:1646  */
    break;

  case 42:
#line 543 "parser.ypp" /* yacc.c:1646  */
    {
		if((yyvsp[-1].u.dfield) == (Field*)NULL)
		{
			// Ignore this field, it should have already generated a parser error
			(yyval.u.dfield) = NULL;
			break;
		}

		if((yyvsp[-1].u.dfield)->get_name().empty())
		{
			parser_error("An unnamed field can't be defined in a class.");
			(yyval.u.dfield) = NULL;
			break;
		}

		// Add the keywords to the class
		for(auto it = (yyvsp[0].strings).begin(); it != (yyvsp[0].strings).end(); ++it)
		{
			(yyvsp[-1].u.dfield)->add_keyword(*it);
		}

		(yyval.u.dfield) = (yyvsp[-1].u.dfield);
	}
#line 1925 "parser.cpp" /* yacc.c:1646  */
    break;

  case 43:
#line 567 "parser.ypp" /* yacc.c:1646  */
    {
		(yyval.u.dfield) = (Field*)(yyvsp[0].u.dmolecule);
	}
#line 1933 "parser.cpp" /* yacc.c:1646  */
    break;

  case 44:
#line 574 "parser.ypp" /* yacc.c:1646  */
    {
		current_struct = new Struct(parsed_file, (yyvsp[0].str));
	}
#line 1941 "parser.cpp" /* yacc.c:1646  */
    break;

  case 45:
#line 578 "parser.ypp" /* yacc.c:1646  */
    {
		bool struct_added = parsed_file->add_struct(current_struct);
		if(!struct_added)
		{
			// Lets be really descriptive about why this failed
			DistributedType* dtype = parsed_file->get_type_by_name(current_struct->get_name());
			if(dtype == (DistributedType*)NULL)
			{
				parser_error("Unknown error adding struct to file.");
				break;
			}

			Struct* dstruct = dtype->as_struct();
			if(dstruct == (Struct*)NULL)
			{
				parser_error("Cannot add 'struct " + current_struct->get_name()
				             + "' to file because a typedef was already declared with that name.");
				break;
			}

			if(dstruct->as_class())
			{
				parser_error("Cannot add 'struct " + current_struct->get_name()
				             + "' to file because a class was already declared with that name.");
			}
			else
			{
				parser_error("Cannot add 'struct " + current_struct->get_name()
				             + "' to file because a struct was already declared with that name.");
			}
		}
	}
#line 1978 "parser.cpp" /* yacc.c:1646  */
    break;

  case 48:
#line 616 "parser.ypp" /* yacc.c:1646  */
    {
		if((yyvsp[-1].u.dfield) == (Field*)NULL || (yyvsp[-1].u.dfield)->get_type() == (DistributedType*)NULL)
		{
			// Ignore this field, it should have already generated a parser error
			break;
		}

		if(!current_struct->add_field((yyvsp[-1].u.dfield)))
		{
			// Lets be really descriptive about why this failed
			if(current_struct->get_field_by_name((yyvsp[-1].u.dfield)->get_name()))
			{
				parser_error("Cannot add field '" + (yyvsp[-1].u.dfield)->get_name()
				             + "', a field with that name already exists in 'struct "
				             + current_struct->get_name() + "'.");
			}
			else if(current_struct->get_name() == (yyvsp[-1].u.dfield)->get_name())
			{
				parser_error("A constructor can't be defined in a struct.");
			}
			else if((yyvsp[-1].u.dfield)->get_type()->as_method())
			{
				parser_error("A method can't be defined in a struct.");
			}
			else
			{
				parser_error("Unknown error adding field to struct.");
			}
		}
	}
#line 2013 "parser.cpp" /* yacc.c:1646  */
    break;

  case 55:
#line 662 "parser.ypp" /* yacc.c:1646  */
    {
		(yyval.u.dfield) = new Field((yyvsp[0].u.dtype));
	}
#line 2021 "parser.cpp" /* yacc.c:1646  */
    break;

  case 56:
#line 666 "parser.ypp" /* yacc.c:1646  */
    {
		current_depth = 0;
		type_stack.push(TypeAndDepth((yyvsp[-1].u.dtype), 0));
	}
#line 2030 "parser.cpp" /* yacc.c:1646  */
    break;

  case 57:
#line 671 "parser.ypp" /* yacc.c:1646  */
    {
		Field* field = new Field((yyvsp[-3].u.dtype));
		if(!type_stack.empty()) depth_error(0, "unnamed field");
		field->set_default_value((yyvsp[0].str));
		(yyval.u.dfield) = field;
	}
#line 2041 "parser.cpp" /* yacc.c:1646  */
    break;

  case 58:
#line 681 "parser.ypp" /* yacc.c:1646  */
    {
		(yyval.u.dfield) = new Field((yyvsp[0].nametype).type, (yyvsp[0].nametype).name);
	}
#line 2049 "parser.cpp" /* yacc.c:1646  */
    break;

  case 59:
#line 688 "parser.ypp" /* yacc.c:1646  */
    {
		(yyvsp[-3].u.dfield)->set_type(new ArrayType((yyvsp[-3].u.dfield)->get_type(), (yyvsp[-1].range)));
		(yyval.u.dfield) = (yyvsp[-3].u.dfield);
	}
#line 2058 "parser.cpp" /* yacc.c:1646  */
    break;

  case 60:
#line 693 "parser.ypp" /* yacc.c:1646  */
    {
		(yyvsp[-3].u.dfield)->set_type(new ArrayType((yyvsp[-3].u.dfield)->get_type(), (yyvsp[-1].range)));
		(yyval.u.dfield) = (yyvsp[-3].u.dfield);
	}
#line 2067 "parser.cpp" /* yacc.c:1646  */
    break;

  case 61:
#line 701 "parser.ypp" /* yacc.c:1646  */
    {
		current_depth = 0;
		type_stack.push(TypeAndDepth((yyvsp[-1].u.dfield)->get_type(), 0));
	}
#line 2076 "parser.cpp" /* yacc.c:1646  */
    break;

  case 62:
#line 706 "parser.ypp" /* yacc.c:1646  */
    {
		if(!type_stack.empty()) depth_error(0, "field '" + (yyvsp[-3].u.dfield)->get_name() + "'");
		(yyvsp[-3].u.dfield)->set_default_value((yyvsp[0].str));
		(yyval.u.dfield) = (yyvsp[-3].u.dfield);
	}
#line 2086 "parser.cpp" /* yacc.c:1646  */
    break;

  case 63:
#line 712 "parser.ypp" /* yacc.c:1646  */
    {
		current_depth = 0;
		type_stack.push(TypeAndDepth((yyvsp[-1].u.dfield)->get_type(), 0));
	}
#line 2095 "parser.cpp" /* yacc.c:1646  */
    break;

  case 64:
#line 717 "parser.ypp" /* yacc.c:1646  */
    {
		if(!type_stack.empty()) depth_error(0, "field '" + (yyvsp[-3].u.dfield)->get_name() + "'");
		(yyvsp[-3].u.dfield)->set_default_value((yyvsp[0].str));
		(yyval.u.dfield) = (yyvsp[-3].u.dfield);
	}
#line 2105 "parser.cpp" /* yacc.c:1646  */
    break;

  case 65:
#line 723 "parser.ypp" /* yacc.c:1646  */
    {
		current_depth = 0;
		type_stack.push(TypeAndDepth((yyvsp[-2].u.dfield)->get_type(), 0));
	}
#line 2114 "parser.cpp" /* yacc.c:1646  */
    break;

  case 66:
#line 728 "parser.ypp" /* yacc.c:1646  */
    {
		if(!type_stack.empty()) depth_error(0, "method");
		(yyvsp[-4].u.dfield)->set_default_value((yyvsp[-2].str));
		(yyval.u.dfield) = (yyvsp[-4].u.dfield);
	}
#line 2124 "parser.cpp" /* yacc.c:1646  */
    break;

  case 67:
#line 737 "parser.ypp" /* yacc.c:1646  */
    {
		(yyval.u.dfield) = new Field((yyvsp[0].u.dmethod), (yyvsp[-1].str));
	}
#line 2132 "parser.cpp" /* yacc.c:1646  */
    break;

  case 70:
#line 749 "parser.ypp" /* yacc.c:1646  */
    {
		if((yyvsp[0].u.dtype) == (DistributedType*)NULL)
		{
			// defined_type should have output an error, pass NULL upstream
			(yyval.u.dtype) = NULL;
			break;
		}

		if((yyvsp[0].u.dtype)->get_type() == T_METHOD)
		{
			parser_error("Cannot use a method type here.");
			(yyval.u.dtype) = NULL;
			break;
		}

		(yyval.u.dtype) = (yyvsp[0].u.dtype);
	}
#line 2154 "parser.cpp" /* yacc.c:1646  */
    break;

  case 71:
#line 767 "parser.ypp" /* yacc.c:1646  */
    {
		(yyval.u.dtype) = (DistributedType*)(yyvsp[0].u.dnumeric);
	}
#line 2162 "parser.cpp" /* yacc.c:1646  */
    break;

  case 73:
#line 775 "parser.ypp" /* yacc.c:1646  */
    {
		(yyval.u.dtype) = new ArrayType((yyvsp[-3].u.dnumeric), (yyvsp[-1].range));
	}
#line 2170 "parser.cpp" /* yacc.c:1646  */
    break;

  case 74:
#line 779 "parser.ypp" /* yacc.c:1646  */
    {
		(yyval.u.dtype) = new ArrayType((yyvsp[-3].u.dtype), (yyvsp[-1].range));
	}
#line 2178 "parser.cpp" /* yacc.c:1646  */
    break;

  case 75:
#line 783 "parser.ypp" /* yacc.c:1646  */
    {
		(yyval.u.dtype) = new ArrayType((yyvsp[-3].u.dtype), (yyvsp[-1].range));
	}
#line 2186 "parser.cpp" /* yacc.c:1646  */
    break;

  case 76:
#line 787 "parser.ypp" /* yacc.c:1646  */
    {
		(yyval.u.dtype) = new ArrayType((yyvsp[-3].u.dtype), (yyvsp[-1].range));
	}
#line 2194 "parser.cpp" /* yacc.c:1646  */
    break;

  case 77:
#line 794 "parser.ypp" /* yacc.c:1646  */
    {
		MolecularField* mol = new MolecularField(current_class, (yyvsp[-2].str));
		if((yyvsp[0].u.dfield) == (Field*)NULL)
		{
			// Ignore this field, it should have already generated an error
			(yyval.u.dmolecule) = mol;
			break;
		}

		bool field_added = mol->add_field((yyvsp[0].u.dfield));
		if(!field_added)
		{
			if((yyvsp[0].u.dfield)->as_molecular())
			{
				parser_error("Cannot add molecular '" + (yyvsp[0].u.dfield)->get_name() + "' to a molecular field.");
			}
			else
			{
				parser_error("Unkown error adding field " + (yyvsp[0].u.dfield)->get_name() + " to molecular '"
				             + (yyvsp[-2].str) + "'.");
			}
		}

		(yyval.u.dmolecule) = mol;
	}
#line 2224 "parser.cpp" /* yacc.c:1646  */
    break;

  case 78:
#line 820 "parser.ypp" /* yacc.c:1646  */
    {
		if((yyvsp[0].u.dfield) == (Field*)NULL)
		{
			// Ignore this field, it should have already generated an error
			(yyval.u.dmolecule) = (yyvsp[-2].u.dmolecule);
			break;
		}

		bool field_added = (yyvsp[-2].u.dmolecule)->add_field((yyvsp[0].u.dfield));
		if(!field_added)
		{
			if((yyvsp[0].u.dfield)->as_molecular())
			{
				parser_error("Cannot add molecular '" + (yyvsp[0].u.dfield)->get_name() + "' to a molecular field.");
			}
			else if(!(yyvsp[-2].u.dmolecule)->has_matching_keywords(*(yyvsp[0].u.dfield)))
			{
				parser_error("Mismatched keywords in molecular between " +
					(yyvsp[-2].u.dmolecule)->get_field(0)->get_name() + " and " + (yyvsp[0].u.dfield)->get_name() + ".");
			}
			else
			{
				parser_error("Unkown error adding field " + (yyvsp[0].u.dfield)->get_name() + " to molecular '"
				             + (yyvsp[-2].u.dmolecule)->get_name() + "'.");
			}
		}

		(yyval.u.dmolecule) = (yyvsp[-2].u.dmolecule);
	}
#line 2258 "parser.cpp" /* yacc.c:1646  */
    break;

  case 79:
#line 852 "parser.ypp" /* yacc.c:1646  */
    {
		if(!current_class)
		{
			parser_error("Field '" + (yyvsp[0].str) + "' not defined in current class.");
			(yyval.u.dfield) = NULL;
			break;
		}

		Field *field = current_class->get_field_by_name((yyvsp[0].str));
		if(field == (Field*)NULL)
		{
			parser_error("Field '" + (yyvsp[0].str) + "' not defined in current class.");
			(yyval.u.dfield) = NULL;
			break;
		}

		(yyval.u.dfield) = field;
	}
#line 2281 "parser.cpp" /* yacc.c:1646  */
    break;

  case 80:
#line 874 "parser.ypp" /* yacc.c:1646  */
    {
		if((yyvsp[0].u.type) == T_STRING)
		{
			if(basic_string == NULL)
			{
				basic_string = new ArrayType(new NumericType(T_CHAR));
				basic_string->set_alias("string");
			}

			(yyval.u.dtype) = basic_string;
		}
		else if((yyvsp[0].u.type) == T_BLOB)
		{
			if(basic_blob == NULL)
			{
				basic_blob = new ArrayType(new NumericType(T_UINT8));
				basic_blob->set_alias("blob");
			}

			(yyval.u.dtype) = basic_blob;
		}
		else
		{
			parser_error("Found builtin ArrayType not handled by parser.");
			(yyval.u.dtype) = NULL;
		}
	}
#line 2313 "parser.cpp" /* yacc.c:1646  */
    break;

  case 81:
#line 902 "parser.ypp" /* yacc.c:1646  */
    {
		if((yyvsp[-3].u.type) == T_STRING)
		{
			ArrayType* arr = new ArrayType(new NumericType(T_CHAR), (yyvsp[-1].range));
			arr->set_alias("string");
			(yyval.u.dtype) = arr;
		}
		else if((yyvsp[-3].u.type) == T_BLOB)
		{
			ArrayType* arr = new ArrayType(new NumericType(T_UINT8), (yyvsp[-1].range));
			arr->set_alias("blob");
			(yyval.u.dtype) = arr;
		}
		else
		{
			parser_error("Found builtin ArrayType not handled by parser.");
			(yyval.u.dtype) = NULL;
		}
	}
#line 2337 "parser.cpp" /* yacc.c:1646  */
    break;

  case 86:
#line 931 "parser.ypp" /* yacc.c:1646  */
    { (yyval.u.dnumeric) = new NumericType((yyvsp[0].u.type)); }
#line 2343 "parser.cpp" /* yacc.c:1646  */
    break;

  case 87:
#line 936 "parser.ypp" /* yacc.c:1646  */
    {
		if(!(yyvsp[-3].u.dnumeric)->set_range((yyvsp[-1].range)))
		{
			parser_error("Invalid range for type.");
		}

		(yyval.u.dnumeric) = (yyvsp[-3].u.dnumeric);
	}
#line 2356 "parser.cpp" /* yacc.c:1646  */
    break;

  case 88:
#line 945 "parser.ypp" /* yacc.c:1646  */
    {
		if(!(yyvsp[-3].u.dnumeric)->set_range((yyvsp[-1].range)))
		{
			parser_error("Invalid range for type.");
		}

		(yyval.u.dnumeric) = (yyvsp[-3].u.dnumeric);
	}
#line 2369 "parser.cpp" /* yacc.c:1646  */
    break;

  case 89:
#line 954 "parser.ypp" /* yacc.c:1646  */
    {
		if(!(yyvsp[-3].u.dnumeric)->set_range((yyvsp[-1].range)))
		{
			parser_error("Invalid range for type.");
		}

		(yyval.u.dnumeric) = (yyvsp[-3].u.dnumeric);
	}
#line 2382 "parser.cpp" /* yacc.c:1646  */
    break;

  case 90:
#line 966 "parser.ypp" /* yacc.c:1646  */
    {
		if(!(yyvsp[-2].u.dnumeric)->set_modulus((yyvsp[0].u.real)))
		{
			parser_error("Invalid modulus for type.");
		}

		(yyval.u.dnumeric) = (yyvsp[-2].u.dnumeric);
	}
#line 2395 "parser.cpp" /* yacc.c:1646  */
    break;

  case 91:
#line 978 "parser.ypp" /* yacc.c:1646  */
    {
		if(!(yyvsp[-2].u.dnumeric)->set_divisor((yyvsp[0].u.uint32)))
		{
			parser_error("Invalid divisor for type.");
		}
	}
#line 2406 "parser.cpp" /* yacc.c:1646  */
    break;

  case 92:
#line 985 "parser.ypp" /* yacc.c:1646  */
    {
		if(!(yyvsp[-2].u.dnumeric)->set_divisor((yyvsp[0].u.uint32)))
		{
			parser_error("Invalid divisor for type.");
		}
	}
#line 2417 "parser.cpp" /* yacc.c:1646  */
    break;

  case 93:
#line 995 "parser.ypp" /* yacc.c:1646  */
    {
		(yyval.u.dmethod) = new Method();
	}
#line 2425 "parser.cpp" /* yacc.c:1646  */
    break;

  case 94:
#line 999 "parser.ypp" /* yacc.c:1646  */
    {
		(yyval.u.dmethod) = (yyvsp[-1].u.dmethod);
	}
#line 2433 "parser.cpp" /* yacc.c:1646  */
    break;

  case 95:
#line 1006 "parser.ypp" /* yacc.c:1646  */
    {
		Method* fn = new Method();
		bool param_added = fn->add_parameter((yyvsp[0].u.dparam));
		if(!param_added)
		{
			parser_error("Unknown error adding parameter to method.");
		}
		(yyval.u.dmethod) = fn;
	}
#line 2447 "parser.cpp" /* yacc.c:1646  */
    break;

  case 96:
#line 1016 "parser.ypp" /* yacc.c:1646  */
    {
		bool param_added = (yyvsp[-2].u.dmethod)->add_parameter((yyvsp[0].u.dparam));
		if(!param_added)
		{
			parser_error("Cannot add parameter '" + (yyvsp[0].u.dparam)->get_name()
			             + "', a parameter with that name is already used in this method.");
		}
		(yyval.u.dmethod) = (yyvsp[-2].u.dmethod);
	}
#line 2461 "parser.cpp" /* yacc.c:1646  */
    break;

  case 100:
#line 1032 "parser.ypp" /* yacc.c:1646  */
    {
		(yyval.u.dparam) = new Parameter((yyvsp[0].u.dtype));
	}
#line 2469 "parser.cpp" /* yacc.c:1646  */
    break;

  case 101:
#line 1036 "parser.ypp" /* yacc.c:1646  */
    {
		current_depth = 0;
		type_stack.push(TypeAndDepth((yyvsp[-1].u.dtype),0));
	}
#line 2478 "parser.cpp" /* yacc.c:1646  */
    break;

  case 102:
#line 1041 "parser.ypp" /* yacc.c:1646  */
    {
		Parameter* param = new Parameter((yyvsp[-3].u.dtype));
		if(!type_stack.empty()) depth_error(0, "type");
		param->set_default_value((yyvsp[0].str));
		(yyval.u.dparam) = param;
	}
#line 2489 "parser.cpp" /* yacc.c:1646  */
    break;

  case 103:
#line 1050 "parser.ypp" /* yacc.c:1646  */
    {
		(yyval.u.dparam) = new Parameter((yyvsp[-1].u.dtype), (yyvsp[0].str));
	}
#line 2497 "parser.cpp" /* yacc.c:1646  */
    break;

  case 104:
#line 1057 "parser.ypp" /* yacc.c:1646  */
    {
		(yyvsp[-3].u.dparam)->set_type(new ArrayType((yyvsp[-3].u.dparam)->get_type(), (yyvsp[-1].range)));
		(yyval.u.dparam) = (yyvsp[-3].u.dparam);
	}
#line 2506 "parser.cpp" /* yacc.c:1646  */
    break;

  case 105:
#line 1062 "parser.ypp" /* yacc.c:1646  */
    {
		(yyvsp[-3].u.dparam)->set_type(new ArrayType((yyvsp[-3].u.dparam)->get_type(), (yyvsp[-1].range)));
		(yyval.u.dparam) = (yyvsp[-3].u.dparam);
	}
#line 2515 "parser.cpp" /* yacc.c:1646  */
    break;

  case 106:
#line 1070 "parser.ypp" /* yacc.c:1646  */
    {
		current_depth = 0;
		type_stack.push(TypeAndDepth((yyvsp[-1].u.dparam)->get_type(), 0));
	}
#line 2524 "parser.cpp" /* yacc.c:1646  */
    break;

  case 107:
#line 1075 "parser.ypp" /* yacc.c:1646  */
    {
		if(!type_stack.empty()) depth_error(0, "parameter '" + (yyvsp[-3].u.dparam)->get_name() + "'");
		(yyvsp[-3].u.dparam)->set_default_value((yyvsp[0].str));
		(yyval.u.dparam) = (yyvsp[-3].u.dparam);
	}
#line 2534 "parser.cpp" /* yacc.c:1646  */
    break;

  case 108:
#line 1081 "parser.ypp" /* yacc.c:1646  */
    {
		current_depth = 0;
		type_stack.push(TypeAndDepth((yyvsp[-1].u.dparam)->get_type(), 0));
	}
#line 2543 "parser.cpp" /* yacc.c:1646  */
    break;

  case 109:
#line 1086 "parser.ypp" /* yacc.c:1646  */
    {
		if(!type_stack.empty()) depth_error(0, "parameter '" + (yyvsp[-3].u.dparam)->get_name() + "'");
		(yyvsp[-3].u.dparam)->set_default_value((yyvsp[0].str));
		(yyval.u.dparam) = (yyvsp[-3].u.dparam);
	}
#line 2553 "parser.cpp" /* yacc.c:1646  */
    break;

  case 110:
#line 1094 "parser.ypp" /* yacc.c:1646  */
    { (yyval.range) = NumericRange(); }
#line 2559 "parser.cpp" /* yacc.c:1646  */
    break;

  case 111:
#line 1095 "parser.ypp" /* yacc.c:1646  */
    { (yyval.range) = NumericRange((yyvsp[0].u.real), (yyvsp[0].u.real)); }
#line 2565 "parser.cpp" /* yacc.c:1646  */
    break;

  case 112:
#line 1096 "parser.ypp" /* yacc.c:1646  */
    { (yyval.range) = NumericRange((yyvsp[-2].u.real), (yyvsp[0].u.real)); }
#line 2571 "parser.cpp" /* yacc.c:1646  */
    break;

  case 113:
#line 1097 "parser.ypp" /* yacc.c:1646  */
    { (yyval.range) = NumericRange((yyvsp[-1].u.real), (yyvsp[0].u.real)); }
#line 2577 "parser.cpp" /* yacc.c:1646  */
    break;

  case 114:
#line 1101 "parser.ypp" /* yacc.c:1646  */
    { (yyval.range) = NumericRange(); }
#line 2583 "parser.cpp" /* yacc.c:1646  */
    break;

  case 115:
#line 1102 "parser.ypp" /* yacc.c:1646  */
    { (yyval.range) = NumericRange((yyvsp[0].u.uint32), (yyvsp[0].u.uint32)); }
#line 2589 "parser.cpp" /* yacc.c:1646  */
    break;

  case 116:
#line 1103 "parser.ypp" /* yacc.c:1646  */
    { (yyval.range) = NumericRange((yyvsp[-2].u.uint32), (yyvsp[0].u.uint32)); }
#line 2595 "parser.cpp" /* yacc.c:1646  */
    break;

  case 117:
#line 1104 "parser.ypp" /* yacc.c:1646  */
    { (yyval.range) = NumericRange((yyvsp[-1].u.uint32), (yyvsp[0].u.uint32)); }
#line 2601 "parser.cpp" /* yacc.c:1646  */
    break;

  case 118:
#line 1109 "parser.ypp" /* yacc.c:1646  */
    {
		if((yyvsp[0].str).length() != 1)
		{
			parser_error("Single character required.");
			(yyval.u.uint32) = 0;
		}
		else
		{
			(yyval.u.uint32) = (unsigned char)(yyvsp[0].str)[0];
		}
	}
#line 2617 "parser.cpp" /* yacc.c:1646  */
    break;

  case 120:
#line 1125 "parser.ypp" /* yacc.c:1646  */
    {
		unsigned int num = (unsigned int)(yyvsp[0].u.uint64);
		if(num != (yyvsp[0].u.uint64))
		{
			parser_error("Number out of range.");
			(yyval.u.uint32) = 1;
		}
		(yyval.u.uint32) = num;
	}
#line 2631 "parser.cpp" /* yacc.c:1646  */
    break;

  case 121:
#line 1138 "parser.ypp" /* yacc.c:1646  */
    {
		int num = (unsigned int) -(yyvsp[0].u.int64);
		if((yyvsp[0].u.int64) >= 0)
		{
			parser_error("Syntax error while parsing small_negative_integer.");
		}
		else if(num != -(yyvsp[0].u.int64))
		{
			parser_error("Number out of range.");
			(yyval.u.uint32) = 1;
		}
		(yyval.u.uint32) = num;
	}
#line 2649 "parser.cpp" /* yacc.c:1646  */
    break;

  case 122:
#line 1154 "parser.ypp" /* yacc.c:1646  */
    { (yyval.u.real) = (double)(yyvsp[0].u.uint64); }
#line 2655 "parser.cpp" /* yacc.c:1646  */
    break;

  case 123:
#line 1155 "parser.ypp" /* yacc.c:1646  */
    { (yyval.u.real) = (double)(yyvsp[0].u.int64); }
#line 2661 "parser.cpp" /* yacc.c:1646  */
    break;

  case 125:
#line 1161 "parser.ypp" /* yacc.c:1646  */
    {
		if((yyvsp[0].str).length() != 1)
		{
			parser_error("Single character required.");
			(yyval.u.real) = 0;
		}
		else
		{
			(yyval.u.real) = (double)(unsigned char)(yyvsp[0].str)[0];
		}
	}
#line 2677 "parser.cpp" /* yacc.c:1646  */
    break;

  case 127:
#line 1177 "parser.ypp" /* yacc.c:1646  */
    {
		if(!check_depth()) depth_error("signed integer");

		const DistributedType* dtype = type_stack.top().type;
		type_stack.pop(); // Remove numeric type from stack

		(yyval.str) = number_value(dtype->get_type(), (yyvsp[0].u.int64));
	}
#line 2690 "parser.cpp" /* yacc.c:1646  */
    break;

  case 128:
#line 1186 "parser.ypp" /* yacc.c:1646  */
    {
		if(!check_depth()) depth_error("unsigned integer");

		const DistributedType* dtype = type_stack.top().type;
		type_stack.pop(); // Remove numeric type from stack

		(yyval.str) = number_value(dtype->get_type(), (yyvsp[0].u.uint64));
	}
#line 2703 "parser.cpp" /* yacc.c:1646  */
    break;

  case 129:
#line 1195 "parser.ypp" /* yacc.c:1646  */
    {
		if(!check_depth()) depth_error("floating point");

		const DistributedType* dtype = type_stack.top().type;
		type_stack.pop(); // Remove numeric type from stack

		(yyval.str) = number_value(dtype->get_type(), (yyvsp[0].u.real));
	}
#line 2716 "parser.cpp" /* yacc.c:1646  */
    break;

  case 130:
#line 1204 "parser.ypp" /* yacc.c:1646  */
    {
		if(!check_depth()) depth_error("string");

		const DistributedType* dtype = type_stack.top().type;
		type_stack.pop(); // Remove string type from stack

		if(dtype->get_type() == T_STRING || dtype->get_type() == T_BLOB)
		{
			if((yyvsp[0].str).length() != dtype->get_size())
			{
				parser_error("Value for fixed-length string has incorrect length.");
			}

			(yyval.str) = (yyvsp[0].str);
		}
		else if(dtype->get_type() == T_VARSTRING || dtype->get_type() == T_VARBLOB)
		{
			// TODO: Check for range limits
			// Prepend length tag
			sizetag_t length = (yyvsp[0].str).length();
			(yyval.str) = string((char*)&length, sizeof(sizetag_t)) + (yyvsp[0].str);
		}
		else
		{
			parser_error("Cannot use string value for non-string type '"
			             + format_type(dtype->get_type()) + "'.");
			(yyval.str) = (yyvsp[0].str);
		}
	}
#line 2750 "parser.cpp" /* yacc.c:1646  */
    break;

  case 131:
#line 1234 "parser.ypp" /* yacc.c:1646  */
    {
		if(!check_depth()) depth_error("hex-string");

		const DistributedType* dtype = type_stack.top().type;
		type_stack.pop(); // Remove type from stack

		if(dtype->get_type() == T_BLOB)
		{
			if((yyvsp[0].str).length() != dtype->get_size())
			{
				parser_error("Value for fixed-length blob has incorrect length.");
			}

			(yyval.str) = (yyvsp[0].str);
		}
		else if(dtype->get_type() == T_VARBLOB)
		{
			// TODO: Check for range limits
			// Prepend length tag
			sizetag_t length = (yyvsp[0].str).length();
			(yyval.str) = string((char*)&length, sizeof(sizetag_t)) + (yyvsp[0].str);
		}
		else
		{
			parser_error("Cannot use hex value for non-blob type '"
			             + format_type(dtype->get_type()) + "'.");
			(yyval.str) = (yyvsp[0].str);
		}
	}
#line 2784 "parser.cpp" /* yacc.c:1646  */
    break;

  case 134:
#line 1269 "parser.ypp" /* yacc.c:1646  */
    {
		if(!check_depth()) depth_error("method");

		const DistributedType* dtype = type_stack.top().type;
		if(dtype->as_method())
		{
			current_depth++;
			const Method* method = dtype->as_method();

			size_t num_params = method->get_num_parameters();
			for(unsigned int i = 1; i <= num_params; ++i)
			{
				// Reverse iteration
				const Parameter* param = method->get_parameter(num_params-i);
				// Add parameter types to stack such that the first is on top
				type_stack.push(TypeAndDepth(param->get_type(), current_depth));
			}
		}
		else
		{
			parser_error("Cannot use method-value for non-method type '"
			             + format_type(dtype->get_type()) + "'.");
		}
	}
#line 2813 "parser.cpp" /* yacc.c:1646  */
    break;

  case 135:
#line 1294 "parser.ypp" /* yacc.c:1646  */
    {
		if(type_stack.top().type->as_method())
		{
			current_depth--;
		}
		type_stack.pop(); // Remove method type from the stack
		(yyval.str) = (yyvsp[-1].str);
	}
#line 2826 "parser.cpp" /* yacc.c:1646  */
    break;

  case 137:
#line 1307 "parser.ypp" /* yacc.c:1646  */
    {
		(yyval.str) = (yyvsp[-2].str) + (yyvsp[0].str);
	}
#line 2834 "parser.cpp" /* yacc.c:1646  */
    break;

  case 138:
#line 1314 "parser.ypp" /* yacc.c:1646  */
    {
		if(!check_depth()) depth_error("struct");

		const DistributedType* dtype = type_stack.top().type;
		if(dtype->as_struct())
		{
			current_depth++;
			const Struct* dstruct = dtype->as_struct();

			size_t num_fields = dstruct->get_num_fields();
			for(unsigned int i = 1; i <= num_fields; ++i)
			{
				// Reverse iteration
				const Field* field = dstruct->get_field(num_fields-i);
				// Add field types to stack such that the first is on top
				type_stack.push(TypeAndDepth(field->get_type(), current_depth));
			}
		}
		else
		{
			parser_error("Cannot use struct-composition for non-struct type '"
			             + format_type(dtype->get_type()) + "'.");
		}
	}
#line 2863 "parser.cpp" /* yacc.c:1646  */
    break;

  case 139:
#line 1339 "parser.ypp" /* yacc.c:1646  */
    {
		if(type_stack.top().type->as_struct())
		{
			current_depth--;
		}
		type_stack.pop(); // Remove method type from the stack
		(yyval.str) = (yyvsp[-1].str);
	}
#line 2876 "parser.cpp" /* yacc.c:1646  */
    break;

  case 142:
#line 1353 "parser.ypp" /* yacc.c:1646  */
    {
		(yyval.str) = (yyvsp[-2].str) + (yyvsp[0].str);
	}
#line 2884 "parser.cpp" /* yacc.c:1646  */
    break;

  case 143:
#line 1357 "parser.ypp" /* yacc.c:1646  */
    {
		(yyval.str) = (yyvsp[-2].str) + (yyvsp[0].str);
	}
#line 2892 "parser.cpp" /* yacc.c:1646  */
    break;

  case 144:
#line 1364 "parser.ypp" /* yacc.c:1646  */
    {
		if(!check_depth()) depth_error("array");

		const DistributedType* dtype = type_stack.top().type;
		type_stack.pop();
		if(!dtype->as_array())
		{
			parser_error("Cannot use array-composition for non-array type '"
			             + format_type(dtype->get_type()) + "'.");
			(yyval.str) = "";
			break;
		}

		const ArrayType* array = dtype->as_array();
		if(!array->has_range()) // Simplest case
		{
			(yyval.str) = "";
			break;
		}

		// set to the obvious default value
		uint64_t min_array_elements = array->get_range().min.uinteger;
		(yyval.str) = string(min_array_elements, '\0');
	}
#line 2921 "parser.cpp" /* yacc.c:1646  */
    break;

  case 145:
#line 1389 "parser.ypp" /* yacc.c:1646  */
    {
		if(!check_depth()) depth_error("array");

		const DistributedType* dtype = type_stack.top().type;
		if(dtype->as_array())
		{
			const ArrayType* array = dtype->as_array();

			// For arrays we're going to do something pretty hacky:
			//    For every element we had we are going to increment the depth,
			//    and after we finish the element_values production we will compare
			//    the current_depth to the depth of our original symbol to check
			//    if the array size is proper.
			type_stack.push(TypeAndDepth(array->get_element_type(), current_depth));
		}
		else
		{
			parser_error("Cannot use array-composition for non-array type '"
			             + format_type(dtype->get_type()) + "'.");
		}
	}
#line 2947 "parser.cpp" /* yacc.c:1646  */
    break;

  case 146:
#line 1411 "parser.ypp" /* yacc.c:1646  */
    {
		if(type_stack.top().type->as_array())
		{
			uint64_t actual_size = current_depth - type_stack.top().depth;
			const ArrayType* array = type_stack.top().type->as_array();
			if(array->has_range())
			{
				if(actual_size > array->get_range().max.uinteger)
				{
					parser_error("Too many elements in array value, maximum "
					             + to_string(array->get_range().max.uinteger) + ".");
				}
				else if(actual_size < array->get_range().min.uinteger)
				{
					parser_error("Too few elements in array value, minimum "
					             + to_string(array->get_range().min.uinteger) + ".");
				}
			}
			current_depth = type_stack.top().depth;
		}
		type_stack.pop(); // Remove array type from the stack
		(yyval.str) = (yyvsp[-1].str);
	}
#line 2975 "parser.cpp" /* yacc.c:1646  */
    break;

  case 148:
#line 1439 "parser.ypp" /* yacc.c:1646  */
    {
		// We popped off the only element we added, so we're back to the array
		// Don't increment the depth; the array_expansion will add to
		// the current_depth depending on the number of elements it adds.
		const ArrayType* array = type_stack.top().type->as_array();
		type_stack.push(TypeAndDepth(array->get_element_type(), current_depth));
	}
#line 2987 "parser.cpp" /* yacc.c:1646  */
    break;

  case 149:
#line 1447 "parser.ypp" /* yacc.c:1646  */
    {
		(yyval.str) = (yyvsp[-3].str) + (yyvsp[0].str);
	}
#line 2995 "parser.cpp" /* yacc.c:1646  */
    break;

  case 150:
#line 1454 "parser.ypp" /* yacc.c:1646  */
    {
		current_depth++;
		(yyval.str) = (yyvsp[0].str);
	}
#line 3004 "parser.cpp" /* yacc.c:1646  */
    break;

  case 151:
#line 1459 "parser.ypp" /* yacc.c:1646  */
    {
		const DistributedType* dtype = type_stack.top().type;
		type_stack.pop(); // Pop that array element type
		current_depth += (yyvsp[0].u.uint32); // For arrays, we add 1 to the depth per element

		string base = number_value(dtype->get_type(), (yyvsp[-2].u.int64));

		string val;
		val.reserve(base.length() * (yyvsp[0].u.uint32));
		for(unsigned int i = 0; i < (yyvsp[0].u.uint32); ++i)
		{
			val += base;
		}
		(yyval.str) = val;
	}
#line 3024 "parser.cpp" /* yacc.c:1646  */
    break;

  case 152:
#line 1475 "parser.ypp" /* yacc.c:1646  */
    {
		const DistributedType* dtype = type_stack.top().type;
		type_stack.pop(); // Pop that array element type
		current_depth += (yyvsp[0].u.uint32); // For arrays, we add 1 to the depth per element

		string base = number_value(dtype->get_type(), (yyvsp[-2].u.uint64));

		string val;
		val.reserve(base.length() * (yyvsp[0].u.uint32));
		for(unsigned int i = 0; i < (yyvsp[0].u.uint32); ++i)
		{
			val += base;
		}
		(yyval.str) = val;
	}
#line 3044 "parser.cpp" /* yacc.c:1646  */
    break;

  case 153:
#line 1491 "parser.ypp" /* yacc.c:1646  */
    {
		const DistributedType* dtype = type_stack.top().type;
		type_stack.pop(); // Pop that array element type
		current_depth += (yyvsp[0].u.uint32); // For arrays, we add 1 to the depth per element

		string base = number_value(dtype->get_type(), (yyvsp[-2].u.real));

		string val;
		val.reserve(base.length() * (yyvsp[0].u.uint32));
		for(unsigned int i = 0; i < (yyvsp[0].u.uint32); ++i)
		{
			val += base;
		}
		(yyval.str) = val;
	}
#line 3064 "parser.cpp" /* yacc.c:1646  */
    break;

  case 154:
#line 1507 "parser.ypp" /* yacc.c:1646  */
    {
		const DistributedType* dtype = type_stack.top().type;
		type_stack.pop(); // Pop that array element type
		current_depth += (yyvsp[0].u.uint32); // For arrays, we add 1 to the depth per element

		if(dtype->get_type() == T_STRING)
		{
			if((yyvsp[-2].str).length() != dtype->get_size())
			{
				parser_error("Value for fixed-length string has incorrect length.");
			}

			string val;
			val.reserve((yyvsp[-2].str).length() * (yyvsp[0].u.uint32));
			for(unsigned int i = 0; i < (yyvsp[0].u.uint32); ++i)
			{
				val += (yyvsp[-2].str);
			}
			(yyval.str) = val;
		}
		else if(dtype->get_type() == T_VARSTRING)
		{
			// TODO: Check for range limits
			// Prepend length tag
			sizetag_t length = (yyvsp[-2].str).length();
			string base = string((char*)&length, sizeof(sizetag_t)) + (yyvsp[-2].str);

			string val;
			val.reserve(base.length() * (yyvsp[0].u.uint32));
			for(unsigned int i = 0; i < (yyvsp[0].u.uint32); ++i)
			{
				val += base;
			}
			(yyval.str) = val;
		}
		else
		{
			parser_error("Cannot use string value for non-string type '"
			             + format_type(dtype->get_type()) + "'.");

			string val;
			val.reserve((yyvsp[-2].str).length() * (yyvsp[0].u.uint32));
			for(unsigned int i = 0; i < (yyvsp[0].u.uint32); ++i)
			{
				val += (yyvsp[-2].str);
			}
			(yyval.str) = val;
		}
	}
#line 3118 "parser.cpp" /* yacc.c:1646  */
    break;

  case 155:
#line 1557 "parser.ypp" /* yacc.c:1646  */
    {
		const DistributedType* dtype = type_stack.top().type;
		type_stack.pop(); // Pop that array element type
		current_depth += (yyvsp[0].u.uint32); // For arrays, we add 1 to the depth per element

		if(dtype->get_type() == T_BLOB)
		{
			if((yyvsp[-2].str).length() != dtype->get_size())
			{
				parser_error("Value for fixed-length blob has incorrect length.");
			}

			string val;
			val.reserve((yyvsp[-2].str).length() * (yyvsp[0].u.uint32));
			for(unsigned int i = 0; i < (yyvsp[0].u.uint32); ++i)
			{
				val += (yyvsp[-2].str);
			}
			(yyval.str) = val;
		}
		else if(dtype->get_type() == T_VARBLOB)
		{
			// TODO: Check for range limits
			// Prepend length tag
			sizetag_t length = (yyvsp[-2].str).length();
			string base = string((char*)&length, sizeof(sizetag_t)) + (yyvsp[-2].str);

			string val;
			val.reserve(base.length() * (yyvsp[0].u.uint32));
			for(unsigned int i = 0; i < (yyvsp[0].u.uint32); ++i)
			{
				val += base;
			}
			(yyval.str) = val;

		}
		else
		{
			parser_error("Cannot use hex value for non-blob type '"
			             + format_type(dtype->get_type()) + "'.");

			string val;
			val.reserve((yyvsp[-2].str).length() * (yyvsp[0].u.uint32));
			for(unsigned int i = 0; i < (yyvsp[0].u.uint32); ++i)
			{
				val += (yyvsp[-2].str);
			}
			(yyval.str) = val;
		}
	}
#line 3173 "parser.cpp" /* yacc.c:1646  */
    break;

  case 156:
#line 1610 "parser.ypp" /* yacc.c:1646  */
    { (yyval.u.type) = T_STRING; }
#line 3179 "parser.cpp" /* yacc.c:1646  */
    break;

  case 157:
#line 1611 "parser.ypp" /* yacc.c:1646  */
    { (yyval.u.type) = T_BLOB; }
#line 3185 "parser.cpp" /* yacc.c:1646  */
    break;

  case 158:
#line 1615 "parser.ypp" /* yacc.c:1646  */
    { (yyval.u.type) = T_CHAR; }
#line 3191 "parser.cpp" /* yacc.c:1646  */
    break;

  case 159:
#line 1616 "parser.ypp" /* yacc.c:1646  */
    { (yyval.u.type) = T_INT8; }
#line 3197 "parser.cpp" /* yacc.c:1646  */
    break;

  case 160:
#line 1617 "parser.ypp" /* yacc.c:1646  */
    { (yyval.u.type) = T_INT16; }
#line 3203 "parser.cpp" /* yacc.c:1646  */
    break;

  case 161:
#line 1618 "parser.ypp" /* yacc.c:1646  */
    { (yyval.u.type) = T_INT32; }
#line 3209 "parser.cpp" /* yacc.c:1646  */
    break;

  case 162:
#line 1619 "parser.ypp" /* yacc.c:1646  */
    { (yyval.u.type) = T_INT64; }
#line 3215 "parser.cpp" /* yacc.c:1646  */
    break;

  case 163:
#line 1620 "parser.ypp" /* yacc.c:1646  */
    { (yyval.u.type) = T_UINT8; }
#line 3221 "parser.cpp" /* yacc.c:1646  */
    break;

  case 164:
#line 1621 "parser.ypp" /* yacc.c:1646  */
    { (yyval.u.type) = T_UINT16; }
#line 3227 "parser.cpp" /* yacc.c:1646  */
    break;

  case 165:
#line 1622 "parser.ypp" /* yacc.c:1646  */
    { (yyval.u.type) = T_UINT32; }
#line 3233 "parser.cpp" /* yacc.c:1646  */
    break;

  case 166:
#line 1623 "parser.ypp" /* yacc.c:1646  */
    { (yyval.u.type) = T_UINT64; }
#line 3239 "parser.cpp" /* yacc.c:1646  */
    break;

  case 167:
#line 1624 "parser.ypp" /* yacc.c:1646  */
    { (yyval.u.type) = T_FLOAT32; }
#line 3245 "parser.cpp" /* yacc.c:1646  */
    break;

  case 168:
#line 1625 "parser.ypp" /* yacc.c:1646  */
    { (yyval.u.type) = T_FLOAT64; }
#line 3251 "parser.cpp" /* yacc.c:1646  */
    break;

  case 169:
#line 1630 "parser.ypp" /* yacc.c:1646  */
    {
		(yyval.strings) = vector<string>();
	}
#line 3259 "parser.cpp" /* yacc.c:1646  */
    break;

  case 170:
#line 1634 "parser.ypp" /* yacc.c:1646  */
    {
		if(!parsed_file->has_keyword((yyvsp[0].str)))
		{
			parser_error("Keyword '" + (yyvsp[0].str) + "' has not been declared.");
			break;
		}

		(yyvsp[-1].strings).push_back((yyvsp[0].str));
		(yyval.strings) = (yyvsp[-1].strings);
	}
#line 3274 "parser.cpp" /* yacc.c:1646  */
    break;


#line 3278 "parser.cpp" /* yacc.c:1646  */
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

  /* Now 'shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTOKENS] + *yyssp;
  if (0 <= yystate && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTOKENS];

  goto yynewstate;


/*--------------------------------------.
| yyerrlab -- here on detecting error.  |
`--------------------------------------*/
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

  /* Do not reclaim the symbols of the rule whose action triggered
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
  yyerrstatus = 3;      /* Each real token shifted decrements this.  */

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
  /* Do not reclaim the symbols of the rule whose action triggered
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
  return yyresult;
}
#line 1650 "parser.ypp" /* yacc.c:1906  */
 /* Start helper function section */


bool check_depth()
{
	return (!type_stack.empty() && current_depth == type_stack.top().depth);
}

void depth_error(string what)
{
	if(type_stack.empty() || current_depth < type_stack.top().depth)
	{
		parser_error("Too many nested values while parsing value for " + what + ".");
	}
	else
	{
		parser_error("Too few nested values while parsing value for " + what + ".");	
	}
}

void depth_error(int depth, string what)
{
	if(current_depth > depth)
	{
		parser_error("Too few nested values before this " + what + " value.");
	}
	else
	{
		parser_error("Too many nested values before this " + what + " value.");
	}
}

string number_value(Type type, double &number)
{
	switch(type)
	{
		case T_FLOAT32:
		{
			float v = number;
			if(v == INFINITY || v == -INFINITY)
			{
				parser_error("Value is out of range for type 'float32'.");
			}

			return string((char*)&v, sizeof(float));
		}
		case T_FLOAT64:
		{
			return string((char*)&number, sizeof(double));
		}
		case T_INT8:
		case T_INT16:
		case T_INT32:
		case T_INT64:
		case T_CHAR:
		case T_UINT8:
		case T_UINT16:
		case T_UINT32:
		case T_UINT64:
		{
			parser_error("Cannot use floating-point value for integer datatype.");
			return string();
		}
		default:
		{
			parser_error("Cannot use floating-point value for non-numeric datatype.");
			return string();
		}
	}
}

string number_value(Type type, int64_t &number)
{
	switch(type)
	{
		case T_INT8:
		{
			if(INT8_MIN > number || number > INT8_MAX)
			{
				parser_error("Signed integer out of range for type 'int8'.");
			}

			int8_t v = number;
			return string((char*)&v, sizeof(int8_t));
		}
		case T_INT16:
		{
			if(INT16_MIN > number || number > INT16_MAX)
			{
				parser_error("Signed integer out of range for type 'int16'.");
			}

			uint16_t v = number;
			return string((char*)&v, sizeof(int16_t));
		}
		case T_INT32:
		{
			if(INT32_MIN > number || number > INT32_MAX)
			{
				parser_error("Signed integer out of range for type 'int32'.");
			}

			int32_t v = number;
			return string((char*)&v, sizeof(int32_t));
		}
		case T_INT64:
		{
			return string((char*)&number, sizeof(int64_t));
		}
		case T_CHAR:
		case T_UINT8:
		case T_UINT16:
		case T_UINT32:
		case T_UINT64:
		{
			if(number < 0)
			{
				parser_error("Can't use negative value for unsigned integer datatype.");
			}
			uint64_t v = number;
			return number_value(type, v);
		}
		case T_FLOAT32:
		case T_FLOAT64:
		{
			// Note: Expecting number to be converted to a double by value (ie. -1 becomes -1.0)
			double v = number;
			return number_value(type, v);
		}
		default:
		{
			parser_error("Cannot use signed integer value for non-numeric datatype.");
			return string();
		}
	}
}

string number_value(Type type, uint64_t &number)
{
	switch(type)
	{
		case T_CHAR:
		case T_UINT8:
		{
			if(number > UINT8_MAX)
			{
				parser_error("Unsigned integer out of range for type 'uint8'.");
			}

			uint8_t v = number;
			return string((char*)&v, sizeof(uint8_t));
		}
		case T_UINT16:
		{
			if(number > UINT16_MAX)
			{
				parser_error("Unsigned integer out of range for type 'uint16'.");
			}

			uint16_t v = number;
			return string((char*)&v, sizeof(uint16_t));
		}
		case T_UINT32:
		{
			if(number > UINT32_MAX)
			{
				parser_error("Unsigned integer out of range for type 'uint32'.");
			}

			uint32_t v = number;
			return string((char*)&v, sizeof(uint32_t));
		}
		case T_UINT64:
		{
			return string((char*)&number, sizeof(uint64_t));
		}
		case T_INT8:
		case T_INT16:
		case T_INT32:
		case T_INT64:
		{
			if(number > INT64_MAX)
			{
				parser_error("Unsigned integer out of range for signed integer datatype.");
			}
			int64_t v = number;
			return number_value(type, v);
		}
		case T_FLOAT32:
		case T_FLOAT64:
		{
			// Note: Expecting number to be converted to a double by value (ie. 3 becomes 3.0)
			double v = number;
			return number_value(type, v);
		}
		default:
		{
			parser_error("Cannot use unsigned integer value for non-numeric datatype.");
			return string();
		}
	}
}


} // close namespace dclass
