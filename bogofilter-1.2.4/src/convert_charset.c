/* $Id: convert_charset.c 6017 2005-05-31 15:18:40Z m-a $ */

/*****************************************************************************

NAME:
   charset.c -- provide charset support for bogofilter's lexer.

Note:

   Character translation is done to make life easier for the lexer.
   Text is changed only after the message has been saved for
   passthrough.  The end user (mail reader) never sees any changes -
   only the lexer.

AUTHOR:
   David Relson <relson@osagesoftware.com>

******************************************************************************/

#include "common.h"

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include "charset.h"
#include "convert_charset.h"
#include "xmalloc.h"
#include "xstrdup.h"

#define	SP	' '

static void map_default(void);
static void map_us_ascii(void);

static void map_iso_8859_1(void);
static void map_iso_8859_2(void);
static void map_iso_8859_3(void);
static void map_iso_8859_4(void);
#ifndef	CP866
static void map_iso_8859_5(void);
#endif
static void map_iso_8859_6(void);
static void map_iso_8859_7(void);
static void map_iso_8859_8(void);
static void map_iso_8859_9(void);
static void map_iso_8859_10(void);
static void map_iso_8859_13(void);
static void map_iso_8859_14(void);
static void map_iso_8859_15(void);

static void map_unicode(void);

#ifndef	CP866
static void map_windows_1251_to_koi8r(void);
#else
static void map_windows_1251_to_cp866(void);
static void map_koi8_r_to_cp866(void);
static void map_iso_8859_5_to_cp866(void);
#endif
static void map_windows_1252(void);
static void map_windows_1256(void);

static void map_nonascii_characters(void);

#define	DEBUG
#undef	DEBUG

#ifndef	DEBUG
#define	PRINT_CHARSET_TABLE
#else
#define	PRINT_CHARSET_TABLE	print_charset_table()
#undef	DEBUG_GENERAL
#define	DEBUG_GENERAL(level)	(verbose >= level)
#endif

#ifdef	DEBUG
static void print_charset_table(void)
{
    int c,r,i;
    char ch;
    if (!DEBUG_GENERAL(1))
	return;
    printf( "\n" );
    for (r=0; r<4; r+=1) {
	for (c=0; c<64; c+=1) {
	    i=r*64+c;
	    ch=charset_table[i];
	    if (ch != 0x08 && ch != 0x09 && ch != '\n' && ch != '\r')
		printf(" %02X.%2c.%02X", i, ch, ch);
	    else
		printf(" %02X.%02X.%02X", i, ch, ch);
	    if ((c & 15) == 15)
		printf( "\n" );
	}
    }
    printf( "\n" );
}
#endif

static void map_nonascii_characters(void)
{
    uint ch;
    for (ch = 0; ch < COUNTOF(charset_table); ch += 1)
    {
	/* convert high-bit characters to '?' */
	if (ch & 0x80 && casefold_table[ch] == ch)
	    casefold_table[ch] = '?';
    }
}

static void map_xlate_characters(unsigned char *xlate, uint size)
{
    uint i;
    for (i = 0; i < size; i += 2)
    {
	byte from = xlate[i];
	byte to   = xlate[i+1];
	charset_table[from] = to;
    }
}

static void map_default(void)
{
    unsigned int ch;

    for (ch = 0; ch < COUNTOF(charset_table); ch += 1)
    {
	charset_table[ch] = casefold_table[ch] = ch;
    }

    PRINT_CHARSET_TABLE;

    for (ch=0; ch < COUNTOF(charset_table); ch += 1)
    {
	if (iscntrl(ch) &&		/* convert control characters to blanks */
	    ch != '\t' && ch != '\n')	/* except tabs and newlines		*/
	    charset_table[ch] = SP;
    }

    PRINT_CHARSET_TABLE;
}

static void map_iso_8859_1(void)	/* ISOIEC 8859-1:1998 Latin Alphabet No. 1 */
{
    /* Not yet implemented */
}


static void map_iso_8859_2(void)	/* ISOIEC 8859-2:1999 Latin Alphabet No. 2 */
{
    /* Not yet implemented */
}

static void map_iso_8859_3(void)	/* ISOIEC 8859-3:1999 Latin Alphabet No. 3 */
{
    /* Not yet implemented */
}

