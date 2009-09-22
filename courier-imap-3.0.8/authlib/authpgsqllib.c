/*
** Copyright 2000-2004 Double Precision, Inc.  See COPYING for
** distribution information.
*/


#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	<unistd.h>
#include	<ctype.h>
#include	<sys/types.h>
#include	<sys/stat.h>
#include	<libpq-fe.h>
#include	<time.h>

#include	"authpgsql.h"
#include	"authpgsqlrc.h"
#include	"auth.h"
#include	"debug.h"

/* tom@minnesota.com */
#define		MAX_SUBSTITUTION_LEN	32
#define		SV_BEGIN_MARK		"$("
#define		SV_END_MARK		")"
#define		SV_BEGIN_LEN		((sizeof(SV_BEGIN_MARK))-1)
#define		SV_END_LEN		((sizeof(SV_END_MARK))-1)

static const char rcsid[]="$Id: authpgsqllib.c,v 1.12 2004/05/10 01:31:00 mrsam Exp $";

/* tom@minnesota.com */
struct var_data {			
	const char *name;
	const char *value;
	const size_t size;
	size_t value_length;
	} ;

/* tom@minnesota.com */
typedef int (*parsefunc)(const char *, size_t, void *);

static const char *read_env(const char *env)
{
static char *pgsqlauth=0;
static size_t pgsqlauth_size=0;
size_t	i;
char	*p=0;
int	l=strlen(env);

	if (!pgsqlauth)
	{
	FILE	*f=fopen(AUTHPGSQLRC, "r");
	struct	stat	buf;

		if (!f)	return (0);
		if (fstat(fileno(f), &buf) ||
			(pgsqlauth=malloc(buf.st_size+2)) == 0)
		{
			fclose(f);
			return (0);
		}
		if (fread(pgsqlauth, buf.st_size, 1, f) != 1)
		{
			free(pgsqlauth);
			pgsqlauth=0;
			fclose(f);
			return (0);
		}
		pgsqlauth[pgsqlauth_size=buf.st_size]=0;

		for (i=0; i<pgsqlauth_size; i++)
			if (pgsqlauth[i] == '\n')
			{	/* tom@minnesota.com */
				if (!i || pgsqlauth[i-1] != '\\')
				{
					pgsqlauth[i]='\0';
				}
				else
				{
					pgsqlauth[i]=pgsqlauth[i-1]= ' ';
				}
			}
		fclose(f);
	}

	for (i=0; i<pgsqlauth_size; )
	{
		p=pgsqlauth+i;
		if (memcmp(p, env, l) == 0 &&
			isspace((int)(unsigned char)p[l]))
		{
			p += l;
			while (*p && *p != '\n' &&
				isspace((int)(unsigned char)*p))
				++p;
			break;
		}

		while (i < pgsqlauth_size)
			if (pgsqlauth[i++] == 0)	break;
	}

	if (i < pgsqlauth_size)
		return (p);
	return (0);
}

static PGresult *pgresult=0;

static PGconn *pgconn=0;

/*
static FILE *DEBUG=0;
*/

