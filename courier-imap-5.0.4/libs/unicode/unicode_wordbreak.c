/*
** Copyright 2011-2013 Double Precision, Inc.
** See COPYING for distribution information.
**
*/

#include	"unicode_config.h"
#include	"courier-unicode.h"

#include <unistd.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "wordbreaktab_internal.h"
#include "wordbreaktab.h"

struct unicode_wb_info {
	int (*cb_func)(int, void *);
	void *cb_arg;

	uint8_t prevclass;
	uint8_t wb7_first_char;
	size_t wb4_cnt;

	size_t wb4_extra_cnt;

	int (*next_handler)(unicode_wb_info_t, uint8_t);
	int (*end_handler)(unicode_wb_info_t);
};

static int sot(unicode_wb_info_t i, uint8_t cl);
static int wb4(unicode_wb_info_t i);
static int wb1and2_done(unicode_wb_info_t i, uint8_t cl);

static int seen_wb67_handler(unicode_wb_info_t i, uint8_t cl);
static int seen_wb67_end_handler(unicode_wb_info_t i);
static int wb67_done(unicode_wb_info_t i, uint8_t prevclass, uint8_t cl);

static int seen_wb7bc_handler(unicode_wb_info_t i, uint8_t cl);
static int seen_wb7bc_end_handler(unicode_wb_info_t i);
static int wb7bc_done(unicode_wb_info_t i, uint8_t prevclass, uint8_t cl);

static int seen_wb1112_handler(unicode_wb_info_t i, uint8_t cl);
static int seen_wb1112_end_handler(unicode_wb_info_t i);
static int wb1112_done(unicode_wb_info_t i, uint8_t prevclass, uint8_t cl);

unicode_wb_info_t unicode_wb_init(int (*cb_func)(int, void *),
				  void *cb_arg)
{
	unicode_wb_info_t i=calloc(1, sizeof(struct unicode_wb_info));

	if (!i)
		return NULL;

	i->next_handler=sot;
	i->cb_func=cb_func;
	i->cb_arg=cb_arg;
	return i;
}

int unicode_wb_end(unicode_wb_info_t i)
{
	int rc;

	if (i->end_handler)
		rc=(*i->end_handler)(i);
	else
		rc=wb4(i);

	free(i);
	return rc;
}

int unicode_wb_next_cnt(unicode_wb_info_t i,
			const char32_t *chars,
			size_t cnt)
{
	int rc;

	while (cnt)
	{
		rc=unicode_wb_next(i, *chars++);
		--cnt;
		if (rc)
			return rc;
	}
	return 0;
}

int unicode_wb_next(unicode_wb_info_t i, char32_t ch)
{
	return (*i->next_handler)
		(i, unicode_tab_lookup(ch,
				       unicode_indextab,
				       sizeof(unicode_indextab)
				       / sizeof(unicode_indextab[0]),
				       unicode_rangetab,
				       unicode_classtab,
				       UNICODE_WB_OTHER));
}

static int wb4(unicode_wb_info_t i)
{
	int rc=0;

	while (i->wb4_cnt > 0)
	{
		--i->wb4_cnt;

		if (rc == 0)
			rc=(*i->cb_func)(0, i->cb_arg);
	}
	return rc;
}

static int result(unicode_wb_info_t i, int flag)
{
	int rc=wb4(i);

	if (rc == 0)
		rc=(*i->cb_func)(flag, i->cb_arg);

	return rc;
}

#define SET_HANDLER(next,end) (i->next_handler=next, i->end_handler=end)

static int sot(unicode_wb_info_t i, uint8_t cl)
{
	i->prevclass=cl;
	SET_HANDLER(wb1and2_done, NULL);

	return result(i, 1);	/* WB1 */
}