static void map_iso_8859_4(void)	/* ISOIEC 8859-4:1998 Latin Alphabet No. 4 */
{
    /* Not yet implemented */
}

#ifndef	CP866
static void map_iso_8859_5(void)	/* ISOIEC 8859-5:1999 LatinCyrillic Alphabet */
{
    /* Not yet implemented */
}
#endif

static void map_iso_8859_6(void)	/* ISOIEC 8859-6:1999 LatinArabic Alphabet */
{
    /* Not yet implemented */
}

static void map_iso_8859_7(void)	/* ISO 8859-7:1987 LatinGreek Alphabet     */
{
    /* Not yet implemented */
}

static void map_iso_8859_8(void)	/* ISOIEC 8859-8:1999 LatinHebrew Alphabet */
{
    /* Not yet implemented */
}

static void map_iso_8859_9(void)	/* ISOIEC 8859-9:1999 Latin Alphabet No. 5 */
{
    /* Not yet implemented */
}

static void map_iso_8859_10(void)	/* ISOIEC 8859-10:1998 Latin Alphabet No. 6 */
{
    /* Not yet implemented */
}

static void map_iso_8859_13(void)	/* ISOIEC 8859-13:1998 Latin Alphabet No. 7 (Baltic Rim) */
{
    /* Not yet implemented */
}

static void map_iso_8859_14(void)	/* ISOIEC 8859-14:1998 Latin Alphabet No. 8 (Celtic) */
{
    /* Not yet implemented */
}

static void map_iso_8859_15(void)	/* ISOIEC 8859-15:1999 Latin Alphabet No. 9 */
{
    static unsigned char xlate_15[] = {
	0xA0, ' ',		/* A0  160      160 NO-BREAK SPACE */
	0xA1, '!',		/* A1  161  б   161 INVERTED EXCLAMATION MARK */
	0xA2, '$',		/* A2  162  в   162 CENT SIGN */
	0xA3, '$',		/* A3  163  г   163 POUND SIGN */
	0xA4, '$',		/* A4  164 EUR 8364 EURO SIGN */
	0xA5, '$',		/* A5  165  е   165 YEN SIGN */
	0xA7, ' ',		/* A7  167  з   167 SECTION SIGN */
	0xA9, ' ',		/* A9  169  й   169 COPYRIGHT SIGN */
	0xAA, ' ',		/* AA  170  к   170 FEMININE ORDINAL INDICATOR */
	0xAB, '\"',		/* AB  171  л   171 LEFT-POINTING DOUBLE ANGLE QUOTATION MARK */
	0xAC, ' ',		/* AC  172  м   172 NOT SIGN */
	0xAD, ' ',		/* AD  173      173 SOFT HYPHEN */
	0xAE, ' ',		/* AE  174  о   174 REGISTERED SIGN */
	0xAF, ' ',		/* AF  175  п   175 MACRON */
	0xB0, ' ',		/* B0  176  ░   176 DEGREE SIGN */
	0xB1, ' ',		/* B1  177  ▒   177 PLUS-MINUS SIGN */
	0xB2, ' ',		/* B2  178  ▓   178 SUPERSCRIPT TWO */
	0xB3, ' ',		/* B3  179  │   179 SUPERSCRIPT THREE */
	0xB5, ' ',		/* B5  181  ╡   181 MICRO SIGN */
	0xB6, ' ',		/* B6  182  ╢   182 PILCROW SIGN */
	0xB7, ' ',		/* B7  183  ╖   183 MIDDLE DOT */
	0xB9, ' ',		/* B9  185  ╣   185 SUPERSCRIPT ONE */
	0xBA, ' ',		/* BA  186  ║   186 MASCULINE ORDINAL INDICATOR */
	0xBB, '\"',		/* BB  187  ╗   187 RIGHT-POINTING DOUBLE ANGLE QUOTATION MARK */
	0xBC, ' ',		/* BC  188 OE   338 LATIN CAPITAL LIGATURE OE */
	0xBD, ' ',		/* BD  189 oe   339 LATIN SMALL LIGATURE OE */
	0xBF, '?',		/* BF  191  ┐   191 INVERTED QUESTION MARK */
	0xD7, '*',		/* D7  215  ╫   215 MULTIPLICATION SIGN */
	0xF7, '/',		/* F7  247  ў   247 DIVISION SIGN */
    };
    map_xlate_characters( xlate_15, COUNTOF(xlate_15) );
    /* Not yet implemented */
}

