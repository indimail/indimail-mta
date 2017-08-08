#ifndef maildir_search_h
#define maildir_search_h

/*
** Copyright 2002-2011 Double Precision, Inc.
** See COPYING for distribution information.
*/


/*
** A deterministic automaton-based search mechanism.  Search for a particular
** string in a middle of a larger body of text.
**
** Allocate a struct maildir_searchengine by yourself, and call
** maildir_search_init() to initialize.
**
** Call maildir_search_destroy() to release any allocated memory.
**
** Call maildir_search_start_str() to prep the structure for a particular search
** string. Alternatively, call maildir_search_start_unicode() to specify the
** search string as unicode characters. Alternatively,
** call maildir_search_start_str_chset() to specify the search string in
** a specific character. The search string get converted to unicode, then
** converted to lowercase characters, removing leading/trailing whitespace,
** and replacing multiple occurences of whitespace in the search string with
** a single space character.
**
** Call maildir_search_reset() to start the search, then call
** maildir_search_step() for each character in the text. Use
** maildir_search_step_unicode() if the search string was specified via
** maildir_search_start_unicode(). Use maildir_search_step_unicode_lc() if
** the search string was specified using via maildir_search_start_str_chset().
**
** Call maildir_search_found() to check if the search string is found.
*/

#include "config.h"

#include "unicode/courier-unicode.h"

#include <string.h>
#include <stdlib.h>

#ifdef  __cplusplus
extern "C" {
#endif

struct maildir_searchengine {
	char32_t *string;
	size_t string_l;
	const char32_t *ptr;
	unsigned *r;    /* Retry backoff indexes */
	unsigned i;
	int spc;
	} ;

#define maildir_search_init(sei) (memset((sei), 0, sizeof(struct maildir_searchengine)))

#define maildir_search_destroy(sei) do { if ((sei)->string) free((sei)->string); if ( (sei)->r) free( (sei)->r); } while (0)

int maildir_search_start_str(struct maildir_searchengine *engine,
			     const char *string);
int maildir_search_start_str_chset(struct maildir_searchengine *engine,
				   const char *string,
				   const char *chset);
int maildir_search_start_unicode(struct maildir_searchengine *engine,
				 const char32_t *string);


#define	maildir_search_reset(si)	((si)->i=0, (si)->ptr=(si)->string)

#define	maildir_search_found(si)	((si)->ptr && \
					(si)->ptr[(si)->i] == '\0')
#define maildir_search_len(si)		((si)->string_l)

#define maildir_search_step_unicode(sie,ch) do \
        {\
                if ( (sie)->ptr && (sie)->ptr[(sie)->i]) \
                {\
                        for (;;) \
                        {\
                                if ( (char32_t)(sie)->ptr[(sie)->i] == (char32_t)(ch) )\
                                        { (sie)->i++; break; }\
                                if ( (sie)->i == 0) break;\
                                (sie)->i=(sie)->r[(sie)->i];\
                        }\
                }\
        } while (0)

#define maildir_search_step_unicode_lc(sie,ch) do	\
	{						\
		char32_t c=(ch);			\
		int spc=0;				\
							\
									\
		if (c == ' ' || c == '\t' || c == '\r' || c == '\n')	\
		{							\
			c=' ';						\
			spc=1;						\
		}							\
									\
		if (spc && (sie)->spc)					\
			;						\
		else							\
		{							\
			c=unicode_lc(c);				\
			maildir_search_step_unicode((sie),c);		\
		}							\
									\
		(sie)->spc=spc;						\
	} while(0)

#define maildir_search_step(sie,ch) \
	maildir_search_step_unicode((sie), ((unsigned char)(ch)))

#define maildir_search_atstart(sie) ((sie)->i == 0)

#ifdef  __cplusplus
}

/* A C++ wrapper for the above */

#if HAVE_VECTOR
#include <vector>
#else
#if HAVE_VECTOR_H
#include <vector.h>
#endif
#endif

#include <string>

namespace mail {

class Search {

	struct maildir_searchengine sei;

	std::string String;

	std::vector<unsigned> rbuf;
 public:
	Search();
	virtual ~Search();

	size_t getSearchLen()
	{
		return maildir_search_len(&sei);
	}

	bool setString(std::string s, std::string chset)
	{
		String=s;
		return maildir_search_start_str_chset(&sei, s.c_str(),
						      chset.c_str()) == 0;
	}

	void reset()
	{
		maildir_search_reset(&sei);
	}

	void operator<<(char c) { maildir_search_step(&sei, c); }

	void operator<<(char32_t ch)
	{
		maildir_search_step_unicode_lc(&sei, ch);
	}

	bool atstart() { return maildir_search_atstart(&sei); }
	operator bool() { return maildir_search_found(&sei); }

	bool operator !() { return ! operator bool(); }

 private:
	Search(const Search &); /* UNDEFINED */

	Search &operator=(const Search &); /* UNDEFINED */

};

}

#endif

#endif
