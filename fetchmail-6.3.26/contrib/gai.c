/*
 * File:   gai.c
 * Author: Matthias Andree
 *
 * Created on 3. Februar 2013, 15:03
 * A short file to call getaddrinfo with the same arguments as checkalias.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

/*
 *
 */
int main(int argc, char** argv) {
    struct addrinfo hints;
    struct addrinfo *res;

    if (argc != 2 || 0 == strcmp("-h", argv[1])) {
	fprintf(stderr, "Usage: %s hostname\n", argv[0]);
	exit(EXIT_FAILURE);
    }

    memset(&hints, 0, sizeof hints);
    hints.ai_family=AF_UNSPEC;
    hints.ai_protocol=PF_UNSPEC;
    hints.ai_socktype=SOCK_STREAM;
    hints.ai_flags=AI_CANONNAME;

    int result = getaddrinfo(argv[1], NULL, &hints, &res);
    if (result) {
        fprintf(stderr, "getaddrinfo(\"%s\", ...AI_CANONNAME...) failed: %d (%s)\n", argv[1], result, gai_strerror(result));
        exit(EXIT_FAILURE);
    }

    freeaddrinfo(res);
    return (EXIT_SUCCESS);
}

