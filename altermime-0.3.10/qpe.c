#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>

#include "logger.h"
#include "qpe.h"

#define CRLF "\r\n"

#define QPD if (qpe_debug > 0)

int qpe_debug = 0;

int qp_encode_set_debug( int level ) 
{
	qpe_debug = level;

	return 0;
}


int qp_encode( char *out, size_t out_size, char *in, size_t in_size )
{
	int result = 0;
	size_t out_remaining;
	char *linestart, *lineend, *p, *op;
	char paragraph[100], *pp;
	int pp_remaining = 100;
	char *input_data_limit = in +in_size;
	int current_line_length = 0;


	linestart = NULL;
	lineend = in;

	/** Set the output buffer variables **/
	op = out;
	out_remaining = out_size;

	do {
		char charout[4];
		int charout_size=0;

		if (lineend != '\0') {
			if (linestart == NULL) {
				linestart = in;
			} else {
				linestart = lineend;
			}

			lineend = strstr(linestart, CRLF);
			if (lineend == NULL) {
				QPD fprintf(stdout,"No CRLF found, setting line-end to end of the input\n");
			  	lineend = in +in_size;
			} else {
				QPD {
					*lineend = '\0';
					fprintf(stdout,"INPUT STRING: '%s'\n", linestart);
					*lineend = '\r';
				}
				lineend += strlen(CRLF);
			}

		}


		/** Initialize the paragraph **/
		paragraph[0] = '\0';
		pp = paragraph;
		pp_remaining = sizeof(paragraph);
		current_line_length = 0;

		QPD fprintf(stdout,"Starting new line of encoding...\n");

		/** Set p to point to the start of the new line that we have to encode **/
		p = linestart;
		while ((p < lineend)) {

			if (*p < 32 || *p == 61 || *p > 126) {
				/** encode as hex **/
				snprintf( charout, sizeof(charout), "=%02X", (unsigned char)*p); // 20070212-2238 Fix supplied by Julian Schneider
				charout_size = 3;
			} else {
				/** encode verbatim **/
				snprintf( charout, sizeof(charout), "%c", *p);
				charout_size = 1;
			}

			if (current_line_length +charout_size >= 79) { // Was 76, updated to fix Outlook problems
				//snprintf(op, out_remaining, "%s=\r\n", paragraph);
				snprintf(op, out_remaining, "%s=\r\n", paragraph);
				op+= strlen(paragraph);// +3; /** jump the output + =\r\n **/
				out_remaining-= (strlen(paragraph)); // Was +3, updated to fix Outlook problems

				QPD fprintf(stdout, "Soft break (%d + %d > 76 char) for '%s'\n", current_line_length, charout_size, paragraph);
				
				/** reinitialize the paragraph **/
				paragraph[0] = '\0';
				pp_remaining = sizeof(paragraph);
				pp = paragraph;
				current_line_length=-1;

			}

			snprintf(pp, pp_remaining, "%s", charout);
			QPD fprintf(stdout,"charout='%s', size=%d, pp_remain=%d result='%s'\n", charout, charout_size, pp_remaining, paragraph);
			pp += charout_size;
			pp_remaining -= charout_size;
			p++;
			current_line_length += charout_size; // 2007030901 - Patch provided by Yossi Gottlieb
		} /** for each char in the line to be converted **/


		QPD fprintf(stdout,"Adding paragraph '%s' to output\n", paragraph );
		
		snprintf(op, out_remaining, "%s\r\n", paragraph);
		op += (strlen(paragraph) +2);
		out_remaining -= (strlen(paragraph) +2);

	} while ((lineend < input_data_limit)&&(*lineend != '\0')); /** for each line **/

	return result;
}


int qp_encode_from_file( char *fname )
{
	size_t bc;
	struct stat st;
	int stat_result;
	char *in_buffer;
	char *out_buffer;
	int in_size, out_size;
	FILE *f;

	stat_result = stat( fname, &st );
	if (stat_result != 0){
		QPD fprintf(stderr, "Cannot locate file '%s' for loading and QP encoding (%s)\n", fname, strerror(errno));
		return -1;
	}


	in_size = st.st_size;
	out_size = in_size *3;
	in_buffer = malloc( sizeof(char) *in_size +1);
	if (in_buffer == NULL) {
		QPD fprintf(stdout,"Error allocating %d bytes for input buffer\n", in_size);
		return -1;
	}

	out_buffer = malloc( sizeof(char) *out_size *3 +1);
	if (in_buffer == NULL) {
		QPD fprintf(stdout,"Error allocating %d bytes for output buffer\n", out_size);
		return -1;
	}


	f = fopen( fname, "r" );
	bc = fread( in_buffer, 1, in_size, f );
	if (bc != in_size) LOGGER_log("%s:%d:qp_encode_from_file:ERROR: Read %d bytes but requested %d", FL, bc, in_size);
	fclose(f);

	/** zero terminate the buffer -- uhuh, if you forget that you'll wonder why
	  ** we segfault ;)  **/
	*(in_buffer +in_size) = '\0';

	QPD fprintf(stdout,"file %s is loaded, size = %d\n", fname, in_size);

	qp_encode( out_buffer, out_size, in_buffer, in_size );

	fprintf( stdout, "%s", out_buffer );

	free(in_buffer);
	free(out_buffer);

	return 0;
}





