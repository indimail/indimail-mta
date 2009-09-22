/*
 * $Log: rc.h,v $
 * Revision 1.1  2002-12-16 01:55:41+05:30  Manny
 * Initial revision
 *
 */
#if !defined (_RC_H)
#   define _RC_H

#include"parse.h"

#define rcflag_restricted 0x1
#define rcflag_system_only 0x2

enum rcfile_type
{
	SYSTEM, USER
};

struct rc_commands
{
	char           *command;
	int             flags;
	void            (*runrc) (int, char **);
};

void            rc_file(int wordc, char **wordv, FILE * fp, int *line_no);
void            rc_block(int wordc, char **wordv, FILE * fp, int *line_no);
int             processrcfile(char *rcfilename, enum rcfile_type rctype);

void            rc_setenv(int argc, char **argv);
void            rc_unsetenv(int argc, char **argv);
void            rc_set(int argc, char **argv);
void            rc_unset(int argc, char **argv);
void            rc_exec(int argc, char **argv);
void            rc_system(int argc, char **argv);
void            rc_pause(int argc, char **argv);
void            rc_sleep(int argc, char **argv);
void            rc_logout(int argc, char **argv);
void            rc_set_booloption(int argc, char **argv, int *booloption);
void            rc_if(int argc, char **argv);
void            rc_else(int argc, char **argv);
void            rc_endif(int argc, char **argv);
void            rc_restrict(int argc, char **argv);
void            rc_arseme(int argc, char **argv);

#endif
