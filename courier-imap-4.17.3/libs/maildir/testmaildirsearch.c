/*
** Copyright 2011 Double Precision, Inc.
** See COPYING for distribution information.
*/

#include	"maildirsearch.h"


static int testmaildirsearch(const char *chset, const char *needle,
			     const char *haystack)
{
	struct maildir_searchengine se;
	char32_t *ucptr;
	size_t ucsize;
	size_t i;
	unicode_convert_handle_t h=unicode_convert_tou_init(chset, &ucptr,
								&ucsize, 1);

	if (h == NULL)
	{
		perror("unicode_convert_tou_init");
		return 1;
	}

	if (unicode_convert(h, haystack, strlen(haystack)))
	{
		unicode_convert_deinit(h, NULL);
		perror("unicode_convert_tou");
		return 1;
	}

	if (unicode_convert_deinit(h, NULL))
	{
		perror("unicode_convert_deinit");
		return 1;
	}

	maildir_search_init(&se);

	if (maildir_search_start_str_chset(&se, needle, chset))
	{
		perror("maildir_search_start_str_chset");
		maildir_search_destroy(&se);
		free(ucptr);
		return 1;
	}
	maildir_search_reset(&se);

	for (i=0; ucptr[i]; ++i)
	{
		maildir_search_step_unicode_lc(&se, ucptr[i]);
		if (maildir_search_found(&se))
		{
			free(ucptr);
			maildir_search_destroy(&se);
			return 0;
		}
	}
	free(ucptr);
	maildir_search_destroy(&se);
	return 2;
}

int main(int argc, char **argv)
{
	if (argc < 4)
	{
		fprintf(stderr, "Invalid args\n");
		return (1);
	}

	exit(testmaildirsearch(argv[1], argv[2], argv[3]));
	return (0);
}
