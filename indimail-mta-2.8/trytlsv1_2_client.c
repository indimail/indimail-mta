#include <openssl/ssl.h>
int
main()
{
	SSL_CTX_new(TLSv1_2_client_method());
}
