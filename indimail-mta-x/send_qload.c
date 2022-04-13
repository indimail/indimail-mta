/*
 * $Id: $
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
send_qload(char *queue_ident, unsigned int queue_no,
		long load, unsigned int priority)
{
	mqd_t           mqd;
	qtab            queue_tab;

	if ((mqd = mq_open(queue_ident, O_WRONLY,  0600, NULL)) == -1)
		strerr_die3sys(111, "send_qload: ", queue_ident, ": ");
	queue_tab.queue_no = queue_no;
	queue_tab.load = load;
	if (mq_send(mqd, (char *) &queue_tab, sizeof(qtab), priority) == -1)
		strerr_die3sys(111, "send_qload: mq_send: ", queue_ident, ": ");
	if (mq_close(mqd) == -1)
		strerr_die3sys(111, "send_qload: mq_close: ", queue_ident, ": ");
	return 0;
}
#endif

void
getversion_send_qload_c()
{
	static char    *x = "$Id: $";

	x++;
}

/*
 * $Log: $
 */
