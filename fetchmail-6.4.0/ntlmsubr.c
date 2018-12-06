#include "config.h"

#ifdef NTLM_ENABLE
#include "fetchmail.h"
#include "i18n.h"
#include "ntlm.h"
#include "socket.h"

#include <string.h>

int ntlm_helper(int sock, struct query *ctl, const char *proto)
{
/*
 * NTLM support by Grant Edwards.
 *
 * Handle MS-Exchange NTLM authentication method.  This is the same
 * as the NTLM auth used by Samba for SMB related services. We just
 * encode the packets in base64 instead of sending them out via a
 * network interface.
 *
 * Much source (ntlm.h, smb*.c smb*.h) was borrowed from Samba.
 */
    tSmbNtlmAuthRequest request;
    tSmbNtlmAuthChallenge challenge;
    tSmbNtlmAuthResponse response;

    char msgbuf[2048];
    int result;

    if ((result = gen_recv(sock, msgbuf, sizeof msgbuf)))
	return result;

    if (msgbuf[0] != '+' && strspn(msgbuf+1, " \t") < strlen(msgbuf+1)) {
	if (outlevel >= O_VERBOSE) {
	    report(stdout, GT_("Warning: received malformed challenge to \"AUTH(ENTICATE) NTLM\"!\n"));
	}
	result = PS_AUTHFAIL;
	goto cancelfail;
    }

    buildSmbNtlmAuthRequest(&request,ctl->remotename,NULL);

    if (outlevel >= O_DEBUG)
	dumpSmbNtlmAuthRequest(stdout, &request);

    memset(msgbuf,0,sizeof msgbuf);
    to64frombits (msgbuf, &request, SmbLength(&request), sizeof msgbuf);

    if (outlevel >= O_MONITOR)
	report(stdout, "%s> %s\n", proto, msgbuf);

    strcat(msgbuf,"\r\n");
    SockWrite (sock, msgbuf, strlen (msgbuf));

    if ((result = gen_recv(sock, msgbuf, sizeof msgbuf)))
	goto cancelfail;

    /*
     * < 0: decoding error
     * >= 0 < 32: too short to be plausible
     */
    if ((result = from64tobits (&challenge, msgbuf, sizeof(challenge))) < 0
	    || result < 32)
    {
	report (stderr, GT_("could not decode BASE64 challenge\n"));
	/* We do not goto cancelfail; the server has already sent the
	 * tagged reply, so the protocol exchange has ended, no need
	 * for us to send the asterisk. */
	return PS_AUTHFAIL;
    }

    /* validate challenge:
     * - ident
     * - message type
     * - that offset points into buffer
     * - that offset + length does not wrap
     * - that offset + length is not bigger than buffer */
    if (0 != memcmp("NTLMSSP", challenge.ident, 8)
	    || challenge.msgType != 2
	    || challenge.uDomain.offset > (unsigned)result
	    || (challenge.uDomain.offset + challenge.uDomain.len) < challenge.uDomain.offset
	    || (challenge.uDomain.offset + challenge.uDomain.len) > (unsigned)result)
    {
	report (stderr, GT_("NTLM challenge contains invalid data.\n"));
	result = PS_AUTHFAIL;
	goto cancelfail;
    }

    if (outlevel >= O_DEBUG)
	dumpSmbNtlmAuthChallenge(stdout, &challenge);

    buildSmbNtlmAuthResponse(&challenge, &response,ctl->remotename,ctl->password);

    if (outlevel >= O_DEBUG)
	dumpSmbNtlmAuthResponse(stdout, &response);

    memset(msgbuf,0,sizeof msgbuf);
    to64frombits (msgbuf, &response, SmbLength(&response), sizeof msgbuf);

    if (outlevel >= O_MONITOR)
	report(stdout, "%s> %s\n", proto, msgbuf);

    strcat(msgbuf,"\r\n");
    SockWrite (sock, msgbuf, strlen (msgbuf));

    return PS_SUCCESS;

cancelfail: /* cancel authentication and return failure */
    {
	if (outlevel >= O_MONITOR)
	    report(stdout, "%s> *\n", proto);
	SockWrite(sock, "*\r\n", 3);
	return result;
    }
}

#endif /* NTLM_ENABLE */
