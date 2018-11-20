/*
** Copyright 1998 - 2018 Double Precision, Inc.  See COPYING for
** distribution information.
*/

#if	HAVE_CONFIG_H
#include "rfc2045_config.h"
#endif
#include	<sys/types.h>
#include	<sys/stat.h>
#include	<time.h>
#include	<stdio.h>
#include	<errno.h>
#include	<string.h>
#include	<langinfo.h>

#if	HAVE_STRINGS_H
#include	<strings.h>
#endif

#if	HAVE_LOCALE_H
#include	<locale.h>
#endif

#include	<stdlib.h>
#include	<ctype.h>
#include	<pwd.h>
#include	<fcntl.h>
#include	<signal.h>
#include	"rfc2045.h"
#include	"rfc822/rfc822.h"
#include	"rfc822/rfc2047.h"
#include	"rfc2045charset.h"
#include	"unicode/courier-unicode.h"

#if HAVE_UNISTD_H
#include	<unistd.h>
#endif
#if HAVE_SYS_WAIT_H
#include	<sys/wait.h>
#endif
#include	"numlib/numlib.h"

#if     HAS_GETHOSTNAME
#else
int gethostname(const char *, size_t);
#endif

extern int rfc2045_in_reformime;

static const char *defchset;


void rfc2045_error(const char *errmsg)
{
	fprintf(stderr, "reformime: %s\n", errmsg);
	exit(1);
}

static void do_print_structure(struct rfc2045 *p, struct rfc2045id *id, void *ptr)
{
	ptr=p;

	while (id)
	{
		printf("%d%c", id->idnum, id->next ? '.':'\n');
		id=id->next;
	}
}

static int decode_to_file(const char *p, size_t n, void *ptr)
{
FILE	*fp=(FILE *)ptr;

	while (n)
	{
		--n;
		if (putc((int)(unsigned char)*p++, fp) == EOF)
		{
			perror("write");
			exit(1);
		}
	}
	return (0);
}

void usage()
{
	fprintf(stderr, "Usage: reformime [options]\n");
	fprintf(stderr, "    -d - parse a delivery status notification.\n");
	fprintf(stderr, "    -e - extract contents of MIME section.\n");
	fprintf(stderr, "    -x - extract MIME section to a file.\n");
	fprintf(stderr, "    -X - pipe MIME section to a program.\n");
	fprintf(stderr, "    -i - show MIME info.\n");
	fprintf(stderr, "    -s n.n.n.n[,n.n.n.n]* - specify MIME section(s).\n");
	fprintf(stderr, "    -r - rewrite message, filling in missing MIME headers.\n");
	fprintf(stderr, "    -r7 - also convert 8bit/raw encoding to quoted-printable, if possible.\n");
	fprintf(stderr, "    -r8 - also convert quoted-printable encoding to 8bit, if possible.\n");
	fprintf(stderr, "    -rU - convert quoted-printable encoding to 8bit, unconditionally.\n");
	fprintf(stderr, "    -c charset - default charset for rewriting, -o, and -O.\n");
	fprintf(stderr, "    -m [file] [file]... - create a MIME message digest.\n");
	fprintf(stderr, "    -h \"header\" - decode RFC 2047-encoded header.\n");
	fprintf(stderr, "    -o \"header\" - encode unstructured header using RFC 2047.\n");
	fprintf(stderr, "    -O \"header\" - encode address list header using RFC 2047.\n");

	exit(1);
}

static char *tempname(const char *tempdir)
{
char	pidbuf[NUMBUFSIZE], timebuf[NUMBUFSIZE], hostnamebuf[256];
static unsigned counter=0;
time_t	t;
char	*p;

	libmail_str_pid_t(getpid(), pidbuf);
	time(&t);
	libmail_str_time_t(t, timebuf);
	hostnamebuf[sizeof(hostnamebuf)-1]=0;
	if (gethostname(hostnamebuf, sizeof(hostnamebuf)))
		hostnamebuf[0]=0;
	p=malloc(strlen(tempdir)+strlen(pidbuf)+strlen(timebuf)+
		strlen(hostnamebuf)+100);
	if (!p)	return (0);
	sprintf(p, "%s/%s.%s-%u.%s", tempdir, timebuf, pidbuf, counter++,
		hostnamebuf);
	return (p);
}

