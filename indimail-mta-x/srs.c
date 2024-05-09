/*
 * $Log: srs.c,v $
 * Revision 1.6  2024-05-09 22:03:17+05:30  mbhangui
 * fix discarded-qualifier compiler warnings
 *
 * Revision 1.5  2023-10-02 21:25:29+05:30  Cprogrammer
 * fix srs_result len due to stralloc_0()
 *
 * Revision 1.4  2022-10-12 19:15:03+05:30  Cprogrammer
 * added feature to set SRS parameters using environment variables
 *
 * Revision 1.3  2022-10-12 16:26:26+05:30  Cprogrammer
 * added documentation
 * return -1 for control file open failure
 *
 * Revision 1.2  2021-06-12 19:57:46+05:30  Cprogrammer
 * removed chdir(auto_qmail)
 *
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
#include <env.h>
#include <str.h>
#include <stralloc.h>
#include "control.h"
#include "rcpthosts.h"
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

/*
 * returns  1 if ok
 * returns  0 if srs not configured
 * returns -1 system error
 * returns -2 out of memory
 * opens control files
 * srs_domain
 * srs_secrets
 * srs_maxage
 * srs_hashlength
 * srs_hashmin
 * srs_alwaysrewrite
 * srs_separator
 */
int
srs_setup(int with_rcpthosts)
{
	char           *x;

	if (setup_ok == 1)
		return 1;
	if (control_init() == -1)
		return -1;
	x = env_get("SRS_DOMAIN");
	if (control_readline(&srs_domain, x && *x ? x : "srs_domain") == -1)
		return -1;
	if (!srs_domain.len)
		return 0;
	if (!stralloc_0(&srs_domain))
		return -2;
	x = env_get("SRS_SECRETS");
	if ((srs_secrets_ok = control_readfile(&srs_secrets, x && *x ? x : "srs_secrets", 0)) == -1)
		return -1;
	if (!srs_secrets.len)
		return 0;
	x = env_get("SRS_MAXAGE");
	if (control_readint((int *) &srs_maxage, x && *x ? x : "srs_maxage") == -1)
		return -1;
	x = env_get("SRS_HASHLENGTH");
	if (control_readint((int *) &srs_hashlength, x && *x ? x : "srs_hashlength") == -1)
		return -1;
	x = env_get("SRS_HASHMIN");
	if (control_readint((int *) &srs_hashmin, x && *x ? x : "srs_hashmin") == -1)
		return -1;
	if (srs_hashmin > srs_hashlength)
		srs_hashmin = srs_hashlength;
	x = env_get("SRS_ALWAYSREWRITE");
	if (control_readint((int *) &srs_alwaysrewrite, x && *x ? x : "srs_alwaysrewrite") == -1)
		return -1;
	x = env_get("SRS_SEPARATOR");
	if (control_readline(&srs_separator, x && *x ? x : "srs_separator") == -1)
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
	if (!stralloc_copys(&srs_error, "SRS: ") ||
			!stralloc_cats(&srs_error, (char *) srs_strerror(code)) ||
			!stralloc_0(&srs_error))
		return -2;
	return -3;
}


int
srsforward(const char *address)
{
	int             x = 0;
	char            srsaddress[1000];
	srs_t          *srs;
	int             i = 0;
	int             j = 0;

	/*- Return if setup was unsucessfull */
	if ((x = srs_setup(1)) < 1)
		return (x); /*- return 0, -1 or -2 */
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
			return srs_error_str(x); /*- returns -2 or -3 */
	}
	for (j = 0; j < srs_secrets.len; ++j) {
		if (!srs_secrets.s[j]) {
			if ((x = srs_add_secret(srs, srs_secrets.s + i)) != SRS_SUCCESS)
				return srs_error_str(x); /*- returns -2 or -3 */
			i = j + 1;
		}
	}
	if ((x = srs_forward(srs, srsaddress, 1000, address, srs_domain.s)) != SRS_SUCCESS)
		return srs_error_str(x); /*- returns -2 or -3 */
	if (!stralloc_copys(&srs_result, srsaddress) ||
			!stralloc_0(&srs_result))
		return -2;
	srs_result.len--;
	srs_free(srs);
	return 1;
}

int
srsreverse(const char *srsaddress)
{
	int             x = 0;
	char            address[1000];
	srs_t          *srs;
	int             i = 0;
	int             j = 0;

	/*- Return if setup was unsucessfull */
	if ((x = srs_setup(0)) < 1)
		return (x); /*- return 0, -1 or -2 */
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
			return srs_error_str(x); /*- returns -2 or -3 */
	}
	for (j = 0; j < srs_secrets.len; ++j) {
		if (!srs_secrets.s[j]) {
			if ((x = srs_add_secret(srs, srs_secrets.s + i)) != SRS_SUCCESS)
				return srs_error_str(x); /*- returns -2 or -3 */
			i = j + 1;
		}
	}
	if ((x = srs_reverse(srs, address, 1000, srsaddress)) != SRS_SUCCESS)
		return srs_error_str(x); /*- returns -2 or -3 */
	if (!stralloc_copys(&srs_result, address) ||
			!stralloc_0(&srs_result))
		return -2;
	srs_result.len--;
	srs_free(srs);
	return 1;
}
#endif

void
getversion_srs_c()
{
	const char     *x = "$Id: srs.c,v 1.6 2024-05-09 22:03:17+05:30 mbhangui Exp mbhangui $";

	x++;
}
