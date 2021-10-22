/*
 * $Log: $
 */

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <mqueue.h>
#include <sys/mman.h>
#include <sys/stat.h>
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
#include "qscheduler.h"
#include "qscheduler.h"
#include "control.h"
#include "readsubdir.h"
#include "auto_qmail.h"
#include "auto_uids.h"
#include "auto_split.h"

static pid_tab *queue_table;
static int      qcount, qstart, qmax, qload;
static char    *qbase;
static stralloc envQueue = {0}, QueueBase = {0};
static int      flagexitasap = 0, do_mq = 0;
static q_type   qtype;
static char  **prog_argv;
static char    *msgbuf;
static int      msgbuflen;
static mqd_t    mqd = -1;
static int      shm = -1;
static char     strnum[FMT_ULONG];
static readsubdir todosubdir;
static char     ssoutbuf[512];
static char     sserrbuf[512];
static substdio ssout = SUBSTDIO_FDBUF(write, 1, ssoutbuf, sizeof(ssoutbuf));
static substdio sserr = SUBSTDIO_FDBUF(write, 2, sserrbuf, sizeof(sserrbuf));
int             conf_split;
static char    *(qsargs[]) = { "qmail-start", "./Mailbox", 0};
char           *(qfargs[]) = { "queue-fix", "-s", 0, 0, 0};

void
sigterm()
{
	int             i;

	flagexitasap = 1;
	sig_block(sig_child);
	sig_block(sig_term);
	if (mqd != -1)
		close(mqd);
	if (shm != -1)
		close(shm);
	for (i = 0; queue_table && i <= qcount;i++) {
		if (queue_table[i].pid == -1)
			continue;
		kill(queue_table[i].pid, sig_term);
	}
	sig_unblock(sig_child);
}

void
sigalrm()
{
	int             i;

	sig_block(sig_alarm);
	for (i = 0; queue_table && i <= qcount;i++) {
		if (queue_table[i].pid == -1)
			continue;
		kill(queue_table[i].pid, sig_alarm);
	}
	sig_unblock(sig_alarm);
}

void
sighup()
{
	int             i;

	sig_block(sig_child);
	for (i = 0; queue_table && i <= qcount;i++) {
		if (queue_table[i].pid == -1)
			continue;
		kill(queue_table[i].pid, sig_hangup);
	}
	sig_unblock(sig_child);
}

void
sigint()
{
	int             i;

	sig_block(sig_int);
	for (i = 0; queue_table && i <= qcount;i++) {
		if (queue_table[i].pid == -1)
			continue;
		kill(queue_table[i].pid, sig_int);
	}
	sig_unblock(sig_int);
}

void
sigchld()
{
	int             child, wstat;
	void            restart_send(int);

	sig_block(sig_child);
	for (; !flagexitasap;) {
		if ((child = wait_nohang(&wstat)) == -1)
			break;
		if (!child || flagexitasap)
			break;
		restart_send(child);
		sleep(1);
	} /*- for (child = 0;;) */
	for (; flagexitasap;) {
		if ((child = wait_nohang(&wstat)) == -1)
			break;
	}
	sig_unblock(sig_child);
}

void
log_err(char *s)
{
	if (substdio_puts(&sserr, s) == -1)
		_exit(1);
}

void
log_errf(char *s)
{
	if (substdio_puts(&sserr, s) == -1)
		_exit(1);
	if (substdio_flush(&sserr) == -1)
		_exit(1);
}

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
log_announce(int pid, char *qdir, unsigned long load)
{
	log_out("info: qscheduler: pid ");
	strnum[fmt_ulong(strnum, pid)] = 0;
	log_out(strnum);
	log_out(", queue ");
	log_out(qdir);
	log_out(", load=");
	strnum[fmt_ulong(strnum, load)] = 0;
	log_out(strnum);
	log_outf(" started\n");

}

no_return void
die()
{
	int             i;

	for (i = 0; queue_table && i <= qcount;i++) {
		if (queue_table[i].pid == -1)
			continue;
		kill(queue_table[i].pid, sig_term);
	}
	sleep(1);
	_exit(111);
}

