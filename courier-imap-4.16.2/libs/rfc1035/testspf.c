/*
** Copyright 2004 Double Precision, Inc.
** See COPYING for distribution information.
*/

#include	"config.h"
#include	"rfc1035.h"
#include	"spf.h"
#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>


static struct testsuite_s {
	const char *mailfrom;
	const char *tcpremoteip;
	const char *tcpremotehost;
	const char *helodomain;
	const char *mydomain;
} testsuite[]={
	{"spf1.email-scan.com","192.168.1.10","spf1.email-scan.com","spf1","example.com"},
	{"spf2.email-scan.com","192.168.1.10","spf1.email-scan.com","spf1","example.com"},
	{"spf3.email-scan.com","192.168.1.10","spf1.email-scan.com","spf1","example.com"},
	{"spf3.email-scan.com","191.168.2.10","spf1.email-scan.com","spf3.test","example.com"},
	{"spf3.email-scan.com","1234:5678::9ABC","spf1.email-scan.com","spf1","example.com"},
	{"spf4.email-scan.com","192.168.2.10","spf1.email-scan.com","10-1-168-192","example.com"},
	{"spf5.email-scan.com","::ffff:192.168.1.0","spf5.email-scan.com","helo","example.com"},
	{"spf5.email-scan.com","::ffff:192.168.1.1","spf5.email-scan.com","helo","example.com"},
	{"spf6.email-scan.com","::ffff:192.168.1.0","spf5.email-scan.com","helo","example.com"},
	{"spf6.email-scan.com","::ffff:192.168.1.1","spf5.email-scan.com","helo","example.com"},
	{"spf7.email-scan.com","::ffff:192.168.1.1","spf5.email-scan.com","helo","example.com"},
	{"spf7.email-scan.com","::ffff:192.168.2.1","spf5.email-scan.com","helo","example.com"},
	{"spf7.email-scan.com","::ffff:192.168.1.255","spf5.email-scan.com","helo","example.com"},
	{"spf8.email-scan.com","::ffff:192.168.1.1","spf5.email-scan.com","helo","example.com"},
	{"spf8.email-scan.com","::ffff:192.168.2.1","spf5.email-scan.com","helo","example.com"},
	{"spf8.email-scan.com","::ffff:192.168.1.255","spf5.email-scan.com","helo","example.com"},
	{"spf9.email-scan.com","::ffff:192.168.1.130","spf5.email-scan.com","helo","example.com"},
	{"spf9.email-scan.com","::ffff:192.168.1.129","spf5.email-scan.com","helo","example.com"},
	{"spf9.email-scan.com","::ffff:192.168.1.10","spf5.email-scan.com","helo","example.com"},
	{"spf10.email-scan.com","::ffff:192.168.1.10","spf10.email-scan.com","helo","example.com"},
	{"spf10.email-scan.com","::ffff:192.168.1.50","spf10.email-scan.com","helo","example.com"},
	{"spf11.email-scan.com","::ffff:192.168.0.1","spf10.email-scan.com","helo","example.com"},
	{"spf11.email-scan.com","::ffff:192.168.0.2","spf10.email-scan.com","helo","example.com"},
	{"spf12.email-scan.com","::ffff:192.168.0.1","spf10.email-scan.com","helo","example.com"},
	{"spf12.email-scan.com","::ffff:192.168.1.1","spf10.email-scan.com","helo","example.com"},
	{"spf13.email-scan.com","::ffff:192.168.1.1","spf10.email-scan.com","spf1","example.com"},
	{"spf13.email-scan.com","::ffff:192.168.1.1","spf10.email-scan.com","spf50","example.com"},
	{"spf14.email-scan.com","::ffff:192.168.2.0","spf10.email-scan.com","helo","example.com"},
	{"spf14.email-scan.com","::ffff:192.168.2.1","spf10.email-scan.com","helo","example.com"},
	{"spf14.email-scan.com","::ffff:192.168.2.1","spf10.email-scan.com","spf11","example.com"},
	{"spf14.email-scan.com","::ffff:192.168.0.1","spf10.email-scan.com","spf11","example.com"},
	{"spf15.email-scan.com","::ffff:192.168.0.1","spf10.email-scan.com","spf11","example.com"}
};

static int testspf(const char *mailfrom,
		   const char *tcpremoteip,
		   const char *tcpremotehost,
		   const char *helodomain,
		   const char *mydomain)
{
	char buf[256];

	switch (rfc1035_spf_lookup(mailfrom, tcpremoteip, tcpremotehost,
				   helodomain, mydomain, buf, sizeof(buf))) {
	case SPF_NONE:
		printf("none\n");
		break;
	case SPF_NEUTRAL:
		printf("neutral\n");
		break;
	case SPF_PASS:
		printf("pass\n");
		break;
	case SPF_FAIL:
		printf("fail: %s\n", buf);
		break;
	case SPF_SOFTFAIL:
		printf("softfail\n");
		break;
	case SPF_ERROR:
		printf("error\n");
		break;
	default:
		printf("unknown\n");
	}
	return 0;
}

int main(int argc, char **argv)
{
	if (argc == 2 && strncmp(argv[1], "-test=", 6) == 0)
	{
		int loop_cnt=atoi(argv[1]+6);
		int i;

		for (i=0;i<loop_cnt;i++)
		{
			int j;

			for (j=0; j<sizeof(testsuite)/sizeof(testsuite[0]);
			     j++)
				testspf(testsuite[j].mailfrom,
					testsuite[j].tcpremoteip,
					testsuite[j].tcpremotehost,
					testsuite[j].helodomain,
					testsuite[j].mydomain);
		}
		return 0;
	}

	if (argc < 6)
	{
		printf("Usage: %s mailfrom remoteip remotehost helo me\n",
		       argv[0]);
		exit(1);
	}

	return testspf(argv[1], argv[2], argv[3], argv[4], argv[5]);
}

