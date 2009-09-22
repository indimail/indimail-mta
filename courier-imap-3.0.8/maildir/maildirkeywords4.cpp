/*
** Copyright 2003 Double Precision, Inc.
** See COPYING for distribution information.
*/

#if	HAVE_CONFIG_H
#include	"config.h"
#endif
#include	"maildirkeywords.h"
#include	<iostream>
#include	<stdlib.h>
#include	<errno.h>
#include	<stdio.h>

#include	<vector>

using namespace std;

mail::keywords::Hashtable::Hashtable()
{
	libmail_kwhInit(&kwh);
}

mail::keywords::Hashtable::~Hashtable()
{
	if (libmail_kwhCheck(&kwh))
	{
		cerr << "INTERNAL ERROR: "
			"mail::keywords::Hashtable::~Hashtable: "
			"dangling references remain." << endl;

		abort();
	}
}

mail::keywords::MessageBase::MessageBase()
	: km(NULL), refCnt(0)
{
}

mail::keywords::MessageBase::~MessageBase()
{
	if (km)
		libmail_kwmDestroy(km);
}

mail::keywords::Message::Message()
	: b(new MessageBase)
{
	if (!b)
		throw strerror(errno);

	++b->refCnt;
}

mail::keywords::Message::~Message()
{
	if (b && --b->refCnt == 0)
		delete b;
}

mail::keywords::Message::Message(const mail::keywords::Message &m)
	: b(m.b)
{
	++b->refCnt;
}

mail::keywords::Message &
mail::keywords::Message::operator=(const mail::keywords::Message &m)
{
	++m.b->refCnt;

	if (--b->refCnt == 0)
		delete b;

	b=m.b;
	return *this;
}

void mail::keywords::Message::getFlags(std::set<std::string> &kwSet) const
{
	kwSet.clear();
	if (!b->km)
		return;

	struct libmail_kwMessageEntry *e;

	for (e= b->km->firstEntry; e; e=e->next)
		kwSet.insert(keywordName(e->libmail_keywordEntryPtr));
}

bool mail::keywords::Message::copyOnWrite()
{
	if (b->refCnt > 1) // Sharing a copy? We want our own.
	{
		MessageBase *nb=new MessageBase;

		if (!nb)
			return false;

		--b->refCnt;
		b=nb;
		++b->refCnt;
	}
	return true;
}

bool mail::keywords::Message::setFlags(mail::keywords::Hashtable &h,
				       const std::set<std::string> &kwSet)
{
	if (!copyOnWrite())
		return false;

	if (b->km)
		libmail_kwmDestroy(b->km);
	b->km=NULL;

	if (kwSet.empty())
	{
		return true;
	}

	if ((b->km=libmail_kwmCreate()) == NULL)
		return false;

	set<string>::const_iterator kb=kwSet.begin(), ke=kwSet.end();

	while (kb != ke)
	{
		if (libmail_kwmSetName(&h.kwh, b->km, kb->c_str()))
			return false;

		++kb;
	}
	return true;
}

bool mail::keywords::Message::addFlag(Hashtable &h, string flagName)
{
	if (!copyOnWrite())
		return false;

	if (!b->km)
		if ((b->km=libmail_kwmCreate()) == NULL)
			return false;

	if (libmail_kwmSetName(&h.kwh, b->km, flagName.c_str()))
		return false;
	return true;
}

bool mail::keywords::Message::remFlag(std::string s)
{
	if (!copyOnWrite())
		return false;
	if (b->km)
		libmail_kwmClearName(b->km, s.c_str());
	return true;
}

int maildir_kwSave(const char *maildir,
		   const char *filename,
		   set<string> keywords,

		   char **tmpname,
		   char **newname,

		   int tryAtomic)
{
	vector<const char *> keywordArray;
	set<string>::iterator b, e;

	keywordArray.reserve(keywords.size()+1);

	for (b=keywords.begin(), e=keywords.end(); b != e; ++b)
		keywordArray.push_back(b->c_str());

	keywordArray.push_back((const char *)NULL);
	return maildir_kwSaveArray(maildir, filename, &keywordArray[0],
				   tmpname, newname, tryAtomic);
}
