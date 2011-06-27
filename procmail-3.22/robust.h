/*
 * $Id: robust.h,v 1.13 2001/06/21 09:43:53 guenther Exp $
 */

void nomemerr   Q((const size_t len)) __attribute__ ((noreturn));
void           *tmalloc Q((const size_t len));
void           *trealloc Q((void *const old, const size_t len));
void           *fmalloc Q((const size_t len));
void           *frealloc Q((void *const old, const size_t len));
void tfree      P((void *const p));
void opnlog     P((const char *file));
void ssleep     P((const unsigned seconds));
void doumask    Q((const mode_t mask));
pid_t sfork     P((void));
int opena       P((const char *const a));
int ropen       Q((const char *const name, const mode, const mode_t mask));
int rpipe       P((int fd[2]));
int rdup        P((const int p));
int rclose      P((const int fd));
int rread       P((const int fd, void *const a, const int len));
int rwrite      P((const int fd, const void *const a, const int len));

extern mode_t   cumask;
