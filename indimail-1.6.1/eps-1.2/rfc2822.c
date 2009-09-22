#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "eps.h"

static int      comment_exception = 0;

int
rfc2822_remove_crlf(unsigned char *data)
{
	unsigned char  *p = NULL;

	for (p = data; *p; p++)
	{
		if ((*p == '\n') || (*p == '\r'))
			break;
	}
	if (!(*p))
		return 0;
	*p = '\0';
	return 1;
}

int
rfc2822_is_wsp(unsigned char c)
{
	if ((c == ' ') || (c == '\t'))
		return 1;
	return 0;
}

unsigned char  *
rfc2822_remove_comments(unsigned char *data)
{
	unsigned long   len = 0, i = 0;
	unsigned char   com = 0, lit = 0, *r = NULL, *p = NULL;

	com = lit = 0;
	if(!strncasecmp((const char *) data, "Received:", 9))
		comment_exception = 1;
	else
		comment_exception = 0;
	for (i = 0, p = data; *p; p++)
	{
		if ((com == 0) && ((*p == '\\') && ((*(p + 1)))))
		{
			len += 2;
			p++;
		} else
		if ((com == 1) && ((*p == '\\') && ((*(p + 1)))))
			p++;
		else
		if ((*p == '\"') && (com == 0))
		{
			if (lit)
				lit = 0;
			else
				lit = 1;
			len++;
		} else
		if(!comment_exception)
		{
			if ((*p == '(') && (!lit))
			{
				if (!com)
					com++;
			} else
			if ((*p == ')') && (!lit))
			{
				if (com)
					com--;
			} else
			if (com == 0)
				len++;
		} else
		if (com == 0)
			len++;
	}
	if(!(r = (unsigned char *) mmalloc(len + 1, "rfc2822_remove_comments")))
		return NULL;
	memset((char *) r, 0, (len + 1));
	for (i = 0, p = data; (*p) && (i < len); p++)
	{
		if ((com == 0) && ((*p == '\\') && ((*(p + 1)))))
		{
			r[i++] = *p;
			r[i++] = *(p + 1);
			p++;
		} else
		if ((com == 1) && ((*p == '\\') && ((*(p + 1)))))
			p++;
		else
		if ((*p == '\"') && (com == 0))
		{
			if (lit)
				lit = 0;
			else
				lit = 1;

			r[i++] = *p;
		} else
		if(!comment_exception)
		{
			if ((*p == '(') && (!lit))
			{
				if (!com)
					com++;
			} else
			if ((*p == ')') && (!lit))
			{
				if (com)
					com--;
			} else
			if (com == 0)
				r[i++] = *p;
		} else
		if (com == 0)
			r[i++] = *p;
	}
	return r;
}

unsigned char  *
rfc2822_next_token(unsigned char *line, unsigned char token, unsigned char *term)
{
	unsigned char  *p = NULL, lit = 0, i = 0;

	lit = 0;
	for (p = line; *p; p++)
	{
		if ((*p == '\\') && ((*(p + 1))))
			p++;
		else
		if (*p == '\"')
		{
			if (lit)
				lit = 0;
			else
				lit = 1;
		}
		/*
		 * Token or NULL?
		 */
		else
		if ((token != '\0') && (lit == 0) && (*p == token))
			return p;
		/*
		 * Check for terminator
		 */
		else
		if ((!lit) && (term))
		{
			for (i = 0; *(term + i); i++)
			{
				if (*p == *(term + i))
					return p;
			}
		}
	}

	return p;
}

unsigned char  *
rfc2822_convert_literals(unsigned char *data)
{
	unsigned long   len = 0, i = 0;
	unsigned char   com = 0, lit = 0, *r = NULL, *p = NULL;

	com = lit = 0;
	for (i = 0, p = data; *p; p++)
	{
		if ((*p == '\\') && ((*(p + 1))))
		{
			len++;
			p++;
		} else
		if (*p == '\"')
		{
			if (lit)
				lit = 0;
			else
				lit = 1;
		} else
		if (com == 0)
			len++;
	}
	r = (unsigned char *) mmalloc(len + 1, "rfc2822_convert_literals");
	if (r == NULL)
		return NULL;
	memset((char *) r, 0, (len + 1));
	for (i = 0, p = data; (*p) && (i < len); p++)
	{
		if ((*p == '\\') && ((*(p + 1))))
		{
			r[i++] = *(p + 1);
			p++;
		} else
		if (*p == '\"')
		{
			if (lit)
				lit = 0;
			else
				lit = 1;
		} else
		if (com == 0)
			r[i++] = *p;
	}
	return r;
}
