#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <pwd.h>
#include <shadow.h>

int
main(int argc, char **argv)
{
	struct passwd *pw;
	struct spwd   *spw;
	int i;

	if (!getuid())
	{
		fprintf(stderr, "you must be root!!\n");
		return (1);
	}
	for (i = 1;i < argc;i++)
	{
		if (!(pw = getpwnam(argv[i])))
		{
			fprintf(stderr, "%s: No such user\n", argv[i]);
			return(1);
		}
		printf("%s:", pw->pw_name);
		if (!(spw = getspnam(argv[i])))
		{
			fprintf(stderr, "getspnam: %s: %s\n", argv[i], errno ? strerror(errno) : "not found in shadow");
			printf("%s:", pw->pw_passwd);
		} else
			printf("%s:", spw->sp_pwdp);
		printf("%d:%d:%s:%s:%s\n", pw->pw_uid, pw->pw_gid, pw->pw_gecos, pw->pw_dir, pw->pw_shell);
	}
	return(0);
}
