/*
 * Copyright 1998 - 2001 Double Precision, Inc.  See COPYING for
 * distribution information.
 */

#if	HAVE_CONFIG_H
#include	"config.h"
#endif
#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	<signal.h>
#include	<errno.h>
#if	HAVE_UNISTD_H
#include	<unistd.h>
#endif
#include	"auth.h"
#include	"authmod.h"
#include	"authwait.h"
#include	"authstaticlist.h"
#include	"debug.h"

#if	HAVE_SECURITY_PAM_APPL_H
#include	<security/pam_appl.h>
#endif

#if	HAVE_PAM_PAM_APPL_H
#include	<Pam/pam_appl.h>
#endif

static const char rcsid[] = "$Id: authpam.c,v 1.17 2004/04/18 15:54:38 mrsam Exp $";

static const char *pam_username, *pam_password, *pam_service;

extern void    auth_pwd_enumerate(void (*cb_func)
				   (const char *name, uid_t uid, gid_t gid, const char *homedir, const char *maildir, void *void_arg),
				   void *void_arg);

static int
pam_conv(int num_msg, const struct pam_message **msg, struct pam_response **resp, void *appdata_ptr)
{
	int             i = 0;
	struct pam_response *repl = NULL;

	if (!(repl = malloc(sizeof(struct pam_response) * num_msg)))
		return PAM_CONV_ERR;
	for (i = 0; i < num_msg; i++)
		switch (msg[i]->msg_style)
		{
		case PAM_PROMPT_ECHO_ON:
			repl[i].resp_retcode = PAM_SUCCESS;
			repl[i].resp = strdup(pam_username);
			if (!repl[i].resp)
			{
				perror("strdup");
				authexit(1);
			}
			break;
		case PAM_PROMPT_ECHO_OFF:
			repl[i].resp_retcode = PAM_SUCCESS;
			repl[i].resp = strdup(pam_password);
			if (!repl[i].resp)
			{
				perror("strdup");
				authexit(1);
			}
			break;
		case PAM_TEXT_INFO:
		case PAM_ERROR_MSG:
			write(2, msg[i]->msg, strlen(msg[i]->msg));
			write(2, "\n", 1);
			repl[i].resp_retcode = PAM_SUCCESS;
			repl[i].resp = NULL;
			break;
		default:
			free(repl);
			return PAM_CONV_ERR;
		}

	*resp = repl;
	return PAM_SUCCESS;
}

static struct pam_conv conv = {
	pam_conv,
	NULL
};

static int
dopam(pam_handle_t ** pamh)
{
	int             retval;

	dprintf("pam_service=%s, pam_username=%s", pam_service ? pam_service : "<null>", pam_username ? pam_username : "<null>");

	retval = pam_start(pam_service, pam_username, &conv, pamh);
	if (retval != PAM_SUCCESS)
		dprintf("pam_start failed, result %d [Hint: bad PAM configuration?][%s]", retval, pam_strerror(*pamh, retval));

#if 0
	if (retval == PAM_SUCCESS)
	{
		retval = pam_set_item(*pamh, PAM_AUTHTOK, pam_password);
		if (retval != PAM_SUCCESS)
			dprintf("pam_set_item failed, result %d", retval);
	}
#endif

	if (retval == PAM_SUCCESS)
	{
		retval = pam_authenticate(*pamh, 0);
		if (retval != PAM_SUCCESS)
			dprintf("pam_authenticate: %s", pam_strerror(*pamh, retval));
	}
#if 0

#if	HAVE_PAM_SETCRED
	if (retval == PAM_SUCCESS)
	{
		retval = pam_setcred(*pamh, PAM_ESTABLISH_CRED);
		if (retval != PAM_SUCCESS)
			dprintf("pam_setcred failed, result %d", retval);
	}
#endif

	if (retval == PAM_SUCCESS)
	{
		retval = pam_acct_mgmt(*pamh, 0);
		if (retval != PAM_SUCCESS)
			dprintf("pam_acct_mgmt failed, result %d", retval);
	}
#endif
	if (retval == PAM_SUCCESS)
		dprintf("dopam successful");

	return (retval);
}

struct callback_info
{
	char           *username;
	int             issession;
	void            (*callback_func) (struct authinfo *, void *);
	void           *callback_arg;
};

