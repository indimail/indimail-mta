/*
 * $Log: $
 */
#include <stdio.h>
#include <unistd.h>
#include "haslibrt.h"
#ifdef HASLIBRT
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <mqueue.h>
#endif
#include <sys/types.h>
#include <substdio.h>
#include <env.h>
#include <open.h>
#include <lock.h>
#include <stralloc.h>
#include <signal.h>
#include <sig.h>
#include <str.h>
#include <fmt.h>
#include <scan.h>
#include <alloc.h>
#include <error.h>
#include <strerr.h>
#include <wait.h>
#include <noreturn.h>
#include <getEnvConfig.h>
#ifdef HASLIBRT
#include <sgetopt.h>
#include "auto_uids.h"
#endif
#include "qscheduler.h"
#include "control.h"
#include "readsubdir.h"
#include "auto_qmail.h"
#include "auto_split.h"

static int      qstart, qcount;
static char    *qbase;
static stralloc envQueue = {0}, QueueBase = {0};
static int      flagexitasap = 0;
static char  **prog_argv;
#ifdef HASLIBRT
static int      qconf, qmax, qload;
static q_type   qtype = fixed;
static char    *msgbuf;
static int      msgbuflen;
static mqd_t    mq_sch = (mqd_t) -1;
static int      shm_conf = -1;
static int     *shm_queue;
static int      compat_mode;
#endif
static char     strnum1[FMT_ULONG], strnum2[FMT_ULONG];
static readsubdir todosubdir;
static char     ssoutbuf[512];
static substdio ssout = SUBSTDIO_FDBUF(write, 1, ssoutbuf, sizeof(ssoutbuf));
int             conf_split;
typedef struct
{
	pid_t           pid;
	unsigned int    queue_no;
	unsigned long   load;      /*- concurrency used percent */
} qtab;
static qtab    *queue_table;
static char    *(qsargs[]) = { "qmail-start", "-s", "./Mailbox", 0};
char           *(qfargs[]) = { "queue-fix", "-s", 0, 0, 0};

void            start_send(int queueNum, pid_t pid);

void
log_out(char *s)
{
	if (substdio_puts(&ssout, s) == -1)
		_exit(1);
}

void
log_outf(char *s)
{
	if (substdio_puts(&ssout, s) == -1)
		_exit(1);
	if (substdio_flush(&ssout) == -1)
		_exit(1);
}

void
log_announce(int pid, char *qdir, unsigned long load, int died)
{
	log_out("info: qscheduler: pid ");
	strnum1[fmt_ulong(strnum1, pid)] = 0;
	log_out(strnum1);
	log_out(", queue ");
	log_out(qdir);
	log_out(", load=");
	strnum1[fmt_ulong(strnum1, load)] = 0;
	log_out(strnum1);
	log_outf(died ? " died\n" : " started\n");

}

void
sigterm()
{
	int             i;

	flagexitasap = 1;
	sig_block(sig_child);
	sig_block(sig_term);
	log_outf("alert: qscheduler: got SIGTERM\n");
#ifdef HASLIBRT
	if (shm_queue) {
		for (i = 0; i < qcount; i++)
			close(shm_queue[i]);
	}
	if (mq_sch != (mqd_t) -1)
		mq_close(mq_sch);
	if (shm_conf != -1)
		close(shm_conf);
#endif
	for (i = 0; queue_table && i <= qcount;i++) {
		if (queue_table[i].pid == -1)
			continue;
		strnum1[fmt_ulong(strnum1, queue_table[i].pid)] = 0;
		log_out("info: qscheduler: issue SIGTERM to ");
		log_out(strnum1);
		log_outf("\n");
		kill(queue_table[i].pid, sig_term);
	}
	sig_unblock(sig_child);
}

void
sigalrm()
{
	int             i;

	sig_block(sig_alarm);
	log_outf("alert: qscheduler: got SIGALARM\n");
	for (i = 0; queue_table && i <= qcount;i++) {
		if (queue_table[i].pid == -1)
			continue;
		strnum1[fmt_ulong(strnum1, queue_table[i].pid)] = 0;
		log_out("info: qscheduler: issue SIGALRM to ");
		log_out(strnum1);
		log_outf("\n");
		kill(queue_table[i].pid, sig_alarm);
	}
	sig_unblock(sig_alarm);
}

