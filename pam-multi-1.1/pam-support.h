/* 
 * $Log: pam-support.h,v $
 * Revision 1.1  2009-10-06 13:49:57+05:30  Cprogrammer
 * Initial revision
 *
 *
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
 * Copyright (c) Alexey Mahotkin <alexm@hsys.msk.ru> 2002
 *
 * PAM support for checkpassword-pam
 *
*/

#ifndef PAM_SUPPORT_H_
#define PAM_SUPPORT_H_ 1

int auth_pam(const char *, const char *, const char *, int);

#endif /* PAM_SUPPORT_H_ */

