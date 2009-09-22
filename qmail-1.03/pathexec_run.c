/*
 * $Log: pathexec_run.c,v $
 * Revision 1.2  2004-10-22 20:27:55+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.1  2004-07-17 20:57:48+05:30  Cprogrammer
 * Initial revision
 *
 */
#include <unistd.h>
#include "error.h"
#include "stralloc.h"
#include "str.h"
#include "env.h"
#include "pathexec.h"

static stralloc tmp;

void
pathexec_run(char *file, char **argv, char **envp)
{
	char           *path;
	unsigned int    split;
	int             savederrno;

	if (file[str_chr(file, '/')])
	{
		execve(file, argv, envp);
		return;
	}

	path = env_get("PATH");
	if (!path)
		path = "/bin:/usr/bin";

	savederrno = 0;
	for (;;)
	{
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
		if (errno != error_noent)
		{
			savederrno = errno;
			if ((errno != error_acces) && (errno != error_perm) && (errno != error_isdir))
				return;
		}

		if (!path[split])
		{
			if (savederrno)
				errno = savederrno;
			return;
		}
		path += split;
		path += 1;
	}
}

void
getversion_pathexec_run_c()
{
	static char    *x = "$Id: pathexec_run.c,v 1.2 2004-10-22 20:27:55+05:30 Cprogrammer Stab mbhangui $";

	x++;
}
