/*
 * $Id: read_assign.c,v 1.2 2024-05-09 22:03:17+05:30 mbhangui Exp mbhangui $
 */
#include <sys/types.h>
#include <ctype.h>
#include <unistd.h>
#include <fcntl.h>
#include <str.h>
#include <stralloc.h>
#include <strerr.h>
#include <alloc.h>
#include <fmt.h>
#include <scan.h>
#include <env.h>
#include <cdb.h>
#include "auto_assign.h"
#include "read_assign.h"

extern void     die_nomem();
/*
 * get uid, gid, dir from users/assign
 */
static stralloc cdbfilename, in_dir;

/*
 * reads user/cdb file
 *
 * This is same as indimail-virtualdomain's get_assign() function
 * but this doesn't cache the result in static variables
 */
char *
read_assign(const char *domain, stralloc *dir, uid_t *uid, gid_t *gid)
{
	int             fd;
	uint32_t        dlen, i;
	char           *s, *ptr, *cdbdir, *cdbkey, *tmpbuf;

	if (!domain || !*domain)
		return ((char *) 0);
	if (!stralloc_copys(&in_dir, domain) || !stralloc_0(&in_dir))
		die_nomem();
	in_dir.len--;
	for (s = in_dir.s; *s; s++) {
		if (isupper(*s))
			*s = tolower(*s);
	}
	if (!(cdbdir = env_get("ASSIGNDIR")))
		cdbdir = auto_assign;
	if (!stralloc_copys(&cdbfilename, cdbdir) ||
			!stralloc_catb(&cdbfilename, "/cdb", 4) ||
			!stralloc_0(&cdbfilename))
		die_nomem();
	i = in_dir.len;
#define FAILURE {if (uid) *uid = -1; if (gid) *gid = -1; if (dir) dir->len = 0; if (cdbkey) alloc_free(cdbkey);}
	if (!(cdbkey = alloc(sizeof(char ) * (i + 3)))) {
		FAILURE
		return ((char *) 0);
	}
	s = cdbkey;
	s += fmt_strn(s, "!", 1);
	s += fmt_strn(s, in_dir.s, i);
	s += fmt_strn(s, "-", 1);
	*s++ = 0;
	if ((fd = open(cdbfilename.s, O_RDONLY)) == -1) {
		FAILURE
		return ((char *) 0);
	}
	if ((i = cdb_seek(fd, cdbkey, in_dir.len + 2, &dlen)) == 1) {
		if (!(tmpbuf = (char *) alloc(dlen + 1))) {
			close(fd);
			FAILURE
			return ((char *) 0);
		}
		alloc_free(cdbkey);
		i = read(fd, tmpbuf, dlen);
		tmpbuf[dlen] = 0;
		for (ptr = tmpbuf; *ptr; ptr++);
		ptr++;
		if (uid) {
			scan_uint(ptr, &i);
			*uid = i;
		}
		for (; *ptr; ptr++);
		ptr++;
		if (gid) {
			scan_uint(ptr, &i);
			*gid = i;
		}
		for (; *ptr; ptr++);
		ptr++;
		/* directory */
		i = str_len(ptr);
		if (!stralloc_copyb(&in_dir, ptr, i) || !stralloc_0(&in_dir))
			die_nomem();
		if (dir) {
			if (!stralloc_copy(dir, &in_dir))
				die_nomem();
			dir->len--;
		}
		in_dir.len--;
		alloc_free(tmpbuf);
		close(fd);
		return (in_dir.s);
	}
	close(fd);
	FAILURE
	return ((char *) 0);
}

void
getversion_read_assign_c()
{
	const char     *x = "$Id: read_assign.c,v 1.2 2024-05-09 22:03:17+05:30 mbhangui Exp mbhangui $";

	if (x)
		x++;
}

/*
 * $Log: read_assign.c,v $
 * Revision 1.2  2024-05-09 22:03:17+05:30  mbhangui
 * fix discarded-qualifier compiler warnings
 *
 * Revision 1.1  2023-12-08 12:31:19+05:30  Cprogrammer
 * Initial revision
 *
 */