void
sighup()
{
	int             i;

	sig_block(sig_child);
	log_outf("alert: qscheduler: got SIGHUP\n");
	for (i = 0; queue_table && i <= qcount;i++) {
		if (queue_table[i].pid == -1)
			continue;
		strnum1[fmt_ulong(strnum1, queue_table[i].pid)] = 0;
		log_out("info: qscheduler: issue SIGHUP to ");
		log_out(strnum1);
		log_outf("\n");
		kill(queue_table[i].pid, sig_hangup);
	}
	sig_unblock(sig_child);
}

void
sigint()
{
	int             i;

	sig_block(sig_int);
	log_outf("alert: qscheduler: got SIGINT\n");
	for (i = 0; queue_table && i <= qcount;i++) {
		if (queue_table[i].pid == -1)
			continue;
		strnum1[fmt_ulong(strnum1, queue_table[i].pid)] = 0;
		log_out("info: qscheduler: issue SIGINT to ");
		log_out(strnum1);
		log_outf("\n");
		kill(queue_table[i].pid, sig_int);
	}
	sig_unblock(sig_int);
}

void
sigchld()
{
	int             child, wstat;

	sig_block(sig_child);
	log_outf("alert: qscheduler: got SIGCHLD\n");
	for (; !flagexitasap;) {
		if ((child = wait_nohang(&wstat)) == -1)
			break;
		if (!child || flagexitasap)
			break;
		start_send(-1, child);
		sleep(1);
	} /*- for (child = 0;;) */
	for (; flagexitasap;) {
		if ((child = wait_nohang(&wstat)) == -1) {
			if (errno == error_intr)
				continue;
			break;
		}
		if (child) {
			strnum1[fmt_ulong(strnum1, child)] = 0;
			log_out("info: qscheduler: Reaping pid ");
			log_out(strnum1);
			log_outf("\n");
		}
	}
	sig_unblock(sig_child);
}

no_return void
die()
{
	int             i;

	flagexitasap = 1;
	for (i = 0; queue_table && i <= qcount;i++) {
		if (queue_table[i].pid == -1)
			continue;
		strnum1[fmt_ulong(strnum1, queue_table[i].pid)] = 0;
		log_out("info: qscheduler: issue SIGTERM to ");
		log_out(strnum1);
		log_outf("\n");
		kill(queue_table[i].pid, sig_term);
	}
	sleep(1);
	_exit(111);
}

void
die_opendir(char *fn)
{
	strerr_warn3("alert: qscheduler: unable to opendir ", fn, ": ", &strerr_sys);
	die();
}

void
die_chdir(char *s)
{
	strerr_warn3("alert: qcheduler: unable to switch to ", s, ": ", &strerr_sys);
	die();
}

void
nomem()
{
	strerr_warn1("alert: qscheduler: out of memory", 0);
	die();
}

char           *
queuenum_to_dir(int queue_no)
{
	static int      save;

	if (!save) {
		if (!stralloc_copyb(&envQueue, "QUEUEDIR=", 9) ||
				!stralloc_cats(&envQueue, qbase))
			nomem();
		save = envQueue.len;
	} else
		envQueue.len = save;
	if (queue_no) {
		if (!stralloc_catb(&envQueue, "/queue", 6) ||
				!stralloc_catb(&envQueue, strnum1, fmt_ulong(strnum1, (unsigned long) queue_no)) ||
				!stralloc_0(&envQueue))
			nomem();
	} else
		if (!stralloc_catb(&envQueue, "/nqueue", 7) ||
				!stralloc_0(&envQueue))
			nomem();
	envQueue.len--;
	return envQueue.s + 9;
}

int
check_send(int queue_no)
{
	static stralloc lockfile = { 0 };
	int             fd;
	char           *qptr;

	qptr = queuenum_to_dir(queue_no);
	if (!stralloc_copys(&lockfile, qptr) ||
			!stralloc_cats(&lockfile, "/lock/sendmutex") ||
			!stralloc_0(&lockfile))
		nomem();
	if ((fd = open_write(lockfile.s)) == -1) {
		strerr_warn3("alert: qscheduler: cannot start: unable to open ", lockfile.s, ": ", &strerr_sys);
		return 1;
	} else
	if (lock_exnb(fd) == -1) {
		close(fd); /*- send already running */
		strerr_warn3("alert: qscheduler: cannot start: qmail-send with queue ", qptr, " is already running", 0);
		return 1;
	}
	close(fd);
	return 0;
}

