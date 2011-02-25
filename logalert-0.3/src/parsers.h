#ifndef PARSERS_H
#define PARSERS_H

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#include "config.h"
#include "defs.h"
#include "utils.h"
#include "log.h"

#ifdef HAVE_PCRE_H
#include<pcre.h>
#endif

#ifdef HAVE_REGEX_H
#include<regex.h>
#endif

#ifdef HAVE_STRING_H
#include <string.h>
#endif

#define A_EXEC 1
#define A_ARGS 2
#define A_RETRY 3
#define A_USER 4
#define A_GROUP 5
#define A_MATCH 6
#define A_MATCHSLEEP 7
#define A_MATCHCOUNT 1
#define A_READALL 8
#define A_VERBOSE 9

#define MAXARGS 20
#define MAXFILENAME 30

#define MAXRGSTR 20
#define MAXRGSUB MAXRGSTR + MAXRGSTR / 2

struct entry_conf {

	unsigned int retry ;
	unsigned int matchsleep ;
	unsigned int matchcount ;
	unsigned int readall ;
	unsigned int parent_mode ;
	unsigned int verbose ;
#ifdef HAVE_PCRE_H
	pcre *regex;
	int rg_sub[MAXRGSUB]; //to store substring index entries
#else
	regex_t *regex;
#endif
	char *pattern;
	char *user;
	char *cmd;
	char *watchfile;
	char *parent_conffile;

};

struct entry_attribute {

	const char  *a_name ;
	unsigned int    a_id ;
	int          a_nvalues ;
	status_e    (*a_parser)() ;

};



typedef enum { NO_ENTRY,
               BAD_ENTRY,
               FILENAME_ENTRY,
               DEFAULTS_ENTRY,
               INCLUDE_ENTRY,
               INCLUDEDIR_ENTRY,
	       BEGIN_ENTRY,
	       END_ENTRY
} entry_t ;


struct entry_attribute * find_arg(unsigned int a_id);
void handle_entry_args(unsigned int a_id, char *value);
int new_entry_conf(struct entry_conf **new, char *name);
int handle_entry_conf(char *pname);
int yyerror(char *fmt, ...);
int handle_conf_file(const char *file);
int exec_parser(struct entry_conf *p, char *value);
int regex_parser(struct entry_conf *p, char *value);
int readall_parser(struct entry_conf *p);
int matchsleep_parser(struct entry_conf *p, char *value);
int matchcount_parser(struct entry_conf *p, char *value);
int retry_parser(struct entry_conf *p, char *value);
int user_parser(struct entry_conf *p, char *value);
int verbose_parser(struct entry_conf *p);


#endif 
