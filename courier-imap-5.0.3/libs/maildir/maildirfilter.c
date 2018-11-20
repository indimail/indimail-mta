/*
** Copyright 2000-2010 Double Precision, Inc.
** See COPYING for distribution information.
*/

#include	"config.h"
#include	"maildirfilter.h"
#include	"maildirfiltertypelist.h"
#include	"maildirgetquota.h"
#include	"mailbot.h"

#include	"autoresponse.h"
#include	"numlib/numlib.h"
#include	"unicode/courier-unicode.h"
#include	<stdlib.h>
#include	<string.h>
#include	<stdio.h>
#include	<ctype.h>
#include	<errno.h>
#include	<sys/types.h>
#include	"maildirmisc.h"

#if HAVE_SYSEXITS_H
#include	<sysexits.h>
#else
#define	EX_SOFTWARE	70
#endif

#if	HAVE_PCRE_H
#include	<pcre.h>
#else
#if	HAVE_PCRE_PCRE_H
#include	<pcre/pcre.h>
#define HAVE_PCRE_H	1
#endif
#endif

#if	HAVE_SYS_STAT_H
#include	<sys/stat.h>
#endif
#if	HAVE_UNISTD_H
#include	<unistd.h>
#endif


struct maildirfilterrule *maildir_filter_appendrule(struct maildirfilter *r,
					const char *name,
					enum maildirfiltertype type,
					int flags,
					const char *header,
					const char *value,
					const char *folder,
					const char *fromhdr,
					const char *charset,
					int *errcode)
{
struct maildirfilterrule *p=malloc(sizeof(struct maildirfilterrule));

	*errcode=MF_ERR_INTERNAL;

	if (!p)	return (0);
	memset(p, 0, sizeof(*p));

	if ((p->prev=r->last) != 0)
		p->prev->next=p;
	else
		r->first=p;
	r->last=p;

	if (maildir_filter_ruleupdate(r, p, name, type, flags,
				      header, value, folder, fromhdr, charset,
				      errcode))
	{
		maildir_filter_ruledel(r, p);
		return (0);
	}
	return (p);
}

static int maildir_filter_ruleupdate_utf8(struct maildirfilter *r,
					  struct maildirfilterrule *p,
					  const char *name,
					  enum maildirfiltertype type,
					  int flags,
					  const char *header,
					  const char *value,
					  const char *folder,
					  const char *fromhdr,
					  int *errcode);

int maildir_filter_ruleupdate(struct maildirfilter *r,
			      struct maildirfilterrule *p,
			      const char *name,
			      enum maildirfiltertype type,
			      int flags,
			      const char *header,
			      const char *value,
			      const char *folder,
			      const char *fromhdr,
			      const char *charset,
			      int *errcode)
{
	char *name_utf8;
	char *header_utf8;
	char *value_utf8;
	int rc;

	name_utf8=unicode_convert_toutf8(name ? name:"", charset, NULL);

	if (!name_utf8)
	{
		*errcode=MF_ERR_BADRULENAME;
		return -1;
	}

	header_utf8=unicode_convert_toutf8(header ? header:"", charset, NULL);

	if (!header_utf8)
	{
		free(name_utf8);
		*errcode=MF_ERR_BADRULEHEADER;
		return -1;
	}

	value_utf8=unicode_convert_toutf8(value ? value:"", charset, NULL);

	if (!value_utf8)
	{
		free(name_utf8);
		free(header_utf8);
		*errcode=MF_ERR_BADRULEVALUE;
		return -1;
	}
	rc=maildir_filter_ruleupdate_utf8(r, p, name_utf8, type, flags,
					  header_utf8,
					  value_utf8,
					  folder,
					  fromhdr,
					  errcode);
	free(name_utf8);
	free(value_utf8);
	free(header_utf8);
	return rc;
}

