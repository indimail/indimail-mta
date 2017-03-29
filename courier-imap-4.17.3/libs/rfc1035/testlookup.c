/*
** Copyright 1998 - 2011 Double Precision, Inc.
** See COPYING for distribution information.
*/

#include	"config.h"
#include	"rfc1035.h"
#include	"spf.h"
#include	<sys/types.h>
#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	<errno.h>
#include	<arpa/inet.h>

#include	"soxwrap/soxwrap.h"


static void setns(const char *p, struct rfc1035_res *res)
{
RFC1035_ADDR ia[4];
int	i=0;
char	*q=malloc(strlen(p)+1), *r;

	strcpy(q, p);
	for (r=q; (r=strtok(r, ", ")) != 0; r=0)
		if (i < 4)
		{
			if (rfc1035_aton(r, &ia[i]) == 0)
			{
				++i;
			}
			else
			{
				fprintf(stderr, "%s: invalid IP address\n",
					r);
			}
		}
	rfc1035_init_ns(res, ia, i);
}

extern char rfc1035_spf_gettxt(const char *current_domain,
			       char *buf);
extern char rfc1035_spf_gettxt_n(const char *current_domain,
			  char **buf);


static void spflookup(const char *current_domain)
{
	char *buf;

	switch (rfc1035_spf_gettxt_n(current_domain, &buf)) {
	case SPF_NONE:
		printf("none\n");
		return;
	case SPF_NEUTRAL:
		printf("neutral\n");
		return;
	case SPF_PASS:
		printf("pass: %s\n", buf);
		return;
	case SPF_FAIL:
		printf("fail\n");
		return;
	case SPF_SOFTFAIL:
		printf("softfail\n");
		return;
	case SPF_ERROR:
		printf("error\n");
		return;
	default:
		printf("unknown\n");
	}
}

int main(int argc, char **argv)
{
struct  rfc1035_res res;
struct	rfc1035_reply *replyp;
int	argn;
const char *q_name;
int	q_type;
int	q_class;
int	q_xflag=0;
int	q_rflag=0;
char	ptrbuf[RFC1035_MAXNAMESIZE+1];

	rfc1035_init_resolv(&res);

	argn=1;
	while (argn < argc)
	{
		if (argv[argn][0] == '@')
		{
			setns(argv[argn]+1, &res);
			++argn;
			continue;
		}

		if (strcmp(argv[argn], "-x") == 0)
		{
			q_xflag=1;
			++argn;
			continue;
		}
		if (strcmp(argv[argn], "-r") == 0)
		{
			q_rflag=1;
			++argn;
			continue;
		}

		if (strcmp(argv[argn], "-dnssec") == 0)
		{
			rfc1035_init_dnssec_enable(&res, 1);
			++argn;
			continue;
		}

		if (strcmp(argv[argn], "-udpsize") == 0)
		{
			++argn;

			if (argn < argc)
			{
				rfc1035_init_edns_payload(&res,
							  atoi(argv[argn]));
				++argn;
			}
			continue;
		}

		break;
	}

	if (argn >= argc)	exit(0);

	q_name=argv[argn++];

	if (q_xflag)
	{
	struct in_addr ia;
#if	RFC1035_IPV6
	struct in6_addr ia6;

		if (inet_pton(AF_INET6, q_name, &ia6) > 0)
		{
		const char *sin6=(const char *)&ia6;
		unsigned i;

			ptrbuf[0]=0;

			for (i=sizeof(struct in6_addr); i; )
			{
			char    buf[10];

				--i;
				sprintf(buf, "%x.%x.",
					(int)(unsigned char)(sin6[i] & 0x0F),
					(int)(unsigned char)((sin6[i] >> 4)
							& 0x0F));
				strcat(ptrbuf, buf);
			}
			strcat(ptrbuf, "ip6.arpa");
			q_name=ptrbuf;
		}
		else
#endif
		if ( rfc1035_aton_ipv4(q_name, &ia) == 0)
		{
		char buf[RFC1035_MAXNAMESIZE];
		unsigned char a=0,b=0,c=0,d=0;
		const char *p=buf;

			rfc1035_ntoa_ipv4(&ia, buf);

			while (*p >= '0' && *p <= '9')
				a= (int)a * 10 + (*p++ - '0');
			if (*p)	p++;
			while (*p >= '0' && *p <= '9')
				b= (int)b * 10 + (*p++ - '0');
			if (*p)	p++;
			while (*p >= '0' && *p <= '9')
				c= (int)c * 10 + (*p++ - '0');
			if (*p)	p++;
			while (*p >= '0' && *p <= '9')
				d= (int)d * 10 + (*p++ - '0');

			sprintf(ptrbuf, "%d.%d.%d.%d.in-addr.arpa",
				(int)d, (int)c, (int)b, (int)a);
			q_name=ptrbuf;
		}
	}

	if (q_rflag)
	{
	RFC1035_ADDR a;
	int	rc;

		if (rfc1035_aton(q_name, &a) == 0)
		{
			rc=rfc1035_ptr(&res, &a,ptrbuf);
			if (rc == 0)
			{
				printf("%s\n", ptrbuf);
				exit(0);
			}
		}
		else
		{
		RFC1035_ADDR	*aptr;
		unsigned alen;

			rc=rfc1035_a(&res, q_name, &aptr, &alen);
			if (rc == 0)
			{
			unsigned i;

				for (i=0; i<alen; i++)
				{
					rfc1035_ntoa(&aptr[i], ptrbuf);
					printf("%s\n", ptrbuf);
				}
				exit(0);
			}
		}
		fprintf(stderr, "%s error.\n", errno == ENOENT ? "Hard":"Soft");
		exit(1);
	}
	
	q_type= -1;

	if (argn < argc)
	{
		if (strcmp(argv[argn], "spf") == 0)
			q_type= -2;
		else
			q_type=rfc1035_type_strtoi(argv[argn++]);
	}

	if (q_type == -1)
		q_type=q_xflag ? RFC1035_TYPE_PTR:RFC1035_TYPE_ANY;

	q_class= -1;
	if (argn < argc)
		q_class=rfc1035_class_strtoi(argv[argn]);
	if (q_class < 0)
		q_class=RFC1035_CLASS_IN;

	if (q_type == -2)
	{
		spflookup(q_name);
		exit(0);
	}

	replyp=rfc1035_resolve(&res, RFC1035_OPCODE_QUERY,
			       q_name, q_type, q_class);

	if (!replyp)
	{
		perror(argv[0]);
		exit(1);
	}

	rfc1035_dump(replyp, stdout);
	rfc1035_replyfree(replyp);
	rfc1035_destroy_resolv(&res);
	return (0);
}