static int do_connect()
{
const	char *server;
const	char *userid;
const	char *password;
const	char *database;
const 	char *server_port=0;
const 	char *server_opt=0;
/*
	if (!DEBUG) {
		DEBUG=fopen("/tmp/courier.debug","a");
	}
	fprintf(DEBUG,"Apro il DB!\n");
	fflush(DEBUG);
*/

/*
** Periodically detect dead connections.
*/
	if (pgconn)
	{
		static time_t last_time=0;
		time_t t_check;

		time(&t_check);

		if (t_check < last_time)
			last_time=t_check;	/* System clock changed */

		if (t_check < last_time + 60)
			return (0);

		last_time=t_check;
			
		if (PQstatus(pgconn) == CONNECTION_OK) return (0);

		dprintf("authpgsqllib: PQstatus failed, connection lost");
		PQfinish(pgconn);
		pgconn=0;
	}

	server=read_env("PGSQL_HOST");
	server_port=read_env("PGSQL_PORT");
	userid=read_env("PGSQL_USERNAME");
	password=read_env("PGSQL_PASSWORD");
	database=read_env("PGSQL_DATABASE");
	server_opt=read_env("PGSQL_OPT");

/*
	fprintf(DEBUG,"Letti i parametri!\n");
	fflush(DEBUG);
*/

	if (!userid)
	{
		err("authpgsql: PGSQL_USERNAME not set in "
			AUTHPGSQLRC ".");
		return (-1);
	}

	if (!database)
	{
		err("authpgsql: PGSQL_DATABASE not set in "
			AUTHPGSQLRC ".");
		return (-1);
	}

/*
	fprintf(DEBUG,"Connecting to db:%s:%s\%s user=%s pass=%s\n",server,server_port,database,userid,password);
	fflush(DEBUG);
*/
	pgconn = PQsetdbLogin(server, server_port, server_opt, NULL , database,userid,password);

	if (PQstatus(pgconn) == CONNECTION_BAD)
    	{
       		err("Connection to server '%s' userid '%s' database '%s' failed.",
       			server ? server : "<null>",
       			userid ? userid : "<null>",
       			database);
        	err("%s", PQerrorMessage(pgconn));
		pgconn=0;
		return -1;
    	}
/*
	fprintf(DEBUG,"Connected!\n");
	fflush(DEBUG);
*/

	return 0;

}

void auth_pgsql_cleanup()
{
	if (pgconn)
	{
		PQfinish(pgconn);
		pgconn=0;
	}
}

static struct authpgsqluserinfo ui={0, 0, 0, 0, 0, 0, 0, 0};

static void append_username(char *p, const char *username,
			    const char *defdomain)
{
	for (strcpy(p, username); *p; p++)
		if (*p == '\'' || *p == '"' || *p == '\\' ||
		    (int)(unsigned char)*p < ' ')
			*p=' ';	/* No funny business */
	if (strchr(username, '@') == 0 && defdomain && *defdomain)
		strcat(strcpy(p, "@"), defdomain);
}

/* tom@minnesota.com */
static struct var_data *get_variable (const char *begin, size_t len,
  		  	                   struct var_data *vdt)
{
struct var_data *vdp;

	if (!begin || !vdt) /* should never happend */
	{
		err("authpgsql: critical error while "
				 "parsing substitution variable");
		return NULL;
	}
	if (len < 1)
	{
		err("authpgsql: unknown empty substitution "
				 "variable - aborting");
		return NULL;
	}
	if (len > MAX_SUBSTITUTION_LEN)
	{
		err("authpgsql: variable name too long "
				 "while parsing substitution. "
				 "name begins with "
				 SV_BEGIN_MARK
				 "%.*s...", MAX_SUBSTITUTION_LEN, begin);
		return NULL;
	}
	
	for (vdp=vdt; vdp->name; vdp++)
		if (vdp->size == len+1 &&
		    !strncmp(begin, vdp->name, len))
		{
			if (!vdp->value)
				vdp->value = "";
			if (!vdp->value_length)		/* length cache */
				vdp->value_length = strlen (vdp->value);
			return vdp;
		}
	
	err("authpgsql: unknown substitution variable "
			 SV_BEGIN_MARK
			 "%.*s"
			 SV_END_MARK
			 , (int)len, begin);
	
	return NULL;
}

/* tom@minnesota.com */
static int ParsePlugin_counter (const char *p, size_t length, void *vp)
{
	if (!p || !vp || length < 0)
	{
		err("authpgsql: bad arguments while counting "
				 "query string");
		return -1;
	}
	
	*((size_t *)vp) += length;
   
	return 0;
}

/* tom@minnesota.com */
static int ParsePlugin_builder (const char *p, size_t length, void *vp)
{
char	**strptr = (char **) vp;

	if (!p || !vp || length < 0)
	{
		err("authpgsql: bad arguments while building "
				 "query string");
		return -1;
	}
	
	if (!length) return 0;
	memcpy ((void *) *strptr, (void *) p, length);
	*strptr += length;
	
	return 0;
}

