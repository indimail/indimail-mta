/*
 * $Log: dktest.c,v $
 * Revision 1.17  2019-06-07 11:25:58+05:30  Cprogrammer
 * replaced getopt() with subgetopt()
 *
 * Revision 1.16  2013-08-17 16:01:08+05:30  Cprogrammer
 * added case for duplicate DomainKey-Signature header
 *
 * Revision 1.15  2011-07-29 09:28:09+05:30  Cprogrammer
 * fixed gcc 4.6 warnings
 *
 * Revision 1.14  2011-07-22 14:39:27+05:30  Cprogrammer
 * added -D option to specify d= tag
 *
 * Revision 1.13  2009-12-10 15:04:09+05:30  Cprogrammer
 * exit with DK_STAT
 *
 * Revision 1.12  2009-04-20 22:20:55+05:30  Cprogrammer
 * fixed compilation warning
 *
 * Revision 1.11  2009-04-16 22:39:33+05:30  Cprogrammer
 * assign from = dk_from()
 *
 * Revision 1.10  2009-04-05 12:51:22+05:30  Cprogrammer
 * added preprocessor warning
 *
 * Revision 1.9  2009-03-14 16:28:22+05:30  Cprogrammer
 * Fixed dktest.c to check for DK_STAT_GRANULARITY
 *
 * Revision 1.8  2009-03-14 08:50:41+05:30  Cprogrammer
 * Added -h option to dktest to add h= tag when signing
 * Added -r option to dktest to enable ignoring duplicate headers when signing (implies -h)
 * Added -T option to dktest to enable generation of trace headers
 * Added -d option to fetch dns text record for domainkey
 *
 * Revision 1.7  2005-08-23 17:15:02+05:30  Cprogrammer
 * gcc 4 compliance
 *
 * Revision 1.6  2005-04-25 22:52:45+05:30  Cprogrammer
 * removed printing of comments
 *
 * Revision 1.5  2005-04-01 19:54:27+05:30  Cprogrammer
 * libdomainkeys-0.64
 *
 * Revision 1.4  2004-10-25 14:54:02+05:30  Cprogrammer
 * libdomainkeys-0.63
 *
 * Revision 1.3  2004-10-22 20:24:18+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.2  2004-10-20 20:01:58+05:30  Cprogrammer
 * upgrade to libdomainkeys-0.62
 *
 * Revision 1.1  2004-09-22 23:30:32+05:30  Cprogrammer
 * Initial revision
 *
 */
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include "domainkeys.h"
#include "sgetopt.h"

#ifdef DOMAIN_KEYS
int             optf = 0;

void
errorout(DK *dk, DK_STAT st)
{
	if (optf && dk)
		fprintf(stderr, "%s(%d):", dk_errfile(dk), dk_errline(dk));
	fprintf(stderr, "dktest: %s\n", DK_STAT_to_string(st));
	exit(st);
}

