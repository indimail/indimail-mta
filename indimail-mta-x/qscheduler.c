/*
 * $Log: $
 */
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
#include <alloc.h>
#include <error.h>
#include <strerr.h>
#include <wait.h>
#include <noreturn.h>
#include <getEnvConfig.h>
#include <sgetopt.h>
#ifdef HASLIBRT
#include "auto_uids.h"
#endif
#include "qscheduler.h"
#include "control.h"
#include "readsubdir.h"
#include "auto_qmail.h"
#include "auto_split.h"

#define ERROR_INTERVAL 5

static int      qstart, qcount;
static char    *qbase;
static stralloc envQueue = {0}, QueueBase = {0};
static int      flagexitasap = 0;
static char  **prog_argv;
static char     strnum1[FMT_ULONG];
static int      bigtodo;
#ifdef HASLIBRT
static int      qconf, qmax, qload, error_interval = ERROR_INTERVAL;
static q_type   qtype = fixed;
static char    *msgbuf;
static int      msgbuflen;
static mqd_t    mq_sch = (mqd_t) -1;
static int      shm_conf = -1;
static int     *shm_queue;
static int      compat_mode;
static char     strnum2[FMT_ULONG];
#endif
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
static void     nomem();

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

char           *
queuenum_to_dir(int queue_no)
{
	static int      save;
	char            tmp[FMT_ULONG];

	if (!save) {
		if (!stralloc_copyb(&envQueue, "QUEUEDIR=", 9) ||
				!stralloc_cats(&envQueue, qbase))
			nomem();
		save = envQueue.len;
	} else
		envQueue.len = save;
	if (queue_no) {
		if (!stralloc_catb(&envQueue, "/queue", 6) ||
				!stralloc_catb(&envQueue, tmp, fmt_ulong(tmp, (unsigned long) queue_no)) ||
				!stralloc_0(&envQueue))
			nomem();
	} else
		if (!stralloc_catb(&envQueue, "/nqueue", 7) ||
				!stralloc_0(&envQueue))
			nomem();
	envQueue.len--;
	return envQueue.s + 9;
}

no_return void
die()
{
	int             i;
	char           *qptr;

	flagexitasap = 1;
	for (i = 0; queue_table && i <= qcount;i++) {
		if (queue_table[i].pid == -1)
			continue;
		qptr = queuenum_to_dir(i);
		strnum1[fmt_ulong(strnum1, queue_table[i].pid)] = 0;
		log_out("info: qscheduler: issue SIGTERM to ");
		log_out(strnum1);
		log_out(", queue ");
		log_out(qptr);
		log_outf("\n");
		kill(queue_table[i].pid, sig_term);
	}
	sleep(1);
	_exit(111);
}

static void
nomem()
{
	strerr_warn1("alert: qscheduler: out of memory", 0);
	die();
}

static void
die_opendir(char *fn)
{
	strerr_warn3("alert: qscheduler: unable to opendir ", fn, ": ", &strerr_sys);
	die();
}

static void
die_chdir(char *s)
{
	strerr_warn3("alert: qcheduler: unable to switch to ", s, ": ", &strerr_sys);
	die();
}

void
sigterm()
{
	int             i;
	char           *qptr;

	flagexitasap = 1;
	sig_block(sig_child);
	sig_block(sig_term);
	log_outf("alert: qscheduler: got TERM\n");
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
		qptr = queuenum_to_dir(i);
		strnum1[fmt_ulong(strnum1, queue_table[i].pid)] = 0;
		log_out("info: qscheduler: issue SIGTERM to ");
		log_out(strnum1);
		log_out(", queue ");
		log_out(qptr);
		log_outf("\n");
		kill(queue_table[i].pid, sig_term);
	}
	sig_unblock(sig_child);
}

void
sigalrm()
{
	int             i;
	char           *qptr;

	sig_block(sig_alarm);
	log_outf("alert: qscheduler: got ALARM\n");
	for (i = 0; queue_table && i <= qcount;i++) {
		if (queue_table[i].pid == -1)
			continue;
		qptr = queuenum_to_dir(i);
		strnum1[fmt_ulong(strnum1, queue_table[i].pid)] = 0;
		log_out("info: qscheduler: issue SIGALRM to ");
		log_out(strnum1);
		log_out(", queue ");
		log_out(qptr);
		log_outf("\n");
		kill(queue_table[i].pid, sig_alarm);
	}
	sig_unblock(sig_alarm);
}