/* tom@minnesota.com */
static int parse_core  (const char *source, struct var_data *vdt,
			parsefunc outfn, void *result)
{
size_t	v_size		= 0,
	t_size		= 0;
const char	*p, *q, *e,
		*v_begin, *v_end,
		*t_begin, *t_end;
struct var_data	*v_ptr;

	if (!source)
		source = "";
	if (!result)
	{
		err("authpgsql: no memory allocated for result "
				 "while parser core was invoked");
		return -1;
	}
	if (!vdt)
	{
		err("authpgsql: no substitution table found "
				 "while parser core was invoked");
		return -1;
	}
	
	q = source;
	while ( (p=strstr(q, SV_BEGIN_MARK)) )
	{
		e = strstr (p, SV_END_MARK);
		if (!e)
		{
			err("authpgsql: syntax error in "
					 "substitution "
					 "- no closing symbol found! "
					 "bad variable begins with:"
					 "%.*s...", MAX_SUBSTITUTION_LEN, p);
			return -1;
		}
		
		/*
		 **
		 **	     __________sometext$(variable_name)_________
		 **	 	       |      |  |    	     |	
		 **	        t_begin' t_end'  `v_begin    `v_end
		 **
		  */

		v_begin	= p+SV_BEGIN_LEN; /* variable field ptr		    */
		v_end	= e-SV_END_LEN;	  /* variable field last character  */
		v_size	= v_end-v_begin+1;/* variable field length	    */
		
		t_begin	= q;		  /* text field ptr		    */
		t_end	= p-1;		  /* text field last character	    */
		t_size	= t_end-t_begin+1;/* text field length		    */

		/* work on text */
		if ( (outfn (t_begin, t_size, result)) == -1 )
			return -1;
		
		/* work on variable */
		v_ptr = get_variable (v_begin, v_size, vdt);
		if (!v_ptr) return -1;
		
		if ( (outfn (v_ptr->value, v_ptr->value_length, result)) == -1 )
			return -1;
		
		q = e + 1;
	}

	/* work on last part of text if any */
	if (*q != '\0')
		if ( (outfn (q, strlen(q), result)) == -1 )
			return -1;

	return 0;
}

/* tom@minnesota.com */
static char *parse_string (const char *source, struct var_data *vdt)
{
struct var_data *vdp	= NULL;
char	*output_buf	= NULL,
	*pass_buf	= NULL;
size_t	buf_size	= 2;

	if (source == NULL || *source == '\0' || 
	    vdt == NULL    || vdt[0].name == NULL)
	{
		err("authpgsql: source clause is empty "
				 "- this is critical error");
		return NULL;
	}

	/* zero var_data length cache - important! */
	for (vdp=vdt; vdp->name; vdp++)
		vdp->value_length = 0;


	/* phase 1 - count and validate string */
	if ( (parse_core (source, vdt, &ParsePlugin_counter, &buf_size)) != 0)
		return NULL;

	/* phase 2 - allocate memory */
	output_buf = malloc (buf_size);
	if (!output_buf)
	{
		perror ("malloc");
		return NULL;
	}
	pass_buf = output_buf;

	/* phase 3 - build the output string */
	if ( (parse_core (source, vdt, &ParsePlugin_builder, &pass_buf)) != 0)
	{
		free (output_buf);
		return NULL;
	}	
	*pass_buf = '\0';
	
	return output_buf;
}

/* tom@minnesota.com */
static const char *get_localpart (const char *username)
{
size_t		lbuf	= 0;
const char	*l_end, *p;
char		*q;
static char	localpart_buf[130];
	
	if (!username || *username == '\0')	return NULL;
	
	p = strchr(username,'@');
	if (p)
	{
		if ((p-username) > 128)
			return NULL;
		l_end = p;
	}
	else
	{
		if ((lbuf = strlen(username)) > 128)
			return NULL;
		l_end = username + lbuf;
	}

	p=username;
	q=localpart_buf;
	
	while (*p && p != l_end)
		if (*p == '\"' || *p == '\\' ||
		    *p == '\'' || (int)(unsigned char)*p < ' ')
			p++;
		else
			*q++ = *p++;

	*q = '\0';
	return localpart_buf;
}

