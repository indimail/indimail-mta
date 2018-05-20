/*
 * query_getdns.c
 */
#define _GNU_SOURCE
#include <stdio.h>
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef HAVE_STDINT_H
#include <stdint.h>
#endif
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif
#ifdef HAVE_STRING_H
#include <string.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_INTTYPES_H
#include <inttypes.h>
#endif
#ifdef HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif
#ifdef HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif
#ifdef HAVE_ARPA_INET_H
#include <arpa/inet.h>
#endif
#ifdef HAVE_NETDB_H
#include <netdb.h>
#endif
#ifdef HAVE_OPENSSL_SSL_H
#include <openssl/ssl.h>
#endif
#ifdef HAVE_GETDNS_GETDNS_H
#include <getdns/getdns.h>
#endif
#ifdef HAVE_GETDNS_GETDNS_EXTRA_H
#include <getdns/getdns_extra.h>
#endif
#ifdef HAVE_GETDNS_GETDNS_EXT_LIBEVENT_H
#include <getdns/getdns_ext_libevent.h>
#endif
#ifdef HAVE_EVENT2_EVENT_H
#include <event2/event.h>
#else
#include <event.h>
#endif
#include "query-getdns.h"

static int      mylen;
static char    *error_store;
static int      cb_address_error, cb_tlsa_error, cb_mx_error;

static void
discard_stack(void)
{
	free((void *) error_store);
	error_store = (char *) 0;
	mylen = 0;
	return;
}

#ifdef HAVE_STDARG_H
#include <stdarg.h>
char           *
print_stack(const char *fmt, ...)
#else
#include <varargs.h>
char           *
print_stack(va_alist)
va_dcl
#endif
{
#ifndef HAVE_STDARG_H
	char           *fmt;
#endif
	va_list         ap;
	int             len, i;
	char           *ptr, *errorstr;
	static char     sserrbuf[512];

#ifndef HAVE_STDARG_H
	va_start(ap);
	fmt = va_arg(ap, char *);
#endif
	if (fmt && *fmt) {
#ifdef HAVE_STDARG_H
		va_start(ap, fmt);
#endif
		if (vasprintf(&errorstr, fmt, ap) == -1)
			return ((char *) 0);
		va_end(ap);
		len = strlen(errorstr) + 1;
		if (!(error_store = realloc(error_store, mylen + len + 1))) {	/*- The man page is wierd on Mac OS */
			free(errorstr);
			return ((char *) 0);
		}
		strncpy(error_store + mylen, errorstr, len); /*- this will copy the null byte */
		free(errorstr);
		error_store[mylen + len - 1] = 0;
		mylen += len;
		return (error_store);
	} else {
		if (!error_store)
			return ((char *) 0);
		for (ptr = error_store, i = len = 0; len < mylen; len++, ptr++) {
			if (*ptr == 0) {
				fprintf(stderr, "%s", error_store + i);
				i = len + 1;
			} 
		}
		discard_stack();
		return ("");
	}
}

static struct addrinfo *
insert_addrinfo(struct addrinfo *current, struct addrinfo *new)
{
	if (current == NULL)
		addresses = new;
	else
		current->ai_next = new;
	return new;
}

static struct addrinfo *
make_addrinfo(getdns_dict *address, const char *hostname, uint16_t port)
{
	getdns_return_t rc;
	getdns_bindata *addr_type, *addr_data;
	struct addrinfo *aip = NULL;

	if ((rc = getdns_dict_get_bindata(address, "address_type", &addr_type))) {
		print_stack("%s: getting addr_type: %s\n", hostname, getdns_get_errorstr_by_id(rc));
		return NULL;
	}

	if ((rc = getdns_dict_get_bindata(address, "address_data", &addr_data))) {
		print_stack("%s: getting addr_data: %s\n", hostname, getdns_get_errorstr_by_id(rc));
		return NULL;
	}

	aip = malloc(sizeof (struct addrinfo));
	aip->ai_flags = 0;
	aip->ai_canonname = NULL;
	aip->ai_next = NULL;

	if (!strncmp((const char *) addr_type->data, "IPv4", 4)) {
		struct sockaddr_in *sa4 = malloc(sizeof (struct sockaddr_storage));
		aip->ai_family = sa4->sin_family = AF_INET;
		sa4->sin_port = htons(port);
		memcpy(&(sa4->sin_addr), addr_data->data, addr_data->size);
		aip->ai_addr = (struct sockaddr *) sa4;
		aip->ai_addrlen = sizeof (struct sockaddr_in);
	} else
	if (!strncmp((const char *) addr_type->data, "IPv6", 4)) {
		struct sockaddr_in6 *sa6 = malloc(sizeof (struct sockaddr_storage));
		aip->ai_family = sa6->sin6_family = AF_INET6;
		sa6->sin6_port = htons(port);
		memcpy(&(sa6->sin6_addr), addr_data->data, addr_data->size);
		aip->ai_addr = (struct sockaddr *) sa6;
		aip->ai_addrlen = sizeof (struct sockaddr_in6);
	} else { /*- shouldn't get here */
		print_stack("make_addrinfo: Unknown address type\n");
		free(aip);
		return NULL;
	}
	return aip;
}