/* For us-ascii, first do the iso-8859-1 setup then add special
** characters.
*/

static void map_us_ascii(void)
{
    static unsigned char xlate_us[] = {
	0xA0, ' ',	/* no-break space      to space        */
	0x92, '\'',	/* windows apostrophe  to single quote */
	0x93, '"',	/* windows left  quote to double quote */
	0x94, '"',	/* windows right quote to double quote */
	0xA9, ' ',	/* copyright sign      to space        */
	0xAE, ' ',	/* registered sign     to space        */
    };

    map_iso_8859_1();

    map_xlate_characters( xlate_us, COUNTOF(xlate_us) );
}

static void map_unicode(void)
{
    /* Not yet implemented */
}

#ifndef	CP866
static void map_windows_1251_to_koi8r(void)
{
    /* Map:  windows-1251 -> KOI8-R (Cyrillic) */
    /* Contributed by: Yar Tikhiy (yarq@users.sourceforge.net) */
    static byte xlate_1251[] = {
	0xA8, 0xB3,
	0xB8, 0xA3,
	0xE0, 0xC1,  0xE1, 0xC2,  0xE2, 0xD7,  0xE3, 0xC7,  0xE4, 0xC4,  0xE5, 0xC5,  0xE6, 0xD6,  0xE7, 0xDA,
	0xE8, 0xC9,  0xE9, 0xCA,  0xEA, 0xCB,  0xEB, 0xCC,  0xEC, 0xCD,  0xED, 0xCE,  0xEE, 0xCF,  0xEF, 0xD0,
	0xF0, 0xD2,  0xF1, 0xD3,  0xF2, 0xD4,  0xF3, 0xD5,  0xF4, 0xC6,  0xF5, 0xC8,  0xF6, 0xC3,  0xF7, 0xDE,
	0xF8, 0xDB,  0xF9, 0xDD,  0xFA, 0xDF,  0xFB, 0xD9,  0xFC, 0xD8,  0xFD, 0xDC,  0xFE, 0xC0,  0xFF, 0xD1,
	0xC0, 0xE1,  0xC1, 0xE2,  0xC2, 0xF7,  0xC3, 0xE7,  0xC4, 0xE4,  0xC5, 0xE5,  0xC6, 0xF6,  0xC7, 0xFA,
	0xC8, 0xE9,  0xC9, 0xEA,  0xCA, 0xEB,  0xCB, 0xEC,  0xCC, 0xED,  0xCD, 0xEE,  0xCE, 0xEF,  0xCF, 0xF0,
	0xD0, 0xF2,  0xD1, 0xF3,  0xD2, 0xF4,  0xD3, 0xF5,  0xD4, 0xE6,  0xD5, 0xE8,  0xD6, 0xE3,  0xD7, 0xFE,
	0xD8, 0xFB,  0xD9, 0xFD,  0xDA, 0xFF,  0xDB, 0xF9,  0xDC, 0xF8,  0xDD, 0xFC,  0xDE, 0xE0,  0xDF, 0xF1,
    };
    map_xlate_characters( xlate_1251, COUNTOF(xlate_1251) );
}
#endif

