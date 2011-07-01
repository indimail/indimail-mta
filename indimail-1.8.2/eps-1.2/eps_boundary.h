#ifndef _BOUNDARY_H_
#define _BOUNDARY_H_

struct member_t
{
	char           *boundary, depth;

	struct member_t *next;
};

struct boundary_t
{
	char            cdepth;
	struct member_t *boundaries, *last;
};

struct boundary_t *boundary_alloc(void);
struct member_t *member_alloc(void);
int             boundary_add(struct eps_t *, char *);
void            boundary_kill(struct boundary_t *);
void            member_kill(struct eps_t *, struct member_t *);
int             boundary_is(struct eps_t *, char *);
int             boundary_remove_last(struct eps_t *);
char           *boundary_fetch(struct eps_t *, char);
void            boundary_debug(struct eps_t *);

#endif
