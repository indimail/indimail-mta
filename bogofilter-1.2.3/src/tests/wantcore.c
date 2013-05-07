/* wantcore -- run a program with soft core file size limit set to hard limit
 * Copyright © 2004 Matthias Andree

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or (at
   your option) any later version.

   This program is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details. The license is found in the file
   ../../COPYING.
 */

#include "system.h"

#include <sys/resource.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

static void barf(const char *e) __attribute__((noreturn));
static void barf(const char *e)
{
    perror(e);
    exit(EXIT_FAILURE);
}

int main(int argc, char **argv) {
#ifndef __EMX__
    struct rlimit rl;
#endif

    if (argc <= 1) {
	fprintf(stderr, "Usage: %s program [args]\n", argv[0]);
	exit(EXIT_FAILURE);
    }
#ifndef __EMX__
    if (getrlimit(RLIMIT_CORE, &rl))
	barf("getrlimit");
    rl.rlim_cur = rl.rlim_max;
    if (setrlimit(RLIMIT_CORE, &rl))
	barf("setrlimit");
#endif
    execv(argv[1], argv+1);
    fprintf(stderr, "execv: ");
    barf(argv[1]);
}
