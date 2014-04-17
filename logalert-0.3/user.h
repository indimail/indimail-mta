/*
 * $Log: user.h,v $
 * Revision 1.2  2014-04-17 11:29:42+05:30  Cprogrammer
 * added grp.h
 * added username argument to set_user()
 *
 * Revision 1.1  2013-05-15 00:15:06+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifndef USER_H
#define USER_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include<stdio.h>
#ifdef HAVE_STDLIB_H
#include<stdlib.h>
#endif
#ifdef HAVE_PWD_H
#include<pwd.h>
#endif
#ifdef HAVE_GRP_H
#include<grp.h>
#endif


uid_t           get_user_id(char *);
gid_t           get_guser_id(char *);
void            set_user(uid_t, gid_t, char *);

#endif
