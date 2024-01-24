/* A Bison parser, made by GNU Bison 3.7.3.  */

/* Bison interface for Yacc-like parsers in C

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

/* DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
   especially those whose name start with YY_ or yy_.  They are
   private implementation details that can be changed or removed.  */

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

#line 134 "winprefsyacc.h"

};
typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;

int yyparse (void);

#endif /* !YY_YY_WINPREFSYACC_H_INCLUDED  */
