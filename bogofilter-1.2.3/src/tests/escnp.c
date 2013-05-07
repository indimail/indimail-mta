#include <stdio.h>
#include <stdlib.h>

int main(void) {
    int c;
    while ((c = getchar()) != EOF) {
	if ((c < 0x20 || c > 0x7e) && c != '\n') {
	    printf("\\%03o", c);
	    continue;
	} else if (c == '\\') {
	    putchar('\\');
	}
	putchar(c);
    }
    exit(0);
}
