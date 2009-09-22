/*
 * EUC-JP <=> Unicode translate functions.
 *   by Hatuka*nezumi - IKEDA Soji <nezumi@jca.apc.org>
 *
 */

#include <stdio.h>
#include <string.h>
#include "unicode.h"

extern const unicode_char* jisx0208_to_uni_tbls[];
extern const unicode_char* jisx0212_to_uni_tbls[];
extern const unsigned* uni_to_jisx0208_tbls[];
extern const unsigned* uni_to_jisx0212_tbls[];

static unicode_char *c2u(const struct unicode_info *u,
			 const char *eucjp_str, int *err)
{
	unicode_char *uc=0;
	unsigned char hi=0, lo=0;
	int len=0;
	int i=0;
	int pos=0;

	if(err) *err = -1;
	
	len = strlen(eucjp_str);
	uc = (unicode_char*)malloc((len+1) * sizeof(unicode_char) *2);

	if (!uc)
		return NULL;

	for(i=0; i<len;) {
		/* US-ASCII */
		if((unsigned char)eucjp_str[i] < 0x80)
		{
			uc[pos++] = (unicode_char)(eucjp_str[i]);
			i++;
		}
		/* JIS X 0201 GR; SS2 */
		else if ((unsigned char)eucjp_str[i] == 0x8e
		    && (unsigned char)eucjp_str[i+1] >= 0xa1
		    && (unsigned char)eucjp_str[i+1] <= 0xdf)
		{
			lo = (unsigned char)eucjp_str[i+1];

			/* EUCJP -> JIS */
			lo -= 0x80;

			uc[pos++] = (unicode_char)(lo+(unsigned)0xff40);
			i+=2;
		}
		/* JIS X 0212; SS3 */
		else if ((unsigned char)eucjp_str[i] == 0x8f
		    && (unsigned char)eucjp_str[i+1] >= 0xa1
		    && (unsigned char)eucjp_str[i+2] >= 0xa1)
		{
			hi = (unsigned char)eucjp_str[i+1];
			lo = (unsigned char)eucjp_str[i+2];

			/* EUCJP -> JIS */	
			hi -= 0x80;
			lo -= 0x80;

			if (jisx0212_to_uni_tbls[hi-0x21] != NULL
			    &&  jisx0212_to_uni_tbls[hi-0x21][lo-0x21] != 0x003f)
				uc[pos++] = jisx0212_to_uni_tbls[hi-0x21][lo-0x21];
			else if (err)
			{
				*err = i;
				free(uc);
				return NULL;
			}
			else
				uc[pos++] = (unicode_char)0xfffd;
			i+=3;
		}
		/* JIS X 0208 */
		else if ((unsigned char)eucjp_str[i] >= 0xa1
		    && (unsigned char)eucjp_str[i+1] >= 0xa1)
		{
			hi = (unsigned char)eucjp_str[i];
			lo = (unsigned char)eucjp_str[i+1];
			
			/* EUCJP -> JIS */
			hi -= 0x80;
			lo -= 0x80;

			/* JIS -> Unicode */
			if (jisx0208_to_uni_tbls[hi-0x21] != NULL
			    && jisx0208_to_uni_tbls[hi-0x21][lo-0x21] != 0x003f)
				uc[pos++] = jisx0208_to_uni_tbls[hi-0x21][lo-0x21];
			
			else if (err)
			{
				*err = i;
				free(uc);
				return NULL;
			}
			else
				uc[pos++] = (unicode_char)0xfffd;
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
			uc[pos++] = (unicode_char)0xfffd;
			i++;
		}
	}
	uc[pos++] = 0;

 	return uc;
}

