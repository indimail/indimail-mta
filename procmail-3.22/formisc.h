/*
 * $Id: formisc.h,v 1.10 1999/04/19 06:42:15 guenther Exp $
 */

void
loadsaved       P((const struct saved * const sp));
void loadbuf    Q((const char *const text, const size_t len));
void loadchar   P((const int c));
void elog       P((const char *const a));
void tputssn    Q((const char *a, size_t l));
void ltputssn   Q((const char *a, size_t l));
void lputcs     P((const int i));
void startprog  P((const char *Const * const argv));
void nofild     P((void));
void nlog       P((const char *const a));
void logqnl     P((const char *const a));
void closemine  P((void));
void opensink   P((void));
char           *skipwords P((char *start));
