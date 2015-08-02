/** \file db_lock.h
 * \brief header file for bogofilter's crash detector
 * \author Matthias Andree
 * \date 2004
 *
 * GNU GPL v2
 */

#ifndef DB_LOCK_H
#define DB_LOCK_H

/* function prototypes */

/** create and open lock file in \a bogohomedir
 * \return
 * - 0 for success
 * - -1 for error */
int	init_dbl(const char *bogohomedir);

/** set the next free lock cell and initialize the periodic crash
 * checker, which will _exit() the program when another process has
 * crashed. Requires that init_dbl() has been called before. \return
 * - 0 for success
 * - -2 if a process has just crashed
 * - -1 for error */
int	set_lock(void);

/** end the periodic crash checker and unlock our lock cell, and closes
 * the lock file. Requires that init_dbl() and set_lock() have been
 * called before. \return
 * - 0 for success
 * - -1 for error */
int	clear_lock(void);

/** reinitialize the lock file, which must pre-exist. Requires that
 * init_dbl() has been called before.  \return
 * - 0 for success
 * - -1 for error */
int	clear_lockfile(void);

/** checks if a process has crashed previously. Requires that init_dbl()
 * has been called before. \return
 * - 0 if no process has previously crashed
 * - 1 if a process has crashed previously. */
int	needs_recovery(void);

#endif /* DB_LOCK_H */
