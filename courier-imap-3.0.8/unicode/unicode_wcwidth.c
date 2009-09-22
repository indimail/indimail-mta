#include "unicode_config.h"
#include "unicode.h"

#include "eastasianwidth.h"

#include <stdlib.h>

int unicode_wcwidth(unicode_char c)
{
	size_t b=0;
	size_t e=sizeof(unicode_wcwidth_tab)/sizeof(unicode_wcwidth_tab[0]);

	while (b < e)
	{
		size_t n=b + (e-b)/2;

		if (c >= unicode_wcwidth_tab[n][0])
		{
			if (c <= unicode_wcwidth_tab[n][1])
				return 2;
			b=n+1;
		}
		else
		{
			e=n;
		}
	}
	return 1;
}
