/*
** Copyright 2011 Double Precision, Inc.
** See COPYING for distribution information.
**
*/

#include	"unicode_config.h"
#include	"courier-unicode.h"
#include	<string.h>
#include	<stdlib.h>
#include	<stdio.h>
#include	<errno.h>

struct collect_buf {
	char *ptr;
	size_t cnt;
	size_t size;
};

static int save_output(const char *p, size_t n, void *ptr)
{
	struct collect_buf *cb=(struct collect_buf *)ptr;

	while (n)
	{
		if (cb->cnt < cb->size)
			cb->ptr[cb->cnt++]=*p;
		++p;
		--n;
	}
	return 0;
}

static void test1()
{
	static const char teststr[]= {
		0x00, 0x00, 0x00, 0x41,
		0x00, 0x00, 0x04, 0x14,
		0x00, 0x00, 0x04, 0x30,
		0x00, 0x00, 0x00, 0x42};
	char outputbuf[12];
	struct collect_buf cb;
	unicode_convert_handle_t h;
	int checkflag;

	cb.ptr=outputbuf;
	cb.cnt=0;
	cb.size=sizeof(outputbuf);

	if ((h=unicode_convert_init("UCS-4BE", "ISO-8859-1",
				      save_output, &cb)) == NULL)
	{
		perror("unicode_convert_init");
		exit(1);
	}

	unicode_convert(h, teststr, sizeof(teststr));

	if (unicode_convert_deinit(h, &checkflag))
	{
		perror("unicode_convert_deinit");
		exit(1);
	}
	if (cb.cnt != 2 || memcmp(cb.ptr, "AB", 2) || !checkflag)
	{
		fprintf(stderr, "Unexpected result from convert()\n");
		exit(1);
	}
}

static void test2()
{
	char32_t *ucptr;
	size_t ucsize;
	unicode_convert_handle_t h=
		unicode_convert_tou_init("utf-8", &ucptr, &ucsize, 1);
	char *cptr;
	size_t csize;

	if (h)
	{
		size_t i;

		for (i=0; i<1024/32; ++i)
			unicode_convert(h, "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA",
					  32);

		if (unicode_convert_deinit(h, NULL) == 0 &&
		    ucsize == 1024+1)
		{
			for (i=0; i<1024; i++)
				if (ucptr[i] != 'A')
					break;

			if (i == 1024)
			{
				h=unicode_convert_fromu_init("utf-8",
							       &cptr, &csize,
							       1);

				if (h)
				{
					unicode_convert_uc(h, ucptr, 1024);
					if (unicode_convert_deinit(h, NULL)
					    == 0 && csize == 1024+1)
					{
						for (i=0; i<1024; i++)
							if (cptr[i] != 'A')
								break;

						free(ucptr);
						free(cptr);
						if (i == 1024)
							return;
					}
				}
			}
		}
		fprintf(stderr, "test2: failed");
		errno=EINVAL;
	}
	perror("test2");
	exit(1);
}

int main(int argc, char **argv)
{
	const char *chset=unicode_x_imap_modutf7;
	int argn=1;

	if (argn < argc && strcmp(argv[argn], "--smap") == 0)
	{
		chset=unicode_x_imap_modutf7 " ./~:";
		++argn;
	}

	if (argn < argc && strcmp(argv[argn], "--smaputf8") == 0)
	{
		chset=unicode_x_smap_modutf8;
		++argn;
	}

	if (argn < argc && strcmp(argv[argn], "--modutf7toutf8") == 0)
	{
		while (++argn < argc)
		{
			int error=0;
			char *p=unicode_convert_tobuf(argv[argn],
						      unicode_x_imap_modutf7,
						      unicode_x_smap_modutf8,
						      &error);

			if (p)
			{
				printf("%s\n", p);
				free(p);
			}
			else
			{
				printf("[error]\n");
			}
		}
	}

	if (argn < argc && strcmp(argv[argn], "--totitle") == 0)
	{
		++argn;

		if (argn < argc)
		{
			char *p=unicode_convert_tocase(argv[argn],
							 "utf-8",
							 unicode_tc,
							 unicode_lc);

			if (p)
			{
				printf("%s\n", p);
				free(p);
				exit(0);
			}
		}
		exit(1);
	}

	if (argn < argc)
	{
		int errflag;
		char *p=unicode_convert_tobuf(argv[argn],
						"utf-8",
						chset,
						&errflag);
		char *q;

		if (!p)
		{
			perror("unicode_convert");
			exit(1);
		}

		if (errflag)
		{
			fprintf(stderr, "Conversion error?\n");
			exit(1);
		}

		q=unicode_convert_tobuf(p, chset, "utf-8", &errflag);
		if (!q)
		{
			perror("unicode_convert");
			exit(1);
		}

		if (errflag)
		{
			fprintf(stderr, "Conversion error?\n");
			exit(1);
		}
		if (strcmp(q, argv[argn]))
		{
			fprintf(stderr, "Round trip error\n");
			exit(1);
		}
		printf("%s\n", p);
		free(p);
		free(q);
	}
	else
	{
		test1();
		test2();
	}
	return 0;
}