/* tom@minnesota.com */
static const char *get_domain (const char *username, const char *defdomain)
{
static char	domain_buf[260];
const char	*p;
char		*q;
	
	if (!username || *username == '\0')	return NULL;
	p = strchr(username,'@');
	
	if (!p || *(p+1) == '\0')
	{
		if (defdomain && *defdomain)
			return defdomain;
		else
			return NULL;
	}

	p++;
	if ((strlen(p)) > 256)
		return NULL;
	
	q = domain_buf;
	while (*p)
		if (*p == '\"' || *p == '\\' ||
	    	    *p == '\'' || (int)(unsigned char)*p < ' ')
			p++;
		else
			*q++ = *p++;

	*q = '\0';
	return domain_buf;
}

/* tom@minnesota.com */

static const char *validate_password (const char *password)
{
static char pass_buf[2][540]; /* Use two buffers, see parse_chpass_clause */
static int next_pass=0;
const char	*p;
char		*q, *endq;
	
	if (!password || *password == '\0' || (strlen(password)) > 256)
		return NULL;
	
	next_pass= 1-next_pass;

	p = password;
	q = pass_buf[next_pass];
	endq = q + sizeof pass_buf[next_pass];
	
	while (*p && q < endq)
	{
		if (*p == '\"' || *p == '\\' || *p == '\'')
			*q++ = '\\';
		*q++ = *p++;
	}
	
	if (q >= endq)
		return NULL;
	
	*q = '\0';
	return pass_buf[next_pass];
}

/* tom@minnesota.com */
static char *parse_select_clause (const char *clause, const char *username,
				  const char *defdomain,
				  const char *service)
{
static struct var_data vd[]={
	    {"local_part",	NULL,	sizeof("local_part"),	0},
	    {"domain",		NULL,	sizeof("domain"),	0},
	    {"service",		NULL,	sizeof("service"),	0},
	    {NULL,		NULL,	0,			0}};

	if (clause == NULL || *clause == '\0' ||
	    !username || *username == '\0')
		return NULL;
	
	vd[0].value	= get_localpart (username);
	vd[1].value	= get_domain (username, defdomain);
	if (!vd[0].value || !vd[1].value)
		return NULL;
	vd[2].value	= service;

	return (parse_string (clause, vd));
}

/* tom@minnesota.com */
static char *parse_chpass_clause (const char *clause, const char *username,
				  const char *defdomain, const char *newpass,
				  const char *newpass_crypt)
{
static struct var_data vd[]={
	    {"local_part",	NULL,	sizeof("local_part"),		0},
	    {"domain",		NULL,	sizeof("domain"),		0},
	    {"newpass",		NULL, 	sizeof("newpass"),		0},
	    {"newpass_crypt",	NULL,	sizeof("newpass_crypt"),	0},
	    {NULL,		NULL,	0,				0}};

	if (clause == NULL || *clause == '\0'		||
	    !username || *username == '\0'		||
	    !newpass || *newpass == '\0'		||
	    !newpass_crypt || *newpass_crypt == '\0')	return NULL;

	vd[0].value	= get_localpart (username);
	vd[1].value	= get_domain (username, defdomain);
	vd[2].value	= validate_password (newpass);
	vd[3].value	= validate_password (newpass_crypt);
	
	if (!vd[0].value || !vd[1].value ||
	    !vd[2].value || !vd[3].value)	return NULL;

	return (parse_string (clause, vd));
}

static void initui()
{

	if (ui.username)
		free(ui.username);
	if (ui.cryptpw)
		free(ui.cryptpw);
	if (ui.clearpw)
		free(ui.clearpw);
	if (ui.home)
		free(ui.home);
	if (ui.maildir)
		free(ui.maildir);
	if (ui.quota)
		free(ui.quota);
	if (ui.fullname)
		free(ui.fullname);
	if (ui.options)
		free(ui.options);
	memset(&ui, 0, sizeof(ui));
}

