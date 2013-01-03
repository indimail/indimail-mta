/* ntlm.h  -- interface declarations for SMB authentication code */

#include "smbtypes.h"

/* 
 * These structures are byte-order dependant, and should not
 * be manipulated except by the use of the routines provided
 */

typedef struct
{
uint16  len;
uint16  maxlen;
uint32  offset;
}tSmbStrHeader;

typedef struct
{
char          ident[8];
uint32        msgType;
uint32        flags;
tSmbStrHeader    user;
tSmbStrHeader    domain;
uint8         buffer[1024];
uint32        bufIndex;
}tSmbNtlmAuthRequest;

typedef struct
{
char          ident[8];
uint32        msgType;
tSmbStrHeader    uDomain;
uint32        flags;
uint8         challengeData[8];
uint32        context[2];
tSmbStrHeader    targetInfo;
uint8         buffer[1024];
uint32        bufIndex;
}tSmbNtlmAuthChallenge;


typedef struct
{
char          ident[8];
uint32        msgType;
tSmbStrHeader    lmResponse;
tSmbStrHeader    ntResponse;
tSmbStrHeader    uDomain;
tSmbStrHeader    uUser;
tSmbStrHeader    uWks;
tSmbStrHeader    sessionKey;
uint32        flags;
uint8         buffer[1024];
uint32        bufIndex;
}tSmbNtlmAuthResponse;

/* public: */

#define SmbLength(ptr) (((ptr)->buffer - (uint8*)(ptr)) + (ptr)->bufIndex)

extern void dumpSmbNtlmAuthRequest(FILE *fp, tSmbNtlmAuthRequest *request);
extern void dumpSmbNtlmAuthChallenge(FILE *fp, tSmbNtlmAuthChallenge *challenge);
extern void dumpSmbNtlmAuthResponse(FILE *fp, tSmbNtlmAuthResponse *response);

extern void buildSmbNtlmAuthRequest(tSmbNtlmAuthRequest *request, char *user, char *domain);
extern void buildSmbNtlmAuthResponse(tSmbNtlmAuthChallenge *challenge, tSmbNtlmAuthResponse *response, char *user, char *password);

/* ntlm.h ends here */
