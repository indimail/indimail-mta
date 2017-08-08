/*
** Copyright 2011 Double Precision, Inc.
** See COPYING for distribution information.
*/

#include	"rfc3676parser.h"

#include	<stdlib.h>
#include	<stdio.h>


static int line_begin(size_t quote_level, void *arg)
{
	printf("[%d: ", (int)quote_level);
	return 0;
}

static int line_contents(const char32_t *txt,
			 size_t txt_size,
			 void *arg)
{
	while (txt_size--)
		putchar(*txt++);
	return 0;
}

static int line_flowed_notify(void *arg)
{
	printf("...");
	return 0;
}

static int line_end(void *arg)
{
	printf("]\n");
	return 0;
}

int main(int argc, char **argv)
{
	struct rfc3676_parser_info info;
	int n=0;
	char buf[BUFSIZ];
	rfc3676_parser_t parser;

	if (argc > 1)
		n=atoi(argv[1]);

	memset(&info, 0, sizeof(info));

	info.charset="utf-8";

	info.isflowed=n != 0;
	info.isdelsp= n == 2;

	info.line_begin=line_begin;
	info.line_contents=line_contents;
	info.line_flowed_notify=line_flowed_notify;
	info.line_end=line_end;

	if ((parser=rfc3676parser_init(&info)) != NULL)
	{
		while (fgets(buf, sizeof(buf), stdin))
			rfc3676parser(parser, buf, strlen(buf));
		rfc3676parser_deinit(parser, NULL);
		printf("\n");
	}

	return (0);
}
