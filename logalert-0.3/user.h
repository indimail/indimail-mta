/*
 * $Log: user.h,v $
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


uid_t           get_user_id(char *username);
gid_t           get_guser_id(char *username);
void            set_user(uid_t uid, gid_t gid);

#endif
