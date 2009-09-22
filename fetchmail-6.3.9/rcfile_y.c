/* A Bison parser, made by GNU Bison 2.3.  */

/* Skeleton implementation for Bison's Yacc-like parsers in C

   Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003, 2004, 2005, 2006
   Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.  */

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
#define YYBISON_VERSION "2.3"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Using locations.  */
#define YYLSP_NEEDED 0



/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     DEFAULTS = 258,
     POLL = 259,
     SKIP = 260,
     VIA = 261,
     AKA = 262,
     LOCALDOMAINS = 263,
     PROTOCOL = 264,
     AUTHENTICATE = 265,
     TIMEOUT = 266,
     KPOP = 267,
     SDPS = 268,
     ENVELOPE = 269,
     QVIRTUAL = 270,
     USERNAME = 271,
     PASSWORD = 272,
     FOLDER = 273,
     SMTPHOST = 274,
     FETCHDOMAINS = 275,
     MDA = 276,
     BSMTP = 277,
     LMTP = 278,
     SMTPADDRESS = 279,
     SMTPNAME = 280,
     SPAMRESPONSE = 281,
     PRECONNECT = 282,
     POSTCONNECT = 283,
     LIMIT = 284,
     WARNINGS = 285,
     INTERFACE = 286,
     MONITOR = 287,
     PLUGIN = 288,
     PLUGOUT = 289,
     IS = 290,
     HERE = 291,
     THERE = 292,
     TO = 293,
     MAP = 294,
     WILDCARD = 295,
     BATCHLIMIT = 296,
     FETCHLIMIT = 297,
     FETCHSIZELIMIT = 298,
     FASTUIDL = 299,
     EXPUNGE = 300,
     PROPERTIES = 301,
     SET = 302,
     LOGFILE = 303,
     DAEMON = 304,
     SYSLOG = 305,
     IDFILE = 306,
     PIDFILE = 307,
     INVISIBLE = 308,
     POSTMASTER = 309,
     BOUNCEMAIL = 310,
     SPAMBOUNCE = 311,
     SHOWDOTS = 312,
     PROTO = 313,
     AUTHTYPE = 314,
     STRING = 315,
     NUMBER = 316,
     NO = 317,
     KEEP = 318,
     FLUSH = 319,
     LIMITFLUSH = 320,
     FETCHALL = 321,
     REWRITE = 322,
     FORCECR = 323,
     STRIPCR = 324,
     PASS8BITS = 325,
     DROPSTATUS = 326,
     DROPDELIVERED = 327,
     DNS = 328,
     SERVICE = 329,
     PORT = 330,
     UIDL = 331,
     INTERVAL = 332,
     MIMEDECODE = 333,
     IDLE = 334,
     CHECKALIAS = 335,
     SSL = 336,
     SSLKEY = 337,
     SSLCERT = 338,
     SSLPROTO = 339,
     SSLCERTCK = 340,
     SSLCERTPATH = 341,
     SSLCOMMONNAME = 342,
     SSLFINGERPRINT = 343,
     PRINCIPAL = 344,
     ESMTPNAME = 345,
     ESMTPPASSWORD = 346,
     AUTHMETH = 347,
     TRACEPOLLS = 348
   };
#endif
/* Tokens.  */
#define DEFAULTS 258
#define POLL 259
#define SKIP 260
#define VIA 261
#define AKA 262
#define LOCALDOMAINS 263
#define PROTOCOL 264
#define AUTHENTICATE 265
#define TIMEOUT 266
#define KPOP 267
#define SDPS 268
#define ENVELOPE 269
#define QVIRTUAL 270
#define USERNAME 271
#define PASSWORD 272
#define FOLDER 273
#define SMTPHOST 274
#define FETCHDOMAINS 275
#define MDA 276
#define BSMTP 277
#define LMTP 278
#define SMTPADDRESS 279
#define SMTPNAME 280
#define SPAMRESPONSE 281
#define PRECONNECT 282
#define POSTCONNECT 283
#define LIMIT 284
#define WARNINGS 285
#define INTERFACE 286
#define MONITOR 287
#define PLUGIN 288
#define PLUGOUT 289
#define IS 290
#define HERE 291
#define THERE 292
#define TO 293
#define MAP 294
#define WILDCARD 295
#define BATCHLIMIT 296
#define FETCHLIMIT 297
#define FETCHSIZELIMIT 298
#define FASTUIDL 299
#define EXPUNGE 300
#define PROPERTIES 301
#define SET 302
#define LOGFILE 303
#define DAEMON 304
#define SYSLOG 305
#define IDFILE 306
#define PIDFILE 307
#define INVISIBLE 308
#define POSTMASTER 309
#define BOUNCEMAIL 310
#define SPAMBOUNCE 311
#define SHOWDOTS 312
#define PROTO 313
#define AUTHTYPE 314
#define STRING 315
#define NUMBER 316
#define NO 317
#define KEEP 318
#define FLUSH 319
#define LIMITFLUSH 320
#define FETCHALL 321
#define REWRITE 322
#define FORCECR 323
#define STRIPCR 324
#define PASS8BITS 325
#define DROPSTATUS 326
#define DROPDELIVERED 327
#define DNS 328
#define SERVICE 329
#define PORT 330
#define UIDL 331
#define INTERVAL 332
#define MIMEDECODE 333
#define IDLE 334
#define CHECKALIAS 335
#define SSL 336
#define SSLKEY 337
#define SSLCERT 338
#define SSLPROTO 339
#define SSLCERTCK 340
#define SSLCERTPATH 341
#define SSLCOMMONNAME 342
#define SSLFINGERPRINT 343
#define PRINCIPAL 344
#define ESMTPNAME 345
#define ESMTPPASSWORD 346
#define TRACEPOLLS 347




/* Copy the first part of user declarations.  */
#line 1 "../rcfile_y.y"

/*
 * rcfile_y.y -- Run control file parser for fetchmail
 *
 * For license terms, see the file COPYING in this directory.
 */

#include "config.h"
#include <stdio.h>
#include <sys/types.h>
#include <sys/file.h>
#if defined(HAVE_SYS_WAIT_H)
#include <sys/wait.h>
#endif
#include <sys/stat.h>
#include <errno.h>
#if defined(STDC_HEADERS)
#include <stdlib.h>
#endif
#if defined(HAVE_UNISTD_H)
#include <unistd.h>
#endif
#include <string.h>

#if defined(__CYGWIN__)
#include <sys/cygwin.h>
#endif /* __CYGWIN__ */

#include "fetchmail.h"
#include "i18n.h"
  
/* parser reads these */
char *rcfile;			/* path name of rc file */
struct query cmd_opts;		/* where to put command-line info */

/* parser sets these */
struct query *querylist;	/* head of server list (globally visible) */

int yydebug;			/* in case we didn't generate with -- debug */

static struct query current;	/* current server record */
static int prc_errflag;
static struct hostdata *leadentry;
static flag trailer;

static void record_current(void);
static void user_reset(void);
static void reset_server(const char *name, int skip);

/* these should be of size PATH_MAX */
char currentwd[1024] = "", rcfiledir[1024] = "";

/* using Bison, this arranges that yydebug messages will show actual tokens */
extern char * yytext;
#define YYPRINT(fp, type, val)	fprintf(fp, " = \"%s\"", yytext)


/* Enabling traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 0
#endif

/* Enabling the token table.  */
#ifndef YYTOKEN_TABLE
# define YYTOKEN_TABLE 0
#endif

#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
#line 58 "../rcfile_y.y"
{
  int proto;
  int number;
  char *sval;
}
/* Line 193 of yacc.c.  */
#line 343 "rcfile_y.c"
	YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif



/* Copy the second part of user declarations.  */


/* Line 216 of yacc.c.  */
#line 356 "rcfile_y.c"

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
# if YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(msgid) dgettext ("bison-runtime", msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(msgid) msgid
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YYUSE(e) ((void) (e))
#else
# define YYUSE(e) /* empty */
#endif

/* Identity function, used to suppress warnings about constant conditions.  */
#ifndef lint
# define YYID(n) (n)
#else
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static int
YYID (int i)
#else
static int
YYID (i)
    int i;
