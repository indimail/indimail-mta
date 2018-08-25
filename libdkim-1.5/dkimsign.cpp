/*
 * $Log: dkimsign.cpp,v $
 * Revision 1.13  2018-08-25 18:01:59+05:30  Cprogrammer
 * fixed dkim signing for From address containing company name
 *
 * Revision 1.12  2018-05-23 13:07:58+05:30  Cprogrammer
 * fixed compiler warnings
 *
 * Revision 1.11  2017-09-05 10:59:03+05:30  Cprogrammer
 * removed compiler warnings
 *
 * Revision 1.10  2017-08-09 22:02:13+05:30  Cprogrammer
 * replaced EVP_MD_CTX_free() with EVP_MD_CTX_reset()
 *
 * Revision 1.9  2017-08-08 23:50:19+05:30  Cprogrammer
 * openssl 1.1.0 port
 *
 * Revision 1.8  2013-07-16 20:18:03+05:30  Cprogrammer
 * replace '%' with domain name in selector
 *
 * Revision 1.7  2013-06-11 00:02:39+05:30  Cprogrammer
 * removed header iostream
 *
 * Revision 1.6  2013-06-09 16:41:28+05:30  Cprogrammer
 * parse address properly from From and Sender header
 *
 * Revision 1.5  2009-04-16 10:32:38+05:30  Cprogrammer
 * added DKIMDOMAIN env variable
 *
 * Revision 1.4  2009-04-15 21:32:12+05:30  Cprogrammer
 * added DKIM-Signature, Received to list of excluded headers
 *
 * Revision 1.3  2009-03-26 15:11:46+05:30  Cprogrammer
 * added GetDomain
 *
 * Revision 1.2  2009-03-21 11:57:19+05:30  Cprogrammer
 * fixed indentation
 *
 * Revision 1.1  2009-03-21 08:43:11+05:30  Cprogrammer
 * Initial revision
 *
 *
 *  Copyright 2005 Alt-N Technologies, Ltd. 
 *
 *  Licensed under the Apache License, Version 2.0 (the "License"); 
 *  you may not use this file except in compliance with the License. 
 *  You may obtain a copy of the License at 
 *
 *      http://www.apache.org/licenses/LICENSE-2.0 
 *
 *  This code incorporates intellectual property owned by Yahoo! and licensed 
 *  pursuant to the Yahoo! DomainKeys Patent License Agreement.
 *
 *  Unless required by applicable law or agreed to in writing, software 
 *  distributed under the License is distributed on an "AS IS" BASIS, 
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. 
 *  See the License for the specific language governing permissions and 
 *  limitations under the License.
 *
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#define _strnicmp strncasecmp
#define _stricmp strcasecmp
#define LOWORD(l) ((unsigned)(l) & 0xffff)
#define HIWORD(l) ((unsigned)(l) >> 16)

#include <string.h>
#include <map>
#include "dkim.h"
#include "dkimsign.h"

CDKIMSign::CDKIMSign()
{
	m_EmptyLineCount = 0;
	m_pfnHdrCallback = NULL;
#if OPENSSL_VERSION_NUMBER >= 0x10100000L
	if (!m_allman_sha1ctx)
		m_allman_sha1ctx = EVP_MD_CTX_new();
	EVP_SignInit(m_allman_sha1ctx, EVP_sha1());
	if (!m_Hdr_ietf_sha1ctx)
		m_Hdr_ietf_sha1ctx = EVP_MD_CTX_new();
	EVP_SignInit(m_Hdr_ietf_sha1ctx, EVP_sha1());
	if (!m_Bdy_ietf_sha1ctx)
		m_Bdy_ietf_sha1ctx = EVP_MD_CTX_new();
	EVP_DigestInit(m_Bdy_ietf_sha1ctx, EVP_sha1());
#ifdef HAVE_EVP_SHA256
	if (!m_Hdr_ietf_sha256ctx)
		m_Hdr_ietf_sha256ctx = EVP_MD_CTX_new();
	EVP_SignInit(m_Hdr_ietf_sha256ctx, EVP_sha256());
	if (!m_Bdy_ietf_sha256ctx)
		m_Bdy_ietf_sha256ctx = EVP_MD_CTX_new();
	EVP_DigestInit(m_Bdy_ietf_sha256ctx, EVP_sha256());
#endif
#else
	EVP_SignInit(&m_allman_sha1ctx, EVP_sha1());
	EVP_SignInit(&m_Hdr_ietf_sha1ctx, EVP_sha1());
	EVP_DigestInit(&m_Bdy_ietf_sha1ctx, EVP_sha1());
#ifdef HAVE_EVP_SHA256
	EVP_SignInit(&m_Hdr_ietf_sha256ctx, EVP_sha256());
	EVP_DigestInit(&m_Bdy_ietf_sha256ctx, EVP_sha256());
#endif
#endif
}

CDKIMSign::~CDKIMSign()
{
#if OPENSSL_VERSION_NUMBER >= 0x10100000L
	EVP_MD_CTX_reset(m_allman_sha1ctx);
	EVP_MD_CTX_reset(m_Hdr_ietf_sha1ctx);
	EVP_MD_CTX_reset(m_Bdy_ietf_sha1ctx);
#ifdef HAVE_EVP_SHA256
	EVP_MD_CTX_reset(m_Hdr_ietf_sha256ctx);
	EVP_MD_CTX_reset(m_Bdy_ietf_sha256ctx);
#endif
#else
	EVP_MD_CTX_cleanup(&m_allman_sha1ctx);
	EVP_MD_CTX_cleanup(&m_Hdr_ietf_sha1ctx);
	EVP_MD_CTX_cleanup(&m_Bdy_ietf_sha1ctx);
#ifdef HAVE_EVP_SHA256
	EVP_MD_CTX_cleanup(&m_Hdr_ietf_sha256ctx);
	EVP_MD_CTX_cleanup(&m_Bdy_ietf_sha256ctx);
#endif
#endif
}


////////////////////////////////////////////////////////////////////////////////
// 
// Init - save the options
//
////////////////////////////////////////////////////////////////////////////////
int
CDKIMSign::Init(DKIMSignOptions * pOptions)
{
	int             nRet = CDKIMBase::Init();
	m_Canon = pOptions->nCanon;

// as of draft 01, these are the only allowed signing types:
	if ((m_Canon != DKIM_SIGN_SIMPLE_RELAXED) && (m_Canon != DKIM_SIGN_RELAXED) && (m_Canon != DKIM_SIGN_RELAXED_SIMPLE)) {
		m_Canon = DKIM_SIGN_SIMPLE;
	}
	sSelector.assign(pOptions->szSelector);
	m_pfnHdrCallback = pOptions->pfnHeaderCallback;
	sDomain.assign(pOptions->szDomain);
	m_IncludeBodyLengthTag = (pOptions->nIncludeBodyLengthTag != 0);
	m_nBodyLength = 0;
	m_ExpireTime = pOptions->expireTime;
	sIdentity.assign(pOptions->szIdentity);
	m_nIncludeTimeStamp = pOptions->nIncludeTimeStamp;
	m_nIncludeQueryMethod = pOptions->nIncludeQueryMethod;
	m_nIncludeCopiedHeaders = pOptions->nIncludeCopiedHeaders;
	m_nIncludeBodyHash = pOptions->nIncludeBodyHash;

// NOTE: the following line is not backwards compatible with MD 8.0.3
// because the szRequiredHeaders member was added after the release
//sRequiredHeaders.assign( pOptions->szRequiredHeaders );

//make sure there is a colon after the last header in the list
	if ((sRequiredHeaders.size() > 0) && sRequiredHeaders.at(sRequiredHeaders.size() - 1) != ':')
		sRequiredHeaders.append(":");
	m_nHash = pOptions->nHash;
	m_bReturnedSigAssembled = false;
	m_sCopiedHeaders.erase();
	return nRet;
}


// Hash - update the hash
void
CDKIMSign::Hash(const char *szBuffer, int nBufLength, bool bHdr, bool bAllmanOnly)
{
	EVP_MD_CTX     *p1, *p2, *p3, *p4, *p5;
#if OPENSSL_VERSION_NUMBER >= 0x10100000L
	p1 = m_allman_sha1ctx;
	p2 = m_Hdr_ietf_sha1ctx;
	p3 = m_Hdr_ietf_sha256ctx;
	p4 = m_Bdy_ietf_sha1ctx;
	p5 = m_Bdy_ietf_sha256ctx;
#else
	p1 = &m_allman_sha1ctx;
	p2 = &m_Hdr_ietf_sha1ctx;
	p3 = &m_Hdr_ietf_sha256ctx;
	p4 = &m_Bdy_ietf_sha1ctx;
	p5 = &m_Bdy_ietf_sha256ctx;
#endif
	if (bAllmanOnly) {
		if (m_nIncludeBodyHash & DKIM_BODYHASH_ALLMAN_1)
			EVP_SignUpdate(p1, szBuffer, nBufLength);
	} else {
		if (m_nIncludeBodyHash < DKIM_BODYHASH_IETF_1)
			EVP_SignUpdate(p1, szBuffer, nBufLength);
		else
		if (m_nIncludeBodyHash & DKIM_BODYHASH_IETF_1) {
			if (m_nIncludeBodyHash & DKIM_BODYHASH_ALLMAN_1)
				EVP_SignUpdate(p1, szBuffer, nBufLength);
#ifdef HAVE_EVP_SHA256
			if (m_nHash & DKIM_HASH_SHA256) {
				if (bHdr)
					EVP_SignUpdate(p3, szBuffer, nBufLength);
				else
					EVP_DigestUpdate(p5, szBuffer, nBufLength);
			}
			if (m_nHash != DKIM_HASH_SHA256) {
				if (bHdr)
					EVP_SignUpdate(p2, szBuffer, nBufLength);
				else
					EVP_DigestUpdate(p4, szBuffer, nBufLength);
			}
#else
			if (bHdr)
				EVP_SignUpdate(p2, szBuffer, nBufLength);
			else
				EVP_DigestUpdate(p4, szBuffer, nBufLength);
#endif
		}
	}
}


////////////////////////////////////////////////////////////////////////////////
// 
// SignThisTag - return boolean whether or not to sign this tag
//
////////////////////////////////////////////////////////////////////////////////
bool CDKIMSign::SignThisTag(const string &sTag)
{
	bool            bRet = true;

	if (_strnicmp(sTag.c_str(), "X-", 2) == 0
		|| _stricmp(sTag.c_str(), "Authentication-Results:") == 0
		|| _stricmp(sTag.c_str(), "DKIM-Signature:") == 0
		|| _stricmp(sTag.c_str(), "Received:") == 0
		|| _stricmp(sTag.c_str(), "Return-Path:") == 0)
	{
		bRet = false;
	}
	return bRet;
}

bool
ConvertHeaderToQuotedPrintable(const char *source, char *dest)
{
	bool            bConvert = false;

// do quoted printable
	static unsigned char hexchars[16] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };
	unsigned char  *d = (unsigned char *) dest;
	for (const unsigned char *s = (const unsigned char *)source; *s != '\0'; s++) {
		if (*s >= 33 && *s <= 126 && *s != '=' && *s != ':' && *s != ';' && *s != '|') {
			*d++ = *s;
		}

		else {
			bConvert = true;
			*d++ = '=';
			*d++ = hexchars[*s >> 4];
			*d++ = hexchars[*s & 15];
		}
	}
	*d = '\0';
	return bConvert;
}


////////////////////////////////////////////////////////////////////////////////
// 
// GetHeaderParams - Extract any needed header parameters
//
////////////////////////////////////////////////////////////////////////////////
void
CDKIMSign::GetHeaderParams(const string & sHdr)
{
	if (_strnicmp(sHdr.c_str(), "X", 1) == 0)
		return;
	if (_strnicmp(sHdr.c_str(), "From:", 5) == 0)
		sFrom.assign(sHdr.c_str() + 5);
	if (_strnicmp(sHdr.c_str(), "Sender:", 7) == 0)
		sSender.assign(sHdr.c_str() + 7);
	if (_strnicmp(sHdr.c_str(), "Return-Path:", 12) == 0)
		sReturnPath.assign(sHdr.c_str() + 12);
	if (m_nIncludeCopiedHeaders) {
		string::size_type pos = sHdr.find(':');
		if (pos != string::npos) {
			string          sTag, sValue;
			char           *workBuffer = new char[sHdr.size() * 3 + 1];
			sTag.assign(sHdr.substr(0, pos));
			sValue.assign(sHdr.substr(pos + 1, string::npos));
			ConvertHeaderToQuotedPrintable(sTag.c_str(), workBuffer);
			if (!m_sCopiedHeaders.empty()) {
				m_sCopiedHeaders.append("|");
			}
			m_sCopiedHeaders.append(workBuffer);
			m_sCopiedHeaders.append(":");
			ConvertHeaderToQuotedPrintable(sValue.c_str(), workBuffer);
			m_sCopiedHeaders.append(workBuffer);
			delete[]workBuffer;
		}
	}
}

// ProcessHeaders - sign headers and save needed parameters
int
CDKIMSign::ProcessHeaders(void)
{
	map <string, list <string>::reverse_iterator> IterMap;
	map <string, list <string>::reverse_iterator>::iterator IterMapIter;
	list <string>::reverse_iterator riter;
	list <string>::iterator iter;
	string          sTag;
	bool            bFromHeaderFound = false;

	// walk the header list
	for (iter = HeaderList.begin(); iter != HeaderList.end(); iter++) {
		sTag.assign(*iter);
		// look for a colon
		string::size_type pos = sTag.find(':');
		if (pos != string::npos) {
			int             nSignThisTag = 1;
			// hack off anything past the colon
			sTag.erase(pos + 1, string::npos);
			// is this the From: header?
			if (_stricmp(sTag.c_str(), "From:") == 0) {
				bFromHeaderFound = true;
				nSignThisTag = 1;
				IsRequiredHeader(sTag);	// remove from required header list
			} 
			// is this in the list of headers that must be signed?
			else
			if (IsRequiredHeader(sTag))
				nSignThisTag = 1;
			else {
				if (m_pfnHdrCallback)
					nSignThisTag = m_pfnHdrCallback(iter->c_str());
				else
					nSignThisTag = SignThisTag(sTag) ? 1 : 0;
			}
			// save header parameters
			GetHeaderParams(*iter);
			if (nSignThisTag > 0) {
				// add this tag to h=
				hParam.append(sTag);
				IterMapIter = IterMap.find(sTag);
				riter = (IterMapIter == IterMap.end())? HeaderList.rbegin() : IterMapIter->second;
				// walk the list in reverse looking for the last instance of this header
				while (riter != HeaderList.rend()) {
					if (_strnicmp(riter->c_str(), sTag.c_str(), sTag.size()) == 0) {
						ProcessHeader(*riter);
						// save the reverse iterator position for this tag
						riter++;
						IterMap[sTag] = riter;
						break;
					}
					riter++;
				}
			}
		}
	}
	Hash("\r\n", 2, true, true);	// only for Allman sig
	if (!bFromHeaderFound) {
		string sFrom("From:");
		hParam.append(sFrom);
		IsRequiredHeader(sFrom);	// remove from required header list
	}
	hParam.append(sRequiredHeaders);
	if (hParam.at(hParam.size() - 1) == ':')
		hParam.erase(hParam.size() - 1, string::npos);
	return DKIM_SUCCESS;
}

char           *DKIM_CALL
CDKIMSign::GetDomain(void)
{
	if (ParseFromAddress() == false)
		return ((char *) 0);
	return ((char *) sDomain.c_str());
}

void
CDKIMSign::ProcessHeader(const string & sHdr)
{
	switch (HIWORD(m_Canon)) {
	case DKIM_CANON_SIMPLE:
		Hash(sHdr.c_str(), sHdr.size(), true);
		Hash("\r\n", 2, true);
		break;
	case DKIM_CANON_NOWSP:
		{
		string sTemp = sHdr;
		RemoveSWSP(sTemp);
		// convert characters before ':' to lower case
		for (char *s = (char *)sTemp.c_str(); *s != '\0' && *s != ':'; s++) {
			if (*s >= 'A' && *s <= 'Z')
				*s += 'a' - 'A';
		}
		Hash(sTemp.c_str(), sTemp.size(), true);
		Hash("\r\n", 2, true);
		}
		break;
	case DKIM_CANON_RELAXED:
		{
		string sTemp = RelaxHeader(sHdr);
		Hash(sTemp.c_str(), sTemp.length(), true);
		Hash("\r\n", 2, true);
		}
		break;
	}
}

int CDKIMSign::ProcessBody(char *szBuffer, int nBufLength, bool bEOF)
{
	switch (LOWORD(m_Canon)) {
	case DKIM_CANON_SIMPLE:
		if (nBufLength > 0) {
			while (m_EmptyLineCount > 0) {
				Hash("\r\n", 2, false);
				m_nBodyLength += 2;
				m_EmptyLineCount--;
			}
			Hash(szBuffer, nBufLength, false);
			Hash("\r\n", 2, false);
			m_nBodyLength += nBufLength + 2;
		} else {
			m_EmptyLineCount++;
			if (bEOF) {
				Hash("\r\n", 2, false);
				m_nBodyLength += 2;
			}
		}
		break;
	case DKIM_CANON_NOWSP:
		RemoveSWSP(szBuffer, nBufLength);
		if (nBufLength > 0) {
			Hash(szBuffer, nBufLength, false);
			m_nBodyLength += nBufLength;
		}
		break;
	case DKIM_CANON_RELAXED:
		CompressSWSP(szBuffer, nBufLength);
		if (nBufLength > 0) {
			while (m_EmptyLineCount > 0) {
				Hash("\r\n", 2, false);
				m_nBodyLength += 2;
				m_EmptyLineCount--;
			}
			Hash(szBuffer, nBufLength, false);
			m_nBodyLength += nBufLength;
			if (!bEOF) {
				Hash("\r\n", 2, false);
				m_nBodyLength += 2;
			}
		} else
			m_EmptyLineCount++;
		break;
	}
	return DKIM_SUCCESS;
}

bool CDKIMSign::ParseFromAddress(void)
{
	string::size_type pos;
	string          sAddress;
	char           *p, *at;

	/* thanks to fred */
	if (!sReturnPath.empty())
		sAddress.assign(sReturnPath);
	else
	if (!sSender.empty())
		sAddress.assign(sSender);
	else
	if (!sFrom.empty())
		sAddress.assign(sFrom);
	else
		return false;
	// simple for now, beef it up later
	// remove '<' and anything before it
	pos = sAddress.find('<');
	if (pos != string::npos)
		sAddress.erase(0, pos);
	// remove '>' and anything after it
	pos = sAddress.find('>');
	if (pos != string::npos)
		sAddress.erase(pos, string::npos);
	// look for '@' symbol
	pos = sAddress.find('@');
	if (pos == string::npos)
		return false;
	if (sDomain.empty()) {
		p = getenv("DKIMDOMAIN");
		if (p && *p)
		{
			if (!(at = strchr(p, '@')))
				at = p;
			else
				at++;
			sDomain.assign(at);
		} else
			sDomain.assign(sAddress.c_str() + pos + 1);
		RemoveSWSP(sDomain);
	}
	return true;
}


