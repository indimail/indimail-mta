#ifndef _DISPLAY_SHM_H_
#define _DISPLAY_SHM_H_
#include <stralloc.h>

typedef struct {
	stralloc queue;
	int lcur;
	int lmax;
	int rcur;
	int rmax;
	char flag;
} QDEF;

void            queue_load(char *, int *, int *, double qload[2], QDEF **);

#endif
