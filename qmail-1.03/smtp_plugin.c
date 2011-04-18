/*
 * $Log: $
 */
#include "smtp_plugin.h"

/*
 * Template for writing qmail-smtpd plugins
 *
 * returning 1 from the functions from_plug & rcpt_plug
 * will case qmail-smtpd to return from smtp_mail & smtp_rcpt
 * function and display the message assigned to variable 'mesg'.
 *
 * return 0 will cause smtp_mail & smtp_rcpt to proceed ahead
 */

int
from_plug_1(char *remoteip, char *from, char **mesg)
{
	*mesg = "530 denied by plugin (#5.7.1)\r\n";
	return (1);
}

int
rcpt_plug_1(char *remoteip, char *from, char *rcpt, char **mesg)
{
	*mesg = "530 denied by plugin (#5.7.1)\r\n";
	return (0);
}

int
data_plug_1(char *local, char *remoteip, char *remotehost, char *remoteinfo, char **mesg)
{
	return (0);
}

PLUGIN         *
plugin_init()
{
	static PLUGIN   plug;
	PLUGIN         *ptr;

	ptr = &plug;
	ptr->mail_func = from_plug_1;
	ptr->rcpt_func = rcpt_plug_1;
	ptr->data_func = data_plug_1;
	return &plug;
}
