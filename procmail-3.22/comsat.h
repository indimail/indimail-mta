/*
 * $Id: comsat.h,v 1.1 2001/06/21 11:59:26 guenther Exp $ 
 */

#ifndef NO_COMSAT

void setlfcs    P((const char *folder));
void setlgcs    P((const char *name));
void sendcomsat P((const char *folder));
int setcomsat   P((const char *chp));

#else

/*
 * If no comsat support, then they all do nothing 
 */
#define setlfcs(x)	0
#define setlgcs(x)	0
#define sendcomsat(x)	0
#define setcomsat(x)	0

#endif
