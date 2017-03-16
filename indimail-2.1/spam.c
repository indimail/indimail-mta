/*
 * $Log: spam.c,v $
 * Revision 2.19  2017-03-13 14:08:26+05:30  Cprogrammer
 * replaced qmaildir with sysconfdir, use PREFIX for bin prefix
 *
 * Revision 2.18  2016-05-17 14:56:59+05:30  Cprogrammer
 * use control directory defined by configure
 *
 * Revision 2.17  2010-04-23 11:02:44+05:30  Cprogrammer
 * use spamcount for project id in ftok
 *
 * Revision 2.16  2010-03-30 12:03:36+05:30  Cprogrammer
 * parse MAIL from instead of Mail from:
 *
 * Revision 2.15  2010-03-30 09:08:20+05:30  Cprogrammer
 * call qmail-cdb to create cdb file instead of qmail-spamdb
 *
 * Revision 2.14  2005-12-29 22:50:24+05:30  Cprogrammer
 * use getEnvConfigStr to set variables from environment variables
 *
 * Revision 2.13  2005-12-21 09:48:56+05:30  Cprogrammer
 * make gcc 4 happy
 *
 * Revision 2.12  2003-12-23 23:11:14+05:30  Cprogrammer
 * use regex() if QREGEX env variable is defined
 *
 * Revision 2.11  2003-12-23 20:04:24+05:30  Cprogrammer
 * spamdb functionality
 *
 * Revision 2.10  2003-12-21 21:15:28+05:30  Cprogrammer
 * execute qmail-spamdb if spammers detected
 *
 * Revision 2.9  2003-12-21 14:28:31+05:30  Cprogrammer
 * added option to parse smtp log file for bogofilter tagged lines
 *
 * Revision 2.8  2003-02-08 21:21:50+05:30  Cprogrammer
 * added braces
 *
 * Revision 2.7  2003-02-01 22:51:59+05:30  Cprogrammer
 * changes for updating badrcptto
 *
 * Revision 2.6  2002-12-26 16:38:17+05:30  Cprogrammer
 * refined pattern matching
 *
 * Revision 2.5  2002-11-04 12:32:22+05:30  Cprogrammer
 * open files in qmail/control in append mode
 *
 * Revision 2.4  2002-10-28 01:53:10+05:30  Cprogrammer
 * numerous big fixes
 * used fnmatch() instead of DoMatch()
 *
 * Revision 2.3  2002-10-24 01:57:46+05:30  Cprogrammer
 * made MAXADDR configurable through env variable MAXADDR
 * major rewrite
 *
 * Revision 2.2  2002-10-20 22:31:10+05:30  Cprogrammer
 * correction for solaris compilation
 * badmailfrom can be picked from environment
 *
 * Revision 2.1  2002-10-20 22:27:44+05:30  Cprogrammer
 * common routines for spam filtering
 *
 */
#include "indimail.h"
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <ctype.h>
#include <sys/ipc.h>

#define BADMAIL 1
#define BADRCPT 2
#define SPAMDB  3

#ifndef	lint
static char     sccsid[] = "$Id: spam.c,v 2.19 2017-03-13 14:08:26+05:30 Cprogrammer Exp mbhangui $";
#endif

static char    *parseLine1(char *);
static char    *parseLine2(char *);
static char    *parseLine3(char *);
static int      matchregex(char *, char *);

static maddr  **spammer_hash;
static maddr  **ignored_hash;
static int      bounce, maxaddr;

/*
 * This function parses one line from log file, checks if any " from " 
 * is matched. If so, returns the mail address 
 * @400000003d97d57c07c6d64c info msg 181049: bytes 1872 from <inditac_escalation@indimail.org> qp 1176 uid 0
 * @400000003da2eb6637759be4 info msg 181997: bytes 50526 from <#@[]> qp 8753 uid 506
 * @400000003da2eb6637759be4 info msg 181997: bytes 50526 from <> qp 8753 uid 506
 */
static char    *
parseLine1(char *str)
{
	char           *ptr, *cptr;
	char           *email;
	int             len;

	if (!(ptr = strstr(str, " from <")))
		return ((char *) 0);
	if (!(cptr = strstr(str, "> qp ")))
		return ((char *) 0);
	*cptr = 0;
	ptr += 7;
	if (!*ptr || !strncmp(ptr, "#@[]", 5))
	{
		bounce++;
		return ((char *) 0);
	}
	len = strlen(ptr) + 1;
	if (!(email = (char *) malloc(sizeof(char) * len)))
	{
		fprintf(stderr, "parseLine1: malloc: %s\n", strerror(errno));
		return ((char *) 0);
	}
	strncpy(email, ptr, len);
	lowerit(email);
	return (email);
}

