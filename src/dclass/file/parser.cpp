/* A Bison parser, made by GNU Bison 3.8.2.  */

/* Bison implementation for Yacc-like parsers in C

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

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
   especially those whose name start with YY_ or yy_.  They are
   private implementation details that can be changed or removed.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output, and Bison version.  */
#define YYBISON 30802

/* Bison version string.  */
#define YYBISON_VERSION "3.8.2"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1




/* First part of user prologue.  */
#line 4 "parser.ypp"

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

	#include "util/byteorder.h"

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
	static File* parsed_file = nullptr;
	static string* parsed_value = nullptr;

	// Parser state
	static Class* current_class = nullptr;
	static Struct* current_struct = nullptr;

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
	static ArrayType* basic_string = nullptr;
	static ArrayType* basic_blob = nullptr;

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
		parsed_file = nullptr;
		parsed_value = nullptr;
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

#line 193 "parser.cpp"

# ifndef YY_CAST
#  ifdef __cplusplus
#   define YY_CAST(Type, Val) static_cast<Type> (Val)
#   define YY_REINTERPRET_CAST(Type, Val) reinterpret_cast<Type> (Val)
#  else
#   define YY_CAST(Type, Val) ((Type) (Val))
#   define YY_REINTERPRET_CAST(Type, Val) ((Type) (Val))
#  endif
# endif
# ifndef YY_NULLPTR
#  if defined __cplusplus
#   if 201103L <= __cplusplus
#    define YY_NULLPTR nullptr
#   else
#    define YY_NULLPTR 0
#   endif
#  else
#   define YY_NULLPTR ((void*)0)
#  endif
# endif

#include "parser.h"
/* Symbol kind.  */
enum yysymbol_kind_t
{
  YYSYMBOL_YYEMPTY = -2,
  YYSYMBOL_YYEOF = 0,                      /* "end of file"  */
  YYSYMBOL_YYerror = 1,                    /* error  */
  YYSYMBOL_YYUNDEF = 2,                    /* "invalid token"  */
  YYSYMBOL_UNSIGNED_INTEGER = 3,           /* UNSIGNED_INTEGER  */
  YYSYMBOL_REAL = 4,                       /* REAL  */
  YYSYMBOL_STRING = 5,                     /* STRING  */
  YYSYMBOL_HEX_STRING = 6,                 /* HEX_STRING  */
  YYSYMBOL_IDENTIFIER = 7,                 /* IDENTIFIER  */
  YYSYMBOL_CHAR = 8,                       /* CHAR  */
  YYSYMBOL_START_DC_FILE = 9,              /* START_DC_FILE  */
  YYSYMBOL_START_DC_VALUE = 10,            /* START_DC_VALUE  */
  YYSYMBOL_KW_DCLASS = 11,                 /* KW_DCLASS  */
  YYSYMBOL_KW_STRUCT = 12,                 /* KW_STRUCT  */
  YYSYMBOL_KW_FROM = 13,                   /* KW_FROM  */
  YYSYMBOL_KW_IMPORT = 14,                 /* KW_IMPORT  */
  YYSYMBOL_KW_TYPEDEF = 15,                /* KW_TYPEDEF  */
  YYSYMBOL_KW_KEYWORD = 16,                /* KW_KEYWORD  */
  YYSYMBOL_KW_INT8 = 17,                   /* KW_INT8  */
  YYSYMBOL_KW_INT16 = 18,                  /* KW_INT16  */
  YYSYMBOL_KW_INT32 = 19,                  /* KW_INT32  */
  YYSYMBOL_KW_INT64 = 20,                  /* KW_INT64  */
  YYSYMBOL_KW_UINT8 = 21,                  /* KW_UINT8  */
  YYSYMBOL_KW_UINT16 = 22,                 /* KW_UINT16  */
  YYSYMBOL_KW_UINT32 = 23,                 /* KW_UINT32  */
  YYSYMBOL_KW_UINT64 = 24,                 /* KW_UINT64  */
  YYSYMBOL_KW_FLOAT32 = 25,                /* KW_FLOAT32  */
  YYSYMBOL_KW_FLOAT64 = 26,                /* KW_FLOAT64  */
  YYSYMBOL_KW_STRING = 27,                 /* KW_STRING  */
  YYSYMBOL_KW_BLOB = 28,                   /* KW_BLOB  */
  YYSYMBOL_KW_CHAR = 29,                   /* KW_CHAR  */
  YYSYMBOL_30_ = 30,                       /* ';'  */
  YYSYMBOL_31_ = 31,                       /* '.'  */
  YYSYMBOL_32_ = 32,                       /* '*'  */
  YYSYMBOL_33_ = 33,                       /* ','  */
  YYSYMBOL_34_ = 34,                       /* '/'  */
  YYSYMBOL_35_ = 35,                       /* '['  */
  YYSYMBOL_36_ = 36,                       /* ']'  */
  YYSYMBOL_37_ = 37,                       /* '{'  */
  YYSYMBOL_38_ = 38,                       /* '}'  */
  YYSYMBOL_39_ = 39,                       /* ':'  */
  YYSYMBOL_40_ = 40,                       /* '='  */
  YYSYMBOL_41_ = 41,                       /* '('  */
  YYSYMBOL_42_ = 42,                       /* ')'  */
  YYSYMBOL_43_ = 43,                       /* '%'  */
  YYSYMBOL_44_ = 44,                       /* '-'  */
  YYSYMBOL_45_ = 45,                       /* '+'  */
  YYSYMBOL_YYACCEPT = 46,                  /* $accept  */
  YYSYMBOL_grammar = 47,                   /* grammar  */
  YYSYMBOL_file = 48,                      /* file  */
  YYSYMBOL_value = 49,                     /* value  */
  YYSYMBOL_import = 50,                    /* import  */
  YYSYMBOL_import_module = 51,             /* import_module  */
  YYSYMBOL_import_symbols = 52,            /* import_symbols  */
  YYSYMBOL_import_symbol_list = 53,        /* import_symbol_list  */
  YYSYMBOL_import_alternatives = 54,       /* import_alternatives  */
  YYSYMBOL_typedef = 55,                   /* typedef  */
  YYSYMBOL_typedef_type = 56,              /* typedef_type  */
  YYSYMBOL_nonmethod_type_with_name = 57,  /* nonmethod_type_with_name  */
  YYSYMBOL_defined_type = 58,              /* defined_type  */
  YYSYMBOL_keyword_decl = 59,              /* keyword_decl  */
  YYSYMBOL_keyword_decl_list = 60,         /* keyword_decl_list  */
  YYSYMBOL_dclass = 61,                    /* dclass  */
  YYSYMBOL_62_1 = 62,                      /* $@1  */
  YYSYMBOL_class_inheritance = 63,         /* class_inheritance  */
  YYSYMBOL_class_parents = 64,             /* class_parents  */
  YYSYMBOL_defined_class = 65,             /* defined_class  */
  YYSYMBOL_class_fields = 66,              /* class_fields  */
  YYSYMBOL_class_field = 67,               /* class_field  */
  YYSYMBOL_dstruct = 68,                   /* dstruct  */
  YYSYMBOL_69_2 = 69,                      /* $@2  */
  YYSYMBOL_struct_fields = 70,             /* struct_fields  */
  YYSYMBOL_struct_field = 71,              /* struct_field  */
  YYSYMBOL_named_field = 72,               /* named_field  */
  YYSYMBOL_unnamed_field = 73,             /* unnamed_field  */
  YYSYMBOL_74_3 = 74,                      /* $@3  */
  YYSYMBOL_field_with_name = 75,           /* field_with_name  */
  YYSYMBOL_field_with_name_as_array = 76,  /* field_with_name_as_array  */
  YYSYMBOL_field_with_name_and_default = 77, /* field_with_name_and_default  */
  YYSYMBOL_78_4 = 78,                      /* $@4  */
  YYSYMBOL_79_5 = 79,                      /* $@5  */
  YYSYMBOL_80_6 = 80,                      /* $@6  */
  YYSYMBOL_method_as_field = 81,           /* method_as_field  */
  YYSYMBOL_nonmethod_type = 82,            /* nonmethod_type  */
  YYSYMBOL_nonmethod_type_no_array = 83,   /* nonmethod_type_no_array  */
  YYSYMBOL_type_with_array = 84,           /* type_with_array  */
  YYSYMBOL_molecular = 85,                 /* molecular  */
  YYSYMBOL_defined_field = 86,             /* defined_field  */
  YYSYMBOL_builtin_array_type = 87,        /* builtin_array_type  */
  YYSYMBOL_numeric_type = 88,              /* numeric_type  */
  YYSYMBOL_numeric_token_only = 89,        /* numeric_token_only  */
  YYSYMBOL_numeric_with_range = 90,        /* numeric_with_range  */
  YYSYMBOL_numeric_with_modulus = 91,      /* numeric_with_modulus  */
  YYSYMBOL_numeric_with_divisor = 92,      /* numeric_with_divisor  */
  YYSYMBOL_method = 93,                    /* method  */
  YYSYMBOL_method_body = 94,               /* method_body  */
  YYSYMBOL_parameter = 95,                 /* parameter  */
  YYSYMBOL_96_7 = 96,                      /* $@7  */
  YYSYMBOL_param_with_name = 97,           /* param_with_name  */
  YYSYMBOL_param_with_name_as_array = 98,  /* param_with_name_as_array  */
  YYSYMBOL_param_with_name_and_default = 99, /* param_with_name_and_default  */
  YYSYMBOL_100_8 = 100,                    /* $@8  */
  YYSYMBOL_101_9 = 101,                    /* $@9  */
  YYSYMBOL_numeric_range = 102,            /* numeric_range  */
  YYSYMBOL_array_range = 103,              /* array_range  */
  YYSYMBOL_char_or_uint = 104,             /* char_or_uint  */
  YYSYMBOL_small_unsigned_integer = 105,   /* small_unsigned_integer  */
  YYSYMBOL_number = 106,                   /* number  */
  YYSYMBOL_char_or_number = 107,           /* char_or_number  */
  YYSYMBOL_type_value = 108,               /* type_value  */
  YYSYMBOL_method_value = 109,             /* method_value  */
  YYSYMBOL_110_10 = 110,                   /* $@10  */
  YYSYMBOL_parameter_values = 111,         /* parameter_values  */
  YYSYMBOL_struct_value = 112,             /* struct_value  */
  YYSYMBOL_113_11 = 113,                   /* $@11  */
  YYSYMBOL_field_values = 114,             /* field_values  */
  YYSYMBOL_array_value = 115,              /* array_value  */
  YYSYMBOL_116_12 = 116,                   /* $@12  */
  YYSYMBOL_element_values = 117,           /* element_values  */
  YYSYMBOL_118_13 = 118,                   /* $@13  */
  YYSYMBOL_array_expansion = 119,          /* array_expansion  */
  YYSYMBOL_signed_integer = 120,           /* signed_integer  */
  YYSYMBOL_array_type_token = 121,         /* array_type_token  */
  YYSYMBOL_numeric_type_token = 122,       /* numeric_type_token  */
  YYSYMBOL_keyword_list = 123,             /* keyword_list  */
  YYSYMBOL_empty = 124                     /* empty  */
};
typedef enum yysymbol_kind_t yysymbol_kind_t;




#ifdef short
# undef short
#endif

/* On compilers that do not define __PTRDIFF_MAX__ etc., make sure
   <limits.h> and (if available) <stdint.h> are included
   so that the code can choose integer types of a good width.  */

#ifndef __PTRDIFF_MAX__
# include <limits.h> /* INFRINGES ON USER NAME SPACE */
# if defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#  include <stdint.h> /* INFRINGES ON USER NAME SPACE */
#  define YY_STDINT_H
# endif
#endif

/* Narrow types that promote to a signed type and that can represent a
   signed or unsigned integer of at least N bits.  In tables they can
   save space and decrease cache pressure.  Promoting to a signed type
   helps avoid bugs in integer arithmetic.  */

