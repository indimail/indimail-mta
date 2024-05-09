/*
 * $Log: smtp_plugin.h,v $
 * Revision 1.3  2024-05-09 22:03:17+05:30  mbhangui
 * fix discarded-qualifier compiler warnings
 *
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
	int             (*mail_func) (const char *, const char *, char **);
	int             (*rcpt_func) (const char *, const char *, const char *, char **);
	int             (*data_func) (const char *, const char *, const char *, const char *, char **);
} PLUGIN;

#endif
