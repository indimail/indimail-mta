#include <stdio.h>

int main(void)
{
    volatile double a=.0, b=.0, c=1.;
    printf("%g\n", a/b);
    printf("%g\n", c/b);
    return 0;
}
