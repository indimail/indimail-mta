cert-req: req.pem
cert cert-req: QMAIL/control/clientcert.pem
	@:

QMAIL/control/clientcert.pem: QMAIL/control/servercert.pem
	cd QMAIL/control;ln -s servercert.pem clientcert.pem

QMAIL/control/servercert.pem:
	PATH=$$PATH:/usr/local/ssl/bin \
		openssl req -new -x509 -nodes -days 366 -out $@ -keyout $@
	chmod 640 $@
	chown indimail:indimail $@

req.pem:
	PATH=$$PATH:/usr/local/ssl/bin openssl req \
		-new -nodes -out $@ -keyout QMAIL/control/servercert.pem
	chmod 640 QMAIL/control/servercert.pem
	chown indimail:indimail QMAIL/control/servercert.pem
	@echo
	@echo "Send req.pem to your CA to obtain signed_req.pem, and do:"
	@echo "cat signed_req.pem >> QMAIL/control/servercert.pem"
