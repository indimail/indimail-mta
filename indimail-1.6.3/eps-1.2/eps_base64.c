#include <stdio.h>
#include <unistd.h>
#include "eps.h"

unsigned char   alphabet[64] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

void
base64_init(struct base64_t *b)
{
	int             i = 0;

	for (i = (sizeof alphabet) - 1; i >= 0; i--)
	{
		b->inalphabet[alphabet[i]] = 1;
		b->decoder[alphabet[i]] = i;
	}
}

int
base64_decode(struct base64_t *b, struct line_t *l, char *data)
{
	unsigned char  *p = NULL, buf[3] = { 0 };
	int             bits = 0, char_count = 0, errors = 0, c = 0;

	p = (unsigned char *) data;
	char_count = bits = 0;

	while (*p)
	{
		if (*p == '=')
			break;
		if (!b->inalphabet[*p])
		{
			p++;
			continue;
		}

		bits += b->decoder[*p];
		char_count++;

		if (char_count == 4)
		{
			buf[0] = (bits >> 16);
			buf[1] = ((bits >> 8) & 0xff);
			buf[2] = (bits & 0xff);
			line_inject(l, (char *) buf, 3);
			bits = 0;
			char_count = 0;
		}

		else
			bits <<= 6;

		p++;
	}

	if (!(*p))
	{
		if (char_count)
		{
#ifdef DECODE_DEBUG
			fprintf(stderr, "base64 encoding incomplete: at least %d bits truncated", ((4 - char_count) * 6));
#endif
			errors++;
		}
	}

	else
	{
		switch (char_count)
		{
		case 1:
#ifdef DECODE_DEBUG
			fprintf(stderr, "base64 encoding incomplete: at least 2 bits missing");
#endif
			errors++;
			break;
		case 2:
			c = (bits >> 10);
			line_inject(l, (char *) &c, 1);
			break;
		case 3:
			buf[0] = (bits >> 16);
			buf[1] = ((bits >> 8) & 0xff);

			line_inject(l, (char *) buf, 2);
			break;
		}
	}

	if (errors)
		return 0;

	return 1;
}

int
base64_encode(int fd, struct line_t *l)
{
	unsigned long   bytes = 0;
	unsigned char  *p = NULL, buf[4] = { 0 };
	int             cols = 0, bits = 0, c = 0, char_count = 0, ret = 0;

	bytes = 0;
	p = (unsigned char *) l->data;
	bits = cols = char_count = c = 0;

	while (bytes < l->bytes)
	{
		bits += *p;
		char_count++;

		if (char_count == 3)
		{
			buf[0] = alphabet[bits >> 18];
			buf[1] = alphabet[(bits >> 12) & 0x3f];
			buf[2] = alphabet[(bits >> 6) & 0x3f];
			buf[3] = alphabet[bits & 0x3f];

			ret = write(fd, buf, 4);
			if (ret < 4)
				return 0;

			cols += 4;

			if (cols == 72)
			{
				c = '\n';

				ret = write(fd, &c, 1);
				if (ret < 1)
					return 0;

				cols = 0;
			}

			bits = 0;
			char_count = 0;
		}

		else
			bits <<= 8;

		p++;
		bytes++;
	}

	if (char_count != 0)
	{
		bits <<= (16 - (8 * char_count));

		buf[0] = alphabet[bits >> 18];
		buf[1] = alphabet[(bits >> 12) & 0x3f];

		ret = write(fd, buf, 2);
		if (ret < 2)
			return 0;

		if (char_count == 1)
		{
			buf[0] = '=';
			buf[1] = '=';

			ret = write(fd, buf, 2);
			if (ret < 2)
				return 0;
		}

		else
		{
			buf[0] = alphabet[(bits >> 6) & 0x3f];
			buf[1] = '=';

			ret = write(fd, buf, 2);
			if (ret < 2)
				return 0;
		}

		if (cols > 0)
		{
			c = '\n';

			ret = write(fd, &c, 1);
			if (ret < 1)
				return 0;
		}
	}

	return 1;
}
