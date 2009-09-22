#ifndef _QPENCODE_
#define _QPENCODE_

int qp_encode_set_debug(int level);
int qp_encode( char *out, size_t out_size, char *in, size_t in_size );
int qp_encode_from_file( char *fname );

#endif
