/*
** Copyright 2002-2010 Double Precision, Inc.
** See COPYING for distribution information.
*/

#include "config.h"
#include "maildirsearch.h"


int maildir_search_start_unicode(struct maildir_searchengine *sei,
				 const char32_t *s)
{
	unsigned i, j, *r;

	size_t n;

	for (n=0; s[n]; ++n)
		;
	++n;

	if (sei->string)
		free(sei->string);

	sei->string=malloc(n * sizeof(*s));
	if (!sei->string)
		return (-1);
	sei->string_l=n-1;

	memcpy(sei->string, s, n * sizeof(*s));

	if (sei->r)
		free(sei->r);

	sei->i=0;
	if ((sei->r=r=(unsigned *)malloc(sizeof(unsigned)*n)) == 0)
		return (-1);

	for (i=0; s[i]; i++)
		r[i]=0;

	for (i=0; s[i]; i++)
		for (j=0; s[i+j]; j++)
			if (s[j] != s[i+j])
			{
				if (r[i+j] < j)
					r[i+j]=j;
				break;
			}

	maildir_search_reset(sei);
	return (0);
}

int maildir_search_start_str(struct maildir_searchengine *sei,
			     const char *s)
{
	char32_t *uc=malloc((strlen(s)+1) * sizeof(char32_t));
	size_t n;
	int rc;

	if (!uc)
		return -1;

	for (n=0; (uc[n]=(unsigned char)s[n]) != 0; ++n)
		;

	rc=maildir_search_start_unicode(sei, uc);
	free(uc);
	return rc;
}

int maildir_search_start_str_chset(struct maildir_searchengine *engine,
				   const char *string,
				   const char *chset)
{
#define SPC(s) ((s) == ' '|| (s) == '\t' || (s) == '\r' || (s) == '\n')

	char32_t *ucptr;
	size_t ucsize;
	unicode_convert_handle_t h=unicode_convert_tou_init(chset, &ucptr,
								&ucsize, 1);
	size_t i, j;
	int rc;

	if (h == NULL)
		return -1;

	if (unicode_convert(h, string, strlen(string)))
	{
		unicode_convert_deinit(h, NULL);
		return -1;
	}

	if (unicode_convert_deinit(h, NULL))
		return -1;

	for (i=j=0; ucptr[i]; )
	{
		while (SPC(ucptr[i]))
			++i;

		if (!ucptr[i])
			break;

		while (ucptr[i])
		{
			ucptr[j]=unicode_lc(ucptr[i]);
			++j;
			if (SPC(ucptr[i]))
				break;

			++i;
		}
	}

	while (j > 0 && SPC(ucptr[j-1]))
		--j;
	ucptr[j]=0;

	rc=maildir_search_start_unicode(engine, ucptr);
	free(ucptr);
	return rc;
}
