/*
** Copyright 1998 - 2000 Double Precision, Inc.
** See COPYING for distribution information.
*/

#include	"rfc1035.h"
#include	<stdlib.h>
#include	<arpa/inet.h>


static void print_hostname(FILE *f, const char *p)
{
	for ( ; *p; p++)
	{
		if (*p < ' ' || *p >= 127)
		{
			fprintf(f, "\\%03o", (int)(unsigned char)*p);
			continue;
		}

		if (*p == '\\')
		{
			fprintf(f, "\\\\");
			continue;
		}
		putc(*p, f);
	}
}

static void tostr_callback(const char *ptr, void *vp)
{
	FILE *fp=*(FILE **)vp;

	fprintf(fp, "%s", ptr);
}

void rfc1035_dump(struct rfc1035_reply *r, FILE *f)
{
unsigned n;
char namebuf[RFC1035_MAXNAMESIZE+1];
char timebuf[RFC1035_MAXTIMEBUFSIZE+1];
char	ipbuf[RFC1035_NTOABUFSIZE];
struct	rfc1035_reply *qr;

	fprintf(f, ";;HEADER");

	rfc1035_ntoa((const RFC1035_ADDR *)&r->server_addr, ipbuf);

	fprintf(f, " (server %s)", ipbuf);

	fprintf(f, ":\n;;  Bytes: %ld\n", 
		(long)r->replylen);

	fprintf(f, ";;  Opcode: %s\n", 
		rfc1035_opcode_itostr(r->opcode));
	fprintf(f, ";;  Flags:");
	if (r->qr)
		fprintf(f, " qr");
	if (r->aa)
		fprintf(f, " aa");
	if (r->tc)
		fprintf(f, " tc");
	if (r->rd)
		fprintf(f, " rd");
	if (r->ra)
		fprintf(f, " ra");
	if (r->ad)
		fprintf(f, " ad");
	if (r->cd)
		fprintf(f, " cd");
	fprintf(f, "\n;;  Status: %s\n", rfc1035_rcode_itostr(r->rcode));
	fprintf(f, ";;  # questions: %u\n", r->qdcount);
	fprintf(f, ";;  # answers: %u\n", r->ancount);
	fprintf(f, ";;  # authoritative: %u\n", r->nscount);
	fprintf(f, ";;  # additional: %u\n", r->arcount);

	fprintf(f, ";;\n;;QUESTIONS:\n");
	for (n=0; n<r->qdcount; n++)
	{
		fprintf(f, ";;  ");
		print_hostname(f, rfc1035_replyhostname(r, r->qdptr[n].name,
							namebuf));
		fprintf(f,".\t%s ",
			rfc1035_class_itostr(r->qdptr[n].qclass));

		rfc1035_type_itostr(r->qdptr[n].qtype, tostr_callback, &f);

		fprintf(f, "\n");
	}

	fprintf(f, "\n;;ANSWERS:\n");
	for (qr=r; qr; qr=qr->next)
	{
		for (n=0; n<qr->ancount; n++)
		{
			char	*c;

			fprintf(f, " ");
			print_hostname(f,
				       rfc1035_replyhostname(qr,
							     qr->anptr[n]
							     .rrname,
							     namebuf));
			fprintf(f, ".\t%s\t%s ",
				rfc1035_fmttime(qr->anptr[n].ttl, timebuf),
				rfc1035_class_itostr(qr->anptr[n].rrclass));

			rfc1035_type_itostr(qr->anptr[n].rrtype, tostr_callback,
					    &f);

			c=rfc1035_dumprrdata(qr, qr->anptr+n);
			if (c)
			{
				fprintf(f, "\t%s", c);
				free(c);
			}
			fprintf(f, "\n");
		}
	}
	fprintf(f, "\n;;AUTHORITATIVE:\n");
	for (n=0; n<r->nscount; n++)
	{
		char	*c;

		fprintf(f, " ");
		print_hostname(f, rfc1035_replyhostname(r,
							r->nsptr[n].rrname,
							namebuf));
		fprintf(f, ".\t%s\t%s ",
			rfc1035_fmttime(r->nsptr[n].ttl, timebuf),
			rfc1035_class_itostr(r->nsptr[n].rrclass));

		rfc1035_type_itostr(r->nsptr[n].rrtype, tostr_callback, &f);

		c=rfc1035_dumprrdata(r, r->nsptr+n);
		if (c)
		{
			fprintf(f, "\t%s", c);
			free(c);
		}
		fprintf(f, "\n");
	}

	fprintf(f, "\n;;ADDITIONAL:\n");
	for (n=0; n<r->arcount; n++)
	{
		char	*c;

		if (r->arptr[n].rrtype == RFC1035_TYPE_OPT)
			continue;

		fprintf(f, " ");
		print_hostname(f, rfc1035_replyhostname(r,
							r->arptr[n].rrname,
							namebuf));
		fprintf(f, ".\t%s\t%s ",
			rfc1035_fmttime(r->arptr[n].ttl, timebuf),
			rfc1035_class_itostr(r->arptr[n].rrclass));

		rfc1035_type_itostr(r->arptr[n].rrtype, tostr_callback, &f);

		c=rfc1035_dumprrdata(r, r->arptr+n);
		if (c)
		{
			fprintf(f, "\t%s", c);
			free(c);
		}
		fprintf(f, "\n");
	}
}
