#ifndef	outbox_h
#define	outbox_h

/*
** Copyright 2002 Double Precision, Inc.
** See COPYING for distribution information.
*/


int check_outbox(const char *message, const char *mailbox);
int is_outbox(const char *mailbox);
int imapd_sendmsg(const char *message, char **argv, void (*err_func)(char *));
const char *defaultSendFrom();

#endif