struct authpgsqluserinfo *auth_pgsql_getuserinfo(const char *username,
						 const char *service)
{
const char *user_table=NULL;
const char *defdomain;
char	*querybuf, *p;

const char *crypt_field=NULL,
	*clear_field=NULL,
       	*maildir_field=NULL,
       	*home_field=NULL,
	*name_field=NULL,
	*login_field=NULL,
       	*uid_field=NULL,
       	*gid_field=NULL,
       	*quota_field=NULL,
	*options_field=NULL,
       	*where_clause=NULL,
	*select_clause=NULL; /* tom@minnesota.com */

static const char query[]=
	"SELECT %s, %s, %s, %s, %s, %s, %s, %s, %s, %s FROM %s WHERE %s = '";

	if (do_connect())	return (0);

	initui();

/*
	fprintf(DEBUG,"1Leggo parametri\n");
	fflush(DEBUG);
*/
	select_clause=read_env("PGSQL_SELECT_CLAUSE");
	defdomain=read_env("DEFAULT_DOMAIN");

	if (!select_clause) /* tom@minnesota.com */
	{
		user_table=read_env("PGSQL_USER_TABLE");

		if (!user_table)
		{
			err("authpgsql: PGSQL_USER_TABLE not set in "
				AUTHPGSQLRC ".");
			return (0);
		}

		crypt_field=read_env("PGSQL_CRYPT_PWFIELD");
		clear_field=read_env("PGSQL_CLEAR_PWFIELD");
		name_field=read_env("PGSQL_NAME_FIELD");
/*
	fprintf(DEBUG,"2Leggo parametri\n");
	fflush(DEBUG);
*/

		if (!crypt_field && !clear_field)
		{
			err("authpgsql: PGSQL_CRYPT_PWFIELD and "
				"PGSQL_CLEAR_PWFIELD not set in " AUTHPGSQLRC ".");
			return (0);
		}
		if (!crypt_field) crypt_field="''";
		if (!clear_field) clear_field="''";
		if (!name_field) name_field="''";

		uid_field = read_env("PGSQL_UID_FIELD");
		if (!uid_field) uid_field = "uid";

		gid_field = read_env("PGSQL_GID_FIELD");
		if (!gid_field) gid_field = "gid";

		login_field = read_env("PGSQL_LOGIN_FIELD");
		if (!login_field) login_field = "id";

		home_field = read_env("PGSQL_HOME_FIELD");
		if (!home_field) home_field = "home";

		maildir_field=read_env(service && strcmp(service, "courier")==0
				       ? "PGSQL_DEFAULTDELIVERY"
				       : "PGSQL_MAILDIR_FIELD");
		if (!maildir_field) maildir_field="''";

		quota_field=read_env("PGSQL_QUOTA_FIELD");
		if (!quota_field) quota_field="''"; 

		options_field=read_env("PGSQL_AUXOPTIONS_FIELD");
		if (!options_field) options_field="''"; 

		where_clause=read_env("PGSQL_WHERE_CLAUSE");
		if (!where_clause) where_clause = "";
	}
	
	if (!defdomain)	defdomain="";

	if (!select_clause) /* tom@minnesota.com */
	{
		querybuf=malloc(sizeof(query) + 100 + strlen(user_table)
				+ strlen(defdomain) + strlen(crypt_field)
				+ strlen(clear_field) + strlen(maildir_field)
				+ strlen(uid_field) + strlen(gid_field)
				+ 2 * strlen(login_field)
				+ strlen(home_field) + strlen(quota_field)
				+ strlen(options_field) + strlen(where_clause)
				+ strlen(name_field)
				+ strlen(username)
				+ strlen(defdomain ? defdomain:""));

		if (!querybuf)
		{
			perror("malloc");
			return (0);
		}

		sprintf(querybuf, query, login_field, crypt_field, clear_field,
			uid_field, gid_field, home_field, maildir_field,
			quota_field,
			name_field,
			options_field,
			user_table, login_field);
		p=querybuf+strlen(querybuf);

		append_username(p, username, defdomain);
		strcat(p, "'");
		
		if (strcmp(where_clause, "")) {
			strcat(p, " AND (");
			strcat(p, where_clause);
			strcat(p, ")");
		}
	}
	else
	{
		/* tom@minnesota.com */
		querybuf=parse_select_clause (select_clause, username,
					      defdomain, service);
		if (!querybuf) return 0;
	}

	dprintf("SQL query: %s", querybuf);
	pgresult = PQexec(pgconn, querybuf);
    	if (!pgresult || PQresultStatus(pgresult) != PGRES_TUPLES_OK)
    	{
		dprintf("PQexec failed, reconnecting");
        	if (pgresult) PQclear(pgresult);
	
		/* <o.blasnik@nextra.de> */

		auth_pgsql_cleanup();

		if (do_connect())
		{
			free(querybuf);
			return (0);
		}

		pgresult = PQexec(pgconn, querybuf);
    		if (!pgresult || PQresultStatus(pgresult) != PGRES_TUPLES_OK)
		{
			dprintf("PQexec failed second time, giving up");
        		if (pgresult) PQclear(pgresult);
			free(querybuf);
			auth_pgsql_cleanup();
			/* Server went down, that's OK,
			** try again next time.
			*/
			return (0);
		}
	}
	free(querybuf);

		if (PQntuples(pgresult)>0)
		{
		char *t, *endp;
		int	num_fields = PQnfields(pgresult);

			if (num_fields < 6)
			{
				dprintf("incomplete row, only %d fields returned",
					num_fields);
				PQclear(pgresult);
				return 0;
			}

			t=PQgetvalue(pgresult,0,0);
			if (t && t[0]) ui.username=strdup(t);
			t=PQgetvalue(pgresult,0,1);
			if (t && t[0]) ui.cryptpw=strdup(t);
			t=PQgetvalue(pgresult,0,2);
			if (t && t[0]) ui.clearpw=strdup(t);
			t=PQgetvalue(pgresult,0,3);
			if (!t || !t[0] ||
			   (ui.uid=strtol(t, &endp, 10), endp[0] != '\0'))
			{
				dprintf("invalid value for uid: '%s'",
					t ? t : "<null>");
				PQclear(pgresult);
				return 0;
			}
			t=PQgetvalue(pgresult,0,4);
			if (!t || !t[0] ||
			   (ui.gid=strtol(t, &endp, 10), endp[0] != '\0'))
			{
				dprintf("invalid value for gid: '%s'",
					t ? t : "<null>");
				PQclear(pgresult);
				return 0;
			}
			t=PQgetvalue(pgresult,0,5);
			if (t && t[0])
				ui.home=strdup(t);
			else
			{
				dprintf("required value for 'home' (column 6) is missing");
				PQclear(pgresult);
				return 0;
			}
			t=num_fields > 6 ? PQgetvalue(pgresult,0,6) : 0;
			if (t && t[0]) ui.maildir=strdup(t);
			t=num_fields > 7 ? PQgetvalue(pgresult,0,7) : 0;
			if (t && t[0]) ui.quota=strdup(t);
			t=num_fields > 8 ? PQgetvalue(pgresult,0,8) : 0;
			if (t && t[0]) ui.fullname=strdup(t);
			t=num_fields > 9 ? PQgetvalue(pgresult,0,9) : 0;
			if (t && t[0]) ui.options=strdup(t);
		}
		else
		{
			dprintf("zero rows returned");
			PQclear(pgresult);
			return (&ui);
		}
        	PQclear(pgresult);

	return (&ui);
}