struct rfc2045 *read_message()
{
char	buf[BUFSIZ];
struct	rfc2045 *p=rfc2045_alloc_ac();
FILE	*tempfp=0;
int	l;

	if (fseek(stdin, 0L, SEEK_END) < 0 ||
		fseek(stdin, 0L, SEEK_SET) < 0)	/* Pipe, save to temp file */
	{
		tempfp=tmpfile();
	}

	while ((l=fread(buf, 1, sizeof(buf), stdin)) > 0)
	{

		rfc2045_parse(p, buf, l);
		if (tempfp && fwrite(buf, l, 1, tempfp) != 1)
		{
			perror("fwrite");
			exit(1);
		}
	}
	rfc2045_parse_partial(p);

	if (tempfp)	/* Replace stdin */
	{
		dup2(fileno(tempfp), 0);
		fclose(tempfp);
	}
	return (p);
}

void print_structure(struct rfc2045 *p)
{
	rfc2045_decode(p, &do_print_structure, 0);
}

static void notfound(const char *p)
{
	fprintf(stderr, "reformime: MIME section %s not found.\n", p);
	exit(1);
}

static void do_print_info(struct rfc2045 *s)
{
const char *content_type, *transfer_encoding, *charset;
off_t start, end, body;
char *content_name;
off_t nlines, nbodylines;
const char *p;

char *disposition_name, *disposition_filename;

	rfc2045_mimeinfo(s, &content_type, &transfer_encoding, &charset);
	rfc2045_mimepos(s, &start, &end, &body, &nlines, &nbodylines);

	if (rfc2231_udecodeType(s, "name", NULL, &content_name) < 0
	    && (content_name=strdup("")) == NULL)
	{
		perror("malloc");
		exit(1);
	}

	printf("content-type: %s\n", content_type);
	if (*content_name)
	{
		printf("content-name: %s\n", content_name);
	}
	free(content_name);

	printf("content-transfer-encoding: %s\n", transfer_encoding);
	printf("charset: %s\n", charset);
	if (s->content_disposition && *s->content_disposition)
		printf("content-disposition: %s\n", s->content_disposition);

	if ((rfc2231_udecodeDisposition(s, "name", NULL, &disposition_name) < 0
	     && (disposition_name=strdup("")) == NULL)
	    ||
	    (rfc2231_udecodeDisposition(s, "filename", NULL,
					&disposition_filename) < 0
	     && (disposition_filename=strdup("")) == NULL))
	{
		perror("malloc");
		exit(1);
	}

	if (*disposition_name)
		printf("content-disposition-name: %s\n", disposition_name);

	free(disposition_name);

	if (*disposition_filename)
	{
		printf("content-disposition-filename: %s\n",
		       disposition_filename);
	}
	free(disposition_filename);

	if (*(p=rfc2045_content_id(s)))
		printf("content-id: <%s>\n", p);
	if (*(p=rfc2045_content_description(s)))
	{
		char *s=rfc822_display_hdrvalue_tobuf("content-description",
						      p,
						      defchset,
						      NULL,
						      NULL);

		if (!s)
		{
			perror("rfc2047_decode_unicode");
			exit(1);
		}
		printf("content-description: %s\n", s);
		free(s);
	}
	if (*(p=rfc2045_content_language(s)))
		printf("content-language: %s\n", p);
	if (*(p=rfc2045_content_md5(s)))
		printf("content-md5: %s\n", p);

	printf("starting-pos: %lu\n", (unsigned long)start);
	printf("starting-pos-body: %lu\n", (unsigned long)body);
	printf("ending-pos: %lu\n", (unsigned long)end);
	printf("line-count: %lu\n", (unsigned long)nlines);
	printf("body-line-count: %lu\n", (unsigned long)nbodylines);
}

static void do_print_info_multiple(struct rfc2045 *p, struct rfc2045id *id,
		void *ptr)
{
	printf("section: ");
	do_print_structure(p, id, ptr);
	do_print_info(p);
	printf("\n");
}

