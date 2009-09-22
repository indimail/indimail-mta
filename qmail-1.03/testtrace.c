/*
 * $Log: $
 */
/*
 * $Id: testtrace.c,v 1.1 2005/06/16 19:37:03 domainkeys Exp $ 
 */

#include "dktrace.h"

main()
{
	DK_TRACE        t;
	char            buf[2000];

	dkt_init(&t);
	dkt_add(&t, DKT_RAW_HEADER, "1234561111", 10);
	dkt_add(&t, DKT_RAW_HEADER, "1234561111", 10);
	dkt_add(&t, DKT_CANON_HEADER, "abcdefg", 7);
	dkt_add(&t, DKT_RAW_BODY, "\n\raline", 7);
	dkt_add(&t, DKT_CANON_BODY, "", 0);

	dkt_generate(&t, DKT_RAW_HEADER, buf, sizeof (buf));
	puts(buf);
	dkt_generate(&t, DKT_CANON_HEADER, buf, sizeof (buf));
	puts(buf);
	dkt_generate(&t, DKT_RAW_BODY, buf, sizeof (buf));
	puts(buf);
	dkt_generate(&t, DKT_CANON_BODY, buf, sizeof (buf));
	puts(buf);
	exit(0);
}
