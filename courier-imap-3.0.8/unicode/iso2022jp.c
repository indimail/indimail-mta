/*
 * ISO-2022-JP <=> Unicode translate functions.
 *   by Norihisa Washitake <nori@washitake.com>
 * US-ASCII/JIS X 0201/JIS X 0212 support
 *   by Hatuka*nezumi - IKEDA Soji <nezumi@jca.apc.org>
 *
 * $Id: iso2022jp.c,v 1.12 2004/05/23 14:28:24 mrsam Exp $
 *
 * This conversion is highly expensive, so it is recommended
 * that you do not include iso-2022-jp supprt unless you need it.
 */

/*
 *  Debug Option.
 *   if you want to make iso2022jp test application,
 *   please set the value of _DEBUG to non-zero.
 */
#define JIS_DEBUG 0
/* #define JIS_BUILD_APP */

#include "iso2022jp.h"

#if (JIS_DEBUG) > 0
#ifdef JIS_BUILD_APP
  #define JIS_OUT fprintf
  #define JIS_OUT_FH stderr
#else
  #include <syslog.h>
  #define JIS_OUT syslog
  #define JIS_OUT_FH (LOG_MAIL|LOG_DEBUG)
#endif
#endif


/*
 *  read_jis_char.
 *  -- from my second kanji conversion library in 2001. --
 *  Arguments:
 *    src: text in iso-2022-jp.
 *    ch:  character info of each character.
 *  Returns:
 *    characters to be skipped in original text.
 *    this value is at least 1.
 */

static size_t read_jis_char(const char* src, struct jischar_t *ch)
{
  /*
   * In most cases, JIS characters are grouped in 0x20
   * characters.  So we switch by value of src[0]/0x20.
   */
  switch (src[0] >> 5) {
  case 0:                 /* 0x00 to 0x1F */
    switch (src[0]) {
    case JIS_CHAR_SI:
      ch->type = JIS_TYPE_8BITKANA;
      ch->value = 0;
      return 1;
    case JIS_CHAR_SO:
      ch->type = JIS_TYPE_ASCII;
      ch->value = 0;
      return 1;
    case JIS_CHAR_ESC:
      ch->value = 0;
      switch (src[1]) {
      case '(':           /* 94 character set (G0) */
	switch (src[2]) {
	case 'B':         /* US-ASCII */
	  ch->type = JIS_TYPE_ASCII;
	  return 3;
	case 'I':         /* JIS X 0201 GR */
	  ch->type = JIS_TYPE_7BITKANA;
	  return 3;
	case 'J':         /* JIS X 0201 GL */
	  ch->type = JIS_TYPE_ROMAN;
	  return 3;
	default:
	  ch->type = JIS_TYPE_ASCII;
	  ch->value = JIS_CHAR_ESC;
	  return 1;
	}
      case '$':           /* 94/96n character set */
	switch (src[2]) {
	case '@':         /* JIS C 6226:1978 */
	  ch->type = JIS_TYPE_JISX0208_1978;
	  return 3;
	case 'B':         /* JIS X 0208:1983/1990/1997 */
	  ch->type = JIS_TYPE_JISX0208;
	  return 3;
	case '(':
	  switch (src[3]) {
	  case '@':       /* JIS C 6226:1978 */
	    ch->type = JIS_TYPE_JISX0208_1978;
	    return 4;
	  case 'B':       /* JIS X 0208:1983/1990/1997 */
	    ch->type = JIS_TYPE_JISX0208;
	    return 4;
	  case 'D':       /* JIS X 0212:1990 */
	    ch->type = JIS_TYPE_JISX0212;
	    return 4;
	  default:
	    ch->type = JIS_TYPE_BINARY;
	    ch->value = JIS_CHAR_ESC;
	    return 1;
	  }
	default:
	  ch->type = JIS_TYPE_BINARY;
	  ch->value = JIS_CHAR_ESC;
	  return 1;
	}
      case 'K':           /* NEC KANJI (IN) */
	ch->type = JIS_TYPE_JISX0208_1978;
	return 1;
      case 'H':           /* NEC KANJI (OUT) */
	ch->type = JIS_TYPE_ASCII;
	return 1;
      }
    default:
      ch->type = JIS_TYPE_BINARY;
      ch->value = src[0];
      return 1;
    }
  case 1:                 /* 0x20 to 0x3F */
  case 2:                 /* 0x40 to 0x5F */
    if (ch->type == JIS_TYPE_7BITKANA) {
      ch->value = src[0] + 0x80;
      return 1;
    }
    /* Other than 7bit kana are passed to next */
  case 3:                 /* 0x60 to 0x7F */
    if (src[0] == 0x7F) {
      ch->type = JIS_TYPE_BINARY;
      ch->value = src[0];
      return 1;
    }
    if ((ch->type == JIS_TYPE_JISX0208
        || ch->type == JIS_TYPE_JISX0208_1978
        || ch->type == JIS_TYPE_JISX0212) && src[1]) {
      ch->value = (src[0] * 0x100) + src[1];
      return 2;
    }
    ch->value = src[0];
    return 1;
  case 4:                 /* 0x80 to 0x9F */
    ch->value = src[0];
    ch->type = JIS_TYPE_BINARY;
    return 1;
  case 5:                 /* 0xA0 to 0xBF */
  case 6:                 /* 0xC0 to 0xDF */
    if (ch->type == JIS_TYPE_8BITKANA) {
      if (0xA0 < (unsigned)src[0] && (unsigned)src[0] <= 0xDF) {
	ch->value = (unsigned char)src[0];
	return 1;
      }
    }
    ch->type = JIS_TYPE_BINARY;
    ch->value = (unsigned char)src[0];
    return 1;
  case 7:                 /* 0xE0 to 0xFF */
    ch->value = (unsigned char)src[0];
    ch->type = JIS_TYPE_BINARY;
    return 1;
  default:
    ch->value = (unsigned char)src[0];
    ch->type = JIS_TYPE_BINARY;
    return 1;
  }
}

