/* A Bison parser, made by GNU Bison 3.7.3.  */

/* Bison implementation for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018-2020 Free Software Foundation,
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

/* DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
   especially those whose name start with YY_ or yy_.  They are
   private implementation details that can be changed or removed.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Bison version.  */
#define YYBISON_VERSION "3.7.3"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1




/* First part of user prologue.  */
#line 35 "parser.y"

#ifdef HAVE_DMX_CONFIG_H
#include <dmx-config.h>
#endif

#include "dmxparse.h"
#include <string.h>
#include <stdlib.h>
#define YYDEBUG 1
#define YYERROR_VERBOSE
#define YY_USE_PROTOS

extern int yylex(void);
DMXConfigEntryPtr dmxConfigEntry = NULL;
#define APPEND(type, h, t)                 \
{                                          \
    type pt;                               \
    for (pt = h; pt->next; pt = pt->next); \
    pt->next = t;                          \
}

#line 93 "parser.c"

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

/* Use api.header.include to #include this header
   instead of duplicating it here.  */
#ifndef YY_YY_PARSER_H_INCLUDED
# define YY_YY_PARSER_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
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
    T_VIRTUAL = 258,               /* T_VIRTUAL  */
    T_DISPLAY = 259,               /* T_DISPLAY  */
    T_WALL = 260,                  /* T_WALL  */
    T_OPTION = 261,                /* T_OPTION  */
    T_PARAM = 262,                 /* T_PARAM  */
    T_STRING = 263,                /* T_STRING  */
    T_DIMENSION = 264,             /* T_DIMENSION  */
    T_OFFSET = 265,                /* T_OFFSET  */
    T_ORIGIN = 266,                /* T_ORIGIN  */
    T_COMMENT = 267,               /* T_COMMENT  */
    T_LINE_COMMENT = 268           /* T_LINE_COMMENT  */
  };
  typedef enum yytokentype yytoken_kind_t;
#endif
/* Token kinds.  */
#define YYEOF 0
#define YYerror 256
#define YYUNDEF 257
#define T_VIRTUAL 258
#define T_DISPLAY 259
#define T_WALL 260
#define T_OPTION 261
#define T_PARAM 262
#define T_STRING 263
#define T_DIMENSION 264
#define T_OFFSET 265
#define T_ORIGIN 266
#define T_COMMENT 267
#define T_LINE_COMMENT 268

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
union YYSTYPE
{
#line 57 "parser.y"

    DMXConfigTokenPtr      token;
    DMXConfigStringPtr     string;
    DMXConfigNumberPtr     number;
    DMXConfigPairPtr       pair;
    DMXConfigFullDimPtr    fdim;
    DMXConfigPartDimPtr    pdim;
    DMXConfigDisplayPtr    display;
    DMXConfigWallPtr       wall;
    DMXConfigOptionPtr     option;
    DMXConfigParamPtr      param;
    DMXConfigCommentPtr    comment;
    DMXConfigSubPtr        subentry;
    DMXConfigVirtualPtr    virtual;
    DMXConfigEntryPtr      entry;

#line 188 "parser.c"

};
typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;

int yyparse (void);

