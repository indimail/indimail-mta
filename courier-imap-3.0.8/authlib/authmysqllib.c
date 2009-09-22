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
#include	<mysql.h>
#include	<time.h>

#include	"authmysql.h"
#include	"authmysqlrc.h"
#include	"auth.h"
#include	"debug.h"

/* siefca@pld.org.pl */
#define		MAX_SUBSTITUTION_LEN	32
#define		SV_BEGIN_MARK		"$("
#define		SV_END_MARK		")"
#define		SV_BEGIN_LEN		((sizeof(SV_BEGIN_MARK))-1)
#define		SV_END_LEN		((sizeof(SV_END_MARK))-1)

static const char rcsid[]="$Id: authmysqllib.c,v 1.36 2004/07/24 03:36:52 mrsam Exp $";

/* siefca@pld.org.pl */
struct var_data {			
	const char *name;
	const char *value;
	const size_t size;
	size_t value_length;
	} ;

/* siefca@pld.org.pl */
typedef int (*parsefunc)(const char *, size_t, void *);

static const char *read_env(const char *env)
{
static char *mysqlauth=0;
static size_t mysqlauth_size=0;
size_t	i;
char	*p=0;
int	l=strlen(env);

	if (!mysqlauth)
	{
	FILE	*f=fopen(AUTHMYSQLRC, "r");
	struct	stat	buf;

		if (!f)	return (0);
		if (fstat(fileno(f), &buf) ||
			(mysqlauth=malloc(buf.st_size+2)) == 0)
		{
			fclose(f);
			return (0);
		}
		if (fread(mysqlauth, buf.st_size, 1, f) != 1)
		{
			free(mysqlauth);
			mysqlauth=0;
			fclose(f);
			return (0);
		}
		mysqlauth[mysqlauth_size=buf.st_size]=0;

		for (i=0; i<mysqlauth_size; i++)
			if (mysqlauth[i] == '\n')
			{	/* siefca@pld.org.pl */
				if (!i || mysqlauth[i-1] != '\\')
				{
					mysqlauth[i]='\0';
				}
				else
				{
					mysqlauth[i]=mysqlauth[i-1]= ' ';
				}
			}
		fclose(f);
	}

	for (i=0; i<mysqlauth_size; )
	{
		p=mysqlauth+i;
		if (memcmp(p, env, l) == 0 &&
			isspace((int)(unsigned char)p[l]))
		{
			p += l;
			while (*p && *p != '\n' &&
				isspace((int)(unsigned char)*p))
				++p;
			break;
		}

		while (i < mysqlauth_size)
			if (mysqlauth[i++] == 0)	break;
	}

	if (i < mysqlauth_size)
		return (p);
	return (0);
}

static MYSQL mysql_buf;

static MYSQL *mysql=0;

static int do_connect()
{
const	char *server;
const	char *userid;
const	char *password;
const	char *database;
const	char *server_socket=0;
unsigned int server_port=0;
unsigned int server_opt=0;
const	char *p;

/*
** Periodically detect dead connections.
*/
	if (mysql)
	{
		static time_t last_time=0;
		time_t t_check;

		time(&t_check);

		if (t_check < last_time)
			last_time=t_check;	/* System clock changed */

		if (t_check < last_time + 60)
			return (0);

		last_time=t_check;
			
		if (mysql_ping(mysql) == 0) return (0);

		dprintf("authmysqllib: mysql_ping failed, connection lost");
		mysql_close(mysql);
		mysql=0;
	}

	server=read_env("MYSQL_SERVER");
	userid=read_env("MYSQL_USERNAME");
	password=read_env("MYSQL_PASSWORD");
	database=read_env("MYSQL_DATABASE");

	server_socket=(char *) read_env("MYSQL_SOCKET");

	if ((p=read_env("MYSQL_PORT")) != 0)
	{
		server_port=(unsigned int) atoi(p);
	}

	if ((p=read_env("MYSQL_OPT")) != 0)
	{
		server_opt=(unsigned int) atol(p);
	}

	if (!server && !server_socket)
	{
		err("authmysql: MYSQL_SERVER nor MYSQL_SOCKET set in"
			AUTHMYSQLRC ".");
		return (-1);
	}

	if (!userid)
	{
		err("authmysql: MYSQL_USERNAME not set in "
			AUTHMYSQLRC ".");
		return (-1);
	}

	if (!database)
	{
		err("authmysql: MYSQL_DATABASE not set in "
			AUTHMYSQLRC ".");
		return (-1);
	}

#if MYSQL_VERSION_ID >= 32200
	mysql_init(&mysql_buf);
	mysql=mysql_real_connect(&mysql_buf, server, userid, password,
				 NULL,
				 server_port,
				 server_socket,
				 server_opt);
#else
	mysql=mysql_connect(&mysql_buf, server, userid, password);
#endif
	if (!mysql)
	{
		err("failed to connect to mysql server (server=%s, userid=%s)",
			server ? server : "<null>",
			userid ? userid : "<null>");
		return (-1);
	}

	if (mysql_select_db(mysql, database))
	{
		err("authmysql: mysql_select_db(%s) error: %s",
			database, mysql_error(mysql));
		mysql_close(mysql);
		mysql=0;
		return (-1);
	}
	return (0);
}