int auth_pgsql_setpass(const char *user, const char *pass)
{
	char *newpass_crypt;
	const char *newpass_crypt_ptr;
	const char *p;
	int l;
	char *sql_buf;
	const char *comma;
	int rc=0;

	const char *clear_field=NULL;
	const char *crypt_field=NULL;
	const char *defdomain=NULL;
	const char *where_clause=NULL;
	const char *user_table=NULL;
	const char *login_field=NULL;
	const char *chpass_clause=NULL; /* tom@minnesota.com */

	if (!pgconn)
		return (-1);


	if (!(newpass_crypt=authcryptpasswd(pass, "{crypt}")))
		return (-1);

	if (!(newpass_crypt_ptr=strchr(newpass_crypt, '}')))
	{
		free(newpass_crypt);	/* WTF???? */
		return (-1);
	}
	++newpass_crypt_ptr;

	for (l=0, p=pass; *p; p++)
	{
		if ((int)(unsigned char)*p < ' ')
		{
			free(newpass_crypt);
			return (-1);
		}
		if (*p == '"' || *p == '\\')
			++l;
		++l;
	}

	/* tom@minnesota.com */
	chpass_clause=read_env("PGSQL_CHPASS_CLAUSE");
	defdomain=read_env("DEFAULT_DOMAIN");
	user_table=read_env("PGSQL_USER_TABLE");
	if (!chpass_clause)
	{
		login_field = read_env("PGSQL_LOGIN_FIELD");
		if (!login_field) login_field = "id";
		crypt_field=read_env("PGSQL_CRYPT_PWFIELD");
		clear_field=read_env("PGSQL_CLEAR_PWFIELD");
		where_clause=read_env("PGSQL_WHERE_CLAUSE");
		sql_buf=malloc(strlen(crypt_field ? crypt_field:"")
			       + strlen(clear_field ? clear_field:"")
			       + strlen(defdomain ? defdomain:"")
			       + strlen(login_field) + l + strlen(newpass_crypt)
			       + strlen(user_table)
			       + strlen(where_clause ? where_clause:"")
			       + 200);
	}
	else
	{
		sql_buf=parse_chpass_clause(chpass_clause,
					    user,
					    defdomain,
					    pass,
					    newpass_crypt_ptr);
	}

	if (!sql_buf)
	{
		free(newpass_crypt);
		return (-1);
	}

	if (!chpass_clause) /* tom@minnesota.com */
	{
		sprintf(sql_buf, "UPDATE %s SET", user_table);

		comma="";

		if (clear_field && *clear_field)
		{
			char *q;

			strcat(strcat(strcat(sql_buf, " "), clear_field),
			       "='");

			q=sql_buf+strlen(sql_buf);
			while (*pass)
			{
				if (*pass == '"' || *pass == '\\')
					*q++= '\\';
				*q++ = *pass++;
			}
			strcpy(q, "'");
			comma=", ";
		}

		if (crypt_field && *crypt_field)
		{
			strcat(strcat(strcat(strcat(strcat(strcat(sql_buf, comma),
							   " "),
						    crypt_field),
					     "='"),
				      newpass_crypt_ptr),
			       "'");
		}
		free(newpass_crypt);

		strcat(strcat(strcat(sql_buf, " WHERE "),
			      login_field),
		       "='");

		append_username(sql_buf+strlen(sql_buf), user, defdomain);

		strcat(sql_buf, "'");

		if (where_clause && *where_clause)
		{
			strcat(sql_buf, " AND (");
			strcat(sql_buf, where_clause);
			strcat(sql_buf, ")");
		}

	} /* end of: if (!chpass_clause) */

	if (auth_debug_login_level >= 2)
		dprintf("setpass SQL: %s", sql_buf);
	pgresult=PQexec (pgconn, sql_buf);
	if (!pgresult || PQresultStatus(pgresult) != PGRES_COMMAND_OK)
	{
		dprintf("setpass SQL failed");
		rc= -1;
		auth_pgsql_cleanup();
	}
	PQclear(pgresult);
	free(sql_buf);
	return (rc);
}

