/*
 * $Id: cstdio.h,v 1.15 2000/09/28 01:23:16 guenther Exp $
 */

void pushrc     P((const char *const name));
void changerc   P((const char *const name));
void duprcs     P((void));
void closerc    P((void));
void ungetb     P((const int x));
void skipline   P((void));
int poprc       P((void));
int bopen       P((const char *const name));
int getbl       P((char *p, char *end));
int getb        P((void));
int testB       P((const int x));
int sgetc       P((void));
int skipspace   P((void));
int getlline    P((char *target, char *end));

extern struct dynstring *incnamed;

#ifdef LMTP
/*
 * extensions for LMTP 
 */
void pushfd     P((int fd));
int endoread    P((void));
int getL        P((void));
int readL       P((char *, const int));
int readLe      P((char *, int));
#endif
