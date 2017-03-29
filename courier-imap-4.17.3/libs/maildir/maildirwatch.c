/*
** Copyright 2002-2009 Double Precision, Inc.
** See COPYING for distribution information.
*/

#include "config.h"
#include "maildirwatch.h"

#include <unistd.h>
#include <string.h>
#include <limits.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <signal.h>
#include <sys/signal.h>

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif


#if HAVE_FAM
static struct maildirwatch_fam *maildirwatch_currentfam;

static void alarm_handler(int signum)
{
	static const char msg[]=
		"Timeout initializing the FAM library. Your FAM library is broken.\n";

	write(2, msg, sizeof(msg)-1);
	kill(getpid(), SIGKILL);
}
#endif

struct maildirwatch *maildirwatch_alloc(const char *maildir)
{
	char wd[PATH_MAX];
	struct maildirwatch *w;

	if (maildir == 0 || *maildir == 0)
		maildir=".";

	if (getcwd(wd, sizeof(wd)-1) == NULL)
		return NULL;

	if (*maildir == '/')
		wd[0]=0;
	else
		strcat(wd, "/");

	if ((w=malloc(sizeof(struct maildirwatch))) == NULL)
		return NULL;

	if ((w->maildir=malloc(strlen(wd)+strlen(maildir)+1)) == NULL)
	{
		free(w);
		return NULL;
	}

	strcat(strcpy(w->maildir, wd), maildir);

#if HAVE_FAM
	if (!maildirwatch_currentfam)
	{
		if ((maildirwatch_currentfam
		     =malloc(sizeof(*maildirwatch_currentfam))) != NULL)
		{
			maildirwatch_currentfam->broken=0;
			maildirwatch_currentfam->refcnt=0;

			signal(SIGALRM, alarm_handler);
			alarm(15);
			if (FAMOpen(&maildirwatch_currentfam->fc) < 0)
			{
				errno=EIO;
				free(maildirwatch_currentfam);
				maildirwatch_currentfam=NULL;
			}
			alarm(0);
			signal(SIGALRM, SIG_DFL);
		}
	}

	if (!maildirwatch_currentfam)
	{
		free(w->maildir);
		free(w);
		w=NULL;
	}
	else
	{
		w->fam=maildirwatch_currentfam;
		++w->fam->refcnt;
	}
#endif
	return w;
}

void maildirwatch_free(struct maildirwatch *w)
{
#if HAVE_FAM
	if (--w->fam->refcnt == 0)
	{
		w->fam->broken=1;
		if (maildirwatch_currentfam &&
		    maildirwatch_currentfam->broken)
		{
			/*
			** Last reference to the current FAM connection,
			** keep it active.
			*/

			w->fam->broken=0;
		}
		else /* Some other connection, with no more refs */
		{
			FAMClose(&w->fam->fc);
			free(w->fam);
		}
	}
#endif

	free(w->maildir);
	free(w);
}

void maildirwatch_cleanup()
{
#if HAVE_FAM

	if (maildirwatch_currentfam && maildirwatch_currentfam->refcnt == 0)
	{
		FAMClose(&maildirwatch_currentfam->fc);
		free(maildirwatch_currentfam);
		maildirwatch_currentfam=NULL;
	}
#endif
}

#if HAVE_FAM
static void maildirwatch_fambroken(struct maildirwatch *w)
{
	w->fam->broken=1;

	if (maildirwatch_currentfam && maildirwatch_currentfam->broken)
		maildirwatch_currentfam=NULL;
	/* Broke the current connection, create another one, next time. */

}

/*
** If the current connection is marked as broken, try to reconnect.
*/

static void maildirwatch_famunbreak(struct maildirwatch *w)
{
	struct maildirwatch *cpy;

	if (!w->fam->broken)
		return;

	if ((cpy=maildirwatch_alloc(w->maildir)) == NULL)
		return;

	/*
	** maildirwatch_alloc succeeds only with a good connection.
	** If this is the last reference to the broken connection, close it.
	*/

	if (--w->fam->refcnt == 0)
	{
		FAMClose(&w->fam->fc);
		free(w->fam);
	}

	w->fam=cpy->fam;
	++w->fam->refcnt;

	maildirwatch_free(cpy);
}

static int waitEvent(struct maildirwatch *w)
{
	int fd;
	fd_set r;
	struct timeval tv;
	time_t now2;

	int rc;

	while ((rc=FAMPending(&w->fam->fc)) == 0)
	{
		if (w->now >= w->timeout)
			return 0;

		fd=FAMCONNECTION_GETFD(&w->fam->fc);

		FD_ZERO(&r);
		FD_SET(fd, &r);

		tv.tv_sec= w->timeout - w->now;
		tv.tv_usec=0;

		select(fd+1, &r, NULL, NULL, &tv);
		now2=time(NULL);

		if (now2 < w->now)
			return 0; /* System clock changed */

		w->now=now2;
	}

	return rc;
}
#endif