void
die_opendir(char *fn)
{
	log_err("alert: qscheduler: unable to opendir ");
	log_err(fn);
	log_err(": ");
	log_err(error_str(errno));
	log_errf("\n");
	die();
}

void
die_chdir(char *s)
{
	log_err("alert: qcheduler: unable to switch to ");
	log_err(s);
	log_err(": ");
	log_err(error_str(errno));
	log_errf("\n");
	die();
}

void
nomem()
{
	log_errf("alert: qscheduler: out of memory\n");
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
				!stralloc_catb(&envQueue, strnum, fmt_ulong(strnum, (unsigned long) queue_no)) ||
				!stralloc_0(&envQueue))
			nomem();
	} else
		if (!stralloc_catb(&envQueue, "/nqueue", 7) ||
				!stralloc_0(&envQueue))
			nomem();
	envQueue.len--;
	return envQueue.s + 9;
}

void
create_ipc()
{
	int             wstat, child, i = 1;

	switch((child = fork()))
	{
	case -1:
		log_err("alert: qscheduler: unable to fork: ");
		log_err(error_str(errno));
		log_errf("\n");
		_exit(111);
	case 0:
		sig_catch(sig_term, SIG_DFL);
		sig_catch(sig_hangup, SIG_DFL);
		sig_catch(sig_child, SIG_DFL);
		sig_catch(sig_int, SIG_DFL);
		if (uidinit(1, 1) == -1 || auto_uidq == -1 || auto_gidq == -1)
			_exit(111);
		if (setregid(auto_gidq, auto_gidq) || setreuid(auto_uidq, auto_uidq))
			_exit(111);
		if ((mqd = mq_open("/qmail-queue", O_CREAT|O_EXCL|O_RDWR,  0644, NULL)) == -1) {
			if (errno != error_exist)
				_exit(111);
		}
		close(mqd);
		if ((shm = shm_open("/qmail-queue", O_CREAT|O_EXCL|O_RDWR, 0644)) == -1) {
			if (errno != error_exist)
				_exit(111);
		}
		close(shm);
		_exit(0);
	}
	if (wait_pid(&wstat, child) != child) {
		log_errf("alert: qscheduler waitpid surprise\n");
		_exit(111);
	}
	if (wait_crashed(wstat)) {
		log_errf("alert: qscheduler child crashed\n");
		_exit(111);
	}
	if (wait_exitcode(wstat)) {
		log_errf("alert: qscheduler: failed to create POSIX IPC communication\n");
		_exit(111);
	}
	if ((mqd = mq_open("/qmail-queue", O_RDONLY,  0644, NULL)) == -1) {
		log_err("alert: qscheduler: failed to create POSIX message queue: ");
		log_err(error_str(errno));
		log_errf("\n");
		_exit(111);
	}
	if ((shm = shm_open("/qmail-queue", O_WRONLY, 0644)) == -1) {
		log_err("alert: qscheduler: failed to create POSIX shared memory: ");
		log_err(error_str(errno));
		log_errf("\n");
		_exit(111);
	}
	if (write(shm, (void *) &i, sizeof(int)) == -1) {
		log_err("alert: qscheduler: failed to write to POSIX shared memory: ");
		log_err(error_str(errno));
		log_errf("\n");
		_exit(111);
	}
	return;
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
		log_err("alert: qscheduler: cannot start: unable to open ");
		log_err(lockfile.s);
		log_err(": ");
		log_err(error_str(errno));
		log_errf("\n");
		return 1;
	} else
	if (lock_exnb(fd) == -1) {
		close(fd); /*- send already running */
		log_err("alert: qscheduler: cannot start: qmail-send with queue ");
		log_err(qptr);
		log_errf(" is already running\n");
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
start_send(int queue_no)
{
	int             child;
	char           *qptr;

	qptr = queuenum_to_dir(queue_no);
	queue_table[queue_no].load = get_load(qptr);
	switch((child = fork()))
	{
	case -1:
		log_err("alert: qscheduler: unable to fork: ");
		log_err(error_str(errno));
		log_errf("\n");
		die();
	case 0:
		sig_catch(sig_term, SIG_DFL);
		sig_catch(sig_hangup, SIG_DFL);
		sig_catch(sig_child, SIG_DFL);
		sig_catch(sig_int, SIG_DFL);
		if (!queue_no) { /*- don't set this for nqueue */
			if (!env_unset("QMAILLOCAL") || !env_unset("QMAILREMOTE")) {
				log_errf("alert: qscheduler: out of memory\n");
				_exit(111);
			}
		}
		if (!env_put(envQueue.s)) {
			log_errf("alert: qscheduler: out of memory\n");
			_exit(111);
		}
		execvp(*qsargs, qsargs);
		strerr_die3sys(111, "alert: qscheduler: execv ", *qsargs, ": ");
	default:
		queue_table[queue_no].pid = child;
		log_announce(child, qptr, queue_table[queue_no].load);
		break;
	}
}

void
restart_send(int pid)
{
	int             i, child;
	char           *qptr;

	for (i = 0; i <= qcount; i++) {
		if (queue_table[i].pid == pid)
			break;
	}
	if (queue_table[i].pid != pid) {
		log_err("alert: qscheduler: could not locate pid "); 
		strnum[fmt_ulong(strnum, pid)] = 0;
		log_err(strnum);
		log_errf("\n");
		die();
	}
	qptr = queuenum_to_dir(queue_table[i].queue_no);
	queue_table[i].load = get_load(qptr);
	log_err("alert: qscheduler: pid ");
	strnum[fmt_ulong(strnum, pid)] = 0;
	log_err(strnum);
	log_err(", queue ");
	log_err(qptr);
	log_err(", load=");
	strnum[fmt_ulong(strnum, queue_table[i].load)] = 0;
	log_err(strnum);
	log_errf(" died\n");
	if (check_send(queue_table[i].queue_no))
		die();
	switch((child = fork()))
	{
	case -1:
		log_err("alert: qscheduler: unable to fork: ");
		log_err(error_str(errno));
		log_errf("\n");
		die();
	case 0:
		sig_catch(sig_term, SIG_DFL);
		sig_catch(sig_hangup, SIG_DFL);
		sig_catch(sig_child, SIG_DFL);
		sig_catch(sig_int, SIG_DFL);
		if (!env_put(envQueue.s)) {
			log_errf("alert: qscheduler: out of memory\n");
			die();
		}
		execvp(*qsargs, qsargs);
		strerr_die3sys(111, "alert: qscheduler: execv ", *qsargs, ": ");
	default:
		queue_table[i].pid = child;
		log_announce(child, qptr, queue_table[i].load);
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
	getEnvConfigInt(&qmax,   "QUEUE_MAX",   QUEUE_MAX);
	getEnvConfigInt(&qload,  "QUEUE_LOAD",  QUEUE_LOAD);
	if (!(qbase = env_get("QUEUE_BASE"))) {
		switch (control_readfile(&QueueBase, "queue_base", 0))
		{
		case -1:
			log_errf("alert: unable to read control file qbase\n");
			_exit(111);
			break;
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
static_queue(char **argv)
{
	int             i, wstat, child, nqueue;
	char           *qptr;

	if (env_get("SPAMFILTER")) {
		qptr = queuenum_to_dir(0);
		nqueue = !access(qptr, F_OK);
		if (!nqueue && errno != error_noent) {
			log_err("alert: queue ");
			log_err(qptr);
			log_err(": ");
			log_err(error_str(errno));
			log_errf("\n");
			die();
		}
	} else
		nqueue = 0;
	log_out("info: qscheduler: static queues qStart/qCount ");
	strnum[fmt_ulong(strnum, qstart)] = 0;
	log_out(strnum);
	log_out("/");
	strnum[fmt_ulong(strnum, qcount)] = 0;
	log_out(strnum);
	log_out(nqueue ? " nqueue+ conf split=" : " nqueue- conf split=");
	strnum[fmt_ulong(strnum, conf_split)] = 0;
	log_out(strnum);
	log_outf("\n");
	if (!queue_table) {
		if (!(queue_table = (pid_tab *) alloc(sizeof(pid_tab) * (qcount + 1))))
			nomem();
		for (i = 0; i <= qcount; i++) {
			queue_table[i].pid = -1;
			queue_table[i].queue_no = i;
			queue_table[i].load = 0;
			queue_table[i].queue_fix = 0;
		}
	}
	if (argv[1])
		qsargs[1] = argv[1]; /*- argument to qmail-start (./Mailbox or ./Maildir) */
	for (i = 0; i <= qcount; i++) {
		if (!i && !nqueue)
			continue;
		if (check_send(i))
			die();
		start_send(i);
	} /*- for (i = 0; i <= qcount; i++) */
	for (;!flagexitasap;) {
		if ((child = wait_pid(&wstat, -1)) == -1)
			break;
		if (flagexitasap)
			break;
		restart_send(child);
		sleep(1);
	} /*- for (; !flagexitasap;) */
	for (; flagexitasap;) {
		if ((child = wait_pid(&wstat, -1)) == -1)
			break;
	}
	log_errf("info: qscheduler: exiting\n");
	_exit(flagexitasap ? 0 : 111);
}

void
queue_fix(char *queuedir)
{
	pid_t           pid;
	int             wstat, exitcode;

	sig_block(sig_int);
	strnum[fmt_ulong(strnum, conf_split)] = 0;
	log_out("info: queue-fix: conf split=");
	log_out(strnum);
	log_out(", queuedir=");
	log_out(queuedir);
	log_outf("\n");
	switch ((pid = fork()))
	{
	case -1:
		log_err("alert: qscheduler: unable to fork: ");
		log_err(error_str(errno));
		log_errf("\n");
		die();
	case 0:
		sig_catch(sig_term, SIG_DFL);
		sig_catch(sig_hangup, SIG_DFL);
		sig_catch(sig_child, SIG_DFL);
		sig_catch(sig_int, SIG_DFL);
		if (chdir(auto_qmail) == -1)
			_exit(111);
		strnum[fmt_int(strnum, conf_split)] = 0;
		qfargs[2] = strnum;
		qfargs[3] = queuedir;
		execvp(*qfargs, qfargs); /*- queue-fix */
		strerr_die3sys(111, "alert: qscheduler: execv ", *qfargs, ": ");
	}
	sig_unblock(sig_int);
	if (wait_pid(&wstat, pid) != pid) {
		log_errf("alert: qscheduler: queue-fix waitpid surprise\n");
		_exit(111);
	}
	if (wait_crashed(wstat)) {
		log_errf("alert: qscheduler: queue-fix crashed\n");
		_exit(111);
	}
	exitcode = wait_exitcode(wstat);
	log_err(exitcode ? "warning: qcheduler: trouble fixing queue directory: " : "info: qcheduler: queue OK: ");
	log_err(queuedir);
	log_errf("\n");
}

void
dynamic_queue(char **argv)
{
	struct mq_attr  attr;
	unsigned int    priority, total_load;
	pid_tab        *client_tab;
	int             i, nqueue;
	char           *qptr;

	if (argv[1])
		qsargs[1] = argv[1]; /*- argument to qmail-start (./Mailbox or ./Maildir) */
	if (env_get("SPAMFILTER")) {
		qptr = queuenum_to_dir(0);
		nqueue = !access(qptr, F_OK);
		if (!nqueue && errno != error_noent)
			strerr_die3sys(111, "alert: qscheduler: queue ", qptr, ": ");
	} else
		nqueue = 0;
	create_ipc();
	for (qcount = 0; qcount < qmax; qcount++) {
		qptr = queuenum_to_dir(qcount + 1);
		if (access(qptr, F_OK)) {
			if (errno == error_noent)
				break;
			strerr_die3sys(111, "alert: qscheduler: queue ", qptr, ": ");
		}
	} /*- for (qcount = 1; qcount < qmax; qcount++) */
	if (lseek(shm, 0, SEEK_SET) == -1 || write(shm, &qcount, sizeof(int)) == -1)
		strerr_die1sys(111, "alert: qscheduler: unable to write to shared memory: ");
	log_out(nqueue? "info: qscheduler: dynamic queues nqueue+ qcount=" : "info: qscheduler: dynamic queues nqueue- qcount=");
	strnum[fmt_int(strnum, qcount)] = 0;
	log_out(strnum);
	log_out(", conf split=");
	strnum[fmt_ulong(strnum, conf_split)] = 0;
	log_out(strnum);
	log_outf("\n");
	if (!(queue_table = (pid_tab *) alloc(sizeof(pid_tab) * (qcount + 1))))
		nomem();
	for (i = 0; i <= qcount; i++) {
		queue_table[i].pid = -1;
		queue_table[i].queue_no = i;
		queue_table[i].queue_fix = 0;
		queue_table[i].load = 0;
	}
	/*- start qmail-send */
	for (i = 0; i <= qcount; i++) {
		if (!i && !nqueue)
			continue;
		if (check_send(i))
			die();
		start_send(i);
	} /*- for (i = 0; i <= qcount; i++) */
	for (; !flagexitasap;) {
		if (mq_getattr(mqd, &attr) == -1) {
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
		do_mq = 1;
		if (mq_receive(mqd, msgbuf, msgbuflen, &priority) == -1) {
			if (errno == error_intr) {
				printf("intr %d\n", flagexitasap);
				continue;
			}
			strerr_warn1("alert: qscheduler: unable to read message queue: ", &strerr_sys);
			die();
		}
		do_mq = 0;
		client_tab = (pid_tab *) msgbuf;
		queue_table[client_tab->queue_no].load += client_tab->load;
		for (i = 1, total_load = 0; i <= qcount; i++)
			total_load += queue_table[i].load;
		if (total_load > qcount * qload && qcount < qmax) {
			if (!alloc_re(&queue_table, qcount + 1, qcount + 2))
				nomem();
			qcount++;
			queue_table[qcount].pid = -1;
			queue_table[qcount].queue_no = i;
			queue_table[qcount].queue_fix = 0;
			queue_table[qcount].load = 0;
			qptr = queuenum_to_dir(qcount);
			if (access(qptr, F_OK)) {
				if (errno == error_noent)
					queue_fix(qptr);
				log_err("alert: qscheduler: ");
				log_err(qptr);
				log_err(": ");
				log_err(error_str(errno));
				log_errf("\n");
				die();
			}
			if (check_send(qcount))
				die();
			start_send(qcount);
			if (lseek(shm, 0, SEEK_SET) == -1 || write(shm, &qcount, sizeof(int)) == -1)
				strerr_die1sys(111, "alert: qscheduler: unable to write to shared memory: ");
		}
		printf("msg[queue_no=%d load=%ld] priority=%d\n", client_tab->queue_no, client_tab->load, priority);
		sleep(1);
	} /*- for (; !flagexitasap;) */
	log_errf("info: qscheduler: exiting\n");
	_exit(flagexitasap ? 0 : 111);
}

int
main(int argc, char **argv)
{
	char           *ptr;
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
	if ((fd = open_trunc("queue/qscheduler")) == -1) {
		log_err("alert: qscheduler: cannot start: unable to open queue/qscheduler: ");
		log_err(error_str(errno));
		log_errf("\n");
		_exit(111);
	}
	if (lock_exnb(fd) == -1) {
		log_errf("alert: cannot start: qscheduler is already running\n");
		_exit(111);
	}
	if (!(ptr = env_get("QUEUE")))
		qtype = fixed;
	else {
		if (!str_diff(ptr, "static"))
			qtype = fixed;
		else
		if (!str_diff(ptr, "dynamic"))
			qtype = dynamic;
		else
			qtype = fixed;
	}
	set_queue_variables();
	if (qtype == dynamic)
		dynamic_queue(argv);
	else
		static_queue(argv);
	if (flagexitasap)
		log_errf("qscheduler: exiting\n");
	return 0;
}

void
getversion_queue_scheduler_c()
{
	static char    *x = "$Id: $";

	x++;
}
