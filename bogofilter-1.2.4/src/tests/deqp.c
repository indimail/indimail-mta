/* $Id: deqp.c 6766 2009-01-12 04:27:36Z relson $ */

#include "common.h"

#include <stdlib.h>
#include <string.h>
#include <unistd.h>		/* for SEEK_*** for SunOS 4.1.x */

#include "qp.h"

static void die(void) __attribute__((noreturn));
static void die(void) {
    perror("deqp");
    exit(EXIT_FAILURE);
}

int main(int argc, char **argv) {
    size_t size;
    qp_mode mode = RFC2045;
    word_t *w;

    if (argc > 1 && strcasecmp(argv[1], "rfc2047")) mode = RFC2047;
    if (argc > 1 && strcasecmp(argv[1], "rfc-2047")) mode = RFC2047;

    if (fseek(stdin, 0, SEEK_END)) die();
    size = ftell(stdin);
    if (fseek(stdin, 0, SEEK_SET)) die();
    w = word_new(NULL, size);
    if (fread(w->u.text, 1, w->leng, stdin) != w->leng) die();
    size = qp_decode(w, mode);
    if (fwrite(w->u.text, 1, size, stdout) != size) die();
    word_free(w);
    if (fflush(stdout)) die();
    if (fclose(stdout)) die();
    return EXIT_SUCCESS;
}
