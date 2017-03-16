#ifndef _LINE_H_
#define _LINE_H_

#define DEFAULT_BUFFER_SIZE 1000
#define DEFAULT_BUFFER_ADD  500

struct line_t
{
	char           *data;
	unsigned long   bytes, size;
};

struct line_t  *line_alloc(void);
int             line_init(struct line_t *, char *, unsigned long);
int             line_inject(struct line_t *, char *, unsigned long);
void            line_restart(struct line_t *);
void            line_kill(struct line_t *);
void            line_print(char *, unsigned long);

#endif