static int maildir_filter_ruleupdate_utf8(struct maildirfilter *r,
					  struct maildirfilterrule *p,
					  const char *name,
					  enum maildirfiltertype type,
					  int flags,
					  const char *header,
					  const char *value,
					  const char *folder,
					  const char *fromhdr,
					  int *errcode)
{
	const char *c;
	struct maildirfilterrule *pom;

/*
** Before creating a new rule, validate all input.
*/

	*errcode=0;

	/* rule name: may not contain quotes or control characters. */
	*errcode=MF_ERR_BADRULENAME;
	if (!*name || strlen(name) > 200)
		return (-1);

	for (c=name; *c; c++)
		if ((unsigned char)*c < ' ' || *c == '\'' || *c == '"' ||
			*c == '`')
			return (-1);

	/* rule name: may not already exist */
	*errcode=MF_ERR_EXISTS;

	for (pom=r->first; pom->next; pom=pom->next) {
	    if (p!=pom && !strcmp(name, pom->rulename_utf8))
		return (-1);
	}

	/* rule type: we must know what it is */

	switch (type)	{
	case startswith:
	case endswith:
	case contains:
	case hasrecipient:
	case mimemultipart:
	case textplain:
	case islargerthan:
	case anymessage:
		break;
	default:
		*errcode=MF_ERR_BADRULETYPE;
		break;
	} ;

	/* header: */

	*errcode=MF_ERR_BADRULEHEADER;

	c=header;
	if (strlen(c) > 200)	return (-1);
	if (*c == 0)
	{
		switch (type)	{
		case hasrecipient:
		case islargerthan:
		case mimemultipart:
		case textplain:
		case anymessage:
			break;
		case contains:
		case startswith:
		case endswith:
			if (flags & MFR_BODY)
				break;
			/* FALLTHRU */
		default:
			/* required */

			return (-1);
		}
	}
	else for ( ; *c; c++)
	{
		/* no control characters */
		if ((unsigned char)*c <= ' ' || *c == MDIRSEP[0] ||
		    *c == '\'' ||
		    *c == '\\' || *c == '"' || *c == '`' || *c == '/')
			return (-1);
	}

	/* rule pattern */

	*errcode=MF_ERR_BADRULEVALUE;

	c=value;
	if (strlen(c) > 200)	return (-1);
	if (*c == 0)
	{
		switch (type)	{
		case mimemultipart:
		case textplain:
		case anymessage:
			break;
		default:
			/* required */

			return (-1);
		}
	}
	else if (!(flags & MFR_PLAINSTRING))
	{
		/*
		** Let PCRE decide if this is a valid pattern.
		**
		** One exception: the forward slash character, and some other
		** special characters, must always be escaped.
		*/

		while (*c)
		{
			if (*c == '/' || *c == '$' || *c == '!'
				|| *c == '`' || (int)(unsigned char)*c < ' '
				|| *c == '\'' || *c == '"') return (-1);
						/* must be escaped */

			if (type == islargerthan)
			{
				if (!isdigit((int)(unsigned char)*c))
					return (-1);
			}

			if (*c == '(')
			{
				if (type == hasrecipient)	return (-1);
				++c;
				if (*c == ')')	return (-1);
				continue;
			}
			if (*c == ')')
			{
				if (type == hasrecipient)	return (-1);
				++c;
				continue;
			}
			if (*c == '[')	/* This is a set */
			{
				if (type == hasrecipient)	return (-1);
				++c;
				for (;;)
				{
					if (*c == '\'' || *c == '"' ||
						*c == '`')
						return (-1); /* must be quoted*/
					if (*c == '\\')
						++c;
					if (!*c)	return (-1);
					if ((int)(unsigned char)*c < ' ')
						return (-1);
					++c;
					if (*c == ']')	break;
					if (*c != '-')	continue;
					++c;

					if (*c == '\'' || *c == '"' ||
						*c == '`')
						return (-1); /* must be quoted*/
					if (*c == '\\')
						++c;
					if ((int)(unsigned char)*c < ' ')
						return (-1);
					if (!*c)	return (-1);
					++c;
					if (*c == ']')	break;
				}
				++c;
				continue;
			}

			if (*c == '\\')
			{
				if (type == hasrecipient)	return (-1);
				++c;
			}
			if (!*c)	return (-1);
			++c;
		}

#if HAVE_PCRE_H
		switch (type) {
		case contains:
		case startswith:
		case endswith:
			{
				const char *errptr;
				int errindex;

				pcre *p=pcre_compile(value, PCRE_UTF8,
						     &errptr,
						     &errindex,
						     0);


				if (p == NULL)
					return -1;
				pcre_free(p);
			}
			break;
		default:
			break;
		}
#endif
	}

	/* validate FROM header */

	*errcode=MF_ERR_BADFROMHDR;

	while (fromhdr && *fromhdr && isspace((int)(unsigned char)*fromhdr))
		++fromhdr;

	for (c=fromhdr; *c; c++)
		if ((int)(unsigned char)*c < ' ')
			return (-1);

	*errcode=MF_ERR_BADRULEFOLDER;

	/* validate name of destination folder */

	c=folder;
	if (!c)	return (-1);
	if (strlen(c) > 200)	return (-1);

	if (*c == '*' || *c == '!')
	{
		/* Forward, or bounce with an error */

		++c;
		for ( ; *c; c++)
		{
			if (strchr("'\"$\\`;(){}#&<>~", *c) ||
				(unsigned char)*c < ' ')
				return (-1);
		}
	}
	else if (*c == '+')	/* Autorespond */
	{
		struct maildir_filter_autoresp_info ai;

		if (maildir_filter_autoresp_info_init_str(&ai, c+1))
			return (-1);

		maildir_filter_autoresp_info_free(&ai);
	}
	else if (strcmp(c, "exit") == 0)	/* Purge */
	{
	}
	else
	{
		char *s;

		if (strcmp(c, INBOX) &&
		    strncmp(c, INBOX ".", sizeof(INBOX)))
			return -1;

		s=maildir_name2dir(".", c);

		if (!s)
			return -1;
		free(s);
	}

	/* OK, we're good */

	*errcode=MF_ERR_INTERNAL;

	if (p->rulename_utf8)	free(p->rulename_utf8);
	if ((p->rulename_utf8=strdup(name)) == 0)	return (-1);
	p->type=type;
	if (p->fieldname_utf8)	free(p->fieldname_utf8);
	if ((p->fieldname_utf8=strdup(header ? header:"")) == 0)	return (-1);
	if (p->fieldvalue_utf8)	free(p->fieldvalue_utf8);
	if ((p->fieldvalue_utf8=strdup(value ? value:"")) == 0)	return (-1);
	if (p->tofolder)	free(p->tofolder);
	if ((p->tofolder=malloc(strlen(folder)+1)) == 0)	return (-1);
	strcpy(p->tofolder, folder);

	if (p->fromhdr)		free(p->fromhdr);
	if ((p->fromhdr=strdup(fromhdr ? fromhdr:"")) == NULL)
		return (-1);

	p->flags=flags;
	return (0);
}

