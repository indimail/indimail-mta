/*
 * $Log: dkimverify.h,v $
 * Revision 1.2  2009-03-26 15:12:15+05:30  Cprogrammer
 * changes for ADSP
 *
 * Revision 1.1  2009-03-21 08:50:22+05:30  Cprogrammer
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

#ifndef DKIMVERIFY_H
#define DKIMVERIFY_H

#include "dkimbase.h"
#include <vector>

#define DKIM_SIG_VERSION_PRE_02			0
#define DKIM_SIG_VERSION_02_PLUS		1

class           SelectorInfo {
 public:
	SelectorInfo(const string & sSelector, const string & sDomain);
	~SelectorInfo();

	string          Domain;
	string          Selector;
	string          Granularity;
	bool            AllowSHA1;
	bool            AllowSHA256;
	EVP_PKEY       *PublicKey;	/* the public key */
	bool            Testing;
	bool            SameDomain;
	int             Status;
	int             Parse(char *Buffer);
};

class           SignatureInfo {
public:
	SignatureInfo(bool SaveCanonicalizedData);
	~SignatureInfo();

	void            Hash(const char *szBuffer, unsigned nBufLength, bool IsBody = false);
	string          Header;
	unsigned        Version;
	string          Domain;
	string          Selector;
	string          SignatureData;
	string          BodyHashData;
	string          IdentityLocalPart;
	string          IdentityDomain;
	string          CanonicalizedData;
	vector <string> SignedHeaders;
	int             BodyLength;
	unsigned        HeaderCanonicalization;
	unsigned        BodyCanonicalization;
	int             ExpireTime;
	unsigned        VerifiedBodyCount;
	unsigned        UnverifiedBodyCount;
	EVP_MD_CTX      m_Hdr_ctx;
	EVP_MD_CTX      m_Bdy_ctx;
	SelectorInfo   *m_pSelector;
	int             Status;
	int             m_nHash;	// use one of the DKIM_HASH_xxx constants here
	unsigned        EmptyLineCount;
	bool            m_SaveCanonicalizedData;
};

class           CDKIMVerify:public CDKIMBase {
public:

	CDKIMVerify();
	~CDKIMVerify();

	int             Init(DKIMVerifyOptions * pOptions);
	int             GetResults(int *sCount, int *sSize);
	int             GetDetails(int *nSigCount, DKIMVerifyDetails ** pDetails);
	virtual int     ProcessHeaders(void);
	virtual int     ProcessBody(char *szBuffer, int nBufLength, bool bEOF);
	const char     *GetPractices() {return Practices.c_str();}
	char           *DKIM_CALL GetDomain(void);

protected:
	int             ParseDKIMSignature(const string & sHeader, SignatureInfo & sig);
	SelectorInfo   &GetSelector(const string & sSelector, const string & sDomain);
	int             GetADSP(const string &sDomain, int &iADSP);
	int             GetSSP(const string &sDomain, int &iSSP, bool & bTesting);
	list <SignatureInfo> Signatures;
	list <SelectorInfo> Selectors;
	DKIMDNSCALLBACK m_pfnSelectorCallback;	// selector record callback
	DKIMDNSCALLBACK m_pfnPracticesCallback;	// SSP record callback
	bool            m_HonorBodyLengthTag;
	bool            m_CheckPractices;
	bool            m_Accept3ps;		//TBS(Luc) : accept 3rd party signature(s)
	bool            m_SubjectIsRequired;
	bool            m_SaveCanonicalizedData;
	vector <DKIMVerifyDetails> Details;
	string          Practices;
};

#endif	/*- DKIMVERIFY_H */
