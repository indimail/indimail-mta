#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <syslog.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <regex.h>

#include "ffget.h"
#include "pldstr.h"
#include "strstack.h"
#include "boundary-stack.h"
#include "MIME_headers.h"
#include "filename-filters.h"
#include "mime_alter.h"
#include "qpe.h"
#include "logger.h"


#define AM_1K_BUFFER_SIZE 1024
#define AM_DISCLAIMER_FILENAME_LENGTH_MAX 256;

#define AM_DNORMAL (glb.debug > 0)
#define AM_VERBOSE (glb.verbose > 0)

#define DAM if (glb.debug > 0)
#define VAM if (glb.verbose > 0)

/* Globals */
unsigned int altermime_status_flags; // Status flags


unsigned char AM_encode64[64]={
	65,   66,   67,   68,   69,   70,   71,   72,   73,   74,   75,   76,   77,   78,   79,   80,\
		81,   82,   83,   84,   85,   86,   87,   88,   89,   90,   97,   98,   99,  100,  101,  102,\
		103,  104,  105,  106,  107,  108,  109,  110,  111,  112,  113,  114,  115,  116,  117,  118,\
		119,  120,  121,  122,   48,   49,   50,   51,   52,   53,   54,   55,   56,   57,   43,   47 \
};

/* our base 64 decoder table */
static unsigned char b64[256]={
	128,  128,  128,  128,  128,  128,  128,  128,  128,  128,  128,  128,  128,  128,  128,  128,\
		128,  128,  128,  128,  128,  128,  128,  128,  128,  128,  128,  128,  128,  128,  128,  128,\
		128,  128,  128,  128,  128,  128,  128,  128,  128,  128,  128,   62,  128,  128,  128,   63,\
		52,   53,   54,   55,   56,   57,   58,   59,   60,   61,  128,  128,  128,    0,  128,  128,\
		128,    0,    1,    2,    3,    4,    5,    6,    7,    8,    9,   10,   11,   12,   13,   14,\
		15,   16,   17,   18,   19,   20,   21,   22,   23,   24,   25,  128,  128,  128,  128,  128,\
		128,   26,   27,   28,   29,   30,   31,   32,   33,   34,   35,   36,   37,   38,   39,   40,\
		41,   42,   43,   44,   45,   46,   47,   48,   49,   50,   51,  128,  128,  128,  128,  128,\
		128,  128,  128,  128,  128,  128,  128,  128,  128,  128,  128,  128,  128,  128,  128,  128,\
		128,  128,  128,  128,  128,  128,  128,  128,  128,  128,  128,  128,  128,  128,  128,  128,\
		128,  128,  128,  128,  128,  128,  128,  128,  128,  128,  128,  128,  128,  128,  128,  128,\
		128,  128,  128,  128,  128,  128,  128,  128,  128,  128,  128,  128,  128,  128,  128,  128,\
		128,  128,  128,  128,  128,  128,  128,  128,  128,  128,  128,  128,  128,  128,  128,  128,\
		128,  128,  128,  128,  128,  128,  128,  128,  128,  128,  128,  128,  128,  128,  128,  128,\
		128,  128,  128,  128,  128,  128,  128,  128,  128,  128,  128,  128,  128,  128,  128,  128,\
		128,  128,  128,  128,  128,  128,  128,  128,  128,  128,  128,  128,  128,  128,  128,  128 \
};


static struct AM_globals glb;




/*-----------------------------------------------------------------\
  Function Name	: AM_version
  Returns Type	: int
  ----Parameter List
  1. void , 
  ------------------
  Exit Codes	: 
  Side Effects	: 
  --------------------------------------------------------------------
Comments:

--------------------------------------------------------------------
Changes:

\------------------------------------------------------------------*/
int AM_version( void )
{
	fprintf(stderr,"alterMIME: %s", LIBAM_VERSION);

	return 0;
}


/*-----------------------------------------------------------------\
  Function Name	: AM_init
  Returns Type	: int
  ----Parameter List
  1. void , 
  ------------------
  Exit Codes	: 
  Side Effects	: 
  --------------------------------------------------------------------
Comments:

--------------------------------------------------------------------
Changes:

\------------------------------------------------------------------*/
int AM_init( void )
{
	glb.debug=0;				/* Low level debugging */
	glb.verbose=0;				/* do we talk as we walk */
	glb.paranoid=1;				/* set paranoid to yes! */
	glb.HTML_too=1;				/* Add footer to the HTML email too */
	glb.multipart_insert=0;	/* do not insert into multipart attachments */
	glb.nullify_all=0;			/* Remove ALL filename'd attachments */
	glb.alter_signed=0;		/* Do we alter signed emails ? - default is NO */
	glb.header_long_search=1;	/* Search into email bodies for more attachments */
	glb.force_into_b64=0;		/* By default don't insert into B64 encoded bodies */
	glb.force_for_bad_html=0;	/* By default don't try append into broken HTML */

	snprintf(glb.ldelimeter,sizeof(glb.ldelimeter),"\r\n");

	glb.disclaimer_plain=NULL;
	glb.disclaimer_plain_type=AM_DISCLAIMER_TYPE_NONE;

	glb.disclaimer_HTML=NULL;
	glb.disclaimer_HTML_type=AM_DISCLAIMER_TYPE_NONE;

	glb.disclaimer_b64=NULL;
	glb.disclaimer_b64_type=AM_DISCLAIMER_TYPE_NONE;

#ifdef ALTERMIME_PRETEXT
	glb.pretext_plain=NULL;
	glb.pretext_plain_type=AM_PRETEXT_TYPE_NONE;
	glb.pretext_HTML=NULL;
	glb.pretext_HTML_type=AM_PRETEXT_TYPE_NONE;
	glb.pretext_insert = 0;
#endif

	glb.headerbuffermax=0;

	return 0;
}



/*-----------------------------------------------------------------\
  Function Name	: AM_done
  Returns Type	: int
  ----Parameter List
  1. void , 
  ------------------
  Exit Codes	: 
  Side Effects	: 
  --------------------------------------------------------------------
Comments:

--------------------------------------------------------------------
Changes:

\------------------------------------------------------------------*/
int AM_done( void )
{
	if (glb.disclaimer_plain != NULL) free(glb.disclaimer_plain);
	if (glb.disclaimer_HTML != NULL) free(glb.disclaimer_HTML);
	if (glb.disclaimer_b64 != NULL) free(glb.disclaimer_b64);

	return 0;
}


/*-----------------------------------------------------------------\
  Function Name	: AM_set_force_for_bad_html
  Returns Type	: int
  ----Parameter List
  1. int level , 
  ------------------
  Exit Codes	: 
  Side Effects	: 
  --------------------------------------------------------------------
Comments:

--------------------------------------------------------------------
Changes:

\------------------------------------------------------------------*/
int AM_set_force_for_bad_html( int level )
{
	glb.force_for_bad_html = level;
	return glb.force_for_bad_html;
}


/*-----------------------------------------------------------------\
  Date Code:	: 20070127-140050
  Function Name	: AM_set_force_into_b64
  Returns Type	: int
  ----Parameter List
  1. int level , 
  ------------------
  Exit Codes	: 
  Side Effects	: 
  --------------------------------------------------------------------
Comments:

Set to non-zero (1) if you want alterMIME to attempt to 
insert disclaimers into BASE64 encoded texts.  This isn't
on by default because there's the possibility of malforming
legitimate attachments.

--------------------------------------------------------------------
Changes:

\------------------------------------------------------------------*/
int AM_set_force_into_b64( int level )
{
	glb.force_into_b64 = level;
	return glb.force_into_b64;
}

/*-----------------------------------------------------------------\
  Function Name	: AM_set_pretext_insert
  Returns Type	: int
  ----Parameter List
  1. int level , 
  ------------------
  Exit Codes	: 
  Side Effects	: 
  --------------------------------------------------------------------
Comments:

--------------------------------------------------------------------
Changes:

\------------------------------------------------------------------*/
#ifdef ALTERMIME_PRETEXT
int AM_set_pretext_insert( int level )
{
	glb.pretext_insert = level;

	return glb.pretext_insert;
}
#endif


/*-----------------------------------------------------------------\
  Function Name	: AM_set_debug
  Returns Type	: int
  ----Parameter List
  1. int level , 
  ------------------
  Exit Codes	: 
  Side Effects	: 
  --------------------------------------------------------------------
Comments:

--------------------------------------------------------------------
Changes:

\------------------------------------------------------------------*/
int AM_set_debug( int level )
{
	glb.debug = level;
	AM_set_verbose( level );
	MIMEH_set_debug(level);

	return level;

}


/*-----------------------------------------------------------------\
  Function Name	: AM_hbuffer_reset
  Returns Type	: int
  ----Parameter List
  1. void , 
  ------------------
  Exit Codes	: 
  Side Effects	: 
  --------------------------------------------------------------------
Comments:

--------------------------------------------------------------------
Changes:

\------------------------------------------------------------------*/
int AM_hbuffer_reset( void )
{
	int i;

	for (i = 0; i < glb.headerbuffermax; i++)
	{
		if (glb.headerbuffer[i]) free(glb.headerbuffer[i]);
	}
	glb.headerbuffermax=0;
	return 0;
}


/*-----------------------------------------------------------------\
  Function Name	: *AM_hbuffer_add
  Returns Type	: char
  ----Parameter List
  1. char *headerline, 
  2.  FILE *f , 
  ------------------
  Exit Codes	: 
  Side Effects	: 
  --------------------------------------------------------------------
Comments:

--------------------------------------------------------------------
Changes:

\------------------------------------------------------------------*/
char *AM_hbuffer_add( char *headerline, FILE *f )
{
	int i;

	if (glb.headerbuffermax == AM_HEADERBUFFER_MAX)
	{
		if (glb.headerbuffer[0])
		{
			fprintf(f,"%s",glb.headerbuffer[0]);
			free(glb.headerbuffer[0]);
		}
		for (i = 0; i < (AM_HEADERBUFFER_MAX-1); i++)
		{
			glb.headerbuffer[i] = glb.headerbuffer[i+1];
		}
		glb.headerbuffermax = AM_HEADERBUFFER_MAX -1;
	}

	glb.headerbuffer[glb.headerbuffermax] = malloc(sizeof(char) *(AM_HEADERBUFFER_ITEM_SIZE +1));

	if (!glb.headerbuffer[glb.headerbuffermax])
	{
		LOGGER_log("alterMIME: AM_hbuffer_add: Error: cannot allocate %d bytes for new header", AM_HEADERBUFFER_ITEM_SIZE +1);
		return NULL;
	}

	strncpy(glb.headerbuffer[glb.headerbuffermax], headerline, AM_HEADERBUFFER_ITEM_SIZE);
	glb.headerbuffermax++;

	return glb.headerbuffer[glb.headerbuffermax-1];
}


/*------------------------------------------------------------------------
Procedure:     AM_hbuffer_getmax ID:1
Purpose:
Input:
Output:
Errors:
------------------------------------------------------------------------*/
int AM_hbuffer_getmax( void )
{
	return glb.headerbuffermax;
}




/*------------------------------------------------------------------------
Procedure:     AM_hbuffer_getline ID:1
Purpose:
Input:
Output:
Errors:
------------------------------------------------------------------------*/
char *AM_hbuffer_getline( int index )
{
	if ((index >=0)&&(index < glb.headerbuffermax))
		return glb.headerbuffer[index];
	else return NULL;
}



/*------------------------------------------------------------------------
Procedure:     AM_set_glb.paranoid ID:1
Purpose:
Input:
Output:
Errors:
------------------------------------------------------------------------*/
int AM_set_paranoid( int level )
{
	glb.paranoid = level;
	return glb.paranoid;
}




/*-----------------------------------------------------------------\
  Function Name	: AM_set_header_long_search
  Returns Type	: int
  ----Parameter List
  1. int level , 
  ------------------
  Exit Codes	: 
  Side Effects	: 
  --------------------------------------------------------------------
Comments:

--------------------------------------------------------------------
Changes:

\------------------------------------------------------------------*/
int AM_set_header_long_search( int level )
{
	glb.header_long_search = level;
	return glb.header_long_search;
}

/*------------------------------------------------------------------------
Procedure:     AM_set_glb.verbose ID:1
Purpose:
Input:
Output:
Errors:
------------------------------------------------------------------------*/
int AM_set_verbose( int level )
{
	glb.verbose = level;
	return glb.verbose;
}




/*------------------------------------------------------------------------
Procedure:     AM_set_HTMLtoo ID:1
Purpose:
Input:
Output:
Errors:
------------------------------------------------------------------------*/
int AM_set_HTMLtoo( int level )
{
	glb.HTML_too = level;
	return glb.HTML_too;
}


/*------------------------------------------------------------------------
Procedure:     AM_set_multipart_insert ID:1
Purpose:       Set the multipart-insertion of dislcaimers to level.  Multipart disclaimers are not on by default
because they would imply inserting disclaimers into previously forwarded messages.
Input:         level
Output:
Errors:
------------------------------------------------------------------------*/
int AM_set_multipart_insert( int level )
{
	glb.multipart_insert = level;
	return glb.multipart_insert;
}



/*-----------------------------------------------------------------\
  Function Name	: *AM_set_disclaimer_plain
  Returns Type	: char
  ----Parameter List
  1. char *filename, 
  2.  int disclaimer_type , 
  ------------------
  Exit Codes	: 
  Side Effects	: 
  --------------------------------------------------------------------
Comments:
Sets the disclaimer text and text-type for Plain-text disclaimers.
What we mean here is, if the disclaimer_type is 'TEXT' then we
don't interpret the contents of .disclaimer_plain as a filename
rather, we take it literally as the text to use for our disclaimer

--------------------------------------------------------------------
Changes:

\------------------------------------------------------------------*/
char *AM_set_disclaimer_plain( char *filename, int disclaimer_type )
{
	glb.disclaimer_plain = strdup( filename );
	glb.disclaimer_plain_type = disclaimer_type;

	return glb.disclaimer_plain;
}



/*-----------------------------------------------------------------\
  Function Name	: *AM_set_disclaimer_HTML
  Returns Type	: char
  ----Parameter List
  1. char *filename, 
  2.  int disclaimer_type , 
  ------------------
  Exit Codes	: 
  Side Effects	: 
  --------------------------------------------------------------------
Comments:

--------------------------------------------------------------------
Changes:

\------------------------------------------------------------------*/
char *AM_set_disclaimer_HTML( char *filename, int disclaimer_type )
{
	glb.disclaimer_HTML = strdup( filename );
	glb.disclaimer_HTML_type = disclaimer_type;

	return glb.disclaimer_HTML;
}



/*-----------------------------------------------------------------\
  Date Code:	: 20081106-065138
  Function Name	: *AM_set_disclaimer_b64
  Returns Type	: char
  ----Parameter List
  1. char *filename, 
  2.  int disclaimer_type , 
  ------------------
  Exit Codes	: 
  Side Effects	: 
  --------------------------------------------------------------------
Comments:

--------------------------------------------------------------------
Changes:

\------------------------------------------------------------------*/
char *AM_set_disclaimer_b64( char *filename, int disclaimer_type )
{
	glb.disclaimer_b64 = strdup( filename );
	glb.disclaimer_b64_type = disclaimer_type;

	return glb.disclaimer_b64;
}

/*-----------------------------------------------------------------\
  Function Name	: *AM_set_pretext_plain
  Returns Type	: char
  ----Parameter List
  1. char *filename, 
  2.  int pretext_type , 
  ------------------
  Exit Codes	: 
  Side Effects	: 
  --------------------------------------------------------------------
Comments:

--------------------------------------------------------------------
Changes:

\------------------------------------------------------------------*/
#ifdef ALTERMIME_PRETEXT
int AM_set_pretext_plain( char *filename, int pretext_type )
{

	glb.pretext_plain = strdup( filename );
	glb.pretext_plain_type = pretext_type;

	return 0;
}
#endif




/*-----------------------------------------------------------------\
  Function Name	: *AM_set_pretext_HTML
  Returns Type	: char
  ----Parameter List
  1. char *filename, 
  2.  int pretext_type , 
  ------------------
  Exit Codes	: 
  Side Effects	: 
  --------------------------------------------------------------------
Comments:

--------------------------------------------------------------------
Changes:

\------------------------------------------------------------------*/
#ifdef ALTERMIME_PRETEXT
int AM_set_pretext_HTML( char *filename, int pretext_type )
{
	glb.pretext_HTML = strdup( filename );
	glb.pretext_HTML_type = pretext_type;

	return 0;
}
#endif

/*------------------------------------------------------------------------
Procedure:     AM_set_nullifyall ID:1
Purpose:
Input:
Output:
Errors:
------------------------------------------------------------------------*/
int AM_set_nullifyall( int level )
{
	glb.nullify_all = level;
	return glb.nullify_all;
}



/*------------------------------------------------------------------------
Procedure:     AM_set_altersigned ID:1
Purpose:       Turns on or off the option to alter signed email messages.
Default is to be -off-.
Input:
Output:
Errors:
------------------------------------------------------------------------*/
int AM_set_altersigned( int level )
{
	glb.alter_signed = level;
	return glb.alter_signed;
}




/*-----------------------------------------------------------------\
  Function Name	: AM_ntorn
  Returns Type	: int
  ----Parameter List
  1. char *in, 
  2.  FILE *out , 
  ------------------
  Exit Codes	: 
  Side Effects	: 
  --------------------------------------------------------------------
Comments:

Converts any singular occurances of \n into \r\n

--------------------------------------------------------------------
Changes:

\------------------------------------------------------------------*/
int AM_ntorn( char *in, FILE *out )
{
	char *p = in;
	char lastchar=' ';

	while (*p) 
	{
		if ((*p == '\n')&&(lastchar != '\r')) fprintf(out, "\r");
		fprintf(out, "%c", *p);
		lastchar = *p;
		p++;
	}

	return 0;
}