void print_info(struct rfc2045 *p, const char *mimesection)
{
struct	rfc2045 *s;

	if (mimesection)
	{
		s=rfc2045_find(p, mimesection);
		if (!s)
			notfound(mimesection);
		printf("section: %s\n", mimesection);
		do_print_info(s);
		return;
	}
	rfc2045_decode(p, &do_print_info_multiple, 0);
}

static void do_print_section(struct rfc2045 *s, FILE *fp)
{
off_t start, end, body;
off_t nlines;
off_t nbodylines;

	rfc2045_mimepos(s, &start, &end, &body, &nlines, &nbodylines);

	if (fseek(stdin, body, SEEK_SET) == -1)
	{
		perror("fseek");
		exit(1);
	}

	rfc2045_cdecode_start(s, &decode_to_file, fp);
	while (body < end)
	{
	char	buf[BUFSIZ];
	size_t	n=sizeof(buf);

		if ((off_t)n > end-body)	n=end-body;
		n=fread(buf, 1, n, stdin);
		if (n == 0)
		{
			perror("fread");
			exit(1);
		}
		rfc2045_cdecode(s, buf, n);
		body += n;
	}
	rfc2045_cdecode_end(s);
}

void print_decode(struct rfc2045 *p, const char *mimesection)
{
struct	rfc2045 *s;

	if (!mimesection)
		usage();

	s=rfc2045_find(p, mimesection);
	if (!s)
		notfound(mimesection);

	do_print_section(s, stdout);
}

void rewrite(struct rfc2045 *p, int rwmode)
{
	struct rfc2045src *src;

	rfc2045_ac_check(p, rwmode);

	src=rfc2045src_init_fd(fileno(stdin));

	if (src == NULL || rfc2045_rewrite(p, src, fileno(stdout),
		"reformime (" RFC2045PKG " " RFC2045VER ")"))
	{
		perror("reformime");
		exit(1);
	}
	rfc2045src_deinit(src);
}

static char *get_suitable_filename(struct rfc2045 *r, const char *pfix,
	int ignore_filename)
{
char *disposition_name;
char *disposition_filename;
char	*filename_buf;
char *content_name;
char	*p, *q;
char	*dyn_disp_name=0;

const char *disposition_filename_s;

	if (rfc2231_udecodeDisposition(r, "name", NULL, &disposition_name) < 0)
		disposition_name=NULL;

	if (rfc2231_udecodeDisposition(r, "filename", NULL,
				       &disposition_filename) < 0)
		disposition_filename=NULL;

	if (rfc2231_udecodeType(r, "name", NULL,
				&content_name) < 0)
		content_name=NULL;

	disposition_filename_s=disposition_filename;

	if (!disposition_filename_s || !*disposition_filename_s)
		disposition_filename_s=disposition_name;
	if (!disposition_filename_s || !*disposition_filename_s)
		disposition_filename_s=content_name;

	filename_buf=strdup(disposition_filename_s ? disposition_filename_s:"");

	if (!filename_buf)
	{
		perror("strdup");
		exit(1);
	}

	if (content_name)		free(content_name);
	if (disposition_name)		free(disposition_name);
	if (disposition_filename)	free(disposition_filename);

	if (strlen(filename_buf) > 32)
	{
		p=filename_buf;
		q=filename_buf + strlen(filename_buf)-32;
		while ( (*p++ = *q++) != 0)
			;
	}

	/* Strip leading/trailing spaces */

	p=filename_buf;
	while (*p && isspace((int)(unsigned char)*p))
		++p;

	q=filename_buf;
	while ((*q=*p) != 0)
	{
		++p;
		++q;
	}

	for (p=q=filename_buf; *p; p++)
		if (!isspace((int)(unsigned char)*p))
			q=p+1;
	*q=0;

	disposition_filename_s=filename_buf;

	if (ignore_filename)
	{
	char	numbuf[NUMBUFSIZE];
	static size_t counter=0;
	const char *p=libmail_str_size_t(++counter, numbuf);

		dyn_disp_name=malloc(strlen(disposition_filename_s)
			+ strlen(p)+2);
		if (!dyn_disp_name)
		{
			perror("malloc");
			exit(1);
		}
		disposition_filename_s=strcat(strcat(strcpy(
			dyn_disp_name, p), "-"),
			disposition_filename_s);
	}
	else if (!disposition_filename_s || !*disposition_filename_s)
	{
		dyn_disp_name=tempname(".");
		disposition_filename_s=dyn_disp_name+2;	/* Skip over ./ */
	}

	p=malloc((pfix ? strlen(pfix):0)+strlen(disposition_filename_s)+1);
	if (!p)
	{
		perror("malloc");
		exit(1);
	}
	*p=0;
	if (pfix)	strcpy(p, pfix);
	q=p+strlen(p);
	for (strcpy(q, disposition_filename_s); *q; q++)
		if (!isalnum(*q) && *q != '.' && *q != '-')
			*q='_';

	if (dyn_disp_name)	free(dyn_disp_name);

	if (!pfix)
	{
        const char *content_type_s;
        const char *content_transfer_encoding_s;
        const char *charset_s;
	int c;
	static char filenamebuf[256];
	char	*t;
	FILE	*tty;

		if ((tty=fopen("/dev/tty", "r+")) == 0)
		{
			perror("/dev/tty");
			exit(1);
		}

		rfc2045_mimeinfo(r, &content_type_s,
			&content_transfer_encoding_s, &charset_s);

		fprintf (tty, "Extract %s? ", content_type_s);
		fflush(tty);
		c=getc(tty);
		if (c != '\n' && c != EOF)
		{
		int	cc;

			while ((cc=getc(tty)) != '\n' && cc != EOF)
				;
		}
		if (c != 'y' && c != 'Y')
		{
			free(p);
			fclose(tty);
			free(filename_buf);
			return (0);
		}
		fprintf (tty, "Filename [%s]: ", p);
		if (fgets(filenamebuf, sizeof(filenamebuf)-1, tty) == NULL)
			filenamebuf[0]=0;

		fclose(tty);
		t=strchr(filenamebuf, '\n');
		if (t)	*t=0;
		else
		{
			fprintf(stderr, "Filename too long.\n");
			exit(1);
		}
		if (filenamebuf[0])
		{
			free(p);
			p=strdup(filenamebuf);
			if (!p)
			{
				perror("malloc");
				exit(1);
			}
		}
	}
	free(filename_buf);
	return (p);
}

