%{
#include "parser.h"
#include "parsers.h"

#ifndef SET_DEFAULT_CONF
#define SET_DEFAULT_CONF(c) (c)->retry = DEFAULT_RETRY; \
                            (c)->matchsleep = DEFAULT_MATCHSLEEP; \
                            (c)->matchcount = DEFAULT_MATCHCOUNT; \
                            (c)->readall = FALSE ; \
                            (c)->parent_mode = FALSE ; \
                            (c)->regex = NULL ;\
							(c)->cmd =  NULL ;\
                            (c)->watchfile = (c)->parent_conffile = NULL
#endif


unsigned int    lineno = 1;
char           *filename;
struct entry_conf **conf_table;
unsigned int    cur_pconf = 0;
FILE           *yyin;


static const struct entry_attribute entry_attributes[] = {
		{"match", A_MATCH, 1, (status_e(*)())regex_parser},
		{"exec", A_EXEC, 1, (status_e(*)())exec_parser},
		{"match_sleep", A_MATCHSLEEP, 1, (status_e(*)())matchsleep_parser},
		{"match_count", A_MATCHCOUNT, 1, (status_e(*)())matchcount_parser},
		{"retry", A_RETRY, 1, (status_e(*)())retry_parser},
		{"user", A_USER, 1, (status_e(*)())user_parser},
		{"readall", A_READALL, 0, (status_e(*)())readall_parser},
		{"verbose", A_VERBOSE, 0, (status_e(*)())verbose_parser},
		{NULL, 0, 0, (status_e(*)())0},
};


%}

%union {
	char           *string;
	int             integer;
}
%token FILENAME
%token MATCH
%token EXECUTE
%token SLEEP
%token RETRY
%token USER
%token READALL
%token VERBOSE
%token EQUAL
%token < string > STRING
%token < integer > NUMBER

%%
config	:
	   |config filename
	   | config match
	   | config exec
	   | config sleep
	   | config retry
	   | config user
	   | config readall
	   | config verbose
	   ;

filename	: FILENAME STRING {
	if (handle_entry_conf($2) == 0)
		yyerror("Error creating file entry %d", cur_pconf);
	free($2);
}

;
match:MATCH EQUAL STRING {
	handle_entry_args(A_MATCH, $3);
	free($3);
}

;
exec:EXECUTE EQUAL STRING {
	handle_entry_args(A_EXEC, $3);
	free($3);
}

;
sleep:SLEEP EQUAL STRING {
	handle_entry_args(A_MATCHSLEEP, $3);
	free($3);
}

;
retry:RETRY EQUAL STRING {
	handle_entry_args(A_RETRY, $3);
	free($3);
}

;
user:USER EQUAL STRING {
	handle_entry_args(A_USER, $3);
	free($3);
}

;
readall:READALL {
	handle_entry_args(A_READALL, NULL);
}

;
verbose:VERBOSE {
	handle_entry_args(A_VERBOSE, NULL);
}

;



%%
struct entry_attribute *
find_arg(unsigned int a_id)
{

	struct entry_attribute *pa;

	for (pa = (struct entry_attribute *) &entry_attributes[0]; pa->a_name; pa++)
		if (a_id == pa->a_id)
			return pa;

	return NULL;
}

void
handle_entry_args(unsigned int a_id, char *value)
{

	struct entry_attribute *pa;
	char           *fixed_val, *aux;

	pa = find_arg(a_id);
	if (!pa)
		return;

	if (pa->a_nvalues == 0) {	//bolean value
		if ((!(*pa->a_parser) (conf_table[cur_pconf])) == OK)
			yyerror("Error while parsing function %s", pa->a_name);
		return;
	}
	//chop out white spaces before and after
	fixed_val = value;
	aux = value + strlen(value);

	while (*fixed_val == ' ')
		fixed_val++;

	while (*aux == ' ')
		*aux-- = '\0';

	if (aux == fixed_val)
		yyerror("null value for %s parameter", pa->a_name);

	// start the specific parser
	if ((!(*pa->a_parser) (conf_table[cur_pconf], fixed_val)) == OK)
		yyerror("Error while parsing function %s", pa->a_name);

	return;

}


int
new_entry_conf(struct entry_conf **new, char *name)
{


	*new = (struct entry_conf *) xmalloc(sizeof (struct entry_conf));

	SET_DEFAULT_CONF(*new);
	(*new)->watchfile = strdup(name);
	if ((*new)->watchfile)
		return 1;
	return 0;
}


int
handle_entry_conf(char *pname)
{

	if (cur_pconf > MAXFILENAME - 1) {
		debug("[*] - MAXFILENAME reached ! No more entries will be created.");
		return 0;
	}
	//fprintf(stdout,"- Criando novo entry conf [%d]: %s\n",cur_pconf,pname);
	return new_entry_conf(&conf_table[cur_pconf], pname);

}


int
yyerror(char *fmt, ...)
{

	va_list         ap;

	va_start(ap, fmt);
	fprintf(stderr, "[!] ERROR:%s:%d: ", filename, lineno);
	vfprintf(stderr, fmt, ap);
	fprintf(stderr, "\n");
	va_end(ap);
	exit(1);
}

int
handle_conf_file(const char *file)
{

	FILE           *input;
	//extern FILE * yyin;

	filename = (char *) file;

	input = fopen(filename, "r");

	if (!input) {
		perror(filename);
		debug("%s: %s", "could not open", (char *) filename);
		exit(1);
	}

	conf_table = (struct entry_conf **) xmalloc((MAXFILENAME + 1) * sizeof (struct entry_conf *));

	yyin = input;

	yyparse();

}


int
exec_parser(struct entry_conf *p, char *value)
{

	//p->cmd = (char *)xmalloc(strlen(value)+1);
	//SCP(p->cmd, value);
	p->cmd = strdup(value);
	return 1;
}

int
matchsleep_parser(struct entry_conf *p, char *value)
{

	p->matchsleep = atoi(value);
	return 1;
}

int
matchcount_parser(struct entry_conf *p, char *value)
{

	p->matchcount = atoi(value);
	return 1;
}


int
regex_parser(struct entry_conf *p, char *value)
{

	char           *s, *e;
/*
 * our regex pattern starts and ends with /
 */

	s = strchr(value, '/');
	e = strrchr(value, '/');
	if ((!s) || (!e) || (s == e))
		yyerror("error in match regex definition - must be inside '/.../' block");

	s++;
	(*e) = '\0';

	p->pattern = strdup(s);

	return 1;
}

int
readall_parser(struct entry_conf *p)
{

	p->readall = 1;
	return 1;

}

int
verbose_parser(struct entry_conf *p)
{

	p->verbose = 1;
	return 1;

}


int
retry_parser(struct entry_conf *p, char *value)
{

	p->retry = atoi(value);
	return 1;
}

int
user_parser(struct entry_conf *p, char *value)
{

	p->user = (char *) xmalloc(strlen(value) + 1);
	SCP(p->user, value);
	return 1;
}