static int wb1and2_done(unicode_wb_info_t i, uint8_t cl)
{
	uint8_t prevclass=i->prevclass;

	i->prevclass=cl;

	if (prevclass == UNICODE_WB_CR && cl == UNICODE_WB_LF)
		return result(i, 0); /* WB3 */

	switch (prevclass) {
	case UNICODE_WB_CR:
	case UNICODE_WB_LF:
	case UNICODE_WB_Newline:
		return result(i, 1); /* WB3a */
	}

	switch (cl) {
	case UNICODE_WB_CR:
	case UNICODE_WB_LF:
	case UNICODE_WB_Newline:
		return result(i, 1); /* WB3b */
	}

	if (cl == UNICODE_WB_Extend || cl == UNICODE_WB_Format)
	{
		i->prevclass=prevclass;
		++i->wb4_cnt;
		return 0; /* WB4 */
	}

	if ((prevclass == UNICODE_WB_ALetter ||
	     prevclass == UNICODE_WB_Hebrew_Letter) &&
	    (cl == UNICODE_WB_ALetter || cl == UNICODE_WB_Hebrew_Letter))
	{
		return result(i, 0); /* WB5 */
	}

	if ((prevclass == UNICODE_WB_ALetter ||
	     prevclass == UNICODE_WB_Hebrew_Letter)
	    &&
	    (cl == UNICODE_WB_MidLetter || cl == UNICODE_WB_MidNumLet ||
	     cl == UNICODE_WB_Single_Quote))
	{
		i->wb4_extra_cnt=0;
		i->wb7_first_char=prevclass;
		SET_HANDLER(seen_wb67_handler, seen_wb67_end_handler);
		return 0;
	}

	return wb67_done(i, prevclass, cl);
}

/*
** (ALetter | Hebrew_Letter) (MidLetter | MidNumLet | Single_quote)  ?
**
**                           prevclass                               cl
**
** Seen (ALetter | Hebrew_Letter)(MidLetter | MidNumLet), with the second
** character's status not returned yet.
*/

static int seen_wb67_handler(unicode_wb_info_t i, uint8_t cl)
{
	int rc;
	uint8_t prevclass;
	size_t extra_cnt;

	if (cl == UNICODE_WB_Extend || cl == UNICODE_WB_Format)
	{
		++i->wb4_extra_cnt;
		return 0;
	}

	extra_cnt=i->wb4_extra_cnt;

	/*
	** Reset the handler to the default, then check WB6
	*/

	SET_HANDLER(wb1and2_done, NULL);

	if (cl == UNICODE_WB_ALetter || cl == UNICODE_WB_Hebrew_Letter)
	{
		rc=result(i, 0); /* WB6 */
		i->wb4_cnt=extra_cnt;

		if (rc == 0)
			rc=result(i, 0); /* WB7 */

		i->prevclass=cl;
			
		return rc;
	}

	prevclass=i->prevclass; /* This was the second character */

	/*
	** Process the second character, starting with WB7
	*/

	rc=wb67_done(i, i->wb7_first_char, prevclass);

	i->prevclass=prevclass;
	i->wb4_cnt=extra_cnt;

	if (rc == 0)
		rc=(*i->next_handler)(i, cl);
	/* Process the current char now */

	return rc;
}

/*
** Seen (ALetter | Hebrew_Letter)(MidLetter | MidNumLet), with the second
** character's status not returned yet, and now sot.
*/

static int seen_wb67_end_handler(unicode_wb_info_t i)
{
	int rc;
	size_t extra_cnt=i->wb4_extra_cnt;

	/*
	** Process the second character, starting with WB7.
	*/

	rc=wb67_done(i, i->wb7_first_char, i->prevclass);
	i->wb4_cnt=extra_cnt;
	if (rc == 0)
		rc=wb4(i);
	return rc;
}