////////////////////////////////////////////////////////////////////////////////
// 
// InitSig - initialize signature folding algorithm
//
////////////////////////////////////////////////////////////////////////////////
void
CDKIMSign::InitSig(void)
{
	m_sSig.reserve(1024);
	m_sSig.assign("DKIM-Signature:");
	m_nSigPos = m_sSig.size();
}

////////////////////////////////////////////////////////////////////////////////
// 
// AddTagToSig - add tag and value to signature folding if necessary
//               if bFold, fold at cbrk char
//
////////////////////////////////////////////////////////////////////////////////
void CDKIMSign::AddTagToSig(char *Tag, const string & sValue, char cbrk, bool bFold)
{
	int
	                nTagLen = strlen(Tag);
	AddInterTagSpace((!bFold) ? sValue.size() + nTagLen + 2 : nTagLen + 2);
	m_sSig.append(Tag);
	m_sSig.append("=");
	m_nSigPos += 1 + nTagLen;
	if (!bFold) {
		m_sSig.append(sValue);
		m_nSigPos += sValue.size();
	}

	else {
		AddFoldedValueToSig(sValue, cbrk);
	}
	m_sSig.append(";");
	m_nSigPos++;
}


////////////////////////////////////////////////////////////////////////////////
// 
// AddTagToSig - add tag and numeric value to signature folding if necessary
//
////////////////////////////////////////////////////////////////////////////////
void CDKIMSign::AddTagToSig(char *Tag, unsigned long nValue)
{
	char            szValue[64];
	sprintf(szValue, "%lu", nValue);
	AddTagToSig(Tag, szValue, 0, false);
}

