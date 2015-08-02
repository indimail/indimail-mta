/* $Id: deb64.c 6766 2009-01-12 04:27:36Z relson $ */

#include "common.h"

#include <stdlib.h>
#include <string.h>
#include <unistd.h>		/* for SEEK_*** for SunOS 4.1.x */

#include "base64.h"

static void die(void) __attribute__((noreturn));
static void die(void) {
    perror("debase64");
    exit(EXIT_FAILURE);
}

int main(void) {
    size_t size;
    word_t *w;

    if (fseek(stdin, 0, SEEK_END)) die();
    size = ftell(stdin);
    if (fseek(stdin, 0, SEEK_SET)) die();
    w = word_new(NULL, size);
    if (fread(w->u.text, 1, w->leng, stdin) != w->leng) die();
    size = base64_decode(w);
    if (fwrite(w->u.text, 1, size, stdout) != size) die();
    word_free(w);
    if (fflush(stdout)) die();
    if (fclose(stdout)) die();
    return EXIT_SUCCESS;
}
