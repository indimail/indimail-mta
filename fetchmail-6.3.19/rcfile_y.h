/* A Bison parser, made by GNU Bison 2.4.3.  */

/* Skeleton interface for Bison's Yacc-like parsers in C
   
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

/* Line 1685 of yacc.c  */
#line 58 "rcfile_y.y"

  int proto;
  int number;
  char *sval;



/* Line 1685 of yacc.c  */
#line 253 "rcfile_y.h"
} YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
#endif

extern YYSTYPE yylval;