/*-
 * all_responses_secure()
 */
static int
all_responses_secure(getdns_dict *response)
{
	size_t          i, cnt_reply = 0, cnt_secure = 0;
	uint32_t        dnssec_status;
	getdns_return_t rc;
	getdns_list    *replies_tree;
	getdns_dict    *reply;

	if ((rc = getdns_dict_get_list(response, "replies_tree", &replies_tree))) {
		print_stack("getdns_dict_get_list: %s\n", getdns_get_errorstr_by_id(rc));
		return (1);
	}

	(void) getdns_list_get_length(replies_tree, &cnt_reply);
	if (cnt_reply == 0) {
		getdns_list_destroy(replies_tree);
		dns_bogus_or_indeterminate = 1;
		return (1);
	}

	for (i = 0; i < cnt_reply; i++) {
		if ((rc = getdns_list_get_dict(replies_tree, i, &reply))) {
			print_stack("get_list_get_dict: %s\n", getdns_get_errorstr_by_id(rc));
			getdns_list_destroy(replies_tree);
			return (1);
		}
		if ((rc = getdns_dict_get_int(reply, "dnssec_status", &dnssec_status))) {
			print_stack("getdns_dict_get_int: dnssec status: %s\n", getdns_get_errorstr_by_id(rc));
			getdns_list_destroy(replies_tree);
			return (1);
		}
		switch (dnssec_status)
		{
		case GETDNS_DNSSEC_SECURE:
			cnt_secure++;
			break;
		case GETDNS_DNSSEC_INSECURE:
			break;
		default:
			dns_bogus_or_indeterminate = 1;
		}
	}
	getdns_list_destroy(replies_tree);
	if (cnt_reply > 0 && cnt_secure == cnt_reply)
		return (0);
	else {
		if (cnt_reply == 0)
			dns_bogus_or_indeterminate = 1;
		return (1);
	}
}

