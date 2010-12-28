/* A Bison parser, made by GNU Bison 2.4.3.  */

/* Skeleton implementation for Bison's Yacc-like parsers in C
   
      Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003, 2004, 2005, 2006,
   2009, 2010 Free Software Foundation, Inc.
   
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
#define YYBISON_VERSION "2.4.3"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1

/* Using locations.  */
#define YYLSP_NEEDED 0



/* Copy the first part of user declarations.  */

/* Line 189 of yacc.c  */
#line 1 "rcfile_y.y"

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


/* Line 189 of yacc.c  */
#line 130 "rcfile_y.c"

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
     BATCHLIMIT = 295,
     FETCHLIMIT = 296,
     FETCHSIZELIMIT = 297,
     FASTUIDL = 298,
     EXPUNGE = 299,
     PROPERTIES = 300,
     SET = 301,
     LOGFILE = 302,
     DAEMON = 303,
     SYSLOG = 304,
     IDFILE = 305,
     PIDFILE = 306,
     INVISIBLE = 307,
     POSTMASTER = 308,
     BOUNCEMAIL = 309,
     SPAMBOUNCE = 310,
     SOFTBOUNCE = 311,
     SHOWDOTS = 312,
     BADHEADER = 313,
     ACCEPT = 314,
     REJECT_ = 315,
     PROTO = 316,
     AUTHTYPE = 317,
     STRING = 318,
     NUMBER = 319,
     NO = 320,
     KEEP = 321,
     FLUSH = 322,
     LIMITFLUSH = 323,
     FETCHALL = 324,
     REWRITE = 325,
     FORCECR = 326,
     STRIPCR = 327,
     PASS8BITS = 328,
     DROPSTATUS = 329,
     DROPDELIVERED = 330,
     DNS = 331,
     SERVICE = 332,
     PORT = 333,
     UIDL = 334,
     INTERVAL = 335,
     MIMEDECODE = 336,
     IDLE = 337,
     CHECKALIAS = 338,
     SSL = 339,
     SSLKEY = 340,
     SSLCERT = 341,
     SSLPROTO = 342,
     SSLCERTCK = 343,
     SSLCERTFILE = 344,
     SSLCERTPATH = 345,
     SSLCOMMONNAME = 346,
     SSLFINGERPRINT = 347,
     PRINCIPAL = 348,
     ESMTPNAME = 349,
     ESMTPPASSWORD = 350,
     AUTHMETH = 351,
     TRACEPOLLS = 352
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
#define BATCHLIMIT 295
#define FETCHLIMIT 296
#define FETCHSIZELIMIT 297
#define FASTUIDL 298
#define EXPUNGE 299
#define PROPERTIES 300
#define SET 301
#define LOGFILE 302
#define DAEMON 303
#define SYSLOG 304
#define IDFILE 305
#define PIDFILE 306
#define INVISIBLE 307
#define POSTMASTER 308
#define BOUNCEMAIL 309
#define SPAMBOUNCE 310
#define SOFTBOUNCE 311
#define SHOWDOTS 312
#define BADHEADER 313
#define ACCEPT 314
#define REJECT_ 315
#define PROTO 316
#define AUTHTYPE 317
#define STRING 318
#define NUMBER 319
#define NO 320
#define KEEP 321
#define FLUSH 322
#define LIMITFLUSH 323
#define FETCHALL 324
#define REWRITE 325
#define FORCECR 326
#define STRIPCR 327
#define PASS8BITS 328
#define DROPSTATUS 329
#define DROPDELIVERED 330
#define DNS 331
#define SERVICE 332
#define PORT 333
#define UIDL 334
#define INTERVAL 335
#define MIMEDECODE 336
#define IDLE 337
#define CHECKALIAS 338
#define SSL 339
#define SSLKEY 340
#define SSLCERT 341
#define SSLPROTO 342
#define SSLCERTCK 343
#define SSLCERTFILE 344
#define SSLCERTPATH 345
#define SSLCOMMONNAME 346
#define SSLFINGERPRINT 347
#define PRINCIPAL 348
#define ESMTPNAME 349
#define ESMTPPASSWORD 350
#define AUTHMETH 351
#define TRACEPOLLS 352




#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
{

/* Line 214 of yacc.c  */
#line 58 "rcfile_y.y"

  int proto;
  int number;
  char *sval;



/* Line 214 of yacc.c  */
#line 368 "rcfile_y.c"
} YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
#endif


/* Copy the second part of user declarations.  */


