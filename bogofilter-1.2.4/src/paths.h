/* $Id: paths.h 6207 2005-08-14 13:10:06Z relson $ */

/**
 * \file
 * paths.h -- prototypes and definitions for paths.c.
 */

#ifndef PATHS_H
#define PATHS_H

#include "common.h"

#define	OLD_PATHS

/* Path Definitions */

#if !defined(_OS2_) && !defined(__riscos__)
  #define BOGODIR ".bogofilter"
#else
  #define BOGODIR "bogofilter"
#endif

#define WORDLIST  "wordlist" DB_EXT

extern char *bogohome;

typedef enum bfmode_e {
    BFP_ERROR,
    BFP_MUST_EXIST,
    BFP_MAY_CREATE
} bfpath_mode;

/* typedef and struct definitions */

typedef struct bfpath {
    bool   checked;	/* if bfpath_check_mode has been called */
    bool   exists;
    bool   isdir;
    bool   isfile;
    char  *dirname;	/* directory only   */
    char  *filename;	/* filename only    */
    char  *filepath;	/* dirname+filename */
} bfpath;

/* Function Prototypes */

bfpath *bfpath_create(const char *path);
void	bfpath_set_bogohome(bfpath *bfp);
bool	bfpath_check_mode(bfpath *bfp, bfpath_mode mode);
bfpath *bfpath_free(bfpath *path);
bool	paths_equal(bfpath *p1, bfpath *p2);
void	bfpath_set_filename(bfpath *bfp, const char *filename);

void	set_bogohome(const char *dirname);
void	chk_bogohome(void);
void	set_bogohome_using_dirname(const char *dirname);
void	bogohome_cleanup(void);
int	set_wordlist_dir(const char* dir, priority_t precedence);

enum bfpath_e { BFP_NORMAL = 1 };
typedef enum bfpath_e bfpath_t;

/** Build a path to a file given a directory and file name in malloc()d
 * memory (caller freed), concatenating dir and file, adding a slash if
 * necessary.  \return
 * - true for success
 * - false for error (esp. overflow)
 */
char *build_path(const char* dir, const char* file);

char *build_progtype(const char *name, const char *db_type);

/** If the given environment variable \a var exists, create a path from
 * it and tack on the optional \a subdir value.
 * \return
 * - buffer address if success
 * - NULL if failure
 */
char *create_path_from_env(const char *var,
			   /*@null@*/ const char *subdir);

char *get_directory(priority_t which);

/** \return malloc'd copy of just the file name of \a path */
char *get_file_from_path(const char *path);

/** \return malloc'd copy of just the directory name of \a path */
char *get_directory_from_path(const char *path);

/** Check whether \a path is a directory or a symlink to a directory.
 * \return
 * - true if \a path is a directory
 * - false if \a path is a file or an error occurred */
bool is_dir(const char* path)		/*@globals errno,stderr@*/;

/** Check whether \a path is a file (everything that is not a directory
 * or a symlink to a directory).
 * \return
 * - true if \a path is a file
 * - false if \a path is a directory or an error occurred */
bool is_file(const char* path)		/*@globals errno,stderr@*/;

/** Check that directory \a path exists and try to create it otherwise.
 * \return
 * - true on success
 * - false on error */
bool check_directory(const char *path)	/*@globals errno,stderr@*/;

#endif	/* PATHS_H */
