#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include "query-getdns.h"

int             attempt_dane = 0;

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
		fprintf(stdout, "DNSSEC status of responses is bogus or indeterminate.\n");
		cleanup(1);
	}
	if (addresses == NULL) {
		fprintf(stdout, "No address records found, exiting.\n");
		cleanup(1);
	}
    if (auth_mode == MODE_DANE || auth_mode == MODE_BOTH) {
		if (tlsa_rdata_list == NULL) {
			fprintf(stdout, "No TLSA records found.\n");
			if (auth_mode == MODE_DANE)
				cleanup(1);
		} else
		if (tlsa_authenticated == 0) {
			fprintf(stdout, "Insecure TLSA records.\n");
			if (auth_mode == MODE_DANE)
				cleanup(1);
		} else
		if (v4_authenticated == 0 || v6_authenticated == 0) {
			fprintf(stdout, "Insecure Address records.\n");
			if (auth_mode == MODE_DANE)
				cleanup(1);
		} else
			attempt_dane = 1;
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

