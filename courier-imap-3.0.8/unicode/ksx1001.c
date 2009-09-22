/*
 * ISO-2022-KR, EUC-KR & CP949 <=> Unicode translate functions.
 *   by Hatuka*nezumi - IKEDA Soji <nezumi@jca.apc.org>
 */

#include <stdio.h>
#include <string.h>
#include "unicode.h"
#include "ksx1001.h"

#define	EUCKR_CP949_EXTENSION	1

/*
 * ISO-2022-KR (RFC1557) Converters
 */

struct kschar_t {
	int	state;
	unsigned int	value;
};

static size_t read_char(const char* src, struct kschar_t *ch)
{
unsigned int hi, lo;

	switch (src[0]) {
	case KS_CHAR_SI:
		/* Shift-in */
		ch->state = KS_STATE_ASCII;
		ch->value = 0;
		return 1;
	case KS_CHAR_SO:
		/* Shift-out */
		ch->state = KS_STATE_KSX1001;
		ch->value = 0;
		return 1;
	case KS_CHAR_ESC:
		/* Announcer sequence */
		if (src[1] == '$' && src[2] == ')' && src[3] == 'C') {
			ch->value = 0;
			return 4;
		}
		/* ESC character */
		else
		{
			ch->state = KS_STATE_BINARY;
			ch->value = KS_CHAR_ESC;
			return 1;
		}
	}

	/* Control Characters */
	if ((unsigned char)src[0] < 0x20)
	{
		/* state will not be changed. */
		ch->value = (unsigned int)src[0];
	}
	/* US-ASCII */
	if ((ch->state == KS_STATE_ASCII || ch->state == KS_STATE_BINARY)
	    && (unsigned char)src[0] < 0x80)
	{
		ch->state = KS_STATE_ASCII;
		ch->value = (unsigned int)src[0];
		return 1;
	}
	/* KS X 1001 */
	else if (ch->state == KS_STATE_KSX1001
	    && 0x21 <= src[0] && src[0] <= 0x7E
	    && 0x21 <= src[1] && src[1] <= 0x7E)
	{
		hi = (unsigned int)src[0];
		lo = (unsigned int)src[1];
		if (cp949_to_uni_tbls[hi-1] != NULL
		    && cp949_to_uni_tbls[hi-1][lo+0x3F] != 0xFFFD)
		{
			ch->value = hi * 256 + lo;
			return 2;
		}
		else
		{
			ch->value = 0x003F;
			return 2;
		}
	}
	else
	{
		ch->state = KS_STATE_BINARY;
		ch->value = 0x003F;
		return 1;
	}
}

static unicode_char c2u_iso2022kr_convchar(unsigned int c, int state)
{
	unsigned int hi = (c >> 8);
	unsigned int lo = c & 0x00FF;

	/* Control characters */
	if (c < (unsigned int)0x0020)
		return (unicode_char)c;
	/* US-ASCII */
	else if (state == KS_STATE_ASCII && c < (unsigned int)0x0080)
		return (unicode_char)c;
	/* KS X 1001 */
	else if (state == KS_STATE_KSX1001 && c != 0x003F
	    && cp949_to_uni_tbls[hi-1] != NULL
	    && cp949_to_uni_tbls[hi-1][lo+0x3F] != 0xFFFD)
		return cp949_to_uni_tbls[hi-1][lo+0x3F];
	/* Uniknown */
	else
		return (unicode_char)0xFFFD;
}

static unicode_char *c2u_iso2022kr(const struct unicode_info *u,
				const char *ks_str, int *err)
{
size_t i, cnt, w;
unicode_char *uc;
struct kschar_t ch;

	if (err)
		*err = -1;

	/* Count the number of potential unicode characters first. */
	i = cnt = 0;
	ch.state = KS_STATE_ASCII;
	ch.value = 0;
	while (ks_str[i]) {
	i += read_char(ks_str+i, &ch);
	if (ch.value)
		++cnt;
	}

	uc = malloc((cnt+1) * sizeof(unicode_char));
	if (!uc)
		return NULL;

	i = cnt = 0;
	ch.state = KS_STATE_ASCII;
	ch.value = 0;
	while (ks_str[i]) {
		w = read_char(ks_str+i, &ch);
		if (ch.value) {
			uc[cnt] = c2u_iso2022kr_convchar(ch.value, ch.state);
			if (uc[cnt] == (unicode_char)0xFFFD && err) {
				*err = i;
				free(uc);
				return NULL;
			}
			++cnt;
		}
		i+=w;
	}
	uc[cnt] = 0;

	return uc;
}

