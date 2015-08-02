/** \file abortme.c
 * just calls abort() to trigger signal 6, to test core dumps
 * and post-mortem backtraces
 * \author Matthias Andree
 * \date 2003
 * GNU GENERAL PUBLIC LICENSE, version 2.
 */

#include <stdlib.h>

int main(void)
{
    abort();
}
