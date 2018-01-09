cert-req: req.pem
cert cert-req: SYSCONFDIR/control/clientcert.pem
	@:

SYSCONFDIR/control/clientcert.pem: SYSCONFDIR/control/servercert.pem
	cd SYSCONFDIR/control;ln -s servercert.pem clientcert.pem

SYSCONFDIR/control/servercert.pem:
	PATH=$$PATH:/usr/local/ssl/bin \
		openssl req -new -x509 -nodes -days 366 -out $@ -keyout $@
	chmod 640 $@
	chown indimail:indimail $@

req.pem:
	PATH=$$PATH:/usr/local/ssl/bin openssl req \
		-new -nodes -out $@ -keyout SYSCONFDIR/control/servercert.pem
	chmod 640 SYSCONFDIR/control/servercert.pem
	chown indimail:indimail SYSCONFDIR/control/servercert.pem
	@echo
	@echo "Send req.pem to your CA to obtain signed_req.pem, and do:"
	@echo "cat signed_req.pem >> SYSCONFDIR/control/servercert.pem"