////////////////////////////////////////////////////////////////////////////////
// 
// AddInterTagSpace - add space or fold here
//
////////////////////////////////////////////////////////////////////////////////
void CDKIMSign::AddInterTagSpace(int nSizeOfNextTag)
{
	if (m_nSigPos + nSizeOfNextTag + 1 > OptimalHeaderLineLength) {
		m_sSig.append("\n\t");
		m_nSigPos = 1;
	}

	else {
		m_sSig.append(" ");
		m_nSigPos++;
	}
}


////////////////////////////////////////////////////////////////////////////////
// 
// AddTagToSig - add value to signature folding if necessary
//               if cbrk == 0 fold anywhere, otherwise fold only at cbrk
//
////////////////////////////////////////////////////////////////////////////////
void CDKIMSign::AddFoldedValueToSig(const string & sValue, char cbrk)
{
	string::size_type pos = 0;
	if (cbrk == 0) {

	// fold anywhere
		while (pos < sValue.size()) {
			string::size_type len = OptimalHeaderLineLength - m_nSigPos;
			if (len > sValue.size() - pos)
				len = sValue.size() - pos;
			m_sSig.append(sValue.substr(pos, len));
			m_nSigPos += len;
			pos += len;
			if (pos < sValue.size()) {
				m_sSig.append("\n\t");
				m_nSigPos = 1;
			}
		}
	}

	else {

	// fold only at cbrk
		while (pos < sValue.size()) {
			string::size_type len = OptimalHeaderLineLength - m_nSigPos;
			string::size_type brkpos;
			if (sValue.size() - pos < len) {
				brkpos = sValue.size() - 1;
			}

			else {
				brkpos = sValue.rfind(cbrk, pos + len);
			}
			if (brkpos == string::npos || brkpos < pos) {
				brkpos = sValue.find(cbrk, pos);
				if (brkpos == string::npos) {
					brkpos = sValue.size();
				}
			}
			len = brkpos - pos + 1;
			m_sSig.append(sValue.substr(pos, len));
			m_nSigPos += len;
			pos += len;
			if (pos < sValue.size()) {

			//m_sSig.append( "\r\n\t" );
				m_sSig.append("\n\t");
				m_nSigPos = 1;
			}
		}
	}
}