void maildir_filter_ruledel(struct maildirfilter *r, struct maildirfilterrule *p)
{
	if (p->prev)	p->prev->next=p->next;
	else		r->first=p->next;

	if (p->next)	p->next->prev=p->prev;
	else		r->last=p->prev;

	if (p->rulename_utf8)	free(p->rulename_utf8);
	if (p->fieldname_utf8)	free(p->fieldname_utf8);
	if (p->fieldvalue_utf8)	free(p->fieldvalue_utf8);
	if (p->tofolder)	free(p->tofolder);
	if (p->fromhdr)		free(p->fromhdr);
	free(p);
}

void maildir_filter_ruleup(struct maildirfilter *r, struct maildirfilterrule *p)
{
struct maildirfilterrule *q;

	q=p->prev;
	if (!q)	return;
	q->next=p->next;
	if (p->next)	p->next->prev=q;
	else		r->last=q;

	if ((p->prev=q->prev) != 0)	p->prev->next=p;
	else	r->first=p;

	p->next=q;
	q->prev=p;
}

void maildir_filter_ruledown(struct maildirfilter *r, struct maildirfilterrule *p)
{
struct maildirfilterrule *q;

	q=p->next;
	if (!q)	return;
	q->prev=p->prev;
	if (q->prev)	q->prev->next=q;
	else		r->first=q;

	if ((p->next=q->next) != 0)	p->next->prev=p;
	else	r->last=p;

	p->prev=q;
	q->next=p;
}