unsigned long
get_load(char *qptr)
{
	unsigned long   id, i;
	int             x;

	if (chdir(qptr) == -1)
		die_chdir(qptr);
	readsubdir_init(&todosubdir, "todo", die_opendir);
	i = 0;
	while ((x = readsubdir_next(&todosubdir, &id))) {
		if (x > 0)
			i++;
	}
	return i;
}

void
start_send(int queueNum, pid_t pid)
{
	int             i, queue_no;
	pid_t           qspid;
	char           *qptr;

	if (pid != -1) { /*- restart */
		for (i = 0; i <= qcount; i++) {
			if (queue_table[i].pid == pid)
				break;
		}
		if (queue_table[i].pid != pid) {
			strnum1[fmt_ulong(strnum1, pid)] = 0;
			strerr_warn2("alert: qscheduler: could not locate pid ", strnum1, 0); 
			die();
		}
		queue_no = i;
		qptr = queuenum_to_dir(queue_no);
		log_announce(pid, qptr, queue_table[queue_no].load, 1); /*- announce qmail-send died */
		if (check_send(queue_no)) /*- test lock/sendmutex */
			die();
	} else { /*- first instance */
		queue_no = queueNum;
		qptr = queuenum_to_dir(queue_no);
	}
	switch((qspid = fork()))
	{
	case -1:
		strerr_warn1("alert: qscheduler: unable to fork: ", &strerr_sys);
		die();
	case 0:
		sig_catch(sig_term, SIG_DFL);
		sig_catch(sig_hangup, SIG_DFL);
		sig_catch(sig_child, SIG_DFL);
		sig_catch(sig_int, SIG_DFL);
		if (!queue_no) { /*- don't set this for nqueue */
			if (!env_unset("QMAILLOCAL") || !env_unset("QMAILREMOTE"))
				strerr_die1x(111, "alert: qscheduler: out of memory");
		}
		if (!env_put(envQueue.s))
			strerr_die1x(111, "alert: qscheduler: out of memory");
		execvp(*qsargs, qsargs);
		strerr_die3sys(111, "alert: qscheduler: execv ", *qsargs, ": ");
	default:
		queue_table[queue_no].pid = qspid;
		queue_table[queue_no].load = 0;
		log_announce(qspid, qptr, queue_table[queue_no].load, 0);
		break;
	}
	return;
}

void
set_queue_variables()
{
	getEnvConfigInt(&conf_split, "CONFSPLIT", auto_split);
	if (conf_split > auto_split)
		conf_split = auto_split;
	getEnvConfigInt(&qcount, "QUEUE_COUNT", QUEUE_COUNT);
	getEnvConfigInt(&qstart, "QUEUE_START", QUEUE_START);
#ifdef HASLIBRT
	getEnvConfigInt(&qmax,   "QUEUE_MAX",   QUEUE_MAX);
	getEnvConfigInt(&qload,  "QUEUE_LOAD",  QUEUE_LOAD);
#endif
	if (!(qbase = env_get("QUEUE_BASE"))) {
		switch (control_readfile(&QueueBase, "queue_base", 0))
		{
		case -1:
			strerr_warn1("alert: unable to read control file qbase: ", &strerr_sys);
			die();
		case 0:
			if (!stralloc_copys(&QueueBase, auto_qmail) ||
					!stralloc_catb(&QueueBase, "/queue", 6) ||
					!stralloc_0(&QueueBase))
				nomem();
			qbase = QueueBase.s;
			break;
		case 1:
			qbase = QueueBase.s;
			break;
		}
	}
}

