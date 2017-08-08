/*
** Copyright 2003 Double Precision, Inc.
** See COPYING for distribution information.
*/

#include "config.h"
#include "maildirwatch.h"
#include "maildircreate.h"
#include "liblock/config.h"
#include "liblock/liblock.h"
#include "liblock/mail.h"
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>


/*
** Courier-IMAP compatible maildir lock.
*/

char *maildir_lock(const char *dir, struct maildirwatch *w,
		   int *tryAnyway)
{
	struct maildir_tmpcreate_info createInfo;
	char *tmpname;
	char *newname;
	int rc;

	*tryAnyway=0;

	maildir_tmpcreate_init(&createInfo);

	createInfo.maildir=dir;
	createInfo.uniq="courierlock";
	createInfo.hostname=getenv("HOSTNAME");
	createInfo.doordie=1;

	if ((rc=maildir_tmpcreate_fd(&createInfo)) < 0)
		return NULL;
	close(rc);

	tmpname=createInfo.tmpname;
	newname=createInfo.newname;

	createInfo.tmpname=NULL;
	createInfo.newname=NULL;
	maildir_tmpcreate_free(&createInfo);

	/* HACK: newname now contains: ".../new/filename" */

	strcpy(strrchr(newname, '/')-3, WATCHDOTLOCK);

	while (ll_dotlock(newname, tmpname, 120) < 0)
	{
		if (errno == EEXIST)
		{
			if (w == NULL || maildirwatch_unlock(w, 120) < 0)
				sleep(1);
			continue;
		}

		if (errno == EAGAIN)
		{
			unlink(newname);
			sleep(5);
			continue;
		}

		free(newname);
		newname=NULL;
		*tryAnyway=1;
		break;
	}

	free(tmpname);

	return newname;
}
