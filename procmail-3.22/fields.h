/*
 * $Id: fields.h,v 1.8 2000/09/28 01:23:20 guenther Exp $
 */

struct field   *findf P((const struct field * const p, struct field ** ah));
struct field  **addfield Q((struct field ** pointer, const char *const text, const size_t totlen));
struct field   *delfield P((struct field ** pointer));
void cleanheader P((void));
void clear_uhead P((struct field * hdr));
void concatenate P((struct field * const fldp));
void flushfield P((struct field ** pointer));
void dispfield  P((const struct field * p));
void addbuf     P((void));
int readhead    P((void));