#ifdef __INT_LEAST8_MAX__
typedef __INT_LEAST8_TYPE__ yytype_int8;
#elif defined YY_STDINT_H
typedef int_least8_t yytype_int8;
#else
typedef signed char yytype_int8;
#endif

#ifdef __INT_LEAST16_MAX__
typedef __INT_LEAST16_TYPE__ yytype_int16;
#elif defined YY_STDINT_H
typedef int_least16_t yytype_int16;
#else
typedef short yytype_int16;
#endif

/* Work around bug in HP-UX 11.23, which defines these macros
   incorrectly for preprocessor constants.  This workaround can likely
   be removed in 2023, as HPE has promised support for HP-UX 11.23
   (aka HP-UX 11i v2) only through the end of 2022; see Table 2 of
   <https://h20195.www2.hpe.com/V2/getpdf.aspx/4AA4-7673ENW.pdf>.  */
#ifdef __hpux
# undef UINT_LEAST8_MAX
# undef UINT_LEAST16_MAX
# define UINT_LEAST8_MAX 255
# define UINT_LEAST16_MAX 65535
#endif

#if defined __UINT_LEAST8_MAX__ && __UINT_LEAST8_MAX__ <= __INT_MAX__
typedef __UINT_LEAST8_TYPE__ yytype_uint8;
#elif (!defined __UINT_LEAST8_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST8_MAX <= INT_MAX)
typedef uint_least8_t yytype_uint8;
#elif !defined __UINT_LEAST8_MAX__ && UCHAR_MAX <= INT_MAX
typedef unsigned char yytype_uint8;
#else
typedef short yytype_uint8;
#endif

#if defined __UINT_LEAST16_MAX__ && __UINT_LEAST16_MAX__ <= __INT_MAX__
typedef __UINT_LEAST16_TYPE__ yytype_uint16;
#elif (!defined __UINT_LEAST16_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST16_MAX <= INT_MAX)
typedef uint_least16_t yytype_uint16;
#elif !defined __UINT_LEAST16_MAX__ && USHRT_MAX <= INT_MAX
typedef unsigned short yytype_uint16;
#else
typedef int yytype_uint16;
#endif

#ifndef YYPTRDIFF_T
# if defined __PTRDIFF_TYPE__ && defined __PTRDIFF_MAX__
#  define YYPTRDIFF_T __PTRDIFF_TYPE__
#  define YYPTRDIFF_MAXIMUM __PTRDIFF_MAX__
# elif defined PTRDIFF_MAX
#  ifndef ptrdiff_t
#   include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  endif
#  define YYPTRDIFF_T ptrdiff_t
#  define YYPTRDIFF_MAXIMUM PTRDIFF_MAX
# else
#  define YYPTRDIFF_T long
#  define YYPTRDIFF_MAXIMUM LONG_MAX
# endif
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned
# endif
#endif

#define YYSIZE_MAXIMUM                                  \
  YY_CAST (YYPTRDIFF_T,                                 \
           (YYPTRDIFF_MAXIMUM < YY_CAST (YYSIZE_T, -1)  \
            ? YYPTRDIFF_MAXIMUM                         \
            : YY_CAST (YYSIZE_T, -1)))

#define YYSIZEOF(X) YY_CAST (YYPTRDIFF_T, sizeof (X))


/* Stored state numbers (used for stacks). */
typedef yytype_int16 yy_state_t;

/* State numbers in computations.  */
typedef int yy_state_fast_t;

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


#ifndef YY_ATTRIBUTE_PURE
# if defined __GNUC__ && 2 < __GNUC__ + (96 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_PURE __attribute__ ((__pure__))
# else
#  define YY_ATTRIBUTE_PURE
# endif
#endif

#ifndef YY_ATTRIBUTE_UNUSED
# if defined __GNUC__ && 2 < __GNUC__ + (7 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_UNUSED __attribute__ ((__unused__))
# else
#  define YY_ATTRIBUTE_UNUSED
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YY_USE(E) ((void) (E))
#else
# define YY_USE(E) /* empty */
#endif

/* Suppress an incorrect diagnostic about yylval being uninitialized.  */
#if defined __GNUC__ && ! defined __ICC && 406 <= __GNUC__ * 100 + __GNUC_MINOR__
# if __GNUC__ * 100 + __GNUC_MINOR__ < 407
#  define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                           \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")
# else
#  define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                           \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")              \
    _Pragma ("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")
# endif
# define YY_IGNORE_MAYBE_UNINITIALIZED_END      \
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

#if defined __cplusplus && defined __GNUC__ && ! defined __ICC && 6 <= __GNUC__
# define YY_IGNORE_USELESS_CAST_BEGIN                          \
    _Pragma ("GCC diagnostic push")                            \
    _Pragma ("GCC diagnostic ignored \"-Wuseless-cast\"")
# define YY_IGNORE_USELESS_CAST_END            \
    _Pragma ("GCC diagnostic pop")
#endif
#ifndef YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_END
#endif


#define YY_ASSERT(E) ((void) (0 && (E)))

#if 1

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
#endif /* 1 */