#endif /* !YY_YY_PARSER_H_INCLUDED  */
/* Symbol kind.  */
enum yysymbol_kind_t
{
  YYSYMBOL_YYEMPTY = -2,
  YYSYMBOL_YYEOF = 0,                      /* "end of file"  */
  YYSYMBOL_YYerror = 1,                    /* error  */
  YYSYMBOL_YYUNDEF = 2,                    /* "invalid token"  */
  YYSYMBOL_3_ = 3,                         /* '{'  */
  YYSYMBOL_4_ = 4,                         /* '}'  */
  YYSYMBOL_5_ = 5,                         /* ';'  */
  YYSYMBOL_6_ = 6,                         /* '/'  */
  YYSYMBOL_T_VIRTUAL = 7,                  /* T_VIRTUAL  */
  YYSYMBOL_T_DISPLAY = 8,                  /* T_DISPLAY  */
  YYSYMBOL_T_WALL = 9,                     /* T_WALL  */
  YYSYMBOL_T_OPTION = 10,                  /* T_OPTION  */
  YYSYMBOL_T_PARAM = 11,                   /* T_PARAM  */
  YYSYMBOL_T_STRING = 12,                  /* T_STRING  */
  YYSYMBOL_T_DIMENSION = 13,               /* T_DIMENSION  */
  YYSYMBOL_T_OFFSET = 14,                  /* T_OFFSET  */
  YYSYMBOL_T_ORIGIN = 15,                  /* T_ORIGIN  */
  YYSYMBOL_T_COMMENT = 16,                 /* T_COMMENT  */
  YYSYMBOL_T_LINE_COMMENT = 17,            /* T_LINE_COMMENT  */
  YYSYMBOL_YYACCEPT = 18,                  /* $accept  */
  YYSYMBOL_Program = 19,                   /* Program  */
  YYSYMBOL_EntryList = 20,                 /* EntryList  */
  YYSYMBOL_Entry = 21,                     /* Entry  */
  YYSYMBOL_Virtual = 22,                   /* Virtual  */
  YYSYMBOL_SubList = 23,                   /* SubList  */
  YYSYMBOL_Sub = 24,                       /* Sub  */
  YYSYMBOL_OptionEntry = 25,               /* OptionEntry  */
  YYSYMBOL_ParamEntry = 26,                /* ParamEntry  */
  YYSYMBOL_ParamList = 27,                 /* ParamList  */
  YYSYMBOL_Param = 28,                     /* Param  */
  YYSYMBOL_PartialDim = 29,                /* PartialDim  */
  YYSYMBOL_FullDim = 30,                   /* FullDim  */
  YYSYMBOL_DisplayEntry = 31,              /* DisplayEntry  */
  YYSYMBOL_WallEntry = 32,                 /* WallEntry  */
  YYSYMBOL_Display = 33,                   /* Display  */
  YYSYMBOL_Name = 34,                      /* Name  */
  YYSYMBOL_Dimension = 35,                 /* Dimension  */
  YYSYMBOL_Offset = 36,                    /* Offset  */
  YYSYMBOL_Origin = 37,                    /* Origin  */
  YYSYMBOL_Terminal = 38,                  /* Terminal  */
  YYSYMBOL_Open = 39,                      /* Open  */
  YYSYMBOL_Close = 40,                     /* Close  */
  YYSYMBOL_Wall = 41,                      /* Wall  */
  YYSYMBOL_NameList = 42                   /* NameList  */
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
typedef yytype_int8 yy_state_t;

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
# define YYUSE(E) ((void) (E))
#else
# define YYUSE(E) /* empty */
#endif

#if defined __GNUC__ && ! defined __ICC && 407 <= __GNUC__ * 100 + __GNUC_MINOR__
/* Suppress an incorrect diagnostic about yylval being uninitialized.  */
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                            \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")              \
    _Pragma ("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")
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

#if !defined yyoverflow

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
#endif /* !defined yyoverflow */

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
#define YYFINAL  13
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   106

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  18
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  25
/* YYNRULES -- Number of rules.  */
#define YYNRULES  59
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  95

/* YYMAXUTOK -- Last valid token kind.  */
#define YYMAXUTOK   268


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
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     6,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     5,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     3,     2,     4,     2,     2,     2,     2,
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
       2,     2,     2,     2,     2,     2,     1,     2,     7,     8,
       9,    10,    11,    12,    13,    14,    15,    16,    17
};

#if YYDEBUG
  /* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_uint8 yyrline[] =
{
       0,    96,    96,    99,   100,   103,   104,   107,   109,   111,
     113,   117,   118,   121,   122,   123,   124,   125,   128,   132,
     134,   140,   141,   144,   148,   150,   152,   156,   158,   160,
     164,   166,   168,   171,   173,   175,   177,   181,   183,   185,
     189,   190,   193,   194,   197,   198,   201,   202,   205,   206,
     209,   210,   213,   214,   217,   218,   221,   222,   225,   226
};
#endif

/** Accessing symbol of state STATE.  */
#define YY_ACCESSING_SYMBOL(State) YY_CAST (yysymbol_kind_t, yystos[State])

#if YYDEBUG || 0
/* The user-facing name of the symbol whose (internal) number is
   YYSYMBOL.  No bounds checking.  */
static const char *yysymbol_name (yysymbol_kind_t yysymbol) YY_ATTRIBUTE_UNUSED;

