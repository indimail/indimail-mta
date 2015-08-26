/*
 * $Log: isPrime.c,v $
 * Revision 2.1  2015-08-26 11:07:33+05:30  Cprogrammer
 * function to check if number is a prime number
 *
 */

#include <math.h>
int 
IsPrime(unsigned int number)
{
	unsigned int	i, j;

	if (number <= 1)
		return (0); /* zero and one are not prime */
	j = (unsigned int) sqrt(number);
	for (i = 2; i <= j; i++) {
		if (number % i == 0)
			return (0);
	}
	return (1);
}

void
getversion_isPrime_c()
{
	printf("%s\n", sccsid);
	return;
}