void
sighup()
{
	int             i;
	char           *qptr;

	sig_block(sig_child);
	log_outf("alert: qscheduler: got HUP\n");
	for (i = 0; queue_table && i <= qcount;i++) {
		if (queue_table[i].pid == -1)
			continue;
		qptr = queuenum_to_dir(i);
		strnum1[fmt_ulong(strnum1, queue_table[i].pid)] = 0;
		log_out("info: qscheduler: issue SIGHUP to ");
		log_out(strnum1);
		log_out(", queue ");
		log_out(qptr);
		log_outf("\n");
		kill(queue_table[i].pid, sig_hangup);
	}
	sig_unblock(sig_child);
}

void
sigint()
{
	int             i;
	char           *qptr;

	sig_block(sig_int);
	log_outf("alert: qscheduler: got INT\n");
	for (i = 0; queue_table && i <= qcount;i++) {
		if (queue_table[i].pid == -1)
			continue;
		qptr = queuenum_to_dir(i);
		strnum1[fmt_ulong(strnum1, queue_table[i].pid)] = 0;
		log_out("info: qscheduler: issue SIGINT to ");
		log_out(strnum1);
		log_out(", queue ");
		log_out(qptr);
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
	sig_unblock(sig_child);
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
	readsubdir_init(&todosubdir, "todo", bigtodo, die_opendir);
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
	int             i, queue_no, count;
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
		for (count = 0;count < 20; count++) {
			if (!(i = check_send(queue_no))) /*- test lock/sendmutex */
				break;
			sleep(5);
		}
		if (i) {
			strerr_warn2("alert: qscheduler: could not shutdown pid ", strnum1, 0); 
			die();
		}
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
	getEnvConfigInt(&bigtodo, "BIGTODO", 0);
	getEnvConfigInt(&conf_split, "CONFSPLIT", auto_split);
	if (conf_split > auto_split)
		conf_split = auto_split;
	getEnvConfigInt(&qcount, "QUEUE_COUNT", QUEUE_COUNT);
	getEnvConfigInt(&qstart, "QUEUE_START", QUEUE_START);
#ifdef HASLIBRT
	getEnvConfigInt(&qmax,   "QUEUE_MAX",   QUEUE_MAX);
	getEnvConfigInt(&qload,  "QUEUE_LOAD",  QUEUE_LOAD);
	getEnvConfigInt(&error_interval, "ERROR_INTERVAL", ERROR_INTERVAL);
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
		strnum1[fmt_ulong(strnum1, child)] = 0;
		strerr_warn3("alert: qscheduler: pid ", strnum1, " has shutdown", 0); 
	}
#ifdef LIBRT
	shm_unlink("/qscheduler");
#endif
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
	} else {
		log_out("info: qcheduler: queue-fix: queue OK ");
		log_out(queuedir);
		log_outf("\n");
	}
	return (exitcode ? 1 : 0);
}

