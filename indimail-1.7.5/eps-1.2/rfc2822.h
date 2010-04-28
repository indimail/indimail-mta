#ifndef _RFC822_H_
#define _RFC822_H_

#define MAX_LINE_LENGTH 1000

int             rfc2822_init(struct eps_t *);
int             rfc2822_is_rolled(unsigned char *);
int             rfc2822_unroll(struct eps_t *, unsigned char *);
unsigned char  *rfc2822_line_addr(struct eps_t *);
int             rfc2822_remove_crlf(unsigned char *);
int             rfc2822_is_wsp(unsigned char);
int             rfc2822_escaped(unsigned char);
unsigned char  *rfc2822_remove_comments(unsigned char *);
unsigned char  *rfc2822_next_token(unsigned char *, unsigned char, unsigned char *);
unsigned char  *rfc2822_convert_literals(unsigned char *);

#endif