#ifdef	CP866
static void map_windows_1251_to_cp866(void)
{
    static byte cp1251_to_cp866[] = {
/*Win-CP1251*/
	192, 'А',  208, 'Р',  224, 'а',  240, 'р',
	193, 'Б',  209, 'С',  225, 'б',  241, 'с',
	194, 'В',  210, 'Т',  226, 'в',  242, 'т',
	195, 'Г',  211, 'У',  227, 'г',  243, 'у',
	196, 'Д',  212, 'Ф',  228, 'д',  244, 'ф',
	197, 'Е',  213, 'Х',  229, 'е',  245, 'х',
	198, 'Ж',  214, 'Ц',  230, 'ж',  246, 'ц',
	151 , '─',199,'З',  215 , 'Ч',  231, 'з',  247, 'ч',
	168 , 'Ё',184, 'ё',  200 , 'И',  216, 'Ш',  232, 'и',  248, 'ш',
/*; символ 169 = (с)/255 в дос */
	169, 255, 201, 'Й',  217 , 'Щ',  233, 'й',  249, 'щ',
	202, 'К',  218 , 'Ъ',  234, 'к',  250, 'ъ',
	203, 'Л',  219 , 'Ы',  235, 'л',  251, 'ы',
	204, 'М',  220 , 'Ь',  236, 'м',  252, 'ь',
	205, 'Н',  221 , 'Э',  237, 'н',  253, 'э',
	206, 'О',  222 , 'Ю',  238, 'о',  254, 'ю',
	207, 'П',  223 , 'Я',  239, 'п',  255, 'я'

    };
    map_xlate_characters( cp1251_to_cp866, COUNTOF(cp1251_to_cp866) );
}
#endif

#ifdef	CP866
static void map_koi8_r_to_cp866(void)
{
/* KOI8-R по Чеpнову
KOI8-R
[CODEPAGE]
*/
    static byte koi8_r_to_cp866[] = {
	128, '─',  144, '░',  160, '═',  176, '╟',  192, 'ю',  208, 'п',  224, 'Ю',  240, 'П',
	129, '│',  145, '▒',  161, '║',  177, '╠',  193, 'а',  209, 'я',  225, 'А',  241, 'Я',
	130, '┌',  146, '▓ ', 162, '╒',  178, '╡',  194, 'б',  210, 'р',  226, 'Б',  242, 'Р',
	131, '┐',  147, 'Ї',  163, 'ё',  179, 'Ё',  195, 'ц',  211, 'с',  227, 'Ц',  243, 'С',
	132, '└',  148, '■',  164, '╓',  180, '╢',  196, 'д',  212, 'т',  228, 'Д',  244, 'Т',
	133, '┘',  149, '·',  165, '╔',  181, '╣',  197, 'е',  213, 'у',  229, 'Е',  245, 'У',
	134, '├',  150, '√',  166, '╕',  182, '╤',  198, 'ф',  214, 'ж',  230, 'Ф',  246, 'Ж',
	135, '┤',  151, 'ў',  167, '╖',  183, '╥',  199, 'г',  215, 'в',  231, 'Г',  247, 'В',
	136, '┬',  152, 'є',  168, '╗',  184, '╦',  200, 'х',  216, 'ь',  232, 'Х',  248, 'Ь',
	137, '┴',  153, 'Є',  169, '╘',  185, '╧',  201, 'и',  217, 'ы',  233, 'И',  249, 'Ы',
	138, '┼',  154, '№',  170, '╙',  186, '╨',  202, 'й',  218, 'з',  234, 'Й',  250, 'З',
	139, '▀',  155, 'ї',  171, '╚',  187, '╩',  203, 'к',  219, 'ш',  235, 'К',  251, 'Ш',
	140, '▄',  156, '°',  172, '╛',  188, '╪',  204, 'л',  220, 'э',  236, 'Л',  252, 'Э',
	141, '█',  157, '¤',  173, '╜',  189, '╫',  205, 'м',  221, 'щ',  237, 'М',  253, 'Щ',
	142, '▌',  158, '∙',  174, '╝',  190, '╬',  206, 'н',  222, 'ч',  238, 'Н',  254, 'Ч',
	143, '▐',  159, 'Ў',  175, '╞',  191, 255,  207, 'о',  223, 'ъ',  239, 'О',  255, 'Ъ'
    };

    /* символ 191 = (с)/255 в дос */
    map_xlate_characters( koi8_r_to_cp866, COUNTOF( koi8_r_to_cp866) );
}
#endif