/*-----------------------------------------------------------------\
  Date Code:	: 20081106-145300
  Function Name	: AM_adapt_linebreak
  Returns Type	: int
  ----Parameter List
  1. char *in, 
  2.  FILE *out, 
  3.  char *lb , 
  ------------------
  Exit Codes	: 
  Side Effects	: 
  --------------------------------------------------------------------
Comments:

ASSUMES that the input buffer (*in) is PROPERLY formatted as
either \n or \r\n terminated text.  This will NOT work properly
if you feed it data with malformed linebreaks.

--------------------------------------------------------------------
Changes:

\------------------------------------------------------------------*/
char *AM_adapt_linebreak( char *in, char *lb ) {

	char safe[]="\r\n";
	char *newblock;


	if (in == NULL) return NULL;


	if ((lb==NULL)||(lb[0] == 0)) lb = safe;

	DAM LOGGER_log("%s:%d:AM_adapt_linebreak:DEBUG: Linebreak=%x:%x",FL,lb[0],lb[1]);

	if (lb[0] == '\r') {
		if (strchr(in,'\r')) {
			DAM LOGGER_log("%s:%d:AM_adapt_linebreak:DEBUG: email and disclaimer are CRLF, no change",FL);
			return in;
		} else {
			DAM LOGGER_log("%s:%d:AM_adapt_linebreak:DEBUG: chaging LF only to CRLF",FL);
			newblock = strdup(in);
			newblock = PLD_strreplace(&newblock, "\n", "\r\n", 0);
		}
	} else if (lb[0] == '\n') {
		if (!strchr(in,'\r')) {
			DAM LOGGER_log("%s:%d:AM_adapt_linebreak:DEBUG: email and disclaimer are LF only, no change",FL);
			return in;
		} else {
			DAM LOGGER_log("%s:%d:AM_adapt_linebreak:DEBUG: changing CRLF to LF",FL);
			newblock = strdup(in);
			newblock = PLD_strreplace(&newblock, "\r\n", "\n", 0);
		}
	}

	return newblock;
}


/*------------------------------------------------------------------------
Procedure:     AM_base64_encodef ID:1
Purpose:       encode to BASE64 an input stream to an output stream
Input:         FILE *fin: Input stream
FILE *fout: Output stream
Output:
Errors:
------------------------------------------------------------------------*/
int AM_base64_encodef( FILE *fin, FILE *fout )
{
	unsigned char inbuf[3];
	unsigned char outbuf[4];
	int cc;
	int byte_count;

	if (!fin)
	{
		LOGGER_log("AM_base64_encodef: Error: input file stream not open, please use AM_base64_encode(infile,outfile)");
		return -1;
	}

	if (!fout)
	{
		LOGGER_log("AM_base64_encodef: Error: output file stream not open, please use AM_base64_encode(infile,outfile)");
		return -1;
	}

	cc = 0;	/* Character Count = 0 */

	while ((byte_count = fread(inbuf, 1, 3, fin)))	/* while we get more than 0 bytes */
	{
		size_t bc;

		/* Split 3 bytes into 4 bytes by taking 6bit blocks out of the 24 bit (3x8bit)
		 * array */
		outbuf[0] = AM_encode64[((inbuf[0] & 0xFC) >> 2)];
		outbuf[1] = AM_encode64[((inbuf[0] & 0x03) << 4) | ((inbuf[1] & 0xF0) >> 4)];
		outbuf[2] = AM_encode64[((inbuf[1] & 0x0F) << 2) | ((inbuf[2] & 0xC0) >> 6)];
		outbuf[3] = AM_encode64[(inbuf[2] & 0x3F)];

		/* if we didn't get a full 3 bytes from the file read, then we need to then
		 * pad the output with '=' (base64 padding char) */
		if ( byte_count < 3 )
		{

			if ( byte_count == 2 ) outbuf[3] = '=';
			if ( byte_count == 1 ) outbuf[3] = outbuf[2] = '=';

			bc = fwrite(outbuf, 1, 4, fout); 	/* write out the buffer */
			if (bc != 4) LOGGER_log("%s:%d:AM_base64_encode:ERROR: Wrote %d bytes rather than %d to output", FL, bc);

			bc= fwrite( "\n", 1, 1, fout ); 	/* as this is the last line, put a trailing \n */
			if (bc != 1) LOGGER_log("%s:%d:AM_base64_encode:ERROR: Wrote %d bytes rather than %d to output", FL, bc);
			break; 								/* exit out of the while loop */
		}

		bc = fwrite(outbuf, 1, 4, fout);		/* normal operation buffer-write */
		if (bc != 4) LOGGER_log("%s:%d:AM_base64_encode:ERROR: Wrote %d bytes rather than %d to output", FL, bc);

		cc += 4;									/* increment the output char count by 4 */

		if (cc > 76)							/* if we have more than 76 chars, then put in a \n */
		{
			bc = fwrite( "\n", 1, 1, fout );	/* write the \n out */
			if (bc != 1) LOGGER_log("%s:%d:AM_base64_encode:ERROR: Wrote %d bytes rather than %d to output", FL, bc);
			cc = 0;								/* reset the output char count */
		} /* if bytecount == 3 */
	} /* while more data to read in */

	return 0;
}






/*------------------------------------------------------------------------
Procedure:     AM_base64_encode ID:1
Purpose:       Encodes a given input stream into BASE64 and outputs to a given
output stream
Input:         char *enc_fname: Input filename of the stream to encode
char *out_fname: Output filename of the stream to send to
Output:
Errors:
------------------------------------------------------------------------*/
int AM_base64_encode( char *enc_fname, char *out_fname )
{
	FILE *fin, *fout;

	fin = fopen( enc_fname, "rb" );
	if (!fin)
	{
		LOGGER_log("AM_base64_encode: Cannot open \"%s\" for reading.(%s)",enc_fname,strerror(errno));
		return -1;
	}

	fout = fopen ( out_fname, "wb" );
	if (!fout)
	{
		LOGGER_log("AM_base64_encode: Cannot open \"%s\" for writing.(%s)",out_fname,strerror(errno));
		fclose(fin);
		return -1;
	}

	AM_base64_encodef( fin, fout );	/* encode and output */

	fclose(fin);	/* close the input file */
	fclose(fout);	/* close the output file */

	return 0;
}






/*-----------------------------------------------------------------\
  Date Code:	: 20070124-133319
  Function Name	: AM_base64_encode_from_buffer
  Returns Type	: int
  ----Parameter List
  1. char *buffer, 
  2.  size_t buffer_size, 
  3.  char *out_fname , 
  ------------------
  Exit Codes	: 
  Side Effects	: 
  --------------------------------------------------------------------
Comments:
Encodes a given buffer contents into BASE64 and writes to
file out_fname

--------------------------------------------------------------------
Changes:

\------------------------------------------------------------------*/
int AM_base64_encode_buffer_to_FILE( char *buffer, size_t buffer_size, FILE *fout)
{
	unsigned char outbuf[4];
	int cc;
	int chars = strlen(buffer);
	size_t bc;

	if (fout == NULL) {
		LOGGER_log("%s:%d:AM_base64_encode_buffer_to_FILE:ERROR: Output file stream was NULL", FL);
		return -1;
	}

	cc = 0;	/* Character Count = 0 */

	DAM LOGGER_log("%s:%d:AM_base64_encode_buffer_to_FILE:DEBUG: Encoding %d chars", FL, chars);

	while (chars > 0)	/* while we get more than 0 bytes */
	{
		/* Split 3 bytes into 4 bytes by taking 6bit blocks out of the 24 bit (3x8bit)
		 * array */
		if (chars >= 3) {
			outbuf[0] = AM_encode64[((buffer[0] & 0xFC) >> 2)];
			outbuf[1] = AM_encode64[((buffer[0] & 0x03) << 4) | ((buffer[1] & 0xF0) >> 4)];
			outbuf[2] = AM_encode64[((buffer[1] & 0x0F) << 2) | ((buffer[2] & 0xC0) >> 6)];
			outbuf[3] = AM_encode64[(buffer[2] & 0x3F)];
			buffer += 3;
			chars -= 3;
		} else {

			/* if we didn't get a full 3 bytes from the file read, then we need to then
			 * pad the output with '=' (base64 padding char) */
			DAM LOGGER_log("%s:%d:AM_base64_encode_buffer_to_FILE:DEBUG: Encoding remaining %d chars '%s'", FL, chars, buffer);

			if ( chars == 2 ) {
				DAM LOGGER_log("%s:%d:AM_base64_encode_buffer_to_FILE:DEBUG: '%c' '%c'", FL, buffer[0], buffer[1]);
				outbuf[0] = AM_encode64[((buffer[0] & 0xFC) >> 2)];
				outbuf[1] = AM_encode64[((buffer[0] & 0x03) << 4) | ((buffer[1] & 0xF0) >> 4)];
				outbuf[2] = AM_encode64[((buffer[1] & 0x0F) << 2)];
				outbuf[3] = '=';
			}
			if ( chars == 1 ) {
				DAM LOGGER_log("%s:%d:AM_base64_encode_buffer_to_FILE:DEBUG: '%c'", FL, buffer[0]);
				outbuf[0] = AM_encode64[((buffer[0] & 0xFC) >> 2)];
				outbuf[1] = AM_encode64[((buffer[0] & 0x03) << 4)];
				outbuf[3] = outbuf[2] = '=';
			}

			bc = fwrite(outbuf, 1, 4, fout); 	/* write out the buffer */
			if (bc != 4) LOGGER_log("%s:%d:AM_base64_encode_buffer_to_FILE:ERROR: Wrote %d bytes rather than %d to output", FL, bc);
			bc = fwrite( "\n", 1, 1, fout ); 	/* as this is the last line, put a trailing \n */
			if (bc != 1) LOGGER_log("%s:%d:AM_base64_encode_buffer_to_FILE:ERROR: Wrote %d bytes rather than %d to output", FL, bc);
			break; 								/* exit out of the while loop */
		}

		bc = fwrite(outbuf, 1, 4, fout);		/* normal operation buffer-write */
		if (bc != 4) LOGGER_log("%s:%d:AM_base64_encode_buffer_to_FILE:ERROR: Wrote %d bytes rather than %d to output", FL, bc);

		cc += 4;									/* increment the output char count by 4 */

		if (cc > 76)							/* if we have more than 76 chars, then put in a \n */
		{
			bc = fwrite( "\n", 1, 1, fout );	/* write the \n out */
			if (bc != 1) LOGGER_log("%s:%d:AM_base64_encode_buffer_to_FILE:ERROR: Wrote %d bytes rather than %d to output", FL, bc);
			cc = 0;								/* reset the output char count */
		} /* if bytecount == 3 */
	} /* while more data to encode */

	bc = fwrite("\n",1,1,fout);
	if (bc != 1) LOGGER_log("%s:%d:AM_base64_encode_buffer_to_FILE:ERROR: Wrote %d bytes rather than %d to output", FL, bc);
	return 0;

}








/*------------------------------------------------------------------------
Procedure:     MDECODE_decode_short64 ID:1
Purpose:       Decodes a BASE64 encoded realm
Input:         char *realm : base64 encoded NUL terminated string
Output:			decoded data is written to the buffer
Errors:
------------------------------------------------------------------------*/
int AM_base64_decode_buffer( char *buffer, size_t length )
{
	int i;
	int realm_size = length;
	int stopcount = 0; /* How many stop (=) characters we've read in */
	int c; /* a single char as retrieved using MDECODE_get_char() */
	int char_count = 0; /* How many chars have been received */
	char output[3]; /* The 4->3 byte output array */
	char input[4]; /* The 4->3 byte input array */
	char *outstring = buffer;

	char_count = 0;
	while (char_count < realm_size)
	{

		/* Initialise the decode buffer */
		input[0] = input[1] = input[2] = input[3] = 0;

		/* snatch 4 characters from the input */
		for (i = 0; i < 4; i++) {

			length--;

			/* get a char from the filestream */
			c = *buffer;

			if (c == '\0') break;

			buffer++;

			/* assuming we've gotten this far, then we increment the char_count */
			char_count++;

			/* if we detect the "stopchar" then we better increment the STOP counter */
			if (c == '=') {
				stopcount++;
			}

			/* test for and discard invalid chars */
			if (b64[c] == 0x80) {
				i--;
				continue;
			}

			/* do the conversion from encoded -> decoded */
			input[i] = (char)b64[c];

		} /* for */

		/* now that our 4-char buffer is full, we can do some fancy bit-shifting and get the required 3-chars of 8-bit data */
		output[0] = (input[0] << 2) | (input[1] >> 4);
		output[1] = (input[1] << 4) | (input[2] >> 2);
		output[2] = (input[2] << 6) | input[3];

		/* determine how many chars to write write and check for errors if our input char count was 4 then we did receive a propper 4:3 Base64 block, hence write it */
		if (i == 4) {
			for (i = 0; i < (3 -stopcount); i++){
				*outstring = output[i];
				outstring++;
			} /* copy our data across */
		} /* if 4 chars were inputted */
	} /* while more chars to proccess */


	*outstring = '\0';  // Set the last char to NULL

	return 0;
}

/*-----------------------------------------------------------------\
  Date Code:	: 20070124-113935
  Function Name	: AM_base64_decodef
  Returns Type	: int
  ----Parameter List
  1. FILE *fin, existing open intput file
  2.   FILE *fout, existing open output file
  3.  size_t input_size , number of bytes to read from input file
  ------------------
  Exit Codes	: 
  Side Effects	: 
  --------------------------------------------------------------------
Comments:
Decodes data contained in 'fin' and writes the decoded output
to 'fout'.

--------------------------------------------------------------------
Changes:

\------------------------------------------------------------------*/
int AM_base64_decodef( FILE *fin,  FILE *fout, size_t input_size )
{
	char *buffer;
	size_t bytes_read;

	/** Allocate memory for text file read **/
	buffer = malloc( input_size *sizeof(char));
	if (buffer == NULL) {
		LOGGER_log("%s:%d:AM_base64_decodef:ERROR: Cannot allocate %d bytes for base64 decoding", FL, input_size);
	}

	DAM LOGGER_log("%s:%d:AM_base64_decodef:DEBUG: Reading in %d bytes of RAW base64 data", FL, input_size);

	/** Read in entire block **/
	bytes_read = fread( buffer, sizeof(char), input_size, fin );
	if (bytes_read != input_size) {
		LOGGER_log("%s:%d:AM_base64_decodef:WARNING: Requested %d bytes from input file, only received %d", FL, input_size, bytes_read);
	}

	DAM LOGGER_log("%s:%d:AM_base64_decodef:DEBUG: %d bytes read", FL, bytes_read );

	/** Don't forget to terminate the data **/
	buffer[bytes_read] = '\0';

	DAM LOGGER_log("%s:%d:AM_base64_decodef:DEBUG: Raw input -------\n%s\n----------", FL, buffer);
	/** Now, decode the data into human-readable **/
	AM_base64_decode_buffer( buffer, bytes_read );

	/** Write data to the output file **/
	DAM LOGGER_log("%s:%d:AM_base64_decode_buffer:DEBUG: decoded output: ----------\n%s\n----------", FL, buffer);

	fprintf(fout, "%s", buffer );
	fflush(fout);

	/** Free the memory used **/
	free(buffer);

	return 0;
}











/*-----------------------------------------------------------------\
  Date Code:	: 20070124-131502
  Function Name	: AM_base64_decode_to_bufferf
  Returns Type	: int
  ----Parameter List
  1. FILE *fin, 
  2.   FILE *fout, 
  3.  size_t input_size , 
  ------------------
  Exit Codes	: 
  Side Effects	: 
  --------------------------------------------------------------------
Comments:
Reads in data from filestream FIN and decodes BASE64 into buffer

--------------------------------------------------------------------
Changes:

\------------------------------------------------------------------*/
int AM_base64_decode_to_bufferf( FILE *fin,  size_t input_size, char **decoded_buffer )
{
	char *buffer;
	size_t bytes_read;

	/** Allocate memory for text file read **/
	buffer = malloc( input_size *sizeof(char) +1);
	if (buffer == NULL) {
		LOGGER_log("%s:%d:AM_base64_decodef:ERROR: Cannot allocate %d bytes for base64 decoding", FL, input_size);
	}

	DAM LOGGER_log("%s:%d:AM_base64_decodef:DEBUG: Reading in %d bytes of RAW base64 data", FL, input_size);

	/** Read in entire block **/
	bytes_read = fread( buffer, sizeof(char), input_size, fin );
	if (bytes_read != input_size) {
		LOGGER_log("%s:%d:AM_base64_decodef:WARNING: Requested %d bytes from input file, only received %d", FL, input_size, bytes_read);
	}

	DAM LOGGER_log("%s:%d:AM_base64_decodef:DEBUG: %d bytes read", FL, bytes_read );

	/** Don't forget to terminate the data **/
	buffer[bytes_read] = '\0';

	DAM LOGGER_log("%s:%d:AM_base64_decodef:DEBUG: Raw input -------\n%s\n----------", FL, buffer);
	/** Now, decode the data into human-readable **/
	AM_base64_decode_buffer( buffer, bytes_read );


	/** Write data to the output file **/
	DAM LOGGER_log("%s:%d:AM_base64_decode_buffer:DEBUG: decoded output: ----------\n%s\n----------", FL, buffer);
	*decoded_buffer = buffer;

	return 0;
}