#ifdef HASLIBRT
void
create_ipc(int *msgqueue_len, int *msgqueue_size)
{
	char            ipc_name[FMT_ULONG + 6];
#ifdef LINUX
	char            shm_dev_name[FMT_ULONG + 6 + 8],
					mq_dev_name[FMT_ULONG + 6 + 11]; /*- /queue + /dev/mqueue */
#endif
	char           *s;
	int             i, j;
	struct mq_attr  attr;
	mqd_t           mq_queue;

	if (uidinit(1, 1) == -1 || auto_uidq == -1 || auto_uids == -1 || auto_gidq == -1) {
		strerr_warn1("alert: qscheduler: failed to get uids, gids: ", &strerr_sys);
		die();
	}
	if ((i = control_readint(msgqueue_len, "msgqueuelen")) == -1) {
		strerr_warn1("alert: qscheduler: failed to open control file msgqueuelen: ", &strerr_sys);
		die();
	} else
	if (!i)
		*msgqueue_len = 65534;
	if ((i = control_readint(msgqueue_size, "msgqueuesize")) == -1) {
		strerr_warn1("alert: qscheduler: failed to open control file msgqueuelen: ", &strerr_sys);
		die();
	} else
	if (!i)
		*msgqueue_size = 1024;
#ifdef FREEBSD
	attr.mq_flags = 0;
#else
	attr.mq_flags = 0;
	attr.mq_maxmsg = *msgqueue_len;
	attr.mq_msgsize = *msgqueue_size;
	attr.mq_curmsgs = 0;
#endif
	for (j = 0; j < qcount; j++) {
		s = ipc_name;
		i = fmt_str(s, "/queue");
		s += i;
		i = fmt_int(s, j + 1);
		s += i;
		*s++ = 0;
		i = 0;
		/*- create shm /queueN for storing concurrency values by qmail-send */
#ifdef FREEBSD
		shm_queue[j] = shm_open(ipc_name, O_CREAT|O_TRUNC|O_EXCL|O_RDWR, 0644);
#else
		shm_queue[j] = shm_open(ipc_name, O_CREAT|O_EXCL|O_RDWR, 0644);
#endif
		if (shm_queue[j] == -1) {
			if (errno != error_exist) {
				strerr_warn3("alert: qscheduler: failed to create POSIX shared memory ", ipc_name, ": ", &strerr_sys);
				die();
			}
#ifdef LINUX
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
#endif
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
#ifdef FREEBSD
		if ((mq_queue = mq_open(ipc_name, O_CREAT|O_EXCL|O_RDWR,  0640, NULL)) != (mqd_t) -1) {
			if (mq_setattr(mq_queue, &attr, 0) == -1) {
				strerr_warn3("alert: qscheduler: failed to set POSIX message attributes ", ipc_name, ": ", &strerr_sys);
				mq_close(mq_queue);
				mq_queue = (mqd_t) -1;
			}
		}
#else
		mq_queue = mq_open(ipc_name, O_CREAT|O_EXCL|O_RDWR,  0640, &attr);
#endif
		if (mq_queue == (mqd_t) -1) {
			if (errno != error_exist)  {
				strerr_warn3("alert: qscheduler: failed to create POSIX message queue ", ipc_name, ": ", &strerr_sys);
				die();
			}
#ifdef LINUX
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
#endif
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
#ifdef LINUX
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
#endif
	} else
		mq_close(mq_sch);

	if ((mq_sch = mq_open("/qscheduler", O_RDONLY,  0600, NULL)) == (mqd_t) -1) {
		strerr_warn1("alert: qscheduler: failed to open POSIX message queue /qscheduler: ", &strerr_sys);
		die();
	}

	/*- shared memory /qscheduler for storing qcount, qconf */
#ifdef FREEBSD
	shm_conf = shm_open("/qscheduler", O_CREAT|O_TRUNC|O_EXCL|O_RDWR, 0644);
#else
	shm_conf = shm_open("/qscheduler", O_CREAT|O_EXCL|O_RDWR, 0644);
#endif
	if (shm_conf == -1) {
		if (errno != error_exist) {
			strerr_warn1("alert: qscheduler: failed to create POSIX shared memory /qscheduler: ", &strerr_sys);
			die();
		}
#ifdef LINUX
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
#endif
	} else
		close(shm_conf);
#ifdef FREEBSD
		/*-
		 * FreeBSD nonsense.
		 * A flag other than O_RDONLY, O_RDWR, O_CREAT,
		 * O_EXCL, or O_TRUNC causes EINVAL
		 */
	shm_conf = shm_open("/qscheduler", O_RDWR, 0644);
#else
	shm_conf = shm_open("/qscheduler", O_WRONLY, 0644);
#endif
	if (shm_conf == -1) {
		strerr_warn1("alert: qscheduler: failed to open POSIX shared memory /qscheduler: ", &strerr_sys);
		die();
	}
#ifdef FREEBSD /*- another FreeBSD idiosyncrasies */
	if (ftruncate(shm_conf, getpagesize()) < 0) {
		strerr_warn1("alert: qscheduler: failed to truncate POSIX shared memory /qscheduler: ", &strerr_sys);
		die();
	}
#endif
	return;
}

void
dynamic_queue()
{
	struct mq_attr  attr;
	unsigned int    priority, total_load;
	int             i, r, nqueue, q[2], qlen, qsize, child, wstat;
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
	create_ipc(&qlen, &qsize);
	q[0] = qcount; /*- queue count */
	q[1] = qconf;  /*- existing no of queues in /var/indimail/queue dir */
	if (lseek(shm_conf, 0, SEEK_SET) == -1 || write(shm_conf, (char *) q, sizeof(int) * 2) <= 0) {
		strerr_warn1("alert: qscheduler: unable to write to shared memory /qscheduler: ", &strerr_sys);
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
	log_out(", msgqueue_len=");
	strnum1[fmt_ulong(strnum1, qlen)] = 0;
	log_out(strnum1);
	log_out(", msgqueue_size=");
	strnum1[fmt_ulong(strnum1, qsize)] = 0;
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
			sleep(error_interval);
		}
		if (!msgbuflen) {
			if (!(msgbuf = (char *) alloc(attr.mq_msgsize)))
				sleep(error_interval);
		} else
		if (msgbuflen < attr.mq_msgsize) {
			if (!alloc_re((char *) &msgbuf, msgbuflen, attr.mq_msgsize))
				sleep(error_interval);
		}
		msgbuflen = attr.mq_msgsize;
		priority = 0;
		if (mq_receive(mq_sch, msgbuf, msgbuflen, &priority) == -1) {
			if (errno == error_intr)
				continue;
			strerr_warn1("alert: qscheduler: unable to read message queue: ", &strerr_sys);
			sleep(error_interval);
		}
		queue_table[((qtab *) msgbuf)->queue_no].load = ((qtab *) msgbuf)->load;
		for (i = 1, total_load = 0; i <= qcount; i++)
			total_load += queue_table[i].load;
		strnum1[fmt_int(strnum1, total_load)] = 0;
		strnum2[fmt_int(strnum2, qcount * qload)] = 0;
		if (total_load > qcount * qload && qcount < qmax) {
			strerr_warn5("alert: qscheduler: average load ",strnum1, " exceeds threshold (> ", strnum2, ")", 0);
			qptr = queuenum_to_dir(qcount + 1);
			r = 0; /*- flag for queue creation */
			if (access(qptr, F_OK)) {
				r = 1;
				if (errno == error_noent)
					r = queue_fix(qptr);
				else {
					strerr_warn3("alert: qscheduler: ", qptr, ": ", &strerr_sys);
					sleep(error_interval);
				}
			}
			if (!r) {
				if (!alloc_re(&queue_table, sizeof(qtab) * (qcount + 1), sizeof(qtab) * (qcount + 2)))
					sleep(error_interval);
				if (++qcount > qconf)
					qconf++;
				queue_table[qcount].pid = -1;
				queue_table[qcount].queue_no = i;
				queue_table[qcount].load = 0;
				create_ipc(&qlen, &qsize);
				if (check_send(qcount)) {
					qcount--;
					sleep(error_interval);
				}
				start_send(qcount, -1);
				q[0] = qcount;
				q[1] = qconf;
				if (lseek(shm_conf, 0, SEEK_SET) == -1 || write(shm_conf, (char *) q, sizeof(int) * 2) <= 0) {
					strerr_warn1("alert: qscheduler: unable to write to shared memory:  /qscheduler", &strerr_sys);
					qcount--;
					sleep(error_interval);
				}
			}
		} else {
			log_out("info: qscheduler: average load ");
			log_out(strnum1);
			log_out(" within limits (<= ");
			log_out(strnum2);
			log_outf(")\n");
		}
		/*-
		 * queue_no load priority qcount -> ((qtab *) msgbuf)->queue_no, ((qtab *) msgbuf)->load, priority, qcount);
		 */ 
	} /*- for (; !flagexitasap;) */

	for (; flagexitasap;) {
		if ((child = wait_pid(&wstat, -1)) == -1) {
			if (errno == error_intr)
				continue;
			break;
		}
		strnum1[fmt_ulong(strnum1, child)] = 0;
		strerr_warn3("alert: qscheduler: pid ", strnum1, " has shutdown", 0); 
	}
	log_outf("info: qscheduler: exiting\n");
	_exit(flagexitasap ? 0 : 111);
}
#endif

int
main(int argc, char **argv)
{
	int             opt;
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
	while ((opt = getopt(argc, argv, "cds")) != opteof) {
		switch (opt)
		{
			case 'c':
#ifdef HASLIBRT
				compat_mode = 1;
				qtype = dynamic;
#else
				strerr_die1x(111, "alert: qscheduler: cannot start: dynamic mode not supported\n");
#endif
				break;
			case 'd':
#ifdef HASLIBRT
				qtype = dynamic;
#else
				strerr_die1x(111, "alert: qscheduler: cannot start: dynamic mode not supported\n");
#endif
				break;
			case 's':
#ifdef HASLIBRT
				qtype = fixed;
#endif
				break;
		}
	}
	argc -= optind;
	argv += optind;
	if (argv[0])
		qsargs[2] = argv[0]; /*- argument to qmail-start (./Mailbox or ./Maildir) */
	set_queue_variables();
#ifdef HASLIBRT
	if (qtype == dynamic)
		dynamic_queue();
	else
		static_queue();
#else
	static_queue();
#endif
	if (flagexitasap)
		log_outf("info: qscheduler: exiting\n");
	return 0;
}

void
getversion_queue_scheduler_c()
{
	static char    *x = "$Id: $";

	x++;
}
