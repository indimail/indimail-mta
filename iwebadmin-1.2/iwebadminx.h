/*
 * $Id: iwebadminx.h,v 1.1 2010-04-26 12:08:04+05:30 Cprogrammer Exp mbhangui $
 * Copyright (C) 1999-2004 Inter7 Internet Technologies, Inc. 
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 */
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>

extern char     Username[MAX_BUFF];
extern char     Domain[MAX_BUFF];
extern char     Password[MAX_BUFF];
extern char     Gecos[MAX_BUFF];
extern char     Quota[MAX_BUFF];
extern char     Time[MAX_BUFF];
extern char     ActionUser[MAX_BUFF];
extern char     Newu[MAX_BUFF];
extern char     Password1[MAX_BUFF];
extern char     Password2[MAX_BUFF];
extern char     Crypted[MAX_BUFF];
extern char     Alias[MAX_BUFF];
extern char     LineData[MAX_BUFF];
extern char     Action[MAX_BUFF];
extern char     Message[MAX_BIG_BUFF];
extern char     StatusMessage[MAX_BIG_BUFF];
extern int      CGIValues[256];
extern char     Pagenumber[MAX_BUFF];
extern char     SearchUser[MAX_BUFF];
extern time_t   Mytime;
extern char    *TmpCGI;
extern char     TmpBuf[MAX_BIG_BUFF];
extern char     TmpBuf1[MAX_BUFF];
extern char     TmpBuf2[MAX_BUFF];
extern char     TmpBuf3[MAX_BUFF];
extern char     TempBuf[MAX_BUFF];
extern int      Compressed;
extern FILE    *actout;
extern char    *html_text[MAX_LANG_STR + 1];

extern struct vlimits Limits;
extern int      num_of_mailinglist;
extern int      AdminType;
extern int      MaxPopAccounts;
extern int      MaxAliases;
extern int      MaxForwards;
extern int      MaxAutoResponders;
extern int      MaxMailingLists;

extern int      CallVmoduser;
extern int      DisablePOP;
extern int      DisableIMAP;
extern int      DisableDialup;
extern int      DisablePasswordChanging;
extern int      DisableWebmail;
extern int      DisableRelay;

extern int      CurPopAccounts;
extern int      CurForwards;
extern int      CurBlackholes;
extern int      CurAutoResponders;
extern int      CurMailingLists;

extern int      Uid;
extern int      Gid;
extern char     RealDir[156];

extern char     Lang[40];
