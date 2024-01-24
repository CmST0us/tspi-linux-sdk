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
#line 1 "winprefsyacc.y"

/*
 * Copyright (C) 1994-2000 The XFree86 Project, Inc. All Rights Reserved.
 * Copyright (C) Colin Harrison 2005-2008
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE XFREE86 PROJECT BE LIABLE FOR
 * ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
 * CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * Except as contained in this notice, the name of the XFree86 Project
 * shall not be used in advertising or otherwise to promote the sale, use
 * or other dealings in this Software without prior written authorization
 * from the XFree86 Project.
 *
 * Authors:     Earle F. Philhower, III
 *              Colin Harrison
 */
/* $XFree86: $ */

#ifdef HAVE_XWIN_CONFIG_H
#include <xwin-config.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#define _STDLIB_H 1 /* bison checks this to know if stdlib has been included */
#include <string.h>
#include "winprefs.h"

/* The following give better error messages in bison at the cost of a few KB */
#define YYERROR_VERBOSE 1

/* YYLTYPE_IS_TRIVIAL and YYENABLE_NLS defined to suppress warnings */
#define YYLTYPE_IS_TRIVIAL 1
#define YYENABLE_NLS 0

/* The global pref settings */
WINPREFS pref;

/* The working menu */  
static MENUPARSED menu;

/* Functions for parsing the tokens into out structure */
/* Defined at the end section of this file */

static void SetIconDirectory (char *path);
static void SetDefaultIcon (char *fname);
static void SetRootMenu (char *menu);
static void SetDefaultSysMenu (char *menu, int pos);
static void SetTrayIcon (char *fname);

static void OpenMenu(char *menuname);
static void AddMenuLine(const char *name, MENUCOMMANDTYPE cmd, const char *param);
static void CloseMenu(void);

static void OpenIcons(void);
static void AddIconLine(char *matchstr, char *iconfile);
static void CloseIcons(void);

static void OpenStyles(void);
static void AddStyleLine(char *matchstr, unsigned long style);
static void CloseStyles(void);

static void OpenSysMenu(void);
static void AddSysMenuLine(char *matchstr, char *menuname, int pos);
static void CloseSysMenu(void);

static int yyerror (const char *s);

extern char *yytext;
extern int yylineno;
extern int yylex(void);


#line 160 "winprefsyacc.c"

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
#ifndef YY_YY_WINPREFSYACC_H_INCLUDED
# define YY_YY_WINPREFSYACC_H_INCLUDED
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
    NEWLINE = 258,                 /* NEWLINE  */
    MENU = 259,                    /* MENU  */
    LB = 260,                      /* LB  */
    RB = 261,                      /* RB  */
    ICONDIRECTORY = 262,           /* ICONDIRECTORY  */
    DEFAULTICON = 263,             /* DEFAULTICON  */
    ICONS = 264,                   /* ICONS  */
    STYLES = 265,                  /* STYLES  */
    TOPMOST = 266,                 /* TOPMOST  */
    MAXIMIZE = 267,                /* MAXIMIZE  */
    MINIMIZE = 268,                /* MINIMIZE  */
    BOTTOM = 269,                  /* BOTTOM  */
    NOTITLE = 270,                 /* NOTITLE  */
    OUTLINE = 271,                 /* OUTLINE  */
    NOFRAME = 272,                 /* NOFRAME  */
    DEFAULTSYSMENU = 273,          /* DEFAULTSYSMENU  */
    SYSMENU = 274,                 /* SYSMENU  */
    ROOTMENU = 275,                /* ROOTMENU  */
    SEPARATOR = 276,               /* SEPARATOR  */
    ATSTART = 277,                 /* ATSTART  */
    ATEND = 278,                   /* ATEND  */
    EXEC = 279,                    /* EXEC  */
    ALWAYSONTOP = 280,             /* ALWAYSONTOP  */
    DEBUGOUTPUT = 281,             /* "DEBUG"  */
    RELOAD = 282,                  /* RELOAD  */
    TRAYICON = 283,                /* TRAYICON  */
    FORCEEXIT = 284,               /* FORCEEXIT  */
    SILENTEXIT = 285,              /* SILENTEXIT  */
    STRING = 286                   /* STRING  */
  };
  typedef enum yytokentype yytoken_kind_t;