/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "\"end of file\"", "error", "\"invalid token\"", "'{'", "'}'", "';'",
  "'/'", "T_VIRTUAL", "T_DISPLAY", "T_WALL", "T_OPTION", "T_PARAM",
  "T_STRING", "T_DIMENSION", "T_OFFSET", "T_ORIGIN", "T_COMMENT",
  "T_LINE_COMMENT", "$accept", "Program", "EntryList", "Entry", "Virtual",
  "SubList", "Sub", "OptionEntry", "ParamEntry", "ParamList", "Param",
  "PartialDim", "FullDim", "DisplayEntry", "WallEntry", "Display", "Name",
  "Dimension", "Offset", "Origin", "Terminal", "Open", "Close", "Wall",
  "NameList", YY_NULLPTR
};

static const char *
yysymbol_name (yysymbol_kind_t yysymbol)
{
  return yytname[yysymbol];
}
#endif

#ifdef YYPRINT
/* YYTOKNUM[NUM] -- (External) token number corresponding to the
   (internal) symbol number NUM (which must be that of a token).  */
static const yytype_int16 yytoknum[] =
{
       0,   256,   257,   123,   125,    59,    47,   258,   259,   260,
     261,   262,   263,   264,   265,   266,   267,   268
};
#endif

#define YYPACT_NINF (-32)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-1)