/*-----------------------------------------------------------------\
  Date Code:	: 20070124-114024
  Function Name	: AM_base64_decode
  Returns Type	: int
  ----Parameter List
  1. char *in_fname,  NUL terminated input filename
  2.  char *out_fname , NUL terminated output filename
  ------------------
  Exit Codes	: 
  Side Effects	: 
  --------------------------------------------------------------------
Comments:
Wrapper for _decodef().  Accepts an input and output filename
which are then opened and then passed to _decodef() along with
the intput file size.


--------------------------------------------------------------------
Changes:

\------------------------------------------------------------------*/
int AM_base64_decode( char *in_fname, char *out_fname )
{
	FILE *fin, *fout;
	struct stat st;
	int stat_result;


	stat_result = stat(in_fname, &st);
	if (stat_result) {
		LOGGER_log("%s:%d:AM_base64_decode:ERROR: Error stat'ing '%s' (%s)",FL, in_fname, strerror(errno));
		return -1;
	}

	DAM LOGGER_log("%s:%d:AM_base64_decode:DEBUG: %s size = %d bytes", FL, in_fname, st.st_size);

	fin = fopen( in_fname, "rb" );
	if (!fin)
	{
		LOGGER_log("AM_base64_decode: Cannot open \"%s\" for reading.(%s)",in_fname,strerror(errno));
		return -1;
	}

	fout = fopen ( out_fname, "wb" );
	if (!fout)
	{
		LOGGER_log("AM_base64_decode: Cannot open \"%s\" for writing.(%s)",out_fname,strerror(errno));
		fclose(fin);
		return -1;
	}

	AM_base64_decodef( fin, fout, st.st_size );	/* encode and output */

	fclose(fin);	/* close the input file */
	fclose(fout);	/* close the output file */

	return 0;

}

/*-----------------------------------------------------------------\
  Date Code:	: 20070124-131401
  Function Name	: AM_base64_decode_to_buffer
  Returns Type	: int
  ----Parameter List
  1. char *in_fname,  input file containing base64 data
  2.  char **buffer , pointer to a char buffer
  ------------------
  Exit Codes	: 
  Side Effects	: 
  --------------------------------------------------------------------
Comments:

Decodes the file in_fname which contains base64 data into the
buffer (which is allocated on demand)

--------------------------------------------------------------------
Changes:

\------------------------------------------------------------------*/
int AM_base64_decode_to_buffer( char *in_fname, char **buffer )
{
	FILE *fin;
	struct stat st;
	int stat_result;


	stat_result = stat(in_fname, &st);
	if (stat_result) {
		LOGGER_log("%s:%d:AM_base64_decode:ERROR: Error stat'ing '%s' (%s)",FL, in_fname, strerror(errno));
		return -1;
	}

	DAM LOGGER_log("%s:%d:AM_base64_decode:DEBUG: %s size = %d bytes", FL, in_fname, st.st_size);

	fin = fopen( in_fname, "rb" );
	if (!fin)
	{
		LOGGER_log("AM_base64_decode: Cannot open \"%s\" for reading.(%s)",in_fname,strerror(errno));
		return -1;
	}

	AM_base64_decode_to_bufferf( fin, st.st_size, buffer );	/* encode and output */

	fclose(fin);	/* close the input file */

	return 0;

}




/*------------------------------------------------------------------------
Procedure:     AM_add_section1 ID:1
Purpose:       Reads the main headers off an email and places the values it finds for
content-type, encoding and other such into the dd record
Input:         struct AM_disclaimer_details *dd : pointer to the record where all the details will go
FFGET_FILE *f : Open stream which we're reading the headers from
FILE *newf : Open stream which we're dumping the headers we read back into.
Output:
Errors:
------------------------------------------------------------------------*/
int AM_read_headers(struct AM_disclaimer_details *dd, FFGET_FILE *f, FILE *newf )
{
	struct MIMEH_header_info hinfo;

	DAM LOGGER_log("%s:%d:AM_read_headers:DEBUG: Starting to read headers", FL );

	dd->isfile = 0;
	dd->ishtml = 0;

	memset( &hinfo, '\0', sizeof(struct MIMEH_header_info));

	hinfo.uudec_name[0] = '\0';

	if (FFGET_feof(f) != 0) return -1;

	//	MIMEH_set_headers_save( newf ); // 20040309-2243:PLD
	MIMEH_set_doubleCR_save(0);
	MIMEH_set_header_longsearch(0); /** Turn off qmail searching, is not applicable here **/
	MIMEH_set_headers_original_save_to_file( newf );
	MIMEH_parse_headers( f, &hinfo );

	MIMEH_set_headers_original_save_to_file( NULL );
	MIMEH_set_doubleCR_save(1);
	MIMEH_set_headers_nosave();

	DAM LOGGER_log("%s:%d:AM_read_headers:DEBUG: lfcount=%d, crlfcount=%d", FL, hinfo.lf_count, hinfo.crlf_count);
	if (hinfo.lf_count > hinfo.crlf_count) {
		DAM LOGGER_log("%s:%d:AM_read_headers:DEBUG:Delimeter set to LF only",FL);
		snprintf(glb.ldelimeter,sizeof(glb.ldelimeter),"\n");
	} else {
		DAM LOGGER_log("%s:%d:AM_read_headers:DEBUG:Delimeter set to CRLF ",FL);
		snprintf(glb.ldelimeter,sizeof(glb.ldelimeter),"\r\n");
	}


	/* Copy over our information */
	dd->content_type = hinfo.content_type;
	dd->content_encoding = hinfo.content_transfer_encoding;
	if (hinfo.boundary_located > 0) {
		dd->boundary_found = 1;
		snprintf( dd->boundary, sizeof(dd->boundary), "%s", hinfo.boundary );
	}

	if (strlen(hinfo.filename) > 0) dd->isfile = 1;
	if (strlen(hinfo.name) > 0) dd->isfile = 1;

	/**
	 ** list here any content transfer encodings you
	 ** don't want to have disclaimers inserted into
	 **/
	switch (hinfo.content_transfer_encoding) {
		//NAB SHORTCIRCUIT		case _CTRANS_ENCODING_B64:
		case _CTRANS_ENCODING_UUENCODE:
		case _CTRANS_ENCODING_BINARY:
			dd->isfile = 1;
			break;
	}

	switch (hinfo.content_type) {
		case _CTYPE_MULTIPART_REPORT:
		case _CTYPE_RFC822:
			dd->isfile = 1;
			break;
	}

	/** If our content-type is of HTML, then we'll mark that we're in a HTML
	 **		section.  NOTE - this doesn't mean that the section isn't a file!
	 **		so, we have to check later on to see that dd.isfile == 0 still.
	 **/
	if (dd->content_type == _CTYPE_TEXT_HTML) dd->ishtml = 1;

	if (hinfo.content_type == _CTYPE_MULTIPART_SIGNED)
	{
		DAM LOGGER_log("%s:%d:AM_read_headers:DEBUG: Email is signed, return SIGNED_EMAIL",FL);
		return AM_RETURN_SIGNED_EMAIL;
	}

	DAM LOGGER_log("%s:%d:AM_read_headers:DEBUG: Exit ( Header read section ).\n\t-- isfile=%d ishtml=%d boundaryfound=%d\n\n", FL, dd->isfile, dd->ishtml, dd->boundary_found );

	return 0;
}



/*------------------------------------------------------------------------
Procedure:     AM_disclaimer_load_text ID:1
Purpose:       Loads the text from the file given in fname, and mallocs the required amount
of memory to place the entire contents of the file into textptr.
Input:         char *fname : The file name from which to read the dislcaimer text
char **textptr : a derefernced pointer to the buffer variable which will hold the text.
Output:
Errors:
------------------------------------------------------------------------*/
int AM_disclaimer_load_text( char *fname, char **textptr )
{
	FILE *f;
	char *p;
	struct stat st;
	size_t bc;

	*textptr = NULL;
	if (0 == stat(fname, &st))
	{
		if (!(p = malloc((st.st_size +1) * sizeof(char)))){ // We have to add 1 so that we delimit the data with a \0
			LOGGER_log("%s:%d:AM_disclaimer_load_text:ERROR: Unable to allocate memory to load file '%s'", FL, fname);
			return 1;
		}
		memset(p, '\0', (st.st_size +1));
		if (!(f = fopen(fname,"r")))
		{
			LOGGER_log("%s:%d:AM_disclaimer_load_text:ERROR: Cannot open %s for reading (%s)",FL,fname,strerror(errno));
			return 1;
		}
		if ((bc = fread(p, 1, st.st_size, f)) != st.st_size)
		{
			LOGGER_log("%s:%d:AM_disclaimer_load_text:ERROR: Read %d bytes instead of %d from %s", FL, bc, st.st_size, fname);
			fclose(f);
			return (1);
		} else
			DAM LOGGER_log("%s:%d:AM_disclaimer_load_text:DEBUG: Disclaimer Loaded (%s:%d):\n%s", FL, fname, st.st_size, p);
		fclose(f);
		*textptr = p;
	} else {
		LOGGER_log("%s:%d:AM_disclaimer_load_text:ERROR: Cannot stat '%s' (%s)",FL,fname,strerror(errno));
		return 2;
	}
	return 0;
}





/*-----------------------------------------------------------------\
  Date Code:	: 20070124-123353
  Function Name	: AM_disclaimer_html_perform_insertion
  Returns Type	: int
  ----Parameter List
  1. struct AM_disclaimer_details *dd, 
  2.  FFGET_FILE *f, 
  3.  FILE *newf , 
  ------------------
  Exit Codes	: 
  Side Effects	: 
  --------------------------------------------------------------------
Comments:

--------------------------------------------------------------------
Changes:

\------------------------------------------------------------------*/
int AM_disclaimer_html_perform_insertion( struct AM_disclaimer_details *dd, FFGET_FILE *f, FILE *newf )
{
	char *p;

	if (dd->content_encoding == _CTRANS_ENCODING_QP) {
		char *qp_data;
		size_t qp_data_size;
		char *data_to_use;

		if (dd->disclaimer_text_HTML == NULL) data_to_use = dd->disclaimer_text_plain;
		else data_to_use = dd->disclaimer_text_HTML;

		qp_data_size = strlen(data_to_use) *3 +1;
		qp_data = malloc( qp_data_size *sizeof(char));
		if (qp_data == NULL) {
			LOGGER_log("%s:%d:AM_disclaimer_html_perform_insertion:DEBUG: Error trying to allocate %d bytes of memory for QP encoded disclaimer",FL, qp_data_size);
			return -1;
		}
		qp_encode( qp_data, qp_data_size, data_to_use, strlen(data_to_use));
		p = AM_adapt_linebreak(qp_data, glb.ldelimeter);

		DAM LOGGER_log("%s:%d:AM_disclaimer_html_perform_insertion:DEBUG: Inserting QP encoded disclaimer",FL);
		if (dd->disclaimer_text_HTML == NULL) {
			fprintf(newf,"<br><pre>=%s",glb.ldelimeter);
			fprintf(newf,"%s",p);
			fprintf(newf,"</pre><br>=%s",glb.ldelimeter);
		} else {
			fprintf(newf,"<br>=%s", glb.ldelimeter);
			fprintf(newf,"%s",p);
			fprintf(newf,"<br>=%s",glb.ldelimeter);
		}


		/** cleanup **/
		if (qp_data) free(qp_data);

		/** Quoted printable email segment, so we have to encode everything **/

	} else {

		if (dd->disclaimer_text_HTML == NULL)
		{
			fprintf(newf,"<br><pre>%s", glb.ldelimeter);
			p = AM_adapt_linebreak(dd->disclaimer_text_plain, glb.ldelimeter);
			fprintf(newf,"%s</pre><br>%s", p, glb.ldelimeter);
		}
		else
		{
			fprintf(newf,"<br>%s", glb.ldelimeter);
			p = AM_adapt_linebreak(dd->disclaimer_text_HTML, glb.ldelimeter);
			fprintf(newf,"%s<br>%s", p, glb.ldelimeter);
		}
	} /** quotedprintable / non encoding selector **/

	return 0;
}

/*------------------------------------------------------------------------
Procedure:     AM_add_disclaimer_insert_html ID:1
Purpose:       Inserts a disclaimer with <PRE> tagging into a HTML block of text just prior to the
end of the HTML. Assumes that it has been placed at the start of the HTML block
Input:         AM_disclaimer_details *dd : Disclaimer tracking information
FFGET_FILE *f : Input file
FILE *newf : Output file
Output:
Errors:
------------------------------------------------------------------------*/
int AM_add_disclaimer_insert_html( 	struct AM_disclaimer_details *dd, FFGET_FILE *f, FILE *newf )
{
	char boundary[ AM_1K_BUFFER_SIZE +1];
	int boundary_length = 0;
	char line[ AM_1K_BUFFER_SIZE +1];
	char lline[ AM_1K_BUFFER_SIZE +1];
	char *prebody, *tmpbody;

	DAM LOGGER_log("%s:%d:AM_add_disclaimer_insert_html:DEBUG: Starting to attempt to insert HTML disclaimer",FL);

	if (dd->boundary_found == 1)
	{
		snprintf(boundary, sizeof(boundary), "--%s", dd->boundary);
		boundary_length = strlen(boundary);
	}

	while (FFGET_fgets(line, AM_1K_BUFFER_SIZE, f))
	{
		/** 20041019-1135:PLD: Applied diff as contributed by Tim (datalore)
		 **
		 ** If we reached the end of the boundary, check if force html insertion is
		 ** enabled. If so, force the html disclaimer into the message.
		 **/
		if(dd->boundary_found == 1 && strncmp(boundary, line, boundary_length) == 0)
		{
			DAM LOGGER_log("%s:%d:AM_add_disclaimer_insert_html: End of boundary reached before html disclamer was added...",FL);
			if (glb.force_for_bad_html == 1)
			{
				DAM LOGGER_log("%s:%d:Forcing insertion of html disclaimer into non valid html body...",FL);

				dd->html_inserted = 1;

				AM_disclaimer_html_perform_insertion( dd, f, newf );

			}

			// write the boundary line
			fprintf(newf, "%s", line);

			// stop searching/adding/whatever
			break;
		}

		/** not at end of boundary, so search for body/html tag **/

		/** End of patch as supplied by Tim (datalore) **/



		strcpy(lline,line);
		PLD_strlower(lline);

		// Look for either a BODY or HTML closing tag.

		prebody = strstr(lline,"</body");
		if (!prebody) prebody = strstr(lline,"</html");


		// If we found one of the tags, then insert our disclaimer aruond about
		// here.

		if (prebody != NULL)
		{
			DAM LOGGER_log("%s:%d:AM_add_disclaimer_insert_html:DEBUG: prebody = %s",FL,prebody);
			DAM LOGGER_log("%s:%d:AM_add_disclaimer_insert_html:DEBUG: Inserting html-body disclaimer",FL);

			dd->html_inserted = 1;

			// prepare to print out the original up to the </body> or </html> part

			tmpbody = line +(prebody -lline);
			*tmpbody = '\0';

			// save to file the first part of the line segment

			fprintf(newf,"%s%s",line, glb.ldelimeter);
			AM_disclaimer_html_perform_insertion( dd, f, newf );
			fprintf(newf,"<%s",(tmpbody+1));

			break;

		} else {
			fprintf(newf,"%s",line);
		}

	} // While FFGET_fgets()

	return dd->html_inserted;
}




/*-----------------------------------------------------------------\
  Function Name	: AM_add_disclaimer_cleanup
  Returns Type	: int
  ----Parameter List
  1. FILE *mp, 
  2.  FILE *newf, 
  3.  char *mpacktmp, 
  4.  char *mpackname , 
  ------------------
  Exit Codes	: 
  Side Effects	: 
  --------------------------------------------------------------------
Comments:

--------------------------------------------------------------------
Changes:

\------------------------------------------------------------------*/
int AM_add_disclaimer_cleanup( FILE *mp, FILE *newf, char *mpacktmp, char *mpackname )
{

	/** If the input file wasn't stdin, then we will
	 ** need to close the input and output files and
	 ** rename the old to the new **/
	if (strcmp(mpackname,"-"))
	{
		// Close our files

		fclose(mp);
		fclose(newf);

		// rename the files

		rename(mpacktmp, mpackname);
	}

	return 0;
}

/*-----------------------------------------------------------------\
  Function Name	: AM_add_disclaimer_flush
  Returns Type	: int
  ----Parameter List
  1. FFGET_FILE *f, 
  2. FILE *newf, 
  ------------------
  Exit Codes	: 
  Side Effects	: 
  --------------------------------------------------------------------
Comments:

--------------------------------------------------------------------
Changes:

\------------------------------------------------------------------*/
int AM_add_disclaimer_flush( FFGET_FILE *f, FILE *newf )
{
	char line[AM_1K_BUFFER_SIZE+1]="";

	if ( ! FFGET_feof( f ) )
	{
		DAM LOGGER_log("%s:%d:AM_add_disclaimer_flush:DEBUG: Appending remaining of file",FL);
		while (FFGET_fgets(line, AM_1K_BUFFER_SIZE, f))
		{
			fprintf(newf,"%s",line);
		}
		DAM LOGGER_log("%s:%d:AM_add_disclaimer_flush:DEBUG: Done appending.",FL);
	}

	return 0;
}


