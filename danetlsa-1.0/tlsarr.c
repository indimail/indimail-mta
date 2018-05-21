#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <getopt.h>
#include "query-getdns.h"

void
cleanup(int status)
{
	if (addresses) {
		freeaddrinfo(addresses);
		addresses = (addrinf *) 0;
	}
	if (tlsa_rdata_list) {
		free_tlsa(tlsa_rdata_list);
		tlsa_rdata_list = (tlsa_rdata *) 0;
	}
	if (mx_rdata_list) {
		free_mx(mx_rdata_list);
		mx_rdata_list = (mx_rdata *) 0;
	}
	exit (status);
}

void
usage()
{
	fprintf(stderr, "usage: tlsarr [-m] [-p port] host\n");
	fprintf(stderr, "         -m       query MX records for host\n");
	fprintf(stderr, "         -p port  port to connect to\n");
	fprintf(stderr, "         -s       stub mode\n");
	exit (1);
}

int
main(int argc, char **argv)
{
    char           *cp, *host, *mxhost;
    tlsa_rdata     *rp;
    mx_rdata       *mp;
	int             i, c, err, verbose = 0, recursive = 1, query_mx = 0, port = 25;

	while ((c = getopt(argc, argv, "svmp:")) != -1) {
		switch (c)
		{
		case 'v':
			verbose = 1;
			break;
		case 's':
			recursive = 0;
			break;
		case 'p':
			port = atoi(optarg);
			break;
		case 'm':
			query_mx = 1;
			break;
		default:
			usage();
		}
	}
	if (optind < argc)
		host = argv[optind++];
	else
		usage();
	if (query_mx) {
		if (verbose) {
			printf("doing MX query for %s\n", host);
			fflush(stdout);
		}
		if ((err = do_mx_queries(host, recursive)))  {
			print_stack("%s\n", get_tlsa_err_str(err));
			print_stack(0);
			return (err);
		}
		if (verbose) {
			printf("MX query finished\n");
			fflush(stdout);
		}
		if (mx_rdata_list) {
			fprintf(stdout, "MX records found: %ld\n", mx_count);
			fflush(stdout);
			for (i = 1, mp = mx_rdata_list; mp != NULL; mp = mp->next, i++) {
				if (!(cp = bin2txt(mp->data, mp->data_len))) {
					fprintf(stderr, "out of memory\n");
					continue;
				}
				fprintf(stdout, "MX[%d]: %d %s\n", i, mp->pref, cp);
				fflush(stdout);
				if (verbose) {
					printf("doing TLSA query for %s\n", cp);
					fflush(stdout);
				}
				if ((err = do_dns_queries(cp, port, recursive))) {
					if (!addresses) {
						fprintf(stderr, "No address records found\n");
						cleanup(1);
					}
					print_stack("%s\n", get_tlsa_err_str(err));
					print_stack(0);
					return (err);
				}
				free(cp);
				if (verbose) {
					printf("TLSA query finished\n");
					fflush(stdout);
				}
			}
			if (tlsa_rdata_list) {
				if (!tlsa_authenticated) {
					fprintf(stderr, "Insecure TLSA records.\n");
					cleanup(1);
				} else
				if (!v4_authenticated || !v6_authenticated) {
					fprintf(stderr, "Insecure Address records.\n");
					cleanup(1);
				}
				fprintf(stdout, "TLSA records found: %ld\n", tlsa_count);
				fflush(stdout);
				for (i = 1, rp = tlsa_rdata_list; rp != NULL; rp = rp->next, i++) {
					if (!(cp = bin2hexstring(rp->data, rp->data_len))) {
						fprintf(stderr, "out of memory\n");
						continue;
					}
					fprintf(stdout, "TLSA[%d][%s]: %d %d %d %s\n", i, rp->host, rp->usage, rp->selector, rp->mtype, cp);
					fflush(stdout);
					free(cp);
				}
			} else
				fprintf(stderr, "No TLSA records found.\n");
		}
		return (0);
	} else
		mxhost = host;
	if (verbose) {
		printf("doing TLSA query for %s\n", mxhost);
		fflush(stdout);
	}
	if ((err = do_dns_queries(mxhost, port, recursive))) {
		if (!addresses) {
			fprintf(stderr, "No address records found\n");
			cleanup(1);
		}
		print_stack("%s\n", get_tlsa_err_str(err));
		print_stack(0);
		return (err);
	} else
	if (tlsa_rdata_list) {
		if (!tlsa_authenticated) {
			fprintf(stderr, "Insecure TLSA records.\n");
			cleanup(1);
		} else
		if (!v4_authenticated || !v6_authenticated) {
			fprintf(stderr, "Insecure Address records.\n");
			cleanup(1);
		}
		fprintf(stdout, "TLSA records found: %ld\n", tlsa_count);
		fflush(stdout);
		for (i = 1, rp = tlsa_rdata_list; rp != NULL; rp = rp->next, i++) {
			if (!(cp = bin2hexstring(rp->data, rp->data_len))) {
				fprintf(stderr, "out of memory\n");
				continue;
			}
			fprintf(stdout, "TLSA[%d][%s]: %d %d %d %s\n", i, rp->host, rp->usage, rp->selector, rp->mtype, cp);
			fflush(stdout);
			free(cp);
		}
	} else
		fprintf(stderr, "No TLSA records found.\n");
	if (verbose) {
		printf("TLSA query finished\n");
		fflush(stdout);
	}
	cleanup(0);
	return 0;
}