static int wb67_done(unicode_wb_info_t i, uint8_t prevclass, uint8_t cl)
{
	if (prevclass == UNICODE_WB_Hebrew_Letter && cl == UNICODE_WB_Single_Quote)
		return result(i, 0); /* WB7a */

	if (prevclass == UNICODE_WB_Hebrew_Letter && cl == UNICODE_WB_Double_Quote)
	{
		i->wb4_extra_cnt=0;
		SET_HANDLER(seen_wb7bc_handler, seen_wb7bc_end_handler);
		return 0;
	}

	return wb7bc_done(i, prevclass, cl);
}

/*
** Hebrew_Letter Double_Quote       ?
**
**               prevclass          cl
**
** Seen Hebrew_Letter Double_Quote, with the second character's status
** not returned yet.
*/

static int seen_wb7bc_handler(unicode_wb_info_t i, uint8_t cl)
{
	int rc;
	uint8_t prevclass;
	size_t extra_cnt;

	if (cl == UNICODE_WB_Extend || cl == UNICODE_WB_Format)
	{
		++i->wb4_extra_cnt;
		return 0;
	}

	extra_cnt=i->wb4_extra_cnt;

	/*
	** Reset the handler to the default, then check WB7a and WB7b
	*/

	SET_HANDLER(wb1and2_done, NULL);

	if (cl == UNICODE_WB_Hebrew_Letter)
	{
		rc=result(i, 0); /* WB7b */
		i->wb4_cnt=extra_cnt;

		if (rc == 0)
			rc=result(i, 0); /* WB7bc */

		i->prevclass=cl;
			
		return rc;
	}

	prevclass=i->prevclass; /* This was the second character */

	/*
	** Process the second character, starting with WB8
	*/

	rc=wb7bc_done(i, UNICODE_WB_Hebrew_Letter, prevclass);

	i->prevclass=prevclass;
	i->wb4_cnt=extra_cnt;

	if (rc == 0)
		rc=(*i->next_handler)(i, cl);
	/* Process the current char now */

	return rc;
}

/*
** Seen Hebrew_Letter Double_Quote, with the second
** character's status not returned yet, and now sot.
*/

static int seen_wb7bc_end_handler(unicode_wb_info_t i)
{
	int rc;
	size_t extra_cnt=i->wb4_extra_cnt;

	/*
	** Process the second character, starting with WB8.
	*/

	rc=wb7bc_done(i, UNICODE_WB_Hebrew_Letter, i->prevclass);
	i->wb4_cnt=extra_cnt;
	if (rc == 0)
		rc=wb4(i);
	return rc;
}

static int wb7bc_done(unicode_wb_info_t i, uint8_t prevclass, uint8_t cl)
{
	if (prevclass == UNICODE_WB_Numeric && cl == UNICODE_WB_Numeric)
		return result(i, 0); /* WB8 */

	if ((prevclass == UNICODE_WB_ALetter ||
	     prevclass == UNICODE_WB_Hebrew_Letter) && cl == UNICODE_WB_Numeric)
		return result(i, 0); /* WB9 */

	if (prevclass == UNICODE_WB_Numeric &&
	    (cl == UNICODE_WB_ALetter || cl == UNICODE_WB_Hebrew_Letter))
		return result(i, 0); /* WB10 */


	if (prevclass == UNICODE_WB_Numeric &&
	    (cl == UNICODE_WB_MidNum || cl == UNICODE_WB_MidNumLet ||
	     cl == UNICODE_WB_Single_Quote))
	{
		i->wb4_extra_cnt=0;
		SET_HANDLER(seen_wb1112_handler, seen_wb1112_end_handler);
		return 0;
	}

	return wb1112_done(i, prevclass, cl);
}

/*
**              Numeric     (MidNum | MidNumLet )     ?
**
**                               prevclass            cl
**
** Seen Numeric (MidNum | MidNumLet), with the second character's status
** not returned yet.
*/