void auth_pgsql_enumerate( void(*cb_func)(const char *name,
					  uid_t uid,
					  gid_t gid,
					  const char *homedir,
					  const char *maildir,
					  void *void_arg),
			   void *void_arg)
{
	const char *user_table=NULL;
	const char *defdomain;
	char	*querybuf, *p;

	const char*maildir_field=NULL,
		*home_field=NULL,
		*login_field=NULL,
		*uid_field=NULL,
		*gid_field=NULL,
		*where_clause=NULL,
		*select_clause=NULL;

static const char query[]=
	"SELECT %s, %s, %s, %s, %s FROM %s WHERE 1=1";
int i,n;

	if (do_connect())	return;

	initui();

/*
	fprintf(DEBUG,"1Leggo parametri\n");
	fflush(DEBUG);
*/
	select_clause=read_env("PGSQL_ENUMERATE_CLAUSE");
	defdomain=read_env("DEFAULT_DOMAIN");

	if (!select_clause) /* tom@minnesota.com */
	{
		user_table=read_env("PGSQL_USER_TABLE");

		if (!user_table)
		{
			err("authpgsql: PGSQL_USER_TABLE not set in "
			       AUTHPGSQLRC ".");
			return;
		}

		uid_field = read_env("PGSQL_UID_FIELD");
		if (!uid_field) uid_field = "uid";

		gid_field = read_env("PGSQL_GID_FIELD");
		if (!gid_field) gid_field = "gid";

		login_field = read_env("PGSQL_LOGIN_FIELD");
		if (!login_field) login_field = "id";

		home_field = read_env("PGSQL_HOME_FIELD");
		if (!home_field) home_field = "home";

		maildir_field=read_env("PGSQL_MAILDIR_FIELD");
		if (!maildir_field) maildir_field="''";

		where_clause=read_env("PGSQL_WHERE_CLAUSE");
		if (!where_clause) where_clause = "";
	}
	
	if (!defdomain)	defdomain="";

	if (!select_clause) /* tom@minnesota.com */
	{
		querybuf=malloc(sizeof(query) + 100 + strlen(user_table)
				+ strlen(defdomain)
				+ strlen(maildir_field)
				+ strlen(uid_field) + strlen(gid_field)
				+ 2 * strlen(login_field)
				+ strlen(home_field) + strlen(where_clause)
				+ strlen(defdomain ? defdomain:""));

		if (!querybuf)
		{
			perror("malloc");
			return;
		}

		sprintf(querybuf, query, login_field, 
			uid_field, gid_field, home_field, maildir_field,
			user_table);
		p=querybuf+strlen(querybuf);
		
		if (strcmp(where_clause, "")) {
			strcat(p, " AND (");
			strcat(p, where_clause);
			strcat(p, ")");
		}
	}
	else
	{
		/* tom@minnesota.com */
		querybuf=parse_select_clause (select_clause, "*",
					      defdomain, "enumerate");
		if (!querybuf) return;
	}

/*
	fprintf(DEBUG,"Eseguo la query:\n%s\n",querybuf);
	fflush(DEBUG);
*/
	
	pgresult = PQexec(pgconn, querybuf);
    	if (!pgresult || PQresultStatus(pgresult) != PGRES_TUPLES_OK)
    	{
/*
	fprintf(DEBUG,"Problema\n");
	fflush(DEBUG);
*/

        	PQclear(pgresult);
	
		/* <o.blasnik@nextra.de> */

		auth_pgsql_cleanup();

		if (do_connect())
		{
			free(querybuf);
			return;
		}

		pgresult = PQexec(pgconn, querybuf);
    		if (!pgresult || PQresultStatus(pgresult) != PGRES_TUPLES_OK)
		{
/*
	fprintf(DEBUG,"Problemadoppio\n");
	fflush(DEBUG);
*/
        		PQclear(pgresult);
			free(querybuf);
			auth_pgsql_cleanup();
			/* Server went down, that's OK,
			** try again next time.
			*/
			return;
		}
	}
	free(querybuf);

	for (n=PQntuples(pgresult), i=0; i<n; i++)
	{
		const char *username;
		uid_t uid;
		gid_t gid;
		const char *homedir;
		const char *maildir;

/*
	fprintf(DEBUG,"Leggo i risultati\n");
	fflush(DEBUG);
*/
		username=PQgetvalue(pgresult,i,0);
		uid=atol(PQgetvalue(pgresult,i,1));
		gid=atol(PQgetvalue(pgresult,i,2));
		homedir=PQgetvalue(pgresult,i,3);
		maildir=PQgetvalue(pgresult,i,4);

		if (!username || !*username || !homedir || !*homedir)
			continue;

		if (maildir && !*maildir)
			maildir=NULL;

		(*cb_func)(username, uid, gid, homedir,
			   maildir, void_arg);

	}
	PQclear(pgresult);
	(*cb_func)(NULL, 0, 0, NULL, NULL, void_arg);
/*
	fprintf(DEBUG,"Mail dir:%s\n",ui.maildir);
	fflush(DEBUG);
*/
}