#if (! defined yyoverflow \
     && (! defined __cplusplus \
         || (defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yy_state_t yyss_alloc;
  YYSTYPE yyvs_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (YYSIZEOF (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (YYSIZEOF (yy_state_t) + YYSIZEOF (YYSTYPE)) \
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
        YYPTRDIFF_T yynewbytes;                                         \
        YYCOPY (&yyptr->Stack_alloc, Stack, yysize);                    \
        Stack = &yyptr->Stack_alloc;                                    \
        yynewbytes = yystacksize * YYSIZEOF (*Stack) + YYSTACK_GAP_MAXIMUM; \
        yyptr += yynewbytes / YYSIZEOF (*yyptr);                        \
      }                                                                 \
    while (0)

#endif

#if defined YYCOPY_NEEDED && YYCOPY_NEEDED
/* Copy COUNT objects from SRC to DST.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(Dst, Src, Count) \
      __builtin_memcpy (Dst, Src, YY_CAST (YYSIZE_T, (Count)) * sizeof (*(Src)))
#  else
#   define YYCOPY(Dst, Src, Count)              \
      do                                        \
        {                                       \
          YYPTRDIFF_T yyi;                      \
          for (yyi = 0; yyi < (Count); yyi++)   \
            (Dst)[yyi] = (Src)[yyi];            \
        }                                       \
      while (0)
#  endif
# endif
#endif /* !YYCOPY_NEEDED */

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  23
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   289

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  46
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  79
/* YYNRULES -- Number of rules.  */
#define YYNRULES  171
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  266

/* YYMAXUTOK -- Last valid token kind.  */
#define YYMAXUTOK   284


/* YYTRANSLATE(TOKEN-NUM) -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, with out-of-bounds checking.  */
#define YYTRANSLATE(YYX)                                \
  (0 <= (YYX) && (YYX) <= YYMAXUTOK                     \
   ? YY_CAST (yysymbol_kind_t, yytranslate[YYX])        \
   : YYSYMBOL_YYUNDEF)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex.  */
static const yytype_int8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,    43,     2,     2,
      41,    42,    32,    45,    33,    44,    31,    34,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    39,    30,
       2,    40,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    35,     2,    36,     2,     2,     2,     2,     2,     2,
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
      25,    26,    27,    28,    29
};

#if YYDEBUG
/* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_int16 yyrline[] =
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
    1094,  1095,  1096,  1100,  1101,  1102,  1106,  1118,  1122,  1135,
    1136,  1137,  1141,  1153,  1157,  1172,  1187,  1202,  1230,  1267,
    1301,  1302,  1307,  1306,  1348,  1349,  1357,  1356,  1398,  1399,
    1400,  1404,  1411,  1456,  1455,  1528,  1530,  1529,  1550,  1555,
    1577,  1599,  1621,  1677,  1737,  1738,  1742,  1743,  1747,  1748,
    1749,  1750,  1751,  1752,  1753,  1754,  1755,  1756,  1757,  1761,
    1765,  1779
};
#endif

/** Accessing symbol of state STATE.  */
#define YY_ACCESSING_SYMBOL(State) YY_CAST (yysymbol_kind_t, yystos[State])

#if 1
/* The user-facing name of the symbol whose (internal) number is
   YYSYMBOL.  No bounds checking.  */
static const char *yysymbol_name (yysymbol_kind_t yysymbol) YY_ATTRIBUTE_UNUSED;

/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "\"end of file\"", "error", "\"invalid token\"", "UNSIGNED_INTEGER",
  "REAL", "STRING", "HEX_STRING", "IDENTIFIER", "CHAR", "START_DC_FILE",
  "START_DC_VALUE", "KW_DCLASS", "KW_STRUCT", "KW_FROM", "KW_IMPORT",
  "KW_TYPEDEF", "KW_KEYWORD", "KW_INT8", "KW_INT16", "KW_INT32",
  "KW_INT64", "KW_UINT8", "KW_UINT16", "KW_UINT32", "KW_UINT64",
  "KW_FLOAT32", "KW_FLOAT64", "KW_STRING", "KW_BLOB", "KW_CHAR", "';'",
  "'.'", "'*'", "','", "'/'", "'['", "']'", "'{'", "'}'", "':'", "'='",
  "'('", "')'", "'%'", "'-'", "'+'", "$accept", "grammar", "file", "value",
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
  "array_range", "char_or_uint", "small_unsigned_integer", "number",
  "char_or_number", "type_value", "method_value", "$@10",
  "parameter_values", "struct_value", "$@11", "field_values",
  "array_value", "$@12", "element_values", "$@13", "array_expansion",
  "signed_integer", "array_type_token", "numeric_type_token",
  "keyword_list", "empty", YY_NULLPTR
};

static const char *
yysymbol_name (yysymbol_kind_t yysymbol)
{
  return yytname[yysymbol];
}
#endif

#define YYPACT_NINF (-188)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-1)

#define yytable_value_is_error(Yyn) \
  0

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
static const yytype_int16 yypact[] =
{
      76,  -188,    14,    31,   259,  -188,  -188,  -188,  -188,  -188,
    -188,    24,  -188,  -188,    68,    71,  -188,  -188,  -188,  -188,
    -188,  -188,  -188,  -188,    45,    83,    84,    84,   219,  -188,
    -188,  -188,  -188,    58,  -188,  -188,  -188,  -188,    73,    14,
      91,  -188,  -188,  -188,  -188,  -188,    11,    64,    69,  -188,
    -188,  -188,  -188,  -188,  -188,  -188,  -188,  -188,  -188,  -188,
    -188,  -188,  -188,  -188,    66,    95,  -188,    70,    74,    77,
      -8,  -188,    16,    63,    72,  -188,   104,  -188,    38,    82,
      87,    92,    93,  -188,    12,  -188,    97,  -188,  -188,    23,
    -188,    -6,    94,    78,     8,    84,   109,    38,  -188,    38,
      38,    38,   128,    20,     9,   128,    20,    20,    38,  -188,
    -188,  -188,    96,    90,  -188,  -188,   128,   128,   128,   128,
    -188,  -188,   128,    14,  -188,    91,  -188,   130,   101,  -188,
    -188,  -188,  -188,   106,    64,    64,  -188,   105,   107,   111,
     112,  -188,  -188,  -188,  -188,    98,  -188,   126,  -188,  -188,
    -188,  -188,   100,   103,   108,  -188,    38,  -188,  -188,  -188,
    -188,    73,  -188,  -188,  -188,  -188,  -188,   116,  -188,  -188,
     165,  -188,    84,  -188,  -188,  -188,  -188,  -188,    20,  -188,
    -188,  -188,  -188,  -188,   130,   190,  -188,   110,  -188,  -188,
    -188,   123,  -188,  -188,    27,    28,  -188,   115,     7,    64,
    -188,  -188,    -7,  -188,  -188,   141,  -188,   140,   139,  -188,
      -3,  -188,    38,  -188,    38,  -188,   133,  -188,   168,  -188,
     169,  -188,   168,  -188,   137,   171,  -188,    40,    49,  -188,
     219,  -188,   143,    91,   160,    91,  -188,    91,  -188,  -188,
    -188,  -188,  -188,  -188,    38,  -188,    38,  -188,  -188,  -188,
    -188,  -188,  -188,    91,  -188,    91,   162,    91,   163,    91,
    -188,  -188,  -188,  -188,  -188,  -188
};

/* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE does not specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       0,   171,   171,     0,     2,     4,   125,   126,   128,   129,
     127,   143,   136,   132,     0,     0,     3,    12,    13,   131,
     130,   124,    11,     1,     0,     0,     0,     0,     0,   171,
       5,     8,     9,    24,    10,     6,     7,   142,     0,     0,
       0,   155,   154,    32,    44,    22,     0,    16,    14,    28,
     159,   160,   161,   162,   163,   164,   165,   166,   167,   168,
     156,   157,   158,    25,    70,     0,    68,    69,    72,    71,
      82,    85,    83,    84,    80,    86,    29,    30,   171,   125,
     126,   128,   129,   148,     0,   145,   124,   138,   139,     0,
     134,     0,   171,     0,     0,     0,     0,   171,    27,   171,
     171,   171,     0,   171,     0,     0,   171,   171,   171,    31,
     118,   116,     0,   114,   117,   113,     0,     0,     0,     0,
     146,   144,     0,     0,   137,     0,   133,     0,     0,    34,
     171,    19,    15,    18,    20,    17,    23,     0,     0,     0,
       0,    91,   119,   121,   122,     0,   123,   111,   120,   110,
      90,    92,     0,     0,     0,    26,     0,   150,   151,   152,
     153,     0,   149,   140,   141,   135,    38,    35,    36,   171,
       0,    46,     0,    74,    76,    75,    73,    87,     0,    88,
      89,    81,   115,   147,     0,     0,    39,    28,    47,    45,
      58,     0,    49,    50,    51,    52,    53,    54,    55,    21,
     112,    37,    28,    40,    33,     0,   171,    43,     0,    67,
       0,    48,   171,    61,   171,    63,     0,    56,     0,    41,
      42,   169,     0,    93,   100,    68,    95,    97,    98,    99,
       0,    94,     0,     0,     0,     0,    65,     0,    79,    77,
     170,    78,   101,   103,   171,   106,   171,   108,    96,    59,
      62,    60,    64,     0,    57,     0,     0,     0,     0,     0,
      66,   102,   104,   107,   105,   109
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -188,  -188,  -188,  -188,  -188,   173,  -188,  -188,   -84,  -188,
    -188,   174,  -188,  -188,  -188,  -188,  -188,  -188,  -188,    17,
    -188,  -188,  -188,  -188,  -188,  -188,    19,  -188,  -188,  -188,
    -188,  -188,  -188,  -188,  -188,  -188,  -164,  -187,  -188,  -188,
     -17,  -188,  -188,  -188,  -188,  -188,  -188,  -188,  -188,    -9,
    -188,  -188,  -188,  -188,  -188,  -188,   -24,   -92,    67,   147,
     118,    46,    -2,   -36,  -188,  -188,  -188,  -188,  -188,  -188,
    -188,  -188,  -188,    89,   -34,  -188,  -188,  -188,     0
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
       0,     3,     4,    16,    31,    46,   132,   133,    47,    32,
      33,   190,    64,    34,    76,    35,    92,   128,   167,   168,
     185,   205,    36,    93,   170,   191,   192,   193,   237,   194,
     195,   196,   233,   235,   253,   197,    65,    66,    67,   207,
     239,    68,    69,    70,    71,    72,    73,   209,   210,   226,
     255,   227,   228,   229,   257,   259,   145,   112,   113,   114,
     146,   147,    83,    18,    40,    91,    19,    39,    89,    20,
      38,    84,   161,    85,    21,    74,    75,   220,   115
};

/* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule whose
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
      17,     5,    22,    88,    86,   137,   198,   138,   139,   140,
     134,   135,   142,   143,    98,    45,   154,     6,     7,     8,
       9,   225,    10,   142,   143,    94,   102,   125,   144,    77,
     230,    23,   218,   103,   208,   104,   126,    87,    90,   231,
     131,   110,    95,   225,   224,   120,   111,   217,   121,    11,
     105,    12,    43,    14,    15,    13,   123,   106,    14,    15,
      37,   124,   212,   214,    14,    15,   224,   213,   215,   148,
     148,    41,   148,   148,    42,   244,    79,    80,    81,    82,
     245,    10,   152,   153,   246,     1,     2,   164,   199,   247,
      44,    45,   129,    78,     6,     7,     8,     9,    96,    10,
      95,    97,    98,   149,   107,    99,   149,   149,    11,   100,
      12,   109,   101,   108,   116,   130,   136,    14,    15,   117,
     232,   163,   234,   165,   118,   119,    11,    86,    12,   122,
     171,   110,   155,   127,   156,    14,    15,   166,   169,   172,
     177,   173,   179,   174,   148,   180,    49,   175,   176,   184,
     181,   208,   256,   211,   258,   216,    50,    51,    52,    53,
      54,    55,    56,    57,    58,    59,    60,    61,    62,   186,
     178,   219,   187,   222,    13,   238,   240,   242,   243,   249,
     236,   223,    50,    51,    52,    53,    54,    55,    56,    57,
      58,    59,    60,    61,    62,   188,   251,   202,   262,   264,
      48,   201,    63,   189,   206,   241,   221,    50,    51,    52,
      53,    54,    55,    56,    57,    58,    59,    60,    61,    62,
     203,   248,   150,   182,   200,     0,    49,     0,   204,     0,
       0,   250,     0,   252,     0,   254,    50,    51,    52,    53,
      54,    55,    56,    57,    58,    59,    60,    61,    62,   141,
     183,   260,   151,   261,     0,   263,     0,   265,     0,     0,
       0,     0,     0,   157,   158,   159,   160,     0,     0,   162,
      24,    25,    26,    27,    28,    29,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    30
};

static const yytype_int16 yycheck[] =
{
       2,     1,     2,    39,    38,    97,   170,    99,   100,   101,
      94,    95,     3,     4,     7,     7,   108,     3,     4,     5,
       6,   208,     8,     3,     4,    14,    34,    33,     8,    29,
      33,     0,    39,    41,    41,    43,    42,    39,    40,    42,
      32,     3,    31,   230,   208,    33,     8,    40,    36,    35,
      34,    37,     7,    44,    45,    41,    33,    41,    44,    45,
      36,    38,    35,    35,    44,    45,   230,    40,    40,   103,
     104,     3,   106,   107,     3,    35,     3,     4,     5,     6,
      40,     8,   106,   107,    35,     9,    10,   123,   172,    40,
       7,     7,    92,    35,     3,     4,     5,     6,    34,     8,
      31,    35,     7,   103,    41,    35,   106,   107,    35,    35,
      37,     7,    35,    41,    32,    37,     7,    44,    45,    32,
     212,   123,   214,   125,    32,    32,    35,   161,    37,    32,
     130,     3,    36,    39,    44,    44,    45,     7,    37,    33,
      42,    36,    42,    36,   178,    42,     7,    36,    36,    33,
      42,    41,   244,    30,   246,    40,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,   169,
      44,    30,     7,    33,    41,     7,     7,    40,     7,    36,
     216,    42,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    36,     7,    36,    36,
      27,   184,    28,    38,   185,   222,   206,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,   230,   104,   156,   178,    -1,     7,    -1,    38,    -1,
      -1,   233,    -1,   235,    -1,   237,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,   102,
     161,   253,   105,   255,    -1,   257,    -1,   259,    -1,    -1,
      -1,    -1,    -1,   116,   117,   118,   119,    -1,    -1,   122,
      11,    12,    13,    14,    15,    16,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    30
};

/* YYSTOS[STATE-NUM] -- The symbol kind of the accessing symbol of
   state STATE-NUM.  */
static const yytype_int8 yystos[] =
{
       0,     9,    10,    47,    48,   124,     3,     4,     5,     6,
       8,    35,    37,    41,    44,    45,    49,   108,   109,   112,
     115,   120,   124,     0,    11,    12,    13,    14,    15,    16,
      30,    50,    55,    56,    59,    61,    68,    36,   116,   113,
     110,     3,     3,     7,     7,     7,    51,    54,    51,     7,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,    57,    58,    82,    83,    84,    87,    88,
      89,    90,    91,    92,   121,   122,    60,   124,    35,     3,
       4,     5,     6,   108,   117,   119,   120,   108,   109,   114,
     108,   111,    62,    69,    14,    31,    34,    35,     7,    35,
      35,    35,    34,    41,    43,    34,    41,    41,    41,     7,
       3,     8,   103,   104,   105,   124,    32,    32,    32,    32,
      33,    36,    32,    33,    38,    33,    42,    39,    63,   124,
      37,    32,    52,    53,    54,    54,     7,   103,   103,   103,
     103,   105,     3,     4,     8,   102,   106,   107,   120,   124,
     106,   105,   102,   102,   103,    36,    44,   105,   105,   105,
     105,   118,   105,   108,   109,   108,     7,    64,    65,    37,
      70,   124,    33,    36,    36,    36,    36,    42,    44,    42,
      42,    42,   104,   119,    33,    66,   124,     7,    30,    38,
      57,    71,    72,    73,    75,    76,    77,    81,    82,    54,
     107,    65,     7,    30,    38,    67,    72,    85,    41,    93,
      94,    30,    35,    40,    35,    40,    40,    40,    39,    30,
     123,   124,    33,    42,    82,    83,    95,    97,    98,    99,
      33,    42,   103,    78,   103,    79,   109,    74,     7,    86,
       7,    86,    40,     7,    35,    40,    35,    40,    95,    36,
     108,    36,   108,    80,   108,    96,   103,   100,   103,   101,
     108,   108,    36,   108,    36,   108
};

/* YYR1[RULE-NUM] -- Symbol kind of the left-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr1[] =
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
     102,   102,   102,   103,   103,   103,   104,   104,   105,   106,
     106,   106,   107,   107,   108,   108,   108,   108,   108,   108,
     108,   108,   110,   109,   111,   111,   113,   112,   114,   114,
     114,   114,   115,   116,   115,   117,   118,   117,   119,   119,
     119,   119,   119,   119,   120,   120,   121,   121,   122,   122,
     122,   122,   122,   122,   122,   122,   122,   122,   122,   123,
     123,   124
};

/* YYR2[RULE-NUM] -- Number of symbols on the right-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr2[] =
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
       1,     1,     3,     1,     1,     3,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     0,     4,     1,     3,     0,     4,     1,     1,
       3,     3,     2,     0,     4,     1,     0,     4,     1,     3,
       3,     3,     3,     3,     2,     2,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       2,     0
};


enum { YYENOMEM = -2 };

#define yyerrok         (yyerrstatus = 0)
#define yyclearin       (yychar = YYEMPTY)

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab
#define YYNOMEM         goto yyexhaustedlab


#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)                                    \
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

/* Backward compatibility with an undocumented macro.
   Use YYerror or YYUNDEF. */
#define YYERRCODE YYUNDEF


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




# define YY_SYMBOL_PRINT(Title, Kind, Value, Location)                    \
do {                                                                      \
  if (yydebug)                                                            \
    {                                                                     \
      YYFPRINTF (stderr, "%s ", Title);                                   \
      yy_symbol_print (stderr,                                            \
                  Kind, Value); \
      YYFPRINTF (stderr, "\n");                                           \
    }                                                                     \
} while (0)


/*-----------------------------------.
| Print this symbol's value on YYO.  |
`-----------------------------------*/

static void
yy_symbol_value_print (FILE *yyo,
                       yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep)
{
  FILE *yyoutput = yyo;
  YY_USE (yyoutput);
  if (!yyvaluep)
    return;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YY_USE (yykind);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}


/*---------------------------.
| Print this symbol on YYO.  |
`---------------------------*/

static void
yy_symbol_print (FILE *yyo,
                 yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep)
{
  YYFPRINTF (yyo, "%s %s (",
             yykind < YYNTOKENS ? "token" : "nterm", yysymbol_name (yykind));

  yy_symbol_value_print (yyo, yykind, yyvaluep);
  YYFPRINTF (yyo, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

static void
yy_stack_print (yy_state_t *yybottom, yy_state_t *yytop)
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
yy_reduce_print (yy_state_t *yyssp, YYSTYPE *yyvsp,
                 int yyrule)
{
  int yylno = yyrline[yyrule];
  int yynrhs = yyr2[yyrule];
  int yyi;
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %d):\n",
             yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr,
                       YY_ACCESSING_SYMBOL (+yyssp[yyi + 1 - yynrhs]),
                       &yyvsp[(yyi + 1) - (yynrhs)]);
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
# define YYDPRINTF(Args) ((void) 0)
# define YY_SYMBOL_PRINT(Title, Kind, Value, Location)
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


/* Context of a parse error.  */
typedef struct
{
  yy_state_t *yyssp;
  yysymbol_kind_t yytoken;
} yypcontext_t;

/* Put in YYARG at most YYARGN of the expected tokens given the
   current YYCTX, and return the number of tokens stored in YYARG.  If
   YYARG is null, return the number of expected tokens (guaranteed to
   be less than YYNTOKENS).  Return YYENOMEM on memory exhaustion.
   Return 0 if there are more than YYARGN expected tokens, yet fill
   YYARG up to YYARGN. */
static int
yypcontext_expected_tokens (const yypcontext_t *yyctx,
                            yysymbol_kind_t yyarg[], int yyargn)
{
  /* Actual size of YYARG. */
  int yycount = 0;
  int yyn = yypact[+*yyctx->yyssp];
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
        if (yycheck[yyx + yyn] == yyx && yyx != YYSYMBOL_YYerror
            && !yytable_value_is_error (yytable[yyx + yyn]))
          {
            if (!yyarg)
              ++yycount;
            else if (yycount == yyargn)
              return 0;
            else
              yyarg[yycount++] = YY_CAST (yysymbol_kind_t, yyx);
          }
    }
  if (yyarg && yycount == 0 && 0 < yyargn)
    yyarg[0] = YYSYMBOL_YYEMPTY;
  return yycount;
}




#ifndef yystrlen
# if defined __GLIBC__ && defined _STRING_H
#  define yystrlen(S) (YY_CAST (YYPTRDIFF_T, strlen (S)))
# else
/* Return the length of YYSTR.  */
static YYPTRDIFF_T
yystrlen (const char *yystr)
{
  YYPTRDIFF_T yylen;
  for (yylen = 0; yystr[yylen]; yylen++)
    continue;
  return yylen;
}
# endif
#endif

#ifndef yystpcpy
# if defined __GLIBC__ && defined _STRING_H && defined _GNU_SOURCE
#  define yystpcpy stpcpy
# else
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
# endif
#endif

#ifndef yytnamerr
/* Copy to YYRES the contents of YYSTR after stripping away unnecessary
   quotes and backslashes, so that it's suitable for yyerror.  The
   heuristic is that double-quoting is unnecessary unless the string
   contains an apostrophe, a comma, or backslash (other than
   backslash-backslash).  YYSTR is taken from yytname.  If YYRES is
   null, do not copy; instead, return the length of what the result
   would have been.  */
static YYPTRDIFF_T
yytnamerr (char *yyres, const char *yystr)
{
  if (*yystr == '"')
    {
      YYPTRDIFF_T yyn = 0;
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
            else
              goto append;

          append:
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

  if (yyres)
    return yystpcpy (yyres, yystr) - yyres;
  else
    return yystrlen (yystr);
}
#endif


static int
yy_syntax_error_arguments (const yypcontext_t *yyctx,
                           yysymbol_kind_t yyarg[], int yyargn)
{
  /* Actual size of YYARG. */
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
  if (yyctx->yytoken != YYSYMBOL_YYEMPTY)
    {
      int yyn;
      if (yyarg)
        yyarg[yycount] = yyctx->yytoken;
      ++yycount;
      yyn = yypcontext_expected_tokens (yyctx,
                                        yyarg ? yyarg + 1 : yyarg, yyargn - 1);
      if (yyn == YYENOMEM)
        return YYENOMEM;
      else
        yycount += yyn;
    }
  return yycount;
}

/* Copy into *YYMSG, which is of size *YYMSG_ALLOC, an error message
   about the unexpected token YYTOKEN for the state stack whose top is
   YYSSP.

   Return 0 if *YYMSG was successfully written.  Return -1 if *YYMSG is
   not large enough to hold the message.  In that case, also set
   *YYMSG_ALLOC to the required number of bytes.  Return YYENOMEM if the
   required number of bytes is too large to store.  */
static int
yysyntax_error (YYPTRDIFF_T *yymsg_alloc, char **yymsg,
                const yypcontext_t *yyctx)
{
  enum { YYARGS_MAX = 5 };
  /* Internationalized format string. */
  const char *yyformat = YY_NULLPTR;
  /* Arguments of yyformat: reported tokens (one for the "unexpected",
     one per "expected"). */
  yysymbol_kind_t yyarg[YYARGS_MAX];
  /* Cumulated lengths of YYARG.  */
  YYPTRDIFF_T yysize = 0;

  /* Actual size of YYARG. */
  int yycount = yy_syntax_error_arguments (yyctx, yyarg, YYARGS_MAX);
  if (yycount == YYENOMEM)
    return YYENOMEM;

  switch (yycount)
    {
#define YYCASE_(N, S)                       \
      case N:                               \
        yyformat = S;                       \
        break
    default: /* Avoid compiler warnings. */
      YYCASE_(0, YY_("syntax error"));
      YYCASE_(1, YY_("syntax error, unexpected %s"));
      YYCASE_(2, YY_("syntax error, unexpected %s, expecting %s"));
      YYCASE_(3, YY_("syntax error, unexpected %s, expecting %s or %s"));
      YYCASE_(4, YY_("syntax error, unexpected %s, expecting %s or %s or %s"));
      YYCASE_(5, YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s"));
#undef YYCASE_
    }

  /* Compute error message size.  Don't count the "%s"s, but reserve
     room for the terminator.  */
  yysize = yystrlen (yyformat) - 2 * yycount + 1;
  {
    int yyi;
    for (yyi = 0; yyi < yycount; ++yyi)
      {
        YYPTRDIFF_T yysize1
          = yysize + yytnamerr (YY_NULLPTR, yytname[yyarg[yyi]]);
        if (yysize <= yysize1 && yysize1 <= YYSTACK_ALLOC_MAXIMUM)
          yysize = yysize1;
        else
          return YYENOMEM;
      }
  }

  if (*yymsg_alloc < yysize)
    {
      *yymsg_alloc = 2 * yysize;
      if (! (yysize <= *yymsg_alloc
             && *yymsg_alloc <= YYSTACK_ALLOC_MAXIMUM))
        *yymsg_alloc = YYSTACK_ALLOC_MAXIMUM;
      return -1;
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
          yyp += yytnamerr (yyp, yytname[yyarg[yyi++]]);
          yyformat += 2;
        }
      else
        {
          ++yyp;
          ++yyformat;
        }
  }
  return 0;
}


/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

static void
yydestruct (const char *yymsg,
            yysymbol_kind_t yykind, YYSTYPE *yyvaluep)
{
  YY_USE (yyvaluep);
  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yykind, yyvaluep, yylocationp);

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YY_USE (yykind);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}


/* Lookahead token kind.  */
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
    yy_state_fast_t yystate = 0;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus = 0;

    /* Refer to the stacks through separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* Their size.  */
    YYPTRDIFF_T yystacksize = YYINITDEPTH;

    /* The state stack: array, bottom, top.  */
    yy_state_t yyssa[YYINITDEPTH];
    yy_state_t *yyss = yyssa;
    yy_state_t *yyssp = yyss;

    /* The semantic value stack: array, bottom, top.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs = yyvsa;
    YYSTYPE *yyvsp = yyvs;

  int yyn;
  /* The return value of yyparse.  */
  int yyresult;
  /* Lookahead symbol kind.  */
  yysymbol_kind_t yytoken = YYSYMBOL_YYEMPTY;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;

  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYPTRDIFF_T yymsg_alloc = sizeof yymsgbuf;

#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yychar = YYEMPTY; /* Cause a token to be read.  */

  goto yysetstate;


/*------------------------------------------------------------.
| yynewstate -- push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;


/*--------------------------------------------------------------------.
| yysetstate -- set current state (the top of the stack) to yystate.  |
`--------------------------------------------------------------------*/
yysetstate:
  YYDPRINTF ((stderr, "Entering state %d\n", yystate));
  YY_ASSERT (0 <= yystate && yystate < YYNSTATES);
  YY_IGNORE_USELESS_CAST_BEGIN
  *yyssp = YY_CAST (yy_state_t, yystate);
  YY_IGNORE_USELESS_CAST_END
  YY_STACK_PRINT (yyss, yyssp);

  if (yyss + yystacksize - 1 <= yyssp)
#if !defined yyoverflow && !defined YYSTACK_RELOCATE
    YYNOMEM;
#else
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYPTRDIFF_T yysize = yyssp - yyss + 1;

# if defined yyoverflow
      {
        /* Give user a chance to reallocate the stack.  Use copies of
           these so that the &'s don't force the real ones into
           memory.  */
        yy_state_t *yyss1 = yyss;
        YYSTYPE *yyvs1 = yyvs;

        /* Each stack pointer address is followed by the size of the
           data in use in that stack, in bytes.  This used to be a
           conditional around just the two extra args, but that might
           be undefined if yyoverflow is a macro.  */
        yyoverflow (YY_("memory exhausted"),
                    &yyss1, yysize * YYSIZEOF (*yyssp),
                    &yyvs1, yysize * YYSIZEOF (*yyvsp),
                    &yystacksize);
        yyss = yyss1;
        yyvs = yyvs1;
      }
# else /* defined YYSTACK_RELOCATE */
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
        YYNOMEM;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
        yystacksize = YYMAXDEPTH;

      {
        yy_state_t *yyss1 = yyss;
        union yyalloc *yyptr =
          YY_CAST (union yyalloc *,
                   YYSTACK_ALLOC (YY_CAST (YYSIZE_T, YYSTACK_BYTES (yystacksize))));
        if (! yyptr)
          YYNOMEM;
        YYSTACK_RELOCATE (yyss_alloc, yyss);
        YYSTACK_RELOCATE (yyvs_alloc, yyvs);
#  undef YYSTACK_RELOCATE
        if (yyss1 != yyssa)
          YYSTACK_FREE (yyss1);
      }
# endif

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;

      YY_IGNORE_USELESS_CAST_BEGIN
      YYDPRINTF ((stderr, "Stack size increased to %ld\n",
                  YY_CAST (long, yystacksize)));
      YY_IGNORE_USELESS_CAST_END

      if (yyss + yystacksize - 1 <= yyssp)
        YYABORT;
    }
