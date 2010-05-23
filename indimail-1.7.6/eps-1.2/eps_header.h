#ifndef _EPS_HEADER_H_
#define _EPS_HEADER_H_

struct atom_t
{
	unsigned char  *name, *data;
	struct atom_t  *next;
};

struct header_t
{
	struct atom_t  *atoms, *atail;
	unsigned char  *name, *data, *orig;
};

struct i_header_t
{
	unsigned char  *name;
	void            (*func) (struct eps_t *, struct header_t *, void *);
};

int             atom_begin(struct header_t *);
struct atom_t  *atom_new(struct header_t *);
struct atom_t  *atom_end(struct header_t *);
int             atom_kill(struct atom_t *);
struct header_t *header_alloc(void);
struct header_t *header_parse(unsigned char *);
struct atom_t  *header_fetch_atoms(struct header_t *h, unsigned char *);
int             header_parse_atom(struct header_t *, unsigned char *);
void            header_kill(struct header_t *);
void            header_show(struct header_t *);
unsigned char  *header_fetch_atom(struct header_t *, unsigned char *);

#endif
