#ifndef _EPS_MISC_H_
#define _EPS_MISC_H_

#ifdef MEM_DEBUG
struct ptr_t
{
	void           *ptr;
	unsigned long   len;
	char           *where;

	struct ptr_t   *next;
};
#endif

unsigned char  *mstrdup(unsigned char *);
#ifdef MEM_DEBUG
int             mem_init(void);
int             mem_kill(void);
void            mem_chk(void *);
void           *mmalloc(unsigned long, unsigned char *);
void           *mrealloc(void *, unsigned long, unsigned char *);
int             mfree(void *);
void            merror(char *, char *);
int             mmemset(void *, unsigned long);
#else
#define mmalloc(a,b) malloc(a)
#define mfree(a) free(a)
#endif

#ifdef COUNT_DEBUG
void            time_init(void);
void            time_compare(void);
#endif

#endif