#endif /* !defined yyoverflow && !defined YYSTACK_RELOCATE */


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

  /* YYCHAR is either empty, or end-of-input, or a valid lookahead.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token\n"));
      yychar = yylex ();
    }

  if (yychar <= YYEOF)
    {
      yychar = YYEOF;
      yytoken = YYSYMBOL_YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else if (yychar == YYerror)
    {
      /* The scanner already issued an error message, process directly
         to error recovery.  But do not keep the error token as
         lookahead, it is too special and may lead us to an endless
         loop in error recovery. */
      yychar = YYUNDEF;
      yytoken = YYSYMBOL_YYerror;
      goto yyerrlab1;
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
  yystate = yyn;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END

  /* Discard the shifted token.  */
  yychar = YYEMPTY;
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
| yyreduce -- do a reduction.  |
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
  case 9: /* file: file typedef  */
#line 241 "parser.ypp"
        {

	}
#line 1790 "parser.cpp"
    break;

  case 11: /* value: empty  */
#line 249 "parser.ypp"
        {
		parsed_value->clear();
	}
#line 1798 "parser.cpp"
    break;

  case 12: /* value: type_value  */
#line 253 "parser.ypp"
        {
		parsed_value->assign((yyvsp[0].str));
		if(!type_stack.empty()) depth_error(0, "type");
	}
