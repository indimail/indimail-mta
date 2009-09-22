/*
 * $Log: event.h,v $
 * Revision 1.1  2002-12-16 01:55:30+05:30  Manny
 * Initial revision
 *
 */
#if !defined (_EVENT_H)
#   define _EVENT_H

#include<time.h>

struct event_s
{
	void            (*action) (void *p);
	void           *p;
	time_t          expire;
	struct event_s *next;
};

void            schedule_event(void (*) (void *), void *, time_t *);
void            init_schedule(void);
void            finish_schedule(void);

#endif