static void extract_file(struct rfc2045 *p,
	const char *filename, int argc, char **argv)
{
char	*f;
FILE	*fp;
int	ignore=0;

	for (;;)
	{
	int	fd;

		f=get_suitable_filename(p, filename, ignore);
		if (!f)	return;

		fd=open(f, O_WRONLY|O_CREAT|O_EXCL, 0666);
		if (fd < 0)
		{
			if (errno == EEXIST)
			{
				printf("%s exists.\n", f);
				free(f);
				ignore=1;
				continue;
			}

			perror(f);
			exit(1);
		}
		fp=fdopen(fd, "w");
		if (!fp)
		{
			perror("fdopen");
			exit(1);
		}
		break;
	}

	do_print_section(p, fp);
	if (fflush(fp) || ferror(fp))
	{
		perror("write");
		exit(1);
	}
	fclose(fp);
	free(f);
}

static void extract_pipe(struct rfc2045 *p,
	const char *filename,
	int argc, char **argv)
{
char	*f=get_suitable_filename(p, "FILENAME=", 0);
int	pipefd[2];
pid_t	pid, p2;
FILE	*fp;
int	waitstat;

	if (argc == 0)
	{
		fprintf(stderr, "reformime: Invalid -X option.\n");
		exit(1);
	}

	if (pipe(pipefd))
	{
		perror("pipe");
		exit(1);
	}

	if ((fp=fdopen(pipefd[1], "w")) == 0)
	{
		perror("fdopen");
		exit(1);
	}

	while ((pid=fork()) == -1)
	{
		sleep(2);
	}

	if (pid == 0)
	{
        const char *content_type_s;
        const char *content_transfer_encoding_s;
        const char *charset_s;

		if (!f)	f="FILENAME=attachment.dat";
		putenv(f);
		rfc2045_mimeinfo(p, &content_type_s,
			&content_transfer_encoding_s, &charset_s);
		f=malloc(strlen(content_type_s)
			+sizeof("CONTENT_TYPE="));
		if (!f)
		{
			perror("malloc");
			exit(1);
		}
		strcat(strcpy(f, "CONTENT_TYPE="), content_type_s);
		putenv(f);
		dup2(pipefd[0], 0);
		close(pipefd[0]);
		close(pipefd[1]);
		execv(argv[0], argv);
		perror("exec");
		_exit(1);
	}
	close(pipefd[0]);
	signal(SIGPIPE, SIG_IGN);
	do_print_section(p, fp);
	signal(SIGPIPE, SIG_DFL);
	fclose(fp);
	close(pipefd[1]);

	while ((p2=wait(&waitstat)) != pid && p2 != -1)
		;
	free(f);

	if ((p2 == pid) && WIFEXITED(waitstat))
	{
		if (WEXITSTATUS(waitstat) != 0)
		{
			fprintf(stderr, "reformime: %s exited with status %d.\n",
				argv[0], WEXITSTATUS(waitstat));
			exit(WEXITSTATUS(waitstat) + 20);
		}
	}
}