/*-----------------------------------------------------------------\
  Function Name	: AM_read_to_boundary
  Returns Type	: int
  ----Parameter List
  1. FFGET_FILE *infile, Source data file
  2.  FILE *outf , File to send data to
  ------------------
  Exit Codes	: 
  Side Effects	: 
  --------------------------------------------------------------------
Comments:
Copies data over from the current MIME segment from infile to outf.
Copying stops when a boundary line is found.
The boundary line is written to the output file.


--------------------------------------------------------------------
Changes:

\------------------------------------------------------------------*/
int AM_read_to_boundary( FFGET_FILE *infile, FILE *outf, char *buffer, size_t buffer_size )
{
	while (FFGET_fgets(buffer, buffer_size, infile))
	{
		fprintf(outf,"%s",buffer);
		if ( (BS_cmp(buffer,strlen(buffer))==1) ) 
		{
			DAM LOGGER_log("%s:%d:AM_read_to_boundary:DEBUG: Boundary hit while reading MIME segment, breaking out of while loop",FL);
			break;
		}
	}
	return 0;
}



/*-----------------------------------------------------------------\
  Function Name	: AM_load_disclaimers
  Returns Type	: int
  ----Parameter List
  1. struct AM_disclaimer_details *dd , 
  ------------------
  Exit Codes	: 
  Side Effects	: 
  --------------------------------------------------------------------
Comments:
Load the disclaimers into the disclaimer structure.
Returns -1 if neither of the disclaimers could be 
loaded.

--------------------------------------------------------------------
Changes:

\------------------------------------------------------------------*/
int AM_load_disclaimers( struct AM_disclaimer_details *dd )
{
	int dud_html=0, dud_text=0, dud_b64=0;

	dd->disclaimer_text_HTML = NULL;
	dd->disclaimer_text_plain = NULL;
	dd->disclaimer_text_b64 = NULL;

	if ((glb.disclaimer_plain != NULL )&&(glb.disclaimer_plain_type != AM_DISCLAIMER_TYPE_NONE))
	{
		if (glb.disclaimer_plain_type == AM_DISCLAIMER_TYPE_FILENAME)
		{
			if (AM_disclaimer_load_text( glb.disclaimer_plain, &(dd->disclaimer_text_plain)))
				return -1;
		}
		else
		{
			dd->disclaimer_text_plain = glb.disclaimer_plain;
		}
	} else {
		/*- LOGGER_log("AM_add_disclaimer: Plain-text disclaimer has not been setup correctly\n");*/
		dud_text=1;
	}


	if ((glb.disclaimer_HTML != NULL )&&(glb.disclaimer_HTML_type != AM_DISCLAIMER_TYPE_NONE))
	{
		if (glb.disclaimer_HTML_type == AM_DISCLAIMER_TYPE_FILENAME)
		{
			if (AM_disclaimer_load_text( glb.disclaimer_HTML, &(dd->disclaimer_text_HTML)))
				return -1;
		}
		else
		{
			dd->disclaimer_text_HTML = glb.disclaimer_HTML;
		}
	} else {
		//		LOGGER_log("AM_add_disclaimer: HTML-text disclaimer has not been setup correctly\n");
		dud_html=1;
	}

	if ((glb.disclaimer_b64 != NULL)&&(glb.disclaimer_b64_type != AM_DISCLAIMER_TYPE_NONE))
	{
		if (glb.disclaimer_b64_type == AM_DISCLAIMER_TYPE_FILENAME)
		{
			AM_base64_decode_to_buffer( glb.disclaimer_b64, &(dd->disclaimer_text_b64) );
		} else {
			dd->disclaimer_text_b64 = strdup( glb.disclaimer_b64 );
			AM_base64_decode_buffer( dd->disclaimer_text_b64, strlen(dd->disclaimer_text_b64));
		}
	} else {
		dud_b64=1;
	}

	// If our disclaiemrs are all 'dud's, then we should just return with an error.
	if ((dud_html == 1)&&(dud_text == 1)&&(dud_b64 == 1))
	{
		LOGGER_log("%s:%d:AM_load_disclaimers: Neither the Plain-text , HTML or BASE64 disclaimer were valid to insert, skipping disclaimer-insertion routine\n");
		return -1;
	}

	return 0;

}





/*-----------------------------------------------------------------\
  Date Code:	: 20070124-135031
  Function Name	: *AM_insert_disclaimer_into_buffer
  Returns Type	: char
  ----Parameter List
  1. char *buffer, 
  2.  struct AM_disclaimer_details *dd , 
  ------------------
  Exit Codes	: 
  Side Effects	: 
  --------------------------------------------------------------------
Comments:

--------------------------------------------------------------------
Changes:

\------------------------------------------------------------------*/
char *AM_insert_disclaimer_into_buffer( char *buffer, struct AM_disclaimer_details *dd )
{
	char *p = buffer;
	size_t total_size;

	total_size = strlen(dd->disclaimer_text_plain) +strlen(buffer) +1;

	p = malloc( total_size *sizeof(char) );
	if (p) {
		snprintf(p, total_size-1, "%s\n%s", buffer, dd->disclaimer_text_plain);
	}

	return p;
}




/*-----------------------------------------------------------------\
  Date Code:	: 20070124-135051
  Function Name	: *AM_insert_HTML_disclaimer_into_buffer
  Returns Type	: char
  ----Parameter List
  1. char *buffer, 
  2.  struct AM_disclaimer_details *dd , 
  ------------------
  Exit Codes	: 
  Side Effects	: 
  --------------------------------------------------------------------
Comments:

Inserts a disclaimer into a HTML body, this is typically
selected based on the content-type of 'text/html' in the 
headers.

If no suitable HTML framework can be found in the text
the disclaimer can still be inserted by forcing it with
--force-for-bad-html

--------------------------------------------------------------------
Changes:

\------------------------------------------------------------------*/
char *AM_insert_HTML_disclaimer_into_buffer( char *buffer, struct AM_disclaimer_details *dd )
{
	char *p;
	char *new_buffer = NULL;

	if (buffer == NULL ) {
		LOGGER_log("%s:%d:AM_insert_HTML_disclaimer_into_buffer:ERROR: Input buffer is NULL",FL);
	}

	DAM LOGGER_log("%s:%d:AM_insert_HTML_disclaimer_into_buffer:DEBUG: Searching for BODY closing tag\n'''%s'''",FL, buffer);

	p = PLD_strstr( buffer, "</BODY", 1); /** case insensitive search **/
	if (!p) {
		DAM LOGGER_log("%s:%d:AM_insert_HTML_disclaimer_into_buffer:DEBUG: Searching for HTML closing tag",FL);
		p = PLD_strstr( buffer, "</HTML", 1);
	}



	if (p) {
		*p = '\0';

		DAM LOGGER_log("%s:%d:AM_insert_HTML_disclaimer_into_buffer:DEBUG: Composing HTML + disclaimer", FL);
		if (dd->disclaimer_text_HTML) {
			new_buffer = PLD_dprintf("%s\n%s\n<%s"
					,buffer
					,dd->disclaimer_text_HTML
					,(p+1)
					);
		} else {
			new_buffer = PLD_dprintf("%s\n<PRE>%s</pre>\n<%s"
					,buffer
					,dd->disclaimer_text_plain
					,(p+1)
					);
		}

		dd->html_inserted = 1;

	} else {

		/** Getting desperate here, try for some other HTML tags that are
		 * fairly common */
		if (
				PLD_strstr(buffer, "<BR", 1)
				||PLD_strstr(buffer, "<FONT", 1)
				||PLD_strstr(buffer, "<A ",1)
				||(glb.force_for_bad_html == 1)
			) {
			if (dd->disclaimer_text_HTML) {
				new_buffer = PLD_dprintf("%s\n%s", buffer, dd->disclaimer_text_HTML);
			} else {
				new_buffer = PLD_dprintf("%s\n<PRE>%s</pre>\n", buffer, dd->disclaimer_text_plain);
			}

		} else {
			/** If all else fails, just dupe the block **/
			new_buffer = strdup(buffer);
		}
	}	  

	DAM LOGGER_log("%s:%d:AM_insert_HTML_disclaimer_into_buffer:DEBUG: Final segment '''%s'''", FL, new_buffer);

	return new_buffer;
}






/*-----------------------------------------------------------------\
  Function Name	: AM_insert_disclaimer_into_segment64
  Returns Type	: int
  ----Parameter List
  1. FFGET_FILE *f,  source file (original mailpack)
  2.  FILE *newf, destination file (new mailpack)
  3.  struct AM_disclaimer_details *dd , 
  ------------------
  Exit Codes	: 
  Side Effects	: 
  --------------------------------------------------------------------
Comments:

Inserts a disclaimer into a BASE64 encoded segment

--------------------------------------------------------------------
Changes:
20061220-1643-PLD: Added

\------------------------------------------------------------------*/
int AM_insert_disclaimer_into_segment64( FFGET_FILE *f, FILE *newf, struct AM_disclaimer_details *dd )
{
	char line[AM_1K_BUFFER_SIZE+1]="";
	int last_boundary_written = -1;
	int insert_success = 0;
	char *b64_buffer, *new_b64_buffer;
	char b64_raw_fname[128]; 
	FILE *b64_raw_file;

	DAM LOGGER_log("%s:%d:AM_insert_disclaimer_into_segment64:DEBUG: Inserting disclaimer into segment",FL);

	/** This is the file that is used to store the B64 encoded segment, which we'll 
	 * DECODE, apply disclaimer, then ENCODE again and insert back into the
	 * original mailpack
	 *
	 */
	snprintf(b64_raw_fname, sizeof(b64_raw_fname), "altermime-raw-%d.b64", getpid());

	b64_raw_file = fopen(b64_raw_fname, "w");
	if (b64_raw_file == NULL) {
		DAM LOGGER_log("%s:%d:AM_insert_disclaimer_into_segment64:ERROR: Cannot open '%s' (%s)", FL, b64_raw_fname, strerror(errno));
		return 1;
	}

	/** From here the output file is ready **/

	if ( (0 == dd->text_inserted) 
			&& (dd->content_type == _CTYPE_TEXT_PLAIN ) 
			&& (dd->content_encoding == _CTRANS_ENCODING_B64)
			&& (dd->isfile == 0) 
		)
	{
		DAM LOGGER_log("%s:%d:AM_insert_disclaimer_into_segment64:DEBUG: Conditions right to insert disclaimer\n",FL);
		DAM LOGGER_log("%s:%d:AM_insert_disclaimer_into_segment64:DEBUG: Reading first segment looking for boundary line\n",FL);

		// Read and write data until we locate the boundary line.
		//	NOTE - we -deliberately- break before writing the boundary
		//	line because we want to insert the disclaimer /before/
		//	we write the boundary line.

		while (FFGET_fgets(line, AM_1K_BUFFER_SIZE, f))
		{
			last_boundary_written = 0;
			if ( BS_cmp(line,strlen(line))==1 )
			{
				DAM LOGGER_log("%s:%d:AM_insert_disclaimer_into_segment64:DEBUG: Boundary hit",FL);
				break;
			}
			fprintf(b64_raw_file, "%s", line); /* write raw base64 data to the tmp file **/
			//fprintf( newf, "%s", line );
			//last_boundary_written = 1;
		}

		fclose(b64_raw_file);

		/** Okay, at this point, we've got our file containing the base64 encoded data
		 * so now we just _decode_ it and add in the disclaimer as before (oh yes, very
		 * easy indeed *cough*
		 *
		 * b64_name contains the raw file, b64_decoded_fname is the decoded file 
		 */
		AM_base64_decode_to_buffer( b64_raw_fname, &b64_buffer );

		/** Remove the raw file **/
		unlink(b64_raw_fname);

		/** b64_buffer now contains the decoded text **/
		new_b64_buffer = AM_insert_disclaimer_into_buffer( b64_buffer, dd );
		if (new_b64_buffer) {

			/** Encode the text file back into B64 **/
			DAM LOGGER_log("%s:%d:AM_insert_disclaimer_into_segment64:DEBUG: Inserting TEXT disclaimer (%s)\n", FL, dd->disclaimer_text_plain);
			AM_base64_encode_buffer_to_FILE( new_b64_buffer, strlen(new_b64_buffer), newf );
			dd->text_inserted = 1;
			free(new_b64_buffer);
		}



		insert_success = 1;
		DAM LOGGER_log("%s:%d:AM_insert_disclaimer_into_segment64:DEBUG: TEXT disclaimer is inserted, now flushing file output",FL);
		fflush(newf);

		if ( last_boundary_written == 0 )
		{
			DAM LOGGER_log("%s:%d:AM_insert_disclaimer_into_segment64:DEBUG: writing the boundary line '%s'", FL, line);
			fprintf( newf, "%s", line );
			last_boundary_written = 1;
		}

	} else {
		DAM LOGGER_log("%s:%d:AM_insert_disclaimer_into_segment64:DEBUG: Conditions not right to insert a TEXT disclaimer",FL);

	}


	// Add in the HTML disclaimer (if wanted)

	DAM LOGGER_log("%s:%d:AM_insert_disclaimer_into_segment64:DEBUG: glb.HTML_too=%d, dd->html_inserted=%d, dd->content_type=%d, dd->isfile=%d"
			,FL
			,glb.HTML_too
			,dd->html_inserted
			,dd->content_type
			,dd->isfile
			);

	if ( ( glb.HTML_too ) 
			&& ( 0 == dd->html_inserted ) 
			&& ( dd->content_type == _CTYPE_TEXT_HTML )
			&& ( dd->content_encoding == _CTRANS_ENCODING_B64)
			&& ( dd->isfile == 0 ) 
		)
	{
		DAM LOGGER_log("%s:%d:AM_insert_disclaimer_into_segment64:DEBUG: Conditions right for HTML disclaimer to be added",FL);

		while (FFGET_fgets(line, AM_1K_BUFFER_SIZE, f))
		{
			last_boundary_written = 0;
			if ( BS_cmp(line,strlen(line))==1 )
			{
				DAM LOGGER_log("%s:%d:AM_insert_disclaimer_into_segment64:DEBUG: Boundary hit",FL);
				break;
			}
			fprintf(b64_raw_file, "%s", line); /* write raw base64 data to the tmp file **/
			//fprintf( newf, "%s", line );
		}

		fclose(b64_raw_file);

		/** Convert the BASE64 code into human text **/
		AM_base64_decode_to_buffer( b64_raw_fname, &b64_buffer );

		/** Remove the raw file **/
		unlink(b64_raw_fname);

		new_b64_buffer = AM_insert_HTML_disclaimer_into_buffer( b64_buffer, dd );
		if (!new_b64_buffer) {
			new_b64_buffer = b64_buffer;
		}
		AM_base64_encode_buffer_to_FILE( new_b64_buffer, strlen(new_b64_buffer), newf );
		free(new_b64_buffer);

		if ((glb.verbose)&&(0 == dd->html_inserted))
		{
			LOGGER_log("WARNING: Could not insert HTML disclaimer into email");
		} else {
			insert_success = 1;
		}

		if ( last_boundary_written == 0 )
		{
			DAM LOGGER_log("%s:%d:AM_insert_disclaimer_into_segment64:DEBUG: writing the boundary line '%s'", FL, line);
			fprintf( newf, "%s", line );
			last_boundary_written = 1;
		}

	} else {
		DAM LOGGER_log("%s:%d:AM_insert_disclaimer_into_segment64:DEBUG: Conditions not right to insert HTML disclaimer",FL);
	}

	/** If we weren't able to insert a disclaimer, then read through to the end of the segment **/
	if (insert_success == 0) {
		while (FFGET_fgets(line, AM_1K_BUFFER_SIZE, f)) {
			last_boundary_written = 0;
			fprintf( newf, "%s", line );
			if ( BS_cmp(line,strlen(line))==1 ) {
				DAM LOGGER_log("%s:%d:AM_insert_disclaimer_into_segment64:DEBUG: Boundary hit, breaking out",FL);
				break;
			}
		}
	}

	return 0;
}







/*-----------------------------------------------------------------\
  Function Name	: AM_add_disclaimer_plain_text
  Returns Type	: int
  ----Parameter List
  1. FFGET_FILE *f, 
  2.  FILE *newf, 
  3.  struct AM_disclaimer_details *dd , 
  ------------------
  Exit Codes	: 
  Side Effects	: 
  --------------------------------------------------------------------
Comments:

--------------------------------------------------------------------
Changes:

\------------------------------------------------------------------*/
int AM_add_disclaimer_no_boudary( FFGET_FILE *f, FILE *newf, struct AM_disclaimer_details *dd )
{
	char line[AM_1K_BUFFER_SIZE+1]="";
	char *p;

	DAM LOGGER_log("%s:%d:AM_add_disclaimer_no_boudary:DEBUG: Inserting disclaimer into a non-boundary email",FL);


	/** Don't insert into Calendar files **/
	if ( dd->content_type == _CTYPE_TEXT_CALENDAR) {
		return -1;
	}

	if ( 
			dd->content_encoding == _CTRANS_ENCODING_B64
			&& (
				(dd->content_type == _CTYPE_TEXT_HTML)
				|| (dd->content_type == _CTYPE_TEXT_PLAIN)
				)
		) {
		/** Normally we don't recommend inserting disclaimers into BASE64 encoded regions
		 * however, if required, we can do it by setting --force-into-b64
		 *
		 * Even still, we'll only (at this point) insert into text/html or text/plain
		 * content-type segments.  Inserting into other segments is just asking for
		 * potential file mangling.
		 *
		 * Originally done for NAB.
		 */
		if (glb.force_into_b64) return AM_insert_disclaimer_into_segment64( f, newf, dd );
		else return -1;

		/** _segment64 duplicates the rest of the existing _segment function, so there's
		 * no need to go beyond here
		 */
		LOGGER_log("%s:%d:AM_insert_disclaimer_into_segment:ERROR: Executed beyond base64 segment insert, please report this.",FL);
	}

	// If we have a HTML email body, then insert the HTML based
	//		disclaimer, or, the plain-text one with <PRE>...</pre>
	//		tagging

	switch (dd->content_type) {
		case _CTYPE_TEXT_HTML:
			DAM LOGGER_log("%s:%d:AM_add_disclaimer_no_boudary:DEBUG: Conditions right for HTML disclaimer to be added",FL);
			//
			if (dd->content_encoding != _CTRANS_ENCODING_B64) {
				AM_add_disclaimer_insert_html(  dd, f, newf );
			}
			if ((glb.verbose)&&( dd->html_inserted == 0 ))
			{
				LOGGER_log("WARNING: Could not insert HTML disclaimer into email\n");
			}
			break;

		default:

			// If we have a plain-text email body, then just get to the end of the file
			//		and append the disclaimer to the end.

			DAM LOGGER_log("%s:%d:AM_add_disclaimer_insert_html:DEBUG: Seeking to the end of the file for plain-text insertion...",FL);
			/** Read to the end of the file **/
			AM_read_to_boundary( f, newf, line, AM_1K_BUFFER_SIZE );

			DAM LOGGER_log("%s:%d:AM_add_disclaimer_no_boudary:DEBUG: About to write disclaimer '%s'",FL,dd->disclaimer_text_plain);
			p = AM_adapt_linebreak(dd->disclaimer_text_plain, glb.ldelimeter); // 200811061459:PLD: converts text linebreaks to appropriate format
			fprintf(newf,"%s",p); 
			dd->text_inserted = 1;
			DAM LOGGER_log("%s:%d:AM_add_disclaimer_no_boudary:DEBUG: Disclaimer now written to file",FL);
			break;
	}

	DAM LOGGER_log("%s:%d:AM_add_disclaimer_no_boudary:DEBUG: Done, text-inserted=%d, html-inserted=%d"
			,FL
			,dd->text_inserted
			,dd->html_inserted
			);

	return 0;

}
















