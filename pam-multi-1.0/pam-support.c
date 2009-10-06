/*
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
#ifdef DARWIN
#include <pam/pam_appl.h>
#else
#include <security/pam_appl.h>
#endif
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static const char *global_password;
static int         debug;

#ifdef HAVE_PAM
static int
conversation(int num_msg, const struct pam_message **msgs, struct pam_response **resp,
	void *appdata_ptr)
{
	int             i;
	struct pam_response *responses;
	(void) appdata_ptr;

	/*- safety check */
	if (num_msg <= 0) {
		fprintf(stderr, "Internal PAM error: num_msgs <= 0\n");
		return PAM_CONV_ERR;
	}
	/*- allocate array of responses */
	if (!(responses = calloc(num_msg, sizeof (struct pam_response)))) {
		fprintf(stderr, "Out of memory\n");
		return PAM_CONV_ERR;
	}
	for (i = 0; i < num_msg; i++) {
		const struct pam_message *msg = msgs[i];
		struct pam_response *response = &(responses[i]);
		char           *style = NULL;
		switch (msg->msg_style) {
		case PAM_PROMPT_ECHO_OFF:
			style = "PAM_PROMPT_ECHO_OFF";
			response->resp = strdup(global_password);
			if (!response->resp)
				return PAM_CONV_ERR;
			break;
		case PAM_PROMPT_ECHO_ON:
			style = "PAM_PROMPT_ECHO_ON";
			break;
		case PAM_ERROR_MSG:
			style = "PAM_ERROR_MSG";
			break;
		case PAM_TEXT_INFO:
			style = "PAM_TEXT_INFO";
			break;
		default:
			fprintf(stderr, "Internal error: invalid msg_style: %d\n", msg->msg_style);
			break;
		}
		if (debug)
			fprintf(stderr, "conversation(): msg[%d], style %s, msg = \"%s\"\n", i, style, msg->msg);
		response->resp_retcode = 0;
	} /*- for (i = 0; i < num_msg; i++) { */
	*resp = responses;
	return PAM_SUCCESS;
}

int
auth_pam(const char *service_name, const char *username, const char *password, int opt_debugging)
{
	struct pam_conv pam_conversation = { conversation, NULL };
	pam_handle_t   *pamh;
	int             retval;
	char           *remoteip;

	if (opt_debugging)
		debug = 1;
	/*- to be used later from conversation() */
	global_password = password;
	/*- initialize the PAM library */
	if (debug)
		fprintf(stderr, "Initializing PAM library using service name '%s'\n", service_name);
	if ((retval = pam_start(service_name, username, &pam_conversation, &pamh)) != PAM_SUCCESS) {
		fprintf(stderr, "Initialization failed: %s\n", pam_strerror(pamh, retval));
		return 111;
	}
	if (debug)
		fprintf(stderr, "PAM library initialization succeeded\n");
	/*- provided by tcpserver */
	remoteip = getenv("TCPREMOTEIP");
	if (remoteip) /*- we don't care if this succeeds or not */
		pam_set_item(pamh, PAM_RHOST, remoteip);
	/*- Authenticate the user */
	if ((retval = pam_authenticate(pamh, 0)) != PAM_SUCCESS) {
		/*
		 * usually PAM itself logs auth failures, but we need to see
		 * how it looks from our side 
		 */
		if (opt_debugging)
			fprintf(stderr, "Authentication failed: %s\n", pam_strerror(pamh, retval));
		return 1;
	}
	if (debug)
		fprintf(stderr, "Authentication passed\n");
	if ((retval = pam_acct_mgmt(pamh, 0)) != PAM_SUCCESS) {
		fprintf(stderr, "PAM account management failed: %s\n", pam_strerror(pamh, retval));
		return 1;
	}
	if (debug)
		fprintf(stderr, "Account management succeeded\n");
#if 0
	if ((retval = pam_setcred(pamh, PAM_ESTABLISH_CRED)) != PAM_SUCCESS){
		fprintf(stderr, "Setting credentials failed: %s", pam_strerror(pamh, retval));
		return 1;
	}
	if (debug)
		fprintf(stderr, "Setting PAM credentials succeeded\n");
	if ((retval = pam_open_session(pamh, PAM_SILENT)) != PAM_SUCCESS) {
		fprintf(stderr, "Session opening failed: %s\n", pam_strerror(pamh, retval));
		return 1;
	}
	if (debug)
		fprintf(stderr, "PAM session opened\n");
	if ((retval = pam_close_session(pamh, PAM_SILENT)) != PAM_SUCCESS) {
		fprintf(stderr, "Session closing failed: %s\n", pam_strerror(pamh, retval));
		return 1;
	}
	if (debug)
		fprintf(stderr, "PAM session closed\n");
#endif
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
