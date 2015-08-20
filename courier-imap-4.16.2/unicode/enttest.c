#include	"unicode_config.h"
#include	"unicode.h"

#include	<string.h>
#include	<stdio.h>
#include	<stdlib.h>

struct i {
	size_t n_start;
	size_t n_size;
	unicode_char v;
};

#include "unicode_htmlent.h"

static void testsuite()
{
	size_t j;

	for (j=0; j<sizeof(ii)/sizeof(ii[0]); ++j)
	{
		char buf[60];

		memcpy(buf, n + ii[j].n_start, ii[j].n_size);
		buf[ii[j].n_size]=0;

		if (unicode_html40ent_lookup(buf) != ii[j].v)
		{
			fprintf(stderr, "Did not find %s\n", buf);
			exit(1);
		}

		strcat(buf, "X");

		if (unicode_html40ent_lookup(buf) == ii[j].v)
		{
			fprintf(stderr, "Found %s?\n", buf);
			exit(1);
		}

		buf[strlen(buf)-2]=0;

		if (unicode_html40ent_lookup(buf) == ii[j].v)
		{
			fprintf(stderr, "Found %s?\n", buf);
			exit(1);
		}
	}
}

int main(int argc, char **argv)
{
	testsuite();
	return 0;
}
