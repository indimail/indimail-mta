/*
** Copyright 2011 Double Precision, Inc.
** See COPYING for distribution information.
**
*/

#include	"unicode_config.h"
#include	"courier-unicode.h"
#include	<unistd.h>
#include	<stdint.h>
#include	<stdlib.h>

#define UNICODE_GRAPHEMEBREAK_ANY		0x00
#define UNICODE_GRAPHEMEBREAK_CR		0x01
#define UNICODE_GRAPHEMEBREAK_LF		0x02
#define UNICODE_GRAPHEMEBREAK_Control		0x03
#define UNICODE_GRAPHEMEBREAK_Extend		0x04
#define UNICODE_GRAPHEMEBREAK_Prepend		0x05
#define UNICODE_GRAPHEMEBREAK_SpacingMark	0x06
#define UNICODE_GRAPHEMEBREAK_L			0x07
#define UNICODE_GRAPHEMEBREAK_V			0x08
#define UNICODE_GRAPHEMEBREAK_T			0x09
#define UNICODE_GRAPHEMEBREAK_LV		0x0A
#define UNICODE_GRAPHEMEBREAK_LVT		0x0B
#define UNICODE_GRAPHEMEBREAK_Regional_Indicator 0x0C

#include "graphemebreaktab.h"

int unicode_grapheme_break(char32_t a, char32_t b)
{
	uint8_t ac=unicode_tab_lookup(a, unicode_indextab,
			 sizeof(unicode_indextab)/sizeof(unicode_indextab[0]),
			 unicode_rangetab,
			 unicode_classtab,
			 UNICODE_GRAPHEMEBREAK_ANY),
		bc=unicode_tab_lookup(b, unicode_indextab,
			 sizeof(unicode_indextab)/sizeof(unicode_indextab[0]),
			 unicode_rangetab,
			 unicode_classtab,
			 UNICODE_GRAPHEMEBREAK_ANY);

	/* GB1 and GB2 are implied */

	if (ac == UNICODE_GRAPHEMEBREAK_CR && bc == UNICODE_GRAPHEMEBREAK_LF)
		return 0; /* GB3 */


	switch (ac) {
	case UNICODE_GRAPHEMEBREAK_CR:
	case UNICODE_GRAPHEMEBREAK_LF:
	case UNICODE_GRAPHEMEBREAK_Control:
		return 1; /* GB4 */
	default:
		break;
	}

	switch (bc) {
	case UNICODE_GRAPHEMEBREAK_CR:
	case UNICODE_GRAPHEMEBREAK_LF:
	case UNICODE_GRAPHEMEBREAK_Control:
		return 1; /* GB5 */
	default:
		break;
	}

	if (ac == UNICODE_GRAPHEMEBREAK_L)
		switch (bc) {
		case UNICODE_GRAPHEMEBREAK_L:
		case UNICODE_GRAPHEMEBREAK_V:
		case UNICODE_GRAPHEMEBREAK_LV:
		case UNICODE_GRAPHEMEBREAK_LVT:
			return 0; /* GB6 */
		}

	if ((ac == UNICODE_GRAPHEMEBREAK_LV ||
	     ac == UNICODE_GRAPHEMEBREAK_V) &&
	    (bc == UNICODE_GRAPHEMEBREAK_V ||
	     bc == UNICODE_GRAPHEMEBREAK_T))
		return 0; /* GB7 */

	if ((ac == UNICODE_GRAPHEMEBREAK_LVT ||
	     ac == UNICODE_GRAPHEMEBREAK_T) &&
	    bc == UNICODE_GRAPHEMEBREAK_T)
		return 0; /* GB8 */

	if (ac == UNICODE_GRAPHEMEBREAK_Regional_Indicator &&
	    bc == UNICODE_GRAPHEMEBREAK_Regional_Indicator)
		return 0; /* GB8a */

	if (bc == UNICODE_GRAPHEMEBREAK_Extend)
		return 0; /* GB9 */

	if (bc == UNICODE_GRAPHEMEBREAK_SpacingMark)
		return 0; /* GB9a */

	if (ac == UNICODE_GRAPHEMEBREAK_Prepend)
		return 0; /* GB9b */

	return 1; /* GB10 */
}