void auth_mysql_cleanup()
{
	if (mysql)
	{
		mysql_close(mysql);
		mysql=0;
	}
}

static struct authmysqluserinfo ui={0, 0, 0, 0, 0, 0, 0, 0};

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

/* siefca@pld.org.pl */
static struct var_data *get_variable (const char *begin, size_t len,
  		  	                   struct var_data *vdt)
{
struct var_data *vdp;

	if (!begin || !vdt) /* should never happend */
	{
		err("authmysql: critical error while "
				 "parsing substitution variable");
		return NULL;
	}
	if (len < 1)
	{
		err("authmysql: unknown empty substitution "
				 "variable - aborting");
		return NULL;
	}
	if (len > MAX_SUBSTITUTION_LEN)
	{
		err("authmysql: variable name too long "
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
	
	err("authmysql: unknown substitution variable "
			 SV_BEGIN_MARK
			 "%.*s"
			 SV_END_MARK
			 , (int)len, begin);
	
	return NULL;
}

/* siefca@pld.org.pl */
static int ParsePlugin_counter (const char *p, size_t length, void *vp)
{
	if (!p || !vp || length < 0)
	{
		err("authmysql: bad arguments while counting "
				 "query string");
		return -1;
	}
	
	*((size_t *)vp) += length;
   
	return 0;
}

/* siefca@pld.org.pl */
static int ParsePlugin_builder (const char *p, size_t length, void *vp)
{
char	**strptr = (char **) vp;

	if (!p || !vp || length < 0)
	{
		err("authmysql: bad arguments while building "
				 "query string");
		return -1;
	}
	
	if (!length) return 0;
	memcpy ((void *) *strptr, (void *) p, length);
	*strptr += length;
	
	return 0;
}

/* siefca@pld.org.pl */
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
		err("authmysql: no memory allocated for result "
				 "while parser core was invoked");
		return -1;
	}
	if (!vdt)
	{
		err("authmysql: no substitution table found "
				 "while parser core was invoked");
		return -1;
	}
	