/*- callback function for address lookups */
#define UNUSED_PARAM(x) ((void) (x))
static void
cb_address(getdns_context *ctx, getdns_callback_type_t cb_type, getdns_dict *response, void *userarg, getdns_transaction_t tid)
{
	UNUSED_PARAM(ctx);
	getdns_return_t rc;
	uint32_t        status = 0;
	qinfo          *qip = (qinfo *) userarg;
	const char     *hostname = qip->qname;
	uint16_t        port = qip->port;
	getdns_list    *just_addresses;
	size_t          cnt_addr;
	getdns_dict    *address;

	cb_address_error = 0;
	switch (cb_type)
	{
	case GETDNS_CALLBACK_COMPLETE:
		break;
	case GETDNS_CALLBACK_TIMEOUT:
		cb_address_error = GETDNS_TIMEOUT_ERR;
		print_stack("Callback: address query timed out: %s\n", hostname);
		return;
	case GETDNS_CALLBACK_CANCEL:
		cb_address_error = GETDNS_CALLBACKCANCEL_ERR;
		break;
	case GETDNS_CALLBACK_ERROR:
	default:
		cb_address_error = GETDNS_CALLBACK_ERR;
		print_stack("Callback address fail: %s, tid=%" PRIu64 " rc=%d\n", hostname, tid, cb_type);
		return;
	}

	/*- Check authenticated status of responses; set dns_bogus_indeterminate flag */
	if (!all_responses_secure(response)) {
		address_authenticated = 1;
		v4_authenticated = 1;
		v6_authenticated = 1;
	}

	if (getdns_dict_get_int(response, "status", &status)) {
		print_stack("could not get \"status\" from response\n");
		return;
	}

	switch (status)
	{
	case GETDNS_RESPSTATUS_GOOD:
		break;
	case GETDNS_RESPSTATUS_NO_NAME:
		cb_address_error = GETDNS_NO_EXIST_DOMAIN_ERR;
		print_stack("%s: Non existent domain name.\n", hostname);
		return;
	case GETDNS_RESPSTATUS_ALL_TIMEOUT:
		cb_address_error = GETDNS_TIMEOUT_ERR;
		dns_bogus_or_indeterminate = 1;
		print_stack("%s: Query timed out.\n", hostname);
		return;
	case GETDNS_RESPSTATUS_NO_SECURE_ANSWERS:
		cb_address_error = GETDNS_NO_SECURE_ANSWER_ERR;
		print_stack("%s: Insecure address records.\n", hostname);
		return;
	case GETDNS_RESPSTATUS_ALL_BOGUS_ANSWERS:
		cb_address_error = GETDNS_ALL_BOGUS_ANSWER_ERR;
		dns_bogus_or_indeterminate = 1;
		print_stack("%s: All bogus answers.\n", hostname);
		return;
	default:
		cb_address_error = GETDNS_RES_INDETERMINATE_ERR;
		dns_bogus_or_indeterminate = 1;
		print_stack("%s: error status code: %d.\n", hostname, status);
		return;
	}

	if ((rc = getdns_dict_get_list(response, "just_address_answers", &just_addresses))) {
		cb_address_error = GETDNS_DICT_RESPONSE_ERR;
		print_stack("getdns_dict_get_list: %s\n", getdns_get_errorstr_by_id(rc));
		return;
	}

	if ((rc = getdns_list_get_length(just_addresses, &cnt_addr))) {
		print_stack("getdns_list_get_length: %s\n", getdns_get_errorstr_by_id(rc));
		return;
	}

	if (cnt_addr <= 0) {
		print_stack("%s: No addresses found.\n", hostname);
		return;
	}

	size_t          i;
	struct addrinfo *current = addresses;

	for (i = 0; i < cnt_addr; i++) {
		struct addrinfo *aip = NULL;
		if ((rc = getdns_list_get_dict(just_addresses, i, &address))) {
			cb_address_error = GETDNS_DICT_RESPONSE_ERR;
			print_stack("getns_list_get_dict: %s: getting address dict: %s\n", hostname, getdns_get_errorstr_by_id(rc));
			break;
		}
		if (!(aip = make_addrinfo(address, hostname, port)))
			continue;
		current = insert_addrinfo(current, aip);
	}
	/*- getdns_dict_destroy(response); -*/
	return;
}

/*-
 * tlsa_rdata: structure to hold TLSA record rdata.
 * insert_tlsa_rdata(): 
 * new records     - insert node at tail of linked list of tlsa_rdata.
 * existing record - return the tail
 * free_tlsa(): free memory in the linked list.
 */

static tlsa_rdata *
insert_tlsa_rdata(tlsa_rdata **headp, tlsa_rdata *new)
{
	tlsa_rdata     *rp, *ptr;
	char           *cp1, *cp2;
	int             len;

    if (*headp == NULL)
        *headp = new;
	else {
		for (ptr = rp = *headp; rp != NULL; rp = rp->next) {
			ptr = rp;
			cp1 = bin2hexstring(rp->data, rp->data_len);
			cp2 = bin2hexstring(new->data, new->data_len);
			len = strlen(cp1);
			if (rp->usage == new->usage && rp->selector == new->selector &&
					rp->mtype == new->mtype && !memcmp(rp->data, new->data, rp->data_len)
					&&!memcmp(rp->host, new->host, rp->hostlen) && !memcmp(cp1, cp2, len)) {
				free(cp1);
				free(cp2);
				return ((tlsa_rdata *) 0);
			}
			free(cp1);
			free(cp2);
		}
        ptr->next = new;
	}
    return new;
}