static void extract_section(struct rfc2045 *top_rfcp, const char *mimesection,
	const char *extract_filename, int argc, char **argv,
	void	(*extract_func)(struct rfc2045 *, const char *,
		int, char **))
{
	if (mimesection)
	{
		top_rfcp=rfc2045_find(top_rfcp, mimesection);
		if (!top_rfcp)
			notfound(mimesection);
		if (top_rfcp->firstpart)
		{
			fprintf(stderr, "reformime: MIME section %s is a compound section.\n", mimesection);
			exit(1);
		}
		(*extract_func)(top_rfcp, extract_filename, argc, argv);
		return;
	}

	/* Recursive */

	if (top_rfcp->firstpart)
	{
		for (top_rfcp=top_rfcp->firstpart; top_rfcp;
			top_rfcp=top_rfcp->next)
			extract_section(top_rfcp, mimesection,
				extract_filename, argc, argv, extract_func);
		return;
	}

	if (!top_rfcp->isdummy)
		(*extract_func)(top_rfcp, extract_filename, argc, argv);
}

static void print_dsn_recip(char *addr, char *action)
{
char *p, *q;

	if (!action || !addr)
	{
		if (action)	free(action);
		if (addr)	free(addr);
		return;
	}

	for (p=action; *p; ++p)
		*p=tolower((int)(unsigned char)*p);

	for (p=addr; *p && isspace((int)(unsigned char)*p); ++p)
		;

	if (strncasecmp(p, "rfc822;", 7) &&
	    strncasecmp(p, "utf-8;", 6))
	{
		free(action);
		free(addr);
		return;
	}
	for (q=action; *q && isspace((int)(unsigned char)*q); ++q)
		;

	p=strchr(p, ';')+1;
	while (*p && isspace((int)(unsigned char)*p))
		++p;
	printf("%s %s\n", q, p);
	free(action);
	free(addr);
}