static char *u2c(const struct unicode_info *u,
		 const unicode_char *str, int *err)
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
		int jis_char = 0;
		unsigned char hi=0, lo=0;

		unsigned char str_i_high=str[i] >> 8;

		/* EUC-JP is mapped inside BMP range. */
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
			s[pos++] = str[i];
		/* For compatibility: 2 characters replaced by JIS X 0201 */
		else if (str[i] == (unicode_char)0x00A5) /* YEN SIGN */
			s[pos++] = 0x5C;
		else if (str[i] == (unicode_char)0x203E) /* OVERLINE */
			s[pos++] = 0x7E;
		/* JIS X 0201 GR */
		else if (str[i] >= (unicode_char)0xff61
		    && str[i] <= (unicode_char)0xff9f)
		{
			lo = (unsigned char)(str[i] - (unsigned)0xff40);
			/* JIS -> EUCJP */
			lo += 0x80;
			s[pos++] = 0x8e;
			s[pos++] = lo;
		}
		/* JIS X 0208 */
		else if (uni_to_jisx0208_tbls[str_i_high] != NULL
		    && uni_to_jisx0208_tbls[str_i_high][str[i] & 0xff] != 0x003F)
		{
			/* Unicode -> JIS */
			jis_char = uni_to_jisx0208_tbls[str_i_high][str[i] & 0xff];
			hi = jis_char >> 8;
			lo = jis_char & 0xff;

			if (hi)
			{		
				/* JIS -> EUCJP */
				hi += 0x80;
				lo += 0x80;

				s[pos++] = hi;
				s[pos++] = lo;
			}
			else if (err)
			{
				*err = i;
				free(s);
				return NULL;
			}
			else
				s[pos++] = '?';
		}
		/* Otherwise, search on JIS X 0212 */
		else if (uni_to_jisx0212_tbls[str_i_high] != NULL
		    && uni_to_jisx0212_tbls[str_i_high][str[i] & 0xff] != 0x003F)
		{
                        /* Unicode -> JIS */
                        jis_char = uni_to_jisx0212_tbls[str_i_high][str[i] & 0xff];
                        hi = jis_char >> 8;
                        lo = jis_char & 0xff;

			if (hi) {
				/* JIS -> EUCJP */
				hi += 0x80;
				lo += 0x80;

				s[pos++] = 0x8f;
				s[pos++] = hi;
				s[pos++] = lo;
			}
			else if (err)
			{
				*err = i;
				free(s);
				return NULL;
			}
			else
				s[pos++] = '?';
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

static char *toupper_func(const struct unicode_info *u,
			  const char *cp, int *ip)
{
  unicode_char *uc = c2u(u, cp, ip);
  char *s;
  size_t i;
  
  if (!uc)
    return (NULL);

  for (i=0; uc[i] && i<10000; i++) {
    if ((unicode_char)'a' <= uc[i] && uc[i] <= (unicode_char)'z')
      uc[i] = uc[i] - ((unicode_char)'a' - (unicode_char)'A');
  }
  
  s = u2c(u, uc, NULL);
  free(uc);
  return (s);
}

static char *tolower_func(const struct unicode_info *u,
			  const char *cp, int *ip)
{
  unicode_char *uc = c2u(u, cp, ip);
  char *s;
  size_t i;
  
  if (!uc)
    return (NULL);

  for (i=0; uc[i]; i++) {
    if ((unicode_char)'A' <= uc[i] && uc[i] <= (unicode_char)'Z')
      uc[i] = uc[i] + ((unicode_char)'a' - (unicode_char)'A');
  }

  s = u2c(u, uc, NULL);
  free(uc);
  
  return (s);
}


static char *totitle_func(const struct unicode_info *u,
			  const char *cp, int *ip)
{
  unicode_char *uc = c2u(u, cp, ip);
  char *s;
  
  if (!uc)
    return (NULL);

  /* Uh, sorry, what's "title" char? */
  /*
   * for (i=0; uc[i]; i++)
   * uc[i] = unicode_tc(uc[i]);
   */

  s = u2c(u, uc, NULL);
  free(uc);
  return (s);
}

extern const struct unicode_info unicode_UTF8;

const struct unicode_info unicode_EUC_JP = {
  "EUC-JP",
  UNICODE_MB | UNICODE_REPLACEABLE | UNICODE_USASCII |
  UNICODE_HEADER_BASE64 | UNICODE_BODY_BASE64,
  c2u,
  u2c,
  toupper_func,
  tolower_func,
  totitle_func,
  &unicode_UTF8
};