	q = source;
	while ( (p=strstr(q, SV_BEGIN_MARK)) )
	{
		e = strstr (p, SV_END_MARK);
		if (!e)
		{
			err("authmysql: syntax error in "
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

/* siefca@pld.org.pl */
static char *parse_string (const char *source, struct var_data *vdt)
{
struct var_data *vdp	= NULL;
char	*output_buf	= NULL,
	*pass_buf	= NULL;
size_t	buf_size	= 2;

	if (source == NULL || *source == '\0' || 
	    vdt == NULL    || vdt[0].name == NULL)
	{
		err("authmysql: source clause is empty "
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

/* siefca@pld.org.pl */
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

/* siefca@pld.org.pl */
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

/* siefca@pld.org.pl */

static const char *validateMyPassword (const char *password)
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

/* siefca@pld.org.pl */
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
	vd[2].value     = service;

	return (parse_string (clause, vd));
}

/* siefca@pld.org.pl */
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
	vd[2].value	= validateMyPassword (newpass);
	vd[3].value	= validateMyPassword (newpass_crypt);
	
	if (!vd[0].value || !vd[1].value ||
	    !vd[2].value || !vd[3].value)	return NULL;

	return (parse_string (clause, vd));
}

struct authmysqluserinfo *auth_mysql_getuserinfo(const char *username,
						 const char *service)
{
const char *user_table	=NULL;
const char *defdomain	=NULL;
char	*querybuf, *p;
MYSQL_ROW	row;
MYSQL_RES	*result;
int		num_fields;
char		*endp;

const char  *crypt_field	=NULL, 
	*clear_field	=NULL,
	*maildir_field	=NULL,
	*home_field		=NULL,
	*name_field		=NULL,
	*login_field	=NULL,
	*uid_field		=NULL,
	*gid_field		=NULL,
	*quota_field	=NULL,
	*options_field	=NULL,
	*where_clause	=NULL,
	*select_clause	=NULL; /* siefca@pld.org.pl */

static const char query[]=
	"SELECT %s, %s, %s, %s, %s, %s, %s, %s, %s, %s FROM %s WHERE %s = \"";

	if (do_connect())	return (0);

	initui();

	select_clause=read_env("MYSQL_SELECT_CLAUSE");
	defdomain=read_env("DEFAULT_DOMAIN");	
	
	if (!select_clause) /* siefca@pld.org.pl */
	{
		user_table=read_env("MYSQL_USER_TABLE");

		if (!user_table)
		{
			err("authmysql: MYSQL_USER_TABLE not set in "
				AUTHMYSQLRC ".");
			return (0);
		}

		crypt_field=read_env("MYSQL_CRYPT_PWFIELD");
		clear_field=read_env("MYSQL_CLEAR_PWFIELD");
		name_field=read_env("MYSQL_NAME_FIELD");

		if (!crypt_field && !clear_field)
		{
			err("authmysql: MYSQL_CRYPT_PWFIELD and "
				"MYSQL_CLEAR_PWFIELD not set in " AUTHMYSQLRC ".");
			return (0);
		}
		if (!crypt_field) crypt_field="\"\"";
		if (!clear_field) clear_field="\"\"";
		if (!name_field) name_field="\"\"";

		uid_field = read_env("MYSQL_UID_FIELD");
		if (!uid_field) uid_field = "uid";

		gid_field = read_env("MYSQL_GID_FIELD");
		if (!gid_field) gid_field = "gid";

		login_field = read_env("MYSQL_LOGIN_FIELD");
		if (!login_field) login_field = "id";

		home_field = read_env("MYSQL_HOME_FIELD");
		if (!home_field) home_field = "home";

		maildir_field=read_env(service && strcmp(service, "courier")==0
				       ? "MYSQL_DEFAULTDELIVERY"
				       : "MYSQL_MAILDIR_FIELD");
		if (!maildir_field) maildir_field="\"\"";

		quota_field=read_env("MYSQL_QUOTA_FIELD");
		if (!quota_field) quota_field="\"\""; 

		options_field=read_env("MYSQL_AUXOPTIONS_FIELD");
		if (!options_field) options_field="\"\"";

		where_clause=read_env("MYSQL_WHERE_CLAUSE");
		if (!where_clause) where_clause = "";
	}
	
	if (!defdomain)	defdomain="";

	if (!select_clause) /* siefca@pld.org.pl */
	{
		querybuf=malloc(sizeof(query) + 100 + strlen(user_table) + strlen(defdomain)
			+ strlen(crypt_field) + strlen(clear_field) + strlen(maildir_field)
			+ strlen(uid_field) + strlen(gid_field) + 2 * strlen(login_field)
			+ strlen(home_field) + strlen(quota_field) + strlen(where_clause)
				+strlen(options_field)
			+ strlen(name_field) + strlen(username)
				+ strlen(defdomain ? defdomain:""));

		if (!querybuf)
		{
			perror("malloc");
			return (0);
		}

		sprintf(querybuf, query, login_field, crypt_field, clear_field, 
			uid_field, gid_field, home_field, maildir_field, quota_field,
			name_field, options_field, user_table, login_field);

		p=querybuf+strlen(querybuf);

		append_username(p, username, defdomain);
		strcat(p, "\"");
	
		if (strcmp(where_clause, "")) {
			strcat(p, " AND (");
			strcat(p, where_clause);
			strcat(p, ")");
		}
	}
	else
	{
		/* siefca@pld.org.pl */
		querybuf=parse_select_clause (select_clause, username,
					      defdomain, service);
		if (!querybuf) return 0;
	}

	dprintf("SQL query: %s", querybuf);
	if (mysql_query (mysql, querybuf))
	{
		/* <o.blasnik@nextra.de> */

		dprintf("mysql_query failed, reconnecting");
		auth_mysql_cleanup();

		if (do_connect())
		{
			free(querybuf);
			return (0);
		}

		if (mysql_query (mysql, querybuf))
		{
			dprintf("mysql_query failed second time, giving up");
			free(querybuf);
			auth_mysql_cleanup();
			/* Server went down, that's OK,
			** try again next time.
			*/
			return (0);
		}
	}
	free(querybuf);

	result = mysql_store_result (mysql);       
	if (result)
	{
		if (mysql_num_rows(result))
		{
			row = mysql_fetch_row (result);
			num_fields = mysql_num_fields (result);

			if (num_fields < 6)
			{
				dprintf("incomplete row, only %d fields returned",
					num_fields);
				mysql_free_result(result);
				return(0);
			}

			if (row[0] && row[0][0])
				ui.username=strdup(row[0]);
			if (row[1] && row[1][0])
				ui.cryptpw=strdup(row[1]);
			if (row[2] && row[2][0])
				ui.clearpw=strdup(row[2]);
			/* perhaps authmysql needs a glob_uid/glob_gid feature
			   like authldap? */
			if (!row[3] || !row[3][0] ||
			   (ui.uid=strtol(row[3], &endp, 10), endp[0] != '\0'))
			{
				dprintf("invalid value for uid: '%s'",
					row[3] ? row[3] : "<null>");
				mysql_free_result(result);
				return 0;
			}
			if (!row[4] || !row[4][0] ||
			   (ui.gid=strtol(row[4], &endp, 10), endp[0] != '\0'))
			{
				dprintf("invalid value for gid: '%s'",
					row[4] ? row[4] : "<null>");
				mysql_free_result(result);
				return 0;
			}
			if (row[5] && row[5][0])
				ui.home=strdup(row[5]);
			else
			{
				dprintf("required value for 'home' (column 6) is missing");
				mysql_free_result(result);
				return(0);
			}
			if (num_fields > 6 && row[6] && row[6][0])
				ui.maildir=strdup(row[6]);
			if (num_fields > 7 && row[7] && row[7][0])
				ui.quota=strdup(row[7]);
			if (num_fields > 8 && row[8] && row[8][0])
				ui.fullname=strdup(row[8]);
			if (num_fields > 9 && row[9] && row[9][0])
				ui.options=strdup(row[9]);
		}
		else
		{
			dprintf("zero rows returned");
			mysql_free_result(result);
			return (&ui); /* User not found */
		}
	}
	else
	{
		dprintf("mysql_store_result failed");
		return (0);
	}
	mysql_free_result(result);
	return (&ui);
}

int auth_mysql_setpass(const char *user, const char *pass)
{
	char *newpass_crypt;
	const char *newpass_crypt_ptr;
	const char *p;
	int l;
	char *sql_buf;
	const char *comma;
	int rc=0;

	const char  *clear_field	=NULL,
		    *crypt_field	=NULL,
		    *defdomain		=NULL,
		    *where_clause	=NULL,
		    *user_table		=NULL,
		    *login_field	=NULL,
		    *chpass_clause	=NULL; /* siefca@pld.org.pl */

	if (!mysql)
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

	/* siefca@pld.org.pl */
	chpass_clause=read_env("MYSQL_CHPASS_CLAUSE");
	defdomain=read_env("DEFAULT_DOMAIN");
	user_table=read_env("MYSQL_USER_TABLE");
	if (!chpass_clause)
	{
		login_field = read_env("MYSQL_LOGIN_FIELD");
		if (!login_field) login_field = "id";
		crypt_field=read_env("MYSQL_CRYPT_PWFIELD");
		clear_field=read_env("MYSQL_CLEAR_PWFIELD");
		where_clause=read_env("MYSQL_WHERE_CLAUSE");
		sql_buf=malloc(strlen(crypt_field ? crypt_field:"")
				+ strlen(clear_field ? clear_field:"")
				+ strlen(defdomain ? defdomain:"")
				+ strlen(login_field) + l + strlen(newpass_crypt_ptr)
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

	if (!chpass_clause) /*siefca@pld.org.pl */
	{
		sprintf(sql_buf, "UPDATE %s SET", user_table);

		comma="";

		if (clear_field && *clear_field)
		{
			char *q;

			strcat(strcat(strcat(sql_buf, " "), clear_field),
		    		"=\"");

			q=sql_buf+strlen(sql_buf);
			while (*pass)
			{
				if (*pass == '"' || *pass == '\\')
					*q++= '\\';
				*q++ = *pass++;
			}
			strcpy(q, "\"");
			comma=", ";
		}

		if (crypt_field && *crypt_field)
		{
			strcat(strcat(strcat(strcat(strcat(strcat(sql_buf, comma),
							   " "),
						    crypt_field),
					     "=\""),
			    	newpass_crypt_ptr),
		    	     "\"");
		}
		free(newpass_crypt);

		strcat(strcat(strcat(sql_buf, " WHERE "),
			      login_field),
	    	    "=\"");

		append_username(sql_buf+strlen(sql_buf), user, defdomain);

		strcat(sql_buf, "\"");

		if (where_clause && *where_clause)
		{
			strcat(sql_buf, " AND (");
			strcat(sql_buf, where_clause);
			strcat(sql_buf, ")");
		}
		
	} /* end of: if (!chpass_clause) */

	if (auth_debug_login_level >= 2)
		dprintf("setpass SQL: %s", sql_buf);
	if (mysql_query (mysql, sql_buf))
	{
		dprintf("setpass SQL failed");
		rc= -1;
		auth_mysql_cleanup();
	}
	free(sql_buf);
	return (rc);
}

void auth_mysql_enumerate( void(*cb_func)(const char *name,
					  uid_t uid,
					  gid_t gid,
					  const char *homedir,
					  const char *maildir,
					  void *void_arg),
			   void *void_arg)
{
	const char *user_table	=NULL;
	const char *defdomain	=NULL;
	char	*querybuf, *p;
	MYSQL_ROW	row;
	MYSQL_RES	*result;

	const char  *maildir_field	=NULL,
		*home_field		=NULL,
		*login_field	=NULL,
		*uid_field		=NULL,
		*gid_field		=NULL,
		*where_clause	=NULL,
		*select_clause	=NULL; /* siefca@pld.org.pl */

	static const char query[]=
		"SELECT %s, %s, %s, %s, %s FROM %s WHERE 1=1";

	if (do_connect())	return;

	initui();

	select_clause=read_env("MYSQL_ENUMERATE_CLAUSE");
	defdomain=read_env("DEFAULT_DOMAIN");	
	
	if (!select_clause)
	{
		user_table=read_env("MYSQL_USER_TABLE");

		if (!user_table)
		{
			err("authmysql: MYSQL_USER_TABLE not set in "
				AUTHMYSQLRC ".");
			return;
		}

		uid_field = read_env("MYSQL_UID_FIELD");
		if (!uid_field) uid_field = "uid";

		gid_field = read_env("MYSQL_GID_FIELD");
		if (!gid_field) gid_field = "gid";

		login_field = read_env("MYSQL_LOGIN_FIELD");
		if (!login_field) login_field = "id";

		home_field = read_env("MYSQL_HOME_FIELD");
		if (!home_field) home_field = "home";

		maildir_field=read_env("MYSQL_MAILDIR_FIELD");
		if (!maildir_field) maildir_field="\"\"";

		where_clause=read_env("MYSQL_WHERE_CLAUSE");
		if (!where_clause) where_clause = "";
	}
	
	if (!defdomain)	defdomain="";

	if (!select_clause) /* siefca@pld.org.pl */
	{
		querybuf=malloc(sizeof(query) + 100 + strlen(user_table) + strlen(defdomain)
				+ strlen(maildir_field)
				+ strlen(uid_field) + strlen(gid_field)
				+ strlen(login_field)
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
		/* siefca@pld.org.pl */
		querybuf=parse_select_clause (select_clause, "*",
					      defdomain, "enumerate");
		if (!querybuf) return;
	}

	if (mysql_query (mysql, querybuf))
	{
		/* <o.blasnik@nextra.de> */

		auth_mysql_cleanup();

		if (do_connect())
		{
			free(querybuf);
			return;
		}

		if (mysql_query (mysql, querybuf))
		{
			free(querybuf);
			auth_mysql_cleanup();
			/* Server went down, that's OK,
			** try again next time.
			*/
			return;
		}
	}
	free(querybuf);

	result = mysql_store_result (mysql);       
	if (result)
	{
		if (mysql_num_rows(result))
		{
			const char *username;
			uid_t uid;
			gid_t gid;
			const char *homedir;
			const char *maildir;

			while ((row = mysql_fetch_row (result)) != NULL)
			{
				if(!row[0] || !row[1] || !row[2] ||
				   !row[3])
				{
					continue;
				}

				username=row[0];

				uid=atol(row[1]);
				gid=atol(row[2]);
				homedir=row[3];
				maildir=row[4];

				if (maildir && !*maildir)
					maildir=NULL;

				(*cb_func)(username, uid, gid, homedir,
					   maildir, void_arg);
			}
			(*cb_func)(NULL, 0, 0, NULL, NULL, void_arg);
		}
		mysql_free_result(result);
	}
}
