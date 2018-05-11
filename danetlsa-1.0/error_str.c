/*
 * $Log: $
 */

#define X(e,s) if (i == e) return s;

char *getdns_error_str(i)
int i;
{
  X(0,"no error")
  X(GETDNS_MEM_ERR,               "out of memory")
  X(GETDNS_TIMEOUT_ERR,           "timed out")
  X(GETDNS_CALLBACK_ERR,          "callback error")
  X(GETDNS_CALLBACKCANCEL_ERR,    "callback cancel error")
  X(GETDNS_NO_EXiST_DOMAIN_ERR,   "non existent domain")
  X(GETDNS_NO_SECURE_ANSWER_ERR,  "no secure answers")
  X(GETDNS_ALL_BOGUS_ANSWER_ERR,  "all bogus answers")
  X(GETDNS_RES_INDETERMINATE_ERR, "result indeterminate")
  X(GETDNS_DICT_RESPONSE_ERR,     "error getting dictionary response")
  X(GETDNS_DICT_ANSWER_ERR,       "error getting dictionary answer")
  X(GETDNS_ZERO_REPLY_ERR,        "no response to query")
  X(GETDNS_DNSSEC_STATUS_ERR,     "error getting DNSSEC status")
  X(GETDNS_DNSSEC_INSECURE_ERR,   "DNSSEC response insecure")
  return "unknown error";
}

void
getversion_getdns_error_str_c()
{
	static char    *x = "$Id: $";

	x++;
}