int maildirwatch_unlock(struct maildirwatch *w, int nseconds)
{
#if HAVE_FAM
	FAMRequest fr;
	FAMEvent fe;
	int cancelled=0;
	char *p;

	if (w->fam->broken)
	{
		errno=EIO;
		return -1;
	}

	p=malloc(strlen(w->maildir)+ sizeof("/" WATCHDOTLOCK));

	if (!p)
		return -1;

	strcat(strcpy(p, w->maildir), "/" WATCHDOTLOCK);

	errno=EIO;
	if (FAMMonitorFile(&w->fam->fc, p, &fr, NULL) < 0)
	{
		free(p);
		fprintf(stderr, "ERR:FAMMonitorFile: %s\n",
			strerror(errno));
		return -1;
	}
	free(p);

	if (nseconds < 0)
		nseconds=0;

	time(&w->now);

	w->timeout=w->now + nseconds;

	for (;;)
	{
		if (waitEvent(w) != 1)
		{
			errno=EIO;

			if (!cancelled && FAMCancelMonitor(&w->fam->fc, &fr) == 0)
			{
				w->timeout=w->now+15;
				cancelled=1;
				continue;
			}

			if (!cancelled)
				fprintf(stderr, "ERR:FAMCancelMonitor: %s\n",
					strerror(errno));

			maildirwatch_fambroken(w);
			break;
		}

		errno=EIO;

		if (FAMNextEvent(&w->fam->fc, &fe) != 1)
		{
			fprintf(stderr, "ERR:FAMNextEvent: %s\n",
				strerror(errno));
			maildirwatch_fambroken(w);
			break;
		}

		if (fe.fr.reqnum != fr.reqnum)
			continue;

		if (fe.code == FAMDeleted && !cancelled)
		{
			errno=EIO;
			if (FAMCancelMonitor(&w->fam->fc, &fr) == 0)
			{
				w->timeout=w->now+15;
				cancelled=1;
				continue;
			}
			fprintf(stderr, "ERR:FAMCancelMonitor: %s\n",
				strerror(errno));
			maildirwatch_fambroken(w);
			break;
		}

		if (fe.code == FAMAcknowledge)
			break;
	}

	if (w->fam->broken)
		return -1;

	return 0;
#else
	return -1;
#endif
}

#define DIRCNT 3

int maildirwatch_start(struct maildirwatch *w,
		       struct maildirwatch_contents *mc)
{
	mc->w=w;

	time(&w->now);
	w->timeout = w->now + 60;

#if HAVE_FAM

	maildirwatch_famunbreak(w);

	if (w->fam->broken)
	{
		errno=EIO;
		return (1);
	}

	{
		char *s=malloc(strlen(w->maildir)
			       +sizeof("/" KEYWORDDIR));

		if (!s)
			return (-1);

		strcat(strcpy(s, w->maildir), "/new");

		mc->endexists_received=0;
		mc->ack_received=0;
		mc->cancelled=0;

		errno=EIO;

		if (FAMMonitorDirectory(&w->fam->fc, s, &mc->new_req, NULL) < 0)
		{
			fprintf(stderr, "ERR:"
				"FAMMonitorDirectory(%s) failed: %s\n",
				s, strerror(errno));
			free(s);
			errno=EIO;
			return (-1);
		}

		strcat(strcpy(s, w->maildir), "/cur");
		errno=EIO;

		if (FAMMonitorDirectory(&w->fam->fc, s, &mc->cur_req, NULL) < 0)
		{
			fprintf(stderr, "ERR:"
				"FAMMonitorDirectory(%s) failed: %s\n",
				s, strerror(errno));

			errno=EIO;

			if (FAMCancelMonitor(&mc->w->fam->fc, &mc->new_req) < 0)
			{
				free(s);
				maildirwatch_fambroken(w);
				fprintf(stderr, "ERR:FAMCancelMonitor: %s\n",
					strerror(errno));
				errno=EIO;
				return (-1);
			}
			mc->cancelled=1;
			mc->ack_received=2;
		}

		strcat(strcpy(s, w->maildir), "/" KEYWORDDIR);
		errno=EIO;

		if (FAMMonitorDirectory(&w->fam->fc, s,
					&mc->courierimapkeywords_req, NULL)<0)
		{
			fprintf(stderr, "ERR:"
				"FAMMonitorDirectory(%s) failed: %s\n",
				s, strerror(errno));

			errno=EIO;

			if (FAMCancelMonitor(&mc->w->fam->fc, &mc->new_req)<0)
			{
				free(s);
				maildirwatch_fambroken(w);
				fprintf(stderr, "ERR:FAMCancelMonitor: %s\n",
					strerror(errno));
				errno=EIO;
				return (-1);
			}

			errno=EIO;

			if (FAMCancelMonitor(&mc->w->fam->fc, &mc->cur_req)<0)
			{
				free(s);
				maildirwatch_fambroken(w);
				fprintf(stderr, "ERR:FAMCancelMonitor: %s\n",
					strerror(errno));
				errno=EIO;
				return (-1);
			}

			mc->cancelled=1;
			mc->ack_received=1;
		}

		free(s);
	}
	return 0;
#else
	return 1;
#endif
}

