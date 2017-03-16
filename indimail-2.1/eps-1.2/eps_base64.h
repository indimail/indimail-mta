#ifndef _BASE64_H_
#define _BASE64_H_

struct base64_t
{
	char            inalphabet[256], decoder[256];
};

void            base64_init(struct base64_t *);
int             base64_decode(struct base64_t *, struct line_t *, char *);
int             base64_encode(int, struct line_t *);

#endif
