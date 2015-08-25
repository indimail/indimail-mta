/*
 * $Log: $
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