#ifdef	CP866
static void map_iso_8859_5_to_cp866(void)
{
    static byte iso_8859_5_to_cp866[] = {
    160, 255, 176, 'А',  192, 'Р',  208, 'а',  224, 'р',
    161, 'Ё', 177, 'Б',  193, 'С',  209, 'б',  225, 'с',  241, 'ё',
              178, 'В',  194, 'Т',     210, 'в',  226, 'т',
              179, 'Г',  195, 'У',  211, 'г',  227, 'у',
    164, 'Є', 180, 'Д',  196, 'Ф',  212, 'д',  228, 'ф',  244, 'є',
              181, 'Е',  197, 'Х',  213, 'е',  229, 'х',
              182, 'Ж',  198, 'Ц',  214, 'ж',  230, 'ц',
    167, 'Ї', 183, 'З',  199, 'Ч',  215, 'з',  231, 'ч',  247, 'ї',
              184, 'И',  200, 'Ш',  216, 'и',  232, 'ш',
              185, 'Й',  201, 'Щ',  217, 'й',  233, 'щ',
              186, 'К',  202, 'Ъ',  218, 'к',  234, 'ъ',
              187, 'Л',  203, 'Ы',  219, 'л',  235, 'ы',
              188, 'М',  204, 'Ь',  220, 'м',  236, 'ь',
    173, '─', 189, 'Н',  205, 'Э',  221, 'н',  237, 'э',
    174, 'Ў', 190, 'О',  206, 'Ю',  222, 'о',  238, 'ю',  254, 'ў',
              191, 'П',  207, 'Я',  223, 'п',  239, 'я'
    };

    /* символ 160 = (с)/255 в дос */
    map_xlate_characters( iso_8859_5_to_cp866, COUNTOF(iso_8859_5_to_cp866) );
}
#endif

#ifdef	CP866
static void map_cp866(void)
{
}
#endif

static void map_windows_1252(void)
{
    /* Not yet implemented */
}

static void map_windows_1256(void)
{
    /* Not yet implemented */
}

static void map_chinese(void)
{
    /* Not yet implemented */
}

static void map_korean(void)
{
    /* Not yet implemented */
}

static void map_japanese(void)
{
    /* Not yet implemented */
}

typedef void (*charset_fcn) (void);

typedef struct charset_def {
    const char *name;
    charset_fcn func;
    bool allow_nonascii_replacement;
} charset_def_t;

#define	T	true
#define	F	false

static charset_def_t charsets[] = {
    { "default",	&map_default,	   T },
    { "us-ascii",	&map_us_ascii,	   T },
#ifdef	CP866
    { "cp866",		&map_windows_1251_to_cp866, F },/* selected by 'charset_default=cp866' in bogofilter.cf */
#endif
    { "iso8859-1",	&map_iso_8859_1,   T },		/* ISOIEC 8859-1:1998 Latin Alphabet No. 1	*/
    /* tests/t.systest.d/inputs/spam.mbx is iso-8859-1 and contains 8-bit characters - " УYour AccountФ " */
    { "iso8859-2",	&map_iso_8859_2,   F },		/* ISOIEC 8859-2:1999 Latin Alphabet No. 2	*/
    { "iso8859-3",	&map_iso_8859_3,   F },		/* ISOIEC 8859-3:1999 Latin Alphabet No. 3	*/
    { "iso8859-4",	&map_iso_8859_4,   F },		/* ISOIEC 8859-4:1998 Latin Alphabet No. 4	*/
#ifndef	CP866
    { "iso8859-5",	&map_iso_8859_5,   F },		/* ISOIEC 8859-5:1999 LatinCyrillic Alphabet	*/
#else
    { "iso8859-5",	&map_iso_8859_5_to_cp866, F },	/* ISOIEC 8859-5:1999 LatinCyrillic Alphabet	*/
#endif
    { "iso8859-6",	&map_iso_8859_6,   F },		/* ISOIEC 8859-6:1999 LatinArabic Alphabet	*/
    { "iso8859-7",	&map_iso_8859_7,   F },		/* ISO	  8859-7:1987 LatinGreek Alphabet	*/
    { "iso8859-8",	&map_iso_8859_8,   F },		/* ISOIEC 8859-8:1999 LatinHebrew Alphabet	*/
    { "iso8859-9",	&map_iso_8859_9,   F },		/* ISOIEC 8859-9:1999 Latin Alphabet No. 5	*/
    { "iso8859-10",	&map_iso_8859_10,  F },		/* ISOIEC 8859-10:1998 Latin Alphabet No. 6	*/
    { "iso8859-13",	&map_iso_8859_13,  F },		/* ISOIEC 8859-13:1998 Latin Alphabet No. 7 (Baltic Rim)*/
    { "iso8859-14",	&map_iso_8859_14,  F },		/* ISOIEC 8859-14:1998 Latin Alphabet No. 8 (Celtic)	*/
    { "iso8859-15",	&map_iso_8859_15,  F },		/* ISOIEC 8859-15:1999 Latin Alphabet No. 9		*/
#ifndef	CP866
    { "windows-1251",	&map_windows_1251_to_koi8r, F },
#else
    { "cp866",		&map_cp866, F },
    { "koi8-r",		&map_koi8_r_to_cp866, F },
    { "windows-1251",	&map_windows_1251_to_cp866, F },
#endif
    { "windows-1252",	&map_windows_1252, T },
    { "windows-1256",	&map_windows_1256, T },
    { "utf-8",		&map_unicode,	   T },
    { "iso2022-jp",	&map_japanese,	   T },		/* rfc-1468 - japanese */
    { "euc-kr",		&map_korean,	   T },		/* extended unix code for korean */
    { "iso2022-kr",	&map_korean,	   T },		/* korean standard code (7-bit)*/
    { "ks-c-5601-1987",	&map_korean,	   T },		/* korean standard (default) */
    { "big5",		&map_chinese,	   T },
    { "csbig5",		&map_chinese,	   T },
    { "gb2312",		&map_chinese,	   T },
    { "csgb2312",	&map_chinese,	   T },
};

