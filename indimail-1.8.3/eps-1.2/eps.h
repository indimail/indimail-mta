#ifndef _EPS_H_
#define _EPS_H_

struct eps_t
{
	int             interface, content_type;

	struct unroll_t *u;
	struct header_t *h;
	struct boundary_t *b;
	struct mime_t  *m;
};

#include "eps_line.h"
#include "eps_buffer.h"
#include "eps_unroll.h"
#include "rfc2822.h"
#include "eps_content.h"
#include "eps_interface.h"
#include "eps_int_stream.h"
#include "eps_misc.h"
#include "eps_header.h"
#include "eps_email.h"
#include "eps_boundary.h"
#include "eps_mime.h"
#include "eps_address.h"
#include "eps_base64.h"
#include "eps_roll.h"
#include "eps_int_buffer.h"

struct eps_t   *eps_alloc(void);
struct eps_t   *eps_begin(int, void *);
void            eps_restart(struct eps_t *, void *);
void            eps_end(struct eps_t *);
struct header_t *eps_next_header(struct eps_t *);
unsigned char  *eps_next_line(struct eps_t *);
void            eps_header_free(struct eps_t *);
void            eps_end(struct eps_t *);
int             eps_is_eof(struct eps_t *);

#endif