void
free_tlsa(tlsa_rdata *head)
{
    tlsa_rdata *current;

    while ((current = head) != NULL) {
		head = head->next;
		free(current->data);
		free(current->host);
		current->data = 0;
		current->host = 0;
		free(current);
    }
    return;
}

/*
 * callback function for tlsa lookups
 */
static void
cb_tlsa(getdns_context *ctx, getdns_callback_type_t cb_type, getdns_dict *response, void *userarg, getdns_transaction_t tid)
{
	UNUSED_PARAM(ctx);
	getdns_return_t rc;
	uint32_t        status = 0, dstatus = 0;
	qinfo          *qip = (qinfo *) userarg;
	const char     *hostname = qip->qname;
	getdns_list    *replies_tree, *answer;
	size_t          i, j, num_replies, num_answers;
	getdns_dict    *reply;

	cb_tlsa_error = 0;
	switch (cb_type)
	{
	case GETDNS_CALLBACK_COMPLETE:
		break;
	case GETDNS_CALLBACK_TIMEOUT:
		cb_tlsa_error = GETDNS_TIMEOUT_ERR;
		return;
	case GETDNS_CALLBACK_CANCEL:
		cb_tlsa_error = GETDNS_CALLBACKCANCEL_ERR;
		break;
	case GETDNS_CALLBACK_ERROR:
	default:
		/*- "Callback address fail: %s/TLSA, tid=%" PRIu64 " rc=%d\n", hostname, tid, cb_type); */
		cb_tlsa_error = GETDNS_CALLBACK_ERR;
		return;
	}

	(void) getdns_dict_get_int(response, "status", &status);

	switch (status)
	{
	case GETDNS_RESPSTATUS_GOOD:
		break;
	case GETDNS_RESPSTATUS_NO_NAME:
		cb_tlsa_error = GETDNS_NO_EXIST_DOMAIN_ERR;
		goto cleanup;
	case GETDNS_RESPSTATUS_ALL_TIMEOUT:
		cb_tlsa_error = GETDNS_TIMEOUT_ERR;
		dns_bogus_or_indeterminate = 1;
		goto cleanup;
	case GETDNS_RESPSTATUS_NO_SECURE_ANSWERS:
		cb_tlsa_error = GETDNS_NO_SECURE_ANSWER_ERR;
		goto cleanup;
	case GETDNS_RESPSTATUS_ALL_BOGUS_ANSWERS:
		cb_tlsa_error = GETDNS_ALL_BOGUS_ANSWER_ERR;
		dns_bogus_or_indeterminate = 1;
		goto cleanup;
	default:
		cb_tlsa_error = GETDNS_RES_INDETERMINATE_ERR;
		dns_bogus_or_indeterminate = 1;
		goto cleanup;
	}

	if ((rc = getdns_dict_get_list(response, "replies_tree", &replies_tree))) {
		cb_tlsa_error = GETDNS_DICT_RESPONSE_ERR;
		goto cleanup;
	}

	(void) getdns_list_get_length(replies_tree, &num_replies);

	if (num_replies <= 0) {
		cb_tlsa_error = GETDNS_ZERO_REPLY_ERR;
		dns_bogus_or_indeterminate = 1;
		goto cleanup;
	}

	size_t          auth_count = 0;

	for (i = 0; i < num_replies; i++) {
		if ((rc = getdns_list_get_dict(replies_tree, i, &reply))) {
			cb_tlsa_error = GETDNS_DICT_RESPONSE_ERR;
			print_stack("%s/TLSA: getting reply: %s\n", hostname, getdns_get_errorstr_by_id(rc));
			break;
		}

		if ((rc = getdns_dict_get_int(reply, "dnssec_status", &dstatus))) {
			cb_tlsa_error = GETDNS_DNSSEC_STATUS_ERR;
			print_stack("%s/TLSA: error obtaining dnssec status: %s\n", hostname, getdns_get_errorstr_by_id(rc));
			goto cleanup;
		}

		switch (dstatus)
		{
		case GETDNS_DNSSEC_SECURE:
			auth_count++;
			break;
		case GETDNS_DNSSEC_INSECURE:
			cb_tlsa_error = GETDNS_DNSSEC_INSECURE_ERR;
			break;
		default:
			dns_bogus_or_indeterminate = 1;
		}

		if ((rc = getdns_dict_get_list(reply, "answer", &answer))) {
			cb_tlsa_error = GETDNS_DICT_ANSWER_ERR;
			print_stack("%s/TLSA: getting answer section: %s\n", hostname, getdns_get_errorstr_by_id(rc));
			break;
		}

		(void) getdns_list_get_length(answer, &num_answers);

		for (j = 0; j < num_answers; j++) {
			getdns_dict    *rr;
			uint32_t        rrtype, usage, selector, mtype;
			getdns_bindata *certdata = NULL;

			if ((rc = getdns_list_get_dict(answer, j, &rr))) {
				cb_tlsa_error = GETDNS_DICT_ANSWER_ERR;
				print_stack("%s/TLSA: getting rr %zu: %s\n", hostname, j, getdns_get_errorstr_by_id(rc));
				break;
			}

			if ((rc = getdns_dict_get_int(rr, "/type", &rrtype))) {
				cb_tlsa_error = GETDNS_DICT_ANSWER_ERR;
				print_stack("%s/TLSA: getting rrtype: %s\n", hostname, getdns_get_errorstr_by_id(rc));
				break;
			}
			if (rrtype != GETDNS_RRTYPE_TLSA)
				continue;
			if ((rc = getdns_dict_get_int(rr, "/rdata/certificate_usage", &usage))) {
				cb_tlsa_error = GETDNS_DICT_ANSWER_ERR;
				print_stack("%s/TLSA: getting usage: %s\n", hostname, getdns_get_errorstr_by_id(rc));
				break;
			}
			if ((rc = getdns_dict_get_int(rr, "/rdata/selector", &selector))) {
				cb_tlsa_error = GETDNS_DICT_ANSWER_ERR;
				print_stack("%s/TLSA: getting selector: %s\n", hostname, getdns_get_errorstr_by_id(rc));
				break;
			}
			if ((rc = getdns_dict_get_int(rr, "/rdata/matching_type", &mtype))) {
				cb_tlsa_error = GETDNS_DICT_ANSWER_ERR;
				print_stack("%s/TLSA: getting mtype: %s\n", hostname, getdns_get_errorstr_by_id(rc));
				break;
			}
			if ((rc = getdns_dict_get_bindata(rr, "/rdata/certificate_association_data", &certdata))) {
				cb_tlsa_error = GETDNS_DICT_ANSWER_ERR;
				print_stack("%s/TLSA: getting certdata: %s\n", hostname, getdns_get_errorstr_by_id(rc));
				break;
			}
			tlsa_rdata     *rp = (tlsa_rdata *) malloc(sizeof (tlsa_rdata));
			if (!rp) {
				cb_tlsa_error = GETDNS_MEM_ERR;
				return;
			}
			rp->usage = usage;
			rp->selector = selector;
			rp->mtype = mtype;
			rp->data_len = certdata->size;
			if (!(rp->data = malloc(certdata->size))) {
				cb_tlsa_error = GETDNS_MEM_ERR;
				goto cleanup;
			}
			rp->hostlen = strlen(qip->qname);
			if (!(rp->host = malloc(rp->hostlen + 1))) {
				cb_tlsa_error = GETDNS_MEM_ERR;
				goto cleanup;
			}
			memcpy(rp->host, qip->qname, rp->hostlen + 1);
			memcpy(rp->data, certdata->data, certdata->size);
			rp->next = NULL;
			if (!(insert_tlsa_rdata(&tlsa_rdata_list, rp))) {
				free(rp->data);
				free(rp->host);
				free(rp);
			} else
				tlsa_count++;
		}
	}
	if (auth_count == num_replies)
		tlsa_authenticated = 1;
cleanup:
	/*- getdns_dict_destroy(response); -*/
	return;
}

