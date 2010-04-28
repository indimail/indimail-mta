#ifndef _BUFFER_H_
#define _BUFFER_H_

struct buffer_t
{
	int             fd;
	struct line_t  *l;
	unsigned long   cin, blen, ulen, clen;
	char           *b, restart, fn, eof, fbrk, *bp;
};

struct buffer_t *buffer_alloc(void);
struct buffer_t *buffer_init(int, unsigned long);
void            buffer_restart(struct buffer_t *, int);
void            buffer_kill(struct buffer_t *);
char           *buffer_next_line(struct buffer_t *);

#endif
