#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include "query-getdns.h"

void
cleanup(int status)
{
	freeaddrinfo(addresses);
	free_tlsa(tlsa_rdata_list);
	exit (status);
}

int
main(int argc, char **argv)
{
    char *cp;
    tlsa_rdata *rp;

	if (argc != 2) {
		fprintf(stderr, "tlsarr mxhost\n");
		return (1);
	}
	do_dns_queries(argv[1], 25, 1);
	if (dns_bogus_or_indeterminate) {
		fprintf(stderr, "DNSSEC status of responses is bogus or indeterminate.\n");
		cleanup(1);
	}
	if (!addresses) {
		fprintf(stderr, "No address records found, exiting[%d].\n", tlsa_count);
		cleanup(0);
	}
	if (!tlsa_rdata_list) {
		fprintf(stderr, "No TLSA records found.\n");
		cleanup(0);
	} else
	if (!tlsa_authenticated) {
		fprintf(stderr, "Insecure TLSA records.\n");
		cleanup(0);
	} else
	if (!v4_authenticated || !v6_authenticated) {
		fprintf(stderr, "Insecure Address records.\n");
		cleanup(0);
	}
    if (tlsa_rdata_list) {
        fprintf(stdout, "TLSA records found: %ld\n", tlsa_count);
        for (rp = tlsa_rdata_list; rp != NULL; rp = rp->next) {
            fprintf(stdout, "TLSA: %d %d %d %s\n", rp->usage, rp->selector,
                    rp->mtype, (cp = bin2hexstring(rp->data, rp->data_len)));
            free(cp);
        }
    }
	cleanup(0);
	return 0;
}

