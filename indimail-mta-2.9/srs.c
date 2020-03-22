/*
 * $Log: srs.c,v $
 * Revision 1.1  2013-08-23 15:34:22+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "hassrs.h"
#ifdef HAVESRS
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <srs2.h>
#include "auto_qmail.h"
#include "control.h"
#include "rcpthosts.h"
#include "str.h"
#include "stralloc.h"
#include "srs.h"

static stralloc srs_domain = { 0 };
static stralloc srs_secrets = { 0 };

static unsigned int srs_maxage = 0;
static unsigned int srs_hashlength = 0;
static unsigned int srs_hashmin = 0;
static unsigned int srs_alwaysrewrite = 0;
static stralloc srs_separator = { 0 };

stralloc        srs_result = { 0 };
stralloc        srs_error = { 0 };

static int      setup_ok = 0;
static int      srs_secrets_ok = 0;

static int
setup(int with_rcpthosts)
{

	if (setup_ok == 1)
		return 1;
	if (chdir(auto_qmail) == -1)
		return -1;
	if (control_init() == -1)
		return -1;
	if (control_readline(&srs_domain, "srs_domain") == -1)
		return -1;
	if (srs_domain.len) {
		if (!stralloc_0(&srs_domain))
			return -2;
	} else {
		return 0;
	}

	if ((srs_secrets_ok = control_readfile(&srs_secrets, "srs_secrets", 0)) == -1)
		return -1;
	if (control_readint((int *) &srs_maxage, "srs_maxage") == -1)
		return 0;
	if (control_readint((int *) &srs_hashlength, "srs_hashlength") == -1)
		return 0;
	if (control_readint((int *) &srs_hashmin, "srs_hashmin") == -1)
		return 0;
	if (srs_hashmin > srs_hashlength)
		srs_hashmin = srs_hashlength;
	if (control_readint((int *) &srs_alwaysrewrite, "srs_alwaysrewrite") == -1)
		return 0;
	if (control_readline(&srs_separator, "srs_separator") == -1)
		return -1;
	if (srs_separator.len && !stralloc_0(&srs_separator))
		return -2;
	if (srs_separator.len && srs_separator.s[0] != '-' && srs_separator.s[0] != '+' && srs_separator.s[0] != '=') {
		if (!stralloc_copys(&srs_separator, ""))
			return -2;
	}
	if (!srs_alwaysrewrite) {
		if (with_rcpthosts && rcpthosts_init() == -1)
			return -1;
	}
	setup_ok = 1;
	return 1;
}

static int
srs_error_str(int code)
{
	if (!stralloc_copys(&srs_error, "SRS: "))
		return -2;
	if (!stralloc_cats(&srs_error, (char *) srs_strerror(code)))
		return -2;
	if (!stralloc_0(&srs_error))
		return -2;
	return -3;
}


int
srsforward(char *address)
{
	int             x = 0;
	char            srsaddress[1000];
	srs_t          *srs;
	int             i = 0;
	int             j = 0;

	/*- Return if setup was unsucessfull */
	if ((x = setup(1)) < 1)
		return (x);
	/*- Return zero if null-sender */
	if ((x = str_len(address)) <= 1)
		return 0;
	/*- Return zero if local address */
	if (!srs_alwaysrewrite && rcpthosts(address, x, 0) == 1)
		return 0;
	/*- Now it's time to rewrite the envelope */
	srs = srs_new();
	if (srs_maxage > 0)
		srs->maxage = srs_maxage;
	if (srs_hashlength > 0)
		srs->hashlength = srs_hashlength;
	if (srs_hashmin > 0)
		srs->hashmin = srs_hashmin;
	if (srs_alwaysrewrite) {
		if ((x = srs_set_alwaysrewrite(srs, TRUE)) != SRS_SUCCESS)
			return srs_error_str(x);
	}
	if (srs_separator.len) {
		if ((x = srs_set_separator(srs, srs_separator.s[0])) != SRS_SUCCESS)
			return srs_error_str(x);
	}
	for (j = 0; j < srs_secrets.len; ++j) {
		if (!srs_secrets.s[j]) {
			if ((x = srs_add_secret(srs, srs_secrets.s + i)) != SRS_SUCCESS)
				return srs_error_str(x);
			i = j + 1;
		}
	}
	if ((x = srs_forward(srs, srsaddress, 1000, address, srs_domain.s)) != SRS_SUCCESS)
		return srs_error_str(x);
	if (!stralloc_copys(&srs_result, srsaddress))
		return -2;
	if (!stralloc_0(&srs_result))
		return -2;
	srs_free(srs);
	return 1;
}

int
srsreverse(char *srsaddress)
{
	int             x = 0;
	char            address[1000];
	srs_t          *srs;
	int             i = 0;
	int             j = 0;

	/*- Return if setup was unsucessfull */
	if ((x = setup(0)) < 1)
		return (x);

	/*- Return error if null-sender */
	if ((x = str_len(srsaddress)) <= 1)
		return -3;
	/*- Now it's time to rewrite the envelope */

	srs = srs_new();
	if (srs_maxage > 0)
		srs->maxage = srs_maxage;
	if (srs_hashlength > 0)
		srs->hashlength = srs_hashlength;
	if (srs_hashmin > 0)
		srs->hashmin = srs_hashmin;
	if (srs_separator.len) {
		if ((x = srs_set_separator(srs, srs_separator.s[0])) != SRS_SUCCESS)
			return srs_error_str(x);
	}
	for (j = 0; j < srs_secrets.len; ++j) {
		if (!srs_secrets.s[j]) {
			if ((x = srs_add_secret(srs, srs_secrets.s + i)) != SRS_SUCCESS)
				return srs_error_str(x);
			i = j + 1;
		}
	}
	if ((x = srs_reverse(srs, address, 1000, srsaddress)) != SRS_SUCCESS)
		return srs_error_str(x);
	if (!stralloc_copys(&srs_result, address))
		return -2;
	if (!stralloc_0(&srs_result))
		return -2;
	srs_free(srs);
	return 1;
}
#endif

void
getversion_srs_c()
{
	static char    *x = "$Id: srs.c,v 1.1 2013-08-23 15:34:22+05:30 Cprogrammer Stab mbhangui $";

	x++;
}
