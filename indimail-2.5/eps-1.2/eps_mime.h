#ifndef _EPS_MIME_H_
#define _EPS_MIME_H_

struct mime_t
{
	char           *filename, *orig, *boundary, depth;
	int             content_type, encoding, disposition;
};

struct mime_t  *mime_new_instance(void);
int             mime_header(struct eps_t *, struct mime_t *, struct header_t *h);
void            mime_content_type(struct eps_t *, struct header_t *, void *);
void            mime_transfer_encoding(struct eps_t *, struct header_t *, void *);
void            mime_content_disposition(struct eps_t *, struct header_t *, void *);
void            mime_kill(struct mime_t *);
int             mime_init_stream(struct eps_t *);
struct header_t *mime_next_header(struct eps_t *);
unsigned char  *mime_next_line(struct eps_t *);

#endif
