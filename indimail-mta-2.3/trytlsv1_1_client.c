#include <openssl/ssl.h>
int
main()
{
	SSL_CTX_new(TLSv1_1_client_method());
}