/*-----------------------------------------------------------------\
  Function Name	: AM_insert_disclaimer_into_segment
  Returns Type	: int
  ----Parameter List
  1. FFGET_FILE *f, 
  2.  FILE *newf, 
  3.  struct AM_disclaimer_details *dd , 
  ------------------
  Exit Codes	: 
  Side Effects	: 
  --------------------------------------------------------------------
Comments:

--------------------------------------------------------------------
Changes:

\------------------------------------------------------------------*/
int AM_insert_disclaimer_into_segment( FFGET_FILE *f, FILE *newf, struct AM_disclaimer_details *dd )
{
	char line[AM_1K_BUFFER_SIZE+1]="";
	int last_boundary_written = -1;
	int result = 0;
	int insert_success = 0;

	DAM LOGGER_log("%s:%d:AM_insert_disclaimer_into_segment:DEBUG: Inserting disclaimer into segment - encoding=%d type=%d",FL, dd->content_encoding, dd->content_type);


	/** check for types that we're not supposed to be inserting into
	 *
	 * Currently the types not to insert to include;
	 *		text/calendar (calendar files)
	 *		
	 *		*/

	if (dd->content_type == _CTYPE_TEXT_CALENDAR) {
		DAM LOGGER_log("%s:%d:AM_insert_disclaimer_into_segment:DEBUG: Not inserting into a calendar file");
		return -1;
	}

	if ( 
			dd->content_encoding == _CTRANS_ENCODING_B64
			&& (
				(dd->content_type == _CTYPE_TEXT_HTML)
				|| (dd->content_type == _CTYPE_TEXT_PLAIN)
				)
		) {
		/** Normally we don't recommend inserting disclaimers into BASE64 encoded regions
		 * however, if required, we can do it by setting --force-into-b64
		 *
		 * Even still, we'll only (at this point) insert into text/html or text/plain
		 * content-type segments.  Inserting into other segments is just asking for
		 * potential file mangling.
		 *
		 * Originally done for NAB.
		 */
		if (glb.force_into_b64) return AM_insert_disclaimer_into_segment64( f, newf, dd );
		else return -1;

		/** _segment64 duplicates the rest of the existing _segment function, so there's
		 * no need to go beyond here
		 */
		LOGGER_log("%s:%d:AM_insert_disclaimer_into_segment:ERROR: Executed beyond base64 segment insert, please report this.",FL);
	}


	if ( (0 == dd->text_inserted) \
			&& (dd->content_type == _CTYPE_TEXT_PLAIN ) \
			&& (dd->isfile == 0) \
		)
	{
		DAM LOGGER_log("%s:%d:AM_insert_disclaimer_into_segment:DEBUG: Conditions right to insert disclaimer\n",FL);
		DAM LOGGER_log("%s:%d:AM_insert_disclaimer_into_segment:DEBUG: Reading first segment looking for boundary line\n",FL);

		// Read and write data until we locate the boundary line.
		//	NOTE - we -deliberately- break before writing the boundary
		//	line because we want to insert the disclaimer /before/
		//	we write the boundary line.

		while (FFGET_fgets(line, AM_1K_BUFFER_SIZE, f))
		{
			last_boundary_written = 0;
			if ( BS_cmp(line,strlen(line))==1 )
			{
				DAM LOGGER_log("%s:%d:AM_insert_disclaimer_into_segment:DEBUG: Boundary hit",FL);
				break;
			}
			fprintf( newf, "%s", line );
			last_boundary_written = 1;
		}

		if ( FFGET_feof( f ) ) return -1;

		DAM LOGGER_log("%s:%d:AM_insert_disclaimer_into_segment:DEBUG: Inserting TEXT disclaimer (%s)\n", FL, dd->disclaimer_text_plain);

		if (dd->content_encoding == _CTRANS_ENCODING_QP)
		{
			/** convert the disclaimer into QP **/
			char *qp_data;
			size_t qp_data_size;

			qp_data_size = strlen(dd->disclaimer_text_plain) *3 +1;
			qp_data = malloc( qp_data_size *sizeof(char));
			if (qp_data == NULL) {
				LOGGER_log("%s:%d:AM_insert_disclaimer_into_segment:DEBUG: Error trying to allocate %d bytes of memory for QP encoded disclaimer",FL, qp_data_size);
				return -1;
			}
			qp_encode( qp_data, qp_data_size, dd->disclaimer_text_plain, strlen(dd->disclaimer_text_plain));
			DAM LOGGER_log("%s:%d:AM_insert_disclaimer_into_segment:DEBUG: Inserting QP encoded disclaimer",FL);
			fprintf( newf, "%s\n", qp_data );

			/** cleanup **/
			if (qp_data) free(qp_data);

		}  else {
			char *p;
			/** Normal plain-text insertion **/
			p = AM_adapt_linebreak(dd->disclaimer_text_plain, glb.ldelimeter); // 200811061501:PLD: Adapts the linebreaks to match the email
			if (p) {
				fprintf(newf, "%s",p);
			} else {
				LOGGER_log("%s:%d:AM_insert_disclaimer_into_segment:WARNING: Disclaimer was NULL after linebreak adaption", FL);
			}
		}

		dd->text_inserted = 1; /** Our disclaimer has been inserted, so set the flag **/
		insert_success = 1;
		DAM LOGGER_log("%s:%d:AM_insert_disclaimer_into_segment:DEBUG: TEXT disclaimer is inserted, now flushing file output",FL);
		fflush(newf);

		if ( last_boundary_written == 0 )
		{
			DAM LOGGER_log("%s:%d:AM_insert_disclaimer_into_segment:DEBUG: Writing boundary down",FL);
			fprintf( newf, "%s", line );
			last_boundary_written = 1;
		}

	} else {
		DAM LOGGER_log("%s:%d:AM_insert_disclaimer_into_segment:DEBUG: Conditions not right to insert a TEXT disclaimer",FL);

	}


	// Add in the HTML disclaimer (if wanted)

	DAM LOGGER_log("%s:%d:AM_insert_disclaimer_into_segment:DEBUG: glb.HTML_too=%d, dd->html_inserted=%d, dd->content_type=%d, dd->isfile=%d"
			,FL
			,glb.HTML_too
			,dd->html_inserted
			,dd->content_type
			,dd->isfile
			);

	if ( ( glb.HTML_too ) \
			&& ( 0 == dd->html_inserted ) \
			&& ( dd->content_type == _CTYPE_TEXT_HTML )\
			&& ( dd->isfile == 0 ) \
		)
	{
		DAM LOGGER_log("%s:%d:AM_insert_disclaimer_into_segment:DEBUG: Conditions potentially right for HTML disclaimer to be added",FL);

		result = AM_add_disclaimer_insert_html(  dd, f, newf );
		if ((glb.verbose)&&(0 == result))
		{
			LOGGER_log("WARNING: Could not insert HTML disclaimer into email");
		} else {
			dd->html_inserted = 1;
			insert_success = 1;
		}
	} else {
		DAM LOGGER_log("%s:%d:AM_insert_disclaimer_into_segment:DEBUG: Conditions not right to insert HTML disclaimer",FL);
	}

	/** If we weren't able to insert a disclaimer, then read through to the end of the segment **/
	if (insert_success == 0) {
		while (FFGET_fgets(line, AM_1K_BUFFER_SIZE, f)) {
			last_boundary_written = 0;
			fprintf( newf, "%s", line );
			if ( BS_cmp(line,strlen(line))==1 ) {
				DAM LOGGER_log("%s:%d:AM_insert_disclaimer_into_segment:DEBUG: Boundary hit, breaking out",FL);
				break;
			}
		}
	}

	return 0;
}

/*------------------------------------------------------------------------
Procedure:     AM_add_disclaimer ID:1
Purpose:       Adds a disclaimer to [potentially] plain-text, mixed/multipart and HTML emails.
Input:         char *mpackname: Mailpack/email name we're modifying
char *disclaimer: either the text, or the filename of the text we want to insert
int istext: 0 = dislciamer is text, 1 = disclaimer is filename
Output:
Errors:
------------------------------------------------------------------------*/
int AM_add_disclaimer( char *mpackname )
{
	FILE *mp, *newf;
	FFGET_FILE f;
	char line[AM_1K_BUFFER_SIZE+1]="";
	char mpacktmp[AM_1K_BUFFER_SIZE+1]="";
	char mpackold[AM_1K_BUFFER_SIZE+1]="";
	struct AM_disclaimer_details dd;
	int result = 0;
	int segment_read = 0;

	/* create our temp filename */
	snprintf(mpacktmp,AM_1K_BUFFER_SIZE, "%s.tmp",mpackname);
	snprintf(mpackold,AM_1K_BUFFER_SIZE, "%s.old",mpackname);

	altermime_status_flags = 0; // clear the global status flags

	if (strcmp(mpackname,"-")==0)
	{
		mp = stdin;
		newf = stdout;
	} else {
		/* Initialise our files */
		newf = fopen(mpacktmp,"w");
		mp = fopen(mpackname,"r");
	}

	/* Initialise the Boundary-stack globals */
	BS_init();
	/* Allow 10 boundaries to be stored */
	BS_set_hold_limit(10);

	// Test file statuses... and hop out if we had troubles with either
	//
	if (!newf)
	{
		LOGGER_log("AM_add_disclaimer: Cannot open %s, %s",mpacktmp,strerror(errno));
		return -1;
	}

	if (!mp)
	{
		LOGGER_log("AM_add_disclaimer: Cannot open %s, %s",mpacktmp,strerror(errno));
		return -1;
	}


	// Set up the disclaimer text as required to be inserted into the file
	//
	result = AM_load_disclaimers( &dd );
	if (result == -1)
		return 111; // Just exit if we couldn't load the disclaimers.


	// Initialise our variables
	dd.content_type = _CTYPE_UNKNOWN;
	dd.content_encoding = _CTRANS_ENCODING_UNKNOWN;
	dd.boundary_found = 0;
	memset(dd.boundary, 0, sizeof(dd.boundary));
	dd.ishtml = 0;
	dd.isfile = 0;
	dd.text_inserted = 0;
	dd.html_inserted = 0;
	dd.b64_inserted = 0;


	VAM LOGGER_log("Attempting to add disclaimer");

	FFGET_setstream(&f, mp);

	// The process....
	//
	//	First, we read through the email until we at least come across the traditional
	//  true-blank seperator line between the headers and the body of the email.
	//  Once we have found that, we can then choose how we're going to detect our first
	//  disclaimer position based on what we discovered in the headers (ie, did we
	//  find a boundary specifier? are we using plain-text here, or does the email have
	//  HTML in it?
	//
	//  If we have just a plain text email, then we wait till we reach the end of the email
	//  and we just append the disclaimer
	//
	//  If we [most likely] dont have a plain email, then we need to wait until we find
	//  the boundary line, then, insert the disclaimer there.
	//
	//  Next, we need to check to see if the NEXT section is the HTML version of the first
	//  section (well, we dont /really/ compare them, but we check the headers to see if
	//  it has things like text/html and no filename
	//  If the section is HTML and non-filenamed, we once more wait until the boundary then
	//  we add in a HTML version of the disclaimer.
	//
	// Sounds easy ?
	//
	//	Note - It should be pointed out also, that alterMIME will not insert disclaimers into what
	//		are just forwarded emails [ Content-Type: message/rfc822 ].
	//
	// Note - the boundary is ONLY taken from the main headers. This is to prevent us
	//		attempting to dive in deeper into the email adding disclaimers to parts which
	//		were not generated by the last mail user agent.


	// Section 1 - read in the MAIN headers until a true-blank.
	//		This will give us some indication of what to expect in the email, be it a multipart
	//		a file, or a plain text email.
	//

	DAM LOGGER_log("%s:%d:AM_add_disclaimer:DEBUG: Reading main headers",FL);

	result = AM_read_headers( &dd, &f, newf );
	DAM LOGGER_log("%s:%d:AM_add_disclaimer:DEBUG: Main Headers have been read",FL);

	// Check to see if we can actually insert into this
	//		email's encoding.
	//
	if (dd.content_encoding == _CTRANS_ENCODING_B64)
	{
		if (
				(dd.content_type == _CTYPE_TEXT_HTML)
				||(dd.content_type == _CTYPE_TEXT_PLAIN)
			)
		{
			/* NAB-SPECIAL
			 * If we've got a base64 encoded email right up front, then we will try
			 * to decode it, insert disclaimer then reinsert.
			 *
			 * 1. Detect
			 * 2. Save B64 segment to disk
			 * 3. Decode B64 segment
			 * 4. Insert disclaimer into the end of the decoded segment
			 * 5. Encode segment in B64
			 * 6. Insert new B64 code into current output file position
			 */
			DAM LOGGER_log("%s:%d:AM_add_disclaimer:DEBUG: B64 encoded body text(s)",FL);

			/** INSERT NAB CODE **/
		} else {


			AM_add_disclaimer_flush( &f, newf );
			/** 20050626-0111: Patch to remove tmp file on failure supplied by Carlos Velasco **/
			fclose(newf);
			remove(mpacktmp);
			/** endpatch **/
			VAM LOGGER_log("Email is BASE64 encoded, we cannot insert into these emails\n");
			return AM_RETURN_B64_ENCODED_EMAIL;
		}
	}

	// Check to see if the email is signed
	//
	//
	if ((AM_RETURN_SIGNED_EMAIL == result)||(-1 == result))
	{
		AM_add_disclaimer_flush( &f, newf );
		fclose(mp);
		fclose(newf);
		remove(mpacktmp);		// Remove the temporary file - patch supplied by Carlos Velasco
		VAM LOGGER_log("Email is signed with a crypto, we do not alter these emails\n");
		return AM_RETURN_SIGNED_EMAIL;
	}

	// If we have no boundary, and we have either text or no content-type
	// then we can just append the disclaimer to the /end/ of the email
	//&&((dd.content_type >= _CTYPE_TEXT_START && dd.content_type <= _CTYPE_TEXT_END ))

	if (
			(0 == dd.boundary_found)
			&&(0 == dd.isfile)
		)
	{
		DAM LOGGER_log("%s:%d:AM_add_disclaimer:DEBUG: Inserting disclaimer into an email with no boundary",FL);
		result = AM_add_disclaimer_no_boudary( &f, newf, &dd );
		DAM LOGGER_log("%s:%d:AM_add_disclaimer:DEBUG: Inserting done, txt-inserted=%d html-inserted=%d"
				,FL
				,dd.text_inserted
				,dd.html_inserted
				);
	}


	// BOUNDARY LIMITED EMAILS -----------------
	//
	// These emails are somewhat /harder/ to insert attachments into
	// namely because of the profound number of convoulted ways in which
	// email clients can forward/reply/mangle emails.
	//
	// We use a 'WHILE' loop here just so that we can cheat a bit when
	// trying to /exit/ out of the processing prematurely when we run
	// out of input (as happens at the end of the input file.
	//
	// By using a 'WHILE', we can just use 'break', rather than resorting
	// to other ugly methods :)


	if ( dd.boundary_found == 1 )
	{
		do { /* DO loop is used here so that we can 'break' out without
				  having to resort to ugly GOTO hacks or similar
				  */


			segment_read = 0;
			if (FFGET_feof(&f)) break;

			// If we've found a boundary and a text content section...
			//

			DAM LOGGER_log("%s:%d:AM_add_disclaimer:DEBUG: Inserting into a MIME email\n",FL);


			// Step 1 - first read off any spurilous data prior to the first
			// 	MIME block, this typically includes things like
			//   "If you can read this, then your email client is not MIME
			//	  compliant"  (etc).

			DAM LOGGER_log("%s:%d:AM_add_disclaimer:DEBUG: Reading spurilous data before first MIME segment\n",FL);

			AM_read_to_boundary( &f, newf, line, AM_1K_BUFFER_SIZE );
			if (FFGET_feof(&f))
			{
				DAM LOGGER_log("%s:%d:AM_add_disclaimer:DEBUG: End of file is reached, pulling out early",FL);
				break;
			}


			// Read SEGMENT 1 headers - and attempt to insert disclaimer
			if (AM_read_headers( &dd, &f, newf ) == 0)
			{
				/** Did we stumble on a boundary, and was it a non-RFC822 form 
				 ** (ie, not likely a forwarded email ) **/
				if ((dd.boundary_found == 1)&&(dd.content_type != _CTYPE_RFC822))
				{
					DAM LOGGER_log("%s:%d:AM_add_disclaimer:DEBUG: Located a new non-RFC822 boundary, adding to list\n",FL);
				}


				DAM LOGGER_log("%s:%d:AM_add_disclaimer:DEBUG: inserting into SEGMENT 1",FL);
				AM_insert_disclaimer_into_segment( &f, newf, &dd );
				DAM LOGGER_log("%s:%d:AM_add_disclaimer:DEBUG: Finished inserting into SEGMENT 1",FL);
			} else {
				DAM LOGGER_log("%s:%d:AM_add_disclaimer: Cannot read headers for SEGMENT 1",FL);
			}


			// Read SEGMENT 2 headers - and attempt to insert disclaimer
			if (AM_read_headers( &dd, &f, newf ) == 0)
			{
				/** Did we stumble on a boundary, and was it a non-RFC822 form 
				 ** (ie, not likely a forwarded email ) **/
				if ((dd.boundary_found == 1)&&(dd.content_type != _CTYPE_RFC822))
				{
					DAM LOGGER_log("%s:%d:AM_add_disclaimer:DEBUG: Located a new non-RFC822 boundary, adding to list\n",FL);
				}

				DAM LOGGER_log("%s:%d:AM_add_disclaimer:DEBUG: inserting into SEGMENT 2",FL);
				AM_insert_disclaimer_into_segment( &f, newf, &dd );
				DAM LOGGER_log("%s:%d:AM_add_disclaimer:DEBUG: Finished inserting into SEGMENT 2",FL);
			} else {
				DAM LOGGER_log("%s:%d:AM_add_disclaimer: Cannot read headers for SEGMENT 2",FL);
			}


			{
				int depth=0; // how many more headers to read before giving up entirely...
				do {

					// Read SEGMENT 3 headers - and attempt to insert disclaimer, typically we do this
					//		to handle TXT + HTML combo emails

					if (AM_read_headers( &dd, &f, newf ) == 0)
					{
						/** Did we stumble on a boundary, and was it a non-RFC822 form 
						 ** (ie, not likely a forwarded email ) **/
						if ((dd.boundary_found == 1)&&(dd.content_type != _CTYPE_RFC822))
						{
							DAM LOGGER_log("%s:%d:AM_add_disclaimer:DEBUG: Located a new non-RFC822 boundary, adding to list\n",FL);
						}

						if (dd.content_type == _CTYPE_RFC822) {
							DAM LOGGER_log("%s:%d:AM_add_disclaimer:DEBUG: Located RFC822 type of header, so no more attempts to insert disclaimers",FL);
							break; // out of the do.
						}

						DAM LOGGER_log("%s:%d:AM_add_disclaimer:DEBUG: inserting into SEGMENT %d",FL, 3+depth);
						AM_insert_disclaimer_into_segment( &f, newf, &dd );
						DAM LOGGER_log("%s:%d:AM_add_disclaimer:DEBUG: Finished inserting into SEGMENT %d",FL, 3+depth);
					} else {
						DAM LOGGER_log("%s:%d:AM_add_disclaimer: Cannot read headers for SEGMENT %d",FL, 3+depth);
					}
				} while (( dd.html_inserted == 0 )&&( depth++ < 3 ));
			}

		} while (0);
	}


	// Clean up any remaining lines in the email
	AM_add_disclaimer_flush( &f, newf );


	VAM LOGGER_log("Done.\n");


	AM_add_disclaimer_cleanup( mp, newf, mpacktmp, mpackname);


	// Free the malloc'd text if required
	if ((glb.disclaimer_plain_type == AM_DISCLAIMER_TYPE_FILENAME)&&(dd.disclaimer_text_plain))
	{
		free(dd.disclaimer_text_plain);
	}

	if ((glb.disclaimer_HTML_type == AM_DISCLAIMER_TYPE_FILENAME)&&(dd.disclaimer_text_HTML))
	{
		free(dd.disclaimer_text_HTML);
	}

	DAM LOGGER_log("%s:%d:AM_add_disclaimer:DEBUG: Inserting done, txt-inserted=%d html-inserted=%d b64-inserted=%d"
			,FL
			,dd.text_inserted
			,dd.html_inserted
			,dd.b64_inserted
			);

	altermime_status_flags |= (dd.text_inserted<< AMSTATUSFLAGS_TEXT_INSERTED)|(dd.html_inserted<< AMSTATUSFLAGS_HTML_INSERTED)|(dd.b64_inserted<< AMSTATUSFLAGS_B64_INSERTED);

	return result;
}