static void dsn(struct rfc2045 *p, int do_orig)
{
const char *content_type_s;
const char *content_transfer_encoding_s;
const char *charset_s;
off_t start_pos, end_pos, start_body;
off_t dummy;
const char *q;
char	buf[BUFSIZ];
unsigned i;
int	ch;
char *recip;
char *action;
char *orecip;

	rfc2045_mimeinfo(p, &content_type_s, &content_transfer_encoding_s,
		&charset_s);
	if (strcasecmp(content_type_s, "multipart/report") ||
		(q=rfc2045_getattr(p->content_type_attr, "report-type")) == 0 ||
		strcasecmp(q, "delivery-status") ||
		!p->firstpart || !p->firstpart->next ||
		!p->firstpart->next->next)
		_exit(1);
	p=p->firstpart->next->next;
	rfc2045_mimeinfo(p, &content_type_s, &content_transfer_encoding_s,
		&charset_s);
	rfc2045_mimepos(p, &start_pos, &end_pos, &start_body, &dummy, &dummy);
	if (!rfc2045_delivery_status_content_type(content_type_s) ||
		fseek(stdin, start_body, SEEK_SET) == -1)
		_exit(1);

	i=0;
	recip=0;
	orecip=0;
	action=0;
	while (start_body < end_pos)
	{
		if ((ch=getchar()) == EOF)	break;
		++start_body;
		if (i < sizeof(buf)-1)
			buf[i++]= ch;
		if (ch != '\n')	continue;
		ch=getchar();
		if (ch != EOF)	ungetc(ch, stdin);
		if (ch != '\n' && isspace((int)(unsigned char)ch))
			continue;
		buf[i-1]=0;
		if (buf[0] == 0)
		{
			if (orecip)
			{
				if (recip)	free(recip);
				recip=orecip;
				orecip=0;
			}
			print_dsn_recip(recip, action);
			recip=0;
			action=0;
		}
		if (strncasecmp(buf, "Final-Recipient:", 16) == 0 &&
			recip == 0)
		{
			recip=strdup(buf+16);
			if (!recip)
			{
				perror("strdup");
				exit(2);
			}
		}
		if (strncasecmp(buf, "Original-Recipient:", 19) == 0 &&
			orecip == 0 && do_orig)
		{
			orecip=strdup(buf+19);
			if (!orecip)
			{
				perror("strdup");
				exit(2);
			}
		}
		if (strncasecmp(buf, "Action:", 7) == 0 && action == 0)
		{
			action=strdup(buf+7);
			if (!action)
			{
				perror("strdup");
				exit(2);
			}
		}
		i=0;
	}
	if (orecip)
	{
		if (recip)	free(recip);
		recip=orecip;
		orecip=0;
	}
	print_dsn_recip(recip, action);
}

static void mimedigest1(int, char **);
static char mimebuf[BUFSIZ];

static void mimedigest(int argc, char **argv)
{
char	*p;
struct filelist { struct filelist *next; char *fn; } *first=0, *last=0;
unsigned pcnt=0;
char	**l;

	if (argc > 0)
	{
		mimedigest1(argc, argv);
		return;
	}

	while (fgets(mimebuf, sizeof(mimebuf), stdin))
	{
	struct	filelist *q;

		if ((p=strchr(mimebuf, '\n')) != 0)	*p=0;
		q=malloc(sizeof(struct filelist));
		if (!q || !(q->fn=strdup(mimebuf)))
		{
			perror("malloc");
			exit(1);
		}

		if (last)	last->next=q;
		else	first=q;
		last=q;
		q->next=0;
		++pcnt;
	}
	if (pcnt == 0)	return;

	if ( (l=malloc(sizeof (char *) * pcnt)) == 0)
	{
		perror("malloc");
	}
	pcnt=0;

	for (last=first; last; last=last->next)
		l[pcnt++]=last->fn;

	mimedigest1(pcnt, l);
	free(l);
	while(first)
	{
		last=first->next;
		free(first->fn);
		free(first);
		first=last;
	}
}

static void mimedigest1(int argc, char **argv)
{
	time_t	t;
	char	boundarybuf[200];
	unsigned boundarycnt=0;
	int	i;
	FILE	*fp;
	int	*utf8;
	if (argc == 0)
		return;

	time (&t);

	utf8=malloc(sizeof(int)*argc);

	/* Search for a suitable boundary */

	do
	{
	int	l;

		sprintf(boundarybuf, "reformime_%lu_%lu_%u",
			(unsigned long)t,
			(unsigned long)getpid(),
			++boundarycnt);

		l=strlen(boundarybuf);

		for (i=0; i<argc; i++)
		{
			int	err=0;
			struct rfc2045 *parser=rfc2045_alloc();

			if (!parser)
			{
				perror(argv[i]);
				exit(1);
			}
			if ((fp=fopen(argv[i], "r")) == 0)
			{
				perror(argv[i]);
				exit(1);
			}

			while (fgets(mimebuf, sizeof(mimebuf), fp))
			{
				rfc2045_parse(parser, mimebuf, strlen(mimebuf));

				if (mimebuf[0] != '-' || mimebuf[1] != '-')
					continue;

				if (strncasecmp(mimebuf+2, boundarybuf, l) == 0)
				{
					err=1;
					break;
				}
			}
			fclose(fp);
			utf8[i]=parser->rfcviolation & RFC2045_ERR8BITHEADER
				? 1:0;
			rfc2045_free(parser);
			if (err)	break;
		}
	} while (i < argc);

	printf("Mime-Version:1.0\n"
		"Content-Type: multipart/digest; boundary=\"%s\"\n\n%s",
			boundarybuf, RFC2045MIMEMSG);

	for (i=0; i<argc; i++)
	{
		if ((fp=fopen(argv[i], "r")) == 0)
		{
			perror(argv[i]);
			exit(1);
		}

		printf("\n--%s\nContent-Type: %s\n\n",
		       boundarybuf,
		       utf8[i] ? RFC2045_MIME_MESSAGE_GLOBAL:
		       RFC2045_MIME_MESSAGE_RFC822);

		while (fgets(mimebuf, sizeof(mimebuf), fp))
			printf("%s", mimebuf);
		fclose(fp);
	}
	free(utf8);
	printf("\n--%s--\n", boundarybuf);
}

