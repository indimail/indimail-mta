/* $Id: wordhash.main.c 6797 2009-02-14 21:13:00Z relson $ */

#include "common.h"

#include "wordhash.h"

typedef struct
{
  int count;
}
wh_elt_t;

/* function definitions */

static void word_init(void *vw){
     wh_elt_t *w = vw;
     w->count = 0;   
}

static void
dump_hash (wordhash_t * h)
{
  hashnode_t *p;
  for (p = wordhash_first (h); p != NULL; p = wordhash_next (h))
    {
      word_t *key = p->key;
      (void)word_puts(key, 0, stdout);
      (void)printf (" %d\n", ((wh_elt_t *) p->data)->count);
    }
}

int
main (void)
{
  wordhash_t *h = wordhash_new ();
  char buf[100];
  wh_elt_t *w;

  while (scanf ("%99s", buf) != EOF)
    {
      word_t *t = word_news(buf);
      w = wordhash_insert (h, t, sizeof (word_t), &word_init);
      w->count++;
    }

  dump_hash (h);
  wordhash_free (h);
  return 0;
}