/*
 * do_dns_queries()
 * asynchronously dispatch address and TLSA queries & wait for results.
 * Response data is obtained by the associated callback functions.
 */
int
do_dns_queries(const char *hostname, uint16_t port, int recursive)
{

	char            domainstring[512];
	getdns_context *context = NULL;
	getdns_dict    *extensions = NULL;
	getdns_return_t rc;
	qinfo           qip;
	struct event_base *evb;

	if ((rc = getdns_context_create(&context, 1)) != GETDNS_RETURN_GOOD) {
		print_stack("getdns_context_create: %s\n", getdns_get_errorstr_by_id(rc));
		return (1);
	}

	if (!recursive)
		getdns_context_set_resolution_type(context, GETDNS_RESOLUTION_STUB);
	if (!(extensions = getdns_dict_create())) {
		print_stack("getdns_dict_create\n");
		getdns_context_destroy(context);
		return (1);
	}

	if ((rc = getdns_dict_set_int(extensions, "dnssec_return_status", GETDNS_EXTENSION_TRUE))) {
		print_stack("getdns_dict_set_int: %s\n", getdns_get_errorstr_by_id(rc));
		getdns_dict_destroy(extensions);
		getdns_context_destroy(context);
		return (1);
	}

	if (!(evb = event_base_new())) {
		print_stack("event base creation failed.\n");
		getdns_dict_destroy(extensions);
		getdns_context_destroy(context);
		return (1);
	}

	(void) getdns_extension_set_libevent_base(context, evb);

	getdns_transaction_t tid = 0;

	/*- Address Records lookup */
	qip.qname = hostname;
	qip.qtype = GETDNS_RRTYPE_A;
	qip.port = port;
	if ((rc = getdns_address(context, hostname, extensions, (void *) &qip, &tid,
			cb_address)) != GETDNS_RETURN_GOOD) {
		print_stack("%s address query failed: %s\n", hostname, getdns_get_errorstr_by_id(rc));
		event_base_free(evb);
		getdns_dict_destroy(extensions);
		getdns_context_destroy(context);
		return (1);
	}
	if (cb_address_error) {
		event_base_free(evb);
		getdns_dict_destroy(extensions);
		getdns_context_destroy(context);
		return (1);
	}

	/*- TLSA Records lookup */
	snprintf(domainstring, sizeof (domainstring), "_%d._tcp.%s", port, hostname);
	qip.qname = domainstring;
	qip.qtype = GETDNS_RRTYPE_TLSA;
	qip.port = port;
	if ((rc = getdns_general(context, domainstring, GETDNS_RRTYPE_TLSA, extensions, (void *) &qip,
			&tid, cb_tlsa)) != GETDNS_RETURN_GOOD) {
		print_stack("%s TLSA query failed: %s\n", domainstring, getdns_get_errorstr_by_id(rc));
		event_base_free(evb);
		getdns_dict_destroy(extensions);
		getdns_context_destroy(context);
		return (1);
	}
	if (event_base_dispatch(evb) == -1) {
		print_stack("event_base_dispatch: Error in dispatching events.\n");
		event_base_free(evb);
		getdns_dict_destroy(extensions);
		getdns_context_destroy(context);
		return (1);
	}
	event_base_free(evb);
	getdns_dict_destroy(extensions);
	getdns_context_destroy(context);
	return ((cb_tlsa_error == GETDNS_NO_ERR || cb_tlsa_error == GETDNS_NO_EXIST_DOMAIN_ERR) ? 0 : cb_tlsa_error);
}

