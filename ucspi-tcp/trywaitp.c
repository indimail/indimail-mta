/*
 * $Log: trywaitp.c,v $
 * Revision 1.1  2003-12-31 19:47:31+05:30  Cprogrammer
 * Initial revision
 *
 */
#include <sys/types.h>
#include <sys/wait.h>

void
main()
{
	waitpid(0, 0, 0);
}
