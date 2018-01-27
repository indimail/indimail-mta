#include <stdio.h>
#include <unistd.h>
#include "eps.h"

int
roll_fd(int fd, char *data)
{
	int             ret = 0;
	unsigned long   n = 0;
	char           *h = NULL, *t = NULL;

	for (n = 0, h = data; ((*h) && (n < 78)); h++, n++);

	if (!(*h))
	{
		ret = write(fd, data, n);
		if (ret < n)
			return 0;

		ret = write(fd, "\n", 1);
		if (ret < 1)
			return 0;

		return 1;
	}

	/*
	 * ret = write(fd, data, n);
	 * if (ret < n)
	 * return 0;
	 * 
	 * ret = write(fd, "\n\t", 2);
	 * if (ret < 2)
	 * return 0;
	 * 
	 * n = 0;
	 */

	t = data;

	while (1)
	{
		for (; ((*h) && (n < 998)); h++, n++)
		{
			if (n >= 78)
			{
				if (rfc2822_is_wsp(*h))
					break;
			}
		}

		if (!(*h))
		{
			if (n)
			{
				ret = write(fd, t, n);
				if (ret < n)
					return 0;
			}

			ret = write(fd, "\n", 1);
			if (ret < 1)
				return 0;

			break;
		}

		ret = write(fd, t, n);
		if (ret < n)
			return 0;

		ret = write(fd, "\n\t", 2);
		if (ret < 2)
			return 0;

		n = 0;
		t = h;
	}

	return 1;
}