static void revlookup(unicode_char u, struct kschar_t *ch)
{
unsigned int hi = u >> 8;
unsigned int lo = u & 0x00ff;
unsigned int k;
unsigned char c1, c2;

	/* ISO-2022-KR is mapped inside BMP range. */
	if (u >= (unicode_char)0x10000)
	{
		ch->state = KS_STATE_BINARY;
		ch->value = 0x003F;
		return;
	}

	/* US-ASCII */
	if (u < (unicode_char)0x0080)
	{
		ch->state = KS_STATE_ASCII;
		ch->value = (unsigned int)u;
		return;
	}

	/* For compatibility: 2 Characters replaced by KS X 1003 */
	if (u == (unicode_char)0x20A9) /* WON SIGN */
	{
		ch->state = KS_STATE_ASCII;
		ch->value = 0x5C;
		return;
	}
	if (u == (unicode_char)0x203E) /* OVERLINE */
	{
		ch->state = KS_STATE_ASCII;
		ch->value = 0x7E;
		return;
	}

	/* KS X 1001 */
	if (uni_to_ksx1001_tbls[hi] != NULL
	    && (k = uni_to_ksx1001_tbls[hi][lo]) != 0x003F)
	{
		c1 = (k >> 8);
		c2 = (k & 0x00FF);
		if (c1 >= (unsigned char)0xA1 && c2 >= (unsigned char)0xA1)
		{
			c1 -= 0x80;
			c2 -= 0x80;
			ch->state = KS_STATE_KSX1001;
			ch->value = c1*256 + c2;
			return;
		}
		else
		{
			ch->state = KS_STATE_BINARY;
			ch->value = 0x003F;
			return;	
		}
	}

	/* Otherwise, return 'unknown' characters */
	ch->state = KS_STATE_BINARY;
	ch->value = 0x003F;
	return;
}

static char *u2c_iso2022kr(const struct unicode_info *u,
		const unicode_char *str, int *err)
{
size_t i, cnt;
int k;
int kstate = KS_STATE_ASCII;
int ks;
int has_ksx1001=0;
char *s;
struct kschar_t ch;

	if (err)
		*err = -1;

	/* Count the number of potential octets first. */
	ch.state = KS_STATE_ASCII;
	ch.value = 0;
	kstate = KS_STATE_ASCII;
	has_ksx1001 = 0;
	for (i = cnt = 0; str[i]; i++) {
		revlookup(str[i], &ch);
		ks = ch.state;
		k = ch.value;
		if (ks != kstate)
		{
			cnt++;
			kstate = ks;
		}
		if (k)
			cnt += ((kstate == KS_STATE_KSX1001)? 2: 1);
		if (kstate == KS_STATE_KSX1001)
			has_ksx1001 = 1;
	}
	if (kstate != KS_STATE_ASCII && kstate != KS_STATE_BINARY)
		cnt++;
	if (has_ksx1001)
		cnt+=4;

	s = malloc(cnt+1);
	if (!s)
		return NULL;

	cnt = 0;
	if (has_ksx1001)
	{
		s[cnt++] = KS_CHAR_ESC;
		s[cnt++] = '$';
		s[cnt++] = ')';
		s[cnt++] = 'C';
	}
	ch.state = KS_STATE_ASCII;
	ch.value = 0;
	kstate = KS_STATE_ASCII;
	for (i = 0; str[i]; i++)
	{
		revlookup(str[i], &ch);
		ks = ch.state;
		k = ch.value;
		if (ks != kstate)
		{
			switch (ks)
			{
			case KS_STATE_KSX1001:
				s[cnt++] = KS_CHAR_SO;
				break;
			default:
				s[cnt++] = KS_CHAR_SI;
				break;
			}
			kstate = ks;
		}
		switch (kstate)
		{
		case KS_STATE_KSX1001:
			s[cnt++] = (char)(k >> 8);
			s[cnt++] = (char)(k & 0x00FF);
			break;
		default:
			s[cnt++] = (char)k;
		}

		if (kstate == KS_STATE_BINARY && k == 0x003F)
			if (err)
			{
				*err = i;
				free(s);
				return NULL;
			}
	}
	if (kstate != KS_STATE_ASCII && kstate != KS_STATE_BINARY)
	{
		s[cnt++] = KS_CHAR_SI;
	}
	s[cnt] = 0;

	return s;
}


/*
 * EUC-KR / CP949 (UHC) Converters
 */

