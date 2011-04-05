/*
 * echo.c, derived from code echo.c in Bash.
 * Copyright (C) 87,89, 1991-1997, 1999-2005, 2007 Free Software
 * Foundation, Inc.
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.  
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include "config.h"

/*
 * The official name of this program (e.g., no `g' prefix).  
 */
#define PROGRAM_NAME "echo"

#define GETOPT_HELP_OPTION_DECL    "help",    no_argument, NULL, GETOPT_HELP_CHAR
#define GETOPT_VERSION_OPTION_DECL "version", no_argument, NULL, GETOPT_VERSION_CHAR
/*
 * Program_name must be a literal string.
 * Usually it is just PROGRAM_NAME.  
 */
#define USAGE_BUILTIN_WARNING \
	"\n" \
	"NOTE: your shell may have its own version of %s, which usually supersedes\n" \
	"the version described here.  Please refer to your shell's documentation\n" \
	"for details about the options it supports.\n"

#define HELP_OPTION_DESCRIPTION \
	"      --help     display this help and exit\n"
#define VERSION_OPTION_DESCRIPTION \
	"      --version  output version information and exit\n"

#define STREQ(a, b) (strcmp (a, b) == 0)

/*
 * echo [-neE] [arg ...]
 * Output the ARGs.  If -n is specified, the trailing newline is
 * suppressed.  If the -e option is given, interpretation of the
 * following backslash-escaped characters is turned on:
 * \a   alert (bell)
 * \b   backspace
 * \c   suppress trailing newline
 * \f   form feed
 * \n   new line
 * \r   carriage return
 * \t   horizontal tab
 * \v   vertical tab
 * \\   backslash
 * \0NNN    the character whose ASCII code is NNN (octal).
 * 
 * You can explicitly turn off the interpretation of the above characters
 * on System V systems with the -E option.
 */

/*
 * If true, interpret backslash escapes by default.  
 */
#ifndef DEFAULT_ECHO_TO_XPG
enum { DEFAULT_ECHO_TO_XPG = false };
#endif

void
emit_bug_reporting_address(void)
{
/*
 * TRANSLATORS: The placeholder indicates the bug-reporting address
 * for this package.  Please add _another line_ saying
 * "Report translation bugs to <...>\n" with the address for translation
 * bugs (typically your translation team's web or email address).  
 */
	printf("\nReport bugs to: %s\n", PACKAGE_BUGREPORT);
#ifdef PACKAGE_PACKAGER_BUG_REPORTS
	printf("Report %s bugs to: %s\n", PACKAGE_PACKAGER, PACKAGE_PACKAGER_BUG_REPORTS);
#endif
#ifdef PACKAGE_URL
	printf("%s home page: <%s>\n", PACKAGE_NAME, PACKAGE_URL);
#else
	printf("%s home page: <http://www.indimail.org/>\n", PACKAGE_NAME);
#endif
}

/*
 * The name this program was run with. 
 */
char           *program_name;

void
usage(int status)
{
	if (status != EXIT_SUCCESS)
		fprintf(stderr, "Try `%s --help' for more information.\n", program_name);
	else {
		printf("Usage: %s [OPTION]... [STRING]...\n", program_name);
		fputs("\
Echo the STRING(s) to standard output.\n\
\n\
  -n             do not output the trailing newline\n\
", stdout);
		fputs(DEFAULT_ECHO_TO_XPG ? "\
  -e             enable interpretation of backslash escapes (default)\n\
  -E             disable interpretation of backslash escapes\n" : "\
  -e             enable interpretation of backslash escapes\n\
  -E             disable interpretation of backslash escapes (default)\n", stdout);
		fputs(HELP_OPTION_DESCRIPTION, stdout);
		fputs(VERSION_OPTION_DESCRIPTION, stdout);
		fputs("\
\n\
If -e is in effect, the following sequences are recognized:\n\
\n\
  \\0NNN   the character whose ASCII code is NNN (octal)\n\
  \\\\     backslash\n\
  \\a     alert (BEL)\n\
  \\b     backspace\n\
", stdout);
		fputs("\
  \\c     suppress trailing newline\n\
  \\f     form feed\n\
  \\n     new line\n\
  \\r     carriage return\n\
  \\t     horizontal tab\n\
  \\v     vertical tab\n\
", stdout);
		printf(USAGE_BUILTIN_WARNING, PROGRAM_NAME);
		emit_bug_reporting_address();
	}
	exit(status);
}