/* Line 264 of yacc.c  */
#line 380 "rcfile_y.c"

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

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  24
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   314

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  98
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  23
/* YYNRULES -- Number of rules.  */
#define YYNRULES  153
/* YYNRULES -- Number of states.  */
#define YYNSTATES  224

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   352

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
      85,    86,    87,    88,    89,    90,    91,    92,    93,    94,
      95,    96,    97
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const yytype_uint16 yyprhs[] =
{
       0,     0,     3,     4,     6,     8,    11,    13,    14,    19,
      24,    29,    34,    39,    42,    46,    49,    53,    56,    60,
      65,    68,    72,    75,    79,    82,    86,    89,    93,    98,
     101,   104,   106,   107,   110,   112,   115,   117,   120,   123,
     126,   129,   132,   135,   138,   141,   144,   147,   149,   152,
     154,   157,   160,   163,   166,   169,   172,   175,   179,   182,
     185,   188,   191,   194,   197,   199,   202,   205,   207,   210,
     213,   216,   218,   220,   222,   225,   228,   231,   235,   239,
     240,   243,   245,   248,   250,   253,   255,   259,   261,   264,
     266,   269,   271,   274,   276,   279,   283,   286,   290,   293,
     297,   300,   303,   306,   309,   312,   315,   318,   321,   324,
     326,   329,   332,   334,   336,   338,   340,   342,   344,   346,
     348,   350,   352,   354,   356,   358,   361,   364,   367,   369,
     372,   375,   378,   381,   384,   387,   390,   393,   396,   399,
     402,   405,   408,   411,   414,   417,   420,   423,   426,   429,
     432,   435,   438,   441
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int8 yyrhs[] =
{
      99,     0,    -1,    -1,   100,    -1,   102,    -1,   100,   102,
      -1,    39,    -1,    -1,    46,    47,   101,    63,    -1,    46,
      50,   101,    63,    -1,    46,    51,   101,    63,    -1,    46,
      48,   101,    64,    -1,    46,    53,   101,    63,    -1,    46,
      54,    -1,    46,    65,    54,    -1,    46,    55,    -1,    46,
      65,    55,    -1,    46,    56,    -1,    46,    65,    56,    -1,
      46,    45,   101,    63,    -1,    46,    49,    -1,    46,    65,
      49,    -1,    46,    52,    -1,    46,    65,    52,    -1,    46,
      57,    -1,    46,    65,    57,    -1,   103,   104,    -1,   103,
     104,   108,    -1,   103,   104,   108,   107,    -1,     4,    63,
      -1,     5,    63,    -1,     3,    -1,    -1,   104,   107,    -1,
      63,    -1,   105,    63,    -1,    63,    -1,   106,    63,    -1,
       7,   105,    -1,     6,    63,    -1,     8,   106,    -1,     9,
      61,    -1,     9,    12,    -1,    93,    63,    -1,    94,    63,
      -1,    95,    63,    -1,     9,    13,    -1,    79,    -1,    65,
      79,    -1,    83,    -1,    65,    83,    -1,    77,    63,    -1,
      77,    64,    -1,    78,    64,    -1,    80,    64,    -1,    10,
      62,    -1,    11,    64,    -1,    14,    64,    63,    -1,    14,
      63,    -1,    15,    63,    -1,    31,    63,    -1,    32,    63,
      -1,    33,    63,    -1,    34,    63,    -1,    76,    -1,    65,
      76,    -1,    65,    14,    -1,    97,    -1,    65,    97,    -1,
      58,    59,    -1,    58,    60,    -1,   113,    -1,   109,    -1,
     110,    -1,   109,   110,    -1,   111,   112,    -1,    16,    63,
      -1,    16,   114,    36,    -1,    16,    63,    37,    -1,    -1,
     112,   120,    -1,   120,    -1,   113,   120,    -1,   115,    -1,
     114,   115,    -1,    63,    -1,    63,    39,    63,    -1,    63,
      -1,   116,    63,    -1,    63,    -1,   117,    63,    -1,    63,
      -1,   118,    63,    -1,    64,    -1,   119,    64,    -1,    38,
     114,    36,    -1,    38,   114,    -1,    35,   114,    36,    -1,
      35,   114,    -1,    35,    63,    37,    -1,    17,    63,    -1,
      18,   116,    -1,    19,   117,    -1,    20,   118,    -1,    24,
      63,    -1,    25,    63,    -1,    26,   119,    -1,    21,    63,
      -1,    22,    63,    -1,    23,    -1,    27,    63,    -1,    28,
      63,    -1,    66,    -1,    67,    -1,    68,    -1,    69,    -1,
      70,    -1,    71,    -1,    72,    -1,    73,    -1,    74,    -1,
      75,    -1,    81,    -1,    82,    -1,    84,    -1,    85,    63,
      -1,    86,    63,    -1,    87,    63,    -1,    88,    -1,    89,
      63,    -1,    90,    63,    -1,    91,    63,    -1,    92,    63,
      -1,    65,    66,    -1,    65,    67,    -1,    65,    68,    -1,
      65,    69,    -1,    65,    70,    -1,    65,    71,    -1,    65,
      72,    -1,    65,    73,    -1,    65,    74,    -1,    65,    75,
      -1,    65,    81,    -1,    65,    82,    -1,    65,    84,    -1,
      29,    64,    -1,    30,    64,    -1,    41,    64,    -1,    42,
      64,    -1,    43,    64,    -1,    40,    64,    -1,    44,    64,
      -1,    45,    63,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,    90,    90,    91,    94,    95,    98,    98,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,   112,
     113,   114,   115,   116,   117,   118,   126,   127,   130,   134,
     135,   136,   139,   140,   143,   144,   147,   148,   151,   152,
     153,   154,   155,   166,   167,   168,   169,   177,   178,   179,
     180,   181,   184,   190,   196,   198,   200,   202,   207,   213,
     214,   222,   230,   231,   232,   233,   234,   235,   236,   237,
     238,   241,   242,   245,   246,   249,   252,   253,   254,   257,
     258,   261,   262,   265,   266,   269,   275,   278,   279,   282,
     283,   286,   287,   290,   296,   304,   305,   306,   307,   309,
     310,   311,   312,   313,   314,   315,   316,   317,   318,   319,
     320,   321,   323,   324,   325,   326,   327,   328,   329,   330,
     331,   332,   333,   334,   336,   343,   344,   345,   346,   347,
     348,   349,   350,   352,   353,   354,   355,   356,   357,   358,
     359,   360,   361,   362,   363,   365,   367,   368,   369,   370,
     371,   372,   373,   375
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
  "MAP", "BATCHLIMIT", "FETCHLIMIT", "FETCHSIZELIMIT", "FASTUIDL",
  "EXPUNGE", "PROPERTIES", "SET", "LOGFILE", "DAEMON", "SYSLOG", "IDFILE",
  "PIDFILE", "INVISIBLE", "POSTMASTER", "BOUNCEMAIL", "SPAMBOUNCE",
  "SOFTBOUNCE", "SHOWDOTS", "BADHEADER", "ACCEPT", "REJECT_", "PROTO",
  "AUTHTYPE", "STRING", "NUMBER", "NO", "KEEP", "FLUSH", "LIMITFLUSH",
  "FETCHALL", "REWRITE", "FORCECR", "STRIPCR", "PASS8BITS", "DROPSTATUS",
  "DROPDELIVERED", "DNS", "SERVICE", "PORT", "UIDL", "INTERVAL",
  "MIMEDECODE", "IDLE", "CHECKALIAS", "SSL", "SSLKEY", "SSLCERT",
  "SSLPROTO", "SSLCERTCK", "SSLCERTFILE", "SSLCERTPATH", "SSLCOMMONNAME",
  "SSLFINGERPRINT", "PRINCIPAL", "ESMTPNAME", "ESMTPPASSWORD", "AUTHMETH",
  "TRACEPOLLS", "$accept", "rcfile", "statement_list", "optmap",
  "statement", "define_server", "serverspecs", "alias_list", "domain_list",
  "serv_option", "userspecs", "explicits", "explicitdef", "userdef",
  "user0opts", "user1opts", "mapping_list", "mapping", "folder_list",
  "smtp_list", "fetch_list", "num_list", "user_option", 0
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
     345,   346,   347,   348,   349,   350,   351,   352
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    98,    99,    99,   100,   100,   101,   101,   102,   102,
     102,   102,   102,   102,   102,   102,   102,   102,   102,   102,
     102,   102,   102,   102,   102,   102,   102,   102,   102,   103,
     103,   103,   104,   104,   105,   105,   106,   106,   107,   107,
     107,   107,   107,   107,   107,   107,   107,   107,   107,   107,
     107,   107,   107,   107,   107,   107,   107,   107,   107,   107,
     107,   107,   107,   107,   107,   107,   107,   107,   107,   107,
     107,   108,   108,   109,   109,   110,   111,   111,   111,   112,
     112,   113,   113,   114,   114,   115,   115,   116,   116,   117,
     117,   118,   118,   119,   119,   120,   120,   120,   120,   120,
     120,   120,   120,   120,   120,   120,   120,   120,   120,   120,
     120,   120,   120,   120,   120,   120,   120,   120,   120,   120,
     120,   120,   120,   120,   120,   120,   120,   120,   120,   120,
     120,   120,   120,   120,   120,   120,   120,   120,   120,   120,
     120,   120,   120,   120,   120,   120,   120,   120,   120,   120,
     120,   120,   120,   120
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     0,     1,     1,     2,     1,     0,     4,     4,
       4,     4,     4,     2,     3,     2,     3,     2,     3,     4,
       2,     3,     2,     3,     2,     3,     2,     3,     4,     2,
       2,     1,     0,     2,     1,     2,     1,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     1,     2,     1,
       2,     2,     2,     2,     2,     2,     2,     3,     2,     2,
       2,     2,     2,     2,     1,     2,     2,     1,     2,     2,
       2,     1,     1,     1,     2,     2,     2,     3,     3,     0,
       2,     1,     2,     1,     2,     1,     3,     1,     2,     1,
       2,     1,     2,     1,     2,     3,     2,     3,     2,     3,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     1,
       2,     2,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     2,     2,     2,     1,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       2,    31,     0,     0,     0,     0,     3,     4,    32,    29,
      30,     7,     7,     7,    20,     7,     7,    22,     7,    13,
      15,    17,    24,     0,     1,     5,    26,     6,     0,     0,
       0,     0,     0,     0,    21,    23,    14,    16,    18,    25,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   109,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   112,   113,   114,
     115,   116,   117,   118,   119,   120,   121,    64,     0,     0,
      47,     0,   122,   123,    49,   124,     0,     0,     0,   128,
       0,     0,     0,     0,     0,     0,     0,    67,    33,    27,
      72,    73,    79,    71,    81,    19,     8,    11,     9,    10,
      12,    39,    34,    38,    36,    40,    42,    46,    41,    55,
      56,    58,     0,    59,    76,     0,    83,   100,    87,   101,
      89,   102,    91,   103,   107,   108,   104,   105,    93,   106,
     110,   111,   146,   147,    60,    61,    62,    63,    85,    98,
      85,    96,   151,   148,   149,   150,   152,   153,    69,    70,
      66,   133,   134,   135,   136,   137,   138,   139,   140,   141,
     142,    65,    48,   143,   144,    50,   145,    68,    51,    52,
      53,    54,   125,   126,   127,   129,   130,   131,   132,    43,
      44,    45,     0,    28,    74,    75,     0,    82,    35,    37,
      57,    78,     0,    77,    84,    88,    90,    92,    94,    99,
      97,    95,    80,    86
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     5,     6,    28,     7,     8,    26,   123,   125,   108,
     109,   110,   111,   112,   205,   113,   135,   136,   139,   141,
     143,   149,   114
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -114
static const yytype_int16 yypact[] =
{
      55,  -114,   -62,   -54,   249,    45,    55,  -114,  -114,  -114,
    -114,    14,    14,    14,  -114,    14,    14,  -114,    14,  -114,
    -114,  -114,  -114,    93,  -114,  -114,    -4,  -114,    -8,    54,
      59,    61,    63,    64,  -114,  -114,  -114,  -114,  -114,  -114,
      65,    66,    67,    39,    56,    71,   -31,    68,    70,    76,
      78,    80,    81,    89,    91,  -114,    92,    94,    87,    95,
      96,    97,    98,   100,   106,   107,   109,   110,   111,   112,
     113,   114,   115,   116,   121,    -3,   120,  -114,  -114,  -114,
    -114,  -114,  -114,  -114,  -114,  -114,  -114,  -114,    49,   133,
    -114,   134,  -114,  -114,  -114,  -114,   137,   142,   143,  -114,
     144,   145,   146,   147,   148,   149,   150,  -114,  -114,    88,
     140,  -114,  -114,   201,  -114,  -114,  -114,  -114,  -114,  -114,
    -114,  -114,  -114,   151,  -114,   152,  -114,  -114,  -114,  -114,
    -114,  -114,   153,  -114,    69,   -28,  -114,  -114,  -114,   169,
    -114,   170,  -114,   171,  -114,  -114,  -114,  -114,  -114,   173,
    -114,  -114,  -114,  -114,  -114,  -114,  -114,  -114,    11,    73,
     136,    74,  -114,  -114,  -114,  -114,  -114,  -114,  -114,  -114,
    -114,  -114,  -114,  -114,  -114,  -114,  -114,  -114,  -114,  -114,
    -114,  -114,  -114,  -114,  -114,  -114,  -114,  -114,  -114,  -114,
    -114,  -114,  -114,  -114,  -114,  -114,  -114,  -114,  -114,  -114,
    -114,  -114,    28,  -114,  -114,   201,   181,  -114,  -114,  -114,
    -114,  -114,   172,  -114,  -114,  -114,  -114,  -114,  -114,  -114,
    -114,  -114,  -114,  -114
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -114,  -114,  -114,    31,   154,  -114,  -114,  -114,  -114,   129,
    -114,  -114,   130,  -114,  -114,  -114,    48,   -21,  -114,  -114,
    -114,  -114,  -113
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -86
static const yytype_int16 yytable[] =
{
     207,     9,    40,    41,    42,    43,    44,    45,   213,    10,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,   131,   132,    68,   160,    69,    70,    71,    72,
      73,    74,   170,    29,    30,    24,    31,    32,   219,    33,
     212,   126,   127,    27,    75,   115,   168,   169,     1,     2,
       3,    76,    77,    78,    79,    80,    81,    82,    83,    84,
      85,    86,    87,    88,    89,    90,    91,    92,    93,    94,
      95,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   222,   107,    40,    41,    42,    43,    44,    45,
     128,     4,    46,    47,   181,   -85,   211,   182,   212,   220,
     221,   185,   188,   189,   214,   159,   161,   116,   129,    63,
      64,    65,    66,   117,   118,   187,   119,   120,   121,   122,
     124,   133,   -85,   134,   170,   130,   160,   160,   214,   137,
     214,   138,    34,   140,   142,    35,    75,    36,    37,    38,
      39,   148,   144,   202,   145,   146,    48,   147,   150,   151,
      25,   152,   153,   154,    87,    88,    89,    90,    91,   155,
     156,    94,   157,   158,   160,   212,   162,   163,   164,   165,
     166,   104,   105,   106,   167,   107,   171,   172,   173,   174,
     175,   176,   177,   178,   179,   180,   181,   190,   191,   182,
     192,   183,   184,   185,   186,   193,   194,   195,   196,   197,
     198,   199,   200,   201,   208,   209,   210,   187,    49,    50,
      51,    52,    53,    54,    55,    56,    57,    58,    59,    60,
      61,    62,   215,   216,   217,   223,    67,   218,   203,    68,
     204,    69,    70,    71,    72,    73,    74,   171,   172,   173,
     174,   175,   176,   177,   178,   179,   180,     0,     0,     0,
       0,     0,   183,   184,     0,   186,   206,    77,    78,    79,
      80,    81,    82,    83,    84,    85,    86,     0,     0,     0,
       0,     0,    92,    93,     0,    95,    96,    97,    98,    99,
     100,   101,   102,   103,    11,     0,    12,    13,    14,    15,
      16,    17,    18,    19,    20,    21,    22,     0,     0,     0,
       0,     0,     0,     0,    23
};

static const yytype_int16 yycheck[] =
{
     113,    63,     6,     7,     8,     9,    10,    11,    36,    63,
      14,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      34,    35,    63,    64,    38,    63,    40,    41,    42,    43,
      44,    45,    14,    12,    13,     0,    15,    16,    37,    18,
      39,    12,    13,    39,    58,    63,    59,    60,     3,     4,
       5,    65,    66,    67,    68,    69,    70,    71,    72,    73,
      74,    75,    76,    77,    78,    79,    80,    81,    82,    83,
      84,    85,    86,    87,    88,    89,    90,    91,    92,    93,
      94,    95,   205,    97,     6,     7,     8,     9,    10,    11,
      61,    46,    14,    15,    76,    36,    37,    79,    39,    36,
      36,    83,    63,    64,   135,    67,    68,    63,    62,    31,
      32,    33,    34,    64,    63,    97,    63,    63,    63,    63,
      63,    63,    63,    63,    14,    64,    63,    63,   159,    63,
     161,    63,    49,    63,    63,    52,    58,    54,    55,    56,
      57,    64,    63,    65,    63,    63,    16,    63,    63,    63,
       6,    64,    64,    63,    76,    77,    78,    79,    80,    63,
      63,    83,    63,    63,    63,    39,    64,    64,    64,    64,
      64,    93,    94,    95,    63,    97,    66,    67,    68,    69,
      70,    71,    72,    73,    74,    75,    76,    64,    64,    79,
      63,    81,    82,    83,    84,    63,    63,    63,    63,    63,
      63,    63,    63,    63,    63,    63,    63,    97,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    63,    63,    63,    63,    35,    64,   109,    38,
     110,    40,    41,    42,    43,    44,    45,    66,    67,    68,
      69,    70,    71,    72,    73,    74,    75,    -1,    -1,    -1,
      -1,    -1,    81,    82,    -1,    84,    65,    66,    67,    68,
      69,    70,    71,    72,    73,    74,    75,    -1,    -1,    -1,
      -1,    -1,    81,    82,    -1,    84,    85,    86,    87,    88,
      89,    90,    91,    92,    45,    -1,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    56,    57,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    65
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,     3,     4,     5,    46,    99,   100,   102,   103,    63,
      63,    45,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    65,     0,   102,   104,    39,   101,   101,
     101,   101,   101,   101,    49,    52,    54,    55,    56,    57,
       6,     7,     8,     9,    10,    11,    14,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    32,    33,    34,    35,    38,    40,
      41,    42,    43,    44,    45,    58,    65,    66,    67,    68,
      69,    70,    71,    72,    73,    74,    75,    76,    77,    78,
      79,    80,    81,    82,    83,    84,    85,    86,    87,    88,
      89,    90,    91,    92,    93,    94,    95,    97,   107,   108,
     109,   110,   111,   113,   120,    63,    63,    64,    63,    63,
      63,    63,    63,   105,    63,   106,    12,    13,    61,    62,
      64,    63,    64,    63,    63,   114,   115,    63,    63,   116,
      63,   117,    63,   118,    63,    63,    63,    63,    64,   119,
      63,    63,    64,    64,    63,    63,    63,    63,    63,   114,
      63,   114,    64,    64,    64,    64,    64,    63,    59,    60,
      14,    66,    67,    68,    69,    70,    71,    72,    73,    74,
      75,    76,    79,    81,    82,    83,    84,    97,    63,    64,
      64,    64,    63,    63,    63,    63,    63,    63,    63,    63,
      63,    63,    65,   107,   110,   112,    65,   120,    63,    63,
      63,    37,    39,    36,   115,    63,    63,    63,    64,    37,
      36,    36,   120,    63
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
# if defined YYLTYPE_IS_TRIVIAL && YYLTYPE_IS_TRIVIAL
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
      case 63: /* "STRING" */

/* Line 1009 of yacc.c  */
#line 86 "rcfile_y.y"
	{ free ((yyvaluep->sval)); };

/* Line 1009 of yacc.c  */
#line 1544 "rcfile_y.c"
	break;

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


/* The lookahead symbol.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;

/* Number of syntax errors so far.  */
int yynerrs;



/*-------------------------.
| yyparse or yypush_parse.  |
`-------------------------*/

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

       Refer to the stacks thru separate pointers, to allow yyoverflow
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
  int yytoken;
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

  yytoken = 0;
  yyss = yyssa;
  yyvs = yyvsa;
  yystacksize = YYINITDEPTH;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY; /* Cause a token to be read.  */

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
  if (yyn == YYPACT_NINF)
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
      if (yyn == 0 || yyn == YYTABLE_NINF)
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

/* Line 1464 of yacc.c  */
#line 101 "rcfile_y.y"
    {run.logfile = prependdir ((yyvsp[(4) - (4)].sval), rcfiledir); free((yyvsp[(4) - (4)].sval));}
    break;

  case 9:

/* Line 1464 of yacc.c  */
#line 102 "rcfile_y.y"
    {run.idfile = prependdir ((yyvsp[(4) - (4)].sval), rcfiledir); free((yyvsp[(4) - (4)].sval));}
    break;

  case 10:

/* Line 1464 of yacc.c  */
#line 103 "rcfile_y.y"
    {run.pidfile = prependdir ((yyvsp[(4) - (4)].sval), rcfiledir); free((yyvsp[(4) - (4)].sval));}
    break;

  case 11:

/* Line 1464 of yacc.c  */
#line 104 "rcfile_y.y"
    {run.poll_interval = (yyvsp[(4) - (4)].number);}
    break;

  case 12:

/* Line 1464 of yacc.c  */
#line 105 "rcfile_y.y"
    {run.postmaster = (yyvsp[(4) - (4)].sval);}
    break;

  case 13:

/* Line 1464 of yacc.c  */
#line 106 "rcfile_y.y"
    {run.bouncemail = TRUE;}
    break;

  case 14:

/* Line 1464 of yacc.c  */
#line 107 "rcfile_y.y"
    {run.bouncemail = FALSE;}
    break;

  case 15:

/* Line 1464 of yacc.c  */
#line 108 "rcfile_y.y"
    {run.spambounce = TRUE;}
    break;

  case 16:

/* Line 1464 of yacc.c  */
#line 109 "rcfile_y.y"
    {run.spambounce = FALSE;}
    break;

  case 17:

/* Line 1464 of yacc.c  */
#line 110 "rcfile_y.y"
    {run.softbounce = TRUE;}
    break;

  case 18:

/* Line 1464 of yacc.c  */
#line 111 "rcfile_y.y"
    {run.softbounce = FALSE;}
    break;

  case 19:

/* Line 1464 of yacc.c  */
#line 112 "rcfile_y.y"
    {run.properties = (yyvsp[(4) - (4)].sval);}
    break;

  case 20:

/* Line 1464 of yacc.c  */
#line 113 "rcfile_y.y"
    {run.use_syslog = TRUE;}
    break;

  case 21:

/* Line 1464 of yacc.c  */
#line 114 "rcfile_y.y"
    {run.use_syslog = FALSE;}
    break;

  case 22:

/* Line 1464 of yacc.c  */
#line 115 "rcfile_y.y"
    {run.invisible = TRUE;}
    break;

  case 23:

/* Line 1464 of yacc.c  */
#line 116 "rcfile_y.y"
    {run.invisible = FALSE;}
    break;

  case 24:

/* Line 1464 of yacc.c  */
#line 117 "rcfile_y.y"
    {run.showdots = FLAG_TRUE;}
    break;

  case 25:

/* Line 1464 of yacc.c  */
#line 118 "rcfile_y.y"
    {run.showdots = FLAG_FALSE;}
    break;

  case 26:

/* Line 1464 of yacc.c  */
#line 126 "rcfile_y.y"
    {record_current();}
    break;

  case 28:

/* Line 1464 of yacc.c  */
#line 131 "rcfile_y.y"
    {yyerror(GT_("server option after user options"));}
    break;

  case 29:

/* Line 1464 of yacc.c  */
#line 134 "rcfile_y.y"
    {reset_server((yyvsp[(2) - (2)].sval), FALSE); free((yyvsp[(2) - (2)].sval));}
    break;

  case 30:

/* Line 1464 of yacc.c  */
#line 135 "rcfile_y.y"
    {reset_server((yyvsp[(2) - (2)].sval), TRUE);  free((yyvsp[(2) - (2)].sval));}
    break;

  case 31:

/* Line 1464 of yacc.c  */
#line 136 "rcfile_y.y"
    {reset_server("defaults", FALSE);}
    break;

  case 34:

/* Line 1464 of yacc.c  */
#line 143 "rcfile_y.y"
    {save_str(&current.server.akalist,(yyvsp[(1) - (1)].sval),0); free((yyvsp[(1) - (1)].sval));}
    break;

  case 35:

/* Line 1464 of yacc.c  */
#line 144 "rcfile_y.y"
    {save_str(&current.server.akalist,(yyvsp[(2) - (2)].sval),0); free((yyvsp[(2) - (2)].sval));}
    break;

  case 36:

/* Line 1464 of yacc.c  */
#line 147 "rcfile_y.y"
    {save_str(&current.server.localdomains,(yyvsp[(1) - (1)].sval),0); free((yyvsp[(1) - (1)].sval));}
    break;

  case 37:

/* Line 1464 of yacc.c  */
#line 148 "rcfile_y.y"
    {save_str(&current.server.localdomains,(yyvsp[(2) - (2)].sval),0); free((yyvsp[(2) - (2)].sval));}
    break;

  case 39:

/* Line 1464 of yacc.c  */
#line 152 "rcfile_y.y"
    {current.server.via = (yyvsp[(2) - (2)].sval);}
    break;

  case 41:

/* Line 1464 of yacc.c  */
#line 154 "rcfile_y.y"
    {current.server.protocol = (yyvsp[(2) - (2)].proto);}
    break;

  case 42:

/* Line 1464 of yacc.c  */
#line 155 "rcfile_y.y"
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

  case 43:

/* Line 1464 of yacc.c  */
#line 166 "rcfile_y.y"
    {current.server.principal = (yyvsp[(2) - (2)].sval);}
    break;

  case 44:

/* Line 1464 of yacc.c  */
#line 167 "rcfile_y.y"
    {current.server.esmtp_name = (yyvsp[(2) - (2)].sval);}
    break;

  case 45:

/* Line 1464 of yacc.c  */
#line 168 "rcfile_y.y"
    {current.server.esmtp_password = (yyvsp[(2) - (2)].sval);}
    break;

  case 46:

/* Line 1464 of yacc.c  */
#line 169 "rcfile_y.y"
    {
#ifdef SDPS_ENABLE
					    current.server.protocol = P_POP3;
					    current.server.sdps = TRUE;
#else
					    yyerror(GT_("SDPS not enabled."));
#endif /* SDPS_ENABLE */
					}
    break;

  case 47:

/* Line 1464 of yacc.c  */
#line 177 "rcfile_y.y"
    {current.server.uidl = FLAG_TRUE;}
    break;

  case 48:

/* Line 1464 of yacc.c  */
#line 178 "rcfile_y.y"
    {current.server.uidl  = FLAG_FALSE;}
    break;

  case 49:

/* Line 1464 of yacc.c  */
#line 179 "rcfile_y.y"
    {current.server.checkalias = FLAG_TRUE;}
    break;

  case 50:

/* Line 1464 of yacc.c  */
#line 180 "rcfile_y.y"
    {current.server.checkalias  = FLAG_FALSE;}
    break;

  case 51:

/* Line 1464 of yacc.c  */
#line 181 "rcfile_y.y"
    {
					current.server.service = (yyvsp[(2) - (2)].sval);
					}
    break;

  case 52:

/* Line 1464 of yacc.c  */
#line 184 "rcfile_y.y"
    {
					int port = (yyvsp[(2) - (2)].number);
					char buf[10];
					snprintf(buf, sizeof buf, "%d", port);
					current.server.service = xstrdup(buf);
		}
    break;

  case 53:

/* Line 1464 of yacc.c  */
#line 190 "rcfile_y.y"
    {
					int port = (yyvsp[(2) - (2)].number);
					char buf[10];
					snprintf(buf, sizeof buf, "%d", port);
					current.server.service = xstrdup(buf);
		}
    break;

  case 54:

/* Line 1464 of yacc.c  */
#line 197 "rcfile_y.y"
    {current.server.interval = (yyvsp[(2) - (2)].number);}
    break;

  case 55:

/* Line 1464 of yacc.c  */
#line 199 "rcfile_y.y"
    {current.server.authenticate = (yyvsp[(2) - (2)].proto);}
    break;

  case 56:

/* Line 1464 of yacc.c  */
#line 201 "rcfile_y.y"
    {current.server.timeout = (yyvsp[(2) - (2)].number);}
    break;

  case 57:

/* Line 1464 of yacc.c  */
#line 203 "rcfile_y.y"
    {
					    current.server.envelope = (yyvsp[(3) - (3)].sval);
					    current.server.envskip = (yyvsp[(2) - (3)].number);
					}
    break;

  case 58:

/* Line 1464 of yacc.c  */
#line 208 "rcfile_y.y"
    {
					    current.server.envelope = (yyvsp[(2) - (2)].sval);
					    current.server.envskip = 0;
					}
    break;

  case 59:

/* Line 1464 of yacc.c  */
#line 213 "rcfile_y.y"
    {current.server.qvirtual = (yyvsp[(2) - (2)].sval);}
    break;

  case 60:

/* Line 1464 of yacc.c  */
#line 214 "rcfile_y.y"
    {
#ifdef CAN_MONITOR
					interface_parse((yyvsp[(2) - (2)].sval), &current.server);
#else
					fprintf(stderr, GT_("fetchmail: interface option is only supported under Linux (without IPv6) and FreeBSD\n"));
#endif
					free((yyvsp[(2) - (2)].sval));
					}
    break;

  case 61:

/* Line 1464 of yacc.c  */
#line 222 "rcfile_y.y"
    {
#ifdef CAN_MONITOR
					current.server.monitor = (yyvsp[(2) - (2)].sval);
#else
					fprintf(stderr, GT_("fetchmail: monitor option is only supported under Linux (without IPv6) and FreeBSD\n"));
					free((yyvsp[(2) - (2)].sval));
#endif
					}
    break;

  case 62:

/* Line 1464 of yacc.c  */
#line 230 "rcfile_y.y"
    { current.server.plugin = (yyvsp[(2) - (2)].sval); }
    break;

  case 63:

/* Line 1464 of yacc.c  */
#line 231 "rcfile_y.y"
    { current.server.plugout = (yyvsp[(2) - (2)].sval); }
    break;

  case 64:

/* Line 1464 of yacc.c  */
#line 232 "rcfile_y.y"
    {current.server.dns = FLAG_TRUE;}
    break;

  case 65:

/* Line 1464 of yacc.c  */
#line 233 "rcfile_y.y"
    {current.server.dns = FLAG_FALSE;}
    break;

  case 66:

/* Line 1464 of yacc.c  */
#line 234 "rcfile_y.y"
    {current.server.envelope = STRING_DISABLED;}
    break;

  case 67:

/* Line 1464 of yacc.c  */
#line 235 "rcfile_y.y"
    {current.server.tracepolls = FLAG_TRUE;}
    break;

  case 68:

/* Line 1464 of yacc.c  */
#line 236 "rcfile_y.y"
    {current.server.tracepolls = FLAG_FALSE;}
    break;

  case 69:

/* Line 1464 of yacc.c  */
#line 237 "rcfile_y.y"
    {current.server.badheader = BHACCEPT;}
    break;

  case 70:

/* Line 1464 of yacc.c  */
#line 238 "rcfile_y.y"
    {current.server.badheader = BHREJECT;}
    break;

  case 71:

/* Line 1464 of yacc.c  */
#line 241 "rcfile_y.y"
    {record_current(); user_reset();}
    break;

  case 73:

/* Line 1464 of yacc.c  */
#line 245 "rcfile_y.y"
    {record_current(); user_reset();}
    break;

  case 74:

/* Line 1464 of yacc.c  */
#line 246 "rcfile_y.y"
    {record_current(); user_reset();}
    break;

  case 76:

/* Line 1464 of yacc.c  */
#line 252 "rcfile_y.y"
    {current.remotename = (yyvsp[(2) - (2)].sval);}
    break;

  case 78:

/* Line 1464 of yacc.c  */
#line 254 "rcfile_y.y"
    {current.remotename = (yyvsp[(2) - (3)].sval);}
    break;

  case 85:

/* Line 1464 of yacc.c  */
#line 269 "rcfile_y.y"
    {if (0 == strcmp((yyvsp[(1) - (1)].sval), "*")) {
					      current.wildcard = TRUE;
					  } else {
					    save_str_pair(&current.localnames, (yyvsp[(1) - (1)].sval), NULL);
					  }
					 free((yyvsp[(1) - (1)].sval));}
    break;

  case 86:

/* Line 1464 of yacc.c  */
#line 275 "rcfile_y.y"
    {save_str_pair(&current.localnames, (yyvsp[(1) - (3)].sval), (yyvsp[(3) - (3)].sval)); free((yyvsp[(1) - (3)].sval)); free((yyvsp[(3) - (3)].sval));}
    break;

  case 87:

/* Line 1464 of yacc.c  */
#line 278 "rcfile_y.y"
    {save_str(&current.mailboxes,(yyvsp[(1) - (1)].sval),0); free((yyvsp[(1) - (1)].sval));}
    break;

  case 88:

/* Line 1464 of yacc.c  */
#line 279 "rcfile_y.y"
    {save_str(&current.mailboxes,(yyvsp[(2) - (2)].sval),0); free((yyvsp[(2) - (2)].sval));}
    break;

  case 89:

/* Line 1464 of yacc.c  */
#line 282 "rcfile_y.y"
    {save_str(&current.smtphunt, (yyvsp[(1) - (1)].sval),TRUE); free((yyvsp[(1) - (1)].sval));}
    break;

  case 90:

/* Line 1464 of yacc.c  */
#line 283 "rcfile_y.y"
    {save_str(&current.smtphunt, (yyvsp[(2) - (2)].sval),TRUE); free((yyvsp[(2) - (2)].sval));}
    break;

  case 91:

/* Line 1464 of yacc.c  */
#line 286 "rcfile_y.y"
    {save_str(&current.domainlist, (yyvsp[(1) - (1)].sval),TRUE); free((yyvsp[(1) - (1)].sval));}
    break;

  case 92:

/* Line 1464 of yacc.c  */
#line 287 "rcfile_y.y"
    {save_str(&current.domainlist, (yyvsp[(2) - (2)].sval),TRUE); free((yyvsp[(2) - (2)].sval));}
    break;

  case 93:

/* Line 1464 of yacc.c  */
#line 291 "rcfile_y.y"
    {
			    struct idlist *id;
			    id = save_str(&current.antispam,STRING_DUMMY,0);
			    id->val.status.num = (yyvsp[(1) - (1)].number);
			}
    break;

  case 94:

/* Line 1464 of yacc.c  */
#line 297 "rcfile_y.y"
    {
			    struct idlist *id;
			    id = save_str(&current.antispam,STRING_DUMMY,0);
			    id->val.status.num = (yyvsp[(2) - (2)].number);
			}
    break;

  case 99:

/* Line 1464 of yacc.c  */
#line 309 "rcfile_y.y"
    {current.remotename  = (yyvsp[(2) - (3)].sval);}
    break;

  case 100:

/* Line 1464 of yacc.c  */
#line 310 "rcfile_y.y"
    {current.password    = (yyvsp[(2) - (2)].sval);}
    break;

  case 104:

/* Line 1464 of yacc.c  */
#line 314 "rcfile_y.y"
    {current.smtpaddress = (yyvsp[(2) - (2)].sval);}
    break;

  case 105:

/* Line 1464 of yacc.c  */
#line 315 "rcfile_y.y"
    {current.smtpname =    (yyvsp[(2) - (2)].sval);}
    break;

  case 107:

/* Line 1464 of yacc.c  */
#line 317 "rcfile_y.y"
    {current.mda         = (yyvsp[(2) - (2)].sval);}
    break;

  case 108:

/* Line 1464 of yacc.c  */
#line 318 "rcfile_y.y"
    {current.bsmtp       = prependdir ((yyvsp[(2) - (2)].sval), rcfiledir); free((yyvsp[(2) - (2)].sval));}
    break;

  case 109:

/* Line 1464 of yacc.c  */
#line 319 "rcfile_y.y"
    {current.listener    = LMTP_MODE;}
    break;

  case 110:

/* Line 1464 of yacc.c  */
#line 320 "rcfile_y.y"
    {current.preconnect  = (yyvsp[(2) - (2)].sval);}
    break;

  case 111:

/* Line 1464 of yacc.c  */
#line 321 "rcfile_y.y"
    {current.postconnect = (yyvsp[(2) - (2)].sval);}
    break;

  case 112:

/* Line 1464 of yacc.c  */
#line 323 "rcfile_y.y"
    {current.keep        = FLAG_TRUE;}
    break;

  case 113:

/* Line 1464 of yacc.c  */
#line 324 "rcfile_y.y"
    {current.flush       = FLAG_TRUE;}
    break;

  case 114:

/* Line 1464 of yacc.c  */
#line 325 "rcfile_y.y"
    {current.limitflush  = FLAG_TRUE;}
    break;

  case 115:

/* Line 1464 of yacc.c  */
#line 326 "rcfile_y.y"
    {current.fetchall    = FLAG_TRUE;}
    break;

  case 116:

/* Line 1464 of yacc.c  */
#line 327 "rcfile_y.y"
    {current.rewrite     = FLAG_TRUE;}
    break;

  case 117:

/* Line 1464 of yacc.c  */
#line 328 "rcfile_y.y"
    {current.forcecr     = FLAG_TRUE;}
    break;

  case 118:

/* Line 1464 of yacc.c  */
#line 329 "rcfile_y.y"
    {current.stripcr     = FLAG_TRUE;}
    break;

  case 119:

/* Line 1464 of yacc.c  */
#line 330 "rcfile_y.y"
    {current.pass8bits   = FLAG_TRUE;}
    break;

  case 120:

/* Line 1464 of yacc.c  */
#line 331 "rcfile_y.y"
    {current.dropstatus  = FLAG_TRUE;}
    break;

  case 121:

/* Line 1464 of yacc.c  */
#line 332 "rcfile_y.y"
    {current.dropdelivered = FLAG_TRUE;}
    break;

  case 122:

/* Line 1464 of yacc.c  */
#line 333 "rcfile_y.y"
    {current.mimedecode  = FLAG_TRUE;}
    break;

  case 123:

/* Line 1464 of yacc.c  */
#line 334 "rcfile_y.y"
    {current.idle        = FLAG_TRUE;}
    break;

  case 124:

/* Line 1464 of yacc.c  */
#line 336 "rcfile_y.y"
    {
#ifdef SSL_ENABLE
		    current.use_ssl = FLAG_TRUE;
#else
		    yyerror(GT_("SSL is not enabled"));
#endif 
		}
    break;

  case 125:

/* Line 1464 of yacc.c  */
#line 343 "rcfile_y.y"
    {current.sslkey = prependdir ((yyvsp[(2) - (2)].sval), rcfiledir); free((yyvsp[(2) - (2)].sval));}
    break;

  case 126:

/* Line 1464 of yacc.c  */
#line 344 "rcfile_y.y"
    {current.sslcert = prependdir ((yyvsp[(2) - (2)].sval), rcfiledir); free((yyvsp[(2) - (2)].sval));}
    break;

  case 127:

/* Line 1464 of yacc.c  */
#line 345 "rcfile_y.y"
    {current.sslproto = (yyvsp[(2) - (2)].sval);}
    break;

  case 128:

/* Line 1464 of yacc.c  */
#line 346 "rcfile_y.y"
    {current.sslcertck = FLAG_TRUE;}
    break;

  case 129:

/* Line 1464 of yacc.c  */
#line 347 "rcfile_y.y"
    {current.sslcertfile = prependdir((yyvsp[(2) - (2)].sval), rcfiledir); free((yyvsp[(2) - (2)].sval));}
    break;

  case 130:

/* Line 1464 of yacc.c  */
#line 348 "rcfile_y.y"
    {current.sslcertpath = prependdir((yyvsp[(2) - (2)].sval), rcfiledir); free((yyvsp[(2) - (2)].sval));}
    break;

  case 131:

/* Line 1464 of yacc.c  */
#line 349 "rcfile_y.y"
    {current.sslcommonname = (yyvsp[(2) - (2)].sval);}
    break;

  case 132:

/* Line 1464 of yacc.c  */
#line 350 "rcfile_y.y"
    {current.sslfingerprint = (yyvsp[(2) - (2)].sval);}
    break;

  case 133:

/* Line 1464 of yacc.c  */
#line 352 "rcfile_y.y"
    {current.keep        = FLAG_FALSE;}
    break;

  case 134:

/* Line 1464 of yacc.c  */
#line 353 "rcfile_y.y"
    {current.flush       = FLAG_FALSE;}
    break;

  case 135:

/* Line 1464 of yacc.c  */
#line 354 "rcfile_y.y"
    {current.limitflush  = FLAG_FALSE;}
    break;

  case 136:

/* Line 1464 of yacc.c  */
#line 355 "rcfile_y.y"
    {current.fetchall    = FLAG_FALSE;}
    break;

  case 137:

/* Line 1464 of yacc.c  */
#line 356 "rcfile_y.y"
    {current.rewrite     = FLAG_FALSE;}
    break;

  case 138:

/* Line 1464 of yacc.c  */
#line 357 "rcfile_y.y"
    {current.forcecr     = FLAG_FALSE;}
    break;

  case 139:

/* Line 1464 of yacc.c  */
#line 358 "rcfile_y.y"
    {current.stripcr     = FLAG_FALSE;}
    break;

  case 140:

/* Line 1464 of yacc.c  */
#line 359 "rcfile_y.y"
    {current.pass8bits   = FLAG_FALSE;}
    break;

  case 141:

/* Line 1464 of yacc.c  */
#line 360 "rcfile_y.y"
    {current.dropstatus  = FLAG_FALSE;}
    break;

  case 142:

/* Line 1464 of yacc.c  */
#line 361 "rcfile_y.y"
    {current.dropdelivered = FLAG_FALSE;}
    break;

  case 143:

/* Line 1464 of yacc.c  */
#line 362 "rcfile_y.y"
    {current.mimedecode  = FLAG_FALSE;}
    break;

  case 144:

/* Line 1464 of yacc.c  */
#line 363 "rcfile_y.y"
    {current.idle        = FLAG_FALSE;}
    break;

  case 145:

/* Line 1464 of yacc.c  */
#line 365 "rcfile_y.y"
    {current.use_ssl     = FLAG_FALSE;}
    break;

  case 146:

/* Line 1464 of yacc.c  */
#line 367 "rcfile_y.y"
    {current.limit       = NUM_VALUE_IN((yyvsp[(2) - (2)].number));}
    break;

  case 147:

/* Line 1464 of yacc.c  */
#line 368 "rcfile_y.y"
    {current.warnings    = NUM_VALUE_IN((yyvsp[(2) - (2)].number));}
    break;

  case 148:

/* Line 1464 of yacc.c  */
#line 369 "rcfile_y.y"
    {current.fetchlimit  = NUM_VALUE_IN((yyvsp[(2) - (2)].number));}
    break;

  case 149:

/* Line 1464 of yacc.c  */
#line 370 "rcfile_y.y"
    {current.fetchsizelimit = NUM_VALUE_IN((yyvsp[(2) - (2)].number));}
    break;

  case 150:

/* Line 1464 of yacc.c  */
#line 371 "rcfile_y.y"
    {current.fastuidl    = NUM_VALUE_IN((yyvsp[(2) - (2)].number));}
    break;

  case 151:

/* Line 1464 of yacc.c  */
#line 372 "rcfile_y.y"
    {current.batchlimit  = NUM_VALUE_IN((yyvsp[(2) - (2)].number));}
    break;

  case 152:

/* Line 1464 of yacc.c  */
#line 373 "rcfile_y.y"
    {current.expunge     = NUM_VALUE_IN((yyvsp[(2) - (2)].number));}
    break;

  case 153:

/* Line 1464 of yacc.c  */
#line 375 "rcfile_y.y"
    {current.properties  = (yyvsp[(2) - (2)].sval);}
    break;



/* Line 1464 of yacc.c  */
#line 2785 "rcfile_y.c"
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

#if !defined(yyoverflow) || YYERROR_VERBOSE
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



/* Line 1684 of yacc.c  */
#line 377 "rcfile_y.y"


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
    newfile = (char *)xmalloc (strlen (dir) + 1 + strlen (file) + 1);
    if (dir[strlen(dir) - 1] != '/')
	sprintf (newfile, "%s/%s", dir, file);
    else
	sprintf (newfile, "%s%s", dir, file);
    return newfile;
}

/* rcfile_y.y ends here */