int
main(int argc, char *argv[])
{
	char            inbuf[1024];
	char            advice[2048];
	char            trace_count[BUFSIZ];
	size_t          inlen;
	size_t          advicelen = sizeof(advice);
	DK             *dk;
	DK_LIB         *dklib;
	DK_STAT         st, dkt_st;
	signed char     ch;
	int             opts = 0, optv = 0, optt = 0, opth = 0, optr = 0, optT = 0,
					optc = DK_CANON_SIMPLE;
	char           *canon = "simple";
	char           *keyfn  = 0, *selector  = 0;
	char           *txtrec, *cp, *from, *dkdomain;
	char            privkey[2048];
	FILE           *privkeyf = 0;
	size_t          privkeylen;
	DK_FLAGS        dkf = 0;
	int             i;
	DK_TRACE_TYPE   dk_trace_tag[4] = {
		DKT_RAW_HEADER,
		DKT_CANON_HEADER,
		DKT_RAW_BODY,
		DKT_CANON_BODY
	};

	for (dkdomain = (char *) 0;;)
	{
		if ((ch = getopt(argc, argv, "s:vt:fb:c:hrTd:D:")) == opteof)
			break;
		switch (ch)
		{
		case 'D':
			dkdomain = optarg;
			break;
		case 'd': /*- optD */
			txtrec = dns_text(optarg);
			cp = txtrec;
			printf("%ld\n", (long) strlen(cp));
			while (*cp) {
				printf("%02x ", *cp++);
				if ((cp - txtrec) % 16 == 0)
					printf("\n");
			}
			printf("\n");
			if (!strcmp(txtrec, "e=perm;"))
				exit(0);
			if (!strcmp(txtrec, "e=temp;"))
				exit(0);
			free(txtrec);
			return (0);
			break;
		case 'T':
			optT = 1;
			break;
		case 'v':
			optv = 1;
			break;
		case 'f':
			optf = 1;
			break;
		case 'r':
			optr = 1;
			opth = 1;
			break;
		case 's':
			opts = 1;
			keyfn = optarg;
			selector = optarg;
			while (*optarg)
			{
				if (*optarg == '/')
					selector = optarg + 1;
				optarg++;
			}
			break;
		case 't':
			optt = atoi(optarg);
			break;
		case 'h':
			opth = 1;
			break;
		case 'b':
			advicelen = atoi(optarg);
			if (advicelen > sizeof(advice))
				advicelen = sizeof(advice);
			break;
		case 'c':
			if (!strcmp(optarg, "simple"))
				optc = DK_CANON_SIMPLE, canon = "simple";
			else
			if (!strcmp(optarg, "nofws"))
				optc = DK_CANON_NOFWS, canon = "nofws";
			else
			{
				fprintf(stderr, "dktest: unrecognized canonicalization.\n");
				exit(1);
			}
		}
	}
	if (opts)
	{
		if (!(privkeyf = fopen(keyfn, "r"))) /*- TC10 */
		{
			fprintf(stderr, "dktest: can't open private key file %s\n", keyfn);
			exit(1);
		}
		if ((privkeylen = fread(privkey, 1, sizeof(privkey), privkeyf)) == sizeof(privkey))
					/*- TC9 */
		{					
			fprintf(stderr, "dktest: private key buffer isn't big enough, use a smaller private key or recompile.\n");
			exit(1);
		}
		privkey[privkeylen] = '\0';
		fclose(privkeyf);
	}
	if (optt == 1)
		errorout(NULL, 0);		/*- TC2 */
	if (optt == 2)
		errorout(NULL, 32767);	/*- TC3 */
	dklib = dk_init(&st);
	if (st != DK_STAT_OK)
		errorout(NULL, st);
	if (!dklib)
		errorout(NULL, 200);
	if (optv)
	{
		dk = dk_verify(dklib, &st);
		if (st != DK_STAT_OK)
			errorout(dk, st);
	} else
	if (opts)
	{
		dk = dk_sign(dklib, &st, optc);
		if (st != DK_STAT_OK)
			errorout(dk, st);
		if (optr)
			st = dk_setopts(dk, DKOPT_RDUPE);
		if (st != DK_STAT_OK)
			errorout(dk, st);
	} else
	{
		fprintf(stderr, "dktest: [-f] [-b#] [-c nofws|simple] [-v|-s selector] [-h] [-t#] [-r] [-T][-d dnsrecord]\n"); /* TC1 */
		exit(1);
	}
	if (optT) /*- trace */
	{
		st = dk_setopts(dk, (DKOPT_TRACE_h|DKOPT_TRACE_H|DKOPT_TRACE_b|DKOPT_TRACE_B));
		if (st != DK_STAT_OK)
			errorout(dk, st);
	}
	if (optt == 3)
		errorout(dk, dk_message(NULL, (const unsigned char *) "", 1));	/*- TC4 */
	if (optt == 4)
		errorout(dk, dk_message(dk, (const unsigned char *) NULL, 1));	/*- TC5 */
	if (optt == 5)
		errorout(dk, dk_message(dk, (const unsigned char *) "", 0));	/*- TC6 */
	if (optt >= 100 && optt <= 140)
		errorout(dk, optt - 100);	/*- TC53 */
	st = DK_STAT_OK;
	/* 
	 * This should work with DOS or UNIX text files -Tim
	 * Reduced calls to dk_message, in lib dkhash called for EVERY char
	 * DOS formatted input (CRLF line terminated) will have fewer calls
	 * to dk_message() than UNIX (LF line terminated) input.
	 */
	while (1)
	{
		char           *inp;

		inlen = fread(inbuf, 1, sizeof(inbuf), stdin);
		inp = inbuf;
		while (inlen--)
		{
			if (*inp == '\n')
				st = dk_message(dk, (const unsigned char *) "\r\n", 2);
			else
				st = dk_message(dk, (const unsigned char *) inp, 1);
			if (st != DK_STAT_OK)
				break;  //stop looping if there was an error
			inp++;
		}
		if ((inp-inbuf < sizeof(inbuf)) || (st != DK_STAT_OK))
			break; /*- if we read in the entire message or encountered an error */
	}
	if (st == DK_STAT_OK)
	{
		if (optt == 10)
			st = dk_end(dk, &dkf);
		else
			st = dk_eom(dk, &dkf);
	}
	if (optT)
	{
		printf("DomainKey-Trace: U=http://domainkeys.sourceforge.net; V=TESTING;\n");
		for (i = 0; i < 4; i++)
		{
			if (dk_get_trace(dk, dk_trace_tag[i], trace_count, sizeof (trace_count)) != DK_STAT_OK)
			{
				fprintf(stderr, "dktest: Not enough resources for trace buffer output\n");
				break;
			} else
				printf("  %s\n", trace_count);
		}
		if (optv)
		{
			printf("DomainKey-Trace-Diff:\n");
			for (i = 0; i < 4; i++) {
				dkt_st = dk_compare_trace(dk,dk_trace_tag[i],trace_count,sizeof(trace_count));
				if (dkt_st == DK_STAT_NOSIG)
				{
					printf("  No DK-Trace: header found\n");
					break;
				} else
				if (dkt_st != DK_STAT_OK)
				{
					fprintf(stderr,"dktest: Not enough resources for trace buffer output\n");
					break;
				} else
					printf("  %s\n",trace_count);
			}
		}
	}
	if ((optt == 6 || optt == 10) && optv)
	{
		printf("flags: ");
		if (dkf & DK_FLAG_SET)
			printf("+");
		if (dkf & DK_FLAG_TESTING)
			printf("t");
		if (dkf & DK_FLAG_SIGNSALL)
			printf("s"); /*- wont be set if dk_end() is sucessful */
		if (dkf & DK_FLAG_G)
			printf("g");
		printf("\n");
	} else
	if (optt == 6 && opts)
		errorout(dk, dk_getsig(dk, NULL, NULL, advicelen));	/*- TC14 */
	else
	if (optt == 7)
	{
		from = dk_from(dk);
		if (!from)
			from = "";
		printf("%s\n", from);	/*- TC14-1, TC14-2 */
	} else
	if (optt == 11)
	{
		from = dk_address(dk);
		printf("%s\n", from);	/*- TC14-3, TC14-4 */
	} else
	if (optt == 9)
	{
		char           *s;

		s = malloc(dk_headers(dk, NULL));
		dk_headers(dk, s);
		printf("%s\n", s);
		free(s);
	} else
	if (optt == 8 && opts)
	{
		dk_getsig(dk, privkey, (unsigned char *) advice, advicelen);
		if (st != DK_STAT_OK)
			errorout(dk, st);
		printf("%d %d\n", (int) dk_siglen(privkey), (int) strlen(advice)); /*- TC39 */
	} else
	if (opts)
	{
		if (st != DK_STAT_OK)
			errorout(dk, st);
		st = dk_getsig(dk, privkey, (unsigned char *) advice, advicelen);
		if (st != DK_STAT_OK)
			errorout(dk, st);
#if 0
		printf("Comment: DomainKeys? See http://antispam.yahoo.com/domainkeys\n");
#endif
		from = dk_from(dk);
		printf("DomainKey-Signature: a=rsa-sha1; q=dns; c=%s;\n"
			"  s=%s; d=%s;\n" "  b=%s;\n", canon, selector, dkdomain ? dkdomain : from, advice);
		if (opth == 1)
		{
			if (dk_headers(dk, NULL) < sizeof(inbuf))
			{
				dk_headers(dk, inbuf);
				printf("  h=%s;\n", inbuf);
			}
		}
	} else
	if (optv)
	{
		char           *status = 0;

		switch (st)
		{
		case DK_STAT_OK:
			status = "good";
			break;
		case DK_STAT_BADSIG:
			status = "bad";
			break;
		case DK_STAT_NOSIG:
			status = "no signature";
			break;
		case DK_STAT_NOKEY:
		case DK_STAT_CANTVRFY:
			status = "no key";
			break;
		case DK_STAT_BADKEY:
			status = "bad key";
			break;
		case DK_STAT_INTERNAL:
		case DK_STAT_ARGS:
		case DK_STAT_SYNTAX:
			status = "bad format";
			break;
		case DK_STAT_NORESOURCE:
			status = "no resources";
			break;
		case DK_STAT_REVOKED:
			status = "revoked";
			break;
		case DK_STAT_GRANULARITY:
			status = "bad sender (g=)";
			break;
		case DK_STAT_DUPLICATE:
			status = "duplicate signature";
			break;
		}
#if 0
		printf("Comment: DomainKeys? See http://antispam.yahoo.com/domainkeys\n");
#endif
		printf("DomainKey-Status: %s\n", status);
		rewind(stdin);
	}
	if (st != DK_STAT_OK)
		errorout(dk, st);
	dk_free(dk, 1);//cleanup properly (not really necessary for single run process)
	dk_shutdown(dklib);
	return(0);
}
#else
#warning "not compiled with -DDOMAIN_KEYS"
int
main(int argc, char *argv[])
{
	fprintf(stderr, "not compiled with -DDOMAIN_KEYS\n");
	return(1);
}
#endif

void
getversion_dktest_c()
{
	static char    *x = "$Id: dktest.c,v 1.17 2019-06-07 11:25:58+05:30 Cprogrammer Exp mbhangui $";

	x++;
}