static unicode_char c2u_conv(int j, int jis_type)
{
  unsigned int upper = (j >> 8);
  unsigned int lower = j & 0xFF;
  const unicode_char **tbls; 
 
  if (!upper)
  {
    switch (jis_type)
    {
    /* JIS X 0201 GR */
    case JIS_TYPE_7BITKANA:
    case JIS_TYPE_8BITKANA:
      if (0xA1 <= lower && lower <=0xDF)
        return (unicode_char)(lower + (0xFF9F - 0xDF));
      else
        return (unicode_char)0xFFFD;
      break;

    /* JIS X 0201 GL */
    case JIS_TYPE_ROMAN:
      /* 2 characters replaced by JIS X 0201 */
      if (lower == 0x5C) /* REVERSE SOLIDUS -> YEN SIGN */
        return (unicode_char)0x00A5;
      if (lower == 0x7E) /* TILDE -> OVERLINE */
        return (unicode_char)0x203E;
      /* break; */
    /* US-ASCII or Control characters */
    case JIS_TYPE_ASCII:
    case JIS_TYPE_BINARY:
      if (lower < 0x80)
        return (unicode_char)lower;
      else
        return (unicode_char)0xFFFD;
      break;

    /* Otherwise return REPLACEMENT CHARACTER. */
    default:
      return (unicode_char)0xFFFD;
    }
  }

  switch (jis_type)
  {
  /* JIS X 0208:1983/1990/1997 */
  case JIS_TYPE_JISX0208:
    tbls = jisx0208_to_uni_tbls;
    break;

  /* JIS C 6226:1978 */
  case JIS_TYPE_JISX0208_1978:
    tbls = jisx0208_1978_to_uni_tbls;
    break;

  /* JIS X 0212:1990 */
  case JIS_TYPE_JISX0212:
    tbls = jisx0212_to_uni_tbls;
    break;

  /* Otherwise return REPLACEMENT CHARACTER. */
  default:
    return (unicode_char)0xFFFD;
    break;
  }
  
  if (0x20 < upper && upper < 0x7F
        && 0x20 < lower && lower < 0x7F)
  {
    if (tbls[upper-0x21] != NULL
        && tbls[upper-0x21][lower-0x21] != (unicode_char)0x003F)
    {
      if (tbls[upper-0x21][lower-0x21])
	return tbls[upper-0x21][lower-0x21];
      return (unicode_char)0xFFFD;
    }
  }
  
  /* we should think of 8bit-JIS, maybe. */
  /* but currently returns the REPLACEMENT CHARACTER. */
  return (unicode_char)0xFFFD;
}

