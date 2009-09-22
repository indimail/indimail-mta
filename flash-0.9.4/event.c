/*
 * $Log: event.c,v $
 * Revision 1.1  2002-12-16 01:54:57+05:30  Manny
 * Initial revision
 *
 */
#include<unistd.h>
#include<sys/time.h>

#include "event.h"

struct event_s  ScheduleHead, *SH;

void
init_schedule(void)
{
	SH = &ScheduleHead;

	SH->action = NULL;
	SH->p = NULL;
	SH->expire = 0;

	SH->next = NULL;
}

void
finish_schedule(void)
{
}

void            schedule_event(void (a) (void *p), void *param, time_t * exp);