static void display_decoded_header(const char *ptr, size_t cnt, void *dummy)
{
	if (cnt == 0)
		putchar('\n');
	else
		fwrite(ptr, cnt, 1, stdout);
}

static int doconvtoutf8_stdout(const char *ptr, size_t n, void *dummy)
{
	if (fwrite(ptr, n, 1, stdout) != 1)
		return -1;

	return 0;
}

static int main2(const char *mimecharset, int argc, char **argv)
{
int	argn;
char	optc;
char	*optarg;
char	*mimesection=0;
char	*section=0;
int	doinfo=0, dodecode=0, dorewrite=0, dodsn=0, domimedigest=0;
int	dodecodehdr=0, dodecodeaddrhdr=0, doencodemime=0, doencodemimehdr=0;

char	*decode_header="";
struct	rfc2045 *p;
int	rwmode=0;
int     convtoutf8=0;
int	dovalidate=0;
void	(*do_extract)(struct rfc2045 *, const char *, int, char **)=0;
const char *extract_filename=0;
int rc=0;


	rfc2045_in_reformime=1;

	for (argn=1; argn<argc; )
	{
		if (argv[argn][0] != '-')	break;
		optarg=0;
		optc=argv[argn][1];
		if (optc && argv[argn][2])	optarg=argv[argn]+2;
		++argn;
		switch	(optc)	{
		case 'c':
			if (!optarg && argn < argc)
				optarg=argv[argn++];
			if (optarg && *optarg)
			{
				char *p=unicode_convert_tobuf("",
								optarg,
								unicode_u_ucs4_native,
								NULL);

				if (!p)
				{
					fprintf(stderr, "Unknown charset: %s\n",
						optarg);
					exit(1);
				}
				free(p);
				mimecharset=optarg;
			}
			break;

		case 's':
			if (!optarg && argn < argc)
				optarg=argv[argn++];
			if (optarg && *optarg)	section=strdup(optarg);
			break;
		case 'i':
			doinfo=1;
			break;
		case 'e':
			dodecode=1;
			break;
		case 'r':
			dorewrite=1;
			if (optarg && *optarg == '7')
				rwmode=RFC2045_RW_7BIT;
			if (optarg && *optarg == '8')
				rwmode=RFC2045_RW_8BIT;
			if (optarg && *optarg == 'U')
				rwmode=RFC2045_RW_8BIT_ALWAYS;
			break;
		case 'm':
			domimedigest=1;
			break;
		case 'd':
			dodsn=1;
			break;
		case 'D':
			dodsn=2;
			break;
		case 'x':
			do_extract=extract_file;
			if (optarg)
				extract_filename=optarg;
			break;
		case 'X':
			do_extract=extract_pipe;
			break;
		case 'V':
			dovalidate=1;
			break;
		case 'h':
			if (!optarg && argn < argc)
				optarg=argv[argn++];
			if (optarg)
			{
				decode_header=optarg;
			}
			dodecodehdr=1;
			break;
		case 'H':
			if (!optarg && argn < argc)
				optarg=argv[argn++];
			if (optarg)
			{
				decode_header=optarg;
			}
			dodecodeaddrhdr=1;
			break;
		case 'o':
			if (!optarg && argn < argc)
				optarg=argv[argn++];
			if (optarg)
			{
				decode_header=optarg;
			}
			doencodemime=1;
			break;
		case 'O':
			if (!optarg && argn < argc)
				optarg=argv[argn++];
			if (optarg)
			{
				decode_header=optarg;
			}
			doencodemimehdr=1;
			break;
		case 'u':
			convtoutf8=1;
			break;
		default:
			usage();
		}
	}

	defchset=mimecharset;

	rfc2045_setdefaultcharset(defchset);

	if (domimedigest)
	{
		mimedigest(argc-argn, argv+argn);
		return (0);
	}
	else if (dodecodehdr)
	{
		if (rfc822_display_hdrvalue("Subject",
					    decode_header,
					    mimecharset,
					    display_decoded_header,
					    NULL,
					    NULL) < 0)
		{
			perror("rfc822_display_hdrvalue");
			return (1);
		}

		printf("\n");
		return (0);
	}
	else if (dodecodeaddrhdr)
	{
		if (rfc822_display_hdrvalue("To",
					    decode_header,
					    mimecharset,
					    display_decoded_header,
					    NULL,
					    NULL) < 0)
		{
			perror("rfc822_display_hdrvalue");
			return (1);
		}

		printf("\n");
		return (0);
	}

	if (doencodemime)
	{
		char *s=rfc2047_encode_str(decode_header, mimecharset,
					   rfc2047_qp_allow_any);

		if (s)
		{
			printf("%s\n", s);
			free(s);
		}
		return (0);
	}
	if (doencodemimehdr)
	{
		struct rfc822t *t=rfc822t_alloc_new(decode_header, NULL, NULL);
		struct rfc822a *a=t ? rfc822a_alloc(t):NULL;
		char *s;

		if (a && (s=rfc2047_encode_header_addr(a, mimecharset)) != NULL)
		{
			printf("%s\n", s);
			free(s);
		}

		if (a) rfc822a_free(a);
		if (t) rfc822t_free(t);
		return (0);
	}

	p=read_message();

	if (doinfo)
	{
		mimesection = section ? strtok(section, ","):NULL;
		do {
			print_info(p, mimesection);
			if (do_extract)
				extract_section(p, mimesection,
						extract_filename, argc-argn,
						argv+argn, do_extract);
			if (mimesection)
				mimesection = strtok(NULL,",");
		} while (mimesection != NULL);
	}
	else if (dodecode)
	{
		mimesection = section ? strtok(section,","):NULL;
		do {
			print_decode(p, mimesection);
			if (mimesection)
				mimesection = strtok(NULL,",");
		} while (mimesection != NULL);
	}
	else if (dorewrite)
		rewrite(p, rwmode);
	else if (dodsn)
		dsn(p, dodsn == 2);
	else if (do_extract)
	{
		mimesection = section ? strtok(section, ","):NULL;
		do {
			extract_section(p, mimesection, extract_filename,
					argc-argn, argv+argn, do_extract);
			if (mimesection)
				mimesection = strtok(NULL,",");
		} while (mimesection != NULL);
	}
	else if (dovalidate)
	{
		rc=1;

		if (p->rfcviolation & RFC2045_ERR2COMPLEX)
			printf("ERROR: MIME complexity.\n");
		else if (p->rfcviolation & RFC2045_ERRBADBOUNDARY)
			printf("ERROR: Ambiguous  MIME boundary delimiters.\n");
		else rc=0;

	}
	else if (convtoutf8)
	{
		struct rfc2045src *src;
		struct rfc2045_decodemsgtoutf8_cb cb;

		memset(&cb, 0, sizeof(cb));

		cb.output_func=doconvtoutf8_stdout;
		cb.arg=NULL;

		src=rfc2045src_init_fd(0);

		if (src)
		{
			rfc2045_decodemsgtoutf8(src, p, &cb);
			rfc2045src_deinit(src);
		}
	}
	else
		print_structure(p);
	rfc2045_free(p);
	exit(rc);
	return (rc);
}

int main(int argc, char **argv)
{
	int	rc;

	rc=main2(unicode_default_chset(), argc, argv);
	return rc;
}