////////////////////////////////////////////////////////////////////////////////
// 
// GetSig - compute hash and return signature header in szSignature
//
////////////////////////////////////////////////////////////////////////////////
int CDKIMSign::GetSig(char *szPrivKey, char *szSignature, unsigned int nSigLength)
{
	if (szPrivKey == NULL) {
		return DKIM_BAD_PRIVATE_KEY;
	}
	if (szSignature == NULL) {
		return DKIM_BUFFER_TOO_SMALL;
	}
	int             nRet = AssembleReturnedSig(szPrivKey);
	if (nRet != DKIM_SUCCESS)
		return nRet;
	if (m_sReturnedSig.size() + 1 < nSigLength)
		strcpy(szSignature, m_sReturnedSig.c_str());
	else 
		return DKIM_BUFFER_TOO_SMALL;
	return DKIM_SUCCESS;
}


////////////////////////////////////////////////////////////////////////////////
// 
// GetSig - compute hash and return signature header in szSignature
//
////////////////////////////////////////////////////////////////////////////////
int CDKIMSign::GetSig2(char *szPrivKey, char **pszSignature)
{
	if (szPrivKey == NULL) {
		return DKIM_BAD_PRIVATE_KEY;
	}
	if (pszSignature == NULL) {
		return DKIM_BUFFER_TOO_SMALL;
	}
	int             nRet = AssembleReturnedSig(szPrivKey);
	if (nRet != DKIM_SUCCESS)
		return nRet;
	*pszSignature = (char *) m_sReturnedSig.c_str();
	return DKIM_SUCCESS;
}