#line 1807 "parser.cpp"
    break;

  case 13: /* value: method_value  */
#line 258 "parser.ypp"
        {
		parsed_value->assign((yyvsp[0].str));
		if(!type_stack.empty()) depth_error(0, "method");
	}
#line 1816 "parser.cpp"
    break;

  case 14: /* import: KW_IMPORT import_module  */
#line 266 "parser.ypp"
        {
		Import* import = new Import((yyvsp[0].str));
		parsed_file->add_import(import);
	}
#line 1825 "parser.cpp"
    break;

  case 15: /* import: KW_FROM import_module KW_IMPORT import_symbols  */
#line 271 "parser.ypp"
        {
		Import* import = new Import((yyvsp[-2].str));
		import->symbols.assign((yyvsp[0].strings).begin(), (yyvsp[0].strings).end());
		parsed_file->add_import(import);
	}
#line 1835 "parser.cpp"
    break;

  case 16: /* import_module: import_alternatives  */
#line 279 "parser.ypp"
                              { (yyval.str) = (yyvsp[0].str); }
#line 1841 "parser.cpp"
    break;

  case 17: /* import_module: import_module '.' import_alternatives  */
#line 281 "parser.ypp"
        {
		(yyval.str) = (yyvsp[-2].str) + string(".") + (yyvsp[0].str);
	}
#line 1849 "parser.cpp"
    break;

  case 18: /* import_symbols: import_symbol_list  */
#line 287 "parser.ypp"
                             { (yyval.strings) = (yyvsp[0].strings); }
#line 1855 "parser.cpp"
    break;

  case 19: /* import_symbols: '*'  */
#line 289 "parser.ypp"
        {
		(yyval.strings) = vector<string>();
	}
#line 1863 "parser.cpp"
    break;

  case 20: /* import_symbol_list: import_alternatives  */
#line 296 "parser.ypp"
        {
		(yyval.strings) = vector<string>(1, (yyvsp[0].str));
	}
#line 1871 "parser.cpp"
    break;

  case 21: /* import_symbol_list: import_symbol_list ',' import_alternatives  */
#line 300 "parser.ypp"
        {
		(yyvsp[-2].strings).push_back((yyvsp[0].str));
		(yyval.strings) = (yyvsp[-2].strings);
	}
#line 1880 "parser.cpp"
    break;

  case 22: /* import_alternatives: IDENTIFIER  */
#line 307 "parser.ypp"
                     { (yyval.str) = (yyvsp[0].str); }
#line 1886 "parser.cpp"
    break;

  case 23: /* import_alternatives: import_alternatives '/' IDENTIFIER  */
#line 309 "parser.ypp"
        {
		(yyval.str) = (yyvsp[-2].str) + string("/") + (yyvsp[0].str);
	}
#line 1894 "parser.cpp"
    break;

  case 24: /* typedef: typedef_type  */
