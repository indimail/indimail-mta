#ifndef _CONTENT_H_
#define _CONTENT_H_

#define TYP_CON 0
#define TYP_ENC 1
#define TYP_DIS 2

/*
 * Content-type
 */
#define CON_NONE     0
#define CON_TEXT     1			/*- text      */
#define CON_MULTI    2			/*- multipart */
/*
 * For now, we'll treat this as CON_TEXT
 */
#define CON_MESSAGE  CON_TEXT	/*- message */
#define CON_UNKNOWN  CON_NONE

/*
 * Content-Transfer-Encoding types
 */
#define ENC_NONE     0
#define ENC_TEXT     1			/*- text             */
#define ENC_BASE64   2			/*- base64           */
#define ENC_7BIT     4			/*- 7bit             */
#define ENC_8BIT     8			/*- 8bit             */
#define ENC_QP      16			/*- quoted/printable */
#define ENC_RAW     32			/*- raw              */
#define ENC_UNSPEC  64			/*- unspecified      */
#define ENC_UNKNOWN ENC_NONE

/*
 * Content-Disposition
 */
#define DIS_NONE     0
#define DIS_INLINE   1			/*- inline      */
#define DIS_ATTACH   2			/*- attachment  */
#define DIS_FORMDATA 4			/*- formdata    */
#define DIS_UNSPEC   8			/*- unspecified */
#define DIS_UNKNOWN  DIS_NONE

struct _content_t
{
	char           *data;
	int             type;
};

int             content_parse(char *, char);

#endif