#endif
/* Token kinds.  */
#define YYEOF 0
#define YYerror 256
#define YYUNDEF 257
#define NEWLINE 258
#define MENU 259
#define LB 260
#define RB 261
#define ICONDIRECTORY 262
#define DEFAULTICON 263
#define ICONS 264
#define STYLES 265
#define TOPMOST 266
#define MAXIMIZE 267
#define MINIMIZE 268
#define BOTTOM 269
#define NOTITLE 270
#define OUTLINE 271
#define NOFRAME 272
#define DEFAULTSYSMENU 273
#define SYSMENU 274
#define ROOTMENU 275
#define SEPARATOR 276
#define ATSTART 277
#define ATEND 278
#define EXEC 279
#define ALWAYSONTOP 280
#define DEBUGOUTPUT 281
#define RELOAD 282
#define TRAYICON 283
#define FORCEEXIT 284
#define SILENTEXIT 285
#define STRING 286

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
union YYSTYPE
{
#line 90 "winprefsyacc.y"

  char *sVal;
  unsigned long uVal;
  int iVal;

#line 280 "winprefsyacc.c"

};
typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;

int yyparse (void);

#endif /* !YY_YY_WINPREFSYACC_H_INCLUDED  */
/* Symbol kind.  */
enum yysymbol_kind_t
{
  YYSYMBOL_YYEMPTY = -2,
  YYSYMBOL_YYEOF = 0,                      /* "end of file"  */
  YYSYMBOL_YYerror = 1,                    /* error  */
  YYSYMBOL_YYUNDEF = 2,                    /* "invalid token"  */
  YYSYMBOL_NEWLINE = 3,                    /* NEWLINE  */
  YYSYMBOL_MENU = 4,                       /* MENU  */
  YYSYMBOL_LB = 5,                         /* LB  */
  YYSYMBOL_RB = 6,                         /* RB  */
  YYSYMBOL_ICONDIRECTORY = 7,              /* ICONDIRECTORY  */
  YYSYMBOL_DEFAULTICON = 8,                /* DEFAULTICON  */
  YYSYMBOL_ICONS = 9,                      /* ICONS  */
  YYSYMBOL_STYLES = 10,                    /* STYLES  */
  YYSYMBOL_TOPMOST = 11,                   /* TOPMOST  */
  YYSYMBOL_MAXIMIZE = 12,                  /* MAXIMIZE  */
  YYSYMBOL_MINIMIZE = 13,                  /* MINIMIZE  */
  YYSYMBOL_BOTTOM = 14,                    /* BOTTOM  */
  YYSYMBOL_NOTITLE = 15,                   /* NOTITLE  */
  YYSYMBOL_OUTLINE = 16,                   /* OUTLINE  */
  YYSYMBOL_NOFRAME = 17,                   /* NOFRAME  */
  YYSYMBOL_DEFAULTSYSMENU = 18,            /* DEFAULTSYSMENU  */
  YYSYMBOL_SYSMENU = 19,                   /* SYSMENU  */
  YYSYMBOL_ROOTMENU = 20,                  /* ROOTMENU  */
  YYSYMBOL_SEPARATOR = 21,                 /* SEPARATOR  */
  YYSYMBOL_ATSTART = 22,                   /* ATSTART  */
  YYSYMBOL_ATEND = 23,                     /* ATEND  */
  YYSYMBOL_EXEC = 24,                      /* EXEC  */
  YYSYMBOL_ALWAYSONTOP = 25,               /* ALWAYSONTOP  */
  YYSYMBOL_DEBUGOUTPUT = 26,               /* "DEBUG"  */
  YYSYMBOL_RELOAD = 27,                    /* RELOAD  */
  YYSYMBOL_TRAYICON = 28,                  /* TRAYICON  */
  YYSYMBOL_FORCEEXIT = 29,                 /* FORCEEXIT  */
  YYSYMBOL_SILENTEXIT = 30,                /* SILENTEXIT  */
  YYSYMBOL_STRING = 31,                    /* STRING  */
  YYSYMBOL_YYACCEPT = 32,                  /* $accept  */
  YYSYMBOL_input = 33,                     /* input  */
  YYSYMBOL_line = 34,                      /* line  */
  YYSYMBOL_newline_or_nada = 35,           /* newline_or_nada  */
  YYSYMBOL_command = 36,                   /* command  */
  YYSYMBOL_trayicon = 37,                  /* trayicon  */
  YYSYMBOL_rootmenu = 38,                  /* rootmenu  */
  YYSYMBOL_defaultsysmenu = 39,            /* defaultsysmenu  */
  YYSYMBOL_defaulticon = 40,               /* defaulticon  */
  YYSYMBOL_icondirectory = 41,             /* icondirectory  */
  YYSYMBOL_menuline = 42,                  /* menuline  */
  YYSYMBOL_menulist = 43,                  /* menulist  */
  YYSYMBOL_menu = 44,                      /* menu  */
  YYSYMBOL_45_1 = 45,                      /* $@1  */
  YYSYMBOL_iconline = 46,                  /* iconline  */
  YYSYMBOL_iconlist = 47,                  /* iconlist  */
  YYSYMBOL_icons = 48,                     /* icons  */
  YYSYMBOL_49_2 = 49,                      /* $@2  */
  YYSYMBOL_group1 = 50,                    /* group1  */
  YYSYMBOL_group2 = 51,                    /* group2  */
  YYSYMBOL_stylecombo = 52,                /* stylecombo  */
  YYSYMBOL_styleline = 53,                 /* styleline  */
  YYSYMBOL_stylelist = 54,                 /* stylelist  */
  YYSYMBOL_styles = 55,                    /* styles  */
  YYSYMBOL_56_3 = 56,                      /* $@3  */
  YYSYMBOL_atspot = 57,                    /* atspot  */
  YYSYMBOL_sysmenuline = 58,               /* sysmenuline  */
  YYSYMBOL_sysmenulist = 59,               /* sysmenulist  */
  YYSYMBOL_sysmenu = 60,                   /* sysmenu  */
  YYSYMBOL_61_4 = 61,                      /* $@4  */
  YYSYMBOL_forceexit = 62,                 /* forceexit  */
  YYSYMBOL_silentexit = 63,                /* silentexit  */
  YYSYMBOL_debug = 64                      /* debug  */
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
#define YYFINAL  2
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   98

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  32
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  33
/* YYNRULES -- Number of rules.  */
#define YYNRULES  65
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  121

/* YYMAXUTOK -- Last valid token kind.  */
#define YYMAXUTOK   286


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
  /* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_uint8 yyrline[] =
{
       0,   133,   133,   134,   137,   138,   142,   143,   146,   147,
     148,   149,   150,   151,   152,   153,   154,   155,   156,   157,
     160,   163,   166,   169,   172,   175,   176,   177,   178,   179,
     182,   183,   186,   186,   189,   192,   193,   196,   196,   199,
     200,   201,   202,   205,   206,   207,   210,   211,   212,   213,
     216,   219,   220,   223,   223,   226,   227,   228,   231,   234,
     235,   238,   238,   241,   244,   247
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
  "\"end of file\"", "error", "\"invalid token\"", "NEWLINE", "MENU",
  "LB", "RB", "ICONDIRECTORY", "DEFAULTICON", "ICONS", "STYLES", "TOPMOST",
  "MAXIMIZE", "MINIMIZE", "BOTTOM", "NOTITLE", "OUTLINE", "NOFRAME",
  "DEFAULTSYSMENU", "SYSMENU", "ROOTMENU", "SEPARATOR", "ATSTART", "ATEND",
  "EXEC", "ALWAYSONTOP", "\"DEBUG\"", "RELOAD", "TRAYICON", "FORCEEXIT",
  "SILENTEXIT", "STRING", "$accept", "input", "line", "newline_or_nada",
  "command", "trayicon", "rootmenu", "defaultsysmenu", "defaulticon",
  "icondirectory", "menuline", "menulist", "menu", "$@1", "iconline",
  "iconlist", "icons", "$@2", "group1", "group2", "stylecombo",
  "styleline", "stylelist", "styles", "$@3", "atspot", "sysmenuline",
  "sysmenulist", "sysmenu", "$@4", "forceexit", "silentexit", "debug", YY_NULLPTR
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
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286
};
#endif

#define YYPACT_NINF (-47)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-1)

#define yytable_value_is_error(Yyn) \
  0

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int8 yypact[] =
{
     -47,     7,   -47,   -47,    -1,     0,     1,    18,    29,    15,
      42,    17,    19,    20,    46,    50,   -47,   -47,   -47,   -47,
     -47,   -47,   -47,   -47,   -47,   -47,   -47,   -47,   -47,   -47,
      49,    53,    54,   -47,   -47,     6,    55,    56,    57,    58,
     -47,   -47,   -47,   -47,   -47,    61,    61,   -47,   -47,    62,
     -47,   -47,   -47,   -47,    61,    61,    35,    38,   -47,    61,
     -19,   -47,    39,    35,    66,    27,    38,    67,    43,    72,
      -3,   -19,    70,    74,   -47,   -47,   -47,   -47,   -47,   -47,
     -47,   -47,   -47,     3,    -8,    75,   -47,   -47,    48,    43,
      76,    61,    52,    59,    77,    78,   -47,   -47,    61,   -47,
     -47,    61,     6,   -47,   -47,   -47,    81,    82,    61,    61,
     -47,   -47,    83,    61,    61,   -47,   -47,    61,   -47,   -47,
     -47
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_int8 yydefact[] =
{
       2,     0,     1,     4,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     3,     5,    17,    14,
      15,     8,     9,    10,    11,    12,    13,    18,    19,    16,
       0,     0,     0,    37,    53,    55,     0,     0,     0,     0,
      63,    64,    32,    24,    23,     6,     6,    56,    57,     0,
      61,    21,    65,    20,     6,     6,     0,     0,    22,     6,
       0,     7,     0,    35,     0,     0,    51,     0,     0,     0,
       0,    30,     0,     0,    36,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,     0,    52,    54,     0,    59,
       0,     6,     0,     0,     0,     0,    31,    33,     6,    48,
      49,     6,    55,    60,    62,    25,     0,     0,     6,     6,
      34,    50,     0,     6,     6,    26,    29,     6,    28,    27,
      58
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int8 yypgoto[] =
{
     -47,   -47,   -47,   -46,   -47,   -47,   -47,   -47,   -47,   -47,
     -47,    16,   -47,   -47,   -47,    25,   -47,   -47,     5,     8,
     -47,   -47,    26,   -47,   -47,    -9,   -47,     9,   -47,   -47,
     -47,   -47,   -47
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int8 yydefgoto[] =
{
      -1,     1,    16,    56,    17,    18,    19,    20,    21,    22,
      71,    72,    23,    54,    63,    64,    24,    45,    83,    84,
      85,    66,    67,    25,    46,    49,    89,    90,    26,    59,
      27,    28,    29
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int8 yytable[] =
{
      57,    92,    69,    76,    77,    78,    79,     2,    60,    61,
       3,     4,    70,    68,     5,     6,     7,     8,    80,    81,
      82,    93,    94,    33,    95,     9,    10,    11,    47,    48,
      30,    31,    32,    12,    34,    13,    14,    15,    76,    77,
      78,    79,    80,    81,    82,   105,    35,    36,    37,    40,
      38,    39,   110,    41,    42,   111,    43,    44,    50,    51,
      52,    53,   115,   116,    55,    58,    62,   118,   119,    65,
      73,   120,    75,    87,    88,    91,    97,    98,   101,   102,
     108,   109,   104,   106,   113,   114,   117,    96,    74,   100,
     107,    99,    86,   112,     0,     0,     0,     0,   103
};

static const yytype_int8 yycheck[] =
{
      46,     4,    21,    11,    12,    13,    14,     0,    54,    55,
       3,     4,    31,    59,     7,     8,     9,    10,    15,    16,
      17,    24,    25,     5,    27,    18,    19,    20,    22,    23,
      31,    31,    31,    26,     5,    28,    29,    30,    11,    12,
      13,    14,    15,    16,    17,    91,    31,     5,    31,     3,
      31,    31,    98,     3,     5,   101,     3,     3,     3,     3,
       3,     3,   108,   109,     3,     3,    31,   113,   114,    31,
      31,   117,     6,     6,    31,     3,     6,     3,     3,    31,
       3,     3,     6,    31,     3,     3,     3,    71,    63,    84,
      31,    83,    66,   102,    -1,    -1,    -1,    -1,    89
};

  /* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
static const yytype_int8 yystos[] =
{
       0,    33,     0,     3,     4,     7,     8,     9,    10,    18,
      19,    20,    26,    28,    29,    30,    34,    36,    37,    38,
      39,    40,    41,    44,    48,    55,    60,    62,    63,    64,
      31,    31,    31,     5,     5,    31,     5,    31,    31,    31,
       3,     3,     5,     3,     3,    49,    56,    22,    23,    57,
       3,     3,     3,     3,    45,     3,    35,    35,     3,    61,
      35,    35,    31,    46,    47,    31,    53,    54,    35,    21,
      31,    42,    43,    31,    47,     6,    11,    12,    13,    14,
      15,    16,    17,    50,    51,    52,    54,     6,    31,    58,
      59,     3,     4,    24,    25,    27,    43,     6,     3,    51,
      50,     3,    31,    59,     6,    35,    31,    31,     3,     3,
      35,    35,    57,     3,     3,    35,    35,     3,    35,    35,
      35
};

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_int8 yyr1[] =
{
       0,    32,    33,    33,    34,    34,    35,    35,    36,    36,
      36,    36,    36,    36,    36,    36,    36,    36,    36,    36,
      37,    38,    39,    40,    41,    42,    42,    42,    42,    42,
      43,    43,    45,    44,    46,    47,    47,    49,    48,    50,
      50,    50,    50,    51,    51,    51,    52,    52,    52,    52,
      53,    54,    54,    56,    55,    57,    57,    57,    58,    59,
      59,    61,    60,    62,    63,    64
};

  /* YYR2[YYN] -- Number of symbols on the right hand side of rule YYN.  */
static const yytype_int8 yyr2[] =
{
       0,     2,     0,     2,     1,     1,     0,     2,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       3,     3,     4,     3,     3,     3,     4,     5,     5,     4,
       1,     2,     0,     7,     4,     1,     2,     0,     6,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     2,     2,
       4,     1,     2,     0,     6,     0,     1,     1,     5,     1,
       2,     0,     7,     2,     2,     3
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
  case 20: /* trayicon: TRAYICON STRING NEWLINE  */
#line 160 "winprefsyacc.y"
                                        { SetTrayIcon((yyvsp[-1].sVal)); free((yyvsp[-1].sVal)); }
#line 1395 "winprefsyacc.c"
    break;

  case 21: /* rootmenu: ROOTMENU STRING NEWLINE  */
#line 163 "winprefsyacc.y"
                                        { SetRootMenu((yyvsp[-1].sVal)); free((yyvsp[-1].sVal)); }
#line 1401 "winprefsyacc.c"
    break;

  case 22: /* defaultsysmenu: DEFAULTSYSMENU STRING atspot NEWLINE  */
#line 166 "winprefsyacc.y"
                                                     { SetDefaultSysMenu((yyvsp[-2].sVal), (yyvsp[-1].iVal)); free((yyvsp[-2].sVal)); }
#line 1407 "winprefsyacc.c"
    break;

  case 23: /* defaulticon: DEFAULTICON STRING NEWLINE  */
#line 169 "winprefsyacc.y"
                                           { SetDefaultIcon((yyvsp[-1].sVal)); free((yyvsp[-1].sVal)); }
#line 1413 "winprefsyacc.c"
    break;

  case 24: /* icondirectory: ICONDIRECTORY STRING NEWLINE  */
#line 172 "winprefsyacc.y"
                                             { SetIconDirectory((yyvsp[-1].sVal)); free((yyvsp[-1].sVal)); }
#line 1419 "winprefsyacc.c"
    break;

  case 25: /* menuline: SEPARATOR NEWLINE newline_or_nada  */
#line 175 "winprefsyacc.y"
                                                   { AddMenuLine("-", CMD_SEPARATOR, ""); }
#line 1425 "winprefsyacc.c"
    break;

  case 26: /* menuline: STRING ALWAYSONTOP NEWLINE newline_or_nada  */
#line 176 "winprefsyacc.y"
                                                      { AddMenuLine((yyvsp[-3].sVal), CMD_ALWAYSONTOP, ""); free((yyvsp[-3].sVal)); }
#line 1431 "winprefsyacc.c"
    break;

  case 27: /* menuline: STRING EXEC STRING NEWLINE newline_or_nada  */
#line 177 "winprefsyacc.y"
                                                      { AddMenuLine((yyvsp[-4].sVal), CMD_EXEC, (yyvsp[-2].sVal)); free((yyvsp[-4].sVal)); free((yyvsp[-2].sVal)); }
#line 1437 "winprefsyacc.c"
    break;

  case 28: /* menuline: STRING MENU STRING NEWLINE newline_or_nada  */
#line 178 "winprefsyacc.y"
                                                      { AddMenuLine((yyvsp[-4].sVal), CMD_MENU, (yyvsp[-2].sVal)); free((yyvsp[-4].sVal)); free((yyvsp[-2].sVal)); }
#line 1443 "winprefsyacc.c"
    break;

  case 29: /* menuline: STRING RELOAD NEWLINE newline_or_nada  */
#line 179 "winprefsyacc.y"
                                                 { AddMenuLine((yyvsp[-3].sVal), CMD_RELOAD, ""); free((yyvsp[-3].sVal)); }
#line 1449 "winprefsyacc.c"
    break;

  case 32: /* $@1: %empty  */
#line 186 "winprefsyacc.y"
                       { OpenMenu((yyvsp[-1].sVal)); free((yyvsp[-1].sVal)); }
#line 1455 "winprefsyacc.c"
    break;

  case 33: /* menu: MENU STRING LB $@1 newline_or_nada menulist RB  */
#line 186 "winprefsyacc.y"
                                                                               {CloseMenu();}
#line 1461 "winprefsyacc.c"
    break;

  case 34: /* iconline: STRING STRING NEWLINE newline_or_nada  */
#line 189 "winprefsyacc.y"
                                                      { AddIconLine((yyvsp[-3].sVal), (yyvsp[-2].sVal)); free((yyvsp[-3].sVal)); free((yyvsp[-2].sVal)); }
#line 1467 "winprefsyacc.c"
    break;

  case 37: /* $@2: %empty  */
#line 196 "winprefsyacc.y"
                 {OpenIcons();}
#line 1473 "winprefsyacc.c"
    break;

  case 38: /* icons: ICONS LB $@2 newline_or_nada iconlist RB  */
#line 196 "winprefsyacc.y"
                                                            {CloseIcons();}
#line 1479 "winprefsyacc.c"
    break;

  case 39: /* group1: TOPMOST  */
#line 199 "winprefsyacc.y"
                { (yyval.uVal)=STYLE_TOPMOST; }
#line 1485 "winprefsyacc.c"
    break;

  case 40: /* group1: MAXIMIZE  */
#line 200 "winprefsyacc.y"
                   { (yyval.uVal)=STYLE_MAXIMIZE; }
#line 1491 "winprefsyacc.c"
    break;

  case 41: /* group1: MINIMIZE  */
#line 201 "winprefsyacc.y"
                   { (yyval.uVal)=STYLE_MINIMIZE; }
#line 1497 "winprefsyacc.c"
    break;

  case 42: /* group1: BOTTOM  */
#line 202 "winprefsyacc.y"
                 { (yyval.uVal)=STYLE_BOTTOM; }
#line 1503 "winprefsyacc.c"
    break;

  case 43: /* group2: NOTITLE  */
#line 205 "winprefsyacc.y"
                { (yyval.uVal)=STYLE_NOTITLE; }
#line 1509 "winprefsyacc.c"
    break;

  case 44: /* group2: OUTLINE  */
#line 206 "winprefsyacc.y"
                  { (yyval.uVal)=STYLE_OUTLINE; }
#line 1515 "winprefsyacc.c"
    break;

  case 45: /* group2: NOFRAME  */
#line 207 "winprefsyacc.y"
                  { (yyval.uVal)=STYLE_NOFRAME; }
#line 1521 "winprefsyacc.c"
    break;

  case 46: /* stylecombo: group1  */
#line 210 "winprefsyacc.y"
                       { (yyval.uVal)=(yyvsp[0].uVal); }
#line 1527 "winprefsyacc.c"
    break;

  case 47: /* stylecombo: group2  */
#line 211 "winprefsyacc.y"
                 { (yyval.uVal)=(yyvsp[0].uVal); }
#line 1533 "winprefsyacc.c"
    break;

  case 48: /* stylecombo: group1 group2  */
#line 212 "winprefsyacc.y"
                        { (yyval.uVal)=(yyvsp[-1].uVal)|(yyvsp[0].uVal); }
#line 1539 "winprefsyacc.c"
    break;

  case 49: /* stylecombo: group2 group1  */
#line 213 "winprefsyacc.y"
                        { (yyval.uVal)=(yyvsp[-1].uVal)|(yyvsp[0].uVal); }
#line 1545 "winprefsyacc.c"
    break;

  case 50: /* styleline: STRING stylecombo NEWLINE newline_or_nada  */
#line 216 "winprefsyacc.y"
                                                          { AddStyleLine((yyvsp[-3].sVal), (yyvsp[-2].uVal)); free((yyvsp[-3].sVal)); }
#line 1551 "winprefsyacc.c"
    break;

  case 53: /* $@3: %empty  */
#line 223 "winprefsyacc.y"
                  {OpenStyles();}
#line 1557 "winprefsyacc.c"
    break;

  case 54: /* styles: STYLES LB $@3 newline_or_nada stylelist RB  */
#line 223 "winprefsyacc.y"
                                                               {CloseStyles();}
#line 1563 "winprefsyacc.c"
    break;

  case 55: /* atspot: %empty  */
#line 226 "winprefsyacc.y"
        { (yyval.iVal)=AT_END; }
#line 1569 "winprefsyacc.c"
    break;

  case 56: /* atspot: ATSTART  */
#line 227 "winprefsyacc.y"
                  { (yyval.iVal)=AT_START; }
#line 1575 "winprefsyacc.c"
    break;

  case 57: /* atspot: ATEND  */
#line 228 "winprefsyacc.y"
                { (yyval.iVal)=AT_END; }
#line 1581 "winprefsyacc.c"
    break;

  case 58: /* sysmenuline: STRING STRING atspot NEWLINE newline_or_nada  */
#line 231 "winprefsyacc.y"
                                                             { AddSysMenuLine((yyvsp[-4].sVal), (yyvsp[-3].sVal), (yyvsp[-2].iVal)); free((yyvsp[-4].sVal)); free((yyvsp[-3].sVal)); }
#line 1587 "winprefsyacc.c"
    break;

  case 61: /* $@4: %empty  */
#line 238 "winprefsyacc.y"
                                   {OpenSysMenu();}
#line 1593 "winprefsyacc.c"
    break;

  case 62: /* sysmenu: SYSMENU LB NEWLINE $@4 newline_or_nada sysmenulist RB  */
#line 238 "winprefsyacc.y"
                                                                                   {CloseSysMenu();}
#line 1599 "winprefsyacc.c"
    break;

  case 63: /* forceexit: FORCEEXIT NEWLINE  */
#line 241 "winprefsyacc.y"
                                  { pref.fForceExit = TRUE; }
#line 1605 "winprefsyacc.c"
    break;

  case 64: /* silentexit: SILENTEXIT NEWLINE  */
#line 244 "winprefsyacc.y"
                                   { pref.fSilentExit = TRUE; }
#line 1611 "winprefsyacc.c"
    break;

  case 65: /* debug: "DEBUG" STRING NEWLINE  */
#line 247 "winprefsyacc.y"
                                   { ErrorF("LoadPreferences: %s\n", (yyvsp[-1].sVal)); free((yyvsp[-1].sVal)); }
#line 1617 "winprefsyacc.c"
    break;


#line 1621 "winprefsyacc.c"

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

#line 251 "winprefsyacc.y"

/*
 * Errors in parsing abort and print log messages
 */
static int
yyerror (const char *s)
{
  ErrorF("LoadPreferences: %s line %d\n", s, yylineno);
  return 1;
}

/* Miscellaneous functions to store TOKENs into the structure */
static void
SetIconDirectory (char *path)
{
  strncpy (pref.iconDirectory, path, PATH_MAX);
  pref.iconDirectory[PATH_MAX] = 0;
}

static void
SetDefaultIcon (char *fname)
{
  strncpy (pref.defaultIconName, fname, NAME_MAX);
  pref.defaultIconName[NAME_MAX] = 0;
}

static void
SetTrayIcon (char *fname)
{
  strncpy (pref.trayIconName, fname, NAME_MAX);
  pref.trayIconName[NAME_MAX] = 0;
}

static void
SetRootMenu (char *menuname)
{
  strncpy (pref.rootMenuName, menuname, MENU_MAX);
  pref.rootMenuName[MENU_MAX] = 0;
}

static void
SetDefaultSysMenu (char *menuname, int pos)
{
  strncpy (pref.defaultSysMenuName, menuname, MENU_MAX);
  pref.defaultSysMenuName[MENU_MAX] = 0;
  pref.defaultSysMenuPos = pos;
}

static void
OpenMenu (char *menuname)
{
  if (menu.menuItem) free(menu.menuItem);
  menu.menuItem = NULL;
  strncpy(menu.menuName, menuname, MENU_MAX);
  menu.menuName[MENU_MAX] = 0;
  menu.menuItems = 0;
}

static void
AddMenuLine (const char *text, MENUCOMMANDTYPE cmd, const char *param)
{
  if (menu.menuItem==NULL)
    menu.menuItem = malloc(sizeof(MENUITEM));
  else
    menu.menuItem = realloc(menu.menuItem, sizeof(MENUITEM)*(menu.menuItems+1));

  strncpy (menu.menuItem[menu.menuItems].text, text, MENU_MAX);
  menu.menuItem[menu.menuItems].text[MENU_MAX] = 0;

  menu.menuItem[menu.menuItems].cmd = cmd;

  strncpy(menu.menuItem[menu.menuItems].param, param, PARAM_MAX);
  menu.menuItem[menu.menuItems].param[PARAM_MAX] = 0;

  menu.menuItem[menu.menuItems].commandID = 0;

  menu.menuItems++;
}

static void
CloseMenu (void)
{
  if (menu.menuItem==NULL || menu.menuItems==0)
    {
      ErrorF("LoadPreferences: Empty menu detected\n");
      return;
    }
  
  if (pref.menuItems)
    pref.menu = realloc (pref.menu, (pref.menuItems+1)*sizeof(MENUPARSED));
  else
    pref.menu = malloc (sizeof(MENUPARSED));
  
  memcpy (pref.menu+pref.menuItems, &menu, sizeof(MENUPARSED));
  pref.menuItems++;

  memset (&menu, 0, sizeof(MENUPARSED));
}

static void 
OpenIcons (void)
{
  if (pref.icon != NULL) {
    ErrorF("LoadPreferences: Redefining icon mappings\n");
    free(pref.icon);
    pref.icon = NULL;
  }
  pref.iconItems = 0;
}

static void
AddIconLine (char *matchstr, char *iconfile)
{
  if (pref.icon==NULL)
    pref.icon = malloc(sizeof(ICONITEM));
  else
    pref.icon = realloc(pref.icon, sizeof(ICONITEM)*(pref.iconItems+1));

  strncpy(pref.icon[pref.iconItems].match, matchstr, MENU_MAX);
  pref.icon[pref.iconItems].match[MENU_MAX] = 0;

  strncpy(pref.icon[pref.iconItems].iconFile, iconfile, PATH_MAX+NAME_MAX+1);
  pref.icon[pref.iconItems].iconFile[PATH_MAX+NAME_MAX+1] = 0;

  pref.icon[pref.iconItems].hicon = 0;

  pref.iconItems++;
}

static void 
CloseIcons (void)
{
}

static void
OpenStyles (void)
{
  if (pref.style != NULL) {
    ErrorF("LoadPreferences: Redefining window style\n");
    free(pref.style);
    pref.style = NULL;
  }
  pref.styleItems = 0;
}

static void
AddStyleLine (char *matchstr, unsigned long style)
{
  if (pref.style==NULL)
    pref.style = malloc(sizeof(STYLEITEM));
  else
    pref.style = realloc(pref.style, sizeof(STYLEITEM)*(pref.styleItems+1));

  strncpy(pref.style[pref.styleItems].match, matchstr, MENU_MAX);
  pref.style[pref.styleItems].match[MENU_MAX] = 0;

  pref.style[pref.styleItems].type = style;

  pref.styleItems++;
}

static void
CloseStyles (void)
{
}

static void
OpenSysMenu (void)
{
  if (pref.sysMenu != NULL) {
    ErrorF("LoadPreferences: Redefining system menu\n");
    free(pref.sysMenu);
    pref.sysMenu = NULL;
  }
  pref.sysMenuItems = 0;
}

static void
AddSysMenuLine (char *matchstr, char *menuname, int pos)
{
  if (pref.sysMenu==NULL)
    pref.sysMenu = malloc(sizeof(SYSMENUITEM));
  else
    pref.sysMenu = realloc(pref.sysMenu, sizeof(SYSMENUITEM)*(pref.sysMenuItems+1));

  strncpy (pref.sysMenu[pref.sysMenuItems].match, matchstr, MENU_MAX);
  pref.sysMenu[pref.sysMenuItems].match[MENU_MAX] = 0;

  strncpy (pref.sysMenu[pref.sysMenuItems].menuName, menuname, MENU_MAX);
  pref.sysMenu[pref.sysMenuItems].menuName[MENU_MAX] = 0;

  pref.sysMenu[pref.sysMenuItems].menuPos = pos;

  pref.sysMenuItems++;
}

static void
CloseSysMenu (void)
{
}

