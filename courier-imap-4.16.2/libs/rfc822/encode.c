/*
** Copyright 2003-2004 Double Precision, Inc.  See COPYING for
** distribution information.
*/

/*
*/
#include	"encode.h"
#include	<string.h>
#include	<stdlib.h>

static int quoted_printable(struct libmail_encode_info *,
			    const char *, size_t);
static int base64(struct libmail_encode_info *,
		  const char *, size_t);
static int eflush(struct libmail_encode_info *,
		 const char *, size_t);

void libmail_encode_start(struct libmail_encode_info *info,
			  const char *transfer_encoding,
			  int (*callback_func)(const char *, size_t, void *),
			  void *callback_arg)
{
	info->output_buf_cnt=0;
	info->input_buf_cnt=0;

	switch (*transfer_encoding) {
	case 'q':
	case 'Q':
		info->encoding_func=quoted_printable;
		info->input_buffer[0]=0; /* Recycle for qp encoding */
		break;
	case 'b':
	case 'B':
		info->encoding_func=base64;
		break;
	default:
		info->encoding_func=eflush;
		break;
	}
	info->callback_func=callback_func;
	info->callback_arg=callback_arg;
}

int libmail_encode(struct libmail_encode_info *info,
		   const char *ptr,
		   size_t cnt)
{
	return ((*info->encoding_func)(info, ptr, cnt));
}

int libmail_encode_end(struct libmail_encode_info *info)
{
	int rc=(*info->encoding_func)(info, NULL, 0);

	if (rc == 0 && info->output_buf_cnt > 0)
	{
		rc= (*info->callback_func)(info->output_buffer,
					   info->output_buf_cnt,
					   info->callback_arg);
		info->output_buf_cnt=0;
	}

	return rc;
}

static int eflush(struct libmail_encode_info *info, const char *ptr, size_t n)
{
	while (n > 0)
	{
		size_t i;

		if (info->output_buf_cnt == sizeof(info->output_buffer))
		{
			int rc= (*info->callback_func)(info->output_buffer,
						       info->output_buf_cnt,
						       info->callback_arg);

			info->output_buf_cnt=0;
			if (rc)
				return rc;
		}

		i=n;

		if (i > sizeof(info->output_buffer) - info->output_buf_cnt)
			i=sizeof(info->output_buffer) - info->output_buf_cnt;

		memcpy(info->output_buffer + info->output_buf_cnt, ptr, i);
		info->output_buf_cnt += i;
		ptr += i;
		n -= i;
	}
	return 0;
}

static int base64_flush(struct libmail_encode_info *);

static int base64(struct libmail_encode_info *info,
		  const char *buf, size_t n)
{
	if (!buf)
	{
		int rc=0;

		if (info->input_buf_cnt > 0)
			rc=base64_flush(info);

		return rc;
	}

	while (n)
	{
		size_t	i;

		if (info->input_buf_cnt == sizeof(info->input_buffer))
		{
			int rc=base64_flush(info);

			if (rc != 0)
				return rc;
		}

		i=n;
		if (i > sizeof(info->input_buffer) - info->input_buf_cnt)
			i=sizeof(info->input_buffer) - info->input_buf_cnt;

		memcpy(info->input_buffer + info->input_buf_cnt,
		       buf, i);
		info->input_buf_cnt += i;
		buf += i;
		n -= i;
	}
	return 0;
}

static const char base64tab[]=
"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

static int base64_flush(struct libmail_encode_info *info)
{
	int	a=0,b=0,c=0;
	int	i, j;
	int	d, e, f, g;
	char	output_buf[ sizeof(info->input_buffer) / 3 * 4+1];

	for (j=i=0; i<info->input_buf_cnt; i += 3)
	{
		a=(unsigned char)info->input_buffer[i];
		b= i+1 < info->input_buf_cnt ?
			(unsigned char)info->input_buffer[i+1]:0;
		c= i+2 < info->input_buf_cnt ?
			(unsigned char)info->input_buffer[i+2]:0;

		d=base64tab[ a >> 2 ];
		e=base64tab[ ((a & 3 ) << 4) | (b >> 4)];
		f=base64tab[ ((b & 15) << 2) | (c >> 6)];
		g=base64tab[ c & 63 ];
		if (i + 1 >= info->input_buf_cnt)	f='=';
		if (i + 2 >= info->input_buf_cnt) g='=';
		output_buf[j++]=d;
		output_buf[j++]=e;
		output_buf[j++]=f;
		output_buf[j++]=g;
	}

	info->input_buf_cnt=0;

	output_buf[j++]='\n';
	return eflush(info, output_buf, j);
}

static const char xdigit[]="0123456789ABCDEF";

static int quoted_printable(struct libmail_encode_info *info,
			    const char *p, size_t n)
{
	char local_buf[256];
	int local_buf_cnt=0;

#define QPUT(c) do { if (local_buf_cnt == sizeof(local_buf)) \
                     { int rc=eflush(info, local_buf, local_buf_cnt); \
			local_buf_cnt=0; if (rc) return (rc); } \
			local_buf[local_buf_cnt]=(c); ++local_buf_cnt; } while(0)

	if (!p)
		return (0);

	while (n)
	{


		/*
		** Repurpose input_buffer[0] as a flag whether the previous
		** character was a space.
		**
		** A space before a newline gets escaped.
		*/

		if (info->input_buffer[0])
		{
			if (*p == '\n')
			{
				QPUT('=');
				QPUT('2');
				QPUT('0');
			}
			else
			{
				QPUT(' ');
			}
			++info->input_buf_cnt;
		}

		info->input_buffer[0]=0;

		if (*p == ' ')
		{
			info->input_buffer[0]=1;
			p++;
			--n;
			continue;
		}

		if (info->input_buf_cnt > 72 && *p != '\n')
		{
			QPUT('=');
			QPUT('\n');
			info->input_buf_cnt=0;
		}

		if ( *p == '\n')
			info->input_buf_cnt=0;
		else if (*p < ' ' || *p == '=' || *p >= 0x7F)
		{
			QPUT('=');
			QPUT(xdigit[ (*p >> 4) & 15]);
			QPUT(xdigit[ *p & 15 ]);
			info->input_buf_cnt += 3;
			p++;
			--n;
			continue;
		}
		else info->input_buf_cnt++;

		QPUT( *p);
		p++;
		--n;
	}

	if (local_buf_cnt > 0)
		return eflush(info, local_buf, local_buf_cnt);

	return 0;
}
