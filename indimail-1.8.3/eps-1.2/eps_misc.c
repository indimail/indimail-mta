#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#ifdef COUNT_DEBUG
#include <sys/time.h>
#include <unistd.h>
#endif
#include "eps_misc.h"

#ifdef MEM_DEBUG
struct ptr_t   *ptr_list = NULL, *ptr_tail = NULL;
unsigned long   max_mem = 0, cur_mem = 0, nmalloc = 0, nfree = 0;
#endif

#ifdef COUNT_DEBUG
struct timeval  tv;
#endif

/*
 * Allocate memory, and copy a given string, then return
 * pointer.  If the string given is NULL, or blank, return
 * an empty string.
 */
unsigned char  *
mstrdup(unsigned char *str)
{
	int             len = 0;
	unsigned char  *r = NULL;

	if ((!str) || (!(*str)))
		len = 1;

	else
		len = strlen((const char *) str);

	r = (unsigned char *) mmalloc(len + 1, "mstrdup");
	if (r == NULL)
		return NULL;

	memset((unsigned char *) r, 0, (len + 1));

#ifdef MEM_DEBUG
	printf("mstrdup: Allocated %d byte(s) at %p\n", len + 1, r);
#endif

	/*
	 * Previously blank string
	 */
	if ((!str) || (!(*str)))
		return r;

	memcpy((unsigned char *) r, (unsigned char *) str, len);

	return r;
}

#ifdef MEM_DEBUG
int
mem_init(void)
{
	ptr_list = (struct ptr_t *) malloc(sizeof(struct ptr_t));
	if (ptr_list == NULL)
		return 0;

	ptr_list->next = NULL;
	ptr_tail = ptr_list;

	max_mem = 0;
	cur_mem = 0;
	nmalloc = 0;
	nfree = 0;

	return 1;
}

int
mem_kill(void)
{
	char           *pp = NULL;
	unsigned long   fixup = 0;
	struct ptr_t   *p = NULL, *op = NULL;

	fixup = 0;

	p = ptr_list;
	while (p->next)
	{
		op = p->next;
		p->next = p->next->next;

		printf("MEM_DEBUG: MEM_KILL: [%s(%p)] %lu byte(s)\n", op->where, op->ptr, op->len);

		pp = (char *) op->ptr;
		if (pp)
		{
			if ((*pp >= 32) && (*pp <= 126))
				printf("     DATA:[%s]\n", pp);
		}

		cur_mem -= op->len;
		fixup += op->len;

		free(op->ptr);
		free(op);
	}

	free(ptr_list);
	ptr_list = NULL;

	printf("MEM_DEBUG: Cleaned up: %lu byte(s)\n", fixup);
	printf("MEM_DEBUG: Current memory allocated: %lu byte(s)\n", cur_mem);
	printf("MEM_DEBUG: Maximum memory allocated: %lu byte(s)\n", max_mem);
	printf("MEM_DEBUG: %lu allocation(s) and %lu deallocation(s)\n", nmalloc, nfree);

	return 1;
}

void
mem_chk(void *p)
{
	struct ptr_t   *ptr = NULL;

	for (ptr = ptr_list; ptr->next; ptr = ptr->next)
	{
		if (ptr->next->ptr == p)
			merror("mem_chk", "Duplicate allocation address");

		if ((p > ptr->next->ptr) && (p <= (ptr->next->ptr + ptr->next->len)))
			merror("mem_chk", "Shared allocation space");
	}
}

void           *
mmalloc(unsigned long len, unsigned char *where)
{
	void           *p = NULL;
	struct ptr_t   *ptr = NULL;

	ptr = (struct ptr_t *) malloc(sizeof(struct ptr_t));
	if (ptr)
	{
		p = (void *) malloc(len);
		if (p)
		{
			mem_chk(p);
			memset((void *) p, 0, len);
		} else
			merror("mmalloc", "Unable to allocate memory (data)");

		ptr->ptr = p;
		ptr->where = where;
		ptr->len = len;
		ptr->next = NULL;

		ptr_tail->next = ptr;
		ptr_tail = ptr;

		cur_mem += len;
		nmalloc++;

		printf("MEM_DEBUG: Allocated %lu byte(s) for %s at %p\n", len, where, p);

		if (cur_mem > max_mem)
			max_mem = cur_mem;
	}

	else
		merror("mmalloc", "Unable to allocate memory (struct)");

	return p;
}

void           *
mrealloc(void *rptr, unsigned long len, unsigned char *where)
{
	void           *p = NULL;

	struct ptr_t   *ptr = NULL;

	p = (void *) realloc(rptr, len);
	if (!p)
		merror("mrealloc", "Unable to allocate memory");

	for (ptr = ptr_list; ptr->next; ptr = ptr->next)
	{
		if (ptr->next->ptr == rptr)
		{
			cur_mem -= ptr->next->len;
			cur_mem += len;

			ptr->next->len = len;
			ptr->next->ptr = p;
			nmalloc++;

			if (cur_mem > max_mem)
				max_mem = cur_mem;

			return p;
		}
	}

	printf("MEM_DEBUG: Didnt find memory pointer for realloc!\n");
	return p;
}

int
mfree(void *p)
{
	int             i = 0;
	struct ptr_t   *pp = NULL, *op = NULL;

	i = 0;
	pp = ptr_list;

	while (pp->next)
	{
		i++;

		if (pp->next->ptr == p)
		{
			op = pp->next;

			if (pp->next->next)
				pp->next = pp->next->next;
			else
			{
				pp->next = NULL;
				ptr_tail = pp;
			}

			nfree++;
			cur_mem -= op->len;

			printf("MEM_DEBUG: Deallocated %lu from %s\n", op->len, op->where);

			free(op->ptr);
			free(op);

			return 1;
		} else
			pp = pp->next;
	}

	printf("MEM_DEBUG: MFREE: Unknown allocation space at %p (Searched %d records)\n", p, i);
	printf("MEM_DEBUG: MFREE: DATA FOLLOWS:\n");
	printf("--->\n%s\n<---\n", (char *) p);

	exit(0);
	return 0;
}

void
merror(char *w, char *e)
{
	printf("%s: %s\n", w, e);
	exit(0);
}
#endif

#ifdef COUNT_DEBUG
/*
 * Initialize the current time
 */
void
time_init(void)
{
	int             ret = 0;

	memset((struct timeval *) &tv, 0, sizeof(struct timeval));

	ret = gettimeofday(&tv, 0);
	if (ret == -1)
		printf("TIME_INIT: gettimeofday failed\n");
}

/*
 * Compare original time with current to
 * see how long processing took.
 */
void
time_compare(void)
{
	int             ret = 0;
	struct timeval  ltv;
	unsigned long   sec = 0, usec = 0;

	memset((struct timeval *) &ltv, 0, sizeof(struct timeval));

	ret = gettimeofday(&ltv, 0);
	if (ret == -1)
	{
		printf("TIME_COMPARE: gettimeofday failed\n");
		return;
	}

	sec = ltv.tv_sec - tv.tv_sec;
	usec = ltv.tv_usec - tv.tv_usec;

	printf("TIME_COMPARE: Processed in %lu.%lu second(s)\n", sec, usec);
}
#endif
