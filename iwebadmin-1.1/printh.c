/*
 * $Id: printh.c,v 1.1 2010-04-26 11:25:09+05:30 Cprogrammer Exp mbhangui $
 * Copyright (C) 2004 Tom Logic LLC 
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 */

#include "printh.h"
/*
 * included from printh.h:
 * #include <stdlib.h>
 * #include <stdarg.h>
 */
#include <string.h>
#include <stdio.h>
#include <ctype.h>

/*
 * vsnprinth - Custom version of sprintf for escaping strings for use on HTML
 *             pages and in cgi script parameters.
 *
 * (based on qnprintf from vpopmail)
 *
 * int vsnprinth (char *buffer, size_t size, const char *format, va_list ap)
 * int snprinth (char *buffer, size_t size, const char *format, ...);
 * int printh (const char *format, ...);
 *
 *   buffer - buffer to print string to
 *   size   - size of buffer
 *   format - a printf-style format string*
 *   ...    - variable arguments for the format string
 *
 *  NOTE: Currently supported formats: %%, %c, %s, %d/%i, %u, %ld/%li, %lu
 *  Since this function was designed to escape strings for use on HTML pages,
 *  the formats don't support any extended options.
 *
 *  Extra formats: %C (like %s, but converts string to cgi-parameter safe
 *               version) and %H (like %s, but converts to HTML-safe version)
 *
 * Returns the number of characters that would have been printed to buffer
 * if it was big enough.  (i.e., if return value is larger than (size-1),
 * buffer received an incomplete copy of the formatted string).
 *
 * It is possible to call snprinth with a NULL buffer of 0 bytes to determine
 * how large the buffer needs to be.  This is inefficient, as snprinth has
 * to run twice.
 *
 * snprinth written November 2004 by Tom Collins <tom@tomlogic.com>
 * qnprintf written February 2004 by Tom Collins <tom@tomlogic.com>
 */
int
vsnprinth(char *buffer, size_t size, const char *format, va_list ap)
{
	const char      hex[] = "0123456789abcdef";

	int             printed;	/* number of characters that would have been printed */
	const char     *f;			/* current position in format string */
	char           *b;			/* current position in output buffer */
	char            n[20];		/* buffer to hold string representation of number */

	char           *s;			/* pointer to string to insert */
	char           *copy;		/* pointer to replacement string for special char */

	int             stringtype;
#define SPRINTH_NORMAL 0
#define SPRINTH_HTML 1
#define SPRINTH_CGI 2

	if (buffer == NULL && size > 0)
		return -1;

	printed = 0;
	b = buffer;
	for (f = format; *f != '\0'; f++) {
		if (*f != '%') {
			if (++printed < size)
				*b++ = *f;
		} else {
			f++;
			s = n;
			stringtype = SPRINTH_NORMAL;
			switch (*f) {
			case '%':
				strcpy(n, "%");
				break;

			case 'c':
				snprintf(n, sizeof (n), "%c", va_arg(ap, int));
				break;

			case 'd':
			case 'i':
				snprintf(n, sizeof (n), "%d", va_arg(ap, int));
				break;

			case 'u':
				snprintf(n, sizeof (n), "%u", va_arg(ap, unsigned int));
				break;

			case 'l':
				f++;
				switch (*f) {
				case 'd':
				case 'i':
					snprintf(n, sizeof (n), "%ld", va_arg(ap, long));
					break;

				case 'u':
					snprintf(n, sizeof (n), "%lu", va_arg(ap, unsigned long));
					break;

				default:
					strcpy(n, "*");
				}
				break;

			case 's':
				s = va_arg(ap, char *);
				break;

			case 'H':
				s = va_arg(ap, char *);
				stringtype = SPRINTH_HTML;
				break;

			case 'C':
				s = va_arg(ap, char *);
				stringtype = SPRINTH_CGI;
				break;

			default:
				strcpy(n, "*");
			}

		/*
		 * now copy the string parameter into the buffer 
		 */
			while (*s != '\0') {
				copy = NULL;	/* default to no special handling */
				if (stringtype == SPRINTH_HTML) {
					switch (*s) {
					case '"':
						copy = "&quot;";
						break;
					case '<':
						copy = "&lt;";
						break;
					case '>':
						copy = "&gt;";
						break;
					case '&':
						copy = "&amp;";
						break;
					}
				} else if (stringtype == SPRINTH_CGI) {
					if (*s == ' ')
						copy = strcpy(n, "+");
					else if (!isalnum(*s) && (strchr("._-", *s) == NULL)) {
						copy = n;
						sprintf(n, "%%%c%c", hex[*s >> 4 & 0x0F], hex[*s & 0x0F]);
					}
				}
				if (copy == NULL) {
					if (++printed < size)
						*b++ = *s;
				} else {
				/*
				 * replace *s with buffer pointed to by copy 
				 */
					while (*copy != '\0') {
						if (++printed < size)
							*b++ = *copy;
						copy++;
					}
				}
				s++;
			}
		}
	}

	*b = '\0';

/*
 * If the formatted string doesn't fit in the buffer, zero out the buffer. 
 */
	if (printed >= size) {
		memset(buffer, '\0', size);
	}

	return printed;
}

int
snprinth(char *buffer, size_t size, const char *format, ...)
{
	int             ret;

	va_list         argp;

	va_start(argp, format);
	ret = vsnprinth(buffer, size, format, argp);
	va_end(argp);

	return ret;
}

int
printh(const char *format, ...)
{
	int             ret;
	char            buffer[1024];

	va_list         argp;

	va_start(argp, format);
	ret = vsnprinth(buffer, sizeof (buffer), format, argp);
	va_end(argp);

	printf("%s", buffer);

	return ret;
}
