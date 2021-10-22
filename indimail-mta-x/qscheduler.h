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

typedef enum {
	fixed,
	dynamic,
} q_type;

typedef struct
{
	pid_t           pid;
	unsigned int    queue_no;
	unsigned long   load;
	char            queue_fix;
} pid_tab;

#endif