/*-
 * bin2hexstring(): convert binary input into a string of hex digits.
 * Caller needs to free returned memory.
 */

char *bin2hexstring(uint8_t *data, size_t length)
{
    size_t k;
    char *outstring, *p;

    outstring = (char *) malloc(2 *length + 1);
    p = outstring;
    for (k = 0; k < length; k++) {
        snprintf(p, 3, "%02x", (unsigned int) *(data+k));
        p += 2;
    }
    return outstring;
}

char *
bin2txt(uint8_t *str, size_t size)
{
	char           *p, *q;
	int             k, pos = 0;

	if (!(q = malloc(sizeof(char *) * size + 1)))
		return ((char *) 0);
	for (p = q, k = 0; k < size; k++) {
		if (pos == k) {
			pos += str[k] + 1;
			if (k)
				*p++ = '.';
			continue;
		}
		*p++ = str[k];
	}
	*(p - 1) = 0;
	return (q);
}

/*-
 * mx_rdata: structure to hold MX record rdata.
 * insert_mx_rdata(): 
 * new records     - insert node at tail of linked list of mx_rdata.
 * existing record - return the tail
 * free_mx(): free memory in the linked list.
 */

static mx_rdata *
insert_mx_rdata(mx_rdata **headp, mx_rdata *new)
{
	mx_rdata       *rp;
	mx_rdata       *ptr;
	char           *cp1, *cp2;
	int             len;

    if (*headp == NULL)
        *headp = new;
	else {
		for (ptr = rp = *headp; rp != NULL; rp = rp->next) {
			ptr = rp;
			cp1 = bin2txt(rp->data, rp->data_len);
			cp2 = bin2txt(new->data, new->data_len);
			len = strlen(cp1);
			if (rp->pref == new->pref && !memcmp(rp->data, new->data, rp->data_len) && !memcmp(cp1, cp2, len)) {
				free(cp1);
				free(cp2);
				return ((mx_rdata *) 0);
			}
			free(cp1);
			free(cp2);
		}
        ptr->next = new;
	}
    return new;
}

