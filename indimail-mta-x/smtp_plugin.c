/*
 * $Log: smtp_plugin.c,v $
 * Revision 1.3  2024-05-09 22:03:17+05:30  mbhangui
 * fix discarded-qualifier compiler warnings
 *
 * Revision 1.2  2011-07-08 13:49:10+05:30  Cprogrammer
 * define stubs as static
 *
 * Revision 1.1  2011-06-29 21:56:13+05:30  Cprogrammer
 * Initial revision
 *
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

static int
from_plug_1(const char *remoteip, const char *from, char **mesg)
{
	*mesg = "530 denied by plugin (#5.7.1)\r\n";
	return (0);
}

static int
rcpt_plug_1(const char *remoteip, const char *from, const char *rcpt, char **mesg)
{
	*mesg = "530 denied by plugin (#5.7.1)\r\n";
	return (0);
}

static int
data_plug_1(const char *local, const char *remoteip, const char *remotehost, const char *remoteinfo, char **mesg)
{
	*mesg = "530 denied by plugin (#5.7.1)\r\n";
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