/*------------------------------------------------------------------------
Procedure:     AM_filename_fix ID:1
Purpose:       Converts a MIME header
Input:         line: string with the filename
removed_prefix: filename prefix to use
removed_count: the number of attachments we've processed / ID
Output:
Errors:
------------------------------------------------------------------------*/
int AM_filename_fix( char *line, char *removed_prefix, int removed_count )
{
	char *pos;
	char lline[AM_1K_BUFFER_SIZE];

	/* process the current line, as it contains the filename, and replace the filename
	 * with a -removed- one */
	pos = strrchr(line,'='); /* locate where the filename starts */
	pos++; /* move one char along */
	*pos = '\0'; /* terminate the string */

	sprintf(lline,"%s\"%s%d\"\n",line,removed_prefix,removed_count); /* create a new string, in lline (scratch pad)*/
	strcpy(line,lline);

	return 0;
}

/*-----------------------------------------------------------------\
  Function Name	: AM_headers_remove_header
  Returns Type	: int
  ----Parameter List
  1. char *headers, 
  2.  char *header_name , 
  ------------------
  Exit Codes	: 
  Side Effects	: 
  --------------------------------------------------------------------
Comments:

--------------------------------------------------------------------
Changes:

\------------------------------------------------------------------*/
int AM_headers_remove_header( char *headers, char *header_name )
{
	char delimeter_size;
	char *segment_start, *segment_end;
	char *p;

	if (AM_DNORMAL)LOGGER_log("%s:%d:AM_headers_remove_header:DEBUG: Removing %s", FL, header_name );

	delimeter_size=1;
	if (strstr(headers,"\n\r") != NULL) delimeter_size=2;
	if (strstr(headers,"\r\n") != NULL) delimeter_size=2;

	segment_start = PLD_strstr( headers, header_name, 1 );
	if (segment_start == NULL) return 1;

	segment_end = segment_start +strlen( header_name );
	do {
		segment_end = strpbrk( segment_end, "\n\r" );
		if (delimeter_size==2) segment_end+=2; else segment_end++;
	} while (*segment_end==' '||*segment_end=='\t');

	p = segment_end;
	while (*p)
	{
		*segment_start = *p;
		segment_start++;
		p++;
	}

	*segment_start = '\0';

	return 0;
}

/*-----------------------------------------------------------------\
  Function Name	: AM_nullify_attachment_clean_headers
  Returns Type	: int
  ----Parameter List
  1. struct MIMEH_header_info *hinfo, 
  2.  char *headers , 
  ------------------
  Exit Codes	: 
  Side Effects	: 
  --------------------------------------------------------------------
Comments:

--------------------------------------------------------------------
Changes:

\------------------------------------------------------------------*/
int AM_nullify_attachment_clean_headers( struct MIMEH_header_info *hinfo, char *headers )
{

	AM_headers_remove_header( headers, "content-type:" );
	AM_headers_remove_header( headers, "content-disposition:");
	AM_headers_remove_header( headers, "content-transfer-encoding");
	/*
		char *p;
		char *q;

		q = headers;
		p = strstr( q, hinfo->filename );
		while (( p != NULL )&&( p > q ))
		{
		if (*p == '_') *p = 'X'; else *p = '_';
		q = p+1;
		p = strstr( q, hinfo->filename );
		}
		*/
	return 0;
}


/*-----------------------------------------------------------------\
  Function Name	: AM_nullify_attachment_recurse
  Returns Type	: int
  ----Parameter List
  1. struct MIMEH_header_info *hinf, 
  2.  FFGET_FILE *f, 
  3.  FILE *outputfile, 
  4.  pregex_t *preg , 
  ------------------
  Exit Codes	: 
  Side Effects	: 
  --------------------------------------------------------------------
Comments:

--------------------------------------------------------------------
Changes:

\------------------------------------------------------------------*/
int AM_nullify_attachment_recurse( struct MIMEH_header_info *hinfo, FFGET_FILE *f, FILE *outputfile, regex_t *preg, int match_mode, int iteration )
{
	int result = 0;
	size_t bc;

	if (AM_DNORMAL) LOGGER_log("%s:%d:AM_nullify_attachment_recurse: Checking segment-iteration '%d'",FL,iteration);
	while (1)
	{
		int regresult=0;
		char *header_ptr=NULL;
		char *original_ptr=NULL;
		char buffer[1024];

		MIMEH_set_doubleCR_save(0);
		MIMEH_set_header_longsearch(glb.header_long_search);
		MIMEH_set_doubleCR_save(1);

		DAM LOGGER_log("%s:%d:AM_nullify_attachment_recurse:DEBUG: Reading headers... Iteration %d",FL);
		result = MIMEH_headers_get( hinfo, f );
		if (result != 0)
		{ 
			DAM LOGGER_log("%s:%d:AM_nullify_attachment_recurse:DEBUG: Failure during header read (EOF?), exiting loop",FL);
			break;
		}

		/** If we detect a signed message and we're not explicitly wanting to alter them, then exit **/
		if ((hinfo->content_type == _CTYPE_MULTIPART_SIGNED)&&(glb.alter_signed==0)) {
			DAM LOGGER_log("%s:%d:AM_nullify_attachment_recurse:DEBUG: Message is signed, exiting",FL);
			return 0;
		}

		original_ptr = MIMEH_get_headers_original_ptr();
		header_ptr = MIMEH_get_headers_ptr();

		if (AM_DNORMAL)LOGGER_log("%s:%d:AM_nullify_attachment_recurse:DEBUG: Headers =\n%s\n-----------END OF HEADERS\n",FL, original_ptr );

		if (original_ptr == NULL)
		{
			LOGGER_log("%s:%d:AM_nullify_attachment_recurse:ERROR: Original headers came back NULL",FL);
			return 1;
		}

		if (header_ptr == NULL)
		{
			LOGGER_log("%s:%d:AM_nullify_attachment_recurse:ERROR: Header ptr (for processing) came back NULL",FL);
			return 1;
		}

		result = MIMEH_headers_process( hinfo, header_ptr );
		if (result != 0)
		{
			LOGGER_log("%s:%d:AM_nullify_attachment:ERROR: While processing headers for mailpack", FL );
			break;
		}

		// Check to see if we have a new boundary
		if ((hinfo->content_type >= _CTYPE_MULTIPART_START && hinfo->content_type <= _CTYPE_MULTIPART_END)\
				&& (hinfo->boundary_located > 0))
		{
			BS_push( hinfo->boundary );
		}

		// Now, determine if this block/segment is the one which contains our file which we must 'nullify'
		regresult = 1;
		switch (match_mode) {
			case AM_NULLIFY_MATCH_MODE_FILENAME:
				if (strlen(hinfo->filename) > 0)
				{
					regresult = regexec( preg, hinfo->filename, 0, NULL, 0 );
					if (AM_DNORMAL)LOGGER_log("%s:%d:AM_nullify_attachment: Match result=%d for '%s'", FL, regresult, hinfo->filename);
				} 
				if ((regresult != 0)&&(strlen(hinfo->name) > 0))
				{
					regresult = regexec( preg, hinfo->name, 0, NULL, 0 );
					if (AM_DNORMAL)LOGGER_log("%s:%d:AM_nullify_attachment: Match result=%d for '%s'", FL, regresult, hinfo->name);
				}
				break;
			case AM_NULLIFY_MATCH_MODE_CONTENT_TYPE:
				if (strlen(hinfo->content_type_string) > 0)
				{
					regresult = regexec( preg, hinfo->content_type_string, 0, NULL, 0 );
					if (AM_DNORMAL)LOGGER_log("%s:%d:AM_nullify_attachment: Match result=%d for '%s'", FL, regresult, hinfo->content_type_string);
				}
				break;
			default:
				LOGGER_log("%s:%d:AM_nullify_attachment: unknown Nullify match mode '%d'", FL, match_mode);
		}


		// If we're on our first pass, or we've not found the section/block with the attachment
		//	then write the headers out.
		//
		//	 A bit of an issue which comes up is, what to do if the entire email is the attachment
		//	ie, it's just a something which was 'right-click->email'd.
		//
		//	Until I come up with a better solution, I'll still write the headers, but I'll then 
		//		eliminate all the content [until the next boundary, assuming a boundary exists]

		if ((regresult != 0)||(iteration == 1))
		{
			int sl;
			// If we've got a match which is in the first section of the email, then
			//		we have to 'modify' the headers rather than not writing them at all.
			//		this way, we still keep the email structure intact, but we make it so
			//		that the attachment-related portions of the email changed.
			//
			if ((iteration == 1)&&(regresult == 0))
			{
				AM_nullify_attachment_clean_headers( hinfo, original_ptr );
			}

			sl = strlen(original_ptr);

			bc = fwrite( original_ptr, sizeof(char), sl, outputfile );
			if (bc != sl) LOGGER_log("%s:%d:AM_nullify_attachment_clean_headers:ERROR: Wrote %d bytes instead of %d", FL, bc, sl);
		}

		// Clean up the memory allocation
		result = MIMEH_headers_cleanup();
		if (result != 0)
		{
			LOGGER_log("%s:%d:AM_nullify_attachment:ERROR: while attempting to clean up headers memory allocation", FL );
			break;
		}

		// Now, if we have a Multipart/Mixed attachment, then, alas, we need to recurse into
		//		it and see if it contains anything interesting for us to seek out.
		if (hinfo->content_type == _CTYPE_RFC822)
		{
			result=AM_nullify_attachment_recurse( hinfo, f, outputfile, preg, match_mode, 1 );
		}

		// Now, we shall go through and read the email until we happen across another boundary.
		while (FFGET_fgets(buffer, sizeof(buffer), f))
		{
			int buffer_len = strlen(buffer);

			if (regresult != 0)
			{
				bc = fwrite( buffer, sizeof(char), buffer_len, outputfile );
				if (bc != buffer_len) LOGGER_log("%s:%d:AM_nullify_attachment_recurse:ERROR: Wrote %d bytes instead of %d", FL, bc, buffer_len);
			}

			if (BS_cmp( buffer, buffer_len ) == 1) {
				break;
			}
		} // While ffgets

		if (FFGET_feof(f)) break;

		iteration++;

	} // Infinite while(1)

	return 0;
}


/*------------------------------------------------------------------------
Procedure:     AM_nullify_attachment ID:1
Purpose:       Removes any attachment within the mailpack which matches
(minimally) with the attachmentname parameter
Input:         mpackname - The mailpack name
attachmentname - The name of attachments which is to be removed
Output:        none
Errors:        If attachment wasn't found, or could not be removed, a non-zero return value.
------------------------------------------------------------------------*/
int AM_nullify_attachment( char *mpackname, char *attachmentname )
{
	struct MIMEH_header_info hinfo;
	regex_t preg;
	int result = 0;
	int match_mode = AM_NULLIFY_MATCH_MODE_NONE;
	char tmpfname[256];
	char oldfname[256];
	FILE *inputfile;
	FILE *outputfile;
	FFGET_FILE f;

	// Nullifying an attachment can sometimes be a little bit tricky, we have to
	//		dig down into the nesting of the MIME email and keep tabs on our
	//		boundaries (via a Boundary-stack).  This all requires about as much work
	//		as ripMIME just to get rid of one attachment.
	//
	// Additionally - we require some special functionality from MIME_headers, so
	//		so that we can hold-off saving the headers to the file until we've 
	//		checked them to see if we want them or not.

	if (AM_DNORMAL) LOGGER_log("%s:%d:AM_nullify_attachment: Starting nullification of file '%s' from mailpack '%s'",FL, attachmentname, mpackname);
	BS_init();

	if (strcmp( mpackname, "-") == 0) {
		inputfile = stdin;
	} else {
		inputfile = fopen( mpackname, "r" );
	}

	if (inputfile == NULL) {
		LOGGER_log("%s:%d:AM_nullify_attachment: Unable to open mailpack '%s' for reading (%s)", FL, mpackname, strerror(errno));
		return 1;
	}

	snprintf( tmpfname, sizeof(tmpfname), "%s.tmp", mpackname );

	if (strcmp( mpackname, "-") == 0) {
		outputfile = stdout;
	} else {
		outputfile = fopen( tmpfname, "w" );
	}

	if (outputfile == NULL)
	{
		if (inputfile != NULL) fclose(inputfile);
		LOGGER_log("%s:%d:AM_nullify_attachment: Unable to open temporary file '%s' for writing (%s)", FL, tmpfname, strerror(errno));
		return 1;
	}

	FFGET_setstream( &f, inputfile );

	MIMEH_set_headers_nosave();
	MIMEH_set_headers_save_original(1);


	// Determine which mode of matching we'll be using
	if (strchr(attachmentname, '/') == NULL) {
		match_mode = AM_NULLIFY_MATCH_MODE_FILENAME;
	} else {
		match_mode = AM_NULLIFY_MATCH_MODE_CONTENT_TYPE;
	}

	// Compile our Regular-expression for the filename.
	result = regcomp( &preg, attachmentname, REG_EXTENDED|REG_ICASE|REG_NOSUB );
	if (result != 0)
	{
		LOGGER_log("%s:%d:AM_nullify_attachment: Unable to compile regular expression '%s'", FL, attachmentname );
		return 0;
	}

	SS_init(&(hinfo.ss_filenames));
	SS_init(&(hinfo.ss_names));
	result=AM_nullify_attachment_recurse( &hinfo, &f, outputfile, &preg, match_mode, 1 );

	MIMEH_set_headers_save_original(0);	

	regfree(&preg);
	FFGET_closestream(&f); // PLD-20070718-1841: We're done with the FFGET bits now, close this, possible truncation cause
	fclose(inputfile);
	fclose(outputfile);

	snprintf(oldfname,sizeof(oldfname),"%s.old", mpackname);

	if (strcmp( mpackname, "-") != 0)
	{
		result = rename( mpackname, oldfname );

		if ( result != 0 ) 
		{
			LOGGER_log("%s:%d:AM_nullify_attachment_recurse:ERROR: Unable to rename original mailpack '%s' to '%s' (%s)", FL, mpackname, oldfname, strerror(errno));
			return 1;
		}

		result = rename( tmpfname, mpackname );
		if (result != 0)
		{
			LOGGER_log("%s:%d:AM_nullify_attachment_recurse:ERROR: Unable to rename temporary mailpack '%s' to '%s' (%s)", FL, tmpfname, mpackname, strerror(errno));
			return 1;
		}	

		result = unlink( oldfname );
		if ( result != 0)
		{
			LOGGER_log("%s:%d:AM_nullify_attachment_recurse:ERROR: Unable to unlink/remove '%s' (%s)", FL, oldfname, strerror(errno));
			return 1;
		}
	}

	// Clean up our boundary stack 
	BS_clear();

	return result;
}


