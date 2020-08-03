/*
 * $Log: pathexec_run.c,v $
 * Revision 1.5  2020-08-03 17:25:25+05:30  Cprogrammer
 * use qmail library
 *
 * Revision 1.4  2017-12-17 19:11:37+05:30  Cprogrammer
 * added documentation
 *
 * Revision 1.3  2017-12-17 19:08:45+05:30  Cprogrammer
 * added documentation
 *
 * Revision 1.2  2016-02-08 21:30:19+05:30  Cprogrammer
 * load shared objects if file has .so extension
 *
 * Revision 1.1  2003-12-31 19:46:55+05:30  Cprogrammer
 * Initial revision
 *
 */
#include <error.h>
#include <stralloc.h>
#include <str.h>
#include <env.h>
#include <unistd.h>
#include "pathexec.h"

static stralloc tmp;

void
pathexec_run(char *file, char **argv, char **envp)
{
	char           *path;
	unsigned int    split;
	int             savederrno;

	if (file[str_chr(file, '/')]) {
#ifdef LOAD_SHARED_OBJECTS
		load_shared(file, argv, envp); /*- does not return */
#else
		execve(file, argv, envp); /*- does not return */
#endif
		return;
	}

	path = env_get("PATH");
	if (!path)
		path = "/bin:/usr/bin";

	savederrno = 0;
	for (;;) {
		split = str_chr(path, ':');
		if (!stralloc_copyb(&tmp, path, split))
			return;
		if (!split)
			if (!stralloc_cats(&tmp, "."))
				return;
		if (!stralloc_cats(&tmp, "/"))
			return;
		if (!stralloc_cats(&tmp, file))
			return;
		if (!stralloc_0(&tmp))
			return;
		execve(tmp.s, argv, envp);
		if (errno != error_noent) {
			savederrno = errno;
			if ((errno != error_acces) && (errno != error_perm) && (errno != error_isdir))
				return;
		}

		if (!path[split]) {
			if (savederrno)
				errno = savederrno;
			return;
		}
		path += split;
		path += 1;
	}
}
