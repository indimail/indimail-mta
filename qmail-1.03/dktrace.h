/*
 * $Log: dktrace.h,v $
 * Revision 1.1  2009-03-14 09:01:16+05:30  Cprogrammer
 * Initial revision
 *
 */
/*
 * $Id: dktrace.h,v 1.1 2009-03-14 09:01:16+05:30 Cprogrammer Exp mbhangui $
 */

#ifndef _DK_TRACE_H
#define _DK_TRACE_H

/*- #define dkt_init(s) memset(s, 0,sizeof(DK_TRACE)) */
#define dkt_init(s) bzero(s, sizeof(DK_TRACE))
typedef struct {
	int             ccounts_h[256];
	int             ccounts_H[256];
	int             ccounts_b[256];
	int             ccounts_B[256];
} DK_TRACE;

typedef enum { DKT_RAW_HEADER = 'h', DKT_CANON_HEADER = 'H',
	DKT_RAW_BODY = 'b', DKT_CANON_BODY = 'B'
} DK_TRACE_TYPE;

extern void     dkt_add(DK_TRACE * dkp, DK_TRACE_TYPE type, const unsigned char *data, int dataLength);
extern int      dkt_diff(DK_TRACE * dka, DK_TRACE * dkb, DK_TRACE_TYPE type, DK_TRACE * table);
extern void     dkt_quickadd(DK_TRACE * dkp, DK_TRACE_TYPE type, int index, int count);
extern int      dkt_getcount(DK_TRACE * dkp, DK_TRACE_TYPE type, int index, int count);
extern int      dkt_generate(DK_TRACE * dkp, DK_TRACE_TYPE type, char *buffer, int maxBufferSize);
extern int      dkt_hdrtotrace(char *ptr, DK_TRACE * store);

#endif
