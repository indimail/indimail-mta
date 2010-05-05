/*
 * $Log: pam-support.c,v $
 * Revision 1.3  2010-05-05 15:21:34+05:30  Cprogrammer
 * added setting of credentials
 *
 * Revision 1.2  2009-10-07 11:58:08+05:30  Cprogrammer
 * removed stray debug statements
 *
 * Revision 1.1  2009-10-07 10:19:03+05:30  Cprogrammer
 * Initial revision
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2, or (at
 * your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 * 
 * Copyright (c) Alexey Mahotkin <alexm@hsys.msk.ru> 2002-2004
 * 
 * PAM support for checkpassword-pam
 * Modified by Manvendra Bhangui <mbhangui@gmail.com>
 * 
 */


#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#ifdef HAVE_SECURITY_PAM_APPL_H
#include <security/pam_appl.h>
#endif
#ifdef HAVE_PAM_PAM_APPL_H
#include <pam/pam_appl.h>
#endif
#ifdef HAVE_PAM_APPL_H
#include <pam_appl.h>
#endif

#ifdef HAVE_PAM
static const char *pam_username, *pam_password;
static int      _debug = 0;

static int
conversation(int num_msg, const struct pam_message **msg, struct pam_response **resp,
	void *appdata_ptr)
{
	int             i = 0;
	char           *style = 0;
	struct pam_response *repl = NULL;

	if (!(repl = malloc(sizeof(struct pam_response) * num_msg)))
		return PAM_CONV_ERR;
	for (i = 0; i < num_msg; i++) {
		const struct pam_message *msgs = msg[i];
		switch (msg[i]->msg_style)
		{
		case PAM_PROMPT_ECHO_ON:
			style = "PAM_PROMPT_ECHO_ON";
			repl[i].resp_retcode = PAM_SUCCESS;
			repl[i].resp = strdup(pam_username);
			if (!repl[i].resp)
			{
				perror("strdup");
				return PAM_CONV_ERR;
			}
			break;
		case PAM_PROMPT_ECHO_OFF:
			style = "PAM_PROMPT_ECHO_OFF";
			repl[i].resp_retcode = PAM_SUCCESS;
			repl[i].resp = strdup(pam_password);
			if (!repl[i].resp)
			{
				perror("strdup");
				return PAM_CONV_ERR;
			}
			break;
		case PAM_TEXT_INFO:
		case PAM_ERROR_MSG:
			style = msg[i]->msg_style == PAM_TEXT_INFO ?  "PAM_TEXT_INFO" : "PAM_ERROR_MSG";
			write(2, msg[i]->msg, strlen(msg[i]->msg));
			write(2, "\n", 1);
			repl[i].resp_retcode = PAM_SUCCESS;
			repl[i].resp = NULL;
			break;
		default:
			free(repl);
			return PAM_CONV_ERR;
		}
		if (_debug)
			fprintf(stderr, "conversation(): msg[%d], style %s, msg = \"%s\"\n",
				i, style, msgs->msg);
	}
	*resp = repl;
	return PAM_SUCCESS;
}

int
auth_pam(const char *service_name, const char *username, const char *password, int debug)
{
	/*- struct pam_conv pam_conversation = { conversation, NULL }; -*/
	struct pam_conv conv = {conversation, NULL };
	pam_handle_t   *pamh;
	int             retval;
	char           *remoteip;

	pam_username = username;
	pam_password = password;
	/*- initialize the PAM library */
	if (debug)
	{
		fprintf(stderr, "Initializing PAM library using service name '%s'\n", service_name);
		_debug = debug;
	}
	if ((retval = pam_start(service_name, username, &conv, &pamh)) != PAM_SUCCESS) {
		fprintf(stderr, "Initialization failed: %s\n", pam_strerror(pamh, retval));
		return 111;
	}
	/*- provided by tcpserver */
	remoteip = getenv("TCPREMOTEIP");
	if (remoteip) /*- we don't care if this succeeds or not */
		pam_set_item(pamh, PAM_RHOST, remoteip);
	/*- Authenticate the user */
	if (debug)
		fprintf(stderr, "Authenticating\n");
	if ((retval = pam_authenticate(pamh, PAM_DISALLOW_NULL_AUTHTOK)) != PAM_SUCCESS) {
		/*
		 * usually PAM itself logs auth failures, but we need to see
		 * how it looks from our side 
		 */
		if (debug)
			fprintf(stderr, "Authentication failed: %s\n", pam_strerror(pamh, retval));
		pam_end(pamh, retval);
		return 1;
	}
	if (debug)
		fprintf(stderr, "doing Account management\n");
	if ((retval = pam_acct_mgmt(pamh, 0)) != PAM_SUCCESS) {
		fprintf(stderr, "PAM account management failed: %s\n", pam_strerror(pamh, retval));
		pam_end(pamh, retval);
		return 1;
	}
	/*
	 * Not sure how this works. If someone can enlighten me
	 * it would be great mbhangui@gmail.com
	 */
	if (debug)
		fprintf(stderr, "Setting PAM credentials\n");
	if ((retval = pam_setcred(pamh, PAM_ESTABLISH_CRED)) != PAM_SUCCESS){
		fprintf(stderr, "Setting credentials failed: %s", pam_strerror(pamh, retval));
		return 1;
	}
	if (debug)
		fprintf(stderr, "opening PAM session\n");
	if ((retval = pam_open_session(pamh, PAM_SILENT)) != PAM_SUCCESS) {
		fprintf(stderr, "Session opening failed: %s\n", pam_strerror(pamh, retval));
		return 1;
	}
	if (debug)
		fprintf(stderr, "closing PAM session\n");
	if ((retval = pam_close_session(pamh, PAM_SILENT)) != PAM_SUCCESS) {
		fprintf(stderr, "Session closing failed: %s\n", pam_strerror(pamh, retval));
		return 1;
	}
	if (debug)
		fprintf(stderr, "Deleting credentials\n");
	if ((retval = pam_setcred(pamh, PAM_DELETE_CRED)) != PAM_SUCCESS){
		fprintf(stderr, "Setting credentials failed: %s", pam_strerror(pamh, retval));
		return 1;
	}
	/*- terminate the PAM library */
	if (debug)
		fprintf(stderr, "Terminating PAM library\n");
	if ((retval = pam_end(pamh, retval)) != PAM_SUCCESS) {
		fprintf(stderr, "Terminating PAM failed: %s\n", pam_strerror(pamh, retval));
		return 1;
	}
	return 0;
}
#endif