static void print_pattern(FILE *f, int flags, const char *v)
{
	if (!(flags & MFR_PLAINSTRING))
	{
		fprintf(f, "%s%s",
			*v && isspace((int)(unsigned char)*v) ? "\\":"", v);
		return;
	}

	while (*v)
	{
		if (((int)(unsigned char)*v) <= 0x80 &&
		    !isalnum((int)(unsigned char)*v))
			putc('\\', f);
		putc((int)(unsigned char)*v, f);
		++v;
	}
}

int maildir_filter_saverules(struct maildirfilter *r, const char *filename,
			     const char *maildirpath, const char *fromaddr)
{
FILE	*f=fopen(filename, "w");
struct maildirfilterrule *p;

	if (!f)	return (-1);

	fprintf(f,	"#MFMAILDROP=2\n"
			"#\n"
			"# DO NOT EDIT THIS FILE.  This is an automatically"
						" generated filter.\n"
			"\n");

	for (fprintf(f, "FROM='"); *fromaddr; fromaddr++)
	{
		if (*fromaddr == '\'' || *fromaddr == '\\')
			putc('\\', f);
		putc(*fromaddr, f);
	}
	fprintf(f, "\'\n");

	for (p=r->first; p; p=p->next)
	{
	const char *fieldname=p->fieldname_utf8 ? p->fieldname_utf8:"";
	const char *fieldvalue=p->fieldvalue_utf8 ? p->fieldvalue_utf8:"";
	const char *tofolder=p->tofolder ? p->tofolder:"";

		fprintf(f, "##Op:%s\n",
			typelist[p->type].name);
		fprintf(f, "##Header:%s\n", fieldname);
		fprintf(f, "##Value:%s\n", fieldvalue);
		fprintf(f, "##Folder:%s\n",
			strcmp(tofolder, INBOX) == 0 ? ".":
			strncmp(tofolder, INBOX ".", sizeof(INBOX)) == 0
			? strchr(tofolder, '.'):tofolder);
		fprintf(f, "##From:%s\n", p->fromhdr ? p->fromhdr:"");

		if (p->flags & MFR_PLAINSTRING)
			fprintf(f, "##PlainString\n");
		if (p->flags & MFR_DOESNOT)
			fprintf(f, "##DoesNot\n");
		if (p->flags & MFR_BODY)
			fprintf(f, "##Body\n");
		if (p->flags & MFR_CONTINUE)
			fprintf(f, "##Continue\n");

		fprintf(f, "##Name:%s\n\n", p->rulename_utf8 ? p->rulename_utf8:"");

		fprintf(f, "\nif (");

		if (p->flags & MFR_DOESNOT)
			fprintf(f, "!");
		fprintf(f, "(");

		switch (p->type)	{
		case startswith:
			if (p->flags & MFR_BODY)
			{
				fprintf(f, "/^");
				print_pattern(f, p->flags, fieldvalue);
				fprintf(f, "/:b");
			}
			else
			{
				fprintf(f, "/^%s: *", fieldname);
				print_pattern(f, p->flags, fieldvalue);
				fprintf(f, "/");
			}
			break;
		case endswith:
			if (p->flags & MFR_BODY)
			{
				fprintf(f, "/");
				print_pattern(f, p->flags, fieldvalue);
				fprintf(f, "$/:b");
			}
			else
			{
				fprintf(f, "/^%s:.*", fieldname);
				print_pattern(f, p->flags, fieldvalue);
				fprintf(f, "$/");
			}
			break;
		case contains:
			if (p->flags & MFR_BODY)
			{
				fprintf(f, "/");
				print_pattern(f, p->flags, fieldvalue);
				fprintf(f, "/:b");
			}
			else
			{
				fprintf(f, "/^%s:.*", fieldname);
				print_pattern(f, p->flags, fieldvalue);
				fprintf(f, "/");
			}
			break;
		case hasrecipient:
			fprintf(f, "hasaddr(\"%s\")", fieldvalue);
			break;
		case mimemultipart:
			fprintf(f, "/^Content-Type: *multipart\\/mixed/");
			break;
		case textplain:
			fprintf(f, " (! /^Content-Type:/) || "
					"/^Content-Type: text\\/plain$/ || "
					"/^Content-Type: text\\/plain;/");
			break;
		case islargerthan:
			fprintf(f, "$SIZE > %s", fieldvalue);
			break;
		case anymessage:
			fprintf(f, "1");
			break;
		}
		fprintf(f, "))\n"
			"{\n");

		if (*tofolder == '!')
		{
			fprintf(f, "    %s \"| $SENDMAIL -f \" '\"\"' \" %s\"\n",
				p->flags & MFR_CONTINUE ? "cc":"to",
					tofolder+1);
		}
		else if (*tofolder == '*')
		{
			fprintf(f, "    echo \"%s\"\n"
				"    EXITCODE=%d\n"
				"    exit\n", tofolder+1, EX_SOFTWARE);
		}
		else if (*tofolder == '+')
		{
			struct maildir_filter_autoresp_info ai;

			if (maildir_filter_autoresp_info_init_str(&ai, tofolder+1) == 0)
			{
				if (p->fromhdr && p->fromhdr[0])
				{
					const char *cp;

					fprintf(f, "    AUTOREPLYFROM='");


					for (cp=p->fromhdr; *cp; ++cp)
					{
						if (*cp == '\'' || *cp == '\\')
							putc('\\', f);
						putc(*cp, f);
					}
					fprintf(f, "'\n");
				}
				else
					fprintf(f, "    AUTOREPLYFROM=\"$FROM\"\n"
						);

				fprintf(f, "   `%s -A \"X-Sender: $FROM\""
					" -A \"From: $AUTOREPLYFROM\"",
					MAILBOT);
				if (ai.dsnflag)
					fprintf(f, " -M \"$FROM\"");
				fprintf(f, " -m \"%s/autoresponses/%s\"",
					maildirpath, ai.name);
				if (ai.noquote)
					fprintf(f, " -N");
				if (ai.days > 0)
					fprintf(f,
						" -d \"%s/autoresponses/"
						"%s.dat\" -D %u",
					maildirpath, ai.name, ai.days);
				fprintf(f, " $SENDMAIL -t -f \"\"`\n");
				maildir_filter_autoresp_info_free(&ai);
			}
		}
		else if (strcmp(tofolder, "exit") == 0)
		{
			fprintf(f, "    exit\n");
		}
		else
		{
			char *s;

			s=maildir_name2dir(maildirpath, tofolder);

			if (!s)
				fprintf(f, "  # INTERNAL ERROR in maildir_name2dir\n");
			else
			{
				fprintf(f,
					"   %s \"%s/.\"\n",
					p->flags & MFR_CONTINUE ? "cc":"to",
					s);
				free(s);
			}
		}
		fprintf(f, "}\n\n");
	}
	fflush(f);
	if (ferror(f))
	{
		fclose(f);
		return (-1);
	}
	fprintf(f, "to \"%s/.\"\n", maildirpath);
	if (fclose(f))
		return (-1);
	if (chmod(filename, 0600))
		return (-1);

	return (0);
}

