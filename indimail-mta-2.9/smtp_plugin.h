/*
 * $Log: smtp_plugin.h,v $
 * Revision 1.2  2011-04-13 20:44:52+05:30  Cprogrammer
 * added data function
 *
 * Revision 1.1  2011-04-13 19:43:55+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifndef SMTP_PLUGIN_H
#define SMTP_PLUGIN_H
typedef struct
{
	int             (*mail_func) (char *, char *, char **);
	int             (*rcpt_func) (char *, char *, char *, char **);
	int             (*data_func) (char *, char *, char *, char *, char **);
} PLUGIN;

#endif