/*
 * Convert C from hexadecimal character to integer.  
 */
static int
hextobin(unsigned char c)
{
	switch (c) {
	default:
		return c - '0';
	case 'a':
	case 'A':
		return 10;
	case 'b':
	case 'B':
		return 11;
	case 'c':
	case 'C':
		return 12;
	case 'd':
	case 'D':
		return 13;
	case 'e':
	case 'E':
		return 14;
	case 'f':
	case 'F':
		return 15;
	}
}

/*
 * Print the words in LIST to standard output.  If the first word is
 * `-n', then don't print a trailing newline.  We also support the
 * echo syntax from Version 9 unix systems. 
 */
int
main(int argc, char **argv)
{
	bool            display_return = true;
	bool            allow_options = (!getenv("POSIXLY_CORRECT")
									 || (!DEFAULT_ECHO_TO_XPG && 1 < argc && STREQ(argv[1], "-n")));

/*
 * System V machines already have a /bin/sh with a v9 behavior.
 * Use the identical behavior for these machines so that the
 * existing system shell scripts won't barf.  
 */
	bool            do_v9 = DEFAULT_ECHO_TO_XPG;

	program_name = argv[0];
	--argc;
	++argv;

	if (allow_options)
		while (argc > 0 && *argv[0] == '-') {
			char const     *temp = argv[0] + 1;
			size_t          i;

		/*
		 * If it appears that we are handling options, then make sure that
		 * all of the options specified are actually valid.  Otherwise, the
		 * string should just be echoed.  
		 */

			for (i = 0; temp[i]; i++)
				switch (temp[i]) {
				case 'e':
				case 'E':
				case 'n':
					break;
				default:
					goto just_echo;
				}

			if (i == 0)
				goto just_echo;

		/*
		 * All of the options in TEMP are valid options to ECHO.
		 * Handle them. 
		 */
			while (*temp)
				switch (*temp++) {
				case 'e':
					do_v9 = true;
					break;

				case 'E':
					do_v9 = false;
					break;

				case 'n':
					display_return = false;
					break;
				}

			argc--;
			argv++;
		}

just_echo:

	if (do_v9) {
		while (argc > 0) {
			char const     *s = argv[0];
			unsigned char   c;

			while ((c = *s++)) {
				if (c == '\\' && *s) {
					switch (c = *s++) {
					case 'a':
						c = '\a';
						break;
					case 'b':
						c = '\b';
						break;
					case 'c':
						exit(EXIT_SUCCESS);
					case 'f':
						c = '\f';
						break;
					case 'n':
						c = '\n';
						break;
					case 'r':
						c = '\r';
						break;
					case 't':
						c = '\t';
						break;
					case 'v':
						c = '\v';
						break;
					case 'x':
					{
						unsigned char   ch = *s;
						if (!isxdigit(ch))
							goto not_an_escape;
						s++;
						c = hextobin(ch);
						ch = *s;
						if (isxdigit(ch)) {
							s++;
							c = c * 16 + hextobin(ch);
						}
					}
						break;
					case '0':
						c = 0;
						if (!('0' <= *s && *s <= '7'))
							break;
						c = *s++;
					/*
					 * Fall through.  
					 */
					case '1':
					case '2':
					case '3':
					case '4':
					case '5':
					case '6':
					case '7':
						c -= '0';
						if ('0' <= *s && *s <= '7')
							c = c * 8 + (*s++ - '0');
						if ('0' <= *s && *s <= '7')
							c = c * 8 + (*s++ - '0');
						break;
					case '\\':
						break;

not_an_escape:
					default:
						putchar('\\');
						break;
					}
				}
				putchar(c);
			}
			argc--;
			argv++;
			if (argc > 0)
				putchar(' ');
		}
	} else {
		while (argc > 0) {
			fputs(argv[0], stdout);
			argc--;
			argv++;
			if (argc > 0)
				putchar(' ');
		}
	}

	if (display_return)
		putchar('\n');
	exit(EXIT_SUCCESS);
}