static unicode_char *c2u_euckr_doconv(const struct unicode_info *u,
				const char *euckr_str, int *err,
				int compat)
{
	unicode_char *uc=0;
	unicode_char c;
	unsigned char hi=0, lo=0;
	int len=0;
	int i=0;
	int pos=0;

	if(err) *err = -1;

	len = strlen(euckr_str);
	uc = (unicode_char*)malloc((len+1) * sizeof(unicode_char) *2);

	if (!uc)
		return NULL;

	for(i=0; i<len;) {
		/* 2 Characters replaced by KS X 1003 */
		if ((compat & EUCKR_CP949_EXTENSION)
		    && euckr_str[i] == 0x5C) /* WON SIGN */
		{
			uc[pos++] = (unicode_char)0x20A9;
			i++;
		}
		else if ((compat & EUCKR_CP949_EXTENSION)
		    && euckr_str[i] == 0x7E) /* OVERLINE */
		{
			uc[pos++] = (unicode_char)0x203E;
			i++;
		}
		/* US-ASCII or KS X 1003 */
		else if((unsigned char)euckr_str[i] < 0x80)
		{
			uc[pos++] = (unicode_char)(euckr_str[i]);
			i++;
		}
		/* KS X 1001 */
		else if ((unsigned char)euckr_str[i] >= 0xa1
		    && (unsigned char)euckr_str[i+1] >= 0xa1)
		{
			hi = (unsigned char)euckr_str[i];
			lo = (unsigned char)euckr_str[i+1];
			
			if (cp949_to_uni_tbls[hi-0x81] == NULL)
				c = (unicode_char)0xFFFD;
			else
				c = cp949_to_uni_tbls[hi-0x81][lo-0x41];

			uc[pos++] = c;
			if (c == (unicode_char)0xFFFD && err)
			{
				*err = i;
				free(uc);
				return NULL;
			}

			i+=2;
		}
		/* CP949 extension */
		else if ((0x81 <= (unsigned)euckr_str[i]
		    && (unsigned)euckr_str[i] <= 0xFE)
		    && ((0x41 <= (unsigned)euckr_str[i+1]
		    && (unsigned)euckr_str[i+1] <= 0x5A)
		    || (0x61 <= (unsigned)euckr_str[i+1]
		    && (unsigned)euckr_str[i+1] <= 0x7A)
		    || (0x81 <= (unsigned)euckr_str[i+1]
		    && (unsigned)euckr_str[i+1] <= 0xFE)))
		{
			hi = (unsigned char)euckr_str[i];
			lo = (unsigned char)euckr_str[i+1];	

			if (!(compat & EUCKR_CP949_EXTENSION))
				c = 0xFFFD;
			else if (cp949_to_uni_tbls[hi-0x81] != NULL)
				c = cp949_to_uni_tbls[hi-0x81][lo-0x41];
			else
				c = 0xFFFD;

			uc[pos++] = c;
			if (c == 0xFFFD && err)
				*err = i;
				free(uc);
				return NULL;
			i+=2;
		}
		/* Not found */
		else if (err)
		{
			*err = i;
			free(uc);
			return NULL;
		}
		else
		{
			uc[pos++] = (unicode_char)0xFFFD;
			i++;
		}
	}
	uc[pos++] = 0;

 	return uc;
}

static unicode_char *c2u_euckr(const struct unicode_info *u,
				const char *euckr_str, int *err)
{
	return c2u_euckr_doconv(u, euckr_str, err, 0);
}

static unicode_char *c2u_cp949(const struct unicode_info *u,
				const char *euckr_str, int *err)
{
	return c2u_euckr_doconv(u, euckr_str, err, EUCKR_CP949_EXTENSION);
}