#define	MAX_LEN 64

void init_charset_table_orig(const char *charset_name)
{
    uint idx;
    bool found = false;
    for (idx = 0; idx < COUNTOF(charsets); idx += 1)
    {
	charset_def_t *charset = &charsets[idx];
	if (strcasecmp(charset->name, charset_name) == 0)
	{
	    if (DEBUG_CONFIG(1))
		fprintf(dbgout, " ... found.\n");
	    map_default();	/* Setup the table defaults. */
	    (*charset->func)();
	    found = true;
	    if (replace_nonascii_characters &&
		charset->allow_nonascii_replacement)
		map_nonascii_characters();
	    break;
	}
    }

    if ( !found ) {
	map_default();
	if (DEBUG_CONFIG(0))
	    fprintf(dbgout, "Unknown charset '%s';  using default.\n", charset_name );
    }

    return;
}

#ifdef	CP866
int  decode_and_htmlUNICODE_to_cp866(byte *buf, int len)
{
    int i,j, j1, l;
    bool is = false;
    byte code = 0;
    byte  *pbuf, str[40];
    int l1;
    const int max_seq_length = 8; /* &#1234; &yacute; */
    l1 = 0;
    pbuf = buf;
    for (i=0;i < len;i++)
    {
	pbuf[l1] = charset_table[buf[i]];  /* should be decode table to the same cp as UnicodeTable */
	l1++;
	if (buf[i] != '&') 	/* check for &-sequences */
	    continue;
	if (buf[i+1] != '#')	/*  "&nbsp;" ? */
	{
	    is = false;
	    for (j=0, l=1; i+l<len; l++,j++)
	    {
		if (l > max_seq_length) break;
		str[j] = buf[l+i];
		if (buf[l+i] == ';' )
		{
		    str[j] = 0;
		    for (j1 = 0; html_code[j1].sht; j1++)
		    {
			if (!strncasecmp(str, html_code[j1].sht, strlen(html_code[j1].sht)))
			{
			    is = true;
			    code = html_code[j1].cht;
			}
		    }
		    break;
		}
   
	    }
	    if (is)
	    { 
		pbuf[l1-1] = code;
		i += l;
	    }
	    continue;
	}
	for (j=0, l=2; i+l<len; l++,j++)  /* &#1055; ? */
	{
	    if (l > max_seq_length) break;
	    str[j] = buf[l+i];
	    if (buf[l+i] == ';' )
	    {
		is = true;
		str[j] = 0;
		code = atoi(str);
		break;
	    }
	}
	if (is)
	{
	    is = false;

	    for (j1 = 0; j1 < 256; j1++)
	    {
		if (code == UnicodeTable[j1])
		{ 
		    is = true;
		    code = j1;
		    break;
		}
	    }
	    if (is)
	    {

		pbuf[l1-1] = code;
		i += l;
	    }
	}
    }
    pbuf[l1] = 0;
    return l1;
}
#endif
