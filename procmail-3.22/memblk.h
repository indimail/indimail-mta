typedef struct memblk {
	char           *p;			/* where it starts */
	long            len;		/* currently allocated size */
#ifdef USE_MMAP
	off_t           filelen;	/* how long is the file */
	int             fd;			/* file which is mmap()ed */
#endif
} memblk;

typedef char   *(read_func_type) P((char *, long, void *));
typedef int     (cleanup_func_type) P((memblk *, long *, long, void *));

void makeblock  P((memblk * const, const long));	/* create block of the given length */
void freeblock  P((memblk * const));	/* deallocate it */
void lockblock  P((memblk * const));	/* protect this block from future changes */
int resizeblock P((memblk * const, const long, const int));	/* change the size by this process */
char           *read2blk P((memblk * const, long *const, read_func_type *, cleanup_func_type *, void *));	/* dynamically grow a block to fit data as it comes in */

#ifdef USE_MMAP
extern int      ISprivate;		/* is themail a private copy or shared? */
#define isprivate	(themail.fd<0||ISprivate)
#define private(x)	(ISprivate=(x))
#else
#define isprivate	1
#define private(x)	1
#endif

extern memblk   themail;
