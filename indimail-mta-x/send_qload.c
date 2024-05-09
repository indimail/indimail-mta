/*
 * $Id: send_qload.c,v 1.2 2024-05-09 22:03:17+05:30 mbhangui Exp mbhangui $
 */
#include "haslibrt.h"
#ifdef HASLIBRT
#include <mqueue.h>
#include <fcntl.h>
#include <strerr.h>
#include <stddef.h>
#include <sys/stat.h>
#include "qscheduler.h"

int
send_qload(const char *queue_ident, unsigned int queue_no,
		long load, unsigned int priority)
{
	mqd_t           mqd;
	qtab            queue_tab;

	if ((mqd = mq_open(queue_ident, O_WRONLY,  0600, NULL)) == (mqd_t) -1) {
		strerr_warn3("send_qload: ", queue_ident, ": ", &strerr_sys);
		return -1;
	}
	queue_tab.queue_no = queue_no;
	queue_tab.load = load;
	if (mq_send(mqd, (char *) &queue_tab, sizeof(qtab), priority) == -1) {
		strerr_warn3("send_qload: mq_send: ", queue_ident, ": ", &strerr_sys);
		return -1;
	}
	if (mq_close(mqd) == -1) {
		strerr_warn3("send_qload: mq_close: ", queue_ident, ": ", &strerr_sys);
		return -1;
	}
	return 0;
}
#endif

void
getversion_send_qload_c()
{
	const char     *x = "$Id: send_qload.c,v 1.2 2024-05-09 22:03:17+05:30 mbhangui Exp mbhangui $";

	x++;
}

/*
 * $Log: send_qload.c,v $
 * Revision 1.2  2024-05-09 22:03:17+05:30  mbhangui
 * fix discarded-qualifier compiler warnings
 *
 * Revision 1.1  2022-04-24 08:48:22+05:30  Cprogrammer
 * Initial revision
 *
 */
