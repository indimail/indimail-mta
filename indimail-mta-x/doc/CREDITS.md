# CREDITS

The credit for inspiration to write/put together this package is entire due to Dr. Daniel Bernstein. Had he not written qmail the way it is written, i wouldn't probably have fallen in love with the code. In no way is this package endorsed by DJB. Though I have put in lot of effort putting together this package, any bugs or flaws could be entirely due to me. So I would be glad if the community could further improve this. Since I have gained so much personally as well as professionally due to qmail, the least I can do is to put back my experiences and learnings for anyone who would be interested. I release my entire code as Open Source GPL.

My contribution is miniscule compared to what I have picked up from others in building the packages. Since this was being done for my own personal use, over the years, I may have missed mentioning some parts of the code that has been written by others. In case you know that I have missed out giving credit to anyone for any of the code or idea, let me know and I will be glad to add it here.

# ACKNOWLEDGEMENTS

I acknowledge all the following people, urls, packages, code mentioned below

qmail, serialmail, qmailanalog, dotforward, fastforward, mess822, daemontools, ucspi-tcp, Courier IMAP/POP3, Bogofilter - A Bayesian Spam Filter, Fetchmail, and other useful utilities for messaging.

I thank Dr. Daniel Bernstein for the following original packages at http://cr.yp.to

1. qmail-1.03
2. dotforward
3. fastforward
4. qmailanalog
5. serialmail
6. ucspi-tcp
7. daemontools
8. mess822
9. checkpassword

My ex colleagues Sushant TS, Murali Panchapakesan, Govind Raghuram, Ramya Krishnan, Anuradha TP, Premnath Sah who came with lot of ideas and suggestion and snippets of code. A great number of bugs were identified by Ramya who became an expert in using strace after this. She also added the X-QHPSI, X-Quarantine-ID: and the VIRUSFORWARD code in qhpsi.

The site http://qmail.org and the qmail mailing list for wealth of information

