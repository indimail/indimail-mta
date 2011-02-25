#include "parsers.h"


static const struct entry_attribute entry_attributes[] =
{
   { "match",    A_REGEX,    1,  regex_parser     },
   { "exec",    A_EXEC,    1,  exec_parser     },
   { "match_sleep",    A_MATCHSLEEP,    0,  matchsleep_parser     },
   { "retry",           A_RETRY,           0,  retry_parser            },
   { "user",           A_USER,           1,  user_parser            },
   { "readall",          A_READALL,          -1,  readall_parser  },
   { NULL,          0,          0,  0           },
} ;


int
check_first_word(char *word)
{

	if ( EQS( word, W_FILENAME ) )
		return FILENAME_ENTRY;

	if ( EQC( *word, ENTRY_BEGIN ) )
		return BEGIN_ENTRY;

	if ( EQC( *word, ENTRY_END ) )
		return END_ENTRY;

	return ( BAD_ENTRY ) ;

}

struct entry_attribute *
find_arg(char *cur_arg)
{

	struct entry_attribute *pa;

	for(pa=&entry_attributes[0];pa->a_name;pa++) 
		if ( EQS( cur_arg, pa->a_name ) )
			return pa;

	return NULL;

}

void
handle_entry_args(struct entry_conf *p, char **words)
{

	struct entry_attribute *pa;

	pa = find_arg(words[0]);
	if(!pa) 
		return;

	if( ! (*pa->a_parser)(p, &words[1] ) == OK) 
		debug("Funcao para %s NAO foi ok\n",pa->a_name);
		
}

int
exec_parser(struct entry_conf *p, const char **p_args)
{

	p->cmd = (char *)xmalloc(strlen(p_args[0])+1);
	SCP(p->cmd, p_args[0]);
	return 1;
}

int
matchsleep_parser(struct entry_conf *p, const char **p_args)
{

        p->matchsleep = atoi(p_args[0]);
        return 1;
}

int
matchcount_parser(struct entry_conf *p, const char **p_args)
{

        p->matchcount = atoi(p_args[0]);
        return 1;
}


int
regex_parser(struct entry_conf *p, const char **p_args)
{

	char *s,*e;

	/* our regex pattern starts and ends with /
	 */
	
	s = strchr(p_args[0],'/');
	e = strrchr(p_args[0],'/');
	if( (!s) || (!e) || (s == e) ) {
		debug("error in match regex definition - must be inside '/.../' block");
		exit(1);
	}
	s++;
	(*e) = '\0';
	
        p->regex = (char *)xmalloc(strlen(s)+1);
        SCP(p->regex, s);
        return 1;
}

int
readall_parser(struct entry_conf *p)
{

        p->readall = 1;
	return 1;

}

int
retry_parser(struct entry_conf *p, const char **p_args)
{

	p->retry = atoi(p_args[0]);
	return 1;
}

int
user_parser(struct entry_conf *p, const char **p_args)
{

	p->user = (char *)xmalloc(strlen(p_args[0])+1);
	SCP(p->user, p_args[0]);
	return 1;
}

