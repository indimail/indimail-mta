/* $Id: bogoQDBMupgrade.c 6484 2006-05-29 14:28:00Z relson $ */

/*
  bogoQDBMupgrade.c -- convert QDBM data base from Hash to B+Tree format
  Copyright (C) 2004  Stefan Bellon

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or (at
  your option) any later version.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
*/

/* additional error checks and cleanups by Matthias Andree */

#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <depot.h>
#include <cabin.h>
#include <villa.h>
#include <stdlib.h>

#include "datastore_qdbm.h"

int main(int argc, char *argv[])
{
    DEPOT *dho;
    VILLA *dhn;
    char *new_name;
#define TACKON "-new"
    const char *tackon = TACKON;

    int ret;
    int ksiz, dsiz;
    char *key, *data;

    if (argc < 2 || argc > 4) {
	fprintf(stderr,
		"Usage: %s <database> [<new database>]\n"
		"   or: %s <database> <tempfile> <backup>\n"
		"  <new database> defaults to <database>" TACKON "\n"
		"  If <backup> is given, <database> is backed up to <backup> and\n"
		"  upgraded in place, via <tempfile>.\n",
		argv[0], argv[0]);
	exit(EXIT_FAILURE);
    }

    dho = dpopen(argv[1], DP_OREADER, 0);
    if (!dho) {
	fprintf(stderr, "Couldn't open database '%s': %s\n", argv[1],
		dperrmsg(dpecode));
	exit(EXIT_FAILURE);
    }

    if (dpgetflags(dho) & 1) {
	fprintf(stderr, "Database '%s' is already in B+ tree format.\n", argv[1]);
	dpclose(dho);
	exit(EXIT_FAILURE);
    }

    if (argc >= 3) {
	new_name = strdup(argv[2]);
    } else {
	new_name = malloc(strlen(argv[1]) + strlen(tackon) + 1);
	if (new_name) {
	    strcpy(new_name, argv[1]);
	    strcat(new_name, tackon);
	}
    }

    if (!new_name) {
	fprintf(stderr, "Couldn't allocate memory.\n");
	dpclose(dho);
	exit(EXIT_FAILURE);
    }

    if (strcmp(new_name, argv[1]) == 0) {
	fprintf(stderr, "database and %s must be different files!\n",
		argc >= 4 ? "tempfile" : "new database");
	dpclose(dho);
	free(new_name);
	exit(EXIT_FAILURE);
    }

    remove(new_name); /* start with a fresh data base */

    dhn = vlopen(new_name, VL_OWRITER | VL_OCREAT, cmpkey);
    if (!dhn) {
	fprintf(stderr, "Couldn't create database '%s': %s\n", new_name,
		dperrmsg(dpecode));
	dpclose(dho);
	remove(new_name);
	free(new_name);
	exit(EXIT_FAILURE);
    }

    ret = dpiterinit(dho);
    if (ret) {
	while ((key = dpiternext(dho, &ksiz))) {
	    data = dpget(dho, key, ksiz, 0, -1, &dsiz);
	    if (data) {
		ret = vlput(dhn, key, ksiz, data, dsiz, VL_DOVER);
		if (!ret) {
		    int i;
		    fprintf(stderr, "Error writing key '%.*s', value '0x",
			    ksiz, key);
		    for (i = 0 ; i < dsiz ; i++)
			fprintf(stderr, "%02x", data[i]);
		    fprintf(stderr, "': %s\n", dperrmsg(dpecode));
		    free(data);
		    free(key);
		    goto barf;
		}
		free(data);
	    } else {
		fprintf(stderr, "Error reading value for key '%.*s': %s\n",
			ksiz, key, dperrmsg(dpecode));
		free(key);
		goto barf;
	    }
	    free(key);
	}
    } else {
	fprintf(stderr, "Error creating database iterator: %s\n",
		dperrmsg(dpecode));
	goto barf;
    }

    if (dpfatalerror(dho)) {
	fprintf(stderr, "Error: database has fatal error state after reading.\n"
		"Last error was: %s", dperrmsg(dpecode));
	goto barf;
    }

    if (!dpclose(dho)) {
	fprintf(stderr, "Error closing input database: %s\n", dperrmsg(dpecode));
	goto barf;
    }
    if (!vlclose(dhn)) {
	fprintf(stderr, "Error closing output database: %s\n", dperrmsg(dpecode));
	goto barf;
    }

    if (argc >= 4) {
	/* rename output to input, taking care the file is always
	 * present */
	remove(argv[3]); /* ignore errors */
	if (link(argv[1], argv[3])) {
	    fprintf(stderr, "Error: cannot link %s to %s: %s\n",
		    argv[1], argv[3], strerror(errno));
	    free(new_name);
	    exit(EXIT_FAILURE);
	}

	if (rename(new_name, argv[1])) {
	    fprintf(stderr, "Error: cannot rename %s to %s: %s\n",
		    new_name, argv[1], strerror(errno));
	    free(new_name);
	    exit(EXIT_FAILURE);
	}
    }

    exit(EXIT_SUCCESS);

    /* clean up in case of error */
barf:
    vlclose(dhn);
    remove(new_name);
    free(new_name);
    dpclose(dho);
    exit(EXIT_FAILURE);
}