void
static_queue()
{
	int             i, wstat, child, nqueue;
	char           *qptr;

	if (env_get("SPAMFILTER")) {
		qptr = queuenum_to_dir(0);
		nqueue = !access(qptr, F_OK);
		if (!nqueue && errno != error_noent) {
			strerr_warn3("alert: queue ", qptr, ": ", &strerr_sys);
			die();
		}
	} else
		nqueue = 0;
	log_out("info: qscheduler: static queues qStart/qCount ");
	strnum1[fmt_ulong(strnum1, qstart)] = 0;
	log_out(strnum1);
	log_out("/");
	strnum1[fmt_ulong(strnum1, qcount)] = 0;
	log_out(strnum1);
	log_out(nqueue ? " nqueue+ conf split=" : " nqueue- conf split=");
	strnum1[fmt_ulong(strnum1, conf_split)] = 0;
	log_out(strnum1);
	log_outf("\n");
	if (!(queue_table = (qtab *) alloc(sizeof(qtab) * (qcount + 1))))
		nomem();
	for (i = 0; i <= qcount; i++) {
		queue_table[i].pid = -1;
		queue_table[i].queue_no = i;
		queue_table[i].load = 0;
	}
	qsargs[1] = "-s";
	for (i = 0; i <= qcount; i++) {
		if (!i && !nqueue)
			continue;
		if (check_send(i))
			die();
		start_send(i, -1);
	} /*- for (i = 0; i <= qcount; i++) */
	for (;!flagexitasap;) {
		if ((child = wait_pid(&wstat, -1)) == -1) {
			if (errno == error_intr)
				continue;
			break;
		}
		if (flagexitasap)
			break;
		start_send(-1, child);
		sleep(1);
	} /*- for (; !flagexitasap;) */
	for (; flagexitasap;) {
		if ((child = wait_pid(&wstat, -1)) == -1) {
			if (errno == error_intr)
				continue;
			break;
		}
	}
	strerr_die1x(flagexitasap ? 0 : 111, "info: qscheduler: exiting");
}

int
queue_fix(char *queuedir)
{
	pid_t           pid;
	int             wstat, exitcode;

	sig_block(sig_int);
	strnum1[fmt_ulong(strnum1, conf_split)] = 0;
	log_out("info: queue-fix: conf split=");
	log_out(strnum1);
	log_out(", queuedir=");
	log_out(queuedir);
	log_outf("\n");
	switch ((pid = fork()))
	{
	case -1:
		strerr_warn1("alert: qscheduler: unable to fork: ", &strerr_sys);
		die();
	case 0:
		sig_catch(sig_term, SIG_DFL);
		sig_catch(sig_hangup, SIG_DFL);
		sig_catch(sig_child, SIG_DFL);
		sig_catch(sig_int, SIG_DFL);
		if (chdir(auto_qmail) == -1)
			strerr_die3sys(111, "alert: qscheduler: queue-fix: chdir ", auto_qmail, ": ");
		strnum1[fmt_int(strnum1, conf_split)] = 0;
		qfargs[2] = strnum1;
		qfargs[3] = queuedir;
		execvp(*qfargs, qfargs); /*- queue-fix */
		strerr_die3sys(111, "alert: qscheduler: execv ", *qfargs, ": ");
	}
	sig_unblock(sig_int);
	if (wait_pid(&wstat, pid) != pid) {
		strerr_warn1("alert: qscheduler: queue-fix waitpid surprise", 0);
		die();
	}
	if (wait_crashed(wstat)) {
		strerr_warn1("alert: qscheduler: queue-fix crashed", 0);
		die();
	}
	exitcode = wait_exitcode(wstat);
	if (exitcode) {
		strnum1[fmt_int(strnum1, exitcode)] = 0;
		strerr_warn4("alert: qscheduler: queue-fix exit code ", strnum1, ": trouble fixing queue directory ", queuedir, 0);
		die();
	} else
		strerr_warn2("info: qcheduler: queue-fix: queue OK ", queuedir, 0);
	return (exitcode ? 1 : 0);
}