1.  Russel Nelson - qmail-lint, qmail-dk, big-todo, antivirus patch for qmail-smtpd, selfhelo, reject relay probes, logselect, MXPS
2.  http://www.inter7.com - vpopmail - Chris Johnson, Ken Jones, Bill Shupp, Tom Collins.
3.  Chris Johnson - tarpitting, RELAYMAILFROM patch
4.  Chris Kennedy - Blackhole patch
5.  Charles Cazabon - doublebounce-trim-patch http://www.qmail.org/doublebounce-trim.patch
6.  Klaus Reimer - bouncecontrol patch 
7.  Johannes Erdfelt - big-concurrency patch
8.  Scott Gifford - 0.0.0.0 patch
9.  Chuck Foster - binding outgoing connections to local interface
10. Claudio Jeker <jeker@n-r-g.com> and Andre Oppermann <opi@nrg4u.com> - ext-todo
11. Andreas Aardal Hanssen - ext-todo + big-todo
12. Bruce Guenter - QMAILQUEUE, qmail-qstat, big-todo Patch mods
13. Bruce Guenter - rate limiting autoresponder based on Eric Huss's design
14. Bruce Guenter - syncdir
15. Bruce Guenter - qmail-qfilter, QUEUE\_EXTRA patch
16. Dave Sill - Life with qmail, inst\_check script
17. Erwin Hoffman - QHPSI, Authenticated SMTP, RFC1870 SMTP SIZE, SPAMCONTROL, RECIPIENTS
18. Gerrit Pape - Man pages for ucspi-tcp, daemontools-0.76
19. Peter Samuels tai64nunix
20. John Levine - patch to matchup.c to accept tai64n dates
21. John Levine - Auth SMTP for ofmipd.c
22. Georg Lehner - qmailanalog better integrated with multilog
23. http://www.magma.com.ni/moin/TipsAnd/QmailAnalog
24. Chris Garrigues - pretty-print Received: lines.
25. ftp://ftp.foxharp.boston.ma.us/pub/pgf/qmail/mailroute.pl
26. Matt Ranney - qmail-lagcheck
27. qmail-liberal-lf patch author: Dean Gaudet version: 0.95
28. Eric Huss - queue-fix (with patch by Matthew Harrel to work for big-todo)
29. Eric Huss - qmail-qmqpc timeout patch
30. William E Baxtar - [qtools](http://www.superscript.com/qtools/intro.html) SIGINT to multilog to stop writing to log, Save/discard logs with multilog (SIGINT/SIGHUP to toggle)
31. Linux Magic - [qmail-remove](http://www.linuxmagic.com/opensource/qmail/qmail-remove/). LinuxMagic is the development arm of Wizard Tower TechnoServices Ltd
32. Alex Kramarov - qmail-print-queue
33. Folkert van Heusden - multitail
34. Russ Nelson/Ivan Kohler - mbox2maildir.pl
35. Tetsu Ushijima - [maildirdeliver](http://www.din.or.jp/~ushijima/maildirdeliver.html)
36. Evan Champion - patch to condredirect
37. John Saunders - patch to date822fmt.c (emit dates in local timezones), newline patch
38. Robert Sander - RECIPIENT Extension
39. Chuck Foster - bindroutes patch
40. Nick Leverton - holdremote patch
41. Fred Lindberg - Preserve MIME-ness of message when bouncing MIME message
42. Krzysztof Dabrowski - Authenticated SMTP Patch
43. Frederik Vermeulen - starttls patch
44. Frank DENIS - patch to truncate bounce messages
45. Matthias Andree - Found the 'qmail-local tab' bug and introduced the 'sendmail -N dsn' compatibility
46. Scott Gifford - ipme, moreipme patch
47. Charles Cazabon - patch to enforce single recipients on bounces.
48. Jay Soffian - auth smtp patch for qmail-remote
49. Adrian Ho - RFC 2821 in qmail-remote
50. Ingo Rohloff - SMTP authentication support to serialsmtp
51. Ward Vandewege - badrcptto patch
52. Kazinori Fujiwara - qmail ipv6 patch
53. Len Budney - qscanq
54. qmail spf implementation (https://www.saout.de/misc/spf/qmail-spf-rc5.patch) by Jana Saout <jana@saout.de>.  Christophe Saout - SPF checker
55. David Phillips - qmail's sendmail's -f set the default for the username like sendmail does.
56. Mark Belnap - Bounce Lifetime patch
57. Christopher K. Davis' [DNS patch](http://www.ckdhr.com/ckd/qmail-103.patch) handles a problem qmail has with DNS responses larger than the standard 512 bytes.
58. Toby Betts - Prevent collisions in Maildir style filenames - maildir uniq patch
59. Mark Delany - wildmat patch
60. Nagy Balazs - patch to ensure that the domain name on the envelope sender is a valid DNS name.
61. Jul - re-read concurrencylocal concurrency remote on SIGHUP.
62. Erik Sjölund - qmail-local tab patch
63. Frank Denis aka Jedi/Sector One <j at 4u.net> - Patched qmail-send to  
	- limit the size of bounces (bouncemaxbytes)
	- MAXRECIPIENT
	- maxhop control file
	- The qmail-link-sync patch for simple fix to the non-synchronous link() problem of qmail on Linux filesystems
64. Paul Gregg - ENFORCE\_FQDN\_HELO patch 29/08/2003
65. Nick Leverton - Qmail Holdremote Patch
66. PLDaniels - ripMIME/altermime
67. evaluate - evaluate algebraic strings(C) 2000-2002 Kyzer/CSG. http://www.kyzer.me.uk/code/evaluate/
68. "Marcus Williams" marcus at quintic.co.uk. - make seekable patch
69. daemontools patches from (additional signals to svc, svscan logging http://www.gluelogic.com/code/daemontools/)
70. Jos Backus - timed log rotation for multilog
71. Alin-Adrian Anton - Fixed qmail-smtpd vulnurability for very long header lines
72. http://members.elysium.pl/brush/qmail-smtpd-auth/ SMTP AUTH
73. Zach White <zwhite@netlsd.org> 20010812 http://www.netable.com/~dburkes/qmail-smtpd-requireauth/ Require AUTH
74. http://www.elysium.pl/members/brush/cmd5checkpw/ CRAM-MD5 Checkpassword RFC-2554, RFC-2222 compliance
75. Rask Ingemann Lambertsen - who provided the original RELAY Patch
76. Markus Stumpf - provided the original LOGGING patch
77. Charles Cazabon - Author of the NULL-Sender modification
78. Bjoern Kalkbrenner - Initial auther of the qmail-smtp-auth-send patch.
79. Peter Ladwig - had the idea to use hard tarpitting in case of too many invalid RECIPIENTS.
80. Flash Secure Menu Shell - WWW: http://www.netsoc.ucd.ie/flash/ Author: Steve Fegan
81. Levent Serinol - MySQL support to tcpserver
82. mpack/munpack from CMU
83. Richard Lyons - moresmtproutes patch
84. Andrew Richard's early talker patch
85. Animesh Bansriyar for his idea to limit some users to receiving only local email.
86. Flavio Curti for qmail-queue custom error patch
87. Fred McIntyre for suggesting DKIMSIGNOPTIONS and other improvements to qmail-dkim
88. Jason Haar <jhaar at users.sourceforge.net> for his tls-cert-check script
89. Andrew Richard, John Levine for Greylisting - http://www.gossamer-threads.com/lists/qmail/users/136740?page=last
90. Bounce Address Tag Validation - John Levine, Joerg Backschues
91. Alexey Mahotkin <alexm at hsys.msk.ru> for pam support functions and new pam-checkpwd program
92. Jeremy Kister - X-Originating-IP header qmail-1.03.originip-field.patch [http://jeremy.kister.net/code/qmail-1.03.originip-field.patch]
93. Joerg Backschues - badremotehost check function (Ability to block hosts listed in badhost)
94. James Raftery - real envelope recipient after host name canonicalization
95. Russ Nelson's QMTP patch for qmail-remote
96. DNSBL Support (DNS Blacklist) Author "Fabio Busatto" <fabio.busatto at sikurezza.org>
97. Pieter Droogendijk <pieter at binky.org.uk> http://binky.org.uk - URL parsing code for SURBL
98. Marcelo Coelho - qmail-srs-0.8.patch and libsrs2 - http://www.libsrs2.org
99. Roberto Puzzanghera <roberto.puzzanghera at gmail.com> - using FORCE\_TLS environment variable to force TLS during AUTH
100. Ed Neville - allow multiple Delivered-To in qmail-local using control file maxdeliveredo
101. Ed Neville - configure TLS method in control/tlsclientmethod (qmail-smtpd), control/tlsservermethod (qmail-remote)
102. Bruce Guenter - qmail-remote patch to reduce cpu
103. Rolf Eike Beer <eike@sf-mail.de> gen\_allocdefs.h, GEN\_ALLOC refactoring changes to fix memory overflow
104. Fefe - felix-libowfat@fefe.de for functions taken from libowfat & ipv6 in ucspi-tcp
105. RFC 6530-32 EAI - Adapted from Arnt Gulbrandsen / Erwin Hoffman's unicode address support patch for qmail.
106. RFC 8463 - Ed25519 cryptographic method adapted from Erwin Hoffman's s/qmail dkim implementation