static int seen_wb1112_handler(unicode_wb_info_t i, uint8_t cl)
{
	int rc;
	uint8_t prevclass;
	size_t extra_cnt;

	if (cl == UNICODE_WB_Extend || cl == UNICODE_WB_Format)
	{
		++i->wb4_extra_cnt;
		return 0;
	}

	extra_cnt=i->wb4_extra_cnt;

	/*
	** Reset the handler to the default, then check WB6
	*/

	SET_HANDLER(wb1and2_done, NULL);

	if (cl == UNICODE_WB_Numeric)
	{
		rc=result(i, 0); /* WB11 */
		i->wb4_cnt=extra_cnt;

		if (rc == 0)
			rc=result(i, 0); /* WB12 */

		i->prevclass=cl;
			
		return rc;
	}

	prevclass=i->prevclass; /* This was the second character */

	/*
	** Process the second character, starting with WB7
	*/

	rc=wb1112_done(i, UNICODE_WB_Numeric, prevclass);

	i->prevclass=prevclass;
	i->wb4_cnt=extra_cnt;

	if (rc == 0)
		rc=(*i->next_handler)(i, cl);
	/* Process the current char now */

	return rc;
}

/*
** Seen Numeric (MidNum | MidNumLet), with the second character's status
** not returned yet, and now sot.
*/

static int seen_wb1112_end_handler(unicode_wb_info_t i)
{
	int rc;
	size_t extra_cnt=i->wb4_extra_cnt;

	/*
	** Process the second character, starting with WB11.
	*/

	rc=wb1112_done(i, UNICODE_WB_Numeric, i->prevclass);
	i->wb4_cnt=extra_cnt;
	if (rc == 0)
		rc=wb4(i);
	return rc;
}

static int wb1112_done(unicode_wb_info_t i, uint8_t prevclass, uint8_t cl)
{
	if (prevclass == UNICODE_WB_Katakana &&
	    cl == UNICODE_WB_Katakana)
		return result(i, 0); /* WB13 */

	switch (prevclass) {
	case UNICODE_WB_ALetter:
	case UNICODE_WB_Hebrew_Letter:
	case UNICODE_WB_Numeric:
	case UNICODE_WB_Katakana:
	case UNICODE_WB_ExtendNumLet:
		if (cl == UNICODE_WB_ExtendNumLet)
			return result(i, 0); /* WB13a */
	}

	if (prevclass == UNICODE_WB_ExtendNumLet)
		switch (cl) {
		case UNICODE_WB_ALetter:
		case UNICODE_WB_Hebrew_Letter:
		case UNICODE_WB_Numeric:
		case UNICODE_WB_Katakana:
			return result(i, 0); /* WB13b */
		}

	if (prevclass == UNICODE_WB_Regional_Indicator &&
	    cl == UNICODE_WB_Regional_Indicator)
		return result(i, 0);
	return result(i, 1); /* WB14 */
}

/* --------------------------------------------------------------------- */

struct unicode_wbscan_info {
	unicode_wb_info_t wb_handle;

	int found;
	size_t cnt;
};

static int unicode_wbscan_callback(int, void *);

unicode_wbscan_info_t unicode_wbscan_init()
{
	unicode_wbscan_info_t i=calloc(1, sizeof(struct unicode_wbscan_info));

	if (!i)
		return NULL;

	if ((i->wb_handle=unicode_wb_init(unicode_wbscan_callback, i)) == NULL)
	{
		free(i);
		return NULL;
	}

	return i;
}

int unicode_wbscan_next(unicode_wbscan_info_t i, char32_t ch)
{
	if (!i->found)
		unicode_wb_next(i->wb_handle, ch);

	return i->found;
}

size_t unicode_wbscan_end(unicode_wbscan_info_t i)
{
	size_t n;

	unicode_wb_end(i->wb_handle);

	n=i->cnt;
	free(i);
	return n;
}

static int unicode_wbscan_callback(int flag, void *arg)
{
	unicode_wbscan_info_t i=(unicode_wbscan_info_t)arg;

	if (flag && i->cnt > 0)
		i->found=1;

	if (!i->found)
		++i->cnt;
	return 0;
}