static char    *
parseLine2(char *str)
{
	char           *ptr, *cptr;
	char           *email;
	int             len;

	if (!(ptr = strstr(str, "starting delivery ")))
		return ((char *) 0);
	if (!(ptr = strrchr(str, ' ')))
		return ((char *) 0);
	if ((cptr = strrchr(str, '\n')))
		*cptr = 0;
	ptr += 1;
	len = strlen(ptr) + 1;
	if (!(email = (char *) malloc(sizeof(char) * len)))
	{
		fprintf(stderr, "parseLine2: malloc: %s\n", strerror(errno));
		return ((char *) 0);
	}
	strncpy(email, ptr, len);
	lowerit(email);
	return (email);
}
/*-
 * @400000004bac90a33711966c qmail-smtpd: pid 14544 from ::ffff:127.0.0.1
 * HELO <indimail.org> MAIL from <sitelist-bounces@lists.sourceforge.net>
 * RCPT <mailstore@indimail.org>
 * AUTH <local-rcpt> Size: 7330
 * X-Bogosity: No, spamicity=0.500000, cutoff=9.90e-01, ham_cutoff=0.00e+00, queueID=x18cs65599wff, msgID=<E1Nv0Sc-0006w3-BF@sfs-web-2.v29.ch3.sourceforge.com>, ipaddr=216.34.181.68
 */
static char    *
parseLine3(char *str)
{
	char           *ptr, *cptr, *tmp;
	char           *email;
	int             len;

	if (!(ptr = strstr(str, " qmail-smtpd: pid ")))
		return ((char *) 0);
	if (!(ptr = strstr(str, "> MAIL from <")))
		return ((char *) 0);
	ptr += 13;
	for (cptr = ptr;*cptr && *cptr != '>';cptr++);
	*cptr = 0;
	tmp = cptr + 1;
	if (!(cptr = strstr(tmp, "X-Bogosity")))
		return ((char *) 0);
	for (;*cptr && *cptr != ':';cptr++);
	if (!*cptr)
		return ((char *) 0);
	for (cptr++;*cptr && isspace(*cptr);cptr++);
	if (!*cptr)
		return ((char *) 0);
	if (memcmp(cptr, "Yes", 3))
		return((char *) 0);
	if (!*ptr || !strncmp(ptr, "#@[]", 5))
	{
		bounce++;
		return ((char *) 0);
	}
	len = strlen(ptr) + 1;
	if (!(email = (char *) malloc(sizeof(char) * len)))
	{
		fprintf(stderr, "parseLine3: malloc: %s\n", strerror(errno));
		return ((char *) 0);
	}
	strncpy(email, ptr, len);
	lowerit(email);
	return (email);
}

/*
 * fills our ignore list. these addresses will not be treated as spammers 
 */
int
loadIgnoreList(char *fn)
{
	FILE           *fp;
	char           *ptr, *cptr;
	int             len, status;
	char            buffer[1024];

	if (!(fp = fopen(fn, "r")))
	{
		if(errno == 2)
			return(0);
		return(-1);
	}
	for (status = 0;;)
	{
		if (!fgets(buffer, 1024, fp))
			break;
		if ((ptr = strchr(buffer, '#')))
			*ptr = 0;
		for (ptr = buffer; *ptr && isspace((int) *ptr); ptr++);
		if (!*ptr)
			continue;
		if((cptr = strchr(ptr, ' ')) || (cptr = strchr(ptr, '\n')))
			*cptr = 0;
		len = strlen(ptr);
		len++;
		if (!(cptr = (char *) malloc(sizeof(char) * len)))
		{
			fprintf(stderr, "loadIgnoreList: malloc: %s\n", strerror(errno));
			fclose(fp);
			return(-1);
		}
		strncpy(cptr, ptr, len);
		lowerit(cptr);
		if(!insertAddr(IGNOREHASHTAB, cptr))
			status = -1;
	}
	fclose(fp);
	return(status);
}

/*
 * traverse "from" linked list and decide whether the hit count per mail address
 * exceeds the "spammer threshold" 
 */