int maildir_filter_loadrules(struct maildirfilter *r, const char *filename)
{
FILE	*f=fopen(filename, "r");
char	buf[BUFSIZ];
char	*p;

enum	maildirfiltertype new_type;
char	new_header[256];
char	new_value[256];
char	new_folder[256];
char	new_autoreplyfrom[512];

int	flags;

	if (!f)	return (MF_LOADNOTFOUND);

	if (fgets(buf, sizeof(buf), f) == 0 ||
		strncmp(buf, "#MFMAILDROP=", 12))
	{
		fclose(f);
		return (MF_LOADFOREIGN);
	}

	flags=atoi(buf+12);
	if (flags != 1 && flags != 2)
	{
		fclose(f);
		return (MF_LOADFOREIGN);
	}

	new_type=contains;
	new_header[0]=0;
	new_value[0]=0;
	new_folder[0]=0;
	new_autoreplyfrom[0]=0;
	flags=0;

#define	SET(f,b) { f[0]=0; strncat( (f), (b), sizeof(f)-1); }

	while ( fgets(buf, sizeof(buf), f))
	{
	int	i;

		p=strchr(buf, '\n');
		if (p)	*p=0;
		if (strncmp(buf, "##", 2))	continue;
		p=buf+2;
		while ( *p && isspace((int)(unsigned char)*p))
			++p;

		if (strncasecmp(p, "From:", 5) == 0)
		{
			p += 5;
			SET(new_autoreplyfrom, p);
			continue;
		}


		if (strncasecmp(p, "Op:", 3) == 0)
		{
			p += 3;

			for (i=0; typelist[i].name; i++)
				if (strcasecmp(typelist[i].name, p) == 0)
					break;
			if (!typelist[i].name)
			{
				fclose(f);
				return (MF_LOADFOREIGN);
			}
			new_type=typelist[i].type;
			continue;
		}

		if (strncasecmp(p, "Header:", 7) == 0)
		{
			p += 7;
			SET(new_header, p);
			continue;
		}

		if (strncasecmp(p, "Value:", 6) == 0)
		{
			p += 6;
			SET(new_value, p);
			continue;
		}

		if (strncasecmp(p, "Folder:", 7) == 0)
		{
			p += 7;

			if (*p == '.')
			{
				strcpy(new_folder, INBOX);
			}
			else
				new_folder[0]=0;

			if (strcmp(p, "."))
				strncat(new_folder, p,
					sizeof(new_folder)-1-strlen(new_folder));
			continue;
		}

		if (strcasecmp(p, "plainstring") == 0)
		{
			flags |= MFR_PLAINSTRING;
			continue;
		}

		if (strcasecmp(p, "doesnot") == 0)
		{
			flags |= MFR_DOESNOT;
			continue;
		}

		if (strcasecmp(p, "continue") == 0)
		{
			flags |= MFR_CONTINUE;
			continue;
		}

		if (strcasecmp(p, "body") == 0)
		{
			flags |= MFR_BODY;
			continue;
		}

		if (strncasecmp(p, "Name:", 5) == 0)
		{
		int dummy;

			p += 5;
			maildir_filter_appendrule(r, p, new_type, flags,
						  new_header,
						  new_value, new_folder,
						  new_autoreplyfrom,
						  "utf-8", &dummy);
			new_type=contains;
			new_header[0]=0;
			new_value[0]=0;
			new_folder[0]=0;
			new_autoreplyfrom[0]=0;
			flags=0;
		}
	}
	fclose(f);
	return (MF_LOADOK);
}

