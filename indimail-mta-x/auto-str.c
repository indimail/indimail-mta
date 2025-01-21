/*
 * $Id: auto-str.c,v 1.10 2025-01-22 00:30:37+05:30 Cprogrammer Exp mbhangui $
 */
#include <ctype.h>
#include <unistd.h>
#include "substdio.h"

char            buf1[256];
substdio        ss1 = SUBSTDIO_FDBUF((ssize_t (*)(int,  char *, size_t)) write, 1, buf1, sizeof(buf1));

/*
 * check if a given character can be printed unquoted in a C string
 * does not accept digits as they may be hardly visible between octal
 * encoded chars
 */
static int
is_legible(unsigned char ch)
{
    if (isascii(ch))
        return 1;
    if (ch == '/' || ch == '_' || ch == '-' || ch == '.')
        return 1;
    return 0;
}

void
my_puts(const char *s)
{
	if (substdio_puts(&ss1, s) == -1)
		_exit(111);
}

int
main(int argc, char **argv)
{
	char           *name;
	char           *value;
	unsigned char   ch;
	char            octal[4];

	if (!(name = argv[1]))
		_exit(100);
	if (!(value = argv[2]))
		_exit(100);
	my_puts("char ");
	my_puts(name);
	my_puts("[] = \"");
	while ((ch = *value++)) {
        if (is_legible(ch)) {
            if (substdio_put(&ss1, (char *) &ch, 1) == -1)
                _exit(111);
        } else {
			my_puts("\\");
			octal[3] = 0;
			octal[2] = '0' + (ch & 7);
			ch >>= 3;
			octal[1] = '0' + (ch & 7);
			ch >>= 3;
			octal[0] = '0' + (ch & 7);
			my_puts(octal);
		}
	}
	my_puts("\";\n");
	if (substdio_flush(&ss1) == -1)
		_exit(111);
	_exit(0);
	/*- Not reached */
	return(0);
}
/*
 * $Log: auto-str.c,v $
 * Revision 1.10  2025-01-22 00:30:37+05:30  Cprogrammer
 * Fixes for gcc14
 *
 * Revision 1.9  2024-05-12 00:20:03+05:30  mbhangui
 * fix function prototypes
 *
 * Revision 1.8  2020-11-24 13:44:02+05:30  Cprogrammer
 * removed exit.h
 *
 * Revision 1.7  2020-06-17 16:58:35+05:30  Cprogrammer
 * make output readable
 *
 * Revision 1.6  2004-10-22 15:34:20+05:30  Cprogrammer
 * replaced readwrite.h with unistd.h
 *
 * Revision 1.5  2004-07-17 21:15:59+05:30  Cprogrammer
 * added RCS log
 *
 */
