cert-req: req.pem
cert cert-req: SYSCONFDIR/certs/clientcert.pem
	@:

SYSCONFDIR/certs/clientcert.pem: SYSCONFDIR/certs/servercert.pem
	cd SYSCONFDIR/certs;ln -s servercert.pem clientcert.pem

SYSCONFDIR/certs/servercert.pem:
	openssl req -new -x509 -nodes -days 366 -out $@ -keyout $@
	chmod 640 $@
	chown indimail:indimail $@

req.pem:
	openssl req -new -nodes -out $@ -keyout SYSCONFDIR/certs/servercert.pem
	chmod 640 SYSCONFDIR/certs/servercert.pem
	chown indimail:indimail SYSCONFDIR/certs/servercert.pem
	@echo
	@echo "Send req.pem to your CA to obtain signed_req.pem, and do:"
	@echo "cat signed_req.pem >> SYSCONFDIR/certs/servercert.pem"
