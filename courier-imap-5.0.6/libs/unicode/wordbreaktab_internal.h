/*
** Copyright 2011 Double Precision, Inc.
** See COPYING for distribution information.
**
*/

#ifndef wordbreaktab_internal_h
#define wordbreaktab_internal_h

#define UNICODE_WB_ALetter	0x00
#define UNICODE_WB_Numeric	0x01
#define UNICODE_WB_MidLetter	0x02
#define UNICODE_WB_MidNum	0x03
#define UNICODE_WB_MidNumLet	0x04
#define UNICODE_WB_ExtendNumLet	0x05

#define UNICODE_WB_CR		0x06
#define UNICODE_WB_LF		0x07
#define UNICODE_WB_Newline	0x08
#define UNICODE_WB_Extend	0x09
#define UNICODE_WB_Format	0x0A
#define UNICODE_WB_Katakana	0x0B

#define UNICODE_WB_Single_Quote	0x0C
#define UNICODE_WB_Double_Quote	0x0D
#define UNICODE_WB_Hebrew_Letter 0x0E
#define UNICODE_WB_Regional_Indicator 0x0F
#define UNICODE_WB_OTHER	0xFF
#endif