static int
callback_pam(struct authinfo *a, void *argptr)
{
	struct callback_info *ci = (struct callback_info *) argptr;
	pam_handle_t   *pamh = NULL;
	int             pipefd[2];
	int             retval;
	pid_t           p;
	int             waitstat;
	char           *s;

	s = strdup(a->sysusername);
	if (!s)
	{
		perror("malloc");
		return (1);
	}

	if (!ci->issession &&		/* Thankfully, no session voodoo needed this time */
		ci->callback_func == 0)
	{
		retval = dopam(&pamh);

		if (retval == PAM_SUCCESS)
		{
			if (pam_end(pamh, retval) != PAM_SUCCESS)
				dprintf("pam_end: %s", pam_strerror(pamh, retval));
			if (ci->callback_func == 0)
				authsuccess(a->homedir, s, 0, &a->sysgroupid, a->address, a->fullname);

			ci->username = s;
			putenv("MAILDIR=");
			return (0);
		}
		free(s);
		if (pam_end(pamh, retval) != PAM_SUCCESS)
			dprintf("pam_end: %s", pam_strerror(pamh, retval));
		return (-1);
	}

/*
 * OK, in order to transparently support PAM sessions inside this
 * authentication module, what we need to do is to fork(), and let
 * the child run in its parent place.  Once the child process exits,
 * the parent calls pam_end_session, and clears the PAM library.
 *
 * This means that upon return from auth_pam(), your process ID
 * might've changed!!!
 *
 * However, if the authentication fails, we can simply exit, without
 * running as a child process.
 *
 * Additionally, the PAM library might allocate some resources that
 * authenticated clients should not access.  Therefore, we fork
 * *before* we try to authenticate.  If the authentication succeeds,
 * the child process will run in the parent's place.  The child
 * process waits until the parent tells it whether the authentication
 * worked.  If it worked, the child keeps running.  If not, the child
 * exits, which the parent waits for.
 *
 * The authentication status is communicated to the child process via
 * a pipe.
 */
	if (pipe(pipefd) < 0)
	{
		perror("pipe");
		free(s);
		return (1);
	}

	if ((p = fork()) == -1)
	{
		perror("fork");
		free(s);
		return (1);
	}

	if (p == 0)
	{
		char            dummy;

		if (ci->callback_func)	/* PAM memory leak */
		{
			close(pipefd[0]);
			retval = dopam(&pamh);
			if (retval == PAM_SUCCESS)
				write(pipefd[1], "", 1);
			close(pipefd[1]);
			_exit(0);
		}

		close(pipefd[1]);

		if (read(pipefd[0], &dummy, 1) != 1 || dummy)
			authexit(1);		/* Authentication failed by parent */
		close(pipefd[0]);

		putenv("MAILDIR=");
		authsuccess(a->homedir, s, 0, &a->sysgroupid, a->address, a->fullname);
		ci->username = s;
		if (ci->callback_func)
		{
			a->address = s;
			(*ci->callback_func) (a, ci->callback_arg);
		}
		return (0);
	}

	if (ci->callback_func)
	{
		char            buf[1];

		close(pipefd[1]);
		while (wait(&waitstat) != p)
			;
		if (read(pipefd[0], buf, 1) > 0)
		{
			close(pipefd[0]);
			a->address = s;
			ci->username = s;
			(*ci->callback_func) (a, ci->callback_arg);
			return (0);
		}
		close(pipefd[0]);
		free(s);
		return (-1);
	}

	free(s);
	close(pipefd[0]);

	retval = dopam(&pamh);
	if (retval == PAM_SUCCESS)
	{
		if ((retval = pam_open_session(pamh, 0)) != PAM_SUCCESS)
			dprintf("pam_open_session: %s", pam_strerror(pamh, retval));
	}
	if (retval != PAM_SUCCESS)
	{
		if (pam_end(pamh, retval) != PAM_SUCCESS)
			dprintf("pam_end: %s", pam_strerror(pamh, retval));

		/*- Wait for child to terminate */
		close(pipefd[1]);		/* Tell the child to shut down */
		while (wait(&waitstat) != p)
			;
		return (-1);
	}
	/*- Tell child process to run in authenticated state */
	write(pipefd[1], "", 1);
	close(pipefd[1]);
	/*- Wait for child process to finish */
	while (wait(&waitstat) != p)
		;
	retval = pam_close_session(pamh, 0);
	if (retval != PAM_SUCCESS)
		dprintf("pam_close_session: %s", pam_strerror(pamh, retval));
	if (pam_end(pamh, retval) != PAM_SUCCESS)
		dprintf("pam_end: %s", pam_strerror(pamh, retval));
	if (WIFEXITED(waitstat))
		authexit(WEXITSTATUS(waitstat));
	authexit(255);
	return (1);
}

extern int      auth_pam_pre(const char *userid, const char *service, int (*callback) (struct authinfo *, void *), void *arg);

char           *
auth_pam(const char *service, const char *type, char *authdata, int issession, void (*callback_func) (struct authinfo *, void *),
		 void *callback_arg)
{
	struct callback_info ci;
	int             rc;

	if (strcmp(type, AUTHTYPE_LOGIN))
	{
		dprintf("authpam only handles authtype=" AUTHTYPE_LOGIN);
		errno = EPERM;
		return (0);
	}

	if ((pam_username = strtok(authdata, "\n")) == 0 || (pam_password = strtok(0, "\n")) == 0)
	{
		dprintf("incomplete username or missing password");
		errno = EPERM;
		return (0);
	}
	pam_service = service;
	ci.issession = issession;
	ci.callback_func = callback_func;
	ci.callback_arg = callback_arg;
	rc = auth_pam_pre(pam_username, service, &callback_pam, &ci);
	if (rc)
		return (0);
	return (ci.username);
}

static void
auth_pam_cleanup()
{
}

struct authstaticinfo authpam_info = {
	"authpam",
	auth_pam,
	auth_pam_pre,
	auth_pam_cleanup,
	auth_syspasswd,
	NULL,
	auth_pwd_enumerate
};
