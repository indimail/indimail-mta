#include <stdio.h>
#include <stdlib.h>
int
main()
{
		double loadavg[3];
		int n;

		if ((n = getloadavg(loadavg, 3)) == -1)
			perror("getloadavg");
		else
			printf("%f\n", loadavg[0]);
}