static unicode_char *c2u(const struct unicode_info *u,
			 const char *jis_str, int *err)
{
  size_t i, cnt, w;
  unicode_char *uc;
  struct jischar_t jchar;
  
  if (err)
    *err = -1;
  
  /* Count the number of potential unicode characters first. */
  i = cnt = 0;
  jchar.type = 0;
  jchar.value = 0;
  while (jis_str[i]) {
    i += read_jis_char(&jis_str[i], &jchar);
    if (jchar.value)
      ++cnt;
  }

  uc = malloc((cnt+1) * sizeof(unicode_char));
#if JIS_DEBUG>0
  if (uc)
    JIS_OUT(JIS_OUT_FH, "c2u: allocated heap; 0x%04X bytes.\n", cnt+1);
  else
    JIS_OUT(JIS_OUT_FH, "c2u: heap allocation failed; 0x%04X bytes.\n", cnt+1);
#endif
  if (!uc)
    return (NULL);
  
  i = cnt = 0;
  jchar.type = 0;
  jchar.value = 0;
  while (jis_str[i]) {
    w = read_jis_char(&jis_str[i], &jchar);
    if (jchar.value) {
      uc[cnt] = c2u_conv(jchar.value, jchar.type);
#if JIS_DEBUG > 1
      JIS_OUT(JIS_OUT_FH, "c2u: converted; JIS 0x%04X => U+%04X", jchar.value, uc[cnt]);
#endif
      if (uc[cnt] == (unicode_char)0xFFFD && err)
      {
        *err = i;
        free(uc);
        return NULL;
      }
      ++cnt;
    }
    i+=w;
  }
  
  uc[cnt] = 0;
#if JIS_DEBUG > 0
  JIS_OUT(JIS_OUT_FH, "c2u: end of heap; 0x%04X bytes.", cnt+1);
#endif
  return (uc);
}

static void revlookup(unicode_char u, struct jischar_t *ch)
{
  unsigned int upper = u >> 8;
  unsigned int lower = u & 0xff;

  /* ISO-2022-JP(-1) is mapped inside BMP range. */
  if (u >= (unicode_char)0x10000)
  {
    ch->type = JIS_TYPE_BINARY;
    ch->value = 0x003F;
    return;
  }

  /* US-ASCII */
  if (u < (unicode_char)0x0080)
  {
    ch->type = JIS_TYPE_ASCII;
    ch->value = (unsigned)u;
    return;
  }

  /* 2 Characters replaced by JIS X 0201 */
  if (u == (unicode_char)0x00a5)
  {
    ch->type = JIS_TYPE_ROMAN;
    ch->value = 0x5C;
    return;
  }
  if (u == (unicode_char)0x203E)
  {
    ch->type = JIS_TYPE_ROMAN;
    ch->value = 0x7E;
    return;
  }

  /* JIS X 0201 GR */
  if ((unicode_char)0xFF61 <= u && u <= (unicode_char)0xFF9F)
  {
    ch->type = JIS_TYPE_8BITKANA;
    ch->value = u - (unsigned)0xFF40 + (unsigned)0x80;
    return;
  }

  /* JIS X 0208/JIS X 0212 */
  if (uni_to_jisx0208_tbls[upper] != NULL
      && uni_to_jisx0208_tbls[upper][lower] != 0x003F)
  {
    ch->type = JIS_TYPE_JISX0208;
    ch->value = uni_to_jisx0208_tbls[upper][lower];
    return;
  }
  if (uni_to_jisx0212_tbls[upper] != NULL
      && uni_to_jisx0212_tbls[upper][lower] != 0x003F)
  {
    ch->type = JIS_TYPE_JISX0212;
    ch->value = uni_to_jisx0212_tbls[upper][lower];
    return;
  }
  
  /* return 'unknown' character if unknown */
  ch->type = JIS_TYPE_BINARY;
  ch->value = 0x003F;
  return;
}

#if 0
static int get_iso2022jp_type(unsigned j)
{
  if (0xA0 < j && j < 0xE0)
    return JIS_TYPE_8BITKANA;
  if (j > 0xff)
    return JIS_TYPE_KANJI;
  return JIS_TYPE_ASCII;
}
#endif

