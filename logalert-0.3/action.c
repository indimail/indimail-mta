/*
 * $Log: action.c,v $
 * Revision 1.2  2014-01-30 00:25:18+05:30  Cprogrammer
 * fixed compilation warnings
 *
 * Revision 1.1  2013-05-15 00:29:40+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "config.h"
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#include "action.h"
#ifdef HAVE_PCRE_H
#include<pcre.h>
#endif


char           *
substitute_args(char *cmd, char **args_list, int nmatch)
{

	int             i;
	char            key[3];
	char           *new_cmd;
	unsigned int    ncmd_size;
	char           *cmd_copy;

	ncmd_size = strlen(cmd) + 1;
	new_cmd = (char *) malloc(ncmd_size);
	cmd_copy = strdup(cmd);

	for (i = nmatch - 1; i > 0 && args_list[i] != NULL; i--) {
		char           *p, *q;
		unsigned int    n;
		char           *temp, *temp2;

		memset(key, 0x0, 3);
		sprintf(key, "$%d", i);

		while (1) {
			p = strstr(cmd_copy, key);
			q = key;
			if (!p)
				break;
			*p = '\0';
			while (*q++ != '\0')
				p++;

			n = strlen(cmd_copy) + strlen(args_list[i]) + strlen(p) + 1;
			temp = strdup(cmd_copy);
			temp2 = strdup(p);
			if (ncmd_size < n) {
				ncmd_size = n;
				new_cmd = realloc(new_cmd, ncmd_size);
			}
			memset(new_cmd, 0x0, ncmd_size);
			snprintf(new_cmd, ncmd_size, "%s%s%s", temp, args_list[i], temp2);

			free(temp);
			free(temp2);
			cmd_copy = new_cmd;
		}

	}
	return new_cmd;
}


int
shell_exec(struct entry_conf *cur_cfg, char *line, int nmatch)
{

	int             fd;
	char           *cmd;

	switch (fork()) {
	case -1:
		return (-1);
	case 0:
		break;
	default:
		return 1;
	}
	if (!cur_cfg->verbose) {
	// redirect std[out|in|err] to oblivion
		fd = open_file_ro("/dev/null");
		(void) dup2(fd, STDIN_FILENO);
		(void) dup2(fd, STDOUT_FILENO);
		(void) dup2(fd, STDERR_FILENO);
		if (fd > 2)
			(void) close(fd);
	}
#ifdef HAVE_PCRE_H
	if (nmatch > 1) {

		char          **args_list;

		pcre_get_substring_list(line, cur_cfg->rg_sub, nmatch, (const char ***) &args_list);
		cmd = substitute_args(cur_cfg->cmd, args_list, nmatch);
		if (!cmd) {
			printf("[!] Error while creating match command\n");
			return 0;
		}

	} else
#endif
		cmd = cur_cfg->cmd;
	if (execl(SHELL, SHELL, SHELL_ARG, cmd, NULL) < 0) {
		perror("execl");
		exit(1);
	}
	/*- does not return */
	exit (1);
}
