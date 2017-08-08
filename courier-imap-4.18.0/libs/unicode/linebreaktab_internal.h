/*
** Copyright 2011 Double Precision, Inc.
** See COPYING for distribution information.
**
*/

#ifndef linebreaktab_internal_h
#define linebreaktab_internal_h

#define UNICODE_LB_BK	0x00
#define UNICODE_LB_CR	0x01
#define UNICODE_LB_LF	0x02
#define UNICODE_LB_CM	0x03
#define UNICODE_LB_NL	0x04
#define UNICODE_LB_SG	0x05
#define UNICODE_LB_WJ	0x06
#define UNICODE_LB_ZW	0x07
#define UNICODE_LB_GL	0x08
#define UNICODE_LB_SP	0x09
#define UNICODE_LB_B2	0x10
#define UNICODE_LB_BA	0x11
#define UNICODE_LB_BB	0x12
#define UNICODE_LB_HY	0x13
#define UNICODE_LB_CB	0x14
#define UNICODE_LB_CL	0x20
#define UNICODE_LB_CP	0x21
#define UNICODE_LB_EX	0x22
#define UNICODE_LB_IN	0x23
#define UNICODE_LB_NS	0x24
#define UNICODE_LB_OP	0x25
#define UNICODE_LB_QU	0x26
#define UNICODE_LB_IS	0x30
#define UNICODE_LB_NU	0x31
#define UNICODE_LB_PO	0x32
#define UNICODE_LB_PR	0x33
#define UNICODE_LB_SY	0x34
#define UNICODE_LB_AI	0x40
#define UNICODE_LB_AL	0x41
#define UNICODE_LB_H2	0x42
#define UNICODE_LB_H3	0x43
#define UNICODE_LB_HL	0x44
#define UNICODE_LB_ID	0x45
#define UNICODE_LB_JL	0x46
#define UNICODE_LB_JV	0x47
#define UNICODE_LB_JT	0x48
#define UNICODE_LB_RI   0x49
#define UNICODE_LB_SA	0x4A

extern int unicode_lb_lookup(char32_t ch);

#endif
