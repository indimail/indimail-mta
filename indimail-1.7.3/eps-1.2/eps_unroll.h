#ifndef _UNROLL_H_
#define _UNROLL_H_

struct unroll_t
{
	char            eof;
	struct buffer_t *b;
	struct line_t  *l, *lb;
};

struct unroll_t *unroll_alloc(void);
struct unroll_t *unroll_init(int, unsigned long);
void            unroll_kill(struct unroll_t *);
void            unroll_restart(struct unroll_t *, int);
char           *unroll_next_line(struct unroll_t *);

#endif
