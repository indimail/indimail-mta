#include <openssl/ssl.h>
int
main()
{
	SSL_CTX_new(TLSv1_2_server_method());
}