/*------------------------------------------------------------------------
Procedure:     AM_insert_Xheader ID:1
Purpose:       Adds an X-header to the first set of headers in an email mailpack.
Input:         char *fname: mailpack name
char *xheader: header string
Output:
Errors:
------------------------------------------------------------------------*/
int AM_insert_Xheader( char *fname, char *xheader)
{

	/* Tempfile tmpfile() fix contributed by David DeMaagd - 29/01/2001 */

	char line[ AM_1K_BUFFER_SIZE +1];
	char tpn[ AM_1K_BUFFER_SIZE +1];
	int header_written = 0;
	int result = 0;
	struct stat st;
	FFGET_FILE f;
	FILE *fi;
	FILE *fo;

	// Sanity checks
	//	
	//	. check that the x-header string is valid
	//	. check the x-header to ensure there's no \r or \n's
	//

	if (!fname) {
		LOGGER_log("%s:%d:AM_insert_Xheader:ERROR: Filename is NULL",FL);
		return 1;
	}

	if (!xheader) {
		LOGGER_log("%s:%d:AM_insert_Xheader:ERROR: Xheader to insert is NULL",FL);
		return 1;
	}

	if (strlen(fname) < 1) {
		LOGGER_log("%s:%d:AM_insert_Xheader:ERROR: Filename is too short",FL);
		return 1;
	}

	if (strlen(xheader) < 1) {
		LOGGER_log("%s:%d:AM_insert_Xheader:ERROR: Header to insert is too short",FL);
		return 1;
	}

	if (1)
	{
		/** Strip off any trailing line breaks **/
		char *p;
		p = strpbrk(xheader,"\r\n");
		if (p) *p = '\0';
	}

	// Create a temporary file name, but, so that we dont
	// overwrite an existing file, we must check the name using
	// stat() to see if we get a non-zero response.

	snprintf(tpn, AM_1K_BUFFER_SIZE, "%s",fname);

	do {
		if (strlen(tpn) < (sizeof(tpn) -2))
		{
			/** Changed the temp filename suffix chat to a hypen because under
			 ** windows appending multiple .'s results in a filename that isn't
			 ** acceptable - Thanks to Nico for bringing this to my attention **/
			//			LOGGER_log("%s:%d:AM_insert_Xheader:NOTICE: Adjusting temp file name for header insert",FL);
			strcat(tpn,"X");

		} else {
			LOGGER_log("%s:%d:AM_insert_Xheader:ERROR: Temporary file name string buffer out of space!",FL);
			return 1;
		}
		//		LOGGER_log("DEBUG:%s:%d: Testing filename %s\n",__FILE__,__LINE__,tpn);
	} while (0 == stat(tpn,&st));


	// Attempt to open up the temporary file in write mode.
	// If the open fails, then this whole operation fails.
	// Ensure there's lots of good logging here so that when something
	// does go wrong, at least people wont be left in the dark as to
	// what went on.
	//
	// Same applies for opening the source file
	//

	fo = fopen(tpn,"w");
	if (!fo)
	{
		LOGGER_log("%s:%d:AM_insert_Xheader:ERROR: Cannot open temporary file, '%s' for writing (%s)",FL, tpn, strerror(errno));
		return 1;
	} else {
		//	LOGGER_log("%s:%d:AM_insert_Xheader:NOTICE: opened '%s' for writing", FL, tpn);
	}

	fi = fopen(fname,"r");
	if (!fi)
	{
		LOGGER_log("%s:%d:AM_insert_Xheader:ERROR: Cannot open Mailpack file '%s' for reading, (%s)",FL, fname, strerror(errno));
		return 2;
	} else {
		//	LOGGER_log("%s:%d:AM_insert_Xheader:NOTICE: opened '%s' for reading", FL, fname);
	}

	//setup our FFGET stream

	FFGET_setstream(&f, fi);
	f.trueblank = 0;


	// Load and go through every line in the email file, testing it for "trueblank"
	// status.  Once a trueblank is found, we write our header, as this marks the
	// end of the headers (the trueblank).  Beyond that, we keep on reading and
	// writing because we obviously want to keep the entire email content.

	while (FFGET_fgets(line, AM_1K_BUFFER_SIZE ,&f))
	{


		// If we've found that the line we just read is a /true/ blank (as apposed to a carefully
		// crafted line length to catch out our fgets(), /and/ if we've not written in a header
		// then we will insert the header.

		if ((0 != f.trueblank)&&(0 == header_written))
		{
			/** 20041104-12H52:PLD: Changed from \n to \r\n **/
			/** 20050204-11H04:PLD: Changed from \r\n to instead use the 'blank line', this ensures
			 ** that the right \r\n or \n combination is used **/
			fprintf(fo,"%s%s",xheader,line);
			header_written = 1;
		}

		// In all cases, we must write out the line we read in, even if it was the true-blank line
		// Otherwise we'll end up losing a the header seperation, and all of a sudden your emails
		// do not make any more sense to email programs.

		fprintf(fo,"%s",line);
	}

	// Close our input files

	FFGET_closestream(&f); // PLD-20070718-1842

	fclose(fo);
	fclose(fi);

	// We now can rename the temporary file to the new file name.
	// NOTE - previously we removed then renamed, however, on checking
	// with the manpages, 'man 2 rename' it seems it's not required, so
	// rather than waste CPU cycles, we'll just go by the book, and use
	// only rename.  There are a couple of situations where rename can/will
	// fail, obviously such as if the original file is marked read-only, or
	// we do not have write permissions to it.


	if (rename(tpn,fname) == -1)
	{
		result = 1;
		LOGGER_log("%s:%d:AM_insert_Xheader:ERROR: while attempting to rename '%s' to '%s' (%s) ", FL, tpn, fname, strerror(errno));
	}

	if (header_written) altermime_status_flags |= (1<<AMSTATUSFLAGS_XHEADER_INSERTED);

	return result;
}




/*------------------------------------------------------------------------
Procedure:     AM_header_adjust ID:1
Purpose:       Adjusts an -existing- header 
Input:         
char *xheader: header string
Output:
Errors:
------------------------------------------------------------------------*/
int AM_alter_header( char *filename, char *header, char *change, int change_mode )
{

	char line[ AM_1K_BUFFER_SIZE +1];
	char tpn[ AM_1K_BUFFER_SIZE +1];
	int main_headers_complete = 0;
	int header_written = 0;
	int result = 0;
	struct stat st;
	FFGET_FILE f;
	FILE *fi;
	FILE *fo;

	// Create a temporary file name, but, so that we dont
	// overwrite an existing file, we must check the name using
	// stat() to see if we get a non-zero response.

	if (AM_DNORMAL) LOGGER_log("%s:%d:AM_alter_header:DEBUG: Start [ %s, %s, %s, %d ]", FL, filename, header, change, change_mode );

	snprintf(tpn, AM_1K_BUFFER_SIZE, "%s",filename);

	do {
		if (strlen(tpn) < (sizeof(tpn) -2))
		{
			strcat(tpn,"X");
		}
		else
		{
			LOGGER_log("%s:%d:AM_header_adjust:ERROR: Temporary file name string buffer out of space!\n",FL);
			return 1;
		}
	}
	while (0 == stat(tpn,&st));

	if (AM_DNORMAL) LOGGER_log("%s:%d:AM_alter_header:DEBUG: Temporary filename = %s", FL, tpn );

	// Attempt to open up the temporary file in write mode.
	// If the open fails, then this whole operation fails.
	// Ensure there's lots of good logging here so that when something
	// does go wrong, at least people wont be left in the dark as to
	// what went on.
	//
	// Same applies for opening the source file
	//

	fo = fopen(tpn,"w");
	if (!fo)
	{
		LOGGER_log("%s:%d:AM_header_adjust:ERROR: Cannot open temporary file '%s' for writing",FL,tpn, strerror(errno));
		return 1;
	}


	fi = fopen(filename,"r");
	if (!fi)
	{
		LOGGER_log("%s:%d:AM_header_adjust:ERROR: Cannot open Mailpack file for reading, %s",FL,filename,strerror(errno));
		return 2;
	}

	//setup our FFGET stream

	FFGET_setstream(&f, fi);

	// Convert the header we're looking for into lower case so that 
	// we make it esaier to search for

	PLD_strlower( header );

	// Load and go through every line in the email file, testing it for "trueblank"
	// status.  Once a trueblank is found, we write our header, as this marks the
	// end of the headers (the trueblank).  Beyond that, we keep on reading and
	// writing because we obviously want to keep the entire email content.

	if (AM_DNORMAL) LOGGER_log("%s:%d:AM_header_adjust:DEBUG: Starting seek through file. header_written = %d", FL, header_written );
	while (FFGET_fgets(line, AM_1K_BUFFER_SIZE ,&f))
	{
		int line_written = 0;

		// If we find a true-blank line, then we note that the main-headers section is now
		//	complete - We do not process any headers after this, as the rest of the data
		//	is the email body/content [ which we do not wish to alter ].

		if ((main_headers_complete == 0)&&(f.trueblank == 1)) main_headers_complete = 1;


		// If we're still processing the main headers, and we've not written/altered our
		//	particular header which we wish to alter, then proceed to check this newly
		//	read line to see if it is the one we wish to alter,
		//
		//	Irrespective of if we alter this line or not, it will be written at the bottom
		//	of this while-loop.  NOTE - we don't write the line within the header-alteration
		//	"if" block, as this would only make things more complicated on exit ( ie, did we
		//	write the line, or are we okay... more logic tests, more chance of a bungle ).

		if ((main_headers_complete == 0)&&(header_written == 0))
		{
			char *p;
			char low_line[AM_1K_BUFFER_SIZE +1];

			if (AM_DNORMAL) LOGGER_log("%s:%d:AM_header_adjust: line=%s",FL, line);

			// Make a copy of the header line, and convert it to lowercase so that we
			//	can make a string comparison

			snprintf( low_line, AM_1K_BUFFER_SIZE, "%s", line );
			PLD_strlower( low_line );

			p = strstr(low_line, header);
			if (p != NULL)
			{
				char *first_colon_position;

				// Because it's quite possible to put a heaer line in which replicates itself, such as...
				//	Subject:  the Subject: is foo
				//	we need to first check that the header position we got from strstr is /BEFORE/ the first
				//	colon in the line.

				first_colon_position = strchr(low_line, ':');
				if ( p < first_colon_position )
				{
					char *header_start, *header_end;
					char startc, finishc;

					// Convert the position in the low_line string over to that
					//	of the normal 'line' string.

					p = line +(p - low_line);

					if (AM_DNORMAL) LOGGER_log("%s:%d:AM_header_adjust:DEBUG: Located header line",FL);
					header_start = strchr(p,':');
					if ( header_start == NULL )
					{
						LOGGER_log("%s:%d:AM_header_adjust:WARNING: Could not locate terminating ':' character on header name (%s)",FL,p);
						continue;
					}
					header_start++;
					startc = *header_start;

					header_end = strpbrk( header_start, "\r\n;" );
					if ( header_end == NULL )
					{
						LOGGER_log("%s:%d:AM_header_adjust:WARNING: Could not locate end of header value (%s)",FL,header_start);
						continue;
					}
					finishc = *header_end;

					// We now have the start and end points of the header string, we now place in the additional 
					// data ( or entirely new data ) based on what the mode is

					switch ( change_mode ) {
						case AM_HEADER_ADJUST_MODE_PREFIX:
							*header_start = '\0';
							fprintf(fo,"%s %s%c%s", line, change, startc, header_start+1);
							header_written = 1;
							line_written = 1;
							if (AM_DNORMAL) LOGGER_log("%s:%d:AM_header_adjust:DEBUG: Prefix mode output written",FL);
							break;
						case AM_HEADER_ADJUST_MODE_SUFFIX:
							*header_end = '\0';
							fprintf(fo,"%s %s%c%s", line, change, finishc, header_end+1 );
							header_written = 1;
							line_written = 1;
							if (AM_DNORMAL) LOGGER_log("%s:%d:AM_header_adjust:DEBUG: Suffix mode output written",FL);
							break;
						case AM_HEADER_ADJUST_MODE_REPLACE:
							*header_start = '\0';
							fprintf(fo,"%s %s%s", line, change, header_end );
							header_written = 1;
							line_written = 1;
							if (AM_DNORMAL) LOGGER_log("%s:%d:AM_header_adjust:DEBUG: Replace mode output written",FL);
							break;

						default:
							LOGGER_log("%s:%d:AM_header_adjust:ERROR: Unknown header adjustment mode '%d'",FL, change_mode );
					} // Switch (change_mode)
				} // If p < first_colon_position

			} // If p != NULL, ie, we found a matching string for the header			
		} // If we're still in the headers and we've still not found a matching header 

		// In all cases, we must write out the line we read in, even if it was the true-blank line
		// Otherwise we'll end up losing a the header seperation, and all of a sudden your emails
		// do not make any more sence to email programs.

		if (line_written == 0) fprintf(fo,"%s",line);
	} // While we still have more lines in the file.

	// Close our input files


	fclose(fo);
	fclose(fi);

	// We now can rename the temporary file to the new file name.
	// NOTE - previously we removed then renamed, however, on checking
	// with the manpages, 'man 2 rename' it seems it's not required, so
	// rather than waste CPU cycles, we'll just go by the book, and use
	// only rename.  There are a couple of situations where rename can/will
	// fail, obviously such as if the original file is marked read-only, or
	// we do not have write permissions to it.


	if (rename(tpn,filename) == -1)
	{
		result = 1;
		LOGGER_log("%s:%d:AM_header_adjust:ERROR: while attempting to rename '%s' to '%s' (%s) ", FL, tpn, filename, strerror(errno));
	}

	return result;
}






/*-----------------------------------------------------------------\
  Function Name	: AM_attachment_replace_header_filter
  Returns Type	: int
  ----Parameter List
  1. struct MIMEH_header_info *hinfo, 
  2.  FILE *outputfile, 
  3.  char *new_attachment_name, 
  4.  char *headers , 
  ------------------
  Exit Codes	: 
  Side Effects	: 
  --------------------------------------------------------------------
Comments:

--------------------------------------------------------------------
Changes:

\------------------------------------------------------------------*/
int AM_attachment_replace_header_filter( struct MIMEH_header_info *hinfo, char *new_attachment_name, char **headers )
{
	struct PLD_strreplace replace;

	// Because we're dealing with the primary headers, we have to be careful
	//		about how we search-replace the data within these headers, lest we
	//		break something we're not supposed to :-(

	if (AM_DNORMAL)LOGGER_log("%s:%d:AM_attachment_replace_header_filter:DEBUG: Start.",FL);
	replace.source = *headers;
	replace.searchfor = hinfo->filename;
	replace.replacewith = new_attachment_name;
	replace.replacenumber = 1; // only 1 replacement thanks!
	replace.insensitive = 1; // Emails come in all shapes and sizes - do not assume a case.
	replace.postexist = NULL;

	replace.preexist = "content-type:";
	*headers = PLD_strreplace_general( &replace );

	replace.source = *headers;
	replace.preexist = "content-disposition:";
	*headers = PLD_strreplace_general( &replace );

	// Because we can currently only encode our new attachment using BASE64
	//		we need to make sure that the content-transfer-encoding field is
	//		appropriately set.
	if (hinfo->content_transfer_encoding != _CTRANS_ENCODING_B64)
	{
		if (strlen(hinfo->content_transfer_encoding_string) < 1)
		{
			char CTE_string[256];

			snprintf(CTE_string, sizeof(CTE_string),"Content-Transfer-Encoding: base64\nContent-Type:");
			replace.preexist=NULL;
			replace.source=*headers;
			replace.searchfor="content-type:";
			replace.replacewith=CTE_string;
			*headers = PLD_strreplace_general( &replace );
		} else {
			replace.preexist = "content-transfer-encoding:";
			replace.source = *headers;
			replace.searchfor = hinfo->content_transfer_encoding_string;
			replace.replacewith = "base64";
			*headers = PLD_strreplace_general( &replace );
		}
	}

	if (AM_DNORMAL)LOGGER_log("%s:%d:AM_attachment_replace_header_filter:DEBUG: End.",FL);

	return 0;
}


