/*
 * $Log: hfield.h,v $
 * Revision 1.4  2024-05-09 22:03:17+05:30  mbhangui
 * fix discarded-qualifier compiler warnings
 *
 * Revision 1.3  2004-10-11 13:54:20+05:30  Cprogrammer
 * added function prototypes
 *
 * Revision 1.2  2004-06-18 23:00:04+05:30  Cprogrammer
 * added RCS log
 *
 */
#ifndef HFIELD_H
#define HFIELD_H

#define H_SENDER 1
#define H_FROM 2
#define H_REPLYTO 3
#define H_TO 4
#define H_CC 5
#define H_BCC 6
#define H_DATE 7
#define H_MESSAGEID 8
#define H_SUBJECT 9
#define H_R_SENDER 10
#define H_R_FROM 11
#define H_R_REPLYTO 12
#define H_R_TO 13
#define H_R_CC 14
#define H_R_BCC 15
#define H_R_DATE 16
#define H_R_MESSAGEID 17
#define H_RETURNRECEIPTTO 18
#define H_ERRORSTO 19
#define H_APPARENTLYTO 20
#define H_RECEIVED 21
#define H_RETURNPATH 22
#define H_DELIVEREDTO 23
#define H_CONTENTLENGTH 24
#define H_CONTENTTYPE 25
#define H_CONTENTTRANSFERENCODING 26
#define H_NOTICEREQUESTEDUPONDELIVERYTO 27
#define H_MAILFOLLOWUPTO 28
#define H_NUM 29

unsigned int    hfield_skipname(const char *, int);
int             hfield_known(const char *, int);
int             hfield_valid(const char *, int);

#endif
