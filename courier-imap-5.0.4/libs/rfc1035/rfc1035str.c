/*
** Copyright 1998 - 1999 Double Precision, Inc.
** See COPYING for distribution information.
*/

#include	"rfc1035.h"
#include	<string.h>


static struct { const char *name; int num; } typetab[]={
	{"A",		1},
	{"NS",		2},
	{"MD",		3},
	{"MF",		4},
	{"CNAME",	5},
	{"SOA",		6},
	{"MB",		7},
	{"MG",		8},
	{"MR",		9},
	{"NULL",	10},
	{"WKS",		11},
	{"PTR",		12},
	{"HINFO",	13},
	{"MINFO",	14},
	{"MX",		15},
	{"TXT",		16},
	{"AAAA",	28},
	{"RRSIG",	46},
	{"AXFR",	252},
	{"MAILB",	253},
	{"MAILA",	254},
	{"ANY",		255}},

	classtab[]={

	{"IN",		1},
	{"CSNET",	2},
	{"CHAOS",	3},
	{"HESIOD",	4},
	{"ANY",		255}},

	opcodetab[]={
	{"QUERY",	0},
	{"IQUERY",	1},
	{"STATUS",	2}},

	rcodetab[]={
	{"NOERROR",	0},
	{"FORMAT",	1},
	{"SERVFAIL",	2},
	{"NXDOMAIN",	3},
	{"UNIMPLEMENTED", 4},
	{"REFUSED",	5}};

#if HAVE_STRCASECMP
#define	COMPARE(a,b)	strcasecmp((a), (b))
#else
#define	COMPARE(a,b)	stricmp((a), (b))
#endif

void rfc1035_type_itostr(int n, void (*func)(const char *, void *), void *arg)
{
	unsigned i;
	char buf[30];

	for (i=0; i<sizeof(typetab)/sizeof(typetab[0]); i++)
		if (typetab[i].num == n)
		{
			(*func)(typetab[i].name, arg);
			return;
		}

	snprintf(buf, sizeof(buf), "TYPE%d", n);

	buf[sizeof(buf)-1]=0;

	(*func)(buf, arg);
}

int rfc1035_type_strtoi(const char *n)
{
unsigned i;

	for (i=0; i<sizeof(typetab)/sizeof(typetab[0]); i++)
		if (COMPARE(typetab[i].name, n) == 0) return (typetab[i].num);
	return (-1);
}

const char *rfc1035_class_itostr(int n)
{
unsigned i;

	for (i=0; i<sizeof(classtab)/sizeof(classtab[0]); i++)
		if (classtab[i].num == n) return (classtab[i].name);
	return ("unknown");
}

int rfc1035_class_strtoi(const char *n)
{
unsigned i;

	for (i=0; i<sizeof(classtab)/sizeof(classtab[0]); i++)
		if (COMPARE(classtab[i].name, n) == 0) return (classtab[i].num);
	return (-1);
}

const char *rfc1035_opcode_itostr(int n)
{
unsigned i;

	for (i=0; i<sizeof(opcodetab)/sizeof(opcodetab[0]); i++)
		if (opcodetab[i].num == n) return (opcodetab[i].name);
	return ("unknown");
}

int rfc1035_opcode_strtoi(const char *n)
{
unsigned i;

	for (i=0; i<sizeof(opcodetab)/sizeof(opcodetab[0]); i++)
		if (COMPARE(opcodetab[i].name, n) == 0)
				return (opcodetab[i].num);
	return (-1);
}

const char *rfc1035_rcode_itostr(int n)
{
unsigned i;

	for (i=0; i<sizeof(rcodetab)/sizeof(rcodetab[0]); i++)
		if (rcodetab[i].num == n) return (rcodetab[i].name);
	return ("unknown");
}

int rfc1035_rcode_strtoi(const char *n)
{
unsigned i;

	for (i=0; i<sizeof(rcodetab)/sizeof(rcodetab[0]); i++)
		if (COMPARE(rcodetab[i].name, n) == 0)
				return (rcodetab[i].num);
	return (-1);
}
