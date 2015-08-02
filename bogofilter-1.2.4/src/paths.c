/* $Id: paths.c 6207 2005-08-14 13:10:06Z relson $ */

/**
 * \file
 * paths.c -- routines for working with file paths.
 */

#include "common.h"

#include <errno.h>
#include <sys/stat.h>
#include <stdlib.h>

#include "find_home.h"
#include "mxcat.h"
#include "paths.h"
#include "xmalloc.h"
#include "xstrdup.h"

/* Local  Data */

typedef struct {
    priority_t p;	/* precedence */
    const char *v;	/* env var    */
    const char *s;	/* sub dir    */
} map_pri_env;

static map_pri_env pri_2_env[] = {
#ifndef __riscos__
    { PR_ENV_BOGO, "BOGOFILTER_DIR", NULL },
    { PR_ENV_BOGO, "BOGODIR",	     NULL },
    { PR_ENV_HOME, "HOME",	     BOGODIR }
#else
    { PR_ENV_HOME, "Choices$Write",  BOGODIR },
    { PR_ENV_HOME, "Bogofilter$Dir", NULL },
#endif
};

/* Function Definitions */

char *bogohome = NULL;

void set_bogohome(const char *path)
{
    xfree(bogohome);
    bogohome = xstrdup(path);
}

void chk_bogohome(void)
{
    if (!check_directory(bogohome)) {
	(void)fprintf(stderr, "%s: cannot find bogofilter directory.\n"
		      "You must specify a directory on the command line, in the config file,\n"
#ifndef __riscos__
		      "or by using the BOGOFILTER_DIR or HOME environment variables.\n"
#else
		      "or by ensuring that <Bogofilter$Dir> is set correctly.\n"
#endif
		      "Program aborting.\n", progname);
	exit(EX_ERROR);
    }
}

static bool cant_find_bogohome(void)
{
    if (bogohome != NULL)
	return false;

    if (set_wordlist_dir(NULL, PR_ENV_BOGO) == 0)
	return false;
    if (set_wordlist_dir(NULL, PR_ENV_HOME) == 0)
	return false;

    return true;
}

void bogohome_cleanup(void)
{
    xfree(bogohome);
    bogohome = NULL;
}

int set_wordlist_dir(const char* d, priority_t precedence)
{
    int rc = 0;
    char *dir;
    static priority_t saved_precedence = PR_NONE;

    if (DEBUG_WORDLIST(2))
	fprintf(dbgout, "p: %d, s: %d\n", (int) precedence, (int) saved_precedence);

    if (precedence < saved_precedence)
	return rc;

    dir = (d != NULL) ? tildeexpand(d) : get_directory(precedence);
    if (dir == NULL)
	return -1;

    if (DEBUG_WORDLIST(2))
	fprintf(dbgout, "d: %s\n", dir);

    saved_precedence = precedence;

    set_bogohome(dir);

    xfree(dir);

    return rc;
}

char *get_directory(priority_t which)
{
    size_t i;
    char *dir = NULL;

    for (i = 0; i < COUNTOF(pri_2_env) ; i += 1) {
	map_pri_env *p2e = &pri_2_env[i];
	if (p2e->p == which) {
	    dir = create_path_from_env(p2e->v, p2e->s);
	    if (dir)
		break;
	}
    }
    return dir;
}

static bfpath *bfpath_split(bfpath *bfp, const char *home)
{
    /* precondition:  bfp->dirname and bfp->filename free'd if need be */
    char *t = strrchr(bfp->filepath, DIRSEP_C);

    xfree(bfp->dirname);
    xfree(bfp->filename);

    if (t != NULL) {
	/* if directory separator present .... */
	*t = '\0';
	bfp->dirname = xstrdup(bfp->filepath);
	*t = DIRSEP_C;
	bfp->filename = xstrdup(t+1);
    }
    else if (home != NULL){
	bfp->dirname = xstrdup(home);
	bfp->filename = bfp->filepath;
	bfp->filepath = mxcat(bfp->dirname, DIRSEP_S, bfp->filename, NULL);
    }
    else {
	bfp->dirname = NULL;
	bfp->filename = xstrdup(bfp->filepath);
    }

    return bfp;
}

bfpath *bfpath_create(const char *path)
{
    bfpath *bfp = xcalloc(1, sizeof(bfpath));
    bfp->filepath = xstrdup(path);

    return bfp;
}

static void check_for_file(bfpath *bfp)
{
    int rc;
    struct stat sb;

    bfp->isdir = bfp->isfile = false;

    rc = stat(bfp->filepath, &sb);
    if (rc == 0) {
	bfp->exists = true;
	xfree(bfp->dirname);
	xfree(bfp->filename);
	if (S_ISDIR(sb.st_mode)) {
	    bfp->isdir = true;
	    bfp->dirname  = xstrdup(bfp->filepath);
	    bfp->filename = NULL;
	}
	if (!S_ISDIR(sb.st_mode)) {
	    bfp->isfile = true;
	    bfp->dirname  = get_directory_from_path(bfp->filepath);
	    bfp->filename = get_file_from_path(bfp->filepath);
	}
    }
    return;
}