#endif
{
  return i;
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
#    if ! defined _ALLOCA_H && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#     ifndef _STDLIB_H
#      define _STDLIB_H 1
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
#  if (defined __cplusplus && ! defined _STDLIB_H \
       && ! ((defined YYMALLOC || defined malloc) \
	     && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef _STDLIB_H
#    define _STDLIB_H 1
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
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
  yytype_int16 yyss;
  YYSTYPE yyvs;
  };

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (yytype_int16) + sizeof (YYSTYPE)) \
      + YYSTACK_GAP_MAXIMUM)

/* Copy COUNT objects from FROM to TO.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(To, From, Count) \
      __builtin_memcpy (To, From, (Count) * sizeof (*(From)))
#  else
#   define YYCOPY(To, From, Count)		\
      do					\
	{					\
	  YYSIZE_T yyi;				\
	  for (yyi = 0; yyi < (Count); yyi++)	\
	    (To)[yyi] = (From)[yyi];		\
	}					\
      while (YYID (0))
#  endif
# endif

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack)					\
    do									\
      {									\
	YYSIZE_T yynewbytes;						\
	YYCOPY (&yyptr->Stack, Stack, yysize);				\
	Stack = &yyptr->Stack;						\
	yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
	yyptr += yynewbytes / sizeof (*yyptr);				\
      }									\
    while (YYID (0))

#endif

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  23
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   297

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  93
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  24
/* YYNRULES -- Number of rules.  */
#define YYNRULES  151
/* YYNRULES -- Number of states.  */
#define YYNSTATES  220

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   347

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const yytype_uint8 yytranslate[] =
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
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    81,    82,    83,    84,
      85,    86,    87,    88,    89,    90,    91,    92
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const yytype_uint16 yyprhs[] =
{
       0,     0,     3,     4,     6,     8,    11,    13,    14,    19,
      24,    29,    34,    39,    42,    46,    49,    53,    58,    61,
      65,    68,    72,    75,    79,    82,    86,    91,    94,    97,
      99,   100,   103,   105,   108,   110,   113,   116,   119,   122,
     125,   128,   131,   134,   137,   140,   142,   145,   147,   150,
     153,   156,   159,   162,   165,   168,   172,   175,   178,   181,
     184,   187,   190,   192,   195,   198,   200,   203,   205,   207,
     209,   212,   215,   218,   222,   226,   227,   230,   232,   235,
     237,   239,   242,   244,   247,   249,   253,   255,   258,   260,
     263,   265,   268,   270,   273,   277,   280,   284,   287,   291,
     294,   297,   300,   303,   306,   309,   312,   315,   318,   320,
     323,   326,   328,   330,   332,   334,   336,   338,   340,   342,
     344,   346,   348,   350,   352,   355,   358,   361,   363,   366,
     369,   372,   375,   378,   381,   384,   387,   390,   393,   396,
     399,   402,   405,   408,   411,   414,   417,   420,   423,   426,
     429,   432
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int8 yyrhs[] =
{
      94,     0,    -1,    -1,    95,    -1,    97,    -1,    95,    97,
      -1,    39,    -1,    -1,    47,    48,    96,    60,    -1,    47,
      51,    96,    60,    -1,    47,    52,    96,    60,    -1,    47,
      49,    96,    61,    -1,    47,    54,    96,    60,    -1,    47,
      55,    -1,    47,    62,    55,    -1,    47,    56,    -1,    47,
      62,    56,    -1,    47,    46,    96,    60,    -1,    47,    50,
      -1,    47,    62,    50,    -1,    47,    53,    -1,    47,    62,
      53,    -1,    47,    57,    -1,    47,    62,    57,    -1,    98,
      99,    -1,    98,    99,   103,    -1,    98,    99,   103,   102,
      -1,     4,    60,    -1,     5,    60,    -1,     3,    -1,    -1,
      99,   102,    -1,    60,    -1,   100,    60,    -1,    60,    -1,
     101,    60,    -1,     7,   100,    -1,     6,    60,    -1,     8,
     101,    -1,     9,    58,    -1,     9,    12,    -1,    89,    60,
      -1,    90,    60,    -1,    91,    60,    -1,     9,    13,    -1,
      76,    -1,    62,    76,    -1,    80,    -1,    62,    80,    -1,
      74,    60,    -1,    74,    61,    -1,    75,    61,    -1,    77,
      61,    -1,    10,    59,    -1,    11,    61,    -1,    14,    61,
      60,    -1,    14,    60,    -1,    15,    60,    -1,    31,    60,
      -1,    32,    60,    -1,    33,    60,    -1,    34,    60,    -1,
      73,    -1,    62,    73,    -1,    62,    14,    -1,    92,    -1,
      62,    92,    -1,   108,    -1,   104,    -1,   105,    -1,   104,
     105,    -1,   106,   107,    -1,    16,    60,    -1,    16,   110,
      36,    -1,    16,    60,    37,    -1,    -1,   107,   116,    -1,
     116,    -1,   108,   116,    -1,    40,    -1,   110,    -1,   110,
      40,    -1,   111,    -1,   110,   111,    -1,    60,    -1,    60,
      39,    60,    -1,    60,    -1,   112,    60,    -1,    60,    -1,
     113,    60,    -1,    60,    -1,   114,    60,    -1,    61,    -1,
     115,    61,    -1,    38,   109,    36,    -1,    38,   109,    -1,
      35,   109,    36,    -1,    35,   109,    -1,    35,    60,    37,
      -1,    17,    60,    -1,    18,   112,    -1,    19,   113,    -1,
      20,   114,    -1,    24,    60,    -1,    25,    60,    -1,    26,
     115,    -1,    21,    60,    -1,    22,    60,    -1,    23,    -1,
      27,    60,    -1,    28,    60,    -1,    63,    -1,    64,    -1,
      65,    -1,    66,    -1,    67,    -1,    68,    -1,    69,    -1,
      70,    -1,    71,    -1,    72,    -1,    78,    -1,    79,    -1,
      81,    -1,    82,    60,    -1,    83,    60,    -1,    84,    60,
      -1,    85,    -1,    86,    60,    -1,    87,    60,    -1,    88,
      60,    -1,    62,    63,    -1,    62,    64,    -1,    62,    65,
      -1,    62,    66,    -1,    62,    67,    -1,    62,    68,    -1,
      62,    69,    -1,    62,    70,    -1,    62,    71,    -1,    62,
      72,    -1,    62,    78,    -1,    62,    79,    -1,    62,    81,
      -1,    29,    61,    -1,    30,    61,    -1,    42,    61,    -1,
      43,    61,    -1,    44,    61,    -1,    41,    61,    -1,    45,
      61,    -1,    46,    60,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,    85,    85,    86,    89,    90,    93,    93,    96,    97,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   119,   120,   123,   127,   128,   129,
     132,   133,   136,   137,   140,   141,   144,   145,   146,   147,
     148,   159,   160,   161,   162,   170,   171,   172,   173,   174,
     177,   183,   189,   191,   193,   195,   201,   208,   209,   216,
     223,   224,   225,   226,   227,   228,   229,   232,   233,   236,
     237,   240,   243,   244,   245,   248,   249,   252,   253,   256,
     257,   258,   261,   262,   265,   267,   271,   272,   275,   276,
     279,   280,   283,   289,   297,   298,   299,   300,   302,   303,
     304,   305,   306,   307,   308,   309,   310,   311,   312,   313,
     314,   316,   317,   318,   319,   320,   321,   322,   323,   324,
     325,   326,   327,   329,   336,   337,   338,   339,   340,   341,
     342,   344,   345,   346,   347,   348,   349,   350,   351,   352,
     353,   354,   355,   357,   359,   360,   361,   362,   363,   364,
     365,   367
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || YYTOKEN_TABLE
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "DEFAULTS", "POLL", "SKIP", "VIA", "AKA",
  "LOCALDOMAINS", "PROTOCOL", "AUTHENTICATE", "TIMEOUT", "KPOP", "SDPS",
  "ENVELOPE", "QVIRTUAL", "USERNAME", "PASSWORD", "FOLDER", "SMTPHOST",
  "FETCHDOMAINS", "MDA", "BSMTP", "LMTP", "SMTPADDRESS", "SMTPNAME",
  "SPAMRESPONSE", "PRECONNECT", "POSTCONNECT", "LIMIT", "WARNINGS",
  "INTERFACE", "MONITOR", "PLUGIN", "PLUGOUT", "IS", "HERE", "THERE", "TO",
  "MAP", "WILDCARD", "BATCHLIMIT", "FETCHLIMIT", "FETCHSIZELIMIT",
  "FASTUIDL", "EXPUNGE", "PROPERTIES", "SET", "LOGFILE", "DAEMON",
  "SYSLOG", "IDFILE", "PIDFILE", "INVISIBLE", "POSTMASTER", "BOUNCEMAIL",
  "SPAMBOUNCE", "SHOWDOTS", "PROTO", "AUTHTYPE", "STRING", "NUMBER", "NO",
  "KEEP", "FLUSH", "LIMITFLUSH", "FETCHALL", "REWRITE", "FORCECR",
  "STRIPCR", "PASS8BITS", "DROPSTATUS", "DROPDELIVERED", "DNS", "SERVICE",
  "PORT", "UIDL", "INTERVAL", "MIMEDECODE", "IDLE", "CHECKALIAS", "SSL",
  "SSLKEY", "SSLCERT", "SSLPROTO", "SSLCERTCK", "SSLCERTPATH",
  "SSLCOMMONNAME", "SSLFINGERPRINT", "PRINCIPAL", "ESMTPNAME",
  "ESMTPPASSWORD", "TRACEPOLLS", "$accept", "rcfile", "statement_list",
  "optmap", "statement", "define_server", "serverspecs", "alias_list",
  "domain_list", "serv_option", "userspecs", "explicits", "explicitdef",
  "userdef", "user0opts", "user1opts", "localnames", "mapping_list",
  "mapping", "folder_list", "smtp_list", "fetch_list", "num_list",
  "user_option", 0
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
     285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     295,   296,   297,   298,   299,   300,   301,   302,   303,   304,
     305,   306,   307,   308,   309,   310,   311,   312,   313,   314,
     315,   316,   317,   318,   319,   320,   321,   322,   323,   324,
     325,   326,   327,   328,   329,   330,   331,   332,   333,   334,
     335,   336,   337,   338,   339,   340,   341,   342,   343,   344,
     345,   346,   347
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    93,    94,    94,    95,    95,    96,    96,    97,    97,
      97,    97,    97,    97,    97,    97,    97,    97,    97,    97,
      97,    97,    97,    97,    97,    97,    97,    98,    98,    98,
      99,    99,   100,   100,   101,   101,   102,   102,   102,   102,
     102,   102,   102,   102,   102,   102,   102,   102,   102,   102,
     102,   102,   102,   102,   102,   102,   102,   102,   102,   102,
     102,   102,   102,   102,   102,   102,   102,   103,   103,   104,
     104,   105,   106,   106,   106,   107,   107,   108,   108,   109,
     109,   109,   110,   110,   111,   111,   112,   112,   113,   113,
     114,   114,   115,   115,   116,   116,   116,   116,   116,   116,
     116,   116,   116,   116,   116,   116,   116,   116,   116,   116,
     116,   116,   116,   116,   116,   116,   116,   116,   116,   116,
     116,   116,   116,   116,   116,   116,   116,   116,   116,   116,
     116,   116,   116,   116,   116,   116,   116,   116,   116,   116,
     116,   116,   116,   116,   116,   116,   116,   116,   116,   116,
     116,   116
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     0,     1,     1,     2,     1,     0,     4,     4,
       4,     4,     4,     2,     3,     2,     3,     4,     2,     3,
       2,     3,     2,     3,     2,     3,     4,     2,     2,     1,
       0,     2,     1,     2,     1,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     1,     2,     1,     2,     2,
       2,     2,     2,     2,     2,     3,     2,     2,     2,     2,
       2,     2,     1,     2,     2,     1,     2,     1,     1,     1,
       2,     2,     2,     3,     3,     0,     2,     1,     2,     1,
       1,     2,     1,     2,     1,     3,     1,     2,     1,     2,
       1,     2,     1,     2,     3,     2,     3,     2,     3,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     1,     2,
       2,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     2,     2,     2,     1,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       2,    29,     0,     0,     0,     0,     3,     4,    30,    27,
      28,     7,     7,     7,    18,     7,     7,    20,     7,    13,
      15,    22,     0,     1,     5,    24,     6,     0,     0,     0,
       0,     0,     0,    19,    21,    14,    16,    23,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   108,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   111,   112,   113,   114,   115,   116,
     117,   118,   119,   120,    62,     0,     0,    45,     0,   121,
     122,    47,   123,     0,     0,     0,   127,     0,     0,     0,
       0,     0,     0,    65,    31,    25,    68,    69,    75,    67,
      77,    17,     8,    11,     9,    10,    12,    37,    32,    36,
      34,    38,    40,    44,    39,    53,    54,    56,     0,    57,
      72,     0,    82,    99,    86,   100,    88,   101,    90,   102,
     106,   107,   103,   104,    92,   105,   109,   110,   144,   145,
      58,    59,    60,    61,    79,    84,    97,    80,    84,    95,
     149,   146,   147,   148,   150,   151,    64,   131,   132,   133,
     134,   135,   136,   137,   138,   139,   140,    63,    46,   141,
     142,    48,   143,    66,    49,    50,    51,    52,   124,   125,
     126,   128,   129,   130,    41,    42,    43,     0,    26,    70,
      71,     0,    78,    33,    35,    55,    74,     0,    73,    83,
      87,    89,    91,    93,    98,    96,    81,    94,    76,    85
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     5,     6,    27,     7,     8,    25,   119,   121,   104,
     105,   106,   107,   108,   200,   109,   156,   157,   132,   135,
     137,   139,   145,   110
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -124
static const yytype_int16 yypact[] =
{
      51,  -124,   -25,   -18,   235,    46,    51,  -124,  -124,  -124,
    -124,    10,    10,    10,  -124,    10,    10,  -124,    10,  -124,
    -124,  -124,    57,  -124,  -124,    -5,  -124,    39,    63,    28,
      68,    69,    73,  -124,  -124,  -124,  -124,  -124,    74,    87,
      89,    32,    93,    84,    55,    94,    95,    96,    97,    98,
     104,   105,   107,  -124,   108,   109,   110,   112,   113,   129,
     130,   114,   133,   138,   139,    -8,    48,   140,   141,   142,
     143,   144,   146,   116,  -124,  -124,  -124,  -124,  -124,  -124,
    -124,  -124,  -124,  -124,  -124,    65,   162,  -124,   163,  -124,
    -124,  -124,  -124,   147,   165,   166,  -124,   168,   169,   171,
     172,   179,   180,  -124,  -124,    86,   154,  -124,  -124,   192,
    -124,  -124,  -124,  -124,  -124,  -124,  -124,  -124,  -124,   181,
    -124,   182,  -124,  -124,  -124,  -124,  -124,  -124,   183,  -124,
      67,   -29,  -124,  -124,  -124,   184,  -124,   185,  -124,   186,
    -124,  -124,  -124,  -124,  -124,   187,  -124,  -124,  -124,  -124,
    -124,  -124,  -124,  -124,  -124,    85,   164,    71,   208,   213,
    -124,  -124,  -124,  -124,  -124,  -124,  -124,  -124,  -124,  -124,
    -124,  -124,  -124,  -124,  -124,  -124,  -124,  -124,  -124,  -124,
    -124,  -124,  -124,  -124,  -124,  -124,  -124,  -124,  -124,  -124,
    -124,  -124,  -124,  -124,  -124,  -124,  -124,    29,  -124,  -124,
     192,    72,  -124,  -124,  -124,  -124,  -124,   190,  -124,  -124,
    -124,  -124,  -124,  -124,  -124,  -124,  -124,  -124,  -124,  -124
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -124,  -124,  -124,    35,   126,  -124,  -124,  -124,  -124,    41,
    -124,  -124,   145,  -124,  -124,  -124,   199,   206,  -123,  -124,
    -124,  -124,  -124,  -109
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -85
static const yytype_int16 yytable[] =
{
     202,    38,    39,    40,    41,    42,    43,   208,   209,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,   158,   154,    66,   209,     9,    67,    68,    69,    70,
      71,    72,    10,   166,   122,   123,    23,    28,    29,    26,
      30,    31,   155,    32,     1,     2,     3,    73,    74,    75,
      76,    77,    78,    79,    80,    81,    82,    83,    84,    85,
      86,    87,    88,    89,    90,    91,    92,    93,    94,    95,
      96,    97,    98,    99,   100,   101,   102,   103,   154,   113,
     124,   218,    38,    39,    40,    41,    42,    43,     4,   111,
      44,    45,   177,   -84,   206,   178,   207,    33,   158,   181,
      34,   216,    35,    36,    37,   127,   128,    61,    62,    63,
      64,   183,   214,   112,   207,   184,   185,   -84,   114,   115,
     166,   158,    24,   116,   117,   167,   168,   169,   170,   171,
     172,   173,   174,   175,   176,   126,   198,   118,   197,   120,
     179,   180,   125,   182,   129,   130,   133,   134,   136,    84,
      85,    86,    87,    88,   138,   140,    91,   141,   142,   143,
      46,   144,   146,   147,   150,   100,   101,   102,   103,   167,
     168,   169,   170,   171,   172,   173,   174,   175,   176,   177,
     148,   149,   178,   151,   179,   180,   181,   182,   152,   153,
     215,   160,   161,   162,   163,   164,   165,   188,   183,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    56,    57,
      58,    59,    60,   186,   187,   189,   190,    65,   191,   192,
      66,   193,   194,    67,    68,    69,    70,    71,    72,   195,
     196,   203,   204,   205,   210,   211,   212,   207,   213,   217,
     219,   199,   131,     0,   201,    74,    75,    76,    77,    78,
      79,    80,    81,    82,    83,   159,     0,     0,     0,     0,
      89,    90,     0,    92,    93,    94,    95,    96,    97,    98,
      99,    11,     0,    12,    13,    14,    15,    16,    17,    18,
      19,    20,    21,     0,     0,     0,     0,    22
};

static const yytype_int16 yycheck[] =
{
     109,     6,     7,     8,     9,    10,    11,    36,   131,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    60,    40,    38,   157,    60,    41,    42,    43,    44,
      45,    46,    60,    14,    12,    13,     0,    12,    13,    39,
      15,    16,    60,    18,     3,     4,     5,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    81,    82,    83,    84,
      85,    86,    87,    88,    89,    90,    91,    92,    40,    61,
      58,   200,     6,     7,     8,     9,    10,    11,    47,    60,
      14,    15,    73,    36,    37,    76,    39,    50,    60,    80,
      53,    40,    55,    56,    57,    60,    61,    31,    32,    33,
      34,    92,    37,    60,    39,    60,    61,    60,    60,    60,
      14,    60,     6,    60,    60,    63,    64,    65,    66,    67,
      68,    69,    70,    71,    72,    61,   105,    60,    62,    60,
      78,    79,    59,    81,    60,    60,    60,    60,    60,    73,
      74,    75,    76,    77,    60,    60,    80,    60,    60,    60,
      16,    61,    60,    60,    60,    89,    90,    91,    92,    63,
      64,    65,    66,    67,    68,    69,    70,    71,    72,    73,
      61,    61,    76,    60,    78,    79,    80,    81,    60,    60,
      36,    61,    61,    61,    61,    61,    60,    60,    92,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    61,    61,    60,    60,    35,    60,    60,
      38,    60,    60,    41,    42,    43,    44,    45,    46,    60,
      60,    60,    60,    60,    60,    60,    60,    39,    61,    36,
      60,   106,    46,    -1,    62,    63,    64,    65,    66,    67,
      68,    69,    70,    71,    72,    66,    -1,    -1,    -1,    -1,
      78,    79,    -1,    81,    82,    83,    84,    85,    86,    87,
      88,    46,    -1,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    -1,    -1,    -1,    -1,    62
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,     3,     4,     5,    47,    94,    95,    97,    98,    60,
      60,    46,    48,    49,    50,    51,    52,    53,    54,    55,
      56,    57,    62,     0,    97,    99,    39,    96,    96,    96,
      96,    96,    96,    50,    53,    55,    56,    57,     6,     7,
       8,     9,    10,    11,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    38,    41,    42,    43,
      44,    45,    46,    62,    63,    64,    65,    66,    67,    68,
      69,    70,    71,    72,    73,    74,    75,    76,    77,    78,
      79,    80,    81,    82,    83,    84,    85,    86,    87,    88,
      89,    90,    91,    92,   102,   103,   104,   105,   106,   108,
     116,    60,    60,    61,    60,    60,    60,    60,    60,   100,
      60,   101,    12,    13,    58,    59,    61,    60,    61,    60,
      60,   110,   111,    60,    60,   112,    60,   113,    60,   114,
      60,    60,    60,    60,    61,   115,    60,    60,    61,    61,
      60,    60,    60,    60,    40,    60,   109,   110,    60,   109,
      61,    61,    61,    61,    61,    60,    14,    63,    64,    65,
      66,    67,    68,    69,    70,    71,    72,    73,    76,    78,
      79,    80,    81,    92,    60,    61,    61,    61,    60,    60,
      60,    60,    60,    60,    60,    60,    60,    62,   102,   105,
     107,    62,   116,    60,    60,    60,    37,    39,    36,   111,
      60,    60,    60,    61,    37,    36,    40,    36,   116,    60
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
   Once GCC version 2 has supplanted version 1, this can go.  */

#define YYFAIL		goto yyerrlab

#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)					\
do								\
  if (yychar == YYEMPTY && yylen == 1)				\
    {								\
      yychar = (Token);						\
      yylval = (Value);						\
      yytoken = YYTRANSLATE (yychar);				\
      YYPOPSTACK (1);						\
      goto yybackup;						\
    }								\
  else								\
    {								\
      yyerror (YY_("syntax error: cannot back up")); \
      YYERROR;							\
    }								\
while (YYID (0))


#define YYTERROR	1
#define YYERRCODE	256


/* YYLLOC_DEFAULT -- Set CURRENT to span from RHS[1] to RHS[N].
   If N is 0, then set CURRENT to the empty location which ends
   the previous symbol: RHS[0] (always defined).  */

#define YYRHSLOC(Rhs, K) ((Rhs)[K])
#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)				\
    do									\
      if (YYID (N))                                                    \
	{								\
	  (Current).first_line   = YYRHSLOC (Rhs, 1).first_line;	\
	  (Current).first_column = YYRHSLOC (Rhs, 1).first_column;	\
	  (Current).last_line    = YYRHSLOC (Rhs, N).last_line;		\
	  (Current).last_column  = YYRHSLOC (Rhs, N).last_column;	\
	}								\
      else								\
	{								\
	  (Current).first_line   = (Current).last_line   =		\
	    YYRHSLOC (Rhs, 0).last_line;				\
	  (Current).first_column = (Current).last_column =		\
	    YYRHSLOC (Rhs, 0).last_column;				\
	}								\
    while (YYID (0))
#endif


/* YY_LOCATION_PRINT -- Print the location on the stream.
   This macro was not mandated originally: define only if we know
   we won't break user code: when these are the locations we know.  */

#ifndef YY_LOCATION_PRINT
# if YYLTYPE_IS_TRIVIAL
#  define YY_LOCATION_PRINT(File, Loc)			\
     fprintf (File, "%d.%d-%d.%d",			\
	      (Loc).first_line, (Loc).first_column,	\
	      (Loc).last_line,  (Loc).last_column)
# else
#  define YY_LOCATION_PRINT(File, Loc) ((void) 0)
# endif
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
  if (!yyvaluep)
    return;
# ifdef YYPRINT
  if (yytype < YYNTOKENS)
    YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# else
  YYUSE (yyoutput);
# endif
  switch (yytype)
    {
      default:
	break;
    }
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
yy_stack_print (yytype_int16 *bottom, yytype_int16 *top)
#else
static void
yy_stack_print (bottom, top)
    yytype_int16 *bottom;
    yytype_int16 *top;
#endif
{
  YYFPRINTF (stderr, "Stack now");
  for (; bottom <= top; ++bottom)
    YYFPRINTF (stderr, " %d", *bottom);
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
      fprintf (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr, yyrhs[yyprhs[yyrule] + yyi],
		       &(yyvsp[(yyi + 1) - (yynrhs)])
		       		       );
      fprintf (stderr, "\n");
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

/* Copy into YYRESULT an error message about the unexpected token
   YYCHAR while in state YYSTATE.  Return the number of bytes copied,
   including the terminating null byte.  If YYRESULT is null, do not
   copy anything; just return the number of bytes that would be
   copied.  As a special case, return 0 if an ordinary "syntax error"
   message will do.  Return YYSIZE_MAXIMUM if overflow occurs during
   size calculation.  */
static YYSIZE_T
yysyntax_error (char *yyresult, int yystate, int yychar)
{
  int yyn = yypact[yystate];

  if (! (YYPACT_NINF < yyn && yyn <= YYLAST))
    return 0;
  else
    {
      int yytype = YYTRANSLATE (yychar);
      YYSIZE_T yysize0 = yytnamerr (0, yytname[yytype]);
      YYSIZE_T yysize = yysize0;
      YYSIZE_T yysize1;
      int yysize_overflow = 0;
      enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
      char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
      int yyx;

# if 0
      /* This is so xgettext sees the translatable formats that are
	 constructed on the fly.  */
      YY_("syntax error, unexpected %s");
      YY_("syntax error, unexpected %s, expecting %s");
      YY_("syntax error, unexpected %s, expecting %s or %s");
      YY_("syntax error, unexpected %s, expecting %s or %s or %s");
      YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s");
# endif
      char *yyfmt;
      char const *yyf;
      static char const yyunexpected[] = "syntax error, unexpected %s";
      static char const yyexpecting[] = ", expecting %s";
      static char const yyor[] = " or %s";
      char yyformat[sizeof yyunexpected
		    + sizeof yyexpecting - 1
		    + ((YYERROR_VERBOSE_ARGS_MAXIMUM - 2)
		       * (sizeof yyor - 1))];
      char const *yyprefix = yyexpecting;

      /* Start YYX at -YYN if negative to avoid negative indexes in
	 YYCHECK.  */
      int yyxbegin = yyn < 0 ? -yyn : 0;

      /* Stay within bounds of both yycheck and yytname.  */
      int yychecklim = YYLAST - yyn + 1;
      int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
      int yycount = 1;

      yyarg[0] = yytname[yytype];
      yyfmt = yystpcpy (yyformat, yyunexpected);

      for (yyx = yyxbegin; yyx < yyxend; ++yyx)
	if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR)
	  {
	    if (yycount == YYERROR_VERBOSE_ARGS_MAXIMUM)
	      {
		yycount = 1;
		yysize = yysize0;
		yyformat[sizeof yyunexpected - 1] = '\0';
		break;
	      }
	    yyarg[yycount++] = yytname[yyx];
	    yysize1 = yysize + yytnamerr (0, yytname[yyx]);
	    yysize_overflow |= (yysize1 < yysize);
	    yysize = yysize1;
	    yyfmt = yystpcpy (yyfmt, yyprefix);
	    yyprefix = yyor;
	  }

      yyf = YY_(yyformat);
      yysize1 = yysize + yystrlen (yyf);
      yysize_overflow |= (yysize1 < yysize);
      yysize = yysize1;

      if (yysize_overflow)
	return YYSIZE_MAXIMUM;

      if (yyresult)
	{
	  /* Avoid sprintf, as that infringes on the user's name space.
	     Don't have undefined behavior even if the translation
	     produced a string with the wrong number of "%s"s.  */
	  char *yyp = yyresult;
	  int yyi = 0;
	  while ((*yyp = *yyf) != '\0')
	    {
	      if (*yyp == '%' && yyf[1] == 's' && yyi < yycount)
		{
		  yyp += yytnamerr (yyp, yyarg[yyi++]);
		  yyf += 2;
		}
	      else
		{
		  yyp++;
		  yyf++;
		}
	    }
	}
      return yysize;
    }
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

  switch (yytype)
    {

      default:
	break;
    }
}


/* Prevent warnings from -Wmissing-prototypes.  */

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



/* The look-ahead symbol.  */
int yychar;

/* The semantic value of the look-ahead symbol.  */
YYSTYPE yylval;

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
  int yyn;
  int yyresult;
  /* Number of tokens to shift before error messages enabled.  */
  int yyerrstatus;
  /* Look-ahead token as an internal (translated) token number.  */
  int yytoken = 0;
#if YYERROR_VERBOSE
  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYSIZE_T yymsg_alloc = sizeof yymsgbuf;
#endif

  /* Three stacks and their tools:
     `yyss': related to states,
     `yyvs': related to semantic values,
     `yyls': related to locations.

     Refer to the stacks thru separate pointers, to allow yyoverflow
     to reallocate them elsewhere.  */

  /* The state stack.  */
  yytype_int16 yyssa[YYINITDEPTH];
  yytype_int16 *yyss = yyssa;
  yytype_int16 *yyssp;

  /* The semantic value stack.  */
  YYSTYPE yyvsa[YYINITDEPTH];
  YYSTYPE *yyvs = yyvsa;
  YYSTYPE *yyvsp;



#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  YYSIZE_T yystacksize = YYINITDEPTH;

  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;


  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY;		/* Cause a token to be read.  */

  /* Initialize stack pointers.
     Waste one element of value and location stack
     so that they stay on the same level as the state stack.
     The wasted elements are never initialized.  */

  yyssp = yyss;
  yyvsp = yyvs;

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
	YYSTACK_RELOCATE (yyss);
	YYSTACK_RELOCATE (yyvs);

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

  goto yybackup;

/*-----------.
| yybackup.  |
`-----------*/
yybackup:

  /* Do appropriate processing given the current state.  Read a
     look-ahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to look-ahead token.  */
  yyn = yypact[yystate];
  if (yyn == YYPACT_NINF)
    goto yydefault;

  /* Not known => get a look-ahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid look-ahead symbol.  */
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
      if (yyn == 0 || yyn == YYTABLE_NINF)
	goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  if (yyn == YYFINAL)
    YYACCEPT;

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the look-ahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);

  /* Discard the shifted token unless it is eof.  */
  if (yychar != YYEOF)
    yychar = YYEMPTY;

  yystate = yyn;
  *++yyvsp = yylval;

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
        case 8:
#line 96 "../rcfile_y.y"
    {run.logfile = prependdir ((yyvsp[(4) - (4)].sval), rcfiledir);}
    break;

  case 9:
#line 97 "../rcfile_y.y"
    {run.idfile = prependdir ((yyvsp[(4) - (4)].sval), rcfiledir);}
    break;

  case 10:
#line 98 "../rcfile_y.y"
    {run.pidfile = prependdir ((yyvsp[(4) - (4)].sval), rcfiledir);}
    break;

  case 11:
#line 99 "../rcfile_y.y"
    {run.poll_interval = (yyvsp[(4) - (4)].number);}
    break;

  case 12:
#line 100 "../rcfile_y.y"
    {run.postmaster = xstrdup((yyvsp[(4) - (4)].sval));}
    break;

  case 13:
#line 101 "../rcfile_y.y"
    {run.bouncemail = TRUE;}
    break;

  case 14:
#line 102 "../rcfile_y.y"
    {run.bouncemail = FALSE;}
    break;

  case 15:
#line 103 "../rcfile_y.y"
    {run.spambounce = TRUE;}
    break;

  case 16:
#line 104 "../rcfile_y.y"
    {run.spambounce = FALSE;}
    break;

  case 17:
#line 105 "../rcfile_y.y"
    {run.properties =xstrdup((yyvsp[(4) - (4)].sval));}
    break;

  case 18:
#line 106 "../rcfile_y.y"
    {run.use_syslog = TRUE;}
    break;

  case 19:
#line 107 "../rcfile_y.y"
    {run.use_syslog = FALSE;}
    break;

  case 20:
#line 108 "../rcfile_y.y"
    {run.invisible = TRUE;}
    break;

  case 21:
#line 109 "../rcfile_y.y"
    {run.invisible = FALSE;}
    break;

  case 22:
#line 110 "../rcfile_y.y"
    {run.showdots = FLAG_TRUE;}
    break;

  case 23:
#line 111 "../rcfile_y.y"
    {run.showdots = FLAG_FALSE;}
    break;

  case 24:
#line 119 "../rcfile_y.y"
    {record_current();}
    break;

  case 26:
#line 124 "../rcfile_y.y"
    {yyerror(GT_("server option after user options"));}
    break;

  case 27:
#line 127 "../rcfile_y.y"
    {reset_server((yyvsp[(2) - (2)].sval), FALSE);}
    break;

  case 28:
#line 128 "../rcfile_y.y"
    {reset_server((yyvsp[(2) - (2)].sval), TRUE);}
    break;

  case 29:
#line 129 "../rcfile_y.y"
    {reset_server("defaults", FALSE);}
    break;

  case 32:
#line 136 "../rcfile_y.y"
    {save_str(&current.server.akalist,(yyvsp[(1) - (1)].sval),0);}
    break;

  case 33:
#line 137 "../rcfile_y.y"
    {save_str(&current.server.akalist,(yyvsp[(2) - (2)].sval),0);}
    break;

  case 34:
#line 140 "../rcfile_y.y"
    {save_str(&current.server.localdomains,(yyvsp[(1) - (1)].sval),0);}
    break;

  case 35:
#line 141 "../rcfile_y.y"
    {save_str(&current.server.localdomains,(yyvsp[(2) - (2)].sval),0);}
    break;

  case 37:
#line 145 "../rcfile_y.y"
    {current.server.via = xstrdup((yyvsp[(2) - (2)].sval));}
    break;

  case 39:
#line 147 "../rcfile_y.y"
    {current.server.protocol = (yyvsp[(2) - (2)].proto);}
    break;

  case 40:
#line 148 "../rcfile_y.y"
    {
					    current.server.protocol = P_POP3;

					    if (current.server.authenticate == A_PASSWORD)
#ifdef KERBEROS_V5
						current.server.authenticate = A_KERBEROS_V5;
#else
						current.server.authenticate = A_KERBEROS_V4;
#endif /* KERBEROS_V5 */
					    current.server.service = KPOP_PORT;
					}
    break;

  case 41:
#line 159 "../rcfile_y.y"
    {current.server.principal = xstrdup((yyvsp[(2) - (2)].sval));}
    break;

  case 42:
#line 160 "../rcfile_y.y"
    {current.server.esmtp_name = xstrdup((yyvsp[(2) - (2)].sval));}
    break;

  case 43:
#line 161 "../rcfile_y.y"
    {current.server.esmtp_password = xstrdup((yyvsp[(2) - (2)].sval));}
    break;

  case 44:
#line 162 "../rcfile_y.y"
    {
#ifdef SDPS_ENABLE
					    current.server.protocol = P_POP3;
					    current.server.sdps = TRUE;
#else
					    yyerror(GT_("SDPS not enabled."));
#endif /* SDPS_ENABLE */
					}
    break;

  case 45:
#line 170 "../rcfile_y.y"
    {current.server.uidl = FLAG_TRUE;}
    break;

  case 46:
#line 171 "../rcfile_y.y"
    {current.server.uidl  = FLAG_FALSE;}
    break;

  case 47:
#line 172 "../rcfile_y.y"
    {current.server.checkalias = FLAG_TRUE;}
    break;

  case 48:
#line 173 "../rcfile_y.y"
    {current.server.checkalias  = FLAG_FALSE;}
    break;

  case 49:
#line 174 "../rcfile_y.y"
    {
					current.server.service = (yyvsp[(2) - (2)].sval);
					}
    break;

  case 50:
#line 177 "../rcfile_y.y"
    {
					int port = (yyvsp[(2) - (2)].number);
					char buf[10];
					snprintf(buf, sizeof buf, "%d", port);
					current.server.service = xstrdup(buf);
		}
    break;

  case 51:
#line 183 "../rcfile_y.y"
    {
					int port = (yyvsp[(2) - (2)].number);
					char buf[10];
					snprintf(buf, sizeof buf, "%d", port);
					current.server.service = xstrdup(buf);
		}
    break;

  case 52:
#line 190 "../rcfile_y.y"
    {current.server.interval = (yyvsp[(2) - (2)].number);}
    break;

  case 53:
#line 192 "../rcfile_y.y"
    {current.server.authenticate = (yyvsp[(2) - (2)].proto);}
    break;

  case 54:
#line 194 "../rcfile_y.y"
    {current.server.timeout = (yyvsp[(2) - (2)].number);}
    break;

  case 55:
#line 196 "../rcfile_y.y"
    {
					    current.server.envelope = 
						xstrdup((yyvsp[(3) - (3)].sval));
					    current.server.envskip = (yyvsp[(2) - (3)].number);
					}
    break;

  case 56:
#line 202 "../rcfile_y.y"
    {
					    current.server.envelope = 
						xstrdup((yyvsp[(2) - (2)].sval));
					    current.server.envskip = 0;
					}
    break;

  case 57:
#line 208 "../rcfile_y.y"
    {current.server.qvirtual=xstrdup((yyvsp[(2) - (2)].sval));}
    break;

  case 58:
#line 209 "../rcfile_y.y"
    {
#ifdef CAN_MONITOR
					interface_parse((yyvsp[(2) - (2)].sval), &current.server);
#else
					fprintf(stderr, GT_("fetchmail: interface option is only supported under Linux (without IPv6) and FreeBSD\n"));
#endif
					}
    break;

  case 59:
#line 216 "../rcfile_y.y"
    {
#ifdef CAN_MONITOR
					current.server.monitor = xstrdup((yyvsp[(2) - (2)].sval));
#else
					fprintf(stderr, GT_("fetchmail: monitor option is only supported under Linux (without IPv6) and FreeBSD\n"));
#endif
					}
    break;

  case 60:
#line 223 "../rcfile_y.y"
    { current.server.plugin = xstrdup((yyvsp[(2) - (2)].sval)); }
    break;

  case 61:
#line 224 "../rcfile_y.y"
    { current.server.plugout = xstrdup((yyvsp[(2) - (2)].sval)); }
    break;

  case 62:
#line 225 "../rcfile_y.y"
    {current.server.dns = FLAG_TRUE;}
    break;

  case 63:
#line 226 "../rcfile_y.y"
    {current.server.dns = FLAG_FALSE;}
    break;

  case 64:
#line 227 "../rcfile_y.y"
    {current.server.envelope = STRING_DISABLED;}
    break;

  case 65:
#line 228 "../rcfile_y.y"
    {current.server.tracepolls = FLAG_TRUE;}
    break;

  case 66:
#line 229 "../rcfile_y.y"
    {current.server.tracepolls = FLAG_FALSE;}
    break;

  case 67:
#line 232 "../rcfile_y.y"
    {record_current(); user_reset();}
    break;

  case 69:
#line 236 "../rcfile_y.y"
    {record_current(); user_reset();}
    break;

  case 70:
#line 237 "../rcfile_y.y"
    {record_current(); user_reset();}
    break;

  case 72:
#line 243 "../rcfile_y.y"
    {current.remotename = xstrdup((yyvsp[(2) - (2)].sval));}
    break;

  case 74:
#line 245 "../rcfile_y.y"
    {current.remotename = xstrdup((yyvsp[(2) - (3)].sval));}
    break;

  case 79:
#line 256 "../rcfile_y.y"
    {current.wildcard =  TRUE;}
    break;

  case 80:
#line 257 "../rcfile_y.y"
    {current.wildcard =  FALSE;}
    break;

  case 81:
#line 258 "../rcfile_y.y"
    {current.wildcard =  TRUE;}
    break;

  case 84:
#line 266 "../rcfile_y.y"
    {save_str_pair(&current.localnames, (yyvsp[(1) - (1)].sval), NULL);}
    break;

  case 85:
#line 268 "../rcfile_y.y"
    {save_str_pair(&current.localnames, (yyvsp[(1) - (3)].sval), (yyvsp[(3) - (3)].sval));}
    break;

  case 86:
#line 271 "../rcfile_y.y"
    {save_str(&current.mailboxes,(yyvsp[(1) - (1)].sval),0);}
    break;

  case 87:
#line 272 "../rcfile_y.y"
    {save_str(&current.mailboxes,(yyvsp[(2) - (2)].sval),0);}
    break;

  case 88:
#line 275 "../rcfile_y.y"
    {save_str(&current.smtphunt, (yyvsp[(1) - (1)].sval),TRUE);}
    break;

  case 89:
#line 276 "../rcfile_y.y"
    {save_str(&current.smtphunt, (yyvsp[(2) - (2)].sval),TRUE);}
    break;

  case 90:
#line 279 "../rcfile_y.y"
    {save_str(&current.domainlist, (yyvsp[(1) - (1)].sval),TRUE);}
    break;

  case 91:
#line 280 "../rcfile_y.y"
    {save_str(&current.domainlist, (yyvsp[(2) - (2)].sval),TRUE);}
    break;

  case 92:
#line 284 "../rcfile_y.y"
    {
			    struct idlist *id;
			    id=save_str(&current.antispam,STRING_DUMMY,0);
			    id->val.status.num = (yyvsp[(1) - (1)].number);
			}
    break;

  case 93:
#line 290 "../rcfile_y.y"
    {
			    struct idlist *id;
			    id=save_str(&current.antispam,STRING_DUMMY,0);
			    id->val.status.num = (yyvsp[(2) - (2)].number);
			}
    break;

  case 98:
#line 302 "../rcfile_y.y"
    {current.remotename  = xstrdup((yyvsp[(2) - (3)].sval));}
    break;

  case 99:
#line 303 "../rcfile_y.y"
    {current.password    = xstrdup((yyvsp[(2) - (2)].sval));}
    break;

  case 103:
#line 307 "../rcfile_y.y"
    {current.smtpaddress = xstrdup((yyvsp[(2) - (2)].sval));}
    break;

  case 104:
#line 308 "../rcfile_y.y"
    {current.smtpname = xstrdup((yyvsp[(2) - (2)].sval));}
    break;

  case 106:
#line 310 "../rcfile_y.y"
    {current.mda         = xstrdup((yyvsp[(2) - (2)].sval));}
    break;

  case 107:
#line 311 "../rcfile_y.y"
    {current.bsmtp       = prependdir ((yyvsp[(2) - (2)].sval), rcfiledir);}
    break;

  case 108:
#line 312 "../rcfile_y.y"
    {current.listener    = LMTP_MODE;}
    break;

  case 109:
#line 313 "../rcfile_y.y"
    {current.preconnect  = xstrdup((yyvsp[(2) - (2)].sval));}
    break;

  case 110:
#line 314 "../rcfile_y.y"
    {current.postconnect = xstrdup((yyvsp[(2) - (2)].sval));}
    break;

  case 111:
#line 316 "../rcfile_y.y"
    {current.keep        = FLAG_TRUE;}
    break;

  case 112:
#line 317 "../rcfile_y.y"
    {current.flush       = FLAG_TRUE;}
    break;

  case 113:
#line 318 "../rcfile_y.y"
    {current.limitflush  = FLAG_TRUE;}
    break;

  case 114:
#line 319 "../rcfile_y.y"
    {current.fetchall    = FLAG_TRUE;}
    break;

  case 115:
#line 320 "../rcfile_y.y"
    {current.rewrite     = FLAG_TRUE;}
    break;

  case 116:
#line 321 "../rcfile_y.y"
    {current.forcecr     = FLAG_TRUE;}
    break;

  case 117:
#line 322 "../rcfile_y.y"
    {current.stripcr     = FLAG_TRUE;}
    break;

  case 118:
#line 323 "../rcfile_y.y"
    {current.pass8bits   = FLAG_TRUE;}
    break;

  case 119:
#line 324 "../rcfile_y.y"
    {current.dropstatus  = FLAG_TRUE;}
    break;

  case 120:
#line 325 "../rcfile_y.y"
    {current.dropdelivered = FLAG_TRUE;}
    break;

  case 121:
#line 326 "../rcfile_y.y"
    {current.mimedecode  = FLAG_TRUE;}
    break;

  case 122:
#line 327 "../rcfile_y.y"
    {current.idle        = FLAG_TRUE;}
    break;

  case 123:
#line 329 "../rcfile_y.y"
    {
#ifdef SSL_ENABLE
		    current.use_ssl = FLAG_TRUE;
#else
		    yyerror(GT_("SSL is not enabled"));
#endif 
		}
    break;

  case 124:
#line 336 "../rcfile_y.y"
    {current.sslkey = prependdir ((yyvsp[(2) - (2)].sval), rcfiledir);}
    break;

  case 125:
#line 337 "../rcfile_y.y"
    {current.sslcert = prependdir ((yyvsp[(2) - (2)].sval), rcfiledir);}
    break;

  case 126:
#line 338 "../rcfile_y.y"
    {current.sslproto = xstrdup((yyvsp[(2) - (2)].sval));}
    break;

  case 127:
#line 339 "../rcfile_y.y"
    {current.sslcertck = FLAG_TRUE;}
    break;

  case 128:
#line 340 "../rcfile_y.y"
    {current.sslcertpath = prependdir((yyvsp[(2) - (2)].sval), rcfiledir);}
    break;

  case 129:
#line 341 "../rcfile_y.y"
    {current.sslcommonname = xstrdup((yyvsp[(2) - (2)].sval));}
    break;

  case 130:
#line 342 "../rcfile_y.y"
    {current.sslfingerprint = xstrdup((yyvsp[(2) - (2)].sval));}
    break;

  case 131:
#line 344 "../rcfile_y.y"
    {current.keep        = FLAG_FALSE;}
    break;

  case 132:
#line 345 "../rcfile_y.y"
    {current.flush       = FLAG_FALSE;}
    break;

  case 133:
#line 346 "../rcfile_y.y"
    {current.limitflush  = FLAG_FALSE;}
    break;

  case 134:
#line 347 "../rcfile_y.y"
    {current.fetchall    = FLAG_FALSE;}
    break;

  case 135:
#line 348 "../rcfile_y.y"
    {current.rewrite     = FLAG_FALSE;}
    break;

  case 136:
#line 349 "../rcfile_y.y"
    {current.forcecr     = FLAG_FALSE;}
    break;

  case 137:
#line 350 "../rcfile_y.y"
    {current.stripcr     = FLAG_FALSE;}
    break;

  case 138:
#line 351 "../rcfile_y.y"
    {current.pass8bits   = FLAG_FALSE;}
    break;

  case 139:
#line 352 "../rcfile_y.y"
    {current.dropstatus  = FLAG_FALSE;}
    break;

  case 140:
#line 353 "../rcfile_y.y"
    {current.dropdelivered = FLAG_FALSE;}
    break;

  case 141:
#line 354 "../rcfile_y.y"
    {current.mimedecode  = FLAG_FALSE;}
    break;

  case 142:
#line 355 "../rcfile_y.y"
    {current.idle        = FLAG_FALSE;}
    break;

  case 143:
#line 357 "../rcfile_y.y"
    {current.use_ssl     = FLAG_FALSE;}
    break;

  case 144:
#line 359 "../rcfile_y.y"
    {current.limit       = NUM_VALUE_IN((yyvsp[(2) - (2)].number));}
    break;

  case 145:
#line 360 "../rcfile_y.y"
    {current.warnings    = NUM_VALUE_IN((yyvsp[(2) - (2)].number));}
    break;

  case 146:
#line 361 "../rcfile_y.y"
    {current.fetchlimit  = NUM_VALUE_IN((yyvsp[(2) - (2)].number));}
    break;

  case 147:
#line 362 "../rcfile_y.y"
    {current.fetchsizelimit = NUM_VALUE_IN((yyvsp[(2) - (2)].number));}
    break;

  case 148:
#line 363 "../rcfile_y.y"
    {current.fastuidl    = NUM_VALUE_IN((yyvsp[(2) - (2)].number));}
    break;

  case 149:
#line 364 "../rcfile_y.y"
    {current.batchlimit  = NUM_VALUE_IN((yyvsp[(2) - (2)].number));}
    break;

  case 150:
#line 365 "../rcfile_y.y"
    {current.expunge     = NUM_VALUE_IN((yyvsp[(2) - (2)].number));}
    break;

  case 151:
#line 367 "../rcfile_y.y"
    {current.properties  = xstrdup((yyvsp[(2) - (2)].sval));}
    break;


/* Line 1267 of yacc.c.  */
#line 2473 "rcfile_y.c"
      default: break;
    }
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
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if ! YYERROR_VERBOSE
      yyerror (YY_("syntax error"));
#else
      {
	YYSIZE_T yysize = yysyntax_error (0, yystate, yychar);
	if (yymsg_alloc < yysize && yymsg_alloc < YYSTACK_ALLOC_MAXIMUM)
	  {
	    YYSIZE_T yyalloc = 2 * yysize;
	    if (! (yysize <= yyalloc && yyalloc <= YYSTACK_ALLOC_MAXIMUM))
	      yyalloc = YYSTACK_ALLOC_MAXIMUM;
	    if (yymsg != yymsgbuf)
	      YYSTACK_FREE (yymsg);
	    yymsg = (char *) YYSTACK_ALLOC (yyalloc);
	    if (yymsg)
	      yymsg_alloc = yyalloc;
	    else
	      {
		yymsg = yymsgbuf;
		yymsg_alloc = sizeof yymsgbuf;
	      }
	  }

	if (0 < yysize && yysize <= yymsg_alloc)
	  {
	    (void) yysyntax_error (yymsg, yystate, yychar);
	    yyerror (yymsg);
	  }
	else
	  {
	    yyerror (YY_("syntax error"));
	    if (yysize != 0)
	      goto yyexhaustedlab;
	  }
      }
#endif
    }



  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse look-ahead token after an
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

  /* Else will try to reuse look-ahead token after shifting the error
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
      if (yyn != YYPACT_NINF)
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

  if (yyn == YYFINAL)
    YYACCEPT;

  *++yyvsp = yylval;


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

#ifndef yyoverflow
/*-------------------------------------------------.
| yyexhaustedlab -- memory exhaustion comes here.  |
`-------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
  if (yychar != YYEOF && yychar != YYEMPTY)
     yydestruct ("Cleanup: discarding lookahead",
		 yytoken, &yylval);
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


#line 369 "../rcfile_y.y"


/* lexer interface */
extern char *rcfile;
extern int prc_lineno;
extern char *yytext;
extern FILE *yyin;

static struct query *hosttail;	/* where to add new elements */

void yyerror (const char *s)
/* report a syntax error */
{
    report_at_line(stderr, 0, rcfile, prc_lineno, GT_("%s at %s"), s, 
		   (yytext && yytext[0]) ? yytext : GT_("end of input"));
    prc_errflag++;
}

/** check that a configuration file is secure, returns PS_* status codes */
int prc_filecheck(const char *pathname,
		  const flag securecheck /** shortcuts permission, filetype and uid tests if false */)
{
#ifndef __EMX__
    struct stat statbuf;

    errno = 0;

    /* special case useful for debugging purposes */
    if (strcmp("/dev/null", pathname) == 0)
	return(PS_SUCCESS);

    /* pass through the special name for stdin */
    if (strcmp("-", pathname) == 0)
	return(PS_SUCCESS);

    /* the run control file must have the same uid as the REAL uid of this 
       process, it must have permissions no greater than 600, and it must not 
       be a symbolic link.  We check these conditions here. */

    if (stat(pathname, &statbuf) < 0) {
	if (errno == ENOENT) 
	    return(PS_SUCCESS);
	else {
	    report(stderr, "lstat: %s: %s\n", pathname, strerror(errno));
	    return(PS_IOERR);
	}
    }

    if (!securecheck)	return PS_SUCCESS;

    if (!S_ISREG(statbuf.st_mode))
    {
	fprintf(stderr, GT_("File %s must be a regular file.\n"), pathname);
	return(PS_IOERR);
    }

#ifndef __BEOS__
#ifdef __CYGWIN__
    if (cygwin_internal(CW_CHECK_NTSEC, pathname))
#endif /* __CYGWIN__ */
    if (statbuf.st_mode & (S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH | S_IXOTH))
    {
	fprintf(stderr, GT_("File %s must have no more than -rwx------ (0700) permissions.\n"), 
		pathname);
	return(PS_IOERR);
    }
#endif /* __BEOS__ */

#ifdef HAVE_GETEUID
    if (statbuf.st_uid != geteuid())
#else
    if (statbuf.st_uid != getuid())
#endif /* HAVE_GETEUID */
    {
	fprintf(stderr, GT_("File %s must be owned by you.\n"), pathname);
	return(PS_IOERR);
    }
#endif
    return(PS_SUCCESS);
}

int prc_parse_file (const char *pathname, const flag securecheck)
/* digest the configuration into a linked list of host records */
{
    prc_errflag = 0;
    querylist = hosttail = (struct query *)NULL;

    errno = 0;

    /* Check that the file is secure */
    if ( (prc_errflag = prc_filecheck(pathname, securecheck)) != 0 )
	return(prc_errflag);

    /*
     * Croak if the configuration directory does not exist.
     * This probably means an NFS mount failed and we can't
     * see a configuration file that ought to be there.
     * Question: is this a portable check? It's not clear
     * that all implementations of lstat() will return ENOTDIR
     * rather than plain ENOENT in this case...
     */
    if (errno == ENOTDIR)
	return(PS_IOERR);
    else if (errno == ENOENT)
	return(PS_SUCCESS);

    /* Open the configuration file and feed it to the lexer. */
    if (strcmp(pathname, "-") == 0)
	yyin = stdin;
    else if ((yyin = fopen(pathname,"r")) == (FILE *)NULL) {
	report(stderr, "open: %s: %s\n", pathname, strerror(errno));
	return(PS_IOERR);
    }

    yyparse();		/* parse entire file */

    fclose(yyin);	/* not checking this should be safe, file mode was r */

    if (prc_errflag) 
	return(PS_SYNTAX);
    else
	return(PS_SUCCESS);
}

static void reset_server(const char *name, int skip)
/* clear the entire global record and initialize it with a new name */
{
    trailer = FALSE;
    memset(&current,'\0',sizeof(current));
    current.smtp_socket = -1;
    current.server.pollname = xstrdup(name);
    current.server.skip = skip;
    current.server.principal = (char *)NULL;
}


static void user_reset(void)
/* clear the global current record (user parameters) used by the parser */
{
    struct hostdata save;

    /*
     * Purpose of this code is to initialize the new server block, but
     * preserve whatever server name was previously set.  Also
     * preserve server options unless the command-line explicitly
     * overrides them.
     */
    save = current.server;

    memset(&current, '\0', sizeof(current));
    current.smtp_socket = -1;

    current.server = save;
}

/** append a host record to the host list */
struct query *hostalloc(struct query *init /** pointer to block containing
					       initial values */)
{
    struct query *node;

    /* allocate new node */
    node = (struct query *) xmalloc(sizeof(struct query));

    /* initialize it */
    if (init)
	memcpy(node, init, sizeof(struct query));
    else
    {
	memset(node, '\0', sizeof(struct query));
	node->smtp_socket = -1;
    }

    /* append to end of list */
    if (hosttail != (struct query *) 0)
	hosttail->next = node;	/* list contains at least one element */
    else
	querylist = node;	/* list is empty */
    hosttail = node;

    if (trailer)
	node->server.lead_server = leadentry;
    else
    {
	node->server.lead_server = NULL;
	leadentry = &node->server;
    }

    return(node);
}

static void record_current(void)
/* register current parameters and append to the host list */
{
    (void) hostalloc(&current);
    trailer = TRUE;
}

char *prependdir (const char *file, const char *dir)
/* if a filename is relative to dir, convert it to an absolute path */
{
    char *newfile;
    if (!file[0] ||			/* null path */
	file[0] == '/' ||		/* absolute path */
	strcmp(file, "-") == 0 ||	/* stdin/stdout */
	!dir[0])			/* we don't HAVE_GETCWD */
	return xstrdup (file);
    newfile = xmalloc (strlen (dir) + 1 + strlen (file) + 1);
    if (dir[strlen(dir) - 1] != '/')
	sprintf (newfile, "%s/%s", dir, file);
    else
	sprintf (newfile, "%s%s", dir, file);
    return newfile;
}

/* rcfile_y.y ends here */