#line 316 "parser.ypp"
        {
		if((yyvsp[0].nametype).type == nullptr)
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
			if(dtype == nullptr)
			{
				parser_error("Unknown error adding typedef to file.");
				break;
			}

			Struct* dstruct = dtype->as_struct();
			if(dstruct == nullptr)
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
#line 1940 "parser.cpp"
    break;

  case 25: /* typedef_type: KW_TYPEDEF nonmethod_type_with_name  */
#line 361 "parser.ypp"
        {
		(yyval.nametype) = (yyvsp[0].nametype);
	}
#line 1948 "parser.cpp"
    break;

  case 26: /* typedef_type: typedef_type '[' array_range ']'  */
#line 365 "parser.ypp"
        {
		(yyval.nametype) = (yyvsp[-3].nametype);
		(yyval.nametype).type = new ArrayType((yyvsp[-3].nametype).type, (yyvsp[-1].range));
	}
#line 1957 "parser.cpp"
    break;

  case 27: /* nonmethod_type_with_name: nonmethod_type IDENTIFIER  */
#line 373 "parser.ypp"
        {
		TokenType::NameType nt;
		nt.type = (yyvsp[-1].u.dtype);
		nt.name = (yyvsp[0].str);
		(yyval.nametype) = nt;
	}
#line 1968 "parser.cpp"
    break;

  case 28: /* defined_type: IDENTIFIER  */
#line 383 "parser.ypp"
        {
		DistributedType* dtype = parsed_file->get_type_by_name((yyvsp[0].str));
		if(dtype == nullptr)
		{
			parser_error("Type '" + string((yyvsp[0].str)) + "' has not been declared.");
			(yyval.u.dtype) = nullptr;
			break;
		}

		(yyval.u.dtype) = dtype;
	}
#line 1984 "parser.cpp"
    break;

  case 31: /* keyword_decl_list: keyword_decl_list IDENTIFIER  */
#line 403 "parser.ypp"
        {
		parsed_file->add_keyword((yyvsp[0].str));
	}
#line 1992 "parser.cpp"
    break;

  case 32: /* $@1: %empty  */
#line 410 "parser.ypp"
        {
		current_class = new Class(parsed_file, (yyvsp[0].str));
	}
#line 2000 "parser.cpp"
    break;

  case 33: /* dclass: KW_DCLASS IDENTIFIER $@1 class_inheritance '{' class_fields '}'  */
#line 414 "parser.ypp"
        {
		bool class_added = parsed_file->add_class(current_class);
		if(!class_added)
		{
			// Lets be really descriptive about why this failed
			DistributedType* dtype = parsed_file->get_type_by_name(current_class->get_name());
			if(dtype == nullptr)
			{
				parser_error("Unknown error adding class to file.");
				break;
			}

			Struct* dstruct = dtype->as_struct();
			if(dstruct == nullptr)
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
#line 2037 "parser.cpp"
    break;

  case 36: /* class_parents: defined_class  */
#line 455 "parser.ypp"
        {
		if((yyvsp[0].u.dclass) != nullptr)
		{
			current_class->add_parent((yyvsp[0].u.dclass));
		}
	}
#line 2048 "parser.cpp"
    break;

  case 37: /* class_parents: class_parents ',' defined_class  */
#line 462 "parser.ypp"
        {
		if((yyvsp[0].u.dclass) != nullptr)
		{
			current_class->add_parent((yyvsp[0].u.dclass));
		}
	}
#line 2059 "parser.cpp"
    break;

  case 38: /* defined_class: IDENTIFIER  */
#line 472 "parser.ypp"
        {
		DistributedType* dtype = parsed_file->get_type_by_name((yyvsp[0].str));
		if(dtype == nullptr)
		{
			parser_error("'dclass " + string((yyvsp[0].str)) + "' has not been declared.");
			(yyval.u.dclass) = nullptr;
			break;
		}

		Struct* dstruct = dtype->as_struct();
		if(dstruct == nullptr)
		{
			parser_error("class cannot inherit from non-class type '" + string((yyvsp[0].str)) + "'.");
			(yyval.u.dclass) = nullptr;
			break;
		}

		Class* dclass = dstruct->as_class();
		if(dclass == nullptr)
		{
			parser_error("class cannot inherit from struct type '" + string((yyvsp[0].str)) + "'.");
			(yyval.u.dclass) = nullptr;
			break;
		}

		(yyval.u.dclass) = dclass;
	}
#line 2091 "parser.cpp"
    break;

  case 41: /* class_fields: class_fields class_field ';'  */
#line 505 "parser.ypp"
        {
		if((yyvsp[-1].u.dfield) == nullptr)
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
#line 2130 "parser.cpp"
    break;

  case 42: /* class_field: named_field keyword_list  */
#line 543 "parser.ypp"
        {
		if((yyvsp[-1].u.dfield) == nullptr)
		{
			// Ignore this field, it should have already generated a parser error
			(yyval.u.dfield) = nullptr;
			break;
		}

		if((yyvsp[-1].u.dfield)->get_name().empty())
		{
			parser_error("An unnamed field can't be defined in a class.");
			(yyval.u.dfield) = nullptr;
			break;
		}

		// Add the keywords to the class
		for(auto it = (yyvsp[0].strings).begin(); it != (yyvsp[0].strings).end(); ++it)
		{
			(yyvsp[-1].u.dfield)->add_keyword(*it);
		}

		(yyval.u.dfield) = (yyvsp[-1].u.dfield);
	}
#line 2158 "parser.cpp"
    break;

  case 43: /* class_field: molecular  */
#line 567 "parser.ypp"
        {
		(yyval.u.dfield) = (Field*)(yyvsp[0].u.dmolecule);
	}
#line 2166 "parser.cpp"
    break;

  case 44: /* $@2: %empty  */
#line 574 "parser.ypp"
        {
		current_struct = new Struct(parsed_file, (yyvsp[0].str));
	}
#line 2174 "parser.cpp"
    break;

  case 45: /* dstruct: KW_STRUCT IDENTIFIER $@2 '{' struct_fields '}'  */
#line 578 "parser.ypp"
        {
		bool struct_added = parsed_file->add_struct(current_struct);
		if(!struct_added)
		{
			// Lets be really descriptive about why this failed
			DistributedType* dtype = parsed_file->get_type_by_name(current_struct->get_name());
			if(dtype == nullptr)
			{
				parser_error("Unknown error adding struct to file.");
				break;
			}

			Struct* dstruct = dtype->as_struct();
			if(dstruct == nullptr)
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
#line 2211 "parser.cpp"
    break;

  case 48: /* struct_fields: struct_fields struct_field ';'  */
#line 616 "parser.ypp"
        {
		if((yyvsp[-1].u.dfield) == nullptr || (yyvsp[-1].u.dfield)->get_type() == nullptr)
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
#line 2246 "parser.cpp"
    break;

  case 55: /* unnamed_field: nonmethod_type  */
#line 662 "parser.ypp"
        {
		(yyval.u.dfield) = new Field((yyvsp[0].u.dtype));
	}
#line 2254 "parser.cpp"
    break;

  case 56: /* $@3: %empty  */
#line 666 "parser.ypp"
        {
		current_depth = 0;
		type_stack.push(TypeAndDepth((yyvsp[-1].u.dtype), 0));
	}
#line 2263 "parser.cpp"
    break;

  case 57: /* unnamed_field: nonmethod_type '=' $@3 type_value  */
#line 671 "parser.ypp"
        {
		Field* field = new Field((yyvsp[-3].u.dtype));
		if(!type_stack.empty()) depth_error(0, "unnamed field");
		field->set_default_value((yyvsp[0].str));
		(yyval.u.dfield) = field;
	}
#line 2274 "parser.cpp"
    break;

  case 58: /* field_with_name: nonmethod_type_with_name  */
#line 681 "parser.ypp"
        {
		(yyval.u.dfield) = new Field((yyvsp[0].nametype).type, (yyvsp[0].nametype).name);
	}
#line 2282 "parser.cpp"
    break;

  case 59: /* field_with_name_as_array: field_with_name '[' array_range ']'  */
#line 688 "parser.ypp"
        {
		(yyvsp[-3].u.dfield)->set_type(new ArrayType((yyvsp[-3].u.dfield)->get_type(), (yyvsp[-1].range)));
		(yyval.u.dfield) = (yyvsp[-3].u.dfield);
	}
#line 2291 "parser.cpp"
    break;

  case 60: /* field_with_name_as_array: field_with_name_as_array '[' array_range ']'  */
#line 693 "parser.ypp"
        {
		(yyvsp[-3].u.dfield)->set_type(new ArrayType((yyvsp[-3].u.dfield)->get_type(), (yyvsp[-1].range)));
		(yyval.u.dfield) = (yyvsp[-3].u.dfield);
	}
#line 2300 "parser.cpp"
    break;

  case 61: /* $@4: %empty  */
#line 701 "parser.ypp"
        {
		current_depth = 0;
		type_stack.push(TypeAndDepth((yyvsp[-1].u.dfield)->get_type(), 0));
	}
#line 2309 "parser.cpp"
    break;

  case 62: /* field_with_name_and_default: field_with_name '=' $@4 type_value  */
#line 706 "parser.ypp"
        {
		if(!type_stack.empty()) depth_error(0, "field '" + (yyvsp[-3].u.dfield)->get_name() + "'");
		(yyvsp[-3].u.dfield)->set_default_value((yyvsp[0].str));
		(yyval.u.dfield) = (yyvsp[-3].u.dfield);
	}
#line 2319 "parser.cpp"
    break;

  case 63: /* $@5: %empty  */
#line 712 "parser.ypp"
        {
		current_depth = 0;
		type_stack.push(TypeAndDepth((yyvsp[-1].u.dfield)->get_type(), 0));
	}
#line 2328 "parser.cpp"
    break;

  case 64: /* field_with_name_and_default: field_with_name_as_array '=' $@5 type_value  */
#line 717 "parser.ypp"
        {
		if(!type_stack.empty()) depth_error(0, "field '" + (yyvsp[-3].u.dfield)->get_name() + "'");
		(yyvsp[-3].u.dfield)->set_default_value((yyvsp[0].str));
		(yyval.u.dfield) = (yyvsp[-3].u.dfield);
	}
#line 2338 "parser.cpp"
    break;

  case 65: /* $@6: %empty  */
#line 723 "parser.ypp"
        {
		current_depth = 0;
		type_stack.push(TypeAndDepth((yyvsp[-2].u.dfield)->get_type(), 0));
	}
#line 2347 "parser.cpp"
    break;

  case 66: /* field_with_name_and_default: method_as_field '=' method_value $@6 type_value  */
#line 728 "parser.ypp"
        {
		if(!type_stack.empty()) depth_error(0, "method");
		(yyvsp[-4].u.dfield)->set_default_value((yyvsp[-2].str));
		(yyval.u.dfield) = (yyvsp[-4].u.dfield);
	}
#line 2357 "parser.cpp"
    break;

  case 67: /* method_as_field: IDENTIFIER method  */
#line 737 "parser.ypp"
        {
		(yyval.u.dfield) = new Field((yyvsp[0].u.dmethod), (yyvsp[-1].str));
	}
#line 2365 "parser.cpp"
    break;

  case 70: /* nonmethod_type_no_array: defined_type  */
#line 749 "parser.ypp"
        {
		if((yyvsp[0].u.dtype) == nullptr)
		{
			// defined_type should have output an error, pass nullptr upstream
			(yyval.u.dtype) = nullptr;
			break;
		}

		if((yyvsp[0].u.dtype)->get_type() == T_METHOD)
		{
			parser_error("Cannot use a method type here.");
			(yyval.u.dtype) = nullptr;
			break;
		}

		(yyval.u.dtype) = (yyvsp[0].u.dtype);
	}
#line 2387 "parser.cpp"
    break;

  case 71: /* nonmethod_type_no_array: numeric_type  */
#line 767 "parser.ypp"
        {
		(yyval.u.dtype) = (DistributedType*)(yyvsp[0].u.dnumeric);
	}
#line 2395 "parser.cpp"
    break;

  case 73: /* type_with_array: numeric_type '[' array_range ']'  */
#line 775 "parser.ypp"
        {
		(yyval.u.dtype) = new ArrayType((yyvsp[-3].u.dnumeric), (yyvsp[-1].range));
	}
#line 2403 "parser.cpp"
    break;

  case 74: /* type_with_array: defined_type '[' array_range ']'  */
#line 779 "parser.ypp"
        {
		(yyval.u.dtype) = new ArrayType((yyvsp[-3].u.dtype), (yyvsp[-1].range));
	}
#line 2411 "parser.cpp"
    break;

  case 75: /* type_with_array: builtin_array_type '[' array_range ']'  */
#line 783 "parser.ypp"
        {
		(yyval.u.dtype) = new ArrayType((yyvsp[-3].u.dtype), (yyvsp[-1].range));
	}
#line 2419 "parser.cpp"
    break;

  case 76: /* type_with_array: type_with_array '[' array_range ']'  */
#line 787 "parser.ypp"
        {
		(yyval.u.dtype) = new ArrayType((yyvsp[-3].u.dtype), (yyvsp[-1].range));
	}
#line 2427 "parser.cpp"
    break;

  case 77: /* molecular: IDENTIFIER ':' defined_field  */
#line 794 "parser.ypp"
        {
		MolecularField* mol = new MolecularField(current_class, (yyvsp[-2].str));
		if((yyvsp[0].u.dfield) == nullptr)
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
#line 2457 "parser.cpp"
    break;

  case 78: /* molecular: molecular ',' defined_field  */
#line 820 "parser.ypp"
        {
		if((yyvsp[0].u.dfield) == nullptr)
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
#line 2491 "parser.cpp"
    break;

  case 79: /* defined_field: IDENTIFIER  */
#line 852 "parser.ypp"
        {
		if(!current_class)
		{
			parser_error("Field '" + (yyvsp[0].str) + "' not defined in current class.");
			(yyval.u.dfield) = nullptr;
			break;
		}

		Field *field = current_class->get_field_by_name((yyvsp[0].str));
		if(field == nullptr)
		{
			parser_error("Field '" + (yyvsp[0].str) + "' not defined in current class.");
			(yyval.u.dfield) = nullptr;
			break;
		}

		(yyval.u.dfield) = field;
	}
#line 2514 "parser.cpp"
    break;

  case 80: /* builtin_array_type: array_type_token  */
#line 874 "parser.ypp"
        {
		if((yyvsp[0].u.type) == T_STRING)
		{
			if(basic_string == nullptr)
			{
				basic_string = new ArrayType(new NumericType(T_CHAR));
				basic_string->set_alias("string");
			}

			(yyval.u.dtype) = basic_string;
		}
		else if((yyvsp[0].u.type) == T_BLOB)
		{
			if(basic_blob == nullptr)
			{
				basic_blob = new ArrayType(new NumericType(T_UINT8));
				basic_blob->set_alias("blob");
			}

			(yyval.u.dtype) = basic_blob;
		}
		else
		{
			parser_error("Found builtin ArrayType not handled by parser.");
			(yyval.u.dtype) = nullptr;
		}
	}
#line 2546 "parser.cpp"
    break;

  case 81: /* builtin_array_type: array_type_token '(' array_range ')'  */
#line 902 "parser.ypp"
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
			(yyval.u.dtype) = nullptr;
		}
	}
#line 2570 "parser.cpp"
    break;

  case 86: /* numeric_token_only: numeric_type_token  */
#line 931 "parser.ypp"
                             { (yyval.u.dnumeric) = new NumericType((yyvsp[0].u.type)); }
#line 2576 "parser.cpp"
    break;

  case 87: /* numeric_with_range: numeric_token_only '(' numeric_range ')'  */
#line 936 "parser.ypp"
        {
		if(!(yyvsp[-3].u.dnumeric)->set_range((yyvsp[-1].range)))
		{
			parser_error("Invalid range for type.");
		}

		(yyval.u.dnumeric) = (yyvsp[-3].u.dnumeric);
	}
#line 2589 "parser.cpp"
    break;

  case 88: /* numeric_with_range: numeric_with_modulus '(' numeric_range ')'  */
#line 945 "parser.ypp"
        {
		if(!(yyvsp[-3].u.dnumeric)->set_range((yyvsp[-1].range)))
		{
			parser_error("Invalid range for type.");
		}

		(yyval.u.dnumeric) = (yyvsp[-3].u.dnumeric);
	}
#line 2602 "parser.cpp"
    break;

  case 89: /* numeric_with_range: numeric_with_divisor '(' numeric_range ')'  */
#line 954 "parser.ypp"
        {
		if(!(yyvsp[-3].u.dnumeric)->set_range((yyvsp[-1].range)))
		{
			parser_error("Invalid range for type.");
		}

		(yyval.u.dnumeric) = (yyvsp[-3].u.dnumeric);
	}
#line 2615 "parser.cpp"
    break;

  case 90: /* numeric_with_modulus: numeric_token_only '%' number  */
#line 966 "parser.ypp"
        {
		if(!(yyvsp[-2].u.dnumeric)->set_modulus((yyvsp[0].u.real)))
		{
			parser_error("Invalid modulus for type.");
		}

		(yyval.u.dnumeric) = (yyvsp[-2].u.dnumeric);
	}
#line 2628 "parser.cpp"
    break;

  case 91: /* numeric_with_divisor: numeric_token_only '/' small_unsigned_integer  */
#line 978 "parser.ypp"
        {
		if(!(yyvsp[-2].u.dnumeric)->set_divisor((yyvsp[0].u.uint32)))
		{
			parser_error("Invalid divisor for type.");
		}
	}
#line 2639 "parser.cpp"
    break;

  case 92: /* numeric_with_divisor: numeric_with_modulus '/' small_unsigned_integer  */
#line 985 "parser.ypp"
        {
		if(!(yyvsp[-2].u.dnumeric)->set_divisor((yyvsp[0].u.uint32)))
		{
			parser_error("Invalid divisor for type.");
		}
	}
#line 2650 "parser.cpp"
    break;

  case 93: /* method: '(' ')'  */
#line 995 "parser.ypp"
        {
		(yyval.u.dmethod) = new Method();
	}
#line 2658 "parser.cpp"
    break;

  case 94: /* method: method_body ')'  */
#line 999 "parser.ypp"
        {
		(yyval.u.dmethod) = (yyvsp[-1].u.dmethod);
	}
#line 2666 "parser.cpp"
    break;

  case 95: /* method_body: '(' parameter  */
#line 1006 "parser.ypp"
        {
		Method* fn = new Method();
		bool param_added = fn->add_parameter((yyvsp[0].u.dparam));
		if(!param_added)
		{
			parser_error("Unknown error adding parameter to method.");
		}
		(yyval.u.dmethod) = fn;
	}
#line 2680 "parser.cpp"
    break;

  case 96: /* method_body: method_body ',' parameter  */
#line 1016 "parser.ypp"
        {
		bool param_added = (yyvsp[-2].u.dmethod)->add_parameter((yyvsp[0].u.dparam));
		if(!param_added)
		{
			parser_error("Cannot add parameter '" + (yyvsp[0].u.dparam)->get_name()
			             + "', a parameter with that name is already used in this method.");
		}
		(yyval.u.dmethod) = (yyvsp[-2].u.dmethod);
	}
#line 2694 "parser.cpp"
    break;

  case 100: /* parameter: nonmethod_type  */
#line 1032 "parser.ypp"
        {
		(yyval.u.dparam) = new Parameter((yyvsp[0].u.dtype));
	}
#line 2702 "parser.cpp"
    break;

  case 101: /* $@7: %empty  */
#line 1036 "parser.ypp"
        {
		current_depth = 0;
		type_stack.push(TypeAndDepth((yyvsp[-1].u.dtype),0));
	}
#line 2711 "parser.cpp"
    break;

  case 102: /* parameter: nonmethod_type '=' $@7 type_value  */
#line 1041 "parser.ypp"
        {
		Parameter* param = new Parameter((yyvsp[-3].u.dtype));
		if(!type_stack.empty()) depth_error(0, "type");
		param->set_default_value((yyvsp[0].str));
		(yyval.u.dparam) = param;
	}
#line 2722 "parser.cpp"
    break;

  case 103: /* param_with_name: nonmethod_type_no_array IDENTIFIER  */
#line 1050 "parser.ypp"
        {
		(yyval.u.dparam) = new Parameter((yyvsp[-1].u.dtype), (yyvsp[0].str));
	}
#line 2730 "parser.cpp"
    break;

  case 104: /* param_with_name_as_array: param_with_name '[' array_range ']'  */
#line 1057 "parser.ypp"
        {
		(yyvsp[-3].u.dparam)->set_type(new ArrayType((yyvsp[-3].u.dparam)->get_type(), (yyvsp[-1].range)));
		(yyval.u.dparam) = (yyvsp[-3].u.dparam);
	}
#line 2739 "parser.cpp"
    break;

  case 105: /* param_with_name_as_array: param_with_name_as_array '[' array_range ']'  */
#line 1062 "parser.ypp"
        {
		(yyvsp[-3].u.dparam)->set_type(new ArrayType((yyvsp[-3].u.dparam)->get_type(), (yyvsp[-1].range)));
		(yyval.u.dparam) = (yyvsp[-3].u.dparam);
	}
#line 2748 "parser.cpp"
    break;

  case 106: /* $@8: %empty  */
#line 1070 "parser.ypp"
        {
		current_depth = 0;
		type_stack.push(TypeAndDepth((yyvsp[-1].u.dparam)->get_type(), 0));
	}
#line 2757 "parser.cpp"
    break;

  case 107: /* param_with_name_and_default: param_with_name '=' $@8 type_value  */
#line 1075 "parser.ypp"
        {
		if(!type_stack.empty()) depth_error(0, "parameter '" + (yyvsp[-3].u.dparam)->get_name() + "'");
		(yyvsp[-3].u.dparam)->set_default_value((yyvsp[0].str));
		(yyval.u.dparam) = (yyvsp[-3].u.dparam);
	}
#line 2767 "parser.cpp"
    break;

  case 108: /* $@9: %empty  */
#line 1081 "parser.ypp"
        {
		current_depth = 0;
		type_stack.push(TypeAndDepth((yyvsp[-1].u.dparam)->get_type(), 0));
	}
#line 2776 "parser.cpp"
    break;

  case 109: /* param_with_name_and_default: param_with_name_as_array '=' $@9 type_value  */
#line 1086 "parser.ypp"
        {
		if(!type_stack.empty()) depth_error(0, "parameter '" + (yyvsp[-3].u.dparam)->get_name() + "'");
		(yyvsp[-3].u.dparam)->set_default_value((yyvsp[0].str));
		(yyval.u.dparam) = (yyvsp[-3].u.dparam);
	}
#line 2786 "parser.cpp"
    break;

  case 110: /* numeric_range: empty  */
#line 1094 "parser.ypp"
                { (yyval.range) = NumericRange(); }
#line 2792 "parser.cpp"
    break;

  case 111: /* numeric_range: char_or_number  */
#line 1095 "parser.ypp"
                         { (yyval.range) = NumericRange((yyvsp[0].u.real), (yyvsp[0].u.real)); }
#line 2798 "parser.cpp"
    break;

  case 112: /* numeric_range: char_or_number '-' char_or_number  */
#line 1096 "parser.ypp"
                                            { (yyval.range) = NumericRange((yyvsp[-2].u.real), (yyvsp[0].u.real)); }
#line 2804 "parser.cpp"
    break;

  case 113: /* array_range: empty  */
#line 1100 "parser.ypp"
                { (yyval.range) = NumericRange(); }
#line 2810 "parser.cpp"
    break;

  case 114: /* array_range: char_or_uint  */
#line 1101 "parser.ypp"
                       { (yyval.range) = NumericRange((yyvsp[0].u.uint32), (yyvsp[0].u.uint32)); }
#line 2816 "parser.cpp"
    break;

  case 115: /* array_range: char_or_uint '-' char_or_uint  */
#line 1102 "parser.ypp"
                                        { (yyval.range) = NumericRange((yyvsp[-2].u.uint32), (yyvsp[0].u.uint32)); }
#line 2822 "parser.cpp"
    break;

  case 116: /* char_or_uint: CHAR  */
#line 1107 "parser.ypp"
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
#line 2838 "parser.cpp"
    break;

  case 118: /* small_unsigned_integer: UNSIGNED_INTEGER  */
#line 1123 "parser.ypp"
        {
		unsigned int num = (unsigned int)(yyvsp[0].u.uint64);
		if(num != (yyvsp[0].u.uint64))
		{
			parser_error("Number out of range.");
			(yyval.u.uint32) = 1;
		}
		(yyval.u.uint32) = num;
	}
#line 2852 "parser.cpp"
    break;

  case 119: /* number: UNSIGNED_INTEGER  */
#line 1135 "parser.ypp"
                           { (yyval.u.real) = (double)(yyvsp[0].u.uint64); }
#line 2858 "parser.cpp"
    break;

  case 120: /* number: signed_integer  */
#line 1136 "parser.ypp"
                         { (yyval.u.real) = (double)(yyvsp[0].u.int64); }
#line 2864 "parser.cpp"
    break;

  case 122: /* char_or_number: CHAR  */
#line 1142 "parser.ypp"
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
#line 2880 "parser.cpp"
    break;

  case 124: /* type_value: signed_integer  */
#line 1158 "parser.ypp"
        {
		if(!check_depth()) depth_error("signed integer");

		const DistributedType* dtype = type_stack.top().type;
		if(dtype == nullptr)
		{
			// Ignore this field, it should have already generated an error
			(yyval.str) = "";
			break;
		}
		type_stack.pop(); // Remove numeric type from stack

		(yyval.str) = number_value(dtype->get_type(), (yyvsp[0].u.int64));
	}
#line 2899 "parser.cpp"
    break;

  case 125: /* type_value: UNSIGNED_INTEGER  */
#line 1173 "parser.ypp"
        {
		if(!check_depth()) depth_error("unsigned integer");

		const DistributedType* dtype = type_stack.top().type;
		if(dtype == nullptr)
		{
			// Ignore this field, it should have already generated an error
			(yyval.str) = "";
			break;
		}
		type_stack.pop(); // Remove numeric type from stack

		(yyval.str) = number_value(dtype->get_type(), (yyvsp[0].u.uint64));
	}
#line 2918 "parser.cpp"
    break;

  case 126: /* type_value: REAL  */
#line 1188 "parser.ypp"
        {
		if(!check_depth()) depth_error("floating point");

		const DistributedType* dtype = type_stack.top().type;
		if(dtype == nullptr)
		{
			// Ignore this field, it should have already generated an error
			(yyval.str) = "";
			break;
		}
		type_stack.pop(); // Remove numeric type from stack

		(yyval.str) = number_value(dtype->get_type(), (yyvsp[0].u.real));
	}
#line 2937 "parser.cpp"
    break;

  case 127: /* type_value: CHAR  */
#line 1203 "parser.ypp"
        {
		if(!check_depth()) depth_error("char");

		const DistributedType* dtype = type_stack.top().type;
		if(dtype == nullptr)
		{
			// Ignore this field, it should have already generated an error
			(yyval.str) = "";
			break;
		}
		type_stack.pop(); // Remove type from stack

		if(dtype->get_type() != T_CHAR) {
			parser_error("Cannot use char value for non-char type '"
			             + format_type(dtype->get_type()) + "'.");
		}

		if((yyvsp[0].str).length() != 1)
		{
			parser_error("Single character required.");
			(yyval.str) = "";
		}
		else
		{
			(yyval.str) = (yyvsp[0].str);
		}
	}
#line 2969 "parser.cpp"
    break;

  case 128: /* type_value: STRING  */
#line 1231 "parser.ypp"
        {
		if(!check_depth()) depth_error("string");

		const DistributedType* dtype = type_stack.top().type;
		if(dtype == nullptr)
		{
			// Ignore this field, it should have already generated an error
			(yyval.str) = "";
			break;
		}
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
			length = swap_le(length);
			(yyval.str) = string((char*)&length, sizeof(sizetag_t)) + (yyvsp[0].str);
		}
		else
		{
			parser_error("Cannot use string value for non-string type '"
			             + format_type(dtype->get_type()) + "'.");
			(yyval.str) = (yyvsp[0].str);
		}
	}
#line 3010 "parser.cpp"
    break;

  case 129: /* type_value: HEX_STRING  */
#line 1268 "parser.ypp"
        {
		if(!check_depth()) depth_error("hex-string");

		const DistributedType* dtype = type_stack.top().type;
		if(dtype == nullptr)
		{
			// Ignore this field, it should have already generated an error
			(yyval.str) = "";
			break;
		}
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
			(yyval.str) = (yyvsp[0].str);
		}
		else
		{
			parser_error("Cannot use hex value for non-blob type '"
			             + format_type(dtype->get_type()) + "'.");
			(yyval.str) = (yyvsp[0].str);
		}
	}
#line 3048 "parser.cpp"
    break;

  case 132: /* $@10: %empty  */
#line 1307 "parser.ypp"
        {
		if(!check_depth()) depth_error("method");

		const DistributedType* dtype = type_stack.top().type;
		if(dtype == nullptr)
		{
			// Ignore this field, it should have already generated an error
			break;
		}
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
#line 3082 "parser.cpp"
    break;

  case 133: /* method_value: '(' $@10 parameter_values ')'  */
#line 1337 "parser.ypp"
        {
		if(type_stack.top().type->as_method())
		{
			current_depth--;
		}
		type_stack.pop(); // Remove method type from the stack
		(yyval.str) = (yyvsp[-1].str);
	}
#line 3095 "parser.cpp"
    break;

  case 135: /* parameter_values: parameter_values ',' type_value  */
#line 1350 "parser.ypp"
        {
		(yyval.str) = (yyvsp[-2].str) + (yyvsp[0].str);
	}
#line 3103 "parser.cpp"
    break;

  case 136: /* $@11: %empty  */
#line 1357 "parser.ypp"
        {
		if(!check_depth()) depth_error("struct");

		const DistributedType* dtype = type_stack.top().type;
		if(dtype == nullptr)
		{
			// Ignore this field, it should have already generated an error
			break;
		}
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
#line 3137 "parser.cpp"
    break;

  case 137: /* struct_value: '{' $@11 field_values '}'  */
#line 1387 "parser.ypp"
        {
		if(type_stack.top().type->as_struct())
		{
			current_depth--;
		}
		type_stack.pop(); // Remove method type from the stack
		(yyval.str) = (yyvsp[-1].str);
	}
#line 3150 "parser.cpp"
    break;

  case 140: /* field_values: field_values ',' type_value  */
#line 1401 "parser.ypp"
        {
		(yyval.str) = (yyvsp[-2].str) + (yyvsp[0].str);
	}
#line 3158 "parser.cpp"
    break;

  case 141: /* field_values: field_values ',' method_value  */
#line 1405 "parser.ypp"
        {
		(yyval.str) = (yyvsp[-2].str) + (yyvsp[0].str);
	}
#line 3166 "parser.cpp"
    break;

  case 142: /* array_value: '[' ']'  */
#line 1412 "parser.ypp"
        {
		if(!check_depth()) depth_error("array");

		const DistributedType* dtype = type_stack.top().type;
		if(dtype == nullptr)
		{
			// Ignore this field, it should have already generated an error
			(yyval.str) = "";
			break;
		}
		type_stack.pop();
		if(!dtype->as_array())
		{
			parser_error("Cannot use array-composition for non-array type '"
			             + format_type(dtype->get_type()) + "'.");
			(yyval.str) = "";
			break;
		}
		const ArrayType* array = dtype->as_array();

		string val;

		if(array->get_array_size() > 0)
		{
			// For fixed size arrays, an empty array is an error
			parser_error("Fixed-sized array of size "
			             + to_string((unsigned long long)array->get_array_size())
			             + " can't have 0 elements.");
		}
		else if(array->has_range())
		{
			// If we have a range, 0 elements must be valid in the range
			if(0 < array->get_range().min.uinteger)
			{
				parser_error("Too few elements in array value, minimum "
				             + to_string(array->get_range().min.uinteger) + ".");
			}
		}

		// Since a fixed-size array can't have zero elements, this always
		// the default value for a varsize array, which is the length-tag 0.
		(yyval.str) = string(sizeof(sizetag_t), '\0');
	}
#line 3214 "parser.cpp"
    break;

  case 143: /* $@12: %empty  */
#line 1456 "parser.ypp"
        {
		if(!check_depth()) depth_error("array");

		const DistributedType* dtype = type_stack.top().type;
		if(dtype == nullptr)
		{
			// Ignore this field, it should have already generated an error
			break;
		}
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
#line 3245 "parser.cpp"
    break;

  case 144: /* array_value: '[' $@12 element_values ']'  */
#line 1483 "parser.ypp"
        {
		if(type_stack.top().type->as_array())
		{
			uint64_t actual_size = current_depth - type_stack.top().depth;

			const DistributedType* dtype = type_stack.top().type;
			if(dtype == nullptr)
			{
				// Ignore this field, it should have already generated an error
				(yyval.str) = "";
				break;
			}

			const ArrayType* array = dtype->as_array();
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

			if(array->get_array_size() == 0)
			{
				sizetag_t length = (yyvsp[-1].str).length();
				(yyval.str) = string((char*)&length, sizeof(sizetag_t)) + (yyvsp[-1].str);
			}
			else
			{
				(yyval.str) = (yyvsp[-1].str);
			}
			current_depth = type_stack.top().depth;
		}

		type_stack.pop(); // Remove array type from the stack
	}
#line 3292 "parser.cpp"
    break;

  case 146: /* $@13: %empty  */
#line 1530 "parser.ypp"
        {
		// We popped off the only element we added, so we're back to the array
		// Don't increment the depth; the array_expansion will add to
		// the current_depth depending on the number of elements it adds.
		const DistributedType* dtype = type_stack.top().type;
		if(dtype == nullptr)
		{
			// Ignore this field, it should have already generated an error
			break;
		}
		const ArrayType* array = dtype->as_array();
		type_stack.push(TypeAndDepth(array->get_element_type(), current_depth));
	}
#line 3310 "parser.cpp"
    break;

  case 147: /* element_values: element_values ',' $@13 array_expansion  */
#line 1544 "parser.ypp"
        {
		(yyval.str) = (yyvsp[-3].str) + (yyvsp[0].str);
	}
#line 3318 "parser.cpp"
    break;

  case 148: /* array_expansion: type_value  */
#line 1551 "parser.ypp"
        {
		current_depth++;
		(yyval.str) = (yyvsp[0].str);
	}
#line 3327 "parser.cpp"
    break;

  case 149: /* array_expansion: signed_integer '*' small_unsigned_integer  */
#line 1556 "parser.ypp"
        {
		const DistributedType* dtype = type_stack.top().type;
		if(dtype == nullptr)
		{
			// Ignore this field, it should have already generated an error
			(yyval.str) = "";
			break;
		}
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
#line 3353 "parser.cpp"
    break;

  case 150: /* array_expansion: UNSIGNED_INTEGER '*' small_unsigned_integer  */
#line 1578 "parser.ypp"
        {
		const DistributedType* dtype = type_stack.top().type;
		if(dtype == nullptr)
		{
			// Ignore this field, it should have already generated an error
			(yyval.str) = "";
			break;
		}
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
#line 3379 "parser.cpp"
    break;

  case 151: /* array_expansion: REAL '*' small_unsigned_integer  */
#line 1600 "parser.ypp"
        {
		const DistributedType* dtype = type_stack.top().type;
		if(dtype == nullptr)
		{
			// Ignore this field, it should have already generated an error
			(yyval.str) = "";
			break;
		}
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
#line 3405 "parser.cpp"
    break;

  case 152: /* array_expansion: STRING '*' small_unsigned_integer  */
#line 1622 "parser.ypp"
        {
		const DistributedType* dtype = type_stack.top().type;
		if(dtype == nullptr)
		{
			// Ignore this field, it should have already generated an error
			(yyval.str) = "";
			break;
		}
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
#line 3465 "parser.cpp"
    break;

  case 153: /* array_expansion: HEX_STRING '*' small_unsigned_integer  */
#line 1678 "parser.ypp"
        {
		const DistributedType* dtype = type_stack.top().type;
		if(dtype == nullptr)
		{
			// Ignore this field, it should have already generated an error
			(yyval.str) = "";
			break;
		}
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
#line 3526 "parser.cpp"
    break;

  case 154: /* signed_integer: '+' UNSIGNED_INTEGER  */
#line 1737 "parser.ypp"
                               { (yyval.u.int64) = int64_t((yyvsp[0].u.uint64)); }
#line 3532 "parser.cpp"
    break;

  case 155: /* signed_integer: '-' UNSIGNED_INTEGER  */
#line 1738 "parser.ypp"
                               { (yyval.u.int64) = -int64_t((yyvsp[0].u.uint64)); }
#line 3538 "parser.cpp"
    break;

  case 156: /* array_type_token: KW_STRING  */
#line 1742 "parser.ypp"
                    { (yyval.u.type) = T_STRING; }
#line 3544 "parser.cpp"
    break;

  case 157: /* array_type_token: KW_BLOB  */
#line 1743 "parser.ypp"
                  { (yyval.u.type) = T_BLOB; }
#line 3550 "parser.cpp"
    break;

  case 158: /* numeric_type_token: KW_CHAR  */
#line 1747 "parser.ypp"
                  { (yyval.u.type) = T_CHAR; }
#line 3556 "parser.cpp"
    break;

  case 159: /* numeric_type_token: KW_INT8  */
#line 1748 "parser.ypp"
                  { (yyval.u.type) = T_INT8; }
#line 3562 "parser.cpp"
    break;

  case 160: /* numeric_type_token: KW_INT16  */
#line 1749 "parser.ypp"
                   { (yyval.u.type) = T_INT16; }
#line 3568 "parser.cpp"
    break;

  case 161: /* numeric_type_token: KW_INT32  */
#line 1750 "parser.ypp"
                   { (yyval.u.type) = T_INT32; }
#line 3574 "parser.cpp"
    break;

  case 162: /* numeric_type_token: KW_INT64  */
#line 1751 "parser.ypp"
                   { (yyval.u.type) = T_INT64; }
#line 3580 "parser.cpp"
    break;

  case 163: /* numeric_type_token: KW_UINT8  */
#line 1752 "parser.ypp"
                   { (yyval.u.type) = T_UINT8; }
#line 3586 "parser.cpp"
    break;

  case 164: /* numeric_type_token: KW_UINT16  */
#line 1753 "parser.ypp"
                    { (yyval.u.type) = T_UINT16; }
#line 3592 "parser.cpp"
    break;

  case 165: /* numeric_type_token: KW_UINT32  */
#line 1754 "parser.ypp"
                    { (yyval.u.type) = T_UINT32; }
#line 3598 "parser.cpp"
    break;

  case 166: /* numeric_type_token: KW_UINT64  */
#line 1755 "parser.ypp"
                    { (yyval.u.type) = T_UINT64; }
#line 3604 "parser.cpp"
    break;

  case 167: /* numeric_type_token: KW_FLOAT32  */
#line 1756 "parser.ypp"
                     { (yyval.u.type) = T_FLOAT32; }
#line 3610 "parser.cpp"
    break;

  case 168: /* numeric_type_token: KW_FLOAT64  */
#line 1757 "parser.ypp"
                     { (yyval.u.type) = T_FLOAT64; }
#line 3616 "parser.cpp"
    break;

  case 169: /* keyword_list: empty  */
#line 1762 "parser.ypp"
        {
		(yyval.strings) = vector<string>();
	}
#line 3624 "parser.cpp"
    break;

  case 170: /* keyword_list: keyword_list IDENTIFIER  */
#line 1766 "parser.ypp"
        {
		if(!parsed_file->has_keyword((yyvsp[0].str)))
		{
			parser_error("Keyword '" + (yyvsp[0].str) + "' has not been declared.");
			break;
		}

		(yyvsp[-1].strings).push_back((yyvsp[0].str));
		(yyval.strings) = (yyvsp[-1].strings);
	}
#line 3639 "parser.cpp"
    break;


#line 3643 "parser.cpp"

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
  YY_SYMBOL_PRINT ("-> $$ =", YY_CAST (yysymbol_kind_t, yyr1[yyn]), &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;

  *++yyvsp = yyval;

  /* Now 'shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */
  {
    const int yylhs = yyr1[yyn] - YYNTOKENS;
    const int yyi = yypgoto[yylhs] + *yyssp;
    yystate = (0 <= yyi && yyi <= YYLAST && yycheck[yyi] == *yyssp
               ? yytable[yyi]
               : yydefgoto[yylhs]);
  }

  goto yynewstate;


/*--------------------------------------.
| yyerrlab -- here on detecting error.  |
`--------------------------------------*/
yyerrlab:
  /* Make sure we have latest lookahead translation.  See comments at
     user semantic actions for why this is necessary.  */
  yytoken = yychar == YYEMPTY ? YYSYMBOL_YYEMPTY : YYTRANSLATE (yychar);
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
      {
        yypcontext_t yyctx
          = {yyssp, yytoken};
        char const *yymsgp = YY_("syntax error");
        int yysyntax_error_status;
        yysyntax_error_status = yysyntax_error (&yymsg_alloc, &yymsg, &yyctx);
        if (yysyntax_error_status == 0)
          yymsgp = yymsg;
        else if (yysyntax_error_status == -1)
          {
            if (yymsg != yymsgbuf)
              YYSTACK_FREE (yymsg);
            yymsg = YY_CAST (char *,
                             YYSTACK_ALLOC (YY_CAST (YYSIZE_T, yymsg_alloc)));
            if (yymsg)
              {
                yysyntax_error_status
                  = yysyntax_error (&yymsg_alloc, &yymsg, &yyctx);
                yymsgp = yymsg;
              }
            else
              {
                yymsg = yymsgbuf;
                yymsg_alloc = sizeof yymsgbuf;
                yysyntax_error_status = YYENOMEM;
              }
          }
        yyerror (yymsgp);
        if (yysyntax_error_status == YYENOMEM)
          YYNOMEM;
      }
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
  /* Pacify compilers when the user code never invokes YYERROR and the
     label yyerrorlab therefore never appears in user code.  */
  if (0)
    YYERROR;
  ++yynerrs;

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

  /* Pop stack until we find a state that shifts the error token.  */
  for (;;)
    {
      yyn = yypact[yystate];
      if (!yypact_value_is_default (yyn))
        {
          yyn += YYSYMBOL_YYerror;
          if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYSYMBOL_YYerror)
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
                  YY_ACCESSING_SYMBOL (yystate), yyvsp);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END


  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", YY_ACCESSING_SYMBOL (yyn), yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturnlab;


/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturnlab;


/*-----------------------------------------------------------.
| yyexhaustedlab -- YYNOMEM (memory exhaustion) comes here.  |
`-----------------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  goto yyreturnlab;


/*----------------------------------------------------------.
| yyreturnlab -- parsing is finished, clean up and return.  |
`----------------------------------------------------------*/
yyreturnlab:
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
                  YY_ACCESSING_SYMBOL (+*yyssp), yyvsp);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
  if (yymsg != yymsgbuf)
    YYSTACK_FREE (yymsg);
  return yyresult;
}

#line 1782 "parser.ypp"
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
			if(v == std::numeric_limits<float>::infinity() ||
			   v == -std::numeric_limits<float>::infinity())
			{
				parser_error("Value is out of range for type 'float32'.");
			}

			v = swap_le(v);
			return string((char*)&v, sizeof(float));
		}
		case T_FLOAT64:
		{
			double v = number;
			v = swap_le(v);
			return string((char*)&v, sizeof(double));
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
			v = swap_le(v);
			return string((char*)&v, sizeof(int16_t));
		}
		case T_INT32:
		{
			if(INT32_MIN > number || number > INT32_MAX)
			{
				parser_error("Signed integer out of range for type 'int32'.");
			}

			int32_t v = number;
			v = swap_le(v);
			return string((char*)&v, sizeof(int32_t));
		}
		case T_INT64:
		{
			int64_t v = number;
			v = swap_le(v);
			return string((char*)&v, sizeof(int64_t));
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
			v = swap_le(v);
			return string((char*)&v, sizeof(uint16_t));
		}
		case T_UINT32:
		{
			if(number > UINT32_MAX)
			{
				parser_error("Unsigned integer out of range for type 'uint32'.");
			}

			uint32_t v = number;
			v = swap_le(v);
			return string((char*)&v, sizeof(uint32_t));
		}
		case T_UINT64:
		{
			uint64_t v = number;
			v = swap_le(v);
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
