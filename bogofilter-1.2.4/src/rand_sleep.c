#include "config.h"
#include "system.h"

#include "rand_sleep.h"

#include <stdlib.h>

void rand_sleep(double min, double max)
{
    long delay;
#ifdef HAVE_ARC4RANDOM
    delay = (int)(min + (max-min)*arc4random()/0xFFFFFFFFu);
#else
    static bool need_init = true;

    if (need_init) {
	struct timeval timeval;
	need_init = false;
	gettimeofday(&timeval, NULL);
	srand48(timeval.tv_usec ^ timeval.tv_sec);
    }
    delay = (int)(min + ((max-min)*drand48()));
#endif
    bf_sleep(delay);
}
