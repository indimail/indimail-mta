/*
 * $Log: $
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

#ifndef DARWIN
typedef enum {
	fixed,
	dynamic,
} q_type;
#endif

/*-
 * queue message
 * used by qmail-queue to communicate with qmail-todo
 */
typedef struct
{
	pid_t           pid;
	int             split;
	ino_t           inum;      /*- inode number */
} q_msg;

#endif