static char *u2c(const struct unicode_info *u,
		 const unicode_char *str, int *err)
{
  size_t i, cnt;
  int j;
  int jtype = JIS_TYPE_ASCII;
  int jt;
  char *s;
  struct jischar_t ch; 

  if (err)
    *err = -1;

  for (i = cnt = 0; str[i]; i++) {
    revlookup(str[i], &ch);
    jt = ch.type;
    j = ch.value;
    if (jt != jtype) {
      cnt += ((jt == JIS_TYPE_JISX0212) ? 4 : 3);
      jtype = jt;
    }
    cnt += ((jtype == JIS_TYPE_JISX0208 || jtype == JIS_TYPE_JISX0212) ? 2 : 1);
  }
  if (jtype != JIS_TYPE_ASCII && jtype != JIS_TYPE_BINARY)
    cnt += 3;

  s = malloc(cnt+1);
#if JIS_DEBUG > 0
  if (s)
    JIS_OUT(JIS_OUT_FH, "u2c: allocated heap; 0x%04X bytes.\n", cnt+1);
  else
    JIS_OUT(JIS_OUT_FH, "u2c: heap allocation failed; 0x%04X bytes.\n", cnt+1);
#endif
  if (!s)
    return (NULL);

  jtype = JIS_TYPE_ASCII;
  for (i = cnt = 0; str[i]; i++) {
    revlookup(str[i], &ch);

    jt = ch.type;
    j = ch.value;
    if (jt != jtype) {
      switch (jt) {
      case JIS_TYPE_JISX0208:
	s[cnt++] = JIS_CHAR_ESC;
	s[cnt++] = '$';
	s[cnt++] = 'B';
#if JIS_DEBUG > 2
	JIS_OUT(JIS_OUT_FH, "u2c: changed map; JIS_TYPE_JISX0208.\n");
#endif
	break;
      case JIS_TYPE_JISX0212:
        s[cnt++] = JIS_CHAR_ESC;
        s[cnt++] = '$';
        s[cnt++] = '(';
        s[cnt++] = 'D';
	break;
      case JIS_TYPE_7BITKANA:
      case JIS_TYPE_8BITKANA:
	s[cnt++] = JIS_CHAR_ESC;
	s[cnt++] = '(';
	s[cnt++] = 'I';
#if JIS_DEBUG > 2
	JIS_OUT(JIS_OUT_FH, "u2c: changed map; JIS_TYPE_8BITKANA.\n");
#endif
	break;
      case JIS_TYPE_ROMAN:
        s[cnt++] = JIS_CHAR_ESC;
        s[cnt++] = '(';
        s[cnt++] = 'J';
        break;
      default:
	s[cnt++] = JIS_CHAR_ESC;
	s[cnt++] = '(';
	s[cnt++] = 'B';
#if JIS_DEBUG > 2
	JIS_OUT(JIS_OUT_FH, "u2c: changed map; JIS_TYPE_ASCII.\n");
#endif
	break;
      }
      jtype = jt;
    }
    switch (jtype) {
    case JIS_TYPE_JISX0208:
    case JIS_TYPE_JISX0212:
      s[cnt++] = (char)(j >> 8);
      s[cnt++] = (char)(j & 0xff);
      break;
    case JIS_TYPE_7BITKANA:
    case JIS_TYPE_8BITKANA:
      s[cnt++] = (char)(j - 0x80);
      break;
    default:
      s[cnt++] = (char)j;
      break;
    }
#if JIS_DEBUG > 1
    JIS_OUT(JIS_OUT_FH, "u2c: converted; U+%04X => JIS 0x%04X\n", str[i], j);
#endif
    if (jtype == JIS_TYPE_BINARY && j == 0x003F)
      if (err)
      {
        *err = i;
        free(s);
        return NULL;
      }
  }
  if (jtype != JIS_TYPE_ASCII && jtype != JIS_TYPE_BINARY) {
    s[cnt++] = JIS_CHAR_ESC;
    s[cnt++] = '(';
    s[cnt++] = 'B';
  }
  s[cnt] = '\x0';

#if JIS_DEBUG > 0
  JIS_OUT(JIS_OUT_FH, "u2c: end of heap; 0x%04X bytes.\n", cnt+1);
#endif
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

  for (i=0; uc[i]; i++) {
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

const struct unicode_info unicode_ISO2022_JP = {
  "ISO-2022-JP",
  UNICODE_MB | UNICODE_REPLACEABLE | UNICODE_SISO |
  UNICODE_HEADER_BASE64,
  c2u,
  u2c,
  toupper_func,
  tolower_func,
  totitle_func,
  &unicode_UTF8
};

const struct unicode_info unicode_ISO2022_JP_1 = {
  "ISO-2022-JP-1",
  UNICODE_MB | UNICODE_REPLACEABLE | UNICODE_SISO |
  UNICODE_HEADER_BASE64,
  c2u,
  u2c,
  toupper_func,
  tolower_func,
  totitle_func,
  &unicode_UTF8
};

#if (JIS_DEBUG > 0) && defined(JIS_BUILD_APP)
int main(int argc, char** argv)
{
  FILE* fp;
  char c;
  int cnt;
  char* str;
  unicode_char* ustr;
  char* jstr;
  int i;

  if (argc<2) {
    JIS_OUT(JIS_OUT_FH, "usage: %s filename(s)\n", argv[0]);
    exit(1);
  }
  
  while (argc > 1) {
    --argc;
    JIS_OUT(JIS_OUT_FH, "main: opening file %s.\n", argv[argc]);
    fp = fopen(argv[argc], "r");
    cnt=0;
    while (c = fgetc(fp) != EOF)
      cnt++;
    
    str = malloc(cnt+1);
    fseek(fp, 0, SEEK_SET);
    fread(str, cnt, 1, fp);
    str[cnt] = 0;
    
    ustr = c2u(str, NULL);
    /*  for (i=0; ustr[i]; i++) 
     *    putchar(ustr[i]);
     */
    jstr = u2c(ustr, NULL);
    for (i=0; jstr[i]; i++)
      putchar(jstr[i]);
    
    free(jstr);
    free(ustr);
    free(str);
  } 
  return 1;
}
#endif /* defined(JIS_BUILD_APP) */
