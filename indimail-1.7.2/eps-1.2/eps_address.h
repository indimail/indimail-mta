#ifndef _ADDRESS_H_
#define _ADDRESS_H_

struct address_t
{
	char           *name, *user, *domain;

	struct address_t *next;
};

struct group_t
{
	char           *group;
	unsigned long   nmembers;
	struct address_t *members;
};

struct group_t *address_evaluate(char *);
struct address_t *address_evaluate_one(char *);
void            address_kill(struct group_t *);
void            address_kill_one(struct address_t *);
void            address_fixup(struct address_t *);

#endif
