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

#line 109 "parser.h"

};
typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;

int yyparse (void);

#endif /* !YY_YY_PARSER_H_INCLUDED  */
