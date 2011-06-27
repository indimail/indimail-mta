/*
 * $Id: variables.h,v 1.7 2001/08/27 08:44:03 guenther Exp $ 
 */

const char     *sputenv P((const char *const a));
const char     *eputenv P((const char *const src, char *const dst));
const char     *tgetenv P((const char *const a));
void primeStdout P((const char *const varname));
void retStdout  P((char *const newmyenv, const int fail, const int unset));
void retbStdout P((char *const newmyenv));
void appendlastvar P((const char *const value));
void cleanupenv P((int preserve));
void initdefenv Q((auth_identity * pass, const char *fallback, int do_presets));
void asenv      P((const char *const chp));
void setdef     P((const char *const name, const char *const value));
void setlastfolder P((const char *const folder));
void allocbuffers Q((size_t lineb, int setenv));
void setmaildir P((const char *const newdir));
void setoverflow P((void));
int asenvcpy    P((char *src));
int setexitcode P((int trapisset));
int alphanum    P((const unsigned c));
char           *gobenv P((char *chp, char *end));
long renvint    P((const long i, const char *const env));

extern long     Stdfilled;
extern char    *Stdout;
extern const char lastfolder[], maildir[], scomsat[], offvalue[];;
extern int      didchd;
