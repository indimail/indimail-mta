/*
 * $Log: printforward.c,v $
 * Revision 1.4  2021-08-29 23:27:08+05:30  Cprogrammer
 * define functions as noreturn
 *
 * Revision 1.3  2005-08-23 17:33:32+05:30  Cprogrammer
 * gcc 4 compliance
 *
 * Revision 1.2  2004-10-22 20:28:00+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.1  2004-10-21 22:47:00+05:30  Cprogrammer
 * Initial revision
 *
 */
#include <unistd.h>
#include <substdio.h>
#include <subfd.h>
#include <strerr.h>
#include <stralloc.h>
#include <cdb.h>
#include <noreturn.h>

#define FATAL "printmaillist: fatal: "

no_return void
badformat()
{
	strerr_die2x(100, FATAL, "bad database format");
}

no_return void
nomem()
{
	strerr_die2x(111, FATAL, "out of memory");
}

void
getch(char *ch)
{
	int             r;

	if ((r = substdio_get(subfdinsmall, ch, 1)) == -1)
		strerr_die2sys(111, FATAL, "unable to read input: ");
	if (r == 0)
		badformat();
}

void
putch(char *ch)
{
	if (substdio_put(subfdoutsmall, ch, 1) == -1)
		strerr_die2x(111, FATAL, "unable to write output: ");
}

void
print(char *buf)
{
	while (*buf)
		putch(buf++);
}

void
printsafe(char *buf, int len)
{
	char            ch;

	while (len) {
		ch = *buf;
		if ((ch <= 32) || (ch == ',') || (ch == ':') || (ch == ';') || (ch == '\\') || (ch == '#'))
			putch("\\");
		putch(&ch);
		++buf;
		--len;
	}
}

int
main()
{
	uint32          eod;
	uint32          pos;
	uint32          klen;
	uint32          dlen;
	char            buf[8];
	char            ch;
	int             i;
	int             j;
	stralloc        key = { 0 };
	stralloc        data = { 0 };

	for (i = 0; i < 4; ++i)
		getch(buf + i);
	eod = cdb_unpack((unsigned char *) buf);
	for (i = 4; i < 2048; ++i)
		getch(&ch);
	pos = 2048;
	while (pos < eod) {
		if (eod - pos < 8)
			badformat();
		pos += 8;
		for (i = 0; i < 8; ++i)
			getch(buf + i);
		klen = cdb_unpack((unsigned char *) buf);
		dlen = cdb_unpack((unsigned char *) buf + 4);
		if (!stralloc_copys(&key, ""))
			nomem();
		if (eod - pos < klen)
			badformat();
		pos += klen;
		while (klen) {
			--klen;
			getch(&ch);
			if (!stralloc_append(&key, &ch))
				nomem();
		}
		if (eod - pos < dlen)
			badformat();
		pos += dlen;
		if (!stralloc_copys(&data, ""))
			nomem();
		while (dlen) {
			--dlen;
			getch(&ch);
			if (!stralloc_append(&data, &ch))
				nomem();
		}
		if (!key.len)
			badformat();
		if (key.s[0] == '?') {
			printsafe(key.s + 1, key.len - 1);
			print(": ?");
			printsafe(data.s, data.len);
			print(";\n");
		} else
		if (key.s[0] == ':') {
			printsafe(key.s + 1, key.len - 1);
			print(":\n");

			i = 0;
			for (j = 0; j < data.len; ++j) {
				if (!data.s[j]) {
					if ((data.s[i] == '.') || (data.s[i] == '/')) {
						print(", ");
						printsafe(data.s + i, j - i);
						print("\n");
					} else
					if ((data.s[i] == '|') || (data.s[i] == '!')) {
						print(", ");
						printsafe(data.s + i, j - i);
						print("\n");
					} else
					if ((data.s[i] == '&') && (j - i < 900)) {
						print(", ");
						printsafe(data.s + i, j - i);
						print("\n");
					} else
						badformat();
					i = j + 1;
				}
			} /*- for (j = 0; j < data.len; ++j) */
			if (i != j)
				badformat();
			print(";\n");
		} else
			badformat();
	}
	if (substdio_flush(subfdoutsmall) == -1)
		strerr_die2sys(111, FATAL, "unable to write output: ");
	_exit(0);
	/*- Not reached */
	return(0);
}

void
getversion_printforward_c()
{
	static char    *x = "$Id: printforward.c,v 1.4 2021-08-29 23:27:08+05:30 Cprogrammer Exp mbhangui $";

	x++;
}