////////////////////////////////////////////////////////////////////////////////
// 
// IsRequiredHeader - Check if header in required list. If so, delete
//                    header from list.
//
////////////////////////////////////////////////////////////////////////////////
bool CDKIMSign::IsRequiredHeader(const string & sTag)
{
	string::size_type start = 0;
	string::size_type end = sRequiredHeaders.find(':');
	while (end != string::npos) {

	// check for a zero-length header
		if (start == end) {
			sRequiredHeaders.erase(start, 1);
		}

		else {
			if (_stricmp(sTag.c_str(), sRequiredHeaders.substr(start, end - start + 1).c_str()) == 0) {
				sRequiredHeaders.erase(start, end - start + 1);
				return true;
			}

			else {
				start = end + 1;
			}
		}
		end = sRequiredHeaders.find(':', start);
	}
	return false;
}

int CDKIMSign::ConstructSignature(char *szPrivKey, bool bUseIetfBodyHash, bool bUseSha256)
{
	string          sSignedSig;
	unsigned char  *sig;
	EVP_PKEY       *pkey;
	BIO            *bio, *b64;
	unsigned int    siglen;
	int             size, len;
	char           *buf, *cptr; 
	const char     *ptr, *dptr, *sptr;
	EVP_MD_CTX     *p1, *p2, *p3, *p4, *p5;
#if OPENSSL_VERSION_NUMBER >= 0x10100000L
	p1 = m_allman_sha1ctx;
	p2 = m_Hdr_ietf_sha1ctx;
	p3 = m_Hdr_ietf_sha256ctx;
	p4 = m_Bdy_ietf_sha1ctx;
	p5 = m_Bdy_ietf_sha256ctx;
#else
	p1 = &m_allman_sha1ctx;
	p2 = &m_Hdr_ietf_sha1ctx;
	p3 = &m_Hdr_ietf_sha256ctx;
	p4 = &m_Bdy_ietf_sha1ctx;
	p5 = &m_Bdy_ietf_sha256ctx;
#endif

// construct the DKIM-Signature: header and add to hash
	InitSig();
	if (bUseIetfBodyHash) {
		AddTagToSig((char *) "v", (char *) "1", 0, false);
	}
#ifdef HAVE_EVP_SHA256
	AddTagToSig((char *) "a", bUseSha256 ? "rsa-sha256" : "rsa-sha1", 0, false);
#else
	AddTagToSig((char *) "a", "rsa-sha1", 0, false);
#endif
	switch (m_Canon) {
	case DKIM_SIGN_SIMPLE:
		AddTagToSig((char *) "c", "simple", 0, false);
		break;
	case DKIM_SIGN_SIMPLE_RELAXED:
		AddTagToSig((char *) "c", "simple/relaxed", 0, false);
		break;
	case DKIM_SIGN_RELAXED:
		AddTagToSig((char *) "c", "relaxed/relaxed", 0, false);
		break;
	case DKIM_SIGN_RELAXED_SIMPLE:
		AddTagToSig((char *) "c", "relaxed", 0, false);
		break;
	}
	AddTagToSig((char *) "d", sDomain, 0, false);
	/*- replace % with domain name */
	ptr = sSelector.c_str();
	if ((sptr = strchr(ptr, '%'))) {
		dptr = sDomain.c_str();
		for (sptr = ptr, len = 0;*sptr;sptr++) {
			if (*sptr == '%')
				len += (int) strlen(dptr) + 1;
			else
				len++;
		}
		if (!(buf = new char[len]))
			return DKIM_OUT_OF_MEMORY;
		for (cptr = buf, sptr = ptr; *sptr; sptr++) {
			if (*sptr == '%') {
				memcpy(cptr, dptr, (len = strlen(dptr)));
				cptr += len;
			} else
				*cptr++ = *sptr;
		}
		*cptr = 0;
		sSelector.assign(buf);
		delete[]buf;
	}
	AddTagToSig((char *) "s", sSelector, 0, false);
	if (m_IncludeBodyLengthTag) {
		AddTagToSig((char *) "l", m_nBodyLength);
	}
	if (m_nIncludeTimeStamp != 0) {
		time_t t;
		time(&t);
		AddTagToSig((char *) "t", t);
	}
	if (m_ExpireTime != 0) {
		AddTagToSig((char *) "x", m_ExpireTime);
	}
	if (!sIdentity.empty()) {
		AddTagToSig((char *) "i", sIdentity, 0, false);
	}
	if (m_nIncludeQueryMethod) {
		AddTagToSig((char *) "q", bUseIetfBodyHash ? "dns/txt" : "dns", 0, false);
	}
	AddTagToSig((char *) "h", hParam, ':', true);
	if (m_nIncludeCopiedHeaders) {
		AddTagToSig((char *) "z", m_sCopiedHeaders, 0, true);
	}
	if (bUseIetfBodyHash) {
		unsigned char Hash[EVP_MAX_MD_SIZE];
		unsigned int nHashLen = 0;
#ifdef HAVE_EVP_SHA256
		EVP_DigestFinal(bUseSha256 ? p5 : p4, Hash, &nHashLen);
#else
		EVP_DigestFinal(p4, Hash, &nHashLen);
#endif
		bio = BIO_new(BIO_s_mem());
		if (!bio) {
			return DKIM_OUT_OF_MEMORY;
		}
		b64 = BIO_new(BIO_f_base64());
		if (!b64) {
			BIO_free(bio);
			return DKIM_OUT_OF_MEMORY;
		}
		BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
		BIO_push(b64, bio);
		if (BIO_write(b64, Hash, nHashLen) < (int) nHashLen) {
			BIO_free_all(b64);
			return DKIM_OUT_OF_MEMORY;
		}
		BIO_flush(b64);
		len = nHashLen * 2;
		buf = new char[len];
		if (buf == NULL) {
			BIO_free_all(b64);
			return DKIM_OUT_OF_MEMORY;
		}
		size = BIO_read(bio, buf, len);
		BIO_free_all(b64);
		// this should never happen
		if (size >= len) {
			delete[]buf;
			return DKIM_OUT_OF_MEMORY;
		}
		buf[size] = '\0';
		AddTagToSig((char *) "bh", buf, 0, true);
		delete[]buf;
	}
	AddInterTagSpace(3);
	m_sSig.append("b=");
	m_nSigPos += 2;
	// Force a full copy - no reference copies please
	sSignedSig.assign(m_sSig.c_str());
	// note that since we're not calling hash here, need to dump this
	// to the debug file if you want the full canonical form
	string          sTemp;
	if (HIWORD(m_Canon) == DKIM_CANON_RELAXED)
		sTemp = RelaxHeader(sSignedSig);
	else
		sTemp = sSignedSig.c_str();
	if (bUseIetfBodyHash) {
#ifdef HAVE_EVP_SHA256
		EVP_SignUpdate(bUseSha256 ? p3 : p2, sTemp.c_str(), sTemp.size());
#else
		EVP_SignUpdate(p2, sTemp.c_str(), sTemp.size());
#endif
	} else
		EVP_SignUpdate(p1, sTemp.c_str(), sTemp.size());
	if (!(bio = BIO_new_mem_buf(szPrivKey, -1)))
		return DKIM_OUT_OF_MEMORY;
	pkey = PEM_read_bio_PrivateKey(bio, NULL, NULL, NULL);
	BIO_free(bio);
	if (!pkey) {
		return DKIM_BAD_PRIVATE_KEY;
	}
	siglen = EVP_PKEY_size(pkey);
	int             nSignRet;
	sig = (unsigned char *) OPENSSL_malloc(siglen);
	if (sig == NULL) {
		EVP_PKEY_free(pkey);
		return DKIM_OUT_OF_MEMORY;
	}
	if (bUseIetfBodyHash) {
#ifdef HAVE_EVP_SHA256
		nSignRet = EVP_SignFinal(bUseSha256 ? p3 : p2, sig, &siglen, pkey);
#else
		nSignRet = EVP_SignFinal(p2, sig, &siglen, pkey);
#endif
	} else
		nSignRet = EVP_SignFinal(p1, sig, &siglen, pkey);
	EVP_PKEY_free(pkey);
	if (!nSignRet) {
		OPENSSL_free(sig);
		return DKIM_BAD_PRIVATE_KEY;	// key too small
	}
	bio = BIO_new(BIO_s_mem());
	if (!bio) {
		return DKIM_OUT_OF_MEMORY;
	}
	b64 = BIO_new(BIO_f_base64());
	if (!b64) {
		BIO_free(bio);
		return DKIM_OUT_OF_MEMORY;
	}
	BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
	BIO_push(b64, bio);
	if (BIO_write(b64, sig, siglen) < (int) siglen) {
		OPENSSL_free(sig);
		BIO_free_all(b64);
		return DKIM_OUT_OF_MEMORY;
	}
	BIO_flush(b64);
	OPENSSL_free(sig);
	len = siglen * 2;
	buf = new char[len];
	if (buf == NULL) {
		BIO_free_all(b64);
		return DKIM_OUT_OF_MEMORY;
	}
	size = BIO_read(bio, buf, len);
	BIO_free_all(b64);
	// this should never happen
	if (size >= len) {
		delete[]buf;
		return DKIM_OUT_OF_MEMORY;
	}
	buf[size] = '\0';
	AddFoldedValueToSig(buf, 0);
	delete[]buf;
	return DKIM_SUCCESS;
}