void
free_mx(mx_rdata *head)
{
    mx_rdata *current;

    while ((current = head) != NULL) {
		head = head->next;
		free(current->data);
		current->data = 0;
		free(current);
    }
    return;
}

static void
cb_mx(getdns_context *ctx, getdns_callback_type_t cb_type, getdns_dict *response, void *userarg, getdns_transaction_t tid)
{
	UNUSED_PARAM(ctx);
	getdns_return_t rc;
	uint32_t        status = 0, dstatus = 0;
	const char     *hostname = (char *) userarg;
	getdns_list    *replies_tree, *answer;
	size_t          i, j, num_replies, num_answers;
	getdns_dict    *reply;

	cb_mx_error = 0;
	switch (cb_type)
	{
	case GETDNS_CALLBACK_COMPLETE:
		break;
	case GETDNS_CALLBACK_TIMEOUT:
		cb_mx_error = GETDNS_TIMEOUT_ERR;
		return;
	case GETDNS_CALLBACK_CANCEL:
		cb_mx_error = GETDNS_CALLBACKCANCEL_ERR;
		break;
	case GETDNS_CALLBACK_ERROR:
	default:
		/*- "Callback address fail: %s/TLSA, tid=%" PRIu64 " rc=%d\n", hostname, tid, cb_type); */
		cb_mx_error = GETDNS_CALLBACK_ERR;
		return;
	}

	(void) getdns_dict_get_int(response, "status", &status);

	switch (status)
	{
	case GETDNS_RESPSTATUS_GOOD:
		break;
	case GETDNS_RESPSTATUS_NO_NAME:
		cb_mx_error = GETDNS_NO_EXIST_DOMAIN_ERR;
		goto cleanup;
	case GETDNS_RESPSTATUS_ALL_TIMEOUT:
		cb_mx_error = GETDNS_TIMEOUT_ERR;
		dns_bogus_or_indeterminate = 1;
		goto cleanup;
	default:
		cb_mx_error = GETDNS_RES_INDETERMINATE_ERR;
		dns_bogus_or_indeterminate = 1;
		goto cleanup;
	}

	if ((rc = getdns_dict_get_list(response, "replies_tree", &replies_tree))) {
		cb_mx_error = GETDNS_DICT_RESPONSE_ERR;
		goto cleanup;
	}

	(void) getdns_list_get_length(replies_tree, &num_replies);

	if (num_replies <= 0) {
		cb_mx_error = GETDNS_ZERO_REPLY_ERR;
		goto cleanup;
	}

	for (i = 0; i < num_replies; i++) {
		if ((rc = getdns_list_get_dict(replies_tree, i, &reply))) {
			cb_mx_error = GETDNS_DICT_RESPONSE_ERR;
			print_stack("%s/MX: getting reply: %s\n", hostname, getdns_get_errorstr_by_id(rc));
			break;
		}

		if ((rc = getdns_dict_get_list(reply, "answer", &answer))) {
			cb_mx_error = GETDNS_DICT_ANSWER_ERR;
			print_stack("%s/MX: getting answer section: %s\n", hostname, getdns_get_errorstr_by_id(rc));
			break;
		}

		(void) getdns_list_get_length(answer, &num_answers);
		for (j = 0; j < num_answers; j++) {
			getdns_dict    *rr;
			uint32_t        rrtype;
			uint32_t        pref;
			char           *cp;
			getdns_bindata *exchange = NULL;

			if ((rc = getdns_list_get_dict(answer, j, &rr))) {
				print_stack("%s/MX: getting rr %zu: %s\n", hostname, j, getdns_get_errorstr_by_id(rc));
				break;
			}

			if ((rc = getdns_dict_get_int(rr, "/type", &rrtype))) {
				print_stack("%s/MX: getting rrtype: %s\n", hostname, getdns_get_errorstr_by_id(rc));
				break;
			}
			if (rrtype != GETDNS_RRTYPE_MX)
				continue;
			if ((rc = getdns_dict_get_int(rr, "/rdata/preference", &pref))) {
				print_stack("%s/MX: getting usage: %s\n", hostname, getdns_get_errorstr_by_id(rc));
				break;
			}
			if ((rc = getdns_dict_get_bindata(rr, "/rdata/exchange", &exchange))) {
				print_stack("%s/MX: getting exchange: %s\n", hostname, getdns_get_errorstr_by_id(rc));
				break;
			}
			mx_rdata     *rp = (mx_rdata *) malloc(sizeof (mx_rdata));
			if (!rp) {
				cb_mx_error = GETDNS_MEM_ERR;
				return;
			}
			rp->pref = pref;
			rp->data_len = exchange->size;
			if (!(rp->data = malloc(exchange->size))) {
				cb_mx_error = GETDNS_MEM_ERR;
				goto cleanup;
			}
			memcpy(rp->data, exchange->data, exchange->size);
			if (!(insert_mx_rdata(&mx_rdata_list, rp))) {
				free(rp->data);
				free(rp);
			} else
				mx_count++;
		}
	}
cleanup:
	/*- getdns_dict_destroy(response); -*/
	return;
}

