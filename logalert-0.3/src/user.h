#ifndef USER_H
#define USER_H

#include "config.h"

#include<stdio.h>
#include<stdlib.h>
//#include<sys/types.h>
#include<pwd.h>


uid_t get_user_id(char *username);
gid_t get_guser_id(char *username);
void set_user(uid_t uid, gid_t gid);

#endif