#define yytable_value_is_error(Yyn) \
  0

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int8 yypact[] =
{
      -3,    41,   -32,    22,    -3,   -32,   -32,    12,    35,    46,
       5,    62,    75,   -32,   -32,   -32,   -32,   -32,    62,    75,
      75,    51,    54,    59,    18,   -32,    65,   -32,   -32,   -32,
     -32,   -32,    88,    37,    75,    65,    65,   -32,   -32,   -32,
      86,    59,    86,    61,   -32,   -32,    79,    -4,    80,    28,
      31,    74,    67,   -32,   -32,    37,    86,    65,   -32,   -32,
     -32,   -32,    56,   -32,    86,   -32,   -32,   -32,   -32,   -32,
      -4,    81,    94,   -32,    31,    94,   -32,   -32,    59,    86,
     -32,   -32,   -32,   -32,   -32,   -32,   -32,   -32,    94,   -32,
     -32,    86,   -32,   -32,   -32
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_int8 yydefact[] =
{
       0,     0,     6,     0,     2,     3,     5,    52,    42,    44,
       0,     0,     0,     1,     4,    53,    43,    45,     0,     0,
       0,    40,    56,     0,     0,    13,     0,    11,    16,    17,
      14,    15,     0,     0,     0,     0,     0,    41,    57,    58,
       0,     0,     0,    54,    12,     7,    50,     0,    46,    29,
       0,     0,    25,    26,    36,     0,     0,     0,     9,     8,
      59,    18,     0,    21,     0,    19,    55,    51,    28,    47,
       0,    48,     0,    34,     0,     0,    35,    24,     0,     0,
      39,    10,    22,    20,    23,    27,    49,    31,     0,    33,
      32,     0,    38,    30,    37
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int8 yypgoto[] =
{
     -32,   -32,   -32,    99,   -32,     6,   -19,   -32,   -32,   -32,
      42,   -28,    55,   -32,   -32,   -32,    -1,     2,    53,   -31,
     -27,    48,   -30,   -32,   -22
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int8 yydefgoto[] =
{
      -1,     3,     4,     5,     6,    26,    27,    28,    29,    62,
      63,    49,    50,    30,    31,    32,    39,    52,    53,    72,
      54,    12,    45,    33,    64
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int8 yytable[] =
{
      10,    40,    42,    11,     1,    58,    59,    44,     7,     9,
      48,    56,    18,    61,     2,    65,    44,    44,     9,    68,
      75,     7,    13,    73,    76,    35,    36,    81,    15,    80,
       8,    51,    83,    79,    70,    55,    46,    84,    44,    60,
      57,    60,    85,    88,     7,    87,    71,    89,    90,     8,
       9,    16,    92,     8,     9,    60,    91,    78,    19,    20,
      43,    93,    17,    60,    94,     7,    34,    37,     8,    43,
      38,     8,    41,    21,    22,    23,    24,    66,    60,    46,
      47,    48,    25,    21,    22,    23,    24,     9,    48,    71,
      60,    46,    25,    46,    47,    67,    69,    86,     8,    46,
       8,     9,    48,    14,    82,    77,    74
};

static const yytype_int8 yycheck[] =
{
       1,    23,    24,     1,     7,    35,    36,    26,     3,    13,
      14,    33,    10,    40,    17,    42,    35,    36,    13,    47,
      51,     3,     0,    50,    51,    19,    20,    57,    16,    56,
      12,    32,    62,    55,     6,    33,     5,    64,    57,    40,
      34,    42,    70,    74,     3,    72,    15,    74,    75,    12,
      13,    16,    79,    12,    13,    56,    78,    55,    10,    11,
       4,    88,    16,    64,    91,     3,    18,    16,    12,     4,
      16,    12,    24,     8,     9,    10,    11,    16,    79,     5,
       6,    14,    17,     8,     9,    10,    11,    13,    14,    15,
      91,     5,    17,     5,     6,    16,    16,    16,    12,     5,
      12,    13,    14,     4,    62,    52,    51
};

  /* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
static const yytype_int8 yystos[] =
{
       0,     7,    17,    19,    20,    21,    22,     3,    12,    13,
      34,    35,    39,     0,    21,    16,    16,    16,    35,    39,
      39,     8,     9,    10,    11,    17,    23,    24,    25,    26,
      31,    32,    33,    41,    39,    23,    23,    16,    16,    34,
      42,    39,    42,     4,    24,    40,     5,     6,    14,    29,
      30,    34,    35,    36,    38,    35,    42,    23,    40,    40,
      34,    38,    27,    28,    42,    38,    16,    16,    29,    16,
       6,    15,    37,    38,    30,    37,    38,    36,    35,    42,
      38,    40,    28,    40,    38,    29,    16,    38,    37,    38,
      38,    42,    38,    38,    38
};

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_int8 yyr1[] =
{
       0,    18,    19,    20,    20,    21,    21,    22,    22,    22,
      22,    23,    23,    24,    24,    24,    24,    24,    25,    26,
      26,    27,    27,    28,    29,    29,    29,    30,    30,    30,
      31,    31,    31,    31,    31,    31,    31,    32,    32,    32,
      33,    33,    34,    34,    35,    35,    36,    36,    37,    37,
      38,    38,    39,    39,    40,    40,    41,    41,    42,    42
};

  /* YYR2[YYN] -- Number of symbols on the right hand side of rule YYN.  */
static const yytype_int8 yyr2[] =
{
       0,     2,     1,     1,     2,     1,     1,     4,     5,     5,
       6,     1,     2,     1,     1,     1,     1,     1,     3,     3,
       4,     1,     2,     2,     2,     1,     1,     3,     2,     1,
       5,     4,     4,     4,     3,     3,     2,     5,     4,     3,
       1,     2,     1,     2,     1,     2,     1,     2,     1,     2,
       1,     2,     1,     2,     1,     2,     1,     2,     1,     2
};


enum { YYENOMEM = -2 };

#define yyerrok         (yyerrstatus = 0)
#define yyclearin       (yychar = YYEMPTY)

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab


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

/* This macro is provided for backward compatibility. */
# ifndef YY_LOCATION_PRINT
#  define YY_LOCATION_PRINT(File, Loc) ((void) 0)
# endif


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
  YYUSE (yyoutput);
  if (!yyvaluep)
    return;
# ifdef YYPRINT
  if (yykind < YYNTOKENS)
    YYPRINT (yyo, yytoknum[yykind], *yyvaluep);
# endif
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YYUSE (yykind);
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






/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

static void
yydestruct (const char *yymsg,
            yysymbol_kind_t yykind, YYSTYPE *yyvaluep)
{
  YYUSE (yyvaluep);
  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yykind, yyvaluep, yylocationp);

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YYUSE (yykind);
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
    goto yyexhaustedlab;
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
        goto yyexhaustedlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
        yystacksize = YYMAXDEPTH;

      {
        yy_state_t *yyss1 = yyss;
        union yyalloc *yyptr =
          YY_CAST (union yyalloc *,
                   YYSTACK_ALLOC (YY_CAST (YYSIZE_T, YYSTACK_BYTES (yystacksize))));
        if (! yyptr)
          goto yyexhaustedlab;
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
  case 2: /* Program: EntryList  */
#line 96 "parser.y"
                    { dmxConfigEntry = (yyvsp[0].entry); }
#line 1262 "parser.c"
    break;

  case 4: /* EntryList: EntryList Entry  */
#line 100 "parser.y"
                            { APPEND(DMXConfigEntryPtr,(yyvsp[-1].entry),(yyvsp[0].entry)); (yyval.entry) = (yyvsp[-1].entry); }
#line 1268 "parser.c"
    break;

  case 5: /* Entry: Virtual  */
#line 103 "parser.y"
                       { (yyval.entry) = dmxConfigEntryVirtual((yyvsp[0].virtual)); }
#line 1274 "parser.c"
    break;

  case 6: /* Entry: T_LINE_COMMENT  */
#line 104 "parser.y"
                       { (yyval.entry) = dmxConfigEntryComment((yyvsp[0].comment)); }
#line 1280 "parser.c"
    break;

  case 7: /* Virtual: T_VIRTUAL Open SubList Close  */
#line 108 "parser.y"
          { (yyval.virtual) = dmxConfigCreateVirtual((yyvsp[-3].token), NULL, NULL, (yyvsp[-2].token), (yyvsp[-1].subentry), (yyvsp[0].token)); }
#line 1286 "parser.c"
    break;

  case 8: /* Virtual: T_VIRTUAL Dimension Open SubList Close  */
#line 110 "parser.y"
          { (yyval.virtual) = dmxConfigCreateVirtual((yyvsp[-4].token), NULL, (yyvsp[-3].pair), (yyvsp[-2].token), (yyvsp[-1].subentry), (yyvsp[0].token)); }
#line 1292 "parser.c"
    break;

  case 9: /* Virtual: T_VIRTUAL Name Open SubList Close  */
#line 112 "parser.y"
          { (yyval.virtual) = dmxConfigCreateVirtual((yyvsp[-4].token), (yyvsp[-3].string), NULL, (yyvsp[-2].token), (yyvsp[-1].subentry), (yyvsp[0].token)); }
#line 1298 "parser.c"
    break;

  case 10: /* Virtual: T_VIRTUAL Name Dimension Open SubList Close  */
#line 114 "parser.y"
          { (yyval.virtual) = dmxConfigCreateVirtual((yyvsp[-5].token), (yyvsp[-4].string), (yyvsp[-3].pair), (yyvsp[-2].token), (yyvsp[-1].subentry), (yyvsp[0].token) ); }
#line 1304 "parser.c"
    break;

  case 12: /* SubList: SubList Sub  */
#line 118 "parser.y"
                      { APPEND(DMXConfigSubPtr,(yyvsp[-1].subentry),(yyvsp[0].subentry)); (yyval.subentry) = (yyvsp[-1].subentry); }
#line 1310 "parser.c"
    break;

  case 13: /* Sub: T_LINE_COMMENT  */
#line 121 "parser.y"
                     { (yyval.subentry) = dmxConfigSubComment((yyvsp[0].comment)); }
#line 1316 "parser.c"
    break;

  case 14: /* Sub: DisplayEntry  */
#line 122 "parser.y"
                     { (yyval.subentry) = dmxConfigSubDisplay((yyvsp[0].display)); }
#line 1322 "parser.c"
    break;

  case 15: /* Sub: WallEntry  */
#line 123 "parser.y"
                     { (yyval.subentry) = dmxConfigSubWall((yyvsp[0].wall)); }
#line 1328 "parser.c"
    break;

  case 16: /* Sub: OptionEntry  */
#line 124 "parser.y"
                     { (yyval.subentry) = dmxConfigSubOption((yyvsp[0].option)); }
#line 1334 "parser.c"
    break;

  case 17: /* Sub: ParamEntry  */
#line 125 "parser.y"
                     { (yyval.subentry) = dmxConfigSubParam((yyvsp[0].param)); }
#line 1340 "parser.c"
    break;

  case 18: /* OptionEntry: T_OPTION NameList Terminal  */
#line 129 "parser.y"
              { (yyval.option) = dmxConfigCreateOption((yyvsp[-2].token), (yyvsp[-1].string), (yyvsp[0].token)); }
#line 1346 "parser.c"
    break;

  case 19: /* ParamEntry: T_PARAM NameList Terminal  */
#line 133 "parser.y"
             { (yyval.param) = dmxConfigCreateParam((yyvsp[-2].token), NULL, (yyvsp[-1].string), NULL, (yyvsp[0].token)); }
#line 1352 "parser.c"
    break;

  case 20: /* ParamEntry: T_PARAM Open ParamList Close  */
#line 135 "parser.y"
             { (yyval.param) = dmxConfigCreateParam((yyvsp[-3].token), (yyvsp[-2].token), NULL, (yyvsp[0].token), NULL);
               (yyval.param)->next = (yyvsp[-1].param);
             }
#line 1360 "parser.c"
    break;

  case 22: /* ParamList: ParamList Param  */
#line 141 "parser.y"
                            { APPEND(DMXConfigParamPtr,(yyvsp[-1].param),(yyvsp[0].param)); (yyval.param) = (yyvsp[-1].param); }
#line 1366 "parser.c"
    break;

  case 23: /* Param: NameList Terminal  */
#line 145 "parser.y"
        { (yyval.param) = dmxConfigCreateParam(NULL, NULL, (yyvsp[-1].string), NULL, (yyvsp[0].token)); }
#line 1372 "parser.c"
    break;

  case 24: /* PartialDim: Dimension Offset  */
#line 149 "parser.y"
             { (yyval.pdim) = dmxConfigCreatePartDim((yyvsp[-1].pair), (yyvsp[0].pair)); }
#line 1378 "parser.c"
    break;

  case 25: /* PartialDim: Dimension  */
#line 151 "parser.y"
             { (yyval.pdim) = dmxConfigCreatePartDim((yyvsp[0].pair), NULL); }
#line 1384 "parser.c"
    break;

  case 26: /* PartialDim: Offset  */
#line 153 "parser.y"
             { (yyval.pdim) = dmxConfigCreatePartDim(NULL, (yyvsp[0].pair)); }
#line 1390 "parser.c"
    break;

  case 27: /* FullDim: PartialDim '/' PartialDim  */
#line 157 "parser.y"
          { (yyval.fdim) = dmxConfigCreateFullDim((yyvsp[-2].pdim), (yyvsp[0].pdim)); }
#line 1396 "parser.c"
    break;

  case 28: /* FullDim: '/' PartialDim  */
#line 159 "parser.y"
          { (yyval.fdim) = dmxConfigCreateFullDim(NULL, (yyvsp[0].pdim)); }
#line 1402 "parser.c"
    break;

  case 29: /* FullDim: PartialDim  */
#line 161 "parser.y"
          { (yyval.fdim) = dmxConfigCreateFullDim((yyvsp[0].pdim), NULL); }
#line 1408 "parser.c"
    break;

  case 30: /* DisplayEntry: Display Name FullDim Origin Terminal  */
#line 165 "parser.y"
               { (yyval.display) = dmxConfigCreateDisplay((yyvsp[-4].token), (yyvsp[-3].string), (yyvsp[-2].fdim), (yyvsp[-1].pair), (yyvsp[0].token)); }
#line 1414 "parser.c"
    break;

  case 31: /* DisplayEntry: Display FullDim Origin Terminal  */
#line 167 "parser.y"
               { (yyval.display) = dmxConfigCreateDisplay((yyvsp[-3].token), NULL, (yyvsp[-2].fdim), (yyvsp[-1].pair), (yyvsp[0].token)); }
#line 1420 "parser.c"
    break;

  case 32: /* DisplayEntry: Display Name Origin Terminal  */
#line 169 "parser.y"
               { (yyval.display) = dmxConfigCreateDisplay((yyvsp[-3].token), (yyvsp[-2].string), NULL, (yyvsp[-1].pair), (yyvsp[0].token)); }
#line 1426 "parser.c"
    break;

  case 33: /* DisplayEntry: Display Name FullDim Terminal  */
#line 172 "parser.y"
               { (yyval.display) = dmxConfigCreateDisplay((yyvsp[-3].token), (yyvsp[-2].string), (yyvsp[-1].fdim), NULL, (yyvsp[0].token)); }
#line 1432 "parser.c"
    break;

  case 34: /* DisplayEntry: Display FullDim Terminal  */
#line 174 "parser.y"
               { (yyval.display) = dmxConfigCreateDisplay((yyvsp[-2].token), NULL, (yyvsp[-1].fdim), NULL, (yyvsp[0].token)); }
#line 1438 "parser.c"
    break;

  case 35: /* DisplayEntry: Display Name Terminal  */
#line 176 "parser.y"
               { (yyval.display) = dmxConfigCreateDisplay((yyvsp[-2].token), (yyvsp[-1].string), NULL, NULL, (yyvsp[0].token)); }
#line 1444 "parser.c"
    break;

  case 36: /* DisplayEntry: Display Terminal  */
#line 178 "parser.y"
               { (yyval.display) = dmxConfigCreateDisplay((yyvsp[-1].token), NULL, NULL, NULL, (yyvsp[0].token)); }
#line 1450 "parser.c"
    break;

  case 37: /* WallEntry: Wall Dimension Dimension NameList Terminal  */
#line 182 "parser.y"
            { (yyval.wall) = dmxConfigCreateWall((yyvsp[-4].token), (yyvsp[-3].pair), (yyvsp[-2].pair), (yyvsp[-1].string), (yyvsp[0].token)); }
#line 1456 "parser.c"
    break;

  case 38: /* WallEntry: Wall Dimension NameList Terminal  */
#line 184 "parser.y"
            { (yyval.wall) = dmxConfigCreateWall((yyvsp[-3].token), (yyvsp[-2].pair), NULL, (yyvsp[-1].string), (yyvsp[0].token)); }
#line 1462 "parser.c"
    break;

  case 39: /* WallEntry: Wall NameList Terminal  */
#line 186 "parser.y"
            { (yyval.wall) = dmxConfigCreateWall((yyvsp[-2].token), NULL, NULL, (yyvsp[-1].string), (yyvsp[0].token)); }
#line 1468 "parser.c"
    break;

  case 41: /* Display: T_DISPLAY T_COMMENT  */
#line 190 "parser.y"
                              { (yyval.token) = (yyvsp[-1].token); (yyval.token)->comment = (yyvsp[0].comment)->comment; }
#line 1474 "parser.c"
    break;

  case 43: /* Name: T_STRING T_COMMENT  */
#line 194 "parser.y"
                          { (yyval.string) = (yyvsp[-1].string); (yyval.string)->comment = (yyvsp[0].comment)->comment; }
#line 1480 "parser.c"
    break;

  case 45: /* Dimension: T_DIMENSION T_COMMENT  */
#line 198 "parser.y"
                                  { (yyval.pair) = (yyvsp[-1].pair); (yyval.pair)->comment = (yyvsp[0].comment)->comment; }
#line 1486 "parser.c"
    break;

  case 47: /* Offset: T_OFFSET T_COMMENT  */
#line 202 "parser.y"
                            { (yyval.pair) = (yyvsp[-1].pair); (yyval.pair)->comment = (yyvsp[0].comment)->comment; }
#line 1492 "parser.c"
    break;

  case 49: /* Origin: T_ORIGIN T_COMMENT  */
#line 206 "parser.y"
                            { (yyval.pair) = (yyvsp[-1].pair); (yyval.pair)->comment = (yyvsp[0].comment)->comment; }
#line 1498 "parser.c"
    break;

  case 51: /* Terminal: ';' T_COMMENT  */
#line 210 "parser.y"
                         { (yyval.token) = (yyvsp[-1].token); (yyval.token)->comment = (yyvsp[0].comment)->comment; }
#line 1504 "parser.c"
    break;

  case 53: /* Open: '{' T_COMMENT  */
#line 214 "parser.y"
                     { (yyval.token) = (yyvsp[-1].token); (yyval.token)->comment = (yyvsp[0].comment)->comment; }
#line 1510 "parser.c"
    break;

  case 55: /* Close: '}' T_COMMENT  */
#line 218 "parser.y"
                      { (yyval.token) = (yyvsp[-1].token); (yyval.token)->comment = (yyvsp[0].comment)->comment; }
#line 1516 "parser.c"
    break;

  case 57: /* Wall: T_WALL T_COMMENT  */
#line 222 "parser.y"
                        { (yyval.token) = (yyvsp[-1].token); (yyval.token)->comment = (yyvsp[0].comment)->comment; }
#line 1522 "parser.c"
    break;

  case 59: /* NameList: NameList Name  */
#line 226 "parser.y"
                         { APPEND(DMXConfigStringPtr, (yyvsp[-1].string), (yyvsp[0].string)); (yyval.string) = (yyvsp[-1].string); }
#line 1528 "parser.c"
    break;


#line 1532 "parser.c"

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
      yyerror (YY_("syntax error"));
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
  goto yyreturn;


/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturn;


#if !defined yyoverflow
/*-------------------------------------------------.
| yyexhaustedlab -- memory exhaustion comes here.  |
`-------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  goto yyreturn;
#endif


/*-------------------------------------------------------.
| yyreturn -- parsing is finished, clean up and return.  |
`-------------------------------------------------------*/
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
                  YY_ACCESSING_SYMBOL (+*yyssp), yyvsp);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif

  return yyresult;
}

