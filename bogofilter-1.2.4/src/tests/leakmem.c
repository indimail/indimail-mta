/**
 * \file leakmem.c
 * deliberately leaks memory, to test memory checkers.
 * \author Matthias Andree
 * \date 2005
 *
 * GNU General Public License v2
 */

#include <stdlib.h>
#include <stdio.h>

int main(void) {
    char *x;

    x = malloc(42);
    printf("got %p\n", x);
    /* deliberately leak memory here */
    x = malloc(42);
    printf("got %p\n", x);
    free(x);
    return 0;
}