#ifdef HASLIBRT
void
create_ipc()
{
	char            ipc_name[FMT_ULONG + 6], shm_dev_name[FMT_ULONG + 6 + 8],
					mq_dev_name[FMT_ULONG + 6 + 11]; /*- /queue + /dev/mqueue */
	char           *s;
	int             i, j;
	mqd_t           mq_queue;

	if (uidinit(1, 1) == -1 || auto_uidq == -1 || auto_uids == -1 || auto_gidq == -1) {
		strerr_warn1("alert: qscheduler: failed to get uids, gids: ", &strerr_sys);
		die();
	}
	for (j = 0; j < qcount; j++) {
		s = ipc_name;
		i = fmt_str(s, "/queue");
		s += i;
		i = fmt_int(s, j + 1);
		s += i;
		*s++ = 0;
		i = 0;
		/*- create shm /queueN for storing concurrency values by qmail-send */
		if ((shm_queue[j] = shm_open(ipc_name, O_CREAT|O_EXCL|O_RDWR, 0644)) == -1) {
			if (errno != error_exist) {
				strerr_warn3("alert: qscheduler: failed to create POSIX shared memory ", ipc_name, ": ", &strerr_sys);
				die();
			}
			if (!access("/dev/shm", F_OK)) {
				s = shm_dev_name;
				i = fmt_str(s, "/dev/shm/queue");
				s += i;
				i = fmt_int(s, j + 1);
				s += i;
				*s++ = 0;
				i = 0;
				if (chmod(shm_dev_name, 0644) == -1) {
					strerr_warn3("alert: qscheduler: failed to set permissions for POSIX shared memory ", shm_dev_name, ": ", &strerr_sys);
					die();
				}
				if (chown(shm_dev_name, auto_uids, auto_gidq) == -1) {
					strerr_warn3("alert: qscheduler: failed to set ownership for POSIX shared memory ", shm_dev_name, ": ", &strerr_sys);
					die();
				}
			}
		} else {
			if (fchown(shm_queue[j], auto_uids, auto_gidq) == -1) {
				strerr_warn3("alert: qscheduler: failed to set ownership for POSIX shared memory ", ipc_name, ": ", &strerr_sys);
				close(shm_queue[j]);
				die();
			}
			close(shm_queue[j]);
		}
		if ((shm_queue[j] = shm_open(ipc_name, O_RDONLY, 0644)) == -1) {
			strerr_warn2("alert: qscheduler: failed to open POSIX shared memory ", ipc_name, &strerr_sys);
			die();
		}
		/*- message queue for communication between qmail-todo, qmail-queue */
		if ((mq_queue = mq_open(ipc_name, O_CREAT|O_EXCL|O_RDWR,  0640, NULL)) == (mqd_t) -1) {
			if (errno != error_exist)  {
				strerr_warn3("alert: qscheduler: failed to create POSIX message queue ", ipc_name, ": ", &strerr_sys);
				die();
			}
			if (!access("/dev/mqueue", F_OK)) {
				s = mq_dev_name;
				i = fmt_str(s, "/dev/mqueue/queue");
				s += i;
				i = fmt_int(s, j + 1);
				s += i;
				*s++ = 0;
				i = 0;
				if (chmod(mq_dev_name, 0640) == -1) {
					strerr_warn3("alert: qscheduler: failed to set permissions for POSIX message queue ", mq_dev_name, ":", &strerr_sys);
					die();
				}
				if (chown(mq_dev_name, auto_uidq, auto_gidq) == -1) {
					strerr_warn3("alert: qscheduler: failed to set ownership for POSIX message queue ", mq_dev_name, ":", &strerr_sys);
					die();
				}
			}
		} else {
#ifdef FREEBSD
			if (fchown(mq_getfd_np(mq_queue), auto_uidq, auto_gidq) == -1) {
#else
			if (fchown(mq_queue, auto_uidq, auto_gidq) == -1) {
#endif
				strerr_warn3("alert: qscheduler: failed to set ownership for POSIX message queue ", ipc_name, ":", &strerr_sys);
				mq_close(mq_queue);
				die();
			}
			mq_close(mq_queue);
		}
	} /*- for (j = 0; j < qcount; j++) */

	/*- message queue /qscheduler for communication between qscheduler, qmail-send */
	if ((mq_sch = mq_open("/qscheduler", O_CREAT|O_EXCL|O_RDWR, 0600, NULL)) == (mqd_t) -1) {
		if (errno != error_exist) {
			strerr_warn1("alert: qscheduler: failed to create POSIX message queue /qscheduler: ", &strerr_sys);
			die();
		}
		if (!access("/dev/mqueue", F_OK)) {
			if (chmod("/dev/mqueue/qscheduler", 0600) == -1) {
				strerr_warn1("alert: qscheduler: failed to set permissions for POSIX message queue /dev/mqueue/qscheduler: ", &strerr_sys);
				die();
			}
			if (chown("/dev/mqueue/qscheduler", auto_uido, auto_gidq) == -1) {
				strerr_warn1("alert: qscheduler: failed to set ownership for POSIX message queue /dev/mqueue/qscheduler: ", &strerr_sys);
				die();
			}
		}
	} else
		mq_close(mq_sch);

	if ((mq_sch = mq_open("/qscheduler", O_RDONLY,  0600, NULL)) == (mqd_t) -1) {
		strerr_warn1("alert: qscheduler: failed to open POSIX message queue /qscheduler: ", &strerr_sys);
		die();
	}

	/*- shared memory /qscheduler for storing qcount, qconf */
	if ((shm_conf = shm_open("/qscheduler", O_CREAT|O_EXCL|O_RDWR, 0644)) == -1) {
		if (errno != error_exist) {
			strerr_warn1("alert: qscheduler: failed to create POSIX shared memory /qscheduler: ", &strerr_sys);
			die();
		}
		if (!access("/dev/shm", F_OK)) {
			if (chmod("/dev/shm/qscheduler", 0644) == -1) {
				strerr_warn1("alert: qscheduler: failed to set permissions for POSIX message queue /dev/shm/qscheduler: ", &strerr_sys);
				die();
			}
			if (chown("/dev/shm/qscheduler", auto_uido, auto_gidq) == -1) {
				strerr_warn1("alert: qscheduler: failed to set ownership for POSIX message queue /dev/shm/qscheduler: ", &strerr_sys);
				die();
			}
		}
	} else
		close(shm_conf);
	if ((shm_conf = shm_open("/qscheduler", O_WRONLY, 0644)) == -1) {
		strerr_warn1("alert: qscheduler: failed to open POSIX shared memory /qscheduler: ", &strerr_sys);
		die();
	}
	return;
}

void
dynamic_queue()
{
	struct mq_attr  attr;
	unsigned int    priority, total_load;
	int             i, r, nqueue, q[2];
	char           *qptr;

	qsargs[1] = compat_mode ? "-cd" : "-d";
	if (env_get("SPAMFILTER")) {
		qptr = queuenum_to_dir(0);
		nqueue = !access(qptr, F_OK);
		if (!nqueue && errno != error_noent) {
			strerr_warn3("alert: qscheduler: queue ", qptr, ": ", &strerr_sys);
			die();
		}
	} else
		nqueue = 0;
	/*-
	 * qmax is QUEUE_MAX compile time default in conf-queue
	 * or value of QUEUE_MAX env variable if set
	 */
	for (qconf = 0; qconf < qmax; qconf++) {
		qptr = queuenum_to_dir(qconf + 1);
		if (access(qptr, F_OK)) {
			if (errno == error_noent)
				break;
			strerr_warn3("alert: qscheduler: queue ", qptr, ": ", &strerr_sys);
			die();
		}
	} /*- for (qconf = 0; qconf < qmax; qconf++) */
	/*- 
	 * qcount is QUEUE_COUNT compile time default in conf-queue
	 * or value of QUEUE_COUNT env variable if set
	 */
	if (!(shm_queue = (int *) alloc(sizeof(int) * qcount)))
		nomem();
	create_ipc();
	q[0] = qcount; /*- queue count */
	q[1] = qconf;  /*- existing no of queues in /var/indimail/queue dir */
	if (lseek(shm_conf, 0, SEEK_SET) == -1 || write(shm_conf, (char *) q, sizeof(int) * 2) == -1) {
		strerr_warn1("alert: qscheduler: unable to write to shared memory: ", &strerr_sys);
		die();
	}
	log_out(nqueue? "info: qscheduler: dynamic queues nqueue+ qcount=" : "info: qscheduler: dynamic queues nqueue- qcount=");
	strnum1[fmt_int(strnum1, qcount)] = 0;
	log_out(strnum1);
	log_out(", qconf=");
	strnum1[fmt_int(strnum1, qconf)] = 0;
	log_out(strnum1);
	log_out(", conf split=");
	strnum1[fmt_ulong(strnum1, conf_split)] = 0;
	log_out(strnum1);
	log_outf("\n");
	if (!(queue_table = (qtab *) alloc(sizeof(qtab) * (qcount + 1))))
		nomem();
	for (i = 0; i <= qcount; i++) {
		queue_table[i].pid = -1;
		queue_table[i].queue_no = i;
		queue_table[i].load = 0;
	}
	/*- start qmail-send */
	for (i = 0; i <= qcount; i++) {
		if (!i && !nqueue)
			continue;
		if (check_send(i))
			die();
		start_send(i, -1);
	} /*- for (i = 0; i <= qcount; i++) */
	for (; !flagexitasap;) {
		if (mq_getattr(mq_sch, &attr) == -1) {
			strerr_warn1("alert: qscheduler: unable to get message queue attributes: ", &strerr_sys);
			die();
		}
		if (!msgbuflen) {
			if (!(msgbuf = (char *) alloc(attr.mq_msgsize)))
				nomem();
		} else
		if (msgbuflen < attr.mq_msgsize) {
			if (!alloc_re((char *) &msgbuf, msgbuflen, attr.mq_msgsize))
				nomem();
		}
		msgbuflen = attr.mq_msgsize;
		priority = 0;
		if (mq_receive(mq_sch, msgbuf, msgbuflen, &priority) == -1) {
			if (errno == error_intr)
				continue;
			strerr_warn1("alert: qscheduler: unable to read message queue: ", &strerr_sys);
			die();
		}
		queue_table[((qtab *) msgbuf)->queue_no].load = ((qtab *) msgbuf)->load;
		for (i = 1, total_load = 0; i <= qcount; i++)
			total_load += queue_table[i].load;
		strnum1[fmt_int(strnum1, total_load)] = 0;
		strnum2[fmt_int(strnum2, qcount * qload)] = 0;
		if (total_load > qcount * qload && qcount < qmax) {
			log_out("alert: qscheduler: average load ");
			log_out(strnum1);
			log_out(" exceeds threshold (> ");
			log_out(strnum2);
			log_outf(")\n");
			qptr = queuenum_to_dir(qcount + 1);
			r = 0; /*- flag for queue creation */
			if (access(qptr, F_OK)) {
				r = 1;
				if (errno == error_noent)
					r = queue_fix(qptr);
				else {
					strerr_warn3("alert: qscheduler: ", qptr, ": ", &strerr_sys);
					die();
				}
			}
			if (!r) {
				if (!alloc_re(&queue_table, qcount + 1, qcount + 2))
					nomem();
				qcount++;
				queue_table[qcount].pid = -1;
				queue_table[qcount].queue_no = i;
				queue_table[qcount].load = 0;
				if (check_send(qcount))
					die();
				start_send(qcount, -1);
				if (lseek(shm_conf, 0, SEEK_SET) == -1 || write(shm_conf, (char *) &qcount, sizeof(int)) == -1) {
					strerr_warn1("alert: qscheduler: unable to write to shared memory: ", &strerr_sys);
					die();
				}
			}
		} else {
			log_out("info: qscheduler: average load ");
			log_out(strnum1);
			log_out(" within limits (<= ");
			log_out(strnum2);
			log_outf(")\n");
		}
		printf("msg[queue_no=%d load=%ld] priority=%d qcount=%d\n", ((qtab *) msgbuf)->queue_no, ((qtab *) msgbuf)->load, priority, qcount);
	} /*- for (; !flagexitasap;) */
	strerr_warn1("info: qscheduler: exiting", 0);
	_exit(flagexitasap ? 0 : 111);
}
#endif

int
main(int argc, char **argv)
{
#ifdef HASLIBRT
	int             opt;
#endif
	int             fd;

	prog_argv = argv;
	sig_catch(sig_term, sigterm);
	sig_catch(sig_alarm, sigalrm);
	sig_catch(sig_hangup, sighup);
	sig_catch(sig_int, sigint);
	sig_catch(sig_child, sigchld);
	if (chdir(auto_qmail) == -1)
		die_chdir(auto_qmail);
	/*- we allow only one copy to run */
	if ((fd = open_trunc("queue/qscheduler")) == -1)
		strerr_die1sys(111, "alert: qscheduler: cannot start: unable to open queue/qscheduler: ");
	if (lock_exnb(fd) == -1)
		strerr_die1x(111, "alert: cannot start: qscheduler is already running\n");
#ifdef HASLIBRT
	while ((opt = getopt(argc, argv, "cds")) != opteof) {
		switch (opt)
		{
			case 'c':
				compat_mode = 1;
				break;
			case 'd':
				qtype = dynamic;
				break;
			case 's':
				qtype = fixed;
				break;
		}
	}
	argc -= optind;
	argv += optind;
	if (argv[0])
		qsargs[2] = argv[0]; /*- argument to qmail-start (./Mailbox or ./Maildir) */
	set_queue_variables();
	if (qtype == dynamic)
		dynamic_queue();
	else
		static_queue();
#else
	set_queue_variables();
	static_queue();
#endif
	if (flagexitasap)
		strerr_warn1("qscheduler: exiting", 0);
	return 0;
}

void
getversion_queue_scheduler_c()
{
	static char    *x = "$Id: $";

	x++;
}