int
spamReport(int spamNumber, char *outfile)
{
	FILE           *fp;
	char            tmpbuf[MAX_BUFF];
	char           *sysconfdir, *controldir, *ptr;
	char           *(spamprog[3]);
	maddr          *p;
	int             i, flag, spamcnt = 0;

	if (!(fp = fopen(outfile, "a+")))
	{
		fprintf(stderr, "fopen: %s: %s\n", outfile, strerror(errno));
		return(-1);
	}
	fprintf(stderr, "%-40s Mail Count\n", "Spammer's Email Address");
	if(!maxaddr)
	{
		getEnvConfigStr(&ptr, "MAXADDR", MAXADDR);
		maxaddr = atoi(ptr);
	}
	getEnvConfigStr(&sysconfdir, "SYSCONFDIR", SYSCONFDIR);
	getEnvConfigStr(&controldir, "CONTROLDIR", CONTROLDIR);
	if (*tmpbuf == '/')
		snprintf(tmpbuf, MAX_BUFF, "%s", controldir);
	else
		snprintf(tmpbuf, MAX_BUFF, "%s/%s", sysconfdir, controldir);
	if (!memcmp(outfile, tmpbuf, strlen(tmpbuf)))
		flag = 1;
	else
		flag = 0;
	for (i = 0; spammer_hash && i < maxaddr; i++)
	{
		for (p = spammer_hash[i]; p != NULL; p = p->next)
		{
			if (p->cnt >= spamNumber && !isIgnored(p->mail))
			{
				spamcnt++;
				if(flag)
					fprintf(fp, "%s\n", p->mail);
				else
					fprintf(fp, "%s %d\n", p->mail, p->cnt);
				fprintf(stderr, "%-40s %d\n", p->mail, p->cnt);
			}
		}
	}
	fclose(fp);
	fprintf(stderr, "Bounces: %d\n", bounce);
	fprintf(stderr, "%d Spammers detected\n", spamcnt);
	if (flag && spamcnt)
	{
		spamprog[0] = PREFIX"/bin/qmail-cdb";
		if ((ptr = strrchr(outfile, '/')))
			ptr++;
		else
			ptr = outfile;
		spamprog[1] = ptr;
		spamprog[2] = 0;
		execv(*spamprog, spamprog);
		perror(*spamprog);
	}
	return (spamcnt);
}

/*
 * This function reads the logfile and fills the " from " linked list 
 */
int
readLogFile(char *fn, int type, int count)
{

	FILE           *fp, *keyfp;
	char           *email;
	char            buf[1024], keyfile[1024];
	int             status;
	unsigned long   pos, seekPos;

	if (!fn || !*fn)
		fp = stdin;
	else
	{
		if ((fp = fopen(fn, "r")) == NULL)
		{
			fprintf(stderr, "readLogFile: fopen: %s: %s\n", fn, strerror(errno));
			return(-1);
		}
		snprintf(keyfile, 1024, "/tmp/%d", ftok(fn, count == -1 ? 1 : count));
		pos = 0;
		if ((keyfp = fopen(keyfile, "r")))
		{
			if (fscanf(keyfp, "%ld", &pos) == 1)
			{
				if (fseek(fp, pos, SEEK_SET))
				{
					fprintf(stderr, "readLogFile: fseek %ld: %s\n", pos, strerror(errno));
					fclose(fp);
					fclose(keyfp);
					return(-1);
				}
			}
			fclose(keyfp);
		}
	}
	for (status = 0, seekPos = -1;;)
	{
		if (!fgets(buf, 1024, fp))
			break;
		seekPos = ftell(fp);
		switch (type)
		{
			case BADMAIL:
				if ((email = parseLine1(buf)) && !insertAddr(SPAMMERHASHTAB, email))
					status = -1;
			case BADRCPT:
				if ((email = parseLine2(buf)) && !insertAddr(SPAMMERHASHTAB, email))
					status = -1;
			case SPAMDB:
				if ((email = parseLine3(buf)) && !insertAddr(SPAMMERHASHTAB, email))
					status = -1;
		}
	}
	if (fp != stdin)
	{
		fclose(fp);
		if (seekPos != -1 && seekPos > pos && (keyfp = fopen(keyfile, "w")))
		{
			fprintf(keyfp, "%ld", seekPos);
			fclose(keyfp);
		}
	}
	return(status);
}

/*
 * check if the supplied mail address exist in our "ignored" table 
 */
int
isIgnored(char *email)
{
	maddr          *p;
	char           *ptr;
	char            pattern[MAX_BUFF];
	int             i, usewildmat;
	char           *qregex;

	if(!maxaddr)
	{
		getEnvConfigStr(&ptr, "MAXADDR", MAXADDR);
		maxaddr = atoi(ptr);
	}
	qregex = getenv("QREGEX");
	for (i = 0; i < maxaddr; i++)
	{
		for (p = ignored_hash[i]; p != NULL; p = p->next)
		{
			if (qregex)
			{
				if (*(p->mail) == '@')
					snprintf(pattern, sizeof(pattern), ".*%s", p->mail);
				else
					snprintf(pattern, sizeof(pattern), "%s", p->mail);
				if (matchregex(email, pattern) == 1)
					return(1);
			} else
			{
				if (*(p->mail) == '@')
					usewildmat = 1;
				else
					usewildmat = 0;
				for (ptr = p->mail;*ptr && !usewildmat;ptr++)
				{
					switch(*ptr)
					{
						case '?':
						case '*':
						case '[':
						case ']':
							usewildmat = 1;
							break;
					}
				}
				if(!usewildmat)
				{
					if(!strncmp(p->mail, email, MAX_BUFF))
						return(1);
				} else
				{
					if(*(p->mail) == '@')
						snprintf(pattern, sizeof(pattern), "*%s", p->mail);
					else
						snprintf(pattern, sizeof(pattern), "%s", p->mail);
					if (wildmat(email, pattern))
						return(1);
				}
			} /*- if (qregex) */
		}
	}
	return(0);
}