int CDKIMSign::AssembleReturnedSig(char *szPrivKey)
{
	int             nRet;
	if (m_bReturnedSigAssembled)
		return DKIM_SUCCESS;
	ProcessFinal();
	if (ParseFromAddress() == false) {
		//return DKIM_NO_SENDER;
	}
	Hash("\r\n", 2, true, true);	// only for Allman sig
	string allmansha1sig,
#ifdef HAVE_EVP_SHA256
		ietfsha256Sig,
#endif
		ietfsha1Sig;
	if (m_nIncludeBodyHash < DKIM_BODYHASH_IETF_1) {
		nRet = ConstructSignature(szPrivKey, false, false);
		if (nRet == DKIM_SUCCESS)
			allmansha1sig.assign(m_sSig);
		else
			return nRet;
	} else
	if (m_nIncludeBodyHash & DKIM_BODYHASH_IETF_1) {
		if (m_nIncludeBodyHash & DKIM_BODYHASH_ALLMAN_1) {
			if ((nRet = ConstructSignature(szPrivKey, false, false)) == DKIM_SUCCESS)
				allmansha1sig.assign(m_sSig);
			else
				return nRet;
		}
#ifdef HAVE_EVP_SHA256
		if (m_nHash & DKIM_HASH_SHA256) {
			if ((nRet = ConstructSignature(szPrivKey, true, true)) == DKIM_SUCCESS)
				ietfsha256Sig.assign(m_sSig);
			else
				return nRet;
		}
		if (m_nHash != DKIM_HASH_SHA256) {
			if ((nRet = ConstructSignature(szPrivKey, true, false)) == DKIM_SUCCESS)
				ietfsha1Sig.assign(m_sSig);
			else
				return nRet;
		}
#else
		if ((nRet = ConstructSignature(szPrivKey, true, false)) == DKIM_SUCCESS)
			ietfsha1Sig.assign(m_sSig);
		else
			return nRet;
#endif
	}
	m_sReturnedSig.assign(allmansha1sig);
	if (!ietfsha1Sig.empty()) {
		if (!m_sReturnedSig.empty())
			m_sReturnedSig.append("\n");
		m_sReturnedSig.append(ietfsha1Sig);
	}
#ifdef HAVE_EVP_SHA256
	if (!ietfsha256Sig.empty()) {
		if (!m_sReturnedSig.empty())
			m_sReturnedSig.append("\n");
		m_sReturnedSig.append(ietfsha256Sig);
	}
#endif
	m_bReturnedSigAssembled = true;
	return DKIM_SUCCESS;
}

void
getversion_dkimsign_cpp()
{
	static char    *x = (char *) "$Id: dkimsign.cpp,v 1.13 2018-08-25 18:01:59+05:30 Cprogrammer Exp mbhangui $";

	x++;
}