bool bfpath_check_mode(bfpath *bfp, bfpath_mode m)
{
    bool ok = true;

    bfp->checked = true;

    if (bfp->filepath != NULL && bfp->dirname == NULL && bfp->filename == NULL) {
	char *t = strrchr(bfp->filepath, DIRSEP_C);
	if (t == NULL)
	    bfp->filename = xstrdup(bfp->filepath);
	else {
	    bfp->dirname = xstrdup(bfp->filepath);
	    bfp->dirname[t - bfp->filepath] = '\0';
	    bfp->filename = xstrdup(t+1);
	}
    }

    check_for_file(bfp);

    switch (m)
    {
    case BFP_MUST_EXIST:
	if (!bfp->exists)
	    ok = false;
	break;
    case BFP_MAY_CREATE:
	break;
    case BFP_ERROR:
	/* can't get here */
	abort();
    }

    if (bfp->dirname != NULL && bogohome == NULL)
	set_bogohome(bfp->dirname);

    return ok;
}

void bfpath_set_bogohome(bfpath *bfp)
{
    /* ensure bogohome is set */
    if (cant_find_bogohome()) {
	fprintf(stderr, "Can't find HOME or BOGOFILTER_DIR in environment.\n");
	exit(EX_ERROR);
    }

    bfpath_split(bfp, bogohome);
}

void bfpath_set_filename(bfpath *bfp, const char *filename)
{
    xfree(bfp->filename);
    bfp->filename = xstrdup(filename);
    xfree(bfp->filepath);
    bfp->filepath = mxcat(bfp->dirname, DIRSEP_S, bfp->filename, NULL);
    check_for_file(bfp);
    return;
}

bfpath *bfpath_free(bfpath *bfp)
{
    xfree(bfp->dirname);
    xfree(bfp->filename);
    xfree(bfp->filepath);
    xfree(bfp);
    return NULL;
}

char *build_progtype(const char *name, const char *db_type)
{
    char *type;
    if (strcmp(db_type, "db") == 0)
	type = xstrdup(name);
    else {
	size_t len = strlen(name) + strlen(db_type) + 2;
	type = xmalloc(len);
	snprintf(type, len, "%s-%s", name, db_type);
    }
    return type;
}

char *create_path_from_env(const char *var,
		/*@null@*/ const char *subdir)
{
    char *buff, *env;
    size_t path_size, env_size;

    env = getenv(var);
    if (env == NULL || *env == '\0') return NULL;

    env_size = strlen(env);
    path_size = env_size + (subdir ? strlen(subdir) : 0) + 2;
    buff = xmalloc(path_size);

    strlcpy(buff, env, path_size);
    if (subdir != NULL) {
	if (buff[env_size-1] != DIRSEP_C)
	    strlcat(buff, DIRSEP_S, path_size);
	strlcat(buff, subdir, path_size);
    }
    if (strlcat(buff, "", path_size) >= path_size)
	abort(); /* buffer overrun, this cannot happen - buff is xmalloc()d */
    return buff;
}

bool check_directory(const char* path) /*@globals errno,stderr@*/
{
    int rc;
    struct stat sb;

    if (path == NULL || *path == '\0')
	return false;

    rc = stat(path, &sb);
    if (rc < 0) {
	if (ENOENT==errno) {
	    if (bf_mkdir(path, S_IRUSR|S_IWUSR|S_IXUSR)) {
		fprintf(stderr, "Error creating directory '%s': %s\n",
			path, strerror(errno));
		return false;
	    } else if (verbose > 0) {
		fprintf(dbgout, "Created directory %s .\n", path);
	    }
	    return true;
	} else {
	    fprintf(stderr, "Error accessing directory '%s': %s\n",
		    path, strerror(errno));
	    return false;
	}
    } else {
	if (! S_ISDIR(sb.st_mode)) {
	    fprintf(stderr, "Error: %s is not a directory.\n", path);
	    return false;
	}
    }
    return true;
}

/** returns malloc()ed copy of the file name part of \a path.
 */
char *get_file_from_path(const char *path)
{
    char *file = strrchr(path, DIRSEP_C);
    if (file == NULL)
	file = xstrdup(path);
    else
	file = xstrdup(file + 1);
    return file;
}

/** returns malloc()ed copy of the directory name of \a path.
 */
char *get_directory_from_path(const char *path)
{
    char *dir = xstrdup(path);
    char *last = strrchr(dir, DIRSEP_C);
    if (last == NULL) {
	xfree(dir);
	return NULL;
    }
    else {
	*last = '\0';
	return dir;
    }
}
