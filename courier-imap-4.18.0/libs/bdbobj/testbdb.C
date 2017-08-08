#include	"config.h"
#include	"bdbobj.h"
#include	<stdio.h>
#include	<string.h>

static void fw(char *p, size_t a, size_t b, FILE *f)
{
size_t i, j=a*b;

	for (i=0; i<j; i++)
		putc(p[i], f);
}

struct	kd {
	struct kd *next;
	char *key, *data;
	} ;

static int kdcmp (const void *a, const void *b)
{
	return (strcmp(  (*(const struct kd **)a)->key,
		(*(const struct kd **)b)->key));
}

int main(int argc, char **argv)
{
	if (argc < 2)	exit(1);

	if (argc < 3)
	{
	BDbObj	dbw;
	char	*key, *data;
	size_t	keylen, datalen;
	struct	kd *kdlist, **kdarray;
	size_t	kdcnt;

		if (dbw.Open(argv[1], "R"))
		{
			perror("open");
			exit(1);
		}

		printf("Dumping %s:\n", argv[1]);
		kdlist=0;
		kdcnt=0;
		for (key=dbw.FetchFirstKeyVal(keylen, data, datalen);
			key; key=dbw.FetchNextKeyVal(keylen, data, datalen))
		{
		struct kd *k=(struct kd *)malloc(sizeof(struct kd));

			if (!k)
			{
				perror("malloc");
				exit(1);
			}
			if ((k->key=(char *)malloc(keylen+1)) == 0 ||
				(k->data=(char *)malloc(datalen+1)) == 0)
			{
				perror("malloc");
				exit(1);
			}
			memcpy(k->key, key, keylen);
			k->key[keylen]=0;
			memcpy(k->data, data, datalen);
			k->data[datalen]=0;
			free(data);
			++kdcnt;
			k->next=kdlist;
			kdlist=k;
		}

		if ((kdarray=(struct kd **)
			malloc( (kdcnt+1)*sizeof(struct kd *))) == 0)
		{
			perror("malloc");
			exit(1);
		}
		kdcnt=0;
		while ( kdlist )
		{
			kdarray[kdcnt++]=kdlist;
			kdlist=kdlist->next;
		}
		kdarray[kdcnt]=0;

		qsort( kdarray, kdcnt, sizeof(kdarray[0]), &kdcmp);

		for (kdcnt=0; kdarray[kdcnt]; kdcnt++)
		{
			printf("Key: ");
			fw(kdarray[kdcnt]->key, strlen(kdarray[kdcnt]->key),
				1, stdout);
			printf(", Data: ");
			fw(kdarray[kdcnt]->data, strlen(kdarray[kdcnt]->data),
				1, stdout);
			printf("\n");
			free(kdarray[kdcnt]->key);
			free(kdarray[kdcnt]->data);
			free(kdarray[kdcnt]);
		}
		free(kdarray);

		dbw.Close();
	} else if (argc < 4 && argv[2][0] == '-')
	{
	BDbObj	dbw;

		if (dbw.Open(argv[1], "W"))
		{
			perror("open");
			exit(1);
		}

		printf("Deleting %s from %s:\n", argv[2], argv[1]);
		if (dbw.Delete(argv[2]+1, strlen(argv[2]+1)))
			fprintf(stderr, "Not found.\n");

		dbw.Close();
	} else if (argc < 4)
	{
	BDbObj	dbw;

		if (dbw.Open(argv[1], "R"))
		{
			perror("open");
			exit(1);
		}

	size_t len;
	char *val=dbw.Fetch(argv[2], strlen(argv[2]), len, 0);

		if (!val)
		{
			fprintf(stderr, "%s: not found.\n", argv[2]);
			exit(1);
		}
		printf("Fetching %s from %s: ", argv[2], argv[1]);
		fw(val, len, 1, stdout);
		printf("\n");
		free(val);
		dbw.Close();
	}
	else
	{
	BDbObj	dbw;

		if (dbw.Open(argv[1], "C"))
		{
			perror("open");
			exit(1);
		}

		printf("Storing %s/%s into %s:\n", argv[2], argv[3], argv[1]);
		if (dbw.Store(argv[2], strlen(argv[2]),
			argv[3], strlen(argv[3]), "R"))
		{
			perror("write");
			exit(1);
		}

		dbw.Close();
	}
	exit(0);
	return (0);
}
