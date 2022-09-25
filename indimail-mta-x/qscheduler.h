/*
 * $Log: qscheduler.h,v $
 * Revision 1.1  2022-04-16 21:45:55+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifndef _QSCHEDULER_H_
#define _QSCHEDULER_H_
#include <sys/types.h>

#ifndef QUEUE_COUNT
#define QUEUE_COUNT 10
#endif
#ifndef QUEUE_MAX
#define QUEUE_MAX 20
#endif
#ifndef QUEUE_LOAD
#define QUEUE_LOAD 50
#endif

typedef enum {
	fixed,
	dynamic,
} q_type;

/*-
 * queue message
 * used by qmail-queue to communicate with todo-proc
 */
typedef struct
{
	pid_t           pid;
	int             split;
	ino_t           inum;      /*- inode number */
} q_msg;

typedef struct
{
	pid_t           pid;
	unsigned int    queue_no;
	unsigned long   load;      /*- concurrency used percent */
} qtab;

#endif
