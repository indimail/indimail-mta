/* $Id: dehex.c 1578 2003-02-03 17:09:47Z relson $ */

#include <stdlib.h>
#include <stdio.h>

int main(void) {
    unsigned int a;
    while(scanf("%2x ", &a) == 1) {
	putchar((int)a);
    }
    exit(0);
}