/*-----------------------------------------------------------------\
  Function Name	: AM_attachment_replace_write_data
  Returns Type	: int
  ----Parameter List
  1. struct MIMEH_header_info *hinfo, 
  2.  char *new_attachment_name, 
  3.  FILE *outputfile , 
  4.  char *delimeter, line termination to use
  ------------------
  Exit Codes	: 
  Side Effects	: 
  --------------------------------------------------------------------
Comments:

--------------------------------------------------------------------
Changes:

\------------------------------------------------------------------*/
int AM_attachment_replace_write_data( char *new_attachment_name, FILE *outputfile, char *delimeter )
{
	int result = 0;
	FILE *newatt;

	newatt = fopen( new_attachment_name, "r" );
	if (newatt == NULL)
	{
		LOGGER_log("%s:%d:AM_attachment_replace_write_data:ERROR: Could not open '%s' for reading to insert into mailpack (%s)"\
				,FL, new_attachment_name, strerror(errno));
		return 1;
	}
	if(AM_DNORMAL)LOGGER_log("%s:%d:AM_attachment_replace_write_data:DEBUG: Writing out new attachment data",FL);
	AM_base64_encodef( newatt, outputfile );
	fprintf( outputfile, "%s%s", delimeter, delimeter );
	if(AM_DNORMAL)LOGGER_log("%s:%d:AM_attachment_replace_write_data:DEBUG: done.",FL);

	return result;
}

/*-----------------------------------------------------------------\
  Function Name	: AM_nullify_attachment_recurse
  Returns Type	: int
  ----Parameter List
  1. struct MIMEH_header_info *hinf, 
  2.  FFGET_FILE *f, 
  3.  FILE *outputfile, 
  4.  pregex_t *preg , 
  ------------------
  Exit Codes	: 
  Side Effects	: 
  --------------------------------------------------------------------
Comments:

--------------------------------------------------------------------
Changes:

\------------------------------------------------------------------*/
int AM_attachment_replace_recurse( struct MIMEH_header_info *hinfo, FFGET_FILE *f, FILE *outputfile, regex_t *preg, char *new_attachment_name, int iteration )
{
	int result = 0;
	int boundary_exists=0;
	size_t bc;

	if (AM_DNORMAL) LOGGER_log("%s:%d:AM_attachment_replace_recurse:DEBUG: Starting: iteration=%d",FL, iteration );
	while (1)
	{
		int regresult=0;
		int attachment_data_written=0;
		char *header_ptr=NULL;
		char *original_ptr=NULL;
		char buffer[1024];
		//		char CR[]="\n";
		//		char CRLF[]="\n\r";
		//		char *delimeter;


		MIMEH_set_doubleCR_save(0);
		result = MIMEH_headers_get( hinfo, f );
		MIMEH_set_doubleCR_save(1);
		if (result != 0)
		{ 
			break;
		}

		/* If we're not supposed to be altering Signed EMAILS, then don't
			start altering them now.  Exit with a 0 return
			*/
		if ((hinfo->content_type == _CTYPE_MULTIPART_SIGNED)&&(glb.alter_signed==0))
		{
			return 0;
		}


		if (AM_DNORMAL)LOGGER_log("%s:%d:AM_attachment_replace_recurse:DEBUG: Headers read, now processing", FL );

		original_ptr = MIMEH_get_headers_original_ptr();
		header_ptr = MIMEH_get_headers_ptr();

		if (AM_DNORMAL)LOGGER_log("%s:%d:AM_attachment_replace_recurse:DEBUG: Headers=\n%s\n", FL, original_ptr );

		if (original_ptr == NULL)
		{
			LOGGER_log("%s:%d:AM_attachment_replace_recurse:ERROR: Original headers came back NULL",FL);
			return 1;
		}

		if (header_ptr == NULL)
		{
			LOGGER_log("%s:%d:AM_attachment_replace_recurse:ERROR: Header ptr (for processing) came back NULL",FL);
			return 1;
		}

		// Because we'll be later on adding new lines into the mailpack, we need to know what
		//		the currently used line-delimeter is, this way we don't end up with a mailpack which
		//		has multiple personalities and thus potentially confusing the MUA.
		//if (strstr(original_ptr,CRLF)) delimeter=CRLF; else delimeter=CR;

		result = MIMEH_headers_process( hinfo, header_ptr );
		if (result != 0)
		{
			LOGGER_log("%s:%d:AM_attachment_replace_recurse:ERROR: While processing headers for mailpack", FL );
			break;
		}

		// Check to see if we have a new boundary
		//		Sometimes a new segment in the email reveals a nested MIME encoded
		//		data within.  This is typically indicated by Content-Type: rfc822
		//
		//	Should we have a new boundary, we add it to the top of the current boundary stack
		//		later, when this MIME segment finishes (and the previous boundary comes back
		//		into appearance, the old boundary will be popped off the stack automatically
		//
		if (((hinfo->content_type == _CTYPE_RFC822)\
					||(hinfo->content_type >= _CTYPE_MULTIPART_START && hinfo->content_type <= _CTYPE_MULTIPART_END))\
				&& (hinfo->boundary_located > 0))
		{
			if (AM_DNORMAL)LOGGER_log("%s:%d:AM_attachment_replace_recurse:DEBUG: pushing BS='%s'",FL, hinfo->boundary );
			BS_push( hinfo->boundary );
			boundary_exists = 1;
		}

		// Now, determine if this block/segment is the one which contains our file which we must 'nullify'
		regresult=1;
		if (strlen(hinfo->filename) > 0)
		{
			regresult = regexec( preg, hinfo->filename, 0, NULL, 0 );
			if(AM_DNORMAL)LOGGER_log("%s:%d:AM_attachment_replace_recurse:DEBUG: FileName Regex match = %d [filename = '%s']"\
					,FL,regresult,hinfo->filename);
		} 


		// If we're on our first pass, or we've not found the section/block with the attachment
		//	then write the headers out.
		//
		//	 A bit of an issue which comes up is, what to do if the entire email is the attachment
		//	ie, it's just a something which was 'right-click->email'd.
		//
		//	Until I come up with a better solution, I'll still write the headers, but I'll then 
		//		eliminate all the content [until the next boundary, assuming a boundary exists]
		//	
		if (regresult > 0)
		{
			int bl = strlen(original_ptr);

			bc = fwrite( original_ptr, sizeof(char), bl, outputfile );
			if (bc != bl) LOGGER_log("%s:%d:AM_attachment_replace_recurse:ERROR: Wrote %d bytes instead of %d", FL, bc, bl);
			if(AM_DNORMAL)LOGGER_log("%s:%d:AM_attachment_replace_recurse:DEBUG: Wrote original headers:\n%s",FL,original_ptr);
		} 
		else 
		{
			// If we did have a filename "HIT" in these headers
			//
			// We will need to suitably alter, or write new headers
			//	in order to reflect the new filenames that we'll be
			//	using for the replaced attachment

			char *new_attachment_filename;

			// Check for delimeters in the new attachment name, and make
			//	the new_attachment_filename just the last segment of that.

			new_attachment_filename=strrchr(new_attachment_name,'/');

			// If looking for the forward-slash failed, try looking for the backslash
			if (new_attachment_filename == NULL) new_attachment_filename=strrchr(new_attachment_name,'\\');

			// If both forward and backslash searches failed, then just let the new attachment filename
			//		be the same as the one passed to us via the parameters
			if (new_attachment_filename == NULL)
			{
				new_attachment_filename = new_attachment_name;
			} else {

				// If we did get a hit - then we increment by one character so that
				//	we don't have the directory seperator in our way.
				new_attachment_filename++;
			}

			// When it comes to creating the new headers, we have to check to see if we're
			//		in a suitable situation to either (a) entirely replace the headers with our own
			//		or (b) modify existing headers.
			//
			// Existing header modification is required when we're dealing with headers that
			//		make up the start of the whole MIME package, this is because there's a lot
			//		more information contained in them than just the file-attachment information
			//		Thus, for this situation, we'll use a header-modification function.
			//
			// If the headers are not the primary ones, we can just remove the existing ones
			//		and write in our own as generated by the content-type, disposition and encoding

			if (iteration > 1)
			{
				// If we're dealing with a non-primary header situation, just replace the old headers
				//		with our new fabricated ones

				if(AM_DNORMAL)LOGGER_log("%s:%d:AM_attachment_replace_recurse:DEBUG: Writing clean headers",FL);

				fprintf( outputfile, "Content-Type: %s;name=\"%s\"%sContent-Transfer-Encoding: base64%sContent-Disposition: attachment;filename=\"%s\"%s%s"\
						, hinfo->content_type_string\
						, new_attachment_filename, glb.ldelimeter, glb.ldelimeter\
						, new_attachment_filename, glb.ldelimeter, glb.ldelimeter\
						);
			} else {

				// If we're dealing with a primary-header situation, we have to carefully
				//		search-replace the old filenames with our own.  This has to be done
				//		within the strict confines between Content-Type:...;\n and/or Content-Disposition

				char *duplicate_headers;

				if(AM_DNORMAL)LOGGER_log("%s:%d:AM_attachment_replace_recurse:DEBUG: Primary header attachment replacement",FL,new_attachment_filename);
				duplicate_headers = strdup( original_ptr );
				if (duplicate_headers == NULL)
				{
					LOGGER_log("%s:%d:AM_attachment_replace_recurse:ERROR: Could not allocate memory to hold temporary copy of headers",FL);
					return 1;
				}

				//if(AM_DNORMAL)LOGGER_log("%s:%d:AM_attachment_replace_recurse:DEBUG: Seeking and replacing content type, disposition headers in main headers",FL);
				AM_attachment_replace_header_filter( hinfo, new_attachment_filename, &duplicate_headers );

				if(AM_DNORMAL)LOGGER_log("%s:%d:AM_attachment_replace_recurse:DEBUG: Writing recycled headers\n%s",FL, duplicate_headers);
				fprintf( outputfile, "%s", duplicate_headers);

				if (duplicate_headers != NULL) free( duplicate_headers );
			}
		}

		// Clean up the memory allocation
		result = MIMEH_headers_cleanup();
		if (result != 0)
		{
			LOGGER_log("%s:%d:AM_attachment_replace_recurse:ERROR: while attempting to clean up headers memory allocation", FL );
			break;
		}

		// Now, if we have a Multipart/Mixed attachment, then, alas, we need to recurse into
		//		it and see if it contains anything interesting for us to seek out.
		if ((regresult != 0)&&((hinfo->content_type == _CTYPE_RFC822)))
		{
			result=AM_attachment_replace_recurse( hinfo, f, outputfile, preg, new_attachment_name, 1 );
		}


		// Now, we shall go through and read the email until we happen across another boundary.
		//		or we reach the end of the file.
		//
		//
		do {
			int buflen;

			FFGET_fgets(buffer, sizeof(buffer), f);
			if (FFGET_feof(f) == 0)
			{
				buflen = strlen(buffer);

				// Once we have a boundary match, it's a end-of-line situation for this DO
				//		loop, as we will 'break' out of it once the attachment has been written
				//		and the trailing 'boundary' written as well.
				if ((BS_cmp(buffer, buflen) == 1))
				{
					/***
					  if ((AM_DNORMAL)&&(boundary_exists==1))\
					  LOGGER_log("%s:%d:AM_attachment_replace_recurse:DEBUG: Boundary hit on line %d\nBoundary Exists=%d\nBoundary line=%s"\
					  ,FL,f->linecount,boundary_exists, buffer);
					 ***/

					// If we have a match for the attachment replacement, then this is where
					//		all the work we do going to find this place comes to a head, as here
					//		we finally insert the encoded attachment into the mailpack we're
					//		creating anew from the existing one.
					if (regresult == 0)
					{
						AM_attachment_replace_write_data( new_attachment_name, outputfile, glb.ldelimeter );
						attachment_data_written=1;
					}

					bc = fwrite( buffer, sizeof(char), buflen, outputfile );
					if (bc != buflen) LOGGER_log("%s:%d:AM_attachment_replace_recurse:ERROR: Wrote %d bytes instead of %d", FL, bc, buflen);
					break;
				} // end of boundary-detect


				// If we didn't have a match on the filename with this particular MIME segment
				//		we simply just write out all the lines we are reading in.  This means
				//		that the data we're saving should be identical to that being read from the
				//		original file.
				if (regresult != 0)
				{
					bc = fwrite( buffer, sizeof(char), buflen, outputfile );
					if (bc != buflen) LOGGER_log("%s:%d:AM_attachment_replace_recurse:ERROR: Wrote %d bytes instead of %d", FL, bc, buflen);
				}

			} else {

				// If we hit the EOF, then we need to check to see if
				//		we were supposed to write any attachment data, and, if so
				//		did we actually get to write it out?
				//
				// If we haven't written it, then we must write it now.  
				//
				// This situation can occur when there's no trailing boundary
				//		line at the end of the last MIME segment of the mailpack.

				if ((regresult == 0)&&(attachment_data_written == 0))
				{
					AM_attachment_replace_write_data( new_attachment_name, outputfile, glb.ldelimeter );
				}

				break; // break if FEOF occurs
			}

		} while (1);


		if (FFGET_feof(f)) break;


		iteration++;

	} // Infinite while(1)

	if(AM_DNORMAL)LOGGER_log("%s:%d:AM_attachment_replace_recurse:DEBUG: End of function.",FL);
	return 0;
}


/*-----------------------------------------------------------------\
  Function Name	: AM_attachment_replace
  Returns Type	: int
  ----Parameter List
  1. char *mpackname,  mailpack which we're going to replace the file in
  2.  char *attachmentname, name of the attachment we're looking for in the mailpack [ to replace ]. This is a regular-expression syntax
  3.  char *new_attachment_name , full path of the file which we're going to use in place of *attachmentname
  ------------------
  Exit Codes	: 
  Side Effects	: 
  --------------------------------------------------------------------
Comments:
Replaces an attachment located in a mailpack with a new file. The
new file is encoded [currently only] in base64.  If the new attachment
name / path contains a path component ie, /usr/local/some-file, then
only the last segment of the full path will be used in the headers
[ ie, 'some-file' ]


--------------------------------------------------------------------
Changes:

\------------------------------------------------------------------*/
int AM_attachment_replace( char *mpackname, char *attachmentname, char *new_attachment_name )
{
	struct MIMEH_header_info hinfo;
	regex_t preg;
	int result = 0;
	char tmpfname[256];
	char oldfname[256];
	FILE *inputfile;
	FILE *outputfile;
	FFGET_FILE f;

	// Nullifying an attachment can sometimes be a little bit tricky, we have to
	//		dig down into the nesting of the MIME email and keep tabs on our
	//		boundaries (via a Boundary-stack).  This all requires about as much work
	//		as ripMIME just to get rid of one attachment.
	//
	// Additionally - we require some special functionality from MIME_headers, so
	//		so that we can hold-off saving the headers to the file until we've 
	//		checked them to see if we want them or not.

	BS_init();

	inputfile = fopen( mpackname, "r" );
	if (inputfile == NULL)
	{
		LOGGER_log("%s:%d:AM_replace_attachment: Unable to open mailpack '%s' for reading (%s)", FL, mpackname, strerror(errno));
		return 1;
	}

	snprintf( tmpfname, sizeof(tmpfname), "%s.tmp", mpackname );
	outputfile = fopen( tmpfname, "w" );
	if (outputfile == NULL)
	{
		if (inputfile != NULL) fclose(inputfile);
		LOGGER_log("%s:%d:AM_replace_attachment: Unable to open temporary file '%s' for writing (%s)", FL, tmpfname, strerror(errno));
		return 1;
	}

	FFGET_setstream( &f, inputfile );
	MIMEH_set_headers_nosave();
	MIMEH_set_headers_save_original(1);


	// Compile our Regular-expression for the filename.
	result = regcomp( &preg, attachmentname, REG_EXTENDED|REG_ICASE|REG_NOSUB );
	if (result != 0)
	{
		LOGGER_log("%s:%d:AM_replace_attachment: Unable to compile regular expression '%s'", FL, attachmentname );
		return 0;
	}

	result=AM_attachment_replace_recurse( &hinfo, &f, outputfile, &preg, new_attachment_name, 1 );

	MIMEH_set_headers_save_original(0);	

	snprintf(oldfname,sizeof(oldfname),"%s.old", mpackname);
	result = rename( mpackname, oldfname );
	if ( result != 0 ) 
	{
		LOGGER_log("%s:%d:AM_attachment_replace_recurse:ERROR: Unable to rename original mailpack '%s' to '%s' (%s)", FL, mpackname, oldfname, strerror(errno));
		return 1;
	}

	result = rename( tmpfname, mpackname );
	if (result != 0)
	{
		LOGGER_log("%s:%d:AM_attachment_replace_recurse:ERROR: Unable to rename temporary mailpack '%s' to '%s' (%s)", FL, tmpfname, mpackname, strerror(errno));
		return 1;
	}	

	result = unlink( oldfname );
	if ( result != 0)
	{
		LOGGER_log("%s:%d:AM_attachment_replace_recurse:ERROR: Unable to unlink/remove '%s' (%s)", FL, oldfname, strerror(errno));
		return 1;
	}

	// Clear the boundary stack
	BS_clear();

	return result;
}

//-----------------------------------END----


