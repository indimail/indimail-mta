/*
 * $Log: module.c,v $
 * Revision 1.3  2008-07-17 21:38:26+05:30  Cprogrammer
 * moved progname to variables.h
 *
 * Revision 1.2  2008-06-09 15:31:09+05:30  Cprogrammer
 * added GPL copyright notice
 *
 * Revision 1.1  2002-12-16 01:55:03+05:30  Manny
 * Initial revision
 *
 *
 * Set up and initialise modules
 *
 * Copyright (C) Stephen Fegan 1995-1997
 *
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation; either version 2 of the License, or (at your option) any later
 * version.
 * 
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 * 
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 675 Mass
 * Ave, Cambridge, MA 02139, USA.
 * 
 * please send patches or advice on flash to: `flash@netsoc.ucd.ie'
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<pwd.h>
#include<sys/types.h>
#include<sys/resource.h>
#include<sys/wait.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<unistd.h>
#include<dlfcn.h>
#include<time.h>

#include"set.h"
#include"rc.h"
#include"exec.h"
#include"parse.h"
#include"misc.h"
#include"mystring.h"
#include"parseline.h"
#include"module.h"

time_t          TimeNow;

typedef void    (*INITMOD) (int argc, char **argv, FILE * fp, int *lc);
typedef int     (*FUNCTION) (int argc, char **argv);

struct InternalModules
{
	char           *name;
	INITMOD         InitModule;
};

struct InternalModules InternalModules[] = {
	{"background", InitBackground},
	{"barclock", InitBarClock},
	{"mailcheck", InitMailCheck},
	{NULL, NULL}
};

void
internal_module(int wordc, char **wordv, FILE * fp, int *line_no)
{
	char           *modname;
	struct InternalModules *x = InternalModules;

	wordv++, wordc--;
	modname = *wordv;
	while (x->name != NULL)
	{
		if (strcasecmp(modname, x->name) == 0)
		{
#ifdef DEBUG
			fprintf(stderr, "Found INTERNAL module %s, initialising\n", x->name);
#endif
			(*x->InitModule) (wordc, wordv, fp, line_no);
			return;
		}
		x++;
	}
	fprintf(stderr, "Could not find INTERNAL module: %s\n", modname);
	return;
}

struct ExternalModule
{
	struct ExternalModule *next, *prev;
	char           *name;
	void           *handle;
	INITMOD         InitModule;
};

struct ExternalModule ExtModules = { &ExtModules, &ExtModules, NULL, NULL, NULL };

static struct ExternalModule *
FindModule(char *modfile)
{
	struct ExternalModule *h, *x;

	for (h = &ExtModules, x = ExtModules.next; x != h; x = x->next)
		if (strcmp(x->name, modfile) == 0)
			break;

	if (x == h)
		return NULL;
	else
		return x;
}

static struct ExternalModule *
LoadModule(char *modfile)
{
	char           *name;
	void           *handle;
	INITMOD         InitMod;
	struct ExternalModule *x, *h;
	const char     *error;

	name = xmalloc(strlen(modfile) + 1);
	strcpy(name, modfile);
	modfile = stradp(modfile, 1);
	handle = dlopen(modfile, RTLD_LAZY);
	if (!handle)
	{
		fprintf(stderr, "module: could not load %s: %s\n", modfile, dlerror());
		free(name);
		return NULL;
	}
#ifdef DEBUG
	fprintf(stderr, "Loaded module %s\n", modfile);
#endif
	InitMod = (INITMOD) dlsym(handle, "ModuleInit");
	if ((error = dlerror()) != NULL)
	{
		fprintf(stderr, "module: could not find module initialisation function: %s\n", error);
		free(name);
		return NULL;
	}
	x = xmalloc(sizeof(*x));
	h = &ExtModules;
	x->name = name;
	x->handle = handle;
	x->InitModule = InitMod;
	x->next = h;
	x->prev = h->prev;
	h->prev->next = x;
	h->prev = x;
	return x;
}

void
module(int wordc, char **wordv, FILE * fp, int *line_no)
{
	char           *modfile;
	struct ExternalModule *x;

	wordv++, wordc--;
	modfile = *wordv;
	if (strcasecmp(modfile, "internal") == 0)
		internal_module(wordc, wordv, fp, line_no);
	else
	{
		x = FindModule(modfile);
		if (x == NULL)
			x = LoadModule(modfile);
		if (x == NULL)
			return;
		time(&TimeNow);
		(*x->InitModule) (wordc, wordv, fp, line_no);
	}

	return;
}

void
RunModuleFunction(int argc, char **argv)
{
	char           *modfile, *function;
	const char     *error;
	struct ExternalModule *x;
	FUNCTION        f;

	modfile = *argv;
	for (function = modfile; (*function != '\0') && (*function != ':'); function++);
	if (*function == '\0')
		function = "ModuleMain";
	else
		*(function++) = '\0';

	x = FindModule(modfile);
	if (x == NULL)
	{
		x = LoadModule(modfile);
		if (x == NULL)
			return;
		time(&TimeNow);
		(*x->InitModule) (argc, argv, NULL, NULL);
	}

	f = (FUNCTION) dlsym(x->handle, function);
	if ((error = dlerror()) != NULL)
	{
		fprintf(stderr, "%s: could not find function %s\nmodule: %s\n", modfile, function, error);
		return;
	}
	time(&TimeNow);
	(*f) (argc, argv);
	return;
}
