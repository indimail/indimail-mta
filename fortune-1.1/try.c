#include <stdio.h>
#include <ctype.h>
main()
{
	int c, d, e;
	for(;;)
	{
		if ((c = getchar()) == EOF)
			break;
		if (c == '%')
		{
			if ((d = getchar()) == EOF)
			{
				putchar(c);
				break;
			}
			if (d == '\n')
			{
				putchar(d);
				if ((e = getchar()) == EOF)
				{
					putchar(c);
					putchar(d);
					break;
				}
				if (isdigit(e))
				{
					putchar(c);
					putchar(d);
					putchar(e);
				} else
				{
					putchar(d);
					putchar(e);
				}
			} else
				putchar(d);
		} else
			putchar(c);
	}
}