#define CANCEL(ww) \
	errno=EIO; if (FAMCancelMonitor(&w->fam->fc, \
			     &ww->new_req) || \
	    FAMCancelMonitor(&w->fam->fc, \
			     &ww->cur_req) || \
	    FAMCancelMonitor(&w->fam->fc, \
			     &ww->courierimapkeywords_req)) \
	{\
		maildirwatch_fambroken(w); \
		fprintf(stderr, \
			"ERR:FAMCancelMonitor: %s\n", \
			strerror(errno)); \
		return (-1); \
	}

int maildirwatch_started(struct maildirwatch_contents *mc,
			 int *fdret)
{
#if HAVE_FAM
	struct maildirwatch *w=mc->w;

	if (w->fam->broken)
		return (1);

	*fdret=FAMCONNECTION_GETFD(&w->fam->fc);

	while (FAMPending(&w->fam->fc))
	{
		FAMEvent fe;

		errno=EIO;

		if (FAMNextEvent(&w->fam->fc, &fe) != 1)
		{
			fprintf(stderr, "ERR:FAMNextEvent: %s\n",
				strerror(errno));
			maildirwatch_fambroken(w);
			return (-1);
		}

		switch (fe.code) {
		case FAMDeleted:
			if (!mc->cancelled)
			{
				mc->cancelled=1;
				CANCEL(mc);
			}
			break;
		case FAMAcknowledge:
			if (++mc->ack_received >= DIRCNT)
				return -1;
			break;
		case FAMEndExist:
			++mc->endexists_received;
			break;
		default:
			break;
		}
	}

	return (mc->endexists_received >= DIRCNT && mc->ack_received == 0);
#else
	*fdret= -1;

	return 1;
#endif
}

int maildirwatch_check(struct maildirwatch_contents *mc,
		       int *changed,
		       int *fdret,
		       int *timeout)
{
	struct maildirwatch *w=mc->w;
	time_t curTime;

	*changed=0;
	*fdret=-1;

	curTime=time(NULL);

	if (curTime < w->now)
		w->timeout=curTime; /* System clock changed */
	w->now=curTime;

#if HAVE_FAM

	if (!w->fam->broken)
	{
		*fdret=FAMCONNECTION_GETFD(&w->fam->fc);

		while (FAMPending(&w->fam->fc))
		{
			FAMEvent fe;

			errno=EIO;

			if (FAMNextEvent(&w->fam->fc, &fe) != 1)
			{
				fprintf(stderr, "ERR:FAMNextEvent: %s\n",
					strerror(errno));
				maildirwatch_fambroken(w);
				return (-1);
			}

			switch (fe.code) {
			case FAMDeleted:
			case FAMCreated:
			case FAMMoved:
				if (!mc->cancelled)
				{
					mc->cancelled=1;
					CANCEL(mc);
				}
				break;
			case FAMAcknowledge:
				++mc->ack_received;
			default:
				break;
			}
		}

		*changed=mc->ack_received >= DIRCNT;
		*timeout=60 * 60;
		return 0;
	}
#endif
	*timeout=60;

 	if ( (*changed= w->now >= w->timeout) != 0)
		w->timeout = w->now + 60;
	return 0;
}

void maildirwatch_end(struct maildirwatch_contents *mc)
{
#if HAVE_FAM
	struct maildirwatch *w=mc->w;

	if (!w->fam->broken)
	{
		if (!mc->cancelled)
		{
			mc->cancelled=1;

#define return(x)
			CANCEL(mc);
#undef return
		}
	}

	while (!w->fam->broken && mc->ack_received < DIRCNT)
	{
		FAMEvent fe;

		time(&w->now);
		w->timeout=w->now + 15;

		errno=EIO;

		if (waitEvent(w) != 1)
		{
			fprintf(stderr, "ERR:FAMPending: timeout\n");
			maildirwatch_fambroken(w);
			break;
		}

		errno=EIO;

		if (FAMNextEvent(&w->fam->fc, &fe) != 1)
		{
			fprintf(stderr, "ERR:FAMNextEvent: %s\n",
				strerror(errno));
			maildirwatch_fambroken(w);
			break;
		}

		if (fe.code == FAMAcknowledge)
			++mc->ack_received;
	}
#endif
}