static char *u2c_euckr_doconv(const struct unicode_info *u,
			const unicode_char *str, int *err,
			int compat)
{
	int i=0;
	int pos=0;
	int len=0;
	char* s;
	
	if(err) *err = -1;
	
	while(str[len])
		len++;
	s = malloc((len+1)*2);

	if (!s)
		return NULL;

	for(i=0; str[i]; i++)
	{
		int ksx_char = 0;
		unsigned char hi=0, lo=0;

		unsigned char str_i_high=str[i] >> 8;

		/* EUC-KR is mapped inside BMP range. */
		if (str[i] >= (unicode_char)0x10000)
		{
			if (err)
			{
				*err = i;
				free(s);
				return NULL;
			}
			s[pos++] = '?';
		}
		/* US-ASCII */
		else if (str[i] < (unicode_char)0x0080)
			s[pos++] = (char)str[i];
		/* For compatibility: 2 characters replaced by KS X 1003 */
		else if (str[i] == (unicode_char)0x20A9) /* WON SIGN */
			s[pos++] = 0x5C;
		else if (str[i] == (unicode_char)0x203E) /* OVERLINE */
			s[pos++] = 0x7E;
		/* KS X 1001 */
		else if (uni_to_ksx1001_tbls[str_i_high] != NULL)
		{
			ksx_char = uni_to_ksx1001_tbls[str_i_high][str[i] & 0xff];
			hi = ksx_char >> 8;
			lo = ksx_char & 0xff;

			if (hi)
			{
				s[pos++] = hi;
				s[pos++] = lo;
			}
			else
			{
				ksx_char = 0x003F;
				s[pos++] = '?';
			}

			if (ksx_char == 0x003F && err)
			{
				*err = i;
				free(s);
				return NULL;
			}
		}
		/* CP949 Extension */
		else if (uni_to_cp949_tbls[str_i_high] != NULL)
		{

			if (!(compat & EUCKR_CP949_EXTENSION))
				ksx_char = 0x003F;
			else
				ksx_char = uni_to_cp949_tbls[str_i_high][str[i] & 0xff];
			hi = ksx_char >> 8;
			lo = ksx_char & 0xff;

			if (hi)
			{
				s[pos++] = hi;
				s[pos++] = lo;
			}
			else
			{
				ksx_char = 0x003F;
				s[pos++] = '?';
			}

			if (ksx_char == 0x003F && err)
			{
				*err = i;
				free(s);
				return NULL;
			}
		}
		/* Not found */
		else if (err)
		{
			*err = i;
			free(s);
			return NULL;
		}
		else
			s[pos++] = '?';
	}
	s[pos] = 0;
    
	return s;
}

static char *u2c_euckr(const struct unicode_info *u,
			const unicode_char *str, int *err)
{
	return u2c_euckr_doconv(u, str, err, 0);
}

static char *u2c_cp949(const struct unicode_info *u,
			const unicode_char *str, int *err)
{
	return u2c_euckr_doconv(u, str, err, EUCKR_CP949_EXTENSION);
}


static char *toupper_func(const struct unicode_info *u,
			  const char *cp, int *ip)
{
  unicode_char *uc = (*u->c2u)(u, cp, ip);
  char *s;
  size_t i;

  if (!uc)
    return (NULL);

  for (i=0; uc[i] && i<10000; i++) {
    if ((unicode_char)'a' <= uc[i] && uc[i] <= (unicode_char)'z')
      uc[i] = uc[i] - ((unicode_char)'a' - (unicode_char)'A');
  }
  
  s = (*u->u2c)(u, uc, NULL);
  free(uc);
  return (s);
}

static char *tolower_func(const struct unicode_info *u,
			  const char *cp, int *ip)
{
  unicode_char *uc = (*u->c2u)(u, cp, ip);
  char *s;
  size_t i;

  if (!uc)
    return (NULL);

  for (i=0; uc[i]; i++) {
    if ((unicode_char)'A' <= uc[i] && uc[i] <= (unicode_char)'Z')
      uc[i] = uc[i] + ((unicode_char)'a' - (unicode_char)'A');
  }

  s = (*u->u2c)(u, uc, NULL);
  free(uc);
  
  return (s);
}


static char *totitle_func(const struct unicode_info *u,
			  const char *cp, int *ip)
{
  unicode_char *uc = (*u->c2u)(u, cp, ip);
  char *s;

  if (!uc)
    return (NULL);

  /* Uh, sorry, what's "title" char? */
  /*
   * for (i=0; uc[i]; i++)
   * uc[i] = unicode_tc(uc[i]);
   */

  s = (*u->u2c)(u, uc, NULL);
  free(uc);
  return (s);
}

extern const struct unicode_info unicode_UTF8;

const struct unicode_info unicode_ISO2022_KR = {
	"ISO-2022-KR",
	UNICODE_MB | UNICODE_REPLACEABLE | UNICODE_SISO |
	UNICODE_HEADER_BASE64,
	c2u_iso2022kr,
	u2c_iso2022kr,
	toupper_func,
	tolower_func,
	totitle_func,
	&unicode_UTF8
};

const struct unicode_info unicode_EUC_KR = {
	"EUC-KR",
	UNICODE_MB | UNICODE_REPLACEABLE | UNICODE_USASCII |
	UNICODE_HEADER_BASE64 | UNICODE_BODY_BASE64,
	c2u_euckr,
	u2c_euckr,
	toupper_func,
	tolower_func,
	totitle_func,
	&unicode_UTF8
};

const struct unicode_info unicode_CP949 = {
	"CP949",
	UNICODE_MB | UNICODE_REPLACEABLE |
	UNICODE_HEADER_BASE64 | UNICODE_BODY_BASE64,
	c2u_cp949,
	u2c_cp949,
	toupper_func,
	tolower_func,
	totitle_func,
	&unicode_UTF8
};

