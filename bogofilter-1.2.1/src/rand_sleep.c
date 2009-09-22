#include "config.h"
#include "system.h"

#include "rand_sleep.h"

#include <stdlib.h>

void rand_sleep(double min, double max)
{
    static bool need_init = true;
    long delay;

    if (need_init) {
	struct timeval timeval;
	need_init = false;
	gettimeofday(&timeval, NULL);
	srand((uint)timeval.tv_usec); /* RATS: ignore - this is safe enough */
    }
    delay = (int)(min + ((max-min)*rand()/(RAND_MAX+1.0)));
    bf_sleep(delay);
}
