#ifndef maildir_search_h
#define maildir_search_h

/*
** Copyright 2002 Double Precision, Inc.
** See COPYING for distribution information.
*/

static const char maildir_search_h_rcsid[]="$Id: maildirsearch.h,v 1.4 2003/01/05 04:22:50 mrsam Exp $";

/*
** A deterministic automaton-based search mechanism.  Search for a particular
** string in a middle of a larger body of text.
**
** Allocate a struct maildir_searchengine by yourself, and call
** maildir_search_init() to initialize.
**
** Call maildir_search_destroy() to release any allocated memory.
**
** Call maildir_search_start() to prep the structure for a particular search
** string.
**
** Call maildir_search_reset() to start the search, then call
** maildir_search_step() for each character in the text.
**
** Call maildir_search_found() to check if the search string is found.
*/

#include "config.h"

#include <string.h>
#include <stdlib.h>

#ifdef  __cplusplus
extern "C" {
#endif

struct maildir_searchengine {
	const char *string;
	const char *ptr;
	unsigned *r;    /* Retry backoff indexes */
	unsigned i;
	} ;

#define maildir_search_init(sei) (memset((sei), 0, sizeof(struct maildir_searchengine)))

#define maildir_search_destroy(sei) do { if ( (sei)->r) free( (sei)->r); } while (0)

int maildir_search_start(struct maildir_searchengine *engine,
			 const char *string);
	/* NOTE: do NOT deallocate string until searching is complete */


#define	maildir_search_reset(si)	((si)->i=0, (si)->ptr=(si)->string)

#define	maildir_search_found(si)	((si)->ptr && \
					(si)->ptr[(si)->i] == '\0')

#define maildir_search_step(sie,ch) do \
        {\
                if ( (sie)->ptr && (sie)->ptr[(sie)->i]) \
                {\
                        for (;;) \
                        {\
                                if ( (unsigned char)(sie)->ptr[(sie)->i] == (unsigned char)(ch) )\
                                        { (sie)->i++; break; }\
                                if ( (sie)->i == 0) break;\
                                (sie)->i=(sie)->r[(sie)->i];\
                        }\
                }\
        } while (0)


#ifdef  __cplusplus
}

// A C++ wrapper for the above

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

	int setString(std::string s)
	{
		String=s;
		return maildir_search_start(&sei, s.c_str());
	}

	void reset()
	{
		maildir_search_reset(&sei);
	}

	void operator<<(char c) { maildir_search_step(&sei, c); }

	operator bool() { return maildir_search_found(&sei); }

	bool operator !() { return ! operator bool(); }

 private:
	Search(const Search &); // UNDEFINED

	Search &operator=(const Search &); // UNDEFINED

};

};

#endif

#endif
 