int
do_mx_queries(const char *hostname, int recursive)
{

	getdns_context *context = NULL;
	getdns_dict    *extensions = NULL;
	getdns_return_t rc;
	struct event_base *evb;

	if ((rc = getdns_context_create(&context, 1)) != GETDNS_RETURN_GOOD) {
		print_stack("getdns_context_create: %s\n", getdns_get_errorstr_by_id(rc));
		return (1);
	}

	if (!recursive)
		getdns_context_set_resolution_type(context, GETDNS_RESOLUTION_STUB);
	if (!(extensions = getdns_dict_create())) {
		print_stack("getdns_dict_create\n");
		getdns_context_destroy(context);
		return (1);
	}

	if ((rc = getdns_dict_set_int(extensions, "dnssec_return_status", GETDNS_EXTENSION_TRUE))) {
		print_stack("getdns_dict_set_int: %s\n", getdns_get_errorstr_by_id(rc));
		getdns_dict_destroy(extensions);
		getdns_context_destroy(context);
		return (1);
	}

	if (!(evb = event_base_new())) {
		print_stack("event_base_new: event base creation failed.\n");
		getdns_dict_destroy(extensions);
		getdns_context_destroy(context);
		return (1);
	}

	(void) getdns_extension_set_libevent_base(context, evb);
	getdns_transaction_t tid = 0;

	/*- MX Records lookup */
	if ((rc = getdns_general(context, hostname, GETDNS_RRTYPE_MX, extensions, (void *) hostname,
			&tid, cb_mx)) != GETDNS_RETURN_GOOD) {
		print_stack("%s MX query failed: %s\n", hostname, getdns_get_errorstr_by_id(rc));
		event_base_free(evb);
		getdns_dict_destroy(extensions);
		getdns_context_destroy(context);
		return (1);
	}
	if (event_base_dispatch(evb) == -1) {
		print_stack("Error in dispatching events.\n");
		event_base_free(evb);
		getdns_dict_destroy(extensions);
		getdns_context_destroy(context);
		return (1);
	}
	event_base_free(evb);
	getdns_dict_destroy(extensions);
	getdns_context_destroy(context);
	return (cb_address_error);
}