/*
 * Check if the email address if already in table.
 * If so, increment its hit count; if not, 
 * add it to table 
 */
maddr          *
insertAddr(int ht, char *email)
{
	int             h;
	maddr          *sym;
	maddr         **hash_tab;
	char           *ptr;

	if(!maxaddr)
	{
		getEnvConfigStr(&ptr, "MAXADDR", MAXADDR);
		maxaddr = atoi(ptr);
	}
	if(!spammer_hash && !(spammer_hash = (maddr **) calloc(1, sizeof(maddr *) * maxaddr)))
	{
		fprintf(stderr, "insertAddr: calloc: %s", strerror(errno));
		return((maddr *) 0);
	} 
	if(!ignored_hash && !(ignored_hash = (maddr **) calloc(1, sizeof(maddr *) * maxaddr)))
	{
		fprintf(stderr, "insertAddr: calloc: %s", strerror(errno));
		return((maddr *) 0);
	}
	switch (ht)
	{
	case IGNOREHASHTAB:
		hash_tab = ignored_hash;
		break;
	case SPAMMERHASHTAB:
		hash_tab = spammer_hash;
		break;
	default:
		return((maddr *) 0);
		break;
	}
	/*
	 * Calculate hash sum, locate and increment count
	 * if email exists, otherwise place it in spammer_hash 
	 */
	h = hash(email);
	for (sym = hash_tab[h]; sym; sym = sym->next)
	{
		if (!strncmp(email, sym->mail, 1024))
		{
			sym->cnt++;
			return sym;
		}
	}
	sym = (maddr *) malloc(sizeof(maddr));
	/*- No need a malloc for mail, because, email's been malloc'ed before */
	sym->mail = email;
	sym->cnt = 1;
	sym->next = hash_tab[h];
	hash_tab[h] = sym;
	return sym;
}

unsigned int
hash(char *str)
{
	unsigned int    h;
	char           *ptr;

	h = 0;
	for (ptr = str; *ptr; ptr++)
		h = MULTIPLIER * h + *ptr;
	if(!maxaddr)
	{
		getEnvConfigStr(&ptr, "MAXADDR", MAXADDR);
		maxaddr = atoi(ptr);
	}
	return h % maxaddr;
}

void
print_list(int list)
{
	maddr         **hash_tab;
	maddr          *sym;
	char           *ptr;
	int             i;

	switch (list)
	{
	case IGNOREHASHTAB:
		hash_tab = ignored_hash;
		break;
	case SPAMMERHASHTAB:
		hash_tab = spammer_hash;
		break;
	default:
		return;
		break;
	}
	getEnvConfigStr(&ptr, "MAXADDR", MAXADDR);
	maxaddr = atoi(ptr);
	for (i = 0; i < maxaddr; i++)
	{
		for (sym = hash_tab[i]; sym; sym = sym->next)
			fprintf(stderr, "%s - %d mails\n", sym->mail, sym->cnt);
	}
}

#include <regex.h>
#define REGCOMP(X,Y)    regcomp(&X, Y, REG_EXTENDED|REG_ICASE)
#define REGEXEC(X,Y)    regexec(&X, Y, (size_t)0, (regmatch_t *)0, (int)0)

static int
matchregex(char *text, char *regex)
{
	regex_t         qreg;
	char            errbuf[512];
	int             retval = 0;

	/*- build the regex */
	if ((retval = REGCOMP(qreg, regex)) != 0)
	{
		regerror(retval, &qreg, errbuf, sizeof(errbuf));
		regfree(&qreg);
		fprintf(stderr, "%s: %s: %s\n", text, regex, errbuf);
		return (-retval);
	}
	/*- execute the regex */
	if ((retval = REGEXEC(qreg, text)) != 0)
	{
		/*- did we just not match anything?  */
		if (retval == REG_NOMATCH)
		{
			regfree(&qreg);
			return (0);
		}
		regfree(&qreg);
		return (-retval);
	}
	/*- signal the match */
	regfree(&qreg);
	return (1);
}

void
getversion_spam_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
