/*
** Copyright 2000 Double Precision, Inc.
** See COPYING for distribution information.
*/

/*
*/
#include	"config.h"
#include	<stdio.h>
#include	<ctype.h>
#include	<stdlib.h>
#include	<string.h>
#include	"rfc822.h"

#if	HAVE_STRCASECMP

#else
#define	strcasecmp	stricmp
#endif

#if	HAVE_STRNCASECMP

#else
#define	strncasecmp	strnicmp
#endif

/* Skip over blobs */

static char *skipblob(char *p, char **save_blob_ptr)
{
	char *q;
	char *orig_p=p;
	int isalldigits=1;

	if (*p == '[')
	{
		for (q= p+1; *q; q++)
			if (*q == '[' || *q == ']')
				break;
			else if (strchr("0123456789", *q) == NULL)
				isalldigits=0;

		if (*q == ']')
		{
			p=q+1;

			while (isspace((int)(unsigned char)*p))
			{
				++p;
			}

			if (save_blob_ptr && *save_blob_ptr && !isalldigits)
			{
				while (orig_p != p)
					*(*save_blob_ptr)++=*orig_p++;
			}

			return (p);
		}
	}
	return (p);
}

static char *skipblobs(char *p, char **save_blob_ptr)
{
	char *q=p;

	do
	{
		p=q;
		q=skipblob(p, save_blob_ptr);
	} while (q != p);
	return (q);
}

/* Remove artifacts from the subject header */

static void stripsubj(char *s, int *hasrefwd, char *save_blob_buf)
{
	char	*p;
	char	*q;
	int doit;

	for (p=q=s; *p; p++)
	{
		if (!isspace((int)(unsigned char)*p))
		{
			*q++=*p;
			continue;
		}
		while (p[1] && isspace((int)(unsigned char)p[1]))
		{
			++p;
		}
		*q++=' ';
	}
	*q=0;

	do
	{
		doit=0;
		/*
		**
		** (2) Remove all trailing text of the subject that matches
		** the subj-trailer ABNF, repeat until no more matches are
		** possible.
		**
		**  subj-trailer    = "(fwd)" / WSP
		*/

		for (p=s; *p; p++)
			;
		while (p > s)
		{
			if ( isspace((int)(unsigned char)p[-1]))
			{
				--p;
				continue;
			}
			if (p-s >= 5 && strncasecmp(p-5, "(FWD)", 5) == 0)
			{
				p -= 5;
				*hasrefwd |= CORESUBJ_FWD;
				continue;
			}
			break;
		}
		*p=0;

		for (p=s; *p; )
		{
			for (;;)
			{
				char *orig_blob_ptr;
				int flag=CORESUBJ_FWD;

				/*
				**
				** (3) Remove all prefix text of the subject
				** that matches the subj-leader ABNF.
				**
				**   subj-leader     = (*subj-blob subj-refwd) / WSP
				**
				**   subj-blob       = "[" *BLOBCHAR "]" *WSP
				**
				**   subj-refwd      = ("re" / ("fw" ["d"])) *WSP [subj-blob] ":"
				**
				**   BLOBCHAR        = %x01-5a / %x5c / %x5e-7f
				**                   ; any CHAR except '[' and ']'
				*/

				if (isspace((int)(unsigned char)*p))
				{
					++p;
					continue;
				}

				q=skipblobs(p, NULL);

				if (strncasecmp(q, "RE", 2) == 0)
				{
					flag=CORESUBJ_RE;
					q += 2;
				}
				else if (strncasecmp(q, "FWD", 3) == 0)
				{
					q += 3;
				}
				else if (strncasecmp(q, "FW", 2) == 0)
				{
					q += 2;
				}
				else q=0;

				if (q)
				{
					orig_blob_ptr=save_blob_buf;

					q=skipblob(q, &save_blob_buf);
					if (*q == ':')
					{
						p=q+1;
						*hasrefwd |= flag;
						continue;
					}

					save_blob_buf=orig_blob_ptr;
				}


				/*
				** (4) If there is prefix text of the subject
				** that matches the subj-blob ABNF, and
				** removing that prefix leaves a non-empty
				** subj-base, then remove the prefix text.
				**
				**   subj-base       = NONWSP *([*WSP] NONWSP)
				**                   ; can be a subj-blob
				*/

				orig_blob_ptr=save_blob_buf;

				q=skipblob(p, &save_blob_buf);

				if (q != p && *q)
				{
					p=q;
					continue;
				}
				save_blob_buf=orig_blob_ptr;
				break;
			}

			/*
			**
			** (6) If the resulting text begins with the
			** subj-fwd-hdr ABNF and ends with the subj-fwd-trl
			** ABNF, remove the subj-fwd-hdr and subj-fwd-trl and
			** repeat from step (2).
			**
			**   subj-fwd-hdr    = "[fwd:"
			**
			**   subj-fwd-trl    = "]"
			*/

			if (strncasecmp(p, "[FWD:", 5) == 0)
			{
				q=strrchr(p, ']');
				if (q && q[1] == 0)
				{
					*q=0;
					p += 5;
					*hasrefwd |= CORESUBJ_FWD;

					for (q=s; (*q++=*p++) != 0; )
						;
					doit=1;
				}
			}
			break;
		}
	} while (doit);

	q=s;
	while ( (*q++ = *p++) != 0)
		;
	if (save_blob_buf)
		*save_blob_buf=0;
}

char *rfc822_coresubj(const char *s, int *hasrefwd)
{
	char *q=strdup(s), *r;
	int dummy;

	if (!hasrefwd)
		hasrefwd= &dummy;

	*hasrefwd=0;
	if (!q)	return (0);

	for (r=q; *r; r++)
		if ((*r & 0x80) == 0)	/* Just US-ASCII casing, thanks */
		{
			if (*r >= 'a' && *r <= 'z')
				*r += 'A'-'a';
		}
	stripsubj(q, hasrefwd, 0);
	return (q);
}

char *rfc822_coresubj_nouc(const char *s, int *hasrefwd)
{
	char *q=strdup(s);
	int dummy;

	if (!hasrefwd)
		hasrefwd= &dummy;

	*hasrefwd=0;
	if (!q)	return (0);

	stripsubj(q, hasrefwd, 0);
	return (q);
}

char *rfc822_coresubj_keepblobs(const char *s)
{
	char *q=strdup(s), *r;
	int dummy;

	if (!q)	return (0);

	r=strdup(s);
	if (!r)
	{
		free(q);
		return (0);
	}

	stripsubj(q, &dummy, r);
	strcat(r, q);
	free(q);
	return (r);
}