int maildir_filter_autoresp_info_init(struct maildir_filter_autoresp_info *i, const char *c)
{
	memset(i, 0, sizeof(*i));

	if (maildir_autoresponse_validate(NULL, c))
		return (-1);
	i->name=strdup(c);
	if (!(i->name))
		return (-1);
	return (0);
}

int maildir_filter_autoresp_info_init_str(struct maildir_filter_autoresp_info *i, const char *c)
{
	char *p;

	memset(i, 0, sizeof(*i));
	i->name=strdup(c);
	if (!(i->name))
		return (-1);

	if (strtok(i->name, " \t\r\n") == NULL)
	{
		errno=EINVAL;
		free(i->name);
		i->name=0;
		return (-1);
	}

	while ((p=strtok(NULL, " \t\r\n")) != NULL)
	{
		if (strncmp(p, "dsn=", 4) == 0)
			i->dsnflag=atoi(p+4) ? 1:0;
		else if (strncmp(p, "days=", 5) == 0)
			i->days=atoi(p+5);
		else if (strcmp(p, "noquote") == 0)
			i->noquote=1;
	}
	return (0);
}

void maildir_filter_autoresp_info_free(struct maildir_filter_autoresp_info *i)
{
	if (i->name)
	{
		free(i->name);
		i->name=0;
	}
}

char *maildir_filter_autoresp_info_asstr(struct maildir_filter_autoresp_info *i)
{
	char days_buf[NUMBUFSIZE+10];

	const char *dsn_arg="";
	const char *days_arg="";
	const char *noquote_arg="";

	char *p;

	if (i->dsnflag)
		dsn_arg=" dsn=1";
	if (i->days > 0)
	{
		strcpy(days_buf, " days=");
		libmail_str_size_t(i->days, days_buf+6);
		days_arg=days_buf;
	}

	if (i->noquote)
		noquote_arg=" noquote";

	p=malloc(strlen(i->name)+1+strlen(dsn_arg)+strlen(days_arg)+
		 strlen(noquote_arg));
	if (!p)
		return (NULL);

	strcat(strcat(strcat(strcpy(p, i->name), dsn_arg), days_arg),
	       noquote_arg);
	return (p);
}
