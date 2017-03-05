#include "unicode_config.h"
#include "unicode.h"

#include "eastasianwidth.h"
#include "linebreaktab_internal.h"

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

	switch (unicode_lb_lookup(c)) {
	case UNICODE_LB_BK:
	case UNICODE_LB_CR:
	case UNICODE_LB_LF:
	case UNICODE_LB_CM:
	case UNICODE_LB_NL:
	case UNICODE_LB_WJ:
	case UNICODE_LB_ZW:
		return 0;
	default:
		break;
	}
	return 1;
}

int unicode_isspace(unicode_char ch)
{
	if (ch == 9)
		return 1;

	switch (unicode_lb_lookup(ch)) {
	case UNICODE_LB_BK:
	case UNICODE_LB_CR:
	case UNICODE_LB_LF:
	case UNICODE_LB_NL:
	case UNICODE_LB_SP:
		return 1;
	}

	return 0;
}

size_t unicode_wcwidth_str(const unicode_char *c)
{
	size_t w=0;

	while (*c)
		w += unicode_wcwidth(*c++);


	return w;
}
