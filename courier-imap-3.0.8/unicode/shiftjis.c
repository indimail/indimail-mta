/*
 * Shift_JIS <=> Unicode translate functions.
 *   by Yu Kobayashi <mail@yukoba.jp>
 * Modification for JIS X 0208:1997 Annex 1 implementation
 *   by Hatuka*nezumi - IKEDA Soji <nezumi@jca.apc.org>
 *
 */

#include <stdio.h>
#include <string.h>
#include "unicode.h"

#define SJIS_DEBUG 0

extern const unicode_char* jisx0208_to_uni_tbls[];
extern const unsigned* uni_to_jisx0208_tbls[];

static unicode_char *c2u(const struct unicode_info *u,
			 const char *sjis_str, int *err)
{
	unicode_char *uc=0;
	unsigned char hi=0, lo=0;
	int len=0;
	int i=0;
	int pos=0;

	if(err) *err = -1;
	
	len = strlen(sjis_str);
	uc = (unicode_char*)malloc((len+1) * sizeof(unicode_char) *2);

	if (!uc)
		return NULL;

	for(i=0; i<len;) {
		/* 2 Characters replaced by JIS X 0201 */
		if (sjis_str[i] == 0x5C) /* YEN SIGN */
		{
			uc[pos++] = (unicode_char)0x00A5;
			i++;
		}
		else if (sjis_str[i] == 0x7E) /* OVERLINE */
		{
			uc[pos++] = (unicode_char)0x203E;
			i++;
		}
		/* Other JIS X 0201 GL */
		else if ((unsigned)sjis_str[i] < 0x80)
		{
			uc[pos++] = (unicode_char)sjis_str[i];
			i++;
		}
		/* JIS X 0201 GR */
		else if ((unsigned char)sjis_str[i] >= 0xa1
                    && (unsigned char)sjis_str[i] <= 0xdf)
		{
			lo = (unsigned char)sjis_str[i];

			/* SHIFT_JIS -> JIS */
			lo -= 0x80;

			uc[pos++] = (unicode_char)(lo+(unsigned)0xff40);
			i++;
		}
		/* 2 byte characters */
		else if ((((unsigned char)sjis_str[i] >= 0x81
		    && (unsigned char)sjis_str[i] <= 0x9F)
		    || ((unsigned char)sjis_str[i] >= 0xE0
		    && (unsigned char)sjis_str[i] <= 0xEF))
		    && (((unsigned char)sjis_str[i+1] >= 0x40
		    && (unsigned char)sjis_str[i+1] <= 0x7E)
		    || ((unsigned char)sjis_str[i+1] >= 0x80
		    && (unsigned char)sjis_str[i+1] <= 0xFC)))
		{
			hi = (unsigned char)sjis_str[i];
			lo = (unsigned char)sjis_str[i+1];
			
			/* SJIS -> JIS */
			if( lo < 0x9f ) {
				if( hi < 0xa0 ) {
					hi -= 0x81;
					hi *= 2;
					hi += 0x21;
				} else {
					hi -= 0xe0;
					hi *= 2;
					hi += 0x5f;
				}
				if( lo > 0x7f )
					--lo;
				lo -= 0x1f;
			} else {
				if( hi < 0xa0 ) {
					hi -= 0x81;
					hi *= 2;
					hi += 0x22;
				} else {
					hi -= 0xe0;
					hi *= 2;
					hi += 0x60;
				}
				lo -= 0x7e;
			}
			
			/* JIS -> Unicode */
			if (jisx0208_to_uni_tbls[hi-0x21] != NULL
			    && jisx0208_to_uni_tbls[hi-0x21][lo-0x21] !=
			    (unicode_char)0x003F)
				uc[pos++] = jisx0208_to_uni_tbls[hi-0x21][lo-0x21];
			else if (err)
			{
				*err = i;
				free(uc);
				return NULL;
			}
			else
				uc[pos++] = (unicode_char)0xFFFD;	
			
			i+=2;
		}
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

	for(i=0; str[i]; i++) {
		int jis_char = 0;
		unsigned char hi=0, lo=0;

		unsigned char str_i_high=str[i] >> 8;

		/* SHIFT_JIS is mapped inside BMP range */
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
		/* JIS X 0201 GL or US-ASCII */
		else if (str[i] < (unicode_char)0x0080)
			s[pos++] = (char)str[i];
		/* 2 characters replaced by JIS X 0201 */
		else if (str[i] == 0x00A5) /* YEN SIGN */
			s[pos++] = (char)0x5C;
		else if (str[i] == 0x203E) /* OVERLINE */
			s[pos++] = (char)0x7E;
		/* JIS X 0201 GR */
		else if (str[i] >= (unicode_char)0xff61
		    && str[i] <= (unicode_char)0xff9f)
		{
			lo = (unsigned char)(str[i] - (unsigned)0xff40);
                        /* JIS -> SHIFT_JIS */
			lo += 0x80;
			s[pos++] = lo;
		}
		/* Not found */
		else if (uni_to_jisx0208_tbls[str_i_high] == NULL
		    || uni_to_jisx0208_tbls[str_i_high][str[i] & 0xff] == '?')
		{
			if (err)
			{
				*err = i;
				free(s);
				return NULL;
			}
			s[pos++] = '?';
		}
		/* 2 byte characters */
		else
		{
			jis_char = uni_to_jisx0208_tbls[str_i_high][str[i] & 0xff];
			hi = jis_char >> 8;
			lo = jis_char & 0xff;
		
			/* JIS -> SJIS */
			if( ( hi % 2 ) == 0 )
				lo += 0x7d;
			else
				lo += 0x1f;
   
			if( lo > 0x7e )
				++ lo;
   
			if( hi < 0x5f ) {
				++hi;
				hi /= 2;
				hi += 0x70;
			} else {
				++hi;
				hi /= 2;
				hi += 0xb0;
			}
			s[pos++] = hi;
			s[pos++] = lo;
		}
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

const struct unicode_info unicode_SHIFT_JIS = {
  "SHIFT_JIS",
  UNICODE_MB | UNICODE_REPLACEABLE |
  UNICODE_HEADER_BASE64 | UNICODE_BODY_BASE64,
  c2u,
  u2c,
  toupper_func,
  tolower_func,
  totitle_func,
  &unicode_UTF8
};

