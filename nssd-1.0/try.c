#include <stdio.h>
#include <pwd.h>

int
main(int argc, char **argv)
{
	struct passwd *pw;
	int i;

	for (i = 1;i < argc;i++)
	{
		if (!(pw = getpwnam(argv[i])))
		{
			fprintf(stderr, "%s: No such user\n", argv[i]);
			return(1);
		}
		printf("%s\n", pw->pw_name);
		printf("%s\n", pw->pw_passwd);
		printf("%d\n", pw->pw_uid);
		printf("%d\n", pw->pw_gid);
		printf("%s\n", pw->pw_gecos);
		printf("%s\n", pw->pw_dir);
		printf("%s\n", pw->pw_shell);
	}
	return(0);
}
