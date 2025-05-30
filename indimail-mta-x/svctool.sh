#
# $Id: svctool.sh,v 2.732 2024-11-05 22:31:45+05:30 Cprogrammer Exp mbhangui $
#

#
# User Configuration
#
[ -z "$QmailHOME" ] && QmailHOME="/var/indimail"
[ -z "$INDIMAILDIR" ] && INDIMAILDIR="/var/indimail"
#indimaildir=`awk -F: '{print $6}' /etc/passwd|grep indimail|head -1`
indimaildir=$INDIMAILDIR
QmailBinPrefix=@prefix@
mandir=@mandir@
libexecdir=@libexecdir@
sysconfdir=@sysconfdir@
shareddir=@shareddir@
LOGDIR=@logdir@
CONCURRENCYINCOMING=150
min_free=52428800
DATABYTES=20971520
MASTER_HOST=localhost
MYSQL_HOST=localhost
MYSQL_USER=indimail
PRIV_USER=mysql
REPL_USER=repl
host=@HOST@
shared_objects=0
use_dlmopen=0
skip_sendmail_check=0
RCSID="# \$Id: svctool.sh,v 2.732 2024-11-05 22:31:45+05:30 Cprogrammer Exp mbhangui $"

#
# End of User Configuration
#
if [ -f /usr/bin/less ] ; then
	MORE=/usr/bin/less
else
	MORE=/usr/bin/more
fi
usage()
{
echo "Press ENTER for options, Cntrl C to quit" 1>&2
read key
$MORE <<EOF
Usage: svctool [OPTION]

Known values for OPTION are:

--smtp=Port --qbase=queue_path --qcount=N --qstart=I
  --servicedir=service_path
  [--cntrldir=cntrl_path]
  --localip=a --maxdaemons=m --maxperip=i
  [--no-multi]
  [--qtype=static|dynamic|compat]
  [--qmailqueue=qmailq]
  [--mysqlhost=mysqlhost --mysqluser=mysqluser --mysqlpass=mysqlpass]
  [--persistdb]
  [--dnscheck]
  [--helofqdn --helocheck]
  [--fsync --syncdir]
  [--qregex]
  [--memory=b]
  [--dmemory=c]
  [--msgqsize=Q]
  [--databytes=d]
  [--overquota_mailsize=o]
  [--domainlimits]
  [--authsmtp --forceauthsmtp --secureauth --enablecram --authall]
  [--chkrecipient --chksender --chkrelay --cugmail]
  [--masquerade] [--antispoof]
  [--routes=smtp|qmtp|static]
  [--cname-lookup]
  [--setuser-priv]
  [--sanitize-env=env_list]
  [--min-free=M --skipsend --deliverylimit-count=D --deliverylimit-size=S]
  [--rbl=list]
  [--content-filter=c]
  [--virus-filter=v]
  [--qhpsi=q]
  [--spamfilter=spamfilter_args]
  [--logfilter=logfifo]
  [--rejectspam=r --spamexitcode=e --localfilter --remotefilter]
  [--paranoid --dmasquerade]
  [--remote-authsmtp=b]
  [--odmr]
  [--starttls]
  [--ssl]
  [--forcetls]
  [--shared-objects=0|1]
  [--use-dlmopen=0|1]
  [--dkverify=dkim|none]
  [--dksign=dkim|none --private_key=private_key]
  [--password-cache] [--query-cache]
  [--smtp-plugin]
  [--setgroups]
  [--utf8]
  [--hide-host]
  [--barelf]
  --default-domain=domain

 Installs a new queue with a SMTP Listener
  m              - Concurrency of connections to allow
  i              - Concurrency of connections to be allowed from a
                   single ip address
  a              - IP Address to listen
  N              - No of Queues
  I              - Numeral Prefix of first queue (i.e 1 for @qmaildir@/queue/queue1)
  M              - Minimum Disk Space to maintain in queue after which
                   Temporary error will be returned
  b              - Max Memory that SMTP can allocate
  c              - Max Memory that qscheduler can allocate
  Q              - Max number of bytes in POSIX message queues
  d              - Max size of email that can be submitted to SMTP or
                   injected locally
  D              - Max deliveries allowed per day for a user
  S              - Max sum total size of mails allowed per day for a user
  o              - Max size of mails to accept after user is overquota
  no-multi       - Do not set QMAILQUEUE to qmail-multi
  qtype          - Set multi-queue as static, dynamic or dynamic in compat mode
  qmailq         - Path to any qmail-queue compatabile binary
  Port           - Bind on this port for SMTP
  service_path   - Path where supervise service will be installed
  queue_path     - Path where the queues are installed. If this is different from
                   @qmaildir@ appropriate links will be created in
                   @qmaildir@
  cntrl_path     - Path where Qmail control files are stored
  domainlimits   - Apply Domain Limits configured by vmoddomlimits(1)
  chkrecipient   - Perform a User Status query before accepting mails
                   (qmail-smtpd)
  chksender      - Enforce checks on local user and force authenticaion
                   (qmail-smtpd)
  cugmail        - Allow only local valid senders to use SMTP.
  antispoof      - Turn on antispoofing code
  authsmtp       - Enable Authenticated SMTP
  secureauth     - Disable AUTH PLAIN, LOGIN methods over un-encrypted channel
  forceauthsmtp  - Enforce Authenticated SMTP before accepting MAIL FROM
  enablecram     - Enable the pw_passwd field of indimail table to be used
                   for CRAM authenticated smtp methods
  authall        - Accept mails to both remote & local domains only after
                   AUTH SMTP or POP/IMAP before SMTP
  chkrelay       - Accept mails to remote domains only after AUTH SMTP or
                   POP/IMAP before SMTP (closed relay)
  qregex         - Use regular expressions for matching entries in
                   badmailfrom, badrcptto, blackholedsenders, spamignore
                   relaymailfrom, authdomains, chkrcptdomains,
                   chksenderdomains
  q              - Full path to an external virus scanner (like clamdscan)
  v              - Enable Virus/Attachment scanner. v can have following values
                   1 - Internal Scanner
                   2 - Internal + External + Attachment Scan
                   3 - Internal + Attachment Scan
                   4 - External + Attachment Scan
                   5 - External Scanner
                   6 - Attachment Scan
  content-filter - Enable content filtering against regex in bodycheck control
                   file or the filename given as argument to content-filter
  c              - Optional filename passed to content-filter argument
  spamfilter     - Spamfilter program along with arguments
  logfifo        - Capture additional status message from spamfilter in
                   qmail-logfifo logfile via logfifo
  r              - Mails get rejected as spam if exit status of spamfilter
                   equals the value of spamexitcode. r can have following values
                   0 - Do not reject SPAM mails
                   1 - Bounce SPAM Mails.
                   2 - Blackhole SPAM Mails
  e              - Exit value of spamfilter that should be treated as spam
  localfilter    - enable filtering of mails to qmail-local
  remotefilter   - enable filtering of mails to qmail-remote
  routes         - Perform SMTPROUTE / QMTPROUTE / static routing
  cname-lookup   - Perform CNAME lookup for recipient host in qmail-remote
  setuser-priv   - Set supplementary groups when run qmail-local
  env_list       - List of env variables to preserve when --sanitize-env
                   is given
  masquerade     - Allow user to change Mail From when using authenticated smtp
  rbl            - Deploy RBL lookups
  skipsend       - Skip creation of send script
  fsync          - Sync files and directories when writing files
  syncdir        - Use BSD style sync semantics for flushing directories
  dnscheck       - Check if mails can be sent to the sender domain
                   (domain should have a proper MX record)
  helofqdn       - Enforce . in the argument to HELO/EHLO
  helocheck      - Check validity of HELO domain
                   (domain should have a proper MX record)
  b              - Authenticated SMTP method to use by qmail-remote (plain, login, cram-md5)
  paranoid       - Paranoid hostaccess check
  dmasquerade    - Allow domains to be masqueraded from known IPs
  default-domain - default domain name for which this host will handle mails
  mysqlhost      - MySQL Host having authentication tables
  mysqluser      - Username for connecting to mysql
  mysqlpass      - Passwd for connecting to mysql
  odmr           - Setup SMTP for ODMR (On Demand Mail Relay)
  private_key    - Path to Private key created by dknewkey
  starttls       - Advertise STARTTLS capability
  forcetls       - Force client to issue STARTTLS
  ssl            - Use SSL encrypted communication
  persistdb      - Enables qmail-lspawn to have persistent MySQL connection and set
                   PWSTRUCT environment variable. This eliminates the need for
                   vdelivermail to make a MySQL connection and hence improves
                   delivery times.
  password-cache - Enable caching of passwords (auth smtp)
  query-cache    - Enable caching of queries   (auth smtp)
  shared-objects - Enabled tcpserver plugins, 0 - disabled, 1 - enabled
  use-dlmopen    - Use dlmopen() instead of dlopen() to load shared objects
  smtp-plugin    - Enable SMTP Plugin support
  setgroups      - Add addition supplementary groups
  utf8           - Enable Email Address Internationalization Support (SMTPUTF8)
  hide-host      - Skip host names and IP addresses in received headers
  barelf         - Convert bare LF into CRLF

--delivery=ident --qbase=queue_path --qcount=N --qstart=I
  --servicedir=service_path
  [--cntrldir=cntrl_path]
  [--no-multi]
  [--qtype=static|dynamic|compat]
  [--qmailqueue=qmailq]
  [--mysqlhost=mysqlhost --mysqluser=mysqluser --mysqlpass=mysqlpass]
  [--persistdb]
  [--fsync --syncdir]
  [--qregex]
  [--dmemory=c]
  [--msgqsize=Q]
  [--overquota_mailsize=o]
  [--domainlimits]
  [--routes=smtp|qmtp|static]
  [--cname-lookup]
  [--setuser-priv]
  [--sanitize-env=env_list]
  [--min-free=M --deliverylimit-count=D --deliverylimit-size=S]
  [--logfilter=logfifo]
  [--localfilter --remotefilter]
  [--dkverify=dkim|none]
  [--dksign=dkim|none --private_key=private_key]
  [--remote-authsmtp=b]
  [--ssl]
  [--setgroups]
  [--utf8]
  [--hide-host]
  --default-domain=domain

 Installs a new queue with a delivery daemon
  N              - No of Queues
  I              - Numeral Prefix of first queue (i.e 1 for @qmaildir@/queue/queue1)
  M              - Minimum Disk Space to maintain in queue after which
                   Temporary error will be returned
  c              - Max Memory that qscheduler can allocate
  Q              - Max number of bytes in POSIX message queues
  D              - Max deliveries allowed per day for a user
  S              - Max sum total size of mails allowed per day for a user
  o              - Max size of mails to accept after user is overquota
  no-multi       - Do not set QMAILQUEUE to qmail-multi
  qtype          - Set multi-queue as static, dynamic or dynamic in compat mode
  qmailq         - Path to any qmail-queue compatabile binary
  service_path   - Path where supervise service will be installed
  queue_path     - Path where the queues are installed. If this is different from
                   @qmaildir@ appropriate links will be created in
                   @qmaildir@
  cntrl_path     - Path where Qmail control files are stored
  domainlimits   - Apply Domain Limits configured by vmoddomlimits(1)
  qregex         - Use regular expressions for matching entries in
                   badmailfrom, badrcptto, blackholedsenders, spamignore
                   relaymailfrom, authdomains, chkrcptdomains,
                   chksenderdomains
  spamfilter     - Spamfilter program along with arguments
  logfifo        - Capture additional status message from spamfilter in
                   qmail-logfifo logfile via logfifo
  r              - Mails get rejected as spam if exit status of spamfilter
                   equals the value of spamexitcode. r can have following values
                   0 - Do not reject SPAM mails
                   1 - Bounce SPAM Mails.
                   2 - Blackhole SPAM Mails
  e              - Exit value of spamfilter that should be treated as spam
  localfilter    - enable filtering of mails to qmail-local
  remotefilter   - enable filtering of mails to qmail-remote
  routes         - Perform SMTPROUTE / QMTPROUTE / static routing
  cname-lookup   - Perform CNAME lookup for recipient host in qmail-remote
  setuser-priv   - Set supplementary groups when run qmail-local
  env_list       - List of env variables to preserve when --sanitize-env
                   is given
  fsync          - Sync files and directories when writing files
  syncdir        - Use BSD style sync semantics for flushing directories
  paranoid       - Paranoid hostaccess check
  dmasquerade    - Allow domains to be masqueraded from known IPs
  default-domain - default domain name for which this host will handle mails
  mysqlhost      - MySQL Host having authentication tables
  mysqluser      - Username for connecting to mysql
  mysqlpass      - Passwd for connecting to mysql
  private_key    - Path to Private key created by dknewkey
  persistdb      - Enables qmail-lspawn to have persisten MySQL connection and set
                   PWSTRUCT environment variable. This eliminates the need for
                   vdelivermail to make a MySQL connection and hence improves
                   delivery times.
  b              - Authenticated SMTP method to use by qmail-remote (plain, login, cram-md5)
  ssl            - Use SSL encrypted communication in qmail-remote
  setgroups      - Add addition supplementary groups
  utf8           - Enable Email Address Internationalization Support (SMTPUTF8)
  hide-host      - Skip host names and IP addresses in received headers

--slowq --qbase=queue_path
  --servicedir=service_path
  [--cntrldir=cntrl_path]
  [--mysqlhost=mysqlhost --mysqluser=mysqluser --mysqlpass=mysqlpass]
  [--persistdb]
  [--fsync --syncdir]
  [--qregex]
  [--dmemory=c]
  [--overquota_mailsize=o]
  [--domainlimits]
  [--routes=smtp|qmtp|static]
  [--cname-lookup]
  [--setuser-priv]
  [--sanitize-env=env_list]
  [--min-free=M --deliverylimit-count=D --deliverylimit-size=S]
  [--localfilter --remotefilter]
  [--dkverify=dkim|none]
  [--dksign=dkim|none --private_key=private_key]
  [--remote-authsmtp=b]
  [--ssl]
  [--setgroups]
  [--utf8]
  [--hide-host]
  --default-domain=domain

 Installs a new queue with a delivery daemon
  M              - Minimum Disk Space to maintain in queue after which
                   Temporary error will be returned
  c              - Max Memory that slowq-start can allocate
  D              - Max deliveries allowed per day for a user
  S              - Max sum total size of mails allowed per day for a user
  o              - Max size of mails to accept after user is overquota
  service_path   - Path where supervise service will be installed
  queue_path     - Path where the queues are installed. If this is different from
                   @qmaildir@ appropriate links will be created in
                   @qmaildir@
  cntrl_path     - Path where Qmail control files are stored
  domainlimits   - Apply Domain Limits configured by vmoddomlimits(1)
  qregex         - Use regular expressions for matching entries in
                   badmailfrom, badrcptto, blackholedsenders, spamignore
                   relaymailfrom, authdomains, chkrcptdomains,
                   chksenderdomains
  localfilter    - enable filtering of mails to qmail-local
  remotefilter   - enable filtering of mails to qmail-remote
  routes         - Perform SMTPROUTE / QMTPROUTE / static routing
  cname-lookup   - Perform CNAME lookup for recipient host in qmail-remote
  setuser-priv   - Set supplementary groups when run qmail-local
  env_list       - List of env variables to preserve when --sanitize-env
                   is given
  fsync          - Sync files and directories when writing files
  syncdir        - Use BSD style sync semantics for flushing directories
  paranoid       - Paranoid hostaccess check
  dmasquerade    - Allow domains to be masqueraded from known IPs
  default-domain - default domain name for which this host will handle mails
  mysqlhost      - MySQL Host having authentication tables
  mysqluser      - Username for connecting to mysql
  mysqlpass      - Passwd for connecting to mysql
  private_key    - Path to Private key created by dknewkey
  persistdb      - Enables qmail-lspawn to have persisten MySQL connection and set
                   PWSTRUCT environment variable. This eliminates the need for
                   vdelivermail to make a MySQL connection and hence improves
                   delivery times.
  b              - Authenticated SMTP method to use by qmail-remote (plain, login, cram-md5)
  ssl            - Use SSL encrypted communication in qmail-remote
  setgroups      - Add addition supplementary groups
  utf8           - Enable Email Address Internationalization Support (SMTPUTF8)
  hide-host      - Skip host names and IP addresses in received headers

--queueParam=dir --qbase=queue_path --qcount=N --qstart=I
  [--cntrldir=cntrl_path]
  [--min-free=M]
  [--no-multi]
  [--qmailqueue=qmailq]
  [--fsync --syncdir]
  [--qregex]
  [--content-filter=c]
  [--virus-filter=v]
  [--qhpsi=q]
  [--spamfilter=spamfilter_args]
  [--logfilter=logfifo]
  [--rejectspam=r --spamexitcode=e --localfilter --remotefilter]
  [--dkverify=dkim|none]
  [--dksign=dkim|none --private_key=private_key]
  [--hide-host]
  [--death=death]

 Installs a new queue without a SMTP Listener
  dir            - dir where to install queuedef environment variable directory
  queue_path     - Path where the queues are installed. If this is different from
                   @qmaildir@ appropriate links will be created in
                   @qmaildir@
  N              - No of Queues
  I              - Numeral Prefix of first queue (i.e 1 for @qmaildir@/queue/queue1)
  cntrl_path     - Path where Qmail control files are stored
  M              - Minimum Disk Space to maintain in queue after which
                   Temporary error will be returned
  no-multi       - Do not set QMAILQUEUE to qmail-multi
  qmailq         - Path to any qmail-queue compatabile binary
  qregex         - Use regular expressions for matching entries in
                   badmailfrom, badrcptto, blackholedsenders, spamignore
                   relaymailfrom, authdomains, chkrcptdomains,
                   chksenderdomains
  q              - Full path to an external virus scanner (like clamdscan)
  v              - Enable Virus/Attachment scanner. v can have following values
                   1 - Internal Scanner
                   2 - Internal + External + Attachment Scan
                   3 - Internal + Attachment Scan
                   4 - External + Attachment Scan
                   5 - External Scanner
                   6 - Attachment Scan
  content-filter - Enable content filtering against regex in bodycheck control
                   file or the filename given as argument to content-filter
  c              - Optional filename passed to content-filter argument
  spamfilter     - Spamfilter program along with arguments
  logfifo        - Capture additional status message from spamfilter in
                   qmail-logfifo logfile via logfifo
  r              - Mails get rejected as spam if exit status of spamfilter
                   equals the value of spamexitcode. r can have following values
                   0 - Do not reject SPAM mails
                   1 - Bounce SPAM Mails.
                   2 - Blackhole SPAM Mails
  e              - Exit value of spamfilter that should be treated as spam
  fsync          - Sync files and directories when writing files
  syncdir        - Use BSD style sync semantics for flushing directories
  private_key    - Path to Private key created by dknewkey
  hide-host      - Skip host names and IP addresses in received headers
  death          - Value in seconds maximum time qmail-queue will run after
                   which it will self terminate

--greylist=port --min-resend-min=m --resend-win-hr=g --timeout-days=t
  --context-file=f --save_interval=s --whitelist=w --hash-size=s
  --localip=a --use-greydaemon --servicedir=service_path

  Installs a new Greylist Daemon
  a              - IP Address to listen
  Port           - Bind on this port for qmail-greyd
  service_path   - Path where supervise service will be installed
  m              - Minimum resend time after which mail will be accepted
  g              - Window in which mail must be seen
  t              - Max period for a mail to be received after which IP will be removed
                   from whitelist
  s              - Size of hash table (0 to disable hashing)
  context_file   - File in which context information is saved for startup
  save_interval  - Time interval in which context file gets saved
  whitelist      - List of whitelisted IP
  use-greydaemon - Use John Levine's greydaemon

--tlsa=port --timeout-days=t --context-file=f --save_interval=s --whitelist=w
  --hash-size=s --localip=a --servicedir=service_path

  Installs a new qmail-daned TLSA verification Daemon
  a              - IP Address to listen
  Port           - Bind on this port for qmail-greyd
  service_path   - Path where supervise service will be installed
  t              - Max period for a mail to be received after which IP will be removed
                   from whitelist
  s              - Size of hash table (0 to disable hashing)
  context_file   - File in which context information is saved for startup
  save_interval  - Time interval in which context file gets saved
  whitelist      - List of whitelisted IP

--qmtp=Port --qbase=queue_path --qcount=N --qstart=I
  --servicedir=service_path
  [--cntrldir=cntrl_path]
  --localip=a --maxdaemons=m --maxperip=i
  [--fsync --syncdir]
  [--memory=b --min-free=M]
  [--databytes=d]
  [--qhpsi=q]
  [--spamfilter=spamfilter_args]
  [--logfilter=logfifo]
  [--rejectspam=r --spamexitcode=e]
  [--dkverify=dkim|none]
  [--dksign=dkim|none --private_key=private_key]

 Installs a new queue with a QMTP Listener
  m              - Concurrency of connections to allow
  i              - Concurrency of connections to be allowed from a
                   single ip address
  a              - IP Address to listen
  N              - No of Queues
  I              - Numeral Prefix of first queue (i.e 1 for @qmaildir@/queue/queue1)
  M              - Minimum Disk Space to maintain in queue after which
                   Temporary error will be returned
  b              - Max Memory that QMTP can allocate
  d              - Max size of email that can be submitted to SMTP or
                   injected locally
  Port           - Bind on this port for QMTP
  service_path   - Path where supervise service will be installed
  queue_path     - Path where the queues are installed. If this is different from
                   @qmaildir@ appropriate links will be created in
                   @qmaildir@
  cntrl_path     - Path where Qmail control files are stored
  fsync          - Sync files and directories when writing files
  syncdir        - Use BSD style sync semantics for flushing directories
  q              - Full path to an external virus scanner (like clamdscan)
  spamfilter     - Spamfilter program along with arguments

  logfifo        - Capture additional status message from spamfilter in
                   qmail-logfifo logfile via logfifo
  r              - Mails get rejected as spam if exit status of spamfilter
                   equals the value of spamexitcode. r can have following values
                   0 - Do not reject SPAM mails
                   1 - Bounce SPAM Mails.
                   2 - Blackhole SPAM Mails
  e              - Exit value of spamfilter that should be treated as spam
  private_key    - Path to Private key created by dknewkey

--qmqp=Port --qbase=queue_path --qcount=N --qstart=I
  --servicedir=service_path
  [--cntrldir=cntrl_path]
  --localip=a --maxdaemons=m --maxperip=i
  [--fsync --syncdir]
  [--memory=b --min-free=M]
  [--qhpsi=q]
  [--spamfilter=spamfilter_args]
  [--logfilter=logfifo]
  [--rejectspam=r --spamexitcode=e]
  [--dkverify=dkim|none]
  [--dksign=dkim|none --private_key=private_key]

 Installs a new queue with a QMQP Listener
  m              - Concurrency of connections to allow
  i              - Concurrency of connections to be allowed from a
                   single ip address
  a              - IP Address to listen
  N              - No of Queues
  I              - Numeral Prefix of first queue (i.e 1 for @qmaildir@/queue/queue1)
  M              - Minimum Disk Space to maintain in queue after which
                   Temporary error will be returned
  b              - Max Memory that QMTP can allocate
  Port           - Bind on this port for QMTP
  service_path   - Path where supervise service will be installed
  queue_path     - Path where the queues are installed. If this is different from
                   @qmaildir@ appropriate links will be created in
                   @qmaildir@
  cntrl_path     - Path where Qmail control files are stored
  fsync          - Sync files and directories when writing files
  syncdir        - Use BSD style sync semantics for flushing directories
  q              - Full path to an external virus scanner (like clamdscan)
  spamfilter     - Spamfilter program along with arguments

  logfifo        - Capture additional status message from spamfilter in
                   qmail-logfifo logfile via logfifo
  r              - Mails get rejected as spam if exit status of spamfilter
                   equals the value of spamexitcode. r can have following values
                   0 - Do not reject SPAM mails
                   1 - Bounce SPAM Mails.
                   2 - Blackhole SPAM Mails
  e              - Exit value of spamfilter that should be treated as spam
  private_key    - Path to Private key created by dknewkey

--imap=Port --servicedir=service_path
  --localip=a --maxdaemons=m --maxperip=i
  --default-domain=domain
  --certdir=certdir
  [--domainlimits --nolastauth]
  [--proxy=destport]
  [--infifo=fifo_path]
  [--legacyserver]
  [--postmaster=user[@domain]]
  [--query-cache]
  [--ssl|--tlsprog|--starttls]

  Installs a new IMAP4 Listner
  Port           - Bind on this port for IMAP
  service_path   - Path where supervise service will be installed
  certdir        - Path for openssl server certificates
  domainlimits   - Apply Domain Limits configured by vmoddomlimits(1)
  nolastauth     - Do not update lastauth
  proxy          - Install as a proxy and connect to actual imap on destport
  legacyserver   - To be set if destination IMAP server is not Indimail's IMAP server
  fifo_path      - Install fifo specified by this path
                   (e.g. /run/indimail/inlookup/infifo)
  postmaster     - name of the user who will be the contact for Certificates.
  common_name    - Common Name (CN) for server
  query-cache    - Enable caching of queries
  tlsprog        - External progam to enable TLS session
  starttls       - Advertise STARTTLS capability
  ssl            - Use SSL encrypted communication
  m              - Concurrency of connections to allow
  i              - Concurrency of connections to be allowed from a
                   single ip address
  a              - IP Address to listen
  domain         - default domain name for which this host will handle mails

--pop3=Port --servicedir=service_path
  --localip=a --maxdaemons=m --maxperip=i
  --default-domain=domain
  --certdir=certdir
  [--domainlimits --nolastauth]
  [--proxy=destport]
  [--legacyserver]
  [--postmaster=user[@domain]]
  [--common_name=CN]
  [--query-cache]
  [--ssl|--tlsprog|--starttls]

  Installs a new POP3 Listner
  Port           - Bind on this port for POP3
  service_path   - Path where supervise service will be installed
  certdir        - Path for openssl server certificates
  domainlimits   - Apply Domain Limits configured by vmoddomlimits(1)
  nolastauth     - Do not update lastauth
  proxy          - Install as a proxy
  legacyserver   - To be set if destination POP3 server is not Indimail's POP3 server
  postmaster     - name of the user who will be the contact for Certificates.
  common_name    - Common Name (CN) for server
  query-cache    - Enable caching of queries
  tlsprog        - External progam to enable TLS session
  starttls       - Advertise STARTTLS capability
  ssl            - Use SSL encrypted communication
  m              - Concurrency of connections to allow
  i              - Concurrency of connections to be allowed from a
                   single ip address
  a              - IP Address to listen
  domain         - default domain name for which this host will handle mails

--inlookup=fifo_path --threads=N
  --servicedir=service_path
  --domainlimits
  [--activeDays=A]
  [--use-btree]
  [--max-btree-count=max_nodes]
  [--routes=smtp|qmtp|static]
  [--cntrldir=cntrl_path]
  [--password-cache] [--query-cache]

  Installs a new Fifo Server
  fifo_path      - Install fifo specified by this path
                   (e.g. /run/indimail/inlookup/infifo)
  N              - No of parallel inlookup threads to spawn
  A              - Max days for which a user has been active
  service_path   - Path where supervise service will be installed
  cntrl_path     - Path where Qmail control files are stored
  domainlimits   - Apply Domain Limits configured by vmoddomlimits(1)
  use-btree      - Use Binary Tree algorithm to search for User records
  max_nodes      - Maximum number of Binary Nodes to allow
  password-cache - Enable caching of passwords
  query-cache    - Enable caching of queries
  routes         - Perform SMTPROUTE / QMTPROUTE / static routing

--pwdlookup=socket_path --threads=N --timeout=t
  [--mysqlhost=mysqlhost --mysqluser=mysqluser --mysqlpass=mysqlpass]
  [--mysqlport=port | --mysqlsocket=socket]
  --servicedir=service_path

  Installs a new Name Service Password Lookup Daemon
  socket_path    - Use socket_path as communication between
                   NSS clients and NSS Password Lookup server
  N              - No of parallel pwdlookup threads to spawn
  t              - Timeout wait for new Password Request
  mysqlhost      - MySQL Host having authentication tables
  mysqluser      - Username for connecting to mysql
  mysqlpass      - Passwd for connecting to mysql
  mysqlport      - Port for connecting to mysql
  mysqlsocket    - Unix Domain Socket for connecting to mysql
  service_path   - Path where supervise service will be installed

--indisrvr=Port --mysqlhost=mysqlhost --mysqluser=mysqluser --mysqlpass=mysqlpass
  --localip=a --maxdaemons=m --maxperip=i
  --domainlimits
  --avguserquota=quota --hardquota=quota
  --base_path=path --servicedir=service_path

  Installs a new Indi Admin Server
  Port           - TCP/IP port on which to bind
  mysqlhost      - MySQL Host having authentication tables
  mysqluser      - Username for connecting to mysql
  mysqlpass      - Passwd for connecting to mysql
  avguserquota   - Average Usage per user in Bytes
  hardqutoa      - Max Quota for a user
  base_path      - Default Filesystem Path for user mailbox creation
  m              - Concurrency of connections to allow
  i              - Concurrency of connections to be allowed from a
                   single ip address
  a              - IP Address to listen
  service_path   - Path where supervise service will be installed
  domainlimits   - Apply Domain Limits configured by vmoddomlimits(1)

--mysql=mysqlport --mysqlPrefix=mysqlPrefix --databasedir=databasedir
  --config=conf_file --servicedir=service_path
  [--mysqlsocket=socket]
  [--default-domain=domain]

  Installs a new MySQL Server
  mysqlport      - Install MySQL to listen on this port
  mysqlPrefix    - Installation Prefix for mysql
  databasedir    - Directory containing the Database and Logs
  mysqlsocket    - Unix Domain Socket for connecting to mysql
  config         - MySQL Configuration file (e.g. @sysconfdir@/indimail.cnf)
  service_path   - Path where supervise service will be installed
  default-domain - default domain name for which this host will handle mails

--fetchmail --qbase=queue_path --qcount=N --qstart=I --servicedir=service_path
  [--cntrldir=cntrl_path]
  --default-domain=domain
  [--silent]
  [--fsync --syncdir]
  [--domainlimits]
  [--memory=b --min-free=M]
  [--spamfilter=spamfilter_args]
  [--logfilter=logfifo]
  [--rejectspam=r --spamexitcode=e]
  [--qhpsi=q]
  [--dkverify=dkim|none]

  Install Fetchmail Server
  queue_path     - Path where the queues are installed. If this is different from
                   @qmaildir@ appropriate links will be created in
                   @qmaildir@
  N              - No of Queues
  I              - Numeral Prefix of first queue (i.e 1 for @qmaildir@/queue/queue1)
  M              - Minimum Disk Space to maintain in queue after which
                   Temporary error will be returned
  b              - Max Memory that Fetchmail can allocate
  service_path   - Path where supervise service will be installed
  cntrl_path     - Path where Qmail control files are stored
  fsync          - Sync files and directories when writing files
  syncdir        - Use BSD style sync semantics for flushing directories
  silent         - Display minimal output on stdout
  domainlimits   - Apply Domain Limits configured by vmoddomlimits(1)
  q              - Full path to an external virus scanner (like clamdscan)
  spamfilter     - Spamfilter program along with arguments
  logfifo        - Capture additional status message from spamfilter in
                   qmail-logfifo logfile via logfifo
  r              - Mails get rejected as spam if exit status of spamfilter
                   equals the value of spamexitcode. r can have following values
                   0 - Do not reject SPAM mails
                   1 - Bounce SPAM Mails.
                   2 - Blackhole SPAM Mails
  e              - Exit value of spamfilter that should be treated as spam
  default-domain - default domain name for which this host will handle mails
  silent         - be less verbose

--qscanq --servicedir=service_path [--scanint=n]

  service_path - Path where supervise service will be installed
  n            - Scan interval for Virus scanning

--clamd --servicedir=service_path --clamdPrefix=clamdPrefix
  [--sysconfdir=sysconfdir]

  service_path - Path where supervise service will be installed
  clamdPrefix  - Installation Prefix for clamd
  sysconfdir   - Path for config files (scan.conf)

--poppass=Port --localip=a --setpassword=cmd --maxdaemons=m --maxperip=i
  --servicedir=service_path
  [--memory=b]
  [--certfile=certificate --ssl]
  Port           - TCP/IP port on which to bind
  m              - Concurrency of connections to allow
  i              - Concurrency of connections to be allowed from a
                   single ip address
  b              - Max Memory to allocate for poppassd
  a              - IP Address to listen
  cmd            - Path of a setpassword compatible program
  certficate     - Path to openssl certificate
  ssl            - Use SSL encrypted communication
  service_path - Path where supervise service will be installed

  Installs a new IndiMail poppassd Server

--udplogger=udp_port --servicedir=service_path --localip=1 --timeout=t
  Port         - UDP/IP port on which to bind
  a            - IP Address to listen
  t            - Timeout for reads
  service_path - Path where supervise service will be installed

--fifologger=fifo_path --servicedir=service_path
  fifo_path    - Path to a writeable fifo which can be used by
                 any application to log messages to qmail-logfifo
                 service
  service_path - Path where supervise service will be installed

--svscanlog --servicedir=service_path [--initcmd=cmmd --scanint=n --resolvconf]

  service_path - Path where supervise service will be installed
  cmmd         - Program/Script to run instead of .svscan/run
  n            - Scan interval for svscan command
  --resolvconf - mount /etc/indimail/resolv.conf as /etc/resolv.conf
                 Use if you have dnscache installed in $servicedir/dnscache

--unshare
  use unshare to mount private /etc/resolv.conf for using local dns

--mrtg=htmldir --servicedir=service_path [--scanint=n]
  htmldir      - Path in /var/www/html directory
  service_path - Path where supervise service will be installed
  n            - Scan interval for mrtg command

--down
  Create any of the above supervisor services in down state

--showctl=DIR || --showctl

  Shows Qmail Control Information for Control Directory at DIR or @sysconfdir@

--dumpconfig --servicedir=service_path --cntrldir=cntrl_path

  Dumps all service configuration for Supervise Scripts, control files and software information
  service_path - Path where supervise service will be installed
  cntrl_path   - Path where Qmail control files are stored

--queuefix=queue_path

  Fix Qmail Queue where queue_path is the absolute path of a queue

--rmsvc=service_path

  Disable supervise for service
  service_path   - Full path of service
                   e.g. (@servicedir@/qmail-smtpd.25)
                   (give multiple services enclosed in double quotes)
                   "@servicedir@/qmail-smtpd.25 @servicedir@/qmail-imapd.143"

--ensvc=service_path

  Enable supervise for service
  service_path   - Full path of service
                   e.g. (@servicedir@/qmail-smtpd.25)
                   (give multiple services enclosed in double quotes)
                   "@servicedir@/qmail-smtpd.25 @servicedir@/qmail-imapd.143"

--refreshsvc=service_path
  [--run-file-only] [--force]

  Refresh run script and variables for a supervise service
  service_path   - Full path of service
                   e.g. (@servicedir@/qmail-smtpd.25)
                   (give multiple services enclosed in double quotes)
                   "@servicedir@/qmail-smtpd.25 @servicedir@/qmail-imapd.143"
                   service_path can be "all" to act on all services
  run-file-only  - Recreate run files but not variables
  force          - Recreate service even if it has norefreshsvc flag
  NOTE: if the file .norefreshsvc is present in the variable or the config
  directory, refresh is skipped

--autorefresh="0|1 service_path"

  Disable/Enable auto refresh of supervise scripts for service
  0              - Disable autorefresh (create .norefreshsvc in variables directory)
  1              - Enable  autorefresh (delete .norefreshsvc in variables directory)
  service_path   - name of service with full path
                   e.g. (@servicedir@/qmail-smtpd.25)
                   service_path can be "all" to act on all services

--enable-service name1 name2 ..

  Add a service to be started at system boot. The unit file for the service should exist
  name1 name2 .. - name of a service or list of services to be added
                   This should be a SYSV style service, systemd, event.d unit file

--disable-service name1 name2 ..

  Remove a service to be started at system boot. The unit file for the service should exist
  name1 name2 .. - name of a service or list of services to be removed
                   This should be a SYSV style service, systemd, event.d unit file

--set-variable=n --variable-value=v
  --servicedir=service_path
  --service-name=service_name
  [--force]

  Set a new variable for a supervise service
  n              - variable name
  v              - variable value
  service_path   - Path where supervise service will be installed
  service_name   - Service name
  force          - Overwrite variable if it exists

--modify-variable=n --variable-value=v
  --servicedir=service_path
  --service-name=service_name
  [--force]

  Modify an existing variable for a supervise service
  n              - variable name
  v              - variable value
  service_path   - Path where supervise service will be installed
  service_name   - Service name
  force          - Set the variable even if does not exist

--unset-variable=n
  --servicedir=service_path
  --service-name=service_name

  Unset any existing environment variable named 'n'
  n              - variable name
  service_name   - Service name

--remove-variable=n
  --servicedir=service_path
  --service-name=service_name

  Remove an existing variable for a supervise service
  n              - variable name
  service_path   - Path where supervise service will be installed
  service_name   - Service name

--restore-variables
  --servicedir=service_path
  --service-name=service_name

  Clean and Restore all variables to original state from
  servicedir/service_name/variables/.variables file
  service_path   - Path where supervise service will be installed
  service_name   - name of supervise service without path
                   e.g. (qmail-smtpd.25)

--import-variables=file
  --servicedir=service_path
  --service-name=service_name
  [--force] [--silent]

  Import new environment variables from a file having one more multiple
  key=value pairs
  file           - File from which to import key=value environment variables
  service_path   - Path where supervise service will be installed
  service_name   - name of supervise service without path
                   e.g. (qmail-smtpd.25)
  force          - Set the variable if variable already exists
  silent         - be less verbose

--export-variables=file
  --servicedir=service_path
  --service-name=service_name
  [--force]

  Export existing environment variables to a file
  file           - File in which to save key=value environment variables
  service_path   - Path where supervise service will be installed
  service_name   - name of supervise service without path
                   e.g. (qmail-smtpd.25)
  force          - overwrite service_path/service_name/variables/.variables

--save-variables
  --servicedir=service_path

  Export variables for all service in @servicedir@ and
  @qsysconfdir@/control/defaultqueue
  service_path   - Path where supervise service will be installed

--restore-all-variables
  --servicedir=service_path

  Restore all variables to orignal state for all service in
  @servicedir@ and @qsysconfdir@/control/defaultqueue
  service_path   - Path where supervise service will be installed

--print-variables
  --servicedir=service_path
  --service-name=service_name | --envdir=dir

  Print environment variables for a service
  service_path   - Path where supervise service will be installed
  service_name   - name of supervise service without path
                   e.g. (qmail-smtpd.25)
  dir            - Environment variables directory

--print-all-variables
  --servicedir=service_path

  Print environment variables all services
  service_path   - Path where supervise service will be installed

--config=mysql|ssl_rsa|mysqldb|qmail|users|rmusers|nssd
--config=snmpdconf|clamd|foxhole|bogofilter|cert|add-boot|rm-boot
--config=add-alt|remove-alt|selinux|qselinux|iselinux|inittab|recontrol
  [--postmaster=user[@domain]]
  [--common_name=CN]
  [--validity_days=days]
  [--certdir=certdir]
  [--update-certs]
  [--capath=ca_path_dir]
  [--servicedir=service_path]
  [--default-domain=domain]
  [--mysqlPrefix=mysqlPrefix]
  [--mysqlhost=mysqlhost --mysqluser=mysqluser --mysqlpass=mysqlpass]
  [--mysqlport=port | --mysqlsocket=socket]
  [--cntrldir=cntrl_path]
  [--sysconfdir=sysconfdir]
  [--databasedir=databasedir]
  [--use-grant]
  [--module=module]
  [--stdout]
  [--wipe]

  mysql          - Create @sysconfdir@/indimail.cnf
  mysqldb        - Create an initidalized MySQL db for IndiMail
  ssl_rsa        - Create SSL/TLS Certs for MariaDB
  qmail          - Create default qmail control files
  users          - Create Internal System Users used by IndiMail
  rmusers        - Remove Internal System Users used by IndiMail
  snmpdconf      - create v2 /etc/snmpd/snmpd.conf
  nssd           - create @sysconfdir@/nssd.conf
  clamd          - Create clamd configuration file
  bogofilter     - Create bogofilter.cf configuration file
  cert           - Generate SSL Certificate
  add-boot       - Add startup scripts for IndiMail to get started during boot
  rm-boot        - Remove Startup scripts to prevent IndiMail to get started after boot
  add-alt        - Install indiamil as default MTA
  remove-alt     - Remove indimail as the default MTA
  selinux        - create selinux module from @sysconfdir@/module.te
  iselinux       - Enable selinux module for indimail
  qselinux       - Enable selinux module for indimail-mta
  inittab        - Install svscan started by configuration in /etc/inittab
  recontrol      - Update control files having domain name configured with a
                   new domain
  postmaster     - name of the user who will recieve bounces on the local host.
                   This can also be a remote user i.e. &postmaster@indimail.org
  common_name    - Common Name (CN) for server
  days           - Number of days for which the Certificate should be valid
  cntrl_path     - Path where Qmail control files are stored
  sysconfdir     - Path for config files
  certdir        - Directory in which new certificates will be placed
  update-certs   - Update CERTDIR, CERTFILE and TLSCACHE for services using them
  ca_path_dir    - Directory having CA certificate
  service_path   - Path where supervise service have been installed
  default-domain - default domain name for which this host will handle mails
  mysqlPrefix    - Installation Prefix for mysql
  mysqlhost      - MySQL Host having authentication tables
  mysqluser      - Username for connecting to mysql
  mysqlpass      - Passwd for connecting to mysql
  mysqlport      - Port for connecting to mysql
  mysqlsocket    - Unix Domain Socket for connecting to mysql
  databasedir    - Directory containing the Database and Logs
  use-grant      - Use the usual create user and grant statements for creating users
                   (implies --stdout)
  stdout         - Dump on the screen. Will not create a configuration
  wipe           - will wipe out users/assign (for config=qmail)

--check-install --servicedir=service_path --qbase=queue_path --qcount=N --qstart=I
  [--skip-sendmail]

  service_path  - Path where supervise service have been installed
  queue_path    - Path where the queues are installed.
  N             - No of Queues
  I             - Numeral Prefix of first queue (i.e 1 for @qmaildir@/queue/queue1)
  skip-sendmail - Skip Checking sendmail paths in /usr/sbin and /usr/lib

--check-certs=[full path of certificate]
  Check certificate given as argument or
  Check IndiMail Certificates in @sysconfdir@/certs without any arguments

--backup=backupdir --servicedir=service_path --mysqlPrefix=mysqlPrefix

  mysqlPrefix  - Installation Prefix for mysql
  backupdir    - Directory to dump backup

--repair-tables
  Repair MySQL tables in @qmaildir@/mysqldb/data

--mysqlupgrade --mysqlPrefix=mysqlPrefix
  mysqlPrefix  - Installation Prefix for mysql

--fixsharedlibs

  fix mysql shared library in @sysconfdir@/control/libmysql

--report=all| zddist| zdeferrals| zfailures| zoverall| zrecipients| zrhosts|
  zrxdelay| zsenders| zsendmail| zsuccesses| zsuids | zsmtp | zspam
  [--postmaster=user[@domain]]
  [--attach]
  [--logdir=multilog_base_dir]

  postmaster - name of the user who will recieve the report.
  attach     - zip and send the report as an attachment

--help

  display this help and exit

--version

  output version information

Global Directory Options

--cntrldir     - sets directory for indimail/qmail control files
--indimaildir  - sets base directory for mysqldb, domains, clamd, inquery
                 directory
--qmaildir     - sets base directory for alias, autoturn, qscanq, queue
                 directory
EOF
exit $1
}

change_config()
{
if [ $# -lt 2 ] ; then
	echo "USAGE: change_config old_config_file new_config_file [overwrite]" 1>&2
	return 1
fi
conf_file=$1
temp_file=$2
if [ $# -eq 3 ] ; then
	if [ " $3" = " 0" -o " $3" = " no" -o " $3" = " NO" ] ; then
		overwrite=0
	else
		overwrite=1
	fi
else
	overwrite=0
fi
if [ -f $conf_file ] ; then
	if ! cmp -s $conf_file $temp_file ; then
		if [ $overwrite -eq 0 ] ; then
			echo "Saving $temp_file as $conf_file.rpmnew"
			/bin/mv $temp_file "$conf_file".rpmnew
		else
			echo "Updating $conf_file"
			/bin/mv $temp_file $conf_file
		fi
	else
		echo "no difference between previous and new - $conf_file not changed" 1>&2
		/bin/rm -f $temp_file
	fi
else
	echo "change_config: Creating $conf_file"
	/bin/mv $temp_file $conf_file
fi
}

check_installation()
{
# check home
# check alias autoturn bin boot control doc etc lib libexec man \
#  modules plugins qscanq sbin share users
# check queues
# check users alias qmaild qmaill qmailp qmailq qmailr qmails mysql indimail qscand
# check groups qmail nofiles indimail qscand
# check IndiMail Installation Permission and Default MySQL Tables"
#       run instcheck
#       run install_tables
# check defaultdelivery"
# check service directory"
# check indimail startup (svscan) "
# check configured services"
# check mandatory services"
# check installed services"
# check Log Directories"
# check Firewall Rules"
# check Alias User"
# check Sendmail Status"
#
if [ $# -lt 5 ] ; then
  echo "USAGE: check_installation qbase queue_count first_queue_no supervise_dir verb" 1>&2
  return 1
fi
QUEUE_BASE=$1
QUEUE_COUNT=$2
QUEUE_START=$3
SERVICEDIR=$4
OK=1

if [ "$5" = "-v" ]; then
VERB=y
else
VERB=n
fi
echo "Checking IndiMail Home"
if [ ! -d "$DESTDIR"$QmailHOME ]; then
  echo "! Couldn't find qmail's home directory, "$DESTDIR"$QmailHOME!" 1>&2
else
  if [ "$VERB" = y ]; then
    echo ""$DESTDIR"$QmailHOME exists"
  fi
  echo "Checking Critical IndiMail directories"
  if [ -f "$DESTDIR"$QmailBinPrefix/bin/vadddomain ] ; then
    indimail_installed=1
    LIST1="alias autoturn bin qscanq sbin" # qmaildir/indimaildir
    LIST2="boot doc" # shareddir
    LIST3="control users" #sysoncdir
    LIST4="plugins modules" # /usr/lib/indimail
    #LIST="alias autoturn bin boot control doc etc lib libexec man modules plugins qscanq sbin share users"
  else
    indimail_installed=0
    LIST1="alias bin qscanq sbin"
    LIST2="boot doc"
    LIST3="control users"
    LIST4="plugins"
    #LIST="alias bin boot control doc etc lib man plugins qscanq sbin share users"
  fi
  for i in $LIST1; do
    echo $i
    if [ ! -d "$DESTDIR"$QmailHOME/$i -a ! -L "$DESTDIR"$QmailHOME/$i ]; then
      echo "! Couldn't find "$DESTDIR"$QmailHOME/$i!" 1>&2
      OK=0
    elif [ "$VERB" = y ]; then
      echo ""$DESTDIR"$QmailHOME/$i exists"
    fi
  done
  for i in $LIST2; do
    echo $i
    if [ ! -d "$DESTDIR"$shareddir/$i -a ! -L "$DESTDIR"$shareddir/$i ]; then
      echo "! Couldn't find "$DESTDIR"$shareddir/$i!" 1>&2
      OK=0
    elif [ "$VERB" = y ]; then
      echo ""$DESTDIR"$shareddir/$i exists"
    fi
  done
  for i in $LIST3; do
    echo $i
    if [ ! -d "$DESTDIR"$sysconfdir/$i -a ! -L "$DESTDIR"$sysconfdir/$i ]; then
      echo "! Couldn't find "$DESTDIR"$sysconfdir/$i!" 1>&2
      OK=0
    elif [ "$VERB" = y ]; then
      echo ""$DESTDIR"$sysconfdir/$i exists"
    fi
  done
  for i in $LIST4; do
    echo $i
    if [ ! -d "$DESTDIR"/usr/lib/indimail/$i -a ! -L "$DESTDIR"/usr/lib/indimail/$i ]; then
      echo "! Couldn't find "$DESTDIR"/usr/lib/indimail/$i!" 1>&2
      OK=0
    elif [ "$VERB" = y ]; then
      echo ""$DESTDIR"/usr/lib/indimail/$i exists"
    fi
  done
  echo $libexecdir
  if [ ! -d "$DESTDIR"$libexecdir ] ; then
      echo "! Couldn't find "$DESTDIR"$libexecdir!" 1>&2
      OK=0
  elif [ "$VERB" = y ]; then
    echo ""$DESTDIR"$libexecdir exists"
  fi
  echo "Checking IndiMail queues"
  QUEUE_NO=$QUEUE_START
  COUNT=1
  while true
  do
    "$DESTDIR"$QmailBinPrefix/bin/queue-fix -N "$DESTDIR"$QUEUE_BASE/queue"$QUEUE_NO" > /dev/null 2>&1
    status=$?
    if [ $status -ne 0 ] ; then
      echo "status $status ! queue"$QUEUE_NO" should be fixed. Run queue-fix -v "$DESTDIR"$QUEUE_BASE/queue"$QUEUE_NO" !" 1>&2
      OK=0
    elif [ "$VERB" = y ]; then
      echo "queue "$DESTDIR"$QUEUE_BASE/queue"$QUEUE_NO" looks OK"
    fi
    if [ $COUNT -eq $QUEUE_COUNT ] ; then
      break
    fi
    COUNT=`expr $COUNT + 1`
    QUEUE_NO=`expr $QUEUE_NO + 1`
  done
  for i in nqueue slowq
  do
    if [ "$i" = "slowq" ] ; then
      "$DESTDIR"$QmailBinPrefix/bin/queue-fix -N -r "$DESTDIR"$QUEUE_BASE/$i > /dev/null 2>&1
    else
      "$DESTDIR"$QmailBinPrefix/bin/queue-fix -N "$DESTDIR"$QUEUE_BASE/$i > /dev/null 2>&1
    fi
status=$?
    if [ $status -ne 0 ] ; then
      echo "status $status ! $i should be fixed. Run queue-fix -v "$DESTDIR"$QUEUE_BASE/$i !" 1>&2
      OK=0
    elif [ "$VERB" = y ]; then
      echo "queue "$DESTDIR"$QUEUE_BASE/$i looks OK"
    fi
  done
fi

echo "Checking IndiMail users"
for i in alias qmaild qmaill qmailp qmailq qmailr qmails mysql indimail qscand; do
  echo $i
  case "$host" in
  *-*-darwin*)
    dscl . -list /Users/$i > /dev/null 2>&1
    if [ $? -ne 0 ]; then
      echo "! Couldn't find $i user in passwd database" 1>&2
      OK=0
    elif [ "$VERB" = y ]; then
      echo "$i user exists"
    fi
  ;;
  *)
    grep "^$i:" /etc/passwd >/dev/null
    if [ $? -ne 0 ]; then
      echo "! Couldn't find $i user in /etc/passwd" 1>&2
      OK=0
    elif [ "$VERB" = y ]; then
      echo "$i user exists"
    fi
    ;;
  esac
done

echo "Checking IndiMail groups"
for i in qmail nofiles indimail qscand; do
  echo $i
  case "$host" in
  *-*-darwin*)
    dscl . -list /Groups/$i > /dev/null 2>&1
    if [ $? -ne 0 ]; then
      echo "! Couldn't find $i group in group database" 1>&2
      OK=0
    elif [ "$VERB" = y ]; then
      echo "$i group exists"
    fi
  ;;
  *)
    grep "^$i:" /etc/group >/dev/null
    if [ $? -ne 0 ]; then
      echo "! Couldn't find $i group in /etc/group" 1>&2
      OK=0
    elif [ "$VERB" = y ]; then
      echo "$i group exists"
    fi
  ;;
  esac
done

echo "Checking IndiMail Installation Permission"
for i in indimail-mta daemontools ucspi-tcp
do
  if [ ! -x "$DESTDIR"$libexecdir/instcheck.$i ]; then
    echo "! Couldn't find "$DESTDIR"$libexecdir/instcheck.$i" 1>&2
    OK=0
  else
    if [ "$VERB" = y ]; then
      echo "instcheck.$i from $i is installed. Executing..."
    fi
    "$DESTDIR"$libexecdir/instcheck.$i "$DESTDIR"$QmailHOME
    if [ $? -ne 0 ]; then
      echo "instcheck.$i returned non-zero exit status. Check Installation"
      OK=0
    fi
  fi
done
if [ $indimail_installed -eq 1 ] ; then
  echo "Checking IndiMail Default MySQL Tables"
  if [ ! -x "$DESTDIR"$QmailBinPrefix/sbin/install_tables ]; then
    echo "! Couldn't find "$DESTDIR"$QmailBinPrefix/sbin/install_tables" 1>&2
    OK=0
  else
    if [ "$VERB" = y ]; then
      echo "install_tables from indimail is installed. Executing..."
    fi
    "$DESTDIR"$QmailBinPrefix/sbin/install_tables > /dev/null
    if [ $? -ne 0 ]; then
      echo "install_tables returned non-zero exit status. Check Installation" 1>&2
      OK=0
    fi
  fi
fi

echo "Checking defaultdelivery"
if [ ! -f "$DESTDIR"$CONTROLDIR/defaultdelivery ]; then
  echo "! Couldn't find "$DESTDIR"$CONTROLDIR/defaultdelivery" 1>&2
  OK=0
elif [ ! -s "$DESTDIR"$CONTROLDIR/defaultdelivery ]; then
  echo "! "$DESTDIR"$CONTROLDIR/defaultdelivery is empty" 1>&2
  OK=0
elif [ "$VERB" = y ]; then
  echo ""$DESTDIR"$CONTROLDIR/defaultdelivery looks OK"
fi

echo "Checking Service Directory"
if [ ! -d $servicedir ]; then
  echo "! $servicedir directory is missing" 1>&2
  OK=0
elif [ "$VERB" = y ]; then
  echo "$servicedir directory exists"
fi

PS="ps ax"
echo "Checking svscan Startup"
if [ -f /etc/init/svscan.conf -a -f /sbin/initctl ] ; then
  /sbin/initctl status svscan
  if [ $? -ne 0 ]; then
    echo "! svscan service not configured in upstart" 1>&2
    OK=0
  else
    echo "svscan service is configured to run via upstart"
  fi
elif [ -d /etc/event.d -a -f /sbin/initctl ] ; then
  /sbin/initctl list svscan
  if [ $? -ne 0 ]; then
    echo "! svscan service not configured in upstart" 1>&2
    OK=0
  else
    echo "svscan service is configured to run via upstart"
  fi
elif [ -f /lib/systemd/system/svscan.service -a -f /bin/systemctl ] ; then
  /bin/systemctl is-enabled svscan > /dev/null
  if [ $? -ne 0 ] ; then
    echo "! svscan service not configured in systemd" 1>&2
    OK=0
  else
    echo "svscan service is configured to run via systemd"
  fi
elif [ -f /usr/lib/systemd/system/svscan.service -a -f /bin/systemctl ] ; then
  /bin/systemctl is-enabled svscan > /dev/null
  if [ $? -ne 0 ] ; then
    echo "! svscan service not configured in systemd" 1>&2
    OK=0
  else
    echo "svscan service is configured to run via systemd"
  fi
elif [ -d /Library/LaunchDaemons ] ; then
  /bin/launchctl list org.indimail.svscan
  if [ $? -ne 0 ]; then
    echo "! svscan service not configured in launchd" 1>&2
    OK=0
  else
    echo "svscan service is configured to run via launchd" 1>&2
  fi
else
  echo "Checking /etc/inittab"
  if [ -f /etc/inittab ]; then
	grep ".*:.*:respawn:.*svscanboot" /etc/inittab >/dev/null
    if [ $? -ne 0 ]; then
      echo "! Couldn't find svscanboot entry in inittab" 1>&2
      OK=0
    else
	  grep ".*:.*:respawn:.*svscanboot" /etc/inittab |grep respawn >/dev/null
      if [ $? -ne 0 ]; then
        echo "! svscanboot entry in inittab is not set to respawn" 1>&2
        OK=0
      elif [ "$VERB" = y ]; then
        echo "svscan is configured to run via /etc/inittab"
      fi
    fi
  else
    if [ -x /sbin/chkconfig -a -f /etc/init.d/svscan ] ; then
      /sbin/chkconfig --list svscan|grep on > /dev/null
      if [ $? -ne 0 ] ; then
      echo "! Couldn't find indimail referenced in any runlevel" 1>&2
        OK=0
      elif [ "$VERB" = y ]; then
        /sbin/chkconfig --list svscan|grep on
        echo "indimail service is configured to run through rc scripts"
      fi
    elif [ -f /etc/init.d/svscan ] ; then
      /bin/ls -l /etc/rc?.d/*/svscan > /dev/null 2>&1
      if [ $? -ne 0 ] ; then
        echo "! Couldn't find indimail referenced in any runlevel" 1>&2
        OK=0
      elif [ "$VERB" = y ]; then
        /sbin/chkconfig --list svscan|grep on
        echo "indimail service is configured to run through rc scripts"
      fi
    fi
    grep "qmailctl start" /etc/rc.local >/dev/null
    if [ $? -ne 0 ]; then
      echo "! Couldn't find 'qmailctl start' in /etc/rc.local" 1>&2
      grep "svscan @servicedir@" /etc/rc.local >/dev/null
      if [ $? -ne 0 ]; then
        echo "! Couldn't find 'svscan @servicedir@' in /etc/rc.local" 1>&2
        OK=0
      elif [ "$VERB" = y ]; then
        echo "svscan @servicedir@ is configured to run via /etc/rc.local"
      fi
    elif [ "$VERB" = y ]; then
      echo "'qmailctl start' is in /etc/rc.local"
    fi
    PS="ps -waux"
  fi
  echo "Checking svscan"
  SVRUN=`$PS | grep "svscan $servicedir" | grep -v grep`
  if [ -z "$SVRUN" ]; then
    echo "'! svscan $servicedir' doesn't seem to be running. ignoring..." 1>&2
  elif [ "$VERB" = y ]; then
    echo "svscan $servicedir is running"
  fi
fi

echo "Checking mandatory services"
if [ -x "$DESTDIR"$QmailBinPrefix/sbin/nssd ] ; then
LIST="pwdlookup"
fi
LIST="$LIST qmail-send.25 qmail-smtpd.25 qmail-smtpd.366 qmail-smtpd.465 qmail-smtpd.587"
LIST="$LIST qmail-daned.1998 qmail-logfifo udplogger.3000 greylist.1999"
if [ -f /usr/sbin/clamd -a -f /usr/bin/clamdscan ] ; then
LIST="$LIST qscanq"
fi
if [ $indimail_installed -eq 1 ] ; then
LIST="$LIST inlookup.infifo indisrvr.4000 mysql.3306 qmail-poppass.106 mrtg"
LIST="$LIST qmail-imapd.143 qmail-pop3d.110 qmail-imapd-ssl.993 qmail-pop3d-ssl.995"
LIST="$LIST proxy-imapd.4143 proxy-imapd-ssl.9143 proxy-pop3d.4110 proxy-pop3d-ssl.9110"
fi
if [ -x "$DESTDIR"$QmailBinPrefix/sbin/clamd -o -x "$DESTDIR"$QmailBinPrefix/bin/freshclam ] ; then
LIST="$LIST clamd freshclam"
elif [ -x /usr/sbin/clamd -o -x /usr/bin/freshclam ] ; then
LIST="$LIST clamd freshclam"
fi
for i in $LIST ; do
  echo $i
  if [ ! -d $servicedir/$i ]; then
    echo "! $servicedir/$i directory is missing" 1>&2
    OK=0
  fi
done

echo "Checking installed services"
SVCDIRS="`ls -A $servicedir`"
for i in $SVCDIRS; do
  if [ " $i" = " .svscan" -o " $i" = " .svlock" ] ; then
    continue
  fi
  echo $i
  interp=$(head -1 $servicedir/$i/run|@prefix@/bin/qcat -vet)
  if [ ! -f $servicedir/$i/run ]; then
    echo "! $servicedir/$i/run file is missing" 1>&2
    OK=0
  elif [ "$interp" != "#!/bin/sh$" -a "$interp" != "#!/bin/bash$" \
	  -a "$interp" != "#!/usr/bin/sh$" -a "$interp" != "#!/usr/bin/bash$" \
	  -a "$interp" != "#!/usr/local/bin/bash$" ] ; then
    echo "! $servicedir/$i/run has bad interpreter [$interp]" 1>&2
    OK=0
  elif [ ! -x $servicedir/$i/run ]; then
    echo "! $servicedir/$i/run file is not executable" 1>&2
    OK=0
  elif [ "$VERB" = y ]; then
    echo "$servicedir/$i/run looks OK"
  fi
  if [ ! -d $servicedir/$i/log ] ; then
    echo "No log for $servicedir/$i" 1>&2
    continue
  fi
  if [ ! -f $servicedir/$i/log/run ]; then
    echo "! $servicedir/$i/log/run file is missing" 1>&2
    OK=0
  elif [ "$interp" != "#!/bin/sh$" -a "$interp" != "#!/bin/bash$" \
	  -a "$interp" != "#!/usr/bin/sh$" -a "$interp" != "#!/usr/bin/bash$" \
	  -a "$interp" != "#!/usr/local/bin/bash$" ] ; then
    echo "! $servicedir/$i/log/run has bad interpreter [$interp]" 1>&2
    OK=0
  elif [ ! -x $servicedir/$i/log/run ]; then
    echo "! $servicedir/$i/log/run file is not executable" 1>&2
    OK=0
  elif [ "$VERB" = y ]; then
    echo "$servicedir/$i/log/run looks OK"
  fi
done

echo "Checking Log Directories"
if [ -h $LOGDIR ] ; then
  logdir=`ls -ld $LOGDIR |awk '{print $11}'`
else
  logdir=$LOGDIR
fi
LOGDIRS="`ls -d $logdir/* 2>/dev/null`"
for i in $LOGDIRS; do
  if [ $i = $logdir"/svctool.log" -o $i = $logdir"/services.log" ] ; then
    continue
  fi
  echo $i
  perm="`ls -ld $i|awk '{print $1}'|cut -d. -f1`"
  if [ ! -d $i ]; then
    echo "! $i looks strange" 1>&2
    echo "...try: rm -f $i" 1>&2
    OK=0
  elif [ "`ls -ld $i|awk '{print $3}'`" != "qmaill" ]; then
    echo "! $i has wrong owner, should be qmaill" 1>&2
    echo "...try: $chown qmaill $i" 1>&2
    OK=0
  elif [ "$perm" != "drwxr-xr-x" -a "$perm" != "drwxr-sr-x" ]; then
    echo "! $i has wrong mode, should be 755" 1>&2
    echo "...try: chmod 755 $i" 1>&2
    OK=0
  elif [ "$VERB" = y ]; then
    echo "$i looks OK"
  fi
done

echo "Checking Firewall Rules"
if [ ! -f "$DESTDIR"$sysconfdir/tcp/tcp.smtp ]; then
  echo "! "$DESTDIR"$sysconfdir/tcp/tcp.smtp is missing" 1>&2
  echo "...try: echo '127.:allow,RELAYCLIENT=\"\"' >> "$DESTDIR"$sysconfdir/tcp/tcp.smtp" 1>&2
  OK=0
elif [ "$VERB" = y ]; then
  echo ""$DESTDIR"$sysconfdir/tcp/tcp.smtp exists"
fi

if [ ! -f "$DESTDIR"$sysconfdir/tcp/tcp.smtp.cdb ]; then
  echo "! "$DESTDIR"$sysconfdir/tcp/tcp.smtp.cdb is missing" 1>&2
  echo "...try: "$DESTDIR"$QmailBinPrefix/bin/qmailctl cdb" 1>&2
  OK=0
elif [ "$VERB" = y ]; then
  echo ""$DESTDIR"$sysconfdir/tcp/tcp.smtp.cdb exists"
fi

echo "Checking Alias User"
case $host in
  *-*-darwin*)
    AHOME=`dscl . -read /Users/alias|grep NFSHomeDi|awk '{print $2}'`
  ;;
  *)
    AHOME=`grep "^alias:" /etc/passwd | awk -F: '{print $6}'`
  ;;
esac
if [ -z "$AHOME" ]; then
  echo "! Couldn't find user alias's home directory" 1>&2
  OK=0
else
  for i in root postmaster mailer-daemon; do
  echo $i
    if [ ! -f "$DESTDIR"$AHOME/.qmail-$i ]; then
      echo "! Alias for $i is missing" 1>&2
      echo "...try: echo me > "$DESTDIR"$AHOME/.qmail-$i" 1>&2
      OK=0
    elif [ "$VERB" = y ]; then
      echo "$i alias exists"
    fi
  done
fi

echo "Checking Sendmail Status"
if netstat -an | grep "^tcp"| grep ":25" | grep -i listen >/dev/null; then
  if $PS | grep sendmail | grep -v grep >/dev/null; then
    echo "! Sendmail is still running" 1>&2
    echo "...try: $RCDIR/init.d/sendmail stop" 1>&2
    OK=0
  elif "$DESTDIR"$QmailBinPrefix/bin/svok $servicedir/qmail-smtpd.25; then
    if [ "$VERB" = y ]; then
      echo "$servicedir/qmail-smtpd.25 is running"
    fi
  else
    echo "! Something is listening on port 25 (not tcpserver/qmail-smtpd)" 1>&2
    echo "...try: disabling current MTA" 1>&2
    OK=0
  fi
fi

if [ $skip_sendmail_check -eq 1 ] ; then
for i in /usr/lib/sendmail /usr/sbin/sendmail; do
  if [ -f $i -a ! -L $i ]; then
    echo "! $i is a file, should be a link" 1>&2
    echo "...try: uninstalling current MTA or: mv $i $i.old; ln -s $QmailBinPrefix/bin/sendmail $i" 1>&2
    OK=0
  elif [ ! -f $i ];then
    echo "! $i is missing" 1>&2
    echo "...try: ln -s $QmailBinPrefix/bin/sendmail $i" 1>&2
    OK=0
  elif [ "$VERB" = y ]; then
    echo "$i exists"
  fi
done
fi

if [ $OK -eq 1 ]; then
  echo "Congratulations, your IndiMail installation in "$DESTDIR"$QmailHOME looks good!"
  return 0
else
  echo "! Potential problems were found with your IndiMail installation in "$DESTDIR"$QmailHOME" 1>&2
  return 1
fi
}

# This creates
# sysconfdir/tcp/tcp.*
# sysconfdir/tcp/tcp.*.cdb
# sysconfdir/host.mysql
# qmailhome/domains
# qmailhome/alias
default_indimail_control()
{
if [ $nooverwrite -eq 1 ] ; then
	return 0
fi
if [ -z "$CONTROLDIR" ] ; then
	CONTROLDIR=$sysconfdir/control
fi
if [ ! -d "$DESTDIR"$CONTROLDIR ] ; then
	/bin/mkdir -p "$DESTDIR"$CONTROLDIR
	if [ $? -ne 0 ] ; then
		return 1
	fi
fi
(
echo "$RCSID"
echo "# generated on $host at `date`"
echo "# by the below command"
echo "$prog_args"
echo ""
) > "$DESTDIR"$CONTROLDIR/.indimail_control
if [ ! -d "$DESTDIR"$sysconfdir/tcp ] ; then
	/bin/mkdir -p "$DESTDIR"$sysconfdir/tcp
	if [ $? -ne 0 ] ; then
		return 1
	fi
fi
echo "Creating Default IndiMail Control Files"
if [ ! -f "$DESTDIR"$sysconfdir/tcp/tcp.imap ] ; then
	(
	echo "127.:allow,IMAPCLIENT=\"\""
	echo "::1:allow,IMAPCLIENT=\"\""
	) > "$DESTDIR"$sysconfdir/tcp/tcp.imap
fi
if [ ! -f "$DESTDIR"$sysconfdir/tcp/tcp.pop3 ] ; then
	(
	echo "127.:allow,POP3CLIENT=\"\""
	echo "::1:allow,POP3CLIENT=\"\""
	) > "$DESTDIR"$sysconfdir/tcp/tcp.pop3
fi
if [ ! -f "$DESTDIR"$sysconfdir/tcp/tcp.poppass ] ; then
	(
	echo "127.:allow"
	echo "::1:allow"
	) > "$DESTDIR"$sysconfdir/tcp/tcp.poppass
fi
if [ ! -f "$DESTDIR"$CONTROLDIR/host.mysql ] ; then
	echo "host.mysql"
	if [ -n "$mysqlSocket" -o -n "$mysql_port" ] ; then
		if [ -z "$mysql_host" ]; then
			mysql_host=$MYSQL_HOST
		fi
		if [ -z "$mysql_user" ]; then
			mysql_user=$MYSQL_USER
		fi
		if [ -z "$mysql_pass" ]; then
			mysql_pass=$MYSQL_PASS
		fi
		if [ -n "$mysqlSocket" ] ; then
			echo "$mysql_host:$mysql_user:$mysql_pass:$mysqlSocket" > \
				"$DESTDIR"$CONTROLDIR/host.mysql
		else
			echo "$mysql_host:$mysql_user:$mysql_pass:$mysql_port" > \
				"$DESTDIR"$CONTROLDIR/host.mysql
		fi
	elif [ -n "$mysqlSocket" ] ;then
		echo "$mysql_host:$mysql_user:$mysql_pass:$mysqlSocket" > \
			"$DESTDIR"$CONTROLDIR/host.mysql
	else
		if [ -n "$MYSQL_SOCKET" ] ; then
			mysqlSocket=$MYSQL_SOCKET
		elif [ -d /run ] ; then
			mysqlSocket=/run/mysqld/mysqld.sock
		elif [ -d /var/run ] ; then
			mysqlSocket=/var/run/mysqld/mysqld.sock
		elif [ -d /var/lib/mysql ] ; then
			mysqlSocket=/var/lib/mysql/mysql.sock
		else
			mysqlSocket=/run/mysqld/mysqld.sock
		fi
		echo "mysql socket|port not specified: using $mysqlSocket" 1>&1
		echo "$mysql_host:$mysql_user:$mysql_pass:$mysqlSocket" > \
			"$DESTDIR"$CONTROLDIR/host.mysql
	fi
	/bin/chmod 640 "$DESTDIR"$CONTROLDIR/host.mysql
	$chown indimail:qmail "$DESTDIR"$CONTROLDIR/host.mysql
fi
LIST="tcp.imap tcp.pop3 tcp.poppass"
if [ -f "$DESTDIR"$QmailBinPrefix/bin/tcprules ] ; then
	for i in $LIST
	do
		t1=1
		if [ -f "$DESTDIR"$sysconfdir/tcp/$i ] ; then
			t1=`date -r "$DESTDIR"$sysconfdir/tcp/$i +'%s'`
		else
			continue
		fi
		if [ -f "$DESTDIR"$sysconfdir/tcp/$i.cdb ] ; then
			t2=`date -r "$DESTDIR"$sysconfdir/tcp/$i.cdb +'%s'`
		else
			t2=0
		fi
		if [ $t1 -gt $t2 ] ; then
			echo "Creating CDB "$DESTDIR"$sysconfdir/tcp/$i.cdb"
			"$DESTDIR"$QmailBinPrefix/bin/tcprules \
			"$DESTDIR"$sysconfdir/tcp/"$i".cdb \
			"$DESTDIR"$sysconfdir/tcp/"$i".tmp \
				< "$DESTDIR"$sysconfdir/tcp/$i
		fi
	done
else
	for i in $LIST
	do
		echo "ucspi-tcp not installed: Run tcprules to build cdb for rules defined in "$DESTDIR"$sysconfdir/tcp/$i" 1>&2
	done
fi
if [ -f "$DESTDIR"$QmailBinPrefix/bin/bogofilter ] ; then
	for i in spam ham register-spam register-ham
	do
		"$DESTDIR"$QmailBinPrefix/bin/maildirmake "$DESTDIR"$QmailHOME/alias/$i >/dev/null 2>&1
		if [ $? -eq 0 ] ; then
			$chown -R alias:qmail "$DESTDIR"$QmailHOME/alias/$i
			/bin/chmod -R 775 "$DESTDIR"$QmailHOME/alias/$i
		fi
		(
			echo "|$libexecdir/bogofilter-qfe"
			echo "$QmailHOME/alias/$i/"
		) > "$DESTDIR"$QmailHOME/alias/.qmail-B"$i"
		$chown alias:qmail "$DESTDIR"$QmailHOME/alias/.qmail-B"$i"
	done
fi
if [ ! -d "$DESTDIR"$QmailHOME/domains ] ; then
	/bin/mkdir -p "$DESTDIR"$QmailHOME/domains
	if [ $? -ne 0 ] ; then
		return 1
	fi
	$chown root:qmail "$DESTDIR"$QmailHOME/domains
	/bin/chmod 775 "$DESTDIR"$QmailHOME/domains
fi
}

check_update_if_diff()
{
	if [ ! -s $1 -o ! -f $1 ] ; then
		update=1
	else
		update=0
		val=$(@prefix@/bin/qcat $1 2>/dev/null)
	fi
	if [ $update -eq 1 -o ! " $val" = " $2" ] ; then
		if [ $silent -eq 0 ] ; then
			echo "updating $1 with $2"
		fi
		echo $2 > $1
		return 1
	else
		return 0
	fi
}

check_libmysqlclient_lib()
{
	if [ -z "$mysqlPrefix" ] ; then
		if [ -d /usr/lib/mysql -o -d /usr/lib64/mysql ] ; then
			mysqlPrefix=/usr
		elif [ -d /usr/local/lib/mysql ] ; then
			mysqlPrefix=/usr/local
		elif [ -L /usr/local/lib/mysql ] ; then
			mysqlPrefix=/usr/local
		elif [ -d /usr/local/mysql/lib ] ; then
			mysqlPrefix=/usr/local/mysql
		elif [ -L /usr/local/mysql/lib ] ; then
			mysqlPrefix=/usr/local/mysql
		elif [ -x /usr/bin/mysqld -o -x /usr/sbin/mysqld -o -x /usr/sbin/mariadbd ] ; then
			mysqlPrefix=/usr
		else
			echo "check_libmysqlclient_lib: Couldn't figure out mysqlPrefix. Specify --mysqlPrefix" 1>&2
			return 1
		fi
	fi
	if [ -z "$CONTROLDIR" ] ; then
		CONTROLDIR=$sysconfdir/control
	fi
	if [ -f "$DESTDIR"$CONTROLDIR/libmysql ] ; then
		mysqllib=$(@prefix@/bin/qcat "$DESTDIR"$CONTROLDIR/libmysql 2>/dev/null)
		if [ -f $mysqllib ] ; then
			return 0
		else
			mysqllib=""
		fi
	fi
	# upgrade MYSQL_LIB for dynamic loading of libmysqlclient
	# mysql-community on 64bit systems
	if [ -z "$mysqllib" -a -d /usr/lib64/mysql ] ; then
		prev_num=0
		for i in `/bin/ls -t /usr/lib64/mysql/libmysqlclient.so.*.*.* 2>/dev/null`
		do
			file=`basename $i`
			num=`echo $file | cut -d. -f3`
			if [ $num -gt $prev_num ] ; then
				prev_num=$num
				mysqllib=$i
			fi
		done
	fi
	if [ -z "$mysqllib" -a -d /usr/lib64 ] ; then
		prev_num=0
		for i in `/bin/ls -t /usr/lib64/libmysqlclient.so.*.*.* 2>/dev/null`
		do
			file=`basename $i`
			num=`echo $file | cut -d. -f3`
			if [ $num -gt $prev_num ] ; then
				prev_num=$num
				mysqllib=$i
			fi
		done
		if [ -z "$mysqllib" ] ; then
			prev_num=0
			for i in `/bin/ls -t /usr/lib64/libmariadb.so.* 2>/dev/null`
			do
				file=`basename $i`
				num=`echo $file | cut -d. -f3`
				if [ $num -gt $prev_num ] ; then
					prev_num=$num
					mysqllib=$i
				fi
			done
		fi
	fi
	if [ -z "$mysqllib" ] ; then
		case "$host" in
			*-*-darwin**)
			LIBMYSQL=`/bin/ls -t $mysqlPrefix/lib/libmysqlclient.*.dylib 2>/dev/null`
			LIBMARIADB=`/bin/ls -t $mysqlPrefix/lib/libmariadb.*.dylib 2>/dev/null`
			x=2
			;;
			*-*-freebsd**)
			LIBMYSQL=`/bin/ls -t $mysqlPrefix/lib/mysql/libmysqlclient.so.* 2>/dev/null`
			LIBMARIADB=`/bin/ls -t $mysqlPrefix/lib/mysql/libmariadb.so.*. 2>/dev/null`
			x=3
			;;
			*)
			LIBMYSQL=`/bin/ls -t $mysqlPrefix/lib*/libmysqlclient.so.*.*.* 2>/dev/null`
			LIBMARIADB=`/bin/ls -t $mysqlPrefix/lib*/libmariadb.so.* 2>/dev/null`
			x=3
			;;
		esac
		prev_num=0
		for i in $LIBMYSQL
		do
			file=`basename $i`
			num=`echo $file | cut -d. -f"$x"`
			if [ $num -gt $prev_num ] ; then
				prev_num=$num
				mysqllib=$i
			fi
		done
		if [ -z "$mysqllib" ] ; then
			for i in $LIBMARIADB
			do
				file=`basename $i`
				num=`echo $file | cut -d. -f"$x"`
				if [ $num -gt $prev_num ] ; then
					prev_num=$num
					mysqllib=$i
				fi
			done
		fi
	fi
	if [ -z "$mysqllib" -a -f /etc/debian_version ] ; then
		# upgrade MYSQL_LIB for dynamic loading of libmysqlclient
		prev_num=0
		for i in `/bin/ls -t /usr/lib/*-linux-gnu/libmariadbclient.so.*.*.* 2>/dev/null`
		do
			file=`basename $i`
			num=`echo $file | cut -d. -f"$x"`
			if [ $num -gt $prev_num ] ; then
				prev_num=$num
				mysqllib=$i
			fi
		done
		if [ -z "$mysqllib" ] ; then
			prev_num=0
			for i in `/bin/ls -t /usr/lib/*-linux-gnu/libmariadb.so.* 2>/dev/null`
			do
				file=`basename $i`
				num=`echo $file | cut -d. -f"$x"`
				if [ $num -gt $prev_num ] ; then
					prev_num=$num
					mysqllib=$i
				fi
			done
		fi
	fi
	#
	# this is crazy. Both mariadb and oracle are breaking things
	# around
	# MariaDB-shared, mariadb-connector-c
	# MariaDB-Compat
	# mysql-community-libs
	#
	if [ -z "$mysqllib" ] ; then
		if [ -f /etc/debian_version ] ; then
			dir="/usr/lib/*-linux-gnu/libmysqlclient.so.*.*.* \
				/usr/lib/*-linux-gnu/libmariadbclient.so.*.*.* \
				/usr/lib*/mysql/libmariadbclient.so.*.*.*"
		else
			dir="$mysqlPrefix/lib*/libmariadb.so.* \
				$mysqlPrefix/lib*/libmysqlclient.so.*.*.* \
				$mysqlPrefix/lib*/mysql/libmysqlclient.so.*.*.*"
		fi
		for i in `ls -t $dir 2>/dev/null`
		do
			file=`basename $i`
			num=`echo $file | cut -d. -f3`
			if [ $num -gt $prev_num ] ; then
				prev_num=$num
				mysqllib=$i
			fi
		done
	fi
	if [ -n "$mysqllib" ] ; then
		check_update_if_diff "$DESTDIR"$CONTROLDIR/libmysql $mysqllib
	else
		echo "Sorry: Couldn't figure out mysql shared library. Manually edit "$DESTDIR"$CONTROLDIR/libmysql" 1>&2
		if [ -f "$DESTDIR"$CONTROLDIR/libmysql ] ; then
			echo "removing "$DESTDIR"$CONTROLDIR/libmysql"
			/bin/rm -f "$DESTDIR"$CONTROLDIR/libmysql
		fi
	fi
	return 0
}

default_qmail_control()
{
if [ -z "$CONTROLDIR" ] ; then
	CONTROLDIR=$sysconfdir/control
fi
t="$DESTDIR"$CONTROLDIR
if [ ! -d $t ] ; then
	/bin/mkdir -p $t
	if [ $? -ne 0 ] ; then
		return 1
	fi
fi
if [ ! -d "$DESTDIR"$sysconfdir/tcp ] ; then
	/bin/mkdir -p "$DESTDIR"$sysconfdir/tcp
	if [ $? -ne 0 ] ; then
		return 1
	fi
fi
(
echo "$RCSID"
echo "# generated on $host at `date`"
echo "# by the below command"
echo "$prog_args"
echo ""
) > $t/.qmail_control
if [ -n "$verbose" ] ; then
	# created by config-fast
	echo "Creating Default qmail Control Files"
	echo "me"
	echo "localiphost"
	echo "defaulthost"
	echo "envnoathost"
	echo "defaultdomain"
	echo "plusdomain"
	echo "locals"
	echo "rcpthosts"
fi

hostname=$([ -n "$HOSTNAME" ] && echo "$HOSTNAME" || uname -n)
if [ -f ./config-fast ] ; then
	if [ -n "$verbose" ] ; then
		./config-fast --verbose --destdir=$DESTDIR $hostname
	else
		./config-fast --quiet --destdir=$DESTDIR $hostname
	fi
else
	if [ -n "$verbose" ] ; then
		"$DESTDIR"$libexecdir/config-fast --verbose --destdir=$DESTDIR $hostname
	else
		"$DESTDIR"$libexecdir/config-fast --quiet --destdir=$DESTDIR $hostname
	fi
fi
if [ ! -f $t/localiphost ] ; then
	[ -n "$verbose" ] && echo "localiphost"
	/bin/cp $t/me $t/localiphost
fi
if [ ! -d $t/global_vars ] ; then
	mkdir -p $t/global_vars
fi
if [ ! -f $t/global_vars/DEFAULT_DOMAIN -a ! -L $t/global_vars/DEFAULT_DOMAIN ] ; then
	if [ -f $CONTROLDIR/defaultdomain ] ; then
		[ -n "$verbose" ] && echo "global variable DEFAULT_DOMAIN"
		/bin/ln -sr $CONTROLDIR/defaultdomain $t/global_vars/DEFAULT_DOMAIN 2>/dev/null
		if [ $? -ne 0 ] ; then
			cd $t/global_vars
			ln -s ../defaultdomain DEFAULT_DOMAIN
		fi
	else
		echo "Warning!!! $CONTROLDIR/defaultdomain missing" 1>&2
	fi
fi
if [ ! -f $t/global_vars/QMAILDEFAULTHOST -a ! -L $t/global_vars/QMAILDEFAULTHOST ] ; then
	if [ -f $CONTROLDIR/defaulthost ] ; then
		[ -n "$verbose" ] && echo "global variable QMAILDEFAULTHOST"
		/bin/ln -sr $CONTROLDIR/defaulthost $t/global_vars/QMAILDEFAULTHOST 2>/dev/null
		if [ $? -ne 0 ] ; then
			cd $t/global_vars
			ln -s ../defaulthost QMAILDEFAULTHOST
		fi
	else
		echo "Warning!!! $CONTROLDIR/defaulthost missing" 1>&2
	fi
fi
if [ -n "$verbose" ] ; then
	echo "Setting control files (if not existing) to defaults"
	echo "locals"
	echo "rcpthosts"
	echo "defaultdelivery"
	echo "servicedir.conf"
	echo "nodnscheck"
	echo "hostip"
	echo "users/assign"
	echo "greylist.white"
	echo "chkrcptdomains"
	echo "chksenderomains"
	echo "smtpgreeting"
	echo "databytes"
	echo "timeoutsmtpd"
	echo "timeoutremote"
	echo "dnsbllist"
	echo "tcp/tcp.[smtp,qmtp,qmqp]"
	echo "servercipherlist"
	echo "serverciphersuite"
	echo "tlsservermethod"
	echo "clientcipherlist"
	echo "clientciphersuite"
	echo "tlsclientmethod"
	echo "procmailrc"
	echo "libindimail"
	echo "libmysql"
fi
if [ ! -f $t/servicedir.conf ] ; then
	[ -n "$verbose" ] && echo "servicedir.conf"
	echo "$servicedir" > $t/servicedir.conf
fi
if [ ! -f $t/defaultdelivery ] ; then
	[ -n "$verbose" ] && echo "defaultdelivery"
	echo "./Maildir/" > $t/defaultdelivery
fi
if [ ! -f $t/nodnscheck ] ; then
	# qmail-smtpd
	[ -n "$verbose" ] && echo "nodnscheck"
	(
	echo "@gmail.com"
	echo "@yahoo.com"
	echo "@hotmail.com"
	echo "@outlook.com"
	echo "@aol.com"
	echo "@$hostname"
	) > $t/nodnscheck
else
	grep "$hostname" $t/nodnscheck >/dev/null
	if [ $? -ne 0 ]; then
		echo "@$hostname" >> $t/nodnscheck
	fi
fi

"$DESTDIR"$libexecdir/ipmeprint | \
	grep -Evw "::|::1|0\.0\.0\.0|127\.0\.0\.1|172.17.0|0000:0000:0000:0000:0000:0000:0000:0000" | \
	head -1|awk '{print $3}'> $TMPDIR/hostip.$$
if [ -s $TMPDIR/hostip.$$ ] ; then
	diff $TMPDIR/hostip.$$ $t/hostip >/dev/null 2>&1
	if [ $? -ne 0 ] ; then
		ip=$(@prefix@/bin/qcat $TMPDIR/hostip.$$)
		echo "hostip [$ip]"
		/bin/mv $TMPDIR/hostip.$$ $t/hostip
	else
		/bin/rm -f $TMPDIR/hostip.$$
	fi
else
	/bin/rm -f $TMPDIR/hostip.$$
	[ -n "$verbose" ] && echo "hostip [127.0.0.1]"
	echo 127.0.0.1 > $t/hostip
fi

if [ ! -f $t/smtpgreeting ] ; then
	[ -n "$verbose" ] && echo "smtpgreeting"
	echo "$hostname"            > $t/smtpgreeting
fi

if [ ! -f $t/databytes ] ; then
	[ -n "$verbose" ] && echo "databytes"
	echo "$databytes"           > $t/databytes
fi

if [ ! -f $t/timeoutsmtpd ] ; then
	[ -n "$verbose" ] && echo "timeoutsmtpd"
	echo 300                    > $t/timeoutsmtpd
fi

if [ ! -f $t/timeoutremote ] ; then
	[ -n "$verbose" ] && echo "timeoutremote"
	echo 300                    > $t/timeoutremote
fi

if [ ! -f $t/chkrcptdomains -a -n "$default_domain" ] ; then
	[ -n "$verbose" ] && echo "chkrcptdomains"
	echo $default_domain > $t/chkrcptdomains
fi

if [ ! -f $t/chksenderdomains -a -n "$default_domain" ] ; then
	[ -n "$verbose" ] && echo "chksenderdomains"
	echo $default_domain > $t/chksenderdomains
fi

if [ ! -f $t/greylist.white ] ; then
	[ -n "$verbose" ] && echo "greylist.white"
	/bin/cp $t/hostip $t/greylist.white
fi

if [ " $wipe_option" = " 1" -o ! -f "$DESTDIR"/$sysconfdir/users/assign ] ; then
	[ -n "$verbose" ] && echo "assign"
	echo "." > "$DESTDIR"$sysconfdir/users/assign
	if [ -f "$DESTDIR"$QmailBinPrefix/sbin/qmail-newu ] ; then
		"$DESTDIR"$QmailBinPrefix/sbin/qmail-newu "$DESTDIR"$sysconfdir/users
	fi
fi

if [ ! -f $t/dnsbllist ] ; then
	[ -n "$verbose" ] && echo "dnsbllist"
	echo "zen.spamhaus.org"     > $t/dnsbllist
fi

for i in smtp qmtp qmqp
do
	if [ ! -f "$DESTDIR"$sysconfdir/tcp/tcp.$i ] ; then
		[ -n "$verbose" ] && echo "tcp.$i"
	fi
done
if [ ! -f "$DESTDIR"$sysconfdir/tcp/tcp.smtp ] ; then
	(
	echo "#127.:allow,RELAYCLIENT=\"\""
	echo "#::1:allow,RELAYCLIENT=\"\""
	echo "#=:allow,GREYIP=\"127.0.0.1:1999\""
	echo "#:allow,GREYIP=\"127.0.0.1:1999\",RBLSMTPD=\"-No DNS PTR Record - MTA Misconfigured.\""
	) > "$DESTDIR"$sysconfdir/tcp/tcp.smtp
fi
if [ ! -f "$DESTDIR"$sysconfdir/tcp/tcp.qmtp ] ; then
	(
	echo ":deny"
	) > "$DESTDIR"$sysconfdir/tcp/tcp.qmtp
fi
if [ ! -f "$DESTDIR"$sysconfdir/tcp/tcp.qmqp ] ; then
	(
	echo ":deny"
	) > "$DESTDIR"$sysconfdir/tcp/tcp.qmqp
fi
LIST="tcp.smtp tcp.qmtp tcp.qmqp"
if [ -f "$DESTDIR"$QmailBinPrefix/bin/tcprules ] ; then
	for i in $LIST
	do
		t1=0
		if [ -f "$DESTDIR"$sysconfdir/tcp/$i ] ; then
			t1=`date -r "$DESTDIR"$sysconfdir/tcp/$i +'%s'`
		else
			continue
		fi
		if [ -f "$DESTDIR"$sysconfdir/tcp/$i.cdb ] ; then
			t2=`date -r "$DESTDIR"$sysconfdir/tcp/$i.cdb +'%s'`
		else
			t2=0
		fi
		if [ $t1 -gt $t2 ] ; then
			"$DESTDIR"$QmailBinPrefix/bin/tcprules \
			"$DESTDIR"$sysconfdir/tcp/"$i".cdb \
			"$DESTDIR"$sysconfdir/tcp/"$i".tmp \
				< "$DESTDIR"$sysconfdir/tcp/$i
		fi
	done
else
	for i in $LIST
	do
		echo "ucspi-tcp not installed: Run tcprules to build cdb for rules defined in "$DESTDIR"$sysconfdir/tcp/$i" 1>&2
	done
fi

for i in locals rcpthosts
do
	if [ ! -f $t/$i ] ; then
		[ -n "$verbose" ] && echo $i
		echo "localhost" > $t/$i
		sort -u $t/$i -o $t/$i
	else
			grep "localhost" $t/$i >/dev/null
			if [ $? -ne 0 ]; then
				echo $i
				echo "localhost" >> $t/$i
				sort -u $t/$i -o $t/$i
			fi
	fi
	chmod 644 $t/$i
done

n1=$(openssl ciphers -v | awk '{print $2}' | grep TLS | sort | uniq | tail -1 |cut -c5- |cut -d. -f1)
n2=$(openssl ciphers -v | awk '{print $2}' | grep TLS | sort | uniq | tail -1 |cut -c5- |cut -d. -f2)
max=$(expr 10 \* $n1 + $n2)
if [ ! -f $t/servercipherlist ] ; then
	[ -n "$verbose" ] && echo "servercipherlist"
	(
	/bin/echo -n "ALL:!aNULL:!ADH:!eNULL:!LOW:!EXP:RC4+RSA:+HIGH:+MEDIUM:"
	/bin/echo -n "-EDH-RSA-DES-CBC3-SHA:-EDH-DSS-DES-CBC3-SHA:-DES-CBC3-SHA:"
	/bin/echo -n "-DES-CBC3-MD5:+SSLv3:+TLSv1:-SSLv2:!DES:!MD5:!PSK:!RC4:"
	/bin/echo -n "!3DES:!SHA1:!SHA256:!SHA384"
	echo
	) > $t/servercipherlist
fi
if [ ! -f $t/serverciphersuite ] ; then
	[ -n "$verbose" ] && echo "serverciphersuite"
	echo "TLS_AES_256_GCM_SHA384:TLS_CHACHA20_POLY1305_SHA256:TLS_AES_128_GCM_SHA256" > $t/serverciphersuite
fi
if [ ! -f $t/clientcipherlist ] ; then
	[ -n "$verbose" ] && echo "clientcipherlist"
	echo "ALL:!aNULL:!ADH:!eNULL:!LOW:!DH:!EXP" > $t/clientcipherlist
fi
if [ ! -f $t/clientciphersuite ] ; then
	[ -n "$verbose" ] && echo "clientciphersuite"
	echo "TLS_AES_256_GCM_SHA384:TLS_CHACHA20_POLY1305_SHA256:TLS_AES_128_GCM_SHA256" > $t/clientciphersuite
fi
[ ! -d $t/global_vars ] && mkdir -p $t/global_vars
[ $max -gt 12 ] && fn=serverciphersuite || fn=""
if [ -n "$fn" -a ! -f $t/global_vars/TLS_CIPHER_SUITE -a ! -L $t/global_vars/TLS_CIPHER_SUITE ] ; then
	if [ -f $CONTROLDIR/$fn ] ; then
		[ -n "$verbose" ] && echo "global variable TLS_CIPHER_SUITE"
		/bin/ln -sr $CONTROLDIR/$fn $t/global_vars/TLS_CIPHER_SUITE 2>/dev/null
		if [ $? -ne 0 ] ; then
			cd $t/global_vars
			ln -s ../$fn TLS_CIPHER_SUITE
		fi
	else
		echo "Warning!!! $CONTROLDIR/$fn missing" 1>&2
	fi
fi
fn=servercipherlist
if [ ! -f $t/global_vars/TLS_CIPHER_LIST -a ! -L $t/global_vars/TLS_CIPHER_LIST ] ; then
	if [ -f $CONTROLDIR/$fn ] ; then
		[ -n "$verbose" ] && echo "global variable TLS_CIPHER_LIST"
		/bin/ln -sr $CONTROLDIR/$fn $t/global_vars/TLS_CIPHER_LIST 2>/dev/null
		if [ $? -ne 0 ] ; then
			cd $t/global_vars
			ln -s ../$fn TLS_CIPHER_LIST
		fi
	else
		echo "Warning!!! $CONTROLDIR/$fn missing" 1>&2
	fi
fi
max=$(openssl ciphers -v | awk '{print $2}' | grep TLS | sort | uniq | tail -1 | sed 's{\.{_{')
if [ -z "$max" ] ; then
	max=$(openssl ciphers -v | awk '{print $2}' | grep SSL | sort | uniq | tail -1)
	[ -z "$max" ] && m="TLSv1_2"
fi
if [ ! -f $t/tlsservermethod ] ; then
	[ -n "$verbose" ] && echo "tlsservermethod"
	echo $max > $t/tlsservermethod
fi
if [ ! -f $t/tlsclientmethod ] ; then
	[ -n "$verbose" ] && echo "tlsclientmethod"
	echo $max > $t/tlsclientmethod
fi

if [ ! -f "$DESTDIR"$sysconfdir/procmailrc ] ; then
	#
	# ensure call to procmail in .qmail-default
	# |preline procmail -p -m $sysconfdir/procmailrc || exit 111
	#
	[ -n "$verbose" ] && echo "procmailrc"
	(
	echo :0w
	echo "|$QmailBinPrefix/sbin/vfilter '' bounce-no-mailbox"
	) > "$DESTDIR"$sysconfdir/procmailrc
fi

if [ -n "$DESTDIR" ] ; then
	if [ -r /etc/debian_version ] ; then
		indlib=`ls -d "$DESTDIR"$QmailBinPrefix/lib/*-linux-gnu/libindimail.so.*.*.* 2>/dev/null|sed -e "s}$DESTDIR}}g"`
	else
		indlib=`ls -d "$DESTDIR"$QmailBinPrefix/lib*/libindimail.so.*.*.* 2>/dev/null|sed -e "s}$DESTDIR}}g"`
	fi
	if [ -z "$indlib" ] ; then
		if [ -r /etc/debian_version ] ; then
			indlib=`ls -d $QmailBinPrefix/lib/*-linux-gnu/libindimail.so.*.*.* 2>/dev/null`
		else
			indlib=`ls -d $QmailBinPrefix/lib*/libindimail.so.*.*.* 2>/dev/null`
		fi
	fi
else
	if [ -r /etc/debian_version ] ; then
		indlib=`ls -d $QmailBinPrefix/lib/*-linux-gnu/libindimail.so.*.*.* 2>/dev/null`
	else
		indlib=`ls -d $QmailBinPrefix/lib*/libindimail.so.*.*.* 2>/dev/null`
	fi
fi
if [ -n "$indlib" ] ; then
	check_update_if_diff $t/libindimail $indlib
else
	if [ -f $t/libindimail ] ; then
		echo "removing $t/libindimail"
		/bin/rm -f $t/libindimail
	fi
fi
check_libmysqlclient_lib
if [ ! -f $sysconfdir/backup.conf ] ; then
	[ -n "$verbose" ] && echo "Creating "$DESTDIR"$sysconfdir/backup.conf file"
	(
	for i in $servicedir /etc/clamd.d $sysconfdir $QmailHOME/domains \
		$QmailHOME/autoturn $QmailHOME/alias/.qmail-\* /etc/inittab \
		/etc/cron.d/indimail.cron /etc/cron.d/indimail-mta.cron /etc/init.d/svscan \
		/etc/passwd /etc/group /etc/hosts /etc/sysconfig/network \
		/etc/nsswitch.conf /etc/resolv.conf /etc/security /etc/alternatives \
		$QmailHOME/.bashrc $QmailHOME/.bash_profile $QmailHOME/.indimail $file
	do
		echo "$i"
	done
	) |sort -u > $sysconfdir/backup.conf
fi
}

dump_run_header()
{
echo "#!/bin/sh"
echo "$RCSID"
echo "# generated run script for $host at `date`"
echo "# by the below command"
echo "# $prog_args"
echo ""
}

dump_log_header()
{
if [ $# -ne 1 ] ; then
	echo "Usage: dump_log_header log_sub_dir" 1>&2
	exit 1
fi
echo "#!/bin/sh"
echo "$RCSID"
echo "# generated log script for $host at `date`"
echo "# by the below command"
echo "# $prog_args"
echo ""
echo "exec $QmailBinPrefix/bin/setuidgid qmaill \\"
echo "  $QmailBinPrefix/sbin/multilog t $LOGDIR/$1"
}

link_with_global()
{
if [ $# -ne 1 ] ; then
	echo "USAGE: link_with_global conf_dir" 1>&2
	return 1
fi
conf_dir=$1
if [ $nooverwrite -eq 1 -a -d $conf_dir ] ; then
	return 0
fi
if [ ! -d "$DESTDIR"$sysconfdir/control/global_vars ] ; then
	mkdir -p "$DESTDIR"$sysconfdir/control/global_vars
	$chown root:qmail "$DESTDIR"$sysconfdir/control/global_vars
	/bin/chmod 755 "$DESTDIR"$sysconfdir/control/global_vars
fi
if [ ! -f "$DESTDIR"$sysconfdir/control/global_vars/CONFSPLIT ] ; then
	echo QUEUESPLIT > "$DESTDIR"$sysconfdir/control/global_vars/CONFSPLIT
fi
if [ ! -f "$DESTDIR"$sysconfdir/control/global_vars/BIGTODO ] ; then
	echo 0 > "$DESTDIR"$sysconfdir/control/global_vars/BIGTODO
fi
if [ $usefsync -eq 1 -o $usefdatasync -eq 1 ] ; then
	echo 1 > "$DESTDIR"$sysconfdir/control/global_vars/USE_FDATASYNC
else
	> "$DESTDIR"$sysconfdir/control/global_vars/USE_FSYNC
	> "$DESTDIR"$sysconfdir/control/global_vars/USE_FDATASYNC
fi
if [ $usesyncdir -eq 1 ] ; then
	SYSTEM=$(uname -s)
	case "$SYSTEM" in
		Linux)
		echo 1 > "$DESTDIR"$sysconfdir/control/global_vars/USE_SYNCDIR
		;;
		*)
		> "$DESTDIR"$sysconfdir/control/global_vars/USE_SYNCDIR
		;;
	esac
else
	> "$DESTDIR"$sysconfdir/control/global_vars/USE_SYNCDIR
fi
if [ ! -f $conf_dir/.envdir -a ! -d $conf_dir/.envdir -a ! -L $conf_dir/.envdir ] ; then
	echo $sysconfdir/control/global_vars > $conf_dir/.envdir
fi
}

create_fifologger()
{
if [ $# -ne 2 ] ; then
	echo "USAGE: create_fifologger fifo_path servicedir" 1>&2
	return 1
fi
fifo_path=$1
SERVICEDIR=$2
if [ " $fifo_path" = " " ] ; then
	echo "fifo_path cannot be null" 1>&2
	return 1
fi
conf_dir="$DESTDIR"$SERVICEDIR/qmail-logfifo/variables
if [ $nooverwrite -eq 1 -a -d $conf_dir ] ; then
	return 0
fi
if [ $run_file_only -eq 1 ] ; then
	if [ -d $conf_dir ] ; then
		mv $conf_dir "$conf_dir".bak
	fi
fi
/bin/mkdir -p $conf_dir
$chown root:indimail $conf_dir
/bin/chmod 555 $conf_dir
link_with_global $conf_dir

# qmail-logfifo logger
/bin/mkdir -p "$DESTDIR"$SERVICEDIR/qmail-logfifo/log
if [ $down_state -eq 1 ] ; then
	$TOUCH "$DESTDIR"$SERVICEDIR/qmail-logfifo/down
fi

echo  $fifo_path > $conf_dir/LOGFILTER
(
dump_run_header
echo "exec 2>&1"
echo "LOGFILTER=\$(@prefix@/bin/qcat variables/LOGFILTER)"
echo "LOGFIFODIR=\$(dirname \$LOGFILTER)"
echo "if [ ! -d \$LOGFIFODIR ] ; then"
echo "  mkdir -p \$LOGFIFODIR"
echo "fi"
echo "if [ ! -p \$LOGFILTER ] ; then"
echo "  /bin/rm -f \$LOGFILTER"
echo "  /usr/bin/mkfifo -m 666 \$LOGFILTER"
echo "fi"
echo "exec @prefix@/bin/setuidgid indimail \\"
echo "@prefix@/bin/qcat \$LOGFILTER"
) > "$DESTDIR"$SERVICEDIR/qmail-logfifo/run
/bin/chmod +x "$DESTDIR"$SERVICEDIR/qmail-logfifo/run

if [ ! -d $LOGDIR ] ; then
	/bin/mkdir -p $LOGDIR
	$chown qmaill:nofiles $LOGDIR
fi
#qmail-logfifo log script
dump_log_header logfifo > "$DESTDIR"$SERVICEDIR/qmail-logfifo/log/run
/bin/chmod +x "$DESTDIR"$SERVICEDIR/qmail-logfifo/log/run
echo "$prog_args" > $conf_dir/.options

if [ $run_file_only -eq 1 ] ; then
	if [ -d "$conf_dir".bak ] ; then
		/bin/rm -rf $conf_dir
		/bin/mv "$conf_dir".bak $conf_dir
	fi
fi
}

create_smtp()
{
if [ $# -ne 9 ] ; then
	echo "USAGE: create_smtp qbase queue_count first_queue_no supervise_dir smtp_port use_ssl=0|1 force_tls=0|1 infifo=fifo_path utf8=0|1" 1>&2
	return 1
fi

QUEUE_BASE=$1
QUEUE_COUNT=$2
QUEUE_START=$3
SERVICEDIR=$4
SMTP_PORT=$5
smtp_ssl=$6
force_tls=$7
infifo=$8
utf8=$9

# qmail-smtpd script
if [ " $servicetag" = " " ] ; then
	tag=$SMTP_PORT
else
	tag=$servicetag
fi
if [ $nooverwrite -eq 1 -a -d "$DESTDIR"$SERVICEDIR/qmail-smtpd.$tag/variables ] ; then
	return 0
fi
conf_dir="$DESTDIR"$SERVICEDIR/qmail-smtpd.$tag/variables
if [ $run_file_only -eq 1 ] ; then
	if [ -d $conf_dir ] ; then
		mv $conf_dir "$conf_dir".bak
	fi
fi
/bin/mkdir -p $conf_dir
$chown root:indimail $conf_dir
/bin/chmod 550 $conf_dir
link_with_global $conf_dir

echo "" > $conf_dir/ENVHEADERS
/bin/mkdir -p "$DESTDIR"$SERVICEDIR/qmail-smtpd.$tag/log
if [ $down_state -eq 1 ] ; then
	$TOUCH "$DESTDIR"$SERVICEDIR/qmail-smtpd.$tag/down
fi
if [ -d /run ] ; then
	rundir=/run/indimail
elif [ -d /var/run ] ; then
	rundir=/var/run/indimail
else
	rundir=/tmp/indimail
fi
if [ -n "$infifo" ] ; then
	fifo_name=`basename $infifo`
	infifo_dir=`dirname $infifo`
	if [ $infifo_dir = "." ] ; then
		infifo_dir=$rundir/inlookup
	fi
fi
if [ " $ipaddress" = " " ] ; then
	echo 0 > $conf_dir/LOCALIP
else
	echo $ipaddress > $conf_dir/LOCALIP
fi
echo $SMTP_PORT > $conf_dir/PORT
if [ ! " $maxdaemons" = " " ] ; then
	echo "$maxdaemons" > $conf_dir/MAXDAEMONS
else
	echo $CONCURRENCYINCOMING > $conf_dir/MAXDAEMONS
fi
if [ -n "$infifo" ] ; then
	echo $fifo_name  > $conf_dir/INFIFO
	echo $infifo_dir > $conf_dir/INFIFODIR
fi

if [ " $enable_smtp_plugin" = " " ] ; then
	echo > $conf_dir/DISABLE_PLUGIN
else
	> $conf_dir/DISABLE_PLUGIN
fi
if [ ! " $maxperip" = " " ] ; then
	echo "$maxperip" > $conf_dir/MAXPERIP
else
	echo 10 > $conf_dir/MAXPERIP
fi
if [ ! " $CONTROLDIR" = " " ] ; then
	echo "$CONTROLDIR" > $conf_dir/CONTROLDIR
else
	> $conf_dir/CONTROLDIR
fi
if [ " $helofqdn" = " " ] ; then
	> $conf_dir/ENFORCE_FQDN_HELO
else
	echo > $conf_dir/ENFORCE_FQDN_HELO
fi
if [ " $helocheck" = " " ] ; then
	> $conf_dir/BADHELOCHECK
else
	echo > $conf_dir/BADHELOCHECK
fi
if [ " $password_cache" = " " ] ; then
	> $conf_dir/PASSWD_CACHE
else
	echo > $conf_dir/PASSWD_CACHE
fi
if [ " $query_cache" = " " ] ; then
	> $conf_dir/QUERY_CACHE
else
	echo > $conf_dir/QUERY_CACHE
fi
if [ -n "$default_domain" ] ; then
	if [ -f $sysconfdir/control/global_vars/DEFAULT_DOMAIN ] ; then
		t=$(@prefix@/bin/qcat $sysconfdir/control/global_vars/DEFAULT_DOMAIN)
		if [ ! "$t" = "$default_domain" ] ; then
			echo $default_domain > $conf_dir/DEFAULT_DOMAIN
		fi
	else
		echo $default_domain > $conf_dir/DEFAULT_DOMAIN
	fi
fi
if [ ! " $memory" = " " ] ; then
	echo $memory > $conf_dir/SOFT_MEM
else
	echo 52428800 > $conf_dir/SOFT_MEM
fi
if [ " $smtp_ssl" = " 1" ] ; then
	echo > $conf_dir/SMTPS
else
	> $conf_dir/SMTPS
fi
if [ " $force_tls" = " 1" ] ; then
	echo > $conf_dir/FORCE_TLS
else
	> $conf_dir/FORCE_TLS
fi
if [ " $smtp_ssl" = " 1" -o " $force_tls" = " 1" ] ; then
	echo $sysconfdir/certs > $conf_dir/CERTDIR
fi
if [ $use_starttls -eq 0 ] ; then
	> $conf_dir/STARTTLS
else
	echo > $conf_dir/STARTTLS
	echo $sysconfdir/certs > $conf_dir/CERTDIR
fi
if [ " $min_free" = " " ] ; then
	echo 52428800 > $conf_dir/MIN_FREE
else
	echo $min_free > $conf_dir/MIN_FREE
fi
if [ " $QmailBinPrefix" = " /usr" ] ; then
	echo "/bin:/usr/bin:/usr/sbin:/sbin" > $conf_dir/PATH
else
	echo "/bin:/usr/bin:$QmailBinPrefix/bin:$QmailBinPrefix/sbin" > $conf_dir/PATH
fi
if [ $enablecram -eq 1 ] ; then
	echo 1 > $conf_dir/ENABLE_CRAM
else
	> $conf_dir/ENABLE_CRAM
fi
if [ " $odmr" = " " ] ; then
	if [ $no_multi -eq 1 ] ; then
		echo $qmailqueue > $conf_dir/QMAILQUEUE
		echo "$QUEUE_BASE"/queue1 > $conf_dir/QUEUEDIR
	else
		> $conf_dir/QUEUEDIR
		for i in QUEUE_BASE QUEUE_COUNT QUEUE_START
		do
			/bin/rm -f $conf_dir/$i
		done
		t=$("$DESTDIR"$QmailBinPrefix/bin/envdir $conf_dir env 2>/dev/null | grep QUEUE_BASE | cut -d= -f2)
		if [ ! "$t" = "$QUEUE_BASE" ] ; then
			echo $QUEUE_BASE   > $conf_dir/QUEUE_BASE
		fi
		t=$("$DESTDIR"$QmailBinPrefix/bin/envdir $conf_dir env 2>/dev/null | grep QUEUE_COUNT | cut -d= -f2)
		if [ ! "$t" = "$QUEUE_COUNT" ] ; then
			echo $QUEUE_COUNT > $conf_dir/QUEUE_COUNT
		fi
		t=$("$DESTDIR"$QmailBinPrefix/bin/envdir $conf_dir env 2>/dev/null | grep QUEUE_START | cut -d= -f2)
		if [ ! "$t" = "$QUEUE_START" ] ; then
			echo $QUEUE_START  > $conf_dir/QUEUE_START
		fi
	fi
	if [ " $qregex" = " " ] ; then
		> $conf_dir/QREGEX
	else
		echo > $conf_dir/QREGEX
	fi
	if [ " $viruscheck" = " " ] ; then
		> $conf_dir/VIRUSCHECK
	else
		echo $viruscheck > $conf_dir/VIRUSCHECK
	fi
	if [ " $qhpsi" = " " ] ; then
		> $conf_dir/QHPSI
	else
		echo $qhpsi > $conf_dir/QHPSI
	fi
	if [ " $bodycheck" = " " ] ; then
		> $conf_dir/BODYCHECK
	else
		if [ " $bodycheck" = " 1" ] ; then
			echo > $conf_dir/BODYCHECK
		else
			echo $bodycheck > $conf_dir/BODYCHECK
		fi
	fi
	if [ $usefsync -ne 0 ] ; then
		echo 1 > $conf_dir/USE_FSYNC
	fi
	if [ $usefdatasync -ne 0 ] ; then
		echo 1 > $conf_dir/USE_FDATASYNC
	fi
	if [ $usesyncdir -ne 0 ] ; then
		echo 1 > $conf_dir/USE_SYNCDIR
	fi
	if [ ! " $spamfilter" = " " ] ; then
		echo "$spamfilter" > $conf_dir/SPAMFILTER
		echo "1" > $conf_dir/MAKE_SEEKABLE
		if [ ! " $spamexitcode" = " " ] ; then
			echo "$spamexitcode" > $conf_dir/SPAMEXITCODE
			if [ ! " $rejectspam" = " " ] ; then
				echo "$rejectspam" > $conf_dir/REJECTSPAM
			else
				> $conf_dir/REJECTSPAM
			fi
		else
			> $conf_dir/SPAMEXITCODE
		fi
	else
		> $conf_dir/SPAMFILTER
	fi
	if [ ! " $masquerade" = " " ] ; then
		echo  > $conf_dir/MASQUERADE
	else
		> $conf_dir/MASQUERADE
	fi
	if [ ! " $domainlimits" = " " ] ; then
		echo  > $conf_dir/DOMAIN_LIMITS
	else
		> $conf_dir/DOMAIN_LIMITS
	fi
	if [ $chkrecipient -eq 1 ] ; then
		echo 1 > $conf_dir/CHECKRECIPIENT
	else
		> $conf_dir/CHECKRECIPIENT
	fi
	if [ $chksender -eq 1 ] ; then
		echo 1 > $conf_dir/CHECKRECIPIENT
	else
		> $conf_dir/CHECKRECIPIENT
	fi
	if [ ! " $antispoof" = " " ] ; then
		echo  > $conf_dir/ANTISPOOFING
	else
		> $conf_dir/ANTISPOOFING
	fi
	if [ ! " $cugmail" = " " ] ; then
		echo  > $conf_dir/CUGMAIL
	else
		> $conf_dir/CUGMAIL
	fi
	if [ ! " $dnscheck" = " " ] ; then
		> $conf_dir/NODNSCHECK
	else
		echo > $conf_dir/NODNSCHECK
	fi
	if [ -f $CONTROLDIR/hostaccess ] ; then
		if [ " $paranoid" =  " 1" ] ; then
			echo > $conf_dir/PARANOID
		else
			> $conf_dir/PARANOID
		fi
		if [ " $dmasquerade" = " 1" ] ; then
			echo > $conf_dir/DOMAIN_MASQUERADE
		else
			> $conf_dir/DOMAIN_MASQUERADE
		fi
	else
		> $conf_dir/PARANOID
		> $conf_dir/DOMAIN_MASQUERADE
	fi
	if [ $secureauth -eq 1 ] ; then
		echo 1 > $conf_dir/SECURE_AUTH
	else
		> $conf_dir/SECURE_AUTH
	fi
	if [ ! " $forceauthsmtp" = " " ] ; then
		echo > $conf_dir/REQUIREAUTH
		> $conf_dir/AUTH_ALL
		> $conf_dir/CHECKRELAY
	else
		> $conf_dir/REQUIREAUTH
		if [ ! " $authall" = " " ] ; then
			echo > $conf_dir/AUTH_ALL
			> $conf_dir/CHECKRELAY
		else
			> $conf_dir/AUTH_ALL
			if [ ! " $chkrelay" = " " ] ; then
				echo  > $conf_dir/CHECKRELAY
			else
				> $conf_dir/CHECKRELAY
			fi
		fi
	fi
	if [ $utf8 -eq 1 ] ; then
		echo > $conf_dir/UTF8
	else
		> $conf_dir/UTF8
	fi
else
	if [ $secureauth -eq 1 ] ; then
		echo 1 > $conf_dir/SECURE_AUTH
	else
		> $conf_dir/SECURE_AUTH
	fi
	if [ ! " $forceauthsmtp" = " " ] ; then
		echo > $conf_dir/REQUIREAUTH
	else
		> $conf_dir/REQUIREAUTH
	fi
fi # odmr
echo 1 > $conf_dir/STRIP_DOMAIN # for sys-checkpwd
if [ $barelf -eq 1 ] ; then
	echo 1 > $conf_dir/ALLOW_BARELF
else
	> $conf_dir/ALLOW_BARELF
fi
if [ $shared_objects -eq 1 ] ; then
	plugin_count=0
	if [ -f "$DESTDIR"@prefix@/lib/indimail/plugins/qmail_smtpd.so ] ; then
	if [ $use_dlmopen -eq 1 ] ; then
	echo "1" > $conf_dir/USE_DLMOPEN
	fi
	echo "@prefix@/lib/indimail/plugins/qmail_smtpd.so" > $conf_dir/PLUGIN"$plugin_count"
	echo "smtp_init" > $conf_dir/PLUGIN"$plugin_count"_init
	echo "$sysconfdir" > $conf_dir/PLUGIN"$plugin_count"_dir
	plugin_count=`expr $plugin_count + 1`
	fi
	if [ -f "$DESTDIR"@prefix@/lib/indimail/plugins/rblsmtpd.so ] ; then
	echo "@prefix@/lib/indimail/plugins/rblsmtpd.so" > $conf_dir/PLUGIN"$plugin_count"
	plugin_count=`expr $plugin_count + 1`
	fi
fi
case "$dkverify_option" in
	dkimstrict)
	/bin/rm -f $conf_dir/DKIMSIGN $conf_dir/DKIMVERIFY $conf_dir/DKIMQUEUE
	echo FGHIKLMNOQRTUVWjp > $conf_dir/DKIMVERIFY
	> $conf_dir/UNSIGNED_SUBJECT
	> $conf_dir/UNSIGNED_FROM
	if [ -n  "$spamfilter" ] ; then
		echo "$QmailBinPrefix/sbin/qmail-spamfilter $QmailBinPrefix/sbin/qmail-dkim" \
			> $conf_dir/QMAILQUEUE
	else
		echo "$QmailBinPrefix/sbin/qmail-dkim" > $conf_dir/QMAILQUEUE
	fi
	if [ $no_multi -eq 1 ] ; then
		echo "$QUEUE_BASE"/queue1 > $conf_dir/QUEUEDIR
	fi
	;;
	dkim)
	/bin/rm -f $conf_dir/DKIMSIGN $conf_dir/DKIMVERIFY $conf_dir/DKIMQUEUE
	echo > $conf_dir/DKIMVERIFY
	echo > $conf_dir/UNSIGNED_SUBJECT
	> $conf_dir/UNSIGNED_FROM
	if [ -n  "$spamfilter" ] ; then
		echo "$QmailBinPrefix/sbin/qmail-spamfilter $QmailBinPrefix/sbin/qmail-dkim" \
			> $conf_dir/QMAILQUEUE
	else
		echo "$QmailBinPrefix/sbin/qmail-dkim" > $conf_dir/QMAILQUEUE
	fi
	if [ $no_multi -eq 1 ] ; then
		echo "$QUEUE_BASE"/queue1 > $conf_dir/QUEUEDIR
	fi
	;;
	none|*)
	/bin/rm -f $conf_dir/DKIMSIGN $conf_dir/DKIMVERIFY $conf_dir/DKIMQUEUE
	if [ $no_multi -eq 1 ] ; then
		echo "$QUEUE_BASE"/queue1 > $conf_dir/QUEUEDIR
		if [ -n  "$spamfilter" ] ; then
			echo "$QmailBinPrefix/sbin/qmail-spamfilter" > $conf_dir/QMAILQUEUE
		else
			echo $qmailqueue > $conf_dir/QMAILQUEUE
		fi
	else
		if [ -n  "$spamfilter" ] ; then
			echo "$QmailBinPrefix/sbin/qmail-spamfilter" > $conf_dir/QMAILQUEUE
		else
			echo "$QmailBinPrefix/sbin/qmail-multi" > $conf_dir/QMAILQUEUE
		fi
		> $conf_dir/QUEUEDIR
	fi
	;;
esac
AUTHM=""
if [ -f $QmailBinPrefix/sbin/sys-checkpwd ] ; then
	if [ -n "$AUTHM" ] ; then
		AUTHM="$AUTHM $QmailBinPrefix/sbin/sys-checkpwd"
	else
		AUTHM="$QmailBinPrefix/sbin/sys-checkpwd"
	fi
fi
if [ -f $QmailBinPrefix/sbin/vchkpass ] ; then
	if [ -n "$AUTHM" ] ; then
		AUTHM="$AUTHM $QmailBinPrefix/sbin/vchkpass"
	else
		AUTHM="$QmailBinPrefix/sbin/vchkpass"
	fi
fi
if [ -n "$authsmtp" -o "$forceauthsmtp" ] ; then
	echo "$AUTHM" > $conf_dir/AUTHMODULES
else
	> $conf_dir/AUTHMODULES
	echo > $conf_dir/DISABLE_VRFY
fi
if [ $hidehost -eq 1 ] ; then
	echo 1 > $conf_dir/HIDE_HOST
else
	> $conf_dir/HIDE_HOST
fi
(
dump_run_header
echo "MYUID=\$(/usr/bin/id -u qmaild)"
case $host in
	*-*-darwin*)
	echo "MYGID=\$(dscl . -read /Groups/qmail PrimaryGroupID|awk '{print \$2}')"
	;;
	*)
	echo "MYGID=\$(/usr/bin/getent group qmail|awk -F: '{print \$3}')"
	;;
esac
echo "ME=\$(head -1 $CONTROLDIR/me)"
echo "HOSTNAME=\$(uname -n)"
echo
echo "if [ -z \"\$MYUID\" -o -z \"\$MYGID\" -o -z \"\$ME\" ]; then"
echo "    echo UID, GID, or ME is unset in"
echo "    echo \$(pwd)/run"
echo "    sleep 5"
echo "    exit 1"
echo "fi"
echo "if [ ! -f $CONTROLDIR/rcpthosts ]; then"
echo "    echo \"No $CONTROLDIR/rcpthosts!\""
echo "    echo \"Refusing to start SMTP listener because it'll create an open relay\""
echo "    sleep 5"
echo "    exit 1"
echo "fi"
echo
echo "exec 2>&1"
echo "exec $QmailBinPrefix/bin/envdir $envdir_opts variables sh -c \""
echo "exec $QmailBinPrefix/bin/softlimit -m \\\$SOFT_MEM -o 1024 \\"
if [ " $rbl" = " " ] ; then
	echo "$QmailBinPrefix/bin/tcpserver -v -H -R -l \$HOSTNAME \\"
	> $conf_dir/RBLCOMMAND
else
	echo "$QmailBinPrefix/bin/tcpserver -v -h -R -l \$HOSTNAME \\"
	(
	if [ " $rbl_list" = " " ] ; then
		if [ $shared_objects -eq 1 -a -f "$DESTDIR"@prefix@/lib/indimail/plugins/rblsmtpd.so ] ; then
		echo "@prefix@/lib/indimail/plugins/rblsmtpd.so -rzen.spamhaus.org -rdnsbl-1.uceprotect.net -rdnsbl.sorbs.net -rbl.spamcop.net"
		else
		echo "$QmailBinPrefix/bin/rblsmtpd -rzen.spamhaus.org -rdnsbl-1.uceprotect.net -rdnsbl.sorbs.net -rbl.spamcop.net"
		fi
	else
		if [ $shared_objects -eq 1 -a -f "$DESTDIR"@prefix@/lib/indimail/plugins/rblsmtpd.so ] ; then
		echo "@prefix@/lib/indimail/plugins/rblsmtpd.so $rbl_list"
		else
		echo "$QmailBinPrefix/bin/rblsmtpd $rbl_list"
		fi
	fi
	) > $conf_dir/RBLCOMMAND
fi
echo "-x $sysconfdir/tcp/tcp.smtp.cdb \\"
echo "-c ./variables/MAXDAEMONS -o -b \\\$MAXDAEMONS \\"
echo "-u qmaild -g qmail,indimail \\\$LOCALIP \\\$PORT \\\$RBLCOMMAND \\"
if [ -f /bin/false ] ; then
	false="/bin/false"
else
	false="/usr/bin/false"
fi
if [ $shared_objects -eq 1 -a -f "$DESTDIR"@prefix@/lib/indimail/plugins/qmail_smtpd.so ] ; then
	if [ -n "$authsmtp" -o -n "$forceauthsmtp" ] ; then
		echo "@prefix@/lib/indimail/plugins/qmail_smtpd.so \$HOSTNAME \\\$AUTHMODULES $false\""
	else
		echo "@prefix@/lib/indimail/plugins/qmail_smtpd.so\""
	fi
else
	if [ -n "$authsmtp" -o -n "$forceauthsmtp" ] ; then
		echo "$qmailsmtpd \$HOSTNAME \\\$AUTHMODULES $false\""
	else
		echo "$qmailsmtpd\""
	fi
fi
) > "$DESTDIR"$SERVICEDIR/qmail-smtpd.$tag/run
/bin/chmod +x "$DESTDIR"$SERVICEDIR/qmail-smtpd.$tag/run

# qmail-smtpd log script
if [ ! -d $LOGDIR ] ; then
	/bin/mkdir -p $LOGDIR
	$chown qmaill:nofiles $LOGDIR
fi
dump_log_header smtpd.$tag > "$DESTDIR"$SERVICEDIR/qmail-smtpd.$tag/log/run
/bin/chmod +x "$DESTDIR"$SERVICEDIR/qmail-smtpd.$tag/log/run
echo "$prog_args" > $conf_dir/.options

if [ $run_file_only -eq 1 ] ; then
	if [ -d "$conf_dir".bak ] ; then
		/bin/rm -rf $conf_dir
		/bin/mv "$conf_dir".bak $conf_dir
	fi
fi

# finally create fifologger as it modifies conf_dir
if [ ! " $logfilter" = " " ] ; then
	echo  $logfilter > $conf_dir/LOGFILTER
	if [ ! -f $SERVICEDIR/qmail-logfifo/run ] ; then
		prog_args="$QmailBinPrefix/sbin/svctool --fifologger=\"$logfilter\" --servicedir=\"$SERVICEDIR\""
		create_fifologger $logfilter $SERVICEDIR
	fi
else
	> $conf_dir/LOGFILTER
fi
}

create_queuedef()
{
if [ $# -ne 4 ] ; then
	echo "USAGE: create_queuedef qbase queue_count first_queue_no queue_def" 1>&2
	return 1
fi
QUEUE_BASE=$1
QUEUE_COUNT=$2
QUEUE_START=$3
QUEUE_DEF=$4

if [ " $CONTROLDIR" = " " ] ; then
	cntrldir=$sysconfdir/control
else
	cntrldir=$CONTROLDIR
fi
conf_dir="$DESTDIR"$cntrldir/$QUEUE_DEF
if [ $nooverwrite -eq 1 -a -d "$DESTDIR"$cntrldir/$QUEUE_DEF ] ; then
	return 0
fi
/bin/mkdir -p $conf_dir
$chown indimail:qmail $conf_dir
/bin/chmod 755 $conf_dir
link_with_global $conf_dir
if [ ! -f $conf_dir/.envdir ] ; then
	echo "$cntrldir/global_vars" > $conf_dir/.envdir
fi

if [ -n "$base_path" ] ; then
	echo $base_path    > $conf_dir/BASE_PATH
fi
if [ $no_multi -eq 1 ] ; then
	echo $qmailqueue > $conf_dir/QMAILQUEUE
	echo "$QUEUE_BASE"/queue1 > $conf_dir/QUEUEDIR
else
	for i in QUEUE_BASE QUEUE_COUNT QUEUE_START
	do
			val=$(eval "echo \${$i}")
			echo $val > $conf_dir/$i
			if [ ! -f "$DESTDIR"$cntrldir/global_vars/$i -a ! -L "$DESTDIR"$cntrldir/global_vars/$i ] ; then
				if [ -f $conf_dir/$i ] ; then
					[ -n "$verbose" ] && echo "global variable $i"
					ln -srf $conf_dir/$i "$DESTDIR"$cntrldir/global_vars/$i 2>/dev/null
					if [ $? -ne 0 ] ; then
						cd "$DESTDIR"$cntrldir/global_vars
						ln -s $conf_dir/$i $i
					fi
				else
					echo "Warning!!! $conf_dir/$i missing" 1>&2
				fi
			fi
	done
fi
if [ " $qregex" = " " ] ; then
	> $conf_dir/QREGEX
else
	echo > $conf_dir/QREGEX
fi
if [ " $viruscheck" = " " ] ; then
	> $conf_dir/VIRUSCHECK
else
	echo $viruscheck > $conf_dir/VIRUSCHECK
fi
if [ " $qhpsi" = " " ] ; then
	> $conf_dir/QHPSI
else
	echo $qhpsi > $conf_dir/QHPSI
fi
if [ $usefsync -ne 0 ] ; then
	echo 1 > $conf_dir/USE_FSYNC
fi
if [ $usefdatasync -ne 0 ] ; then
	echo 1 > $conf_dir/USE_FDATASYNC
fi
if [ $usesyncdir -ne 0 ] ; then
	echo 1 > $conf_dir/USE_SYNCDIR
fi
if [ ! " $spamfilter" = " " ] ; then
	echo "$spamfilter" > $conf_dir/SPAMFILTER
	echo "1" > $conf_dir/MAKE_SEEKABLE
	if [ ! " $spamexitcode" = " " ] ; then
		echo "$spamexitcode" > $conf_dir/SPAMEXITCODE
		if [ ! " $rejectspam" = " " ] ; then
			echo "$rejectspam" > $conf_dir/REJECTSPAM
		else
			> $conf_dir/REJECTSPAM
		fi
	else
		> $conf_dir/SPAMEXITCODE
	fi
else
	> $conf_dir/SPAMFILTER
fi
if [ " $min_free" = " " ] ; then
	echo 52428800 > $conf_dir/MIN_FREE
else
	echo $min_free > $conf_dir/MIN_FREE
fi
/bin/rm -f $conf_dir/QUEUEDIR
case "$dksign_option" in
	dkim)
	/bin/rm -f $conf_dir/DKIMSIGN $conf_dir/DKIMVERIFY $conf_dir/DKIMQUEUE
	echo "$private_key" > $conf_dir/DKIMSIGN
	if [ -n "$spamfilter" ] ; then
		echo "$QmailBinPrefix/sbin/qmail-spamfilter $QmailBinPrefix/sbin/qmail-dkim" \
			> $conf_dir/QMAILQUEUE
	else
		echo "$QmailBinPrefix/sbin/qmail-dkim" > $conf_dir/QMAILQUEUE
	fi
	if [ $no_multi -eq 1 ] ; then
		echo "$QUEUE_BASE"/queue1 > $conf_dir/QUEUEDIR
	fi
	;;
	none|*)
	/bin/rm -f $conf_dir/DKIMSIGN $conf_dir/DKIMVERIFY $conf_dir/DKIMQUEUE
	if [ $no_multi -eq 1 ] ; then
		echo "$QUEUE_BASE"/queue1 > $conf_dir/QUEUEDIR
		if [ -n "$spamfilter" ] ; then
			echo "$QmailBinPrefix/sbin/qmail-spamfilter" > $conf_dir/QMAILQUEUE
		else
			echo $qmailqueue > $conf_dir/QMAILQUEUE
		fi
	else
		> $conf_dir/QUEUEDIR
		if [ -n "$spamfilter" ] ; then
			echo "$QmailBinPrefix/sbin/qmail-spamfilter" > $conf_dir/QMAILQUEUE
		else
			echo "$QmailBinPrefix/sbin/qmail-multi" > $conf_dir/QMAILQUEUE
		fi
	fi
	;;
esac
if [ $hidehost -eq 1 ] ; then
	echo 1 > $conf_dir/HIDE_HOST
else
	> $conf_dir/HIDE_HOST
fi
if [ $death -eq 86400 ] ; then
	/bin/rm -f "$DESTDIR"$sysconfdir/control/global_vars/DEATH \
		"$DESTDIR"$sysconfdir/control/global_vars/OSSIFIED
else
	if [ ! -f "$DESTDIR"$sysconfdir/control/global_vars/DEATH ] ; then
		ossified=$(echo $death | awk '{printf("%.0f\n", $1 + $1 * 0.5)}')
		echo $death   > "$DESTDIR"$sysconfdir/control/global_vars/DEATH
		echo $ossfied > "$DESTDIR"$sysconfdir/control/global_vars/OSSIFIED
	else
		t=$(@prefix@/bin/qcat "$DESTDIR"$sysconfdir/control/global_vars/DEATH)
		if [ "$t" != "$death" ] ; then
			ossified=$(echo $death | awk '{printf("%.0f\n", $1 + $1 * 0.5)}')
			echo $death > "$DESTDIR"$sysconfdir/control/global_vars/DEATH
			echo $ossfied > "$DESTDIR"$sysconfdir/control/global_vars/OSSIFIED
		fi
	fi
fi
echo "$prog_args" > $conf_dir/.options
echo "$prog_args" > "$DESTDIR"$cntrldir/global_vars/.options
if [ ! " $logfilter" = " " ] ; then
	echo  $logfilter > $conf_dir/LOGFILTER
	if [ ! -f $SERVICEDIR/qmail-logfifo/run ] ; then
		prog_args="$QmailBinPrefix/sbin/svctool --fifologger=\"$logfilter\" --servicedir=\"$SERVICEDIR\""
		create_fifologger $logfilter $SERVICEDIR
	fi
else
	> $conf_dir/LOGFILTER
fi
chmod 644 $conf_dir/*
$chown -R indimail:qmail $conf_dir
}

create_qmtp_or_qmqp()
{
if [ $# -ne 6 ] ; then
	echo "USAGE: create_qmtp_or_qmqp qmtpd|qmqpd qbase queue_count first_queue_no supervise_dir port" 1>&2
	return 1
elif [ " $1" != " qmtpd" -a " $1" != " qmqpd" ] ; then
	echo "USAGE: create_qmtp_or_qmqp qmtpd|qmqpd qbase queue_count first_queue_no supervise_dir port" 1>&2
	return 1
fi
type=$1
QUEUE_BASE=$2
QUEUE_COUNT=$3
QUEUE_START=$4
SERVICEDIR=$5
QMTP_PORT=$6

# qmail-qm[t|q]pd script
if [ " $servicetag" = " " ] ; then
	tag=$QMTP_PORT
else
	tag=$servicetag
fi
if [ $nooverwrite -eq 1 -a -d "$DESTDIR"$SERVICEDIR/qmail-$type.$tag/variables ] ; then
	return 0
fi
conf_dir="$DESTDIR"$SERVICEDIR/qmail-$type.$tag/variables
if [ $run_file_only -eq 1 ] ; then
	if [ -d $conf_dir ] ; then
		/bin/mv $conf_dir "$conf_dir".bak
	fi
fi
/bin/mkdir -p $conf_dir
$chown root:indimail $conf_dir
/bin/chmod 550 $conf_dir
link_with_global $conf_dir

/bin/mkdir -p "$DESTDIR"$SERVICEDIR/qmail-$type.$tag/log
if [ $down_state -eq 1 ] ; then
	$TOUCH "$DESTDIR"$SERVICEDIR/qmail-$type.$tag/down
fi

if [ " $ipaddress" = " " ] ; then
	echo 0 > $conf_dir/LOCALIP
else
	echo $ipaddress > $conf_dir/LOCALIP
fi
echo $QMTP_PORT > $conf_dir/PORT
if [ ! " $maxdaemons" = " " ] ; then
	echo "$maxdaemons" > $conf_dir/MAXDAEMONS
else
	echo $CONCURRENCYINCOMING > $conf_dir/MAXDAEMONS
fi
if [ ! " $maxperip" = " " ] ; then
	echo "$maxperip" > $conf_dir/MAXPERIP
else
	echo 10 > $conf_dir/MAXPERIP
fi
if [ ! " $CONTROLDIR" = " " ] ; then
	echo "$CONTROLDIR" > $conf_dir/CONTROLDIR
else
	> $conf_dir/CONTROLDIR
fi
if [ ! " $memory" = " " ] ; then
	echo $memory > $conf_dir/SOFT_MEM
else
	echo 52428800 > $conf_dir/SOFT_MEM
fi
if [ $no_multi -eq 1 ] ; then
	echo $qmailqueue > $conf_dir/QMAILQUEUE
	echo "$QUEUE_BASE"/queue1 > $conf_dir/QUEUEDIR
else
	> $conf_dir/QUEUEDIR
	for i in QUEUE_BASE QUEUE_COUNT QUEUE_START
	do
		/bin/rm -f $conf_dir/$i
	done
	t=$("$DESTDIR"$QmailBinPrefix/bin/envdir $conf_dir env 2>/dev/null | grep QUEUE_BASE | cut -d= -f2)
	if [ ! "$t" = "$QUEUE_BASE" ] ; then
		echo $QUEUE_BASE   > $conf_dir/QUEUE_BASE
	fi
	t=$("$DESTDIR"$QmailBinPrefix/bin/envdir $conf_dir env 2>/dev/null | grep QUEUE_COUNT | cut -d= -f2)
	if [ ! "$t" = "$QUEUE_COUNT" ] ; then
		echo $QUEUE_COUNT > $conf_dir/QUEUE_COUNT
	fi
	t=$("$DESTDIR"$QmailBinPrefix/bin/envdir $conf_dir env 2>/dev/null | grep QUEUE_START | cut -d= -f2)
	if [ ! "$t" = "$QUEUE_START" ] ; then
		echo $QUEUE_START  > $conf_dir/QUEUE_START
	fi
fi
case "$dkverify_option" in
	dkimstrict)
	/bin/rm -f $conf_dir/DKIMSIGN $conf_dir/DKIMVERIFY $conf_dir/DKIMQUEUE
	echo FGHIKLMNOQRTUVWjp > $conf_dir/DKIMVERIFY
	> $conf_dir/UNSIGNED_SUBJECT
	> $conf_dir/UNSIGNED_FROM
	if [ -n "$spamfilter" ] ; then
		echo "$QmailBinPrefix/sbin/qmail-spamfilter $QmailBinPrefix/sbin/qmail-dkim" \
			> $conf_dir/QMAILQUEUE
	else
		echo "$QmailBinPrefix/sbin/qmail-dkim" > $conf_dir/QMAILQUEUE
	fi
	if [ $no_multi -eq 1 ] ; then
		echo "$QUEUE_BASE"/queue1 > $conf_dir/QUEUEDIR
	fi
	;;
	dkim)
	/bin/rm -f $conf_dir/DKIMSIGN $conf_dir/DKIMVERIFY $conf_dir/DKIMQUEUE
	echo > $conf_dir/UNSIGNED_SUBJECT
	> $conf_dir/UNSIGNED_FROM
	echo > $conf_dir/DKIMVERIFY
	if [ -n "$spamfilter" ] ; then
		echo "$QmailBinPrefix/sbin/qmail-spamfilter $QmailBinPrefix/sbin/qmail-dkim" \
			> $conf_dir/QMAILQUEUE
	else
		echo "$QmailBinPrefix/sbin/qmail-dkim" > $conf_dir/QMAILQUEUE
	fi
	if [ $no_multi -eq 1 ] ; then
		echo "$QUEUE_BASE"/queue1 > $conf_dir/QUEUEDIR
	fi
	;;
	none|*)
	/bin/rm -f $conf_dir/DKIMSIGN $conf_dir/DKIMVERIFY $conf_dir/DKIMQUEUE
	if [ $no_multi -eq 1 ] ; then
		echo "$QUEUE_BASE"/queue1 > $conf_dir/QUEUEDIR
		if [ -n  "$spamfilter" ] ; then
			echo "$QmailBinPrefix/sbin/qmail-spamfilter" > $conf_dir/QMAILQUEUE
		else
			echo $qmailqueue > $conf_dir/QMAILQUEUE
		fi
	else
		> $conf_dir/QUEUEDIR
		if [ -n "$spamfilter" ] ; then
			echo "$QmailBinPrefix/sbin/qmail-spamfilter" > $conf_dir/QMAILQUEUE
		else
			echo "$QmailBinPrefix/sbin/qmail-multi" > $conf_dir/QMAILQUEUE
		fi
	fi
	;;
esac
if [ $usefsync -ne 0 ] ; then
	echo 1 > $conf_dir/USE_FSYNC
fi
if [ $usefdatasync -ne 0 ] ; then
	echo 1 > $conf_dir/USE_FDATASYNC
fi
if [ $usesyncdir -ne 0 ] ; then
	echo 1 > $conf_dir/USE_SYNCDIR
fi
if [ " $min_free" = " " ] ; then
	echo 52428800 > $conf_dir/MIN_FREE
else
	echo $min_free > $conf_dir/MIN_FREE
fi
if [ " $min_free" = " " ] ; then
	echo 52428800 > $conf_dir/MIN_FREE
else
	echo $min_free > $conf_dir/MIN_FREE
fi
if [ " $qhpsi" = " " ] ; then
	> $conf_dir/QHPSI
else
	echo $qhpsi > $conf_dir/QHPSI
fi
if [ ! " $spamfilter" = " " ] ; then
	echo "$spamfilter" > $conf_dir/SPAMFILTER
	echo "1" > $conf_dir/MAKE_SEEKABLE
	if [ ! " $spamexitcode" = " " ] ; then
		echo "$spamexitcode" > $conf_dir/SPAMEXITCODE
		if [ ! " $rejectspam" = " " ] ; then
			echo "$rejectspam" > $conf_dir/REJECTSPAM
		else
			> $conf_dir/REJECTSPAM
		fi
	else
		> $conf_dir/SPAMEXITCODE
	fi
else
	> $conf_dir/SPAMFILTER
fi
if [ " $QmailBinPrefix" = " /usr" ] ; then
	echo "/bin:/usr/bin:/usr/sbin:/sbin" > $conf_dir/PATH
else
	echo "/bin:/usr/bin:$QmailBinPrefix/bin:$QmailBinPrefix/sbin" > $conf_dir/PATH
fi
if [ "$type" = "qmtpd" ] ; then
	if [ $hidehost -eq 1 ] ; then
		echo 1 > $conf_dir/HIDE_HOST
	else
		> $conf_dir/HIDE_HOST
	fi
else
	> $conf_dir/HIDE_HOST
fi
(
dump_run_header
echo "MYUID=\`/usr/bin/id -u qmaild\`"
echo "MYGID=\`/usr/bin/id -g qmaild\`"
echo "ME=\`head -1 $CONTROLDIR/me\`"
echo "HOSTNAME=\`uname -n\`"
echo
echo "if [ -z \"\$MYUID\" -o -z \"\$MYGID\" -o -z \"\$ME\" ]; then"
echo "    echo UID, GID, or ME is unset in"
echo "    echo \`pwd\`/run"
echo "    sleep 5"
echo "    exit 1"
echo "fi"
echo
echo "exec 2>&1"
echo "exec $QmailBinPrefix/bin/envdir $envdir_opts variables sh -c \""
echo "exec $QmailBinPrefix/bin/softlimit -m \\\$SOFT_MEM -o 1024 \\"
echo "$QmailBinPrefix/bin/tcpserver -v -H -R -l \$HOSTNAME \\"
case $type in
	qmtpd)
	echo "-x $sysconfdir/tcp/tcp.qmtp.cdb \\"
	;;
	qmqpd)
	echo "-x $sysconfdir/tcp/tcp.qmqp.cdb \\"
	;;
esac
echo "-c ./variables/MAXDAEMONS -o -b \\\$MAXDAEMONS \\"
echo "-u \$MYUID -g \$MYGID \\\$LOCALIP \\\$PORT \\"
echo "$QmailBinPrefix/sbin/qmail-$type\""
) > "$DESTDIR"$SERVICEDIR/qmail-$type.$tag/run
/bin/chmod +x "$DESTDIR"$SERVICEDIR/qmail-$type.$tag/run

# qmail-qm[t|q]pd log script
if [ ! -d $LOGDIR ] ; then
	/bin/mkdir -p $LOGDIR
	$chown qmaill:nofiles $LOGDIR
fi
dump_log_header $type.$tag > "$DESTDIR"$SERVICEDIR/qmail-$type.$tag/log/run
/bin/chmod +x "$DESTDIR"$SERVICEDIR/qmail-$type.$tag/log/run
echo "$prog_args" > $conf_dir/.options

if [ $run_file_only -eq 1 ] ; then
	if [ -d "$conf_dir".bak ] ; then
		/bin/rm -rf $conf_dir
		/bin/mv "$conf_dir".bak $conf_dir
	fi
fi

if [ ! " $logfilter" = " " ] ; then
	echo  $logfilter > $conf_dir/LOGFILTER
	if [ ! -f $SERVICEDIR/qmail-logfifo/run ] ; then
		prog_args="$QmailBinPrefix/sbin/svctool --fifologger=\"$logfilter\" --servicedir=\"$SERVICEDIR\""
		create_fifologger $logfilter $SERVICEDIR
	fi
else
	> $conf_dir/LOGFILTER
fi
}

create_udplogger()
{
if [ $# -ne 2 ] ; then
	echo "USAGE: create_udplogger udplogger_port supervise_dir" 1>&2
	return 1
fi
UDP_PORT=$1
SERVICEDIR=$2
if [ " $servicetag" = " " ] ; then
	tag=$UDP_PORT
else
	tag=$servicetag
fi
if [ $nooverwrite -eq 1 -a -d "$DESTDIR"$SERVICEDIR/udplogger.$tag/variables ] ; then
	return 0
fi
conf_dir="$DESTDIR"$SERVICEDIR/udplogger.$tag/variables
if [ $run_file_only -eq 1 ] ; then
	if [ -d $conf_dir ] ; then
		/bin/mv $conf_dir "$conf_dir".bak
	fi
fi
/bin/mkdir -p $conf_dir
/bin/chmod 500 $conf_dir
link_with_global $conf_dir

/bin/mkdir -p "$DESTDIR"$SERVICEDIR/udplogger.$tag/log
if [ $down_state -eq 1 ] ; then
	$TOUCH "$DESTDIR"$SERVICEDIR/udplogger.$tag/down
fi
if [ " $ipaddress" = " " ] ; then
	echo 127.0.0.1 > $conf_dir/LOCALIP
else
	echo $ipaddress > $conf_dir/LOCALIP
fi
if [ " $timeout" = " " ] ; then
	echo "60" > $conf_dir/TIMEOUT
else
	echo $timeout > $conf_dir/TIMEOUT
fi
echo $UDP_PORT > $conf_dir/PORT
if [ ! " $CONTROLDIR" = " " ] ; then
	echo "$CONTROLDIR" > $conf_dir/CONTROLDIR
else
	> $conf_dir/CONTROLDIR
fi
if [ ! " $memory" = " " ] ; then
	echo $memory > $conf_dir/SOFT_MEM
else
	echo 52428800 > $conf_dir/SOFT_MEM
fi

(
dump_run_header
echo "exec 2>&1"
echo "exec $QmailBinPrefix/bin/envdir $envdir_opts variables sh -c \""
echo "exec $QmailBinPrefix/bin/setuidgid indimail \\"
echo "$QmailBinPrefix/bin/softlimit -m \\\$SOFT_MEM -o 1024 \\"
echo " $QmailBinPrefix/sbin/udplogger -p \\\$PORT -t \\\$TIMEOUT \\\$LOCALIP\""
) > "$DESTDIR"$SERVICEDIR/udplogger.$tag/run
/bin/chmod +x "$DESTDIR"$SERVICEDIR/udplogger.$tag/run

# udplogger log script
if [ ! -d $LOGDIR ] ; then
	/bin/mkdir -p $LOGDIR
	$chown qmaill:nofiles $LOGDIR
fi
dump_log_header udplogger.$tag > "$DESTDIR"$SERVICEDIR/udplogger.$tag/log/run
/bin/chmod +x "$DESTDIR"$SERVICEDIR/udplogger.$tag/log/run
echo "$prog_args" > $conf_dir/.options

if [ $run_file_only -eq 1 ] ; then
	if [ -d "$conf_dir".bak ] ; then
		/bin/rm -rf $conf_dir
		/bin/mv "$conf_dir".bak $conf_dir
	fi
fi
}

create_dane()
{
if [ $# -ne 7 ] ; then
	echo "USAGE: create_dane supervise_dir port timeout_days context_file save_interval hash_size whitelist" 1>&2
	return 1
fi
SERVICEDIR=$1
DANE_PORT=$2
TIMEOUT_DAYS=$3
CONTEXT_FILE=$4
SAVE_INTERVAL=$5
HASH_SIZE=$6
WHITELIST=$7
if [ " $DANE_PORT" = " " ] ; then
	DANE_PORT=1998
fi
if [ " $servicetag" = " " ] ; then
	tag=$DANE_PORT
else
	tag=$servicetag
fi
if [ $nooverwrite -eq 1 -a -d "$DESTDIR"$SERVICEDIR/qmail-daned.$tag/variables ] ; then
	return 0
fi
conf_dir="$DESTDIR"$SERVICEDIR/qmail-daned.$tag/variables
if [ $run_file_only -eq 1 ] ; then
	if [ -d $conf_dir ] ; then
		/bin/mv $conf_dir "$conf_dir".bak
	fi
fi
/bin/mkdir -p $conf_dir
/bin/chmod 500 $conf_dir
link_with_global $conf_dir

/bin/mkdir -p "$DESTDIR"$SERVICEDIR/qmail-daned.$tag/log
if [ $down_state -eq 1 ] ; then
	$TOUCH "$DESTDIR"$SERVICEDIR/qmail-daned.$tag/down
fi
if [ " $ipaddress" = " " ] ; then
	echo 127.0.0.1 > $conf_dir/LOCALIP
else
	echo $ipaddress > $conf_dir/LOCALIP
fi
if [ ! " $CONTROLDIR" = " " ] ; then
	echo "$CONTROLDIR" > $conf_dir/CONTROLDIR
else
	> $conf_dir/CONTROLDIR
fi
echo $DANE_PORT > $conf_dir/PORT
if [ ! " $memory" = " " ] ; then
	echo $memory > $conf_dir/SOFT_MEM
else
	echo 52428800 > $conf_dir/SOFT_MEM
fi
echo "$libexecdir/daneprog" > $conf_dir/DANEPROG
WHITELIST=$CONTROLDIR/$WHITELIST
CONTEXT_FILE=$CONTROLDIR/$CONTEXT_FILE

(
dump_run_header
echo "exec 2>&1"
echo "exec $QmailBinPrefix/bin/envdir $envdir_opts variables sh -c \""
echo "exec $QmailBinPrefix/bin/setuidgid indimail \\"
echo "  $QmailBinPrefix/sbin/qmail-daned -w $WHITELIST -t $TIMEOUT_DAYS \\"
echo "  -s $SAVE_INTERVAL -h $HASH_SIZE \\\$LOCALIP $CONTEXT_FILE\""
) > "$DESTDIR"$SERVICEDIR/qmail-daned.$tag/run
/bin/chmod +x "$DESTDIR"$SERVICEDIR/qmail-daned.$tag/run

# qmail-daned log script
if [ ! -d $LOGDIR ] ; then
	/bin/mkdir -p $LOGDIR
	$chown qmaill:nofiles $LOGDIR
fi
dump_log_header daned.$tag > "$DESTDIR"$SERVICEDIR/qmail-daned.$tag/log/run
/bin/chmod +x "$DESTDIR"$SERVICEDIR/qmail-daned.$tag/log/run
echo "$prog_args" > $conf_dir/.options

if [ $run_file_only -eq 1 ] ; then
	if [ -d "$conf_dir".bak ] ; then
		/bin/rm -rf $conf_dir
		/bin/mv "$conf_dir".bak $conf_dir
	fi
fi
}

create_greylist()
{
if [ $# -ne 10 ] ; then
	echo "USAGE: create_greylist supervise_dir port min_resend_min resend_win_hr timeout_days context_file save_interval hash_size whitelist use-greydaemon" 1>&2
	return 1
fi
SERVICEDIR=$1
GREY_PORT=$2
MIN_RESEND_MIN=$3
RESEND_WIN_HR=$4
TIMEOUT_DAYS=$5
CONTEXT_FILE=$6
SAVE_INTERVAL=$7
HASH_SIZE=$8
WHITELIST=$9
shift
GREYDAEMON="$9"
if [ " $GREY_PORT" = " " ] ; then
	GREY_PORT=1999
fi
if [ " $servicetag" = " " ] ; then
	tag=$GREY_PORT
else
	tag=$servicetag
fi
if [ $nooverwrite -eq 1 -a -d "$DESTDIR"$SERVICEDIR/greylist.$tag/variables ] ; then
	return 0
fi
conf_dir="$DESTDIR"$SERVICEDIR/greylist.$tag/variables
if [ $run_file_only -eq 1 ] ; then
	if [ -d $conf_dir ] ; then
		/bin/mv $conf_dir "$conf_dir".bak
	fi
fi
/bin/mkdir -p $conf_dir
/bin/chmod 500 $conf_dir
link_with_global $conf_dir

/bin/mkdir -p "$DESTDIR"$SERVICEDIR/greylist.$tag/log
if [ $down_state -eq 1 ] ; then
	$TOUCH "$DESTDIR"$SERVICEDIR/greylist.$tag/down
fi
if [ " $ipaddress" = " " ] ; then
	echo 127.0.0.1 > $conf_dir/LOCALIP
else
	echo $ipaddress > $conf_dir/LOCALIP
fi
echo $GREY_PORT > $conf_dir/PORT
if [ ! " $CONTROLDIR" = " " ] ; then
	echo "$CONTROLDIR" > $conf_dir/CONTROLDIR
else
	> $conf_dir/CONTROLDIR
fi
if [ ! " $memory" = " " ] ; then
	echo $memory > $conf_dir/SOFT_MEM
else
	echo 52428800 > $conf_dir/SOFT_MEM
fi
if [ " $GREYDAEMON" = " " ] ; then
	GREYPROG=$QmailBinPrefix/sbin/qmail-greyd
else
	GREYPROG=$QmailBinPrefix/sbin/greydaemon
fi
WHITELIST=$CONTROLDIR/$WHITELIST
CONTEXT_FILE=$CONTROLDIR/$CONTEXT_FILE

(
dump_run_header
echo "exec 2>&1"
echo "exec $QmailBinPrefix/bin/envdir $envdir_opts variables sh -c \""
echo "exec $QmailBinPrefix/bin/setuidgid indimail \\"
echo "$QmailBinPrefix/bin/softlimit -m \\\$SOFT_MEM -o 1024 \\"
echo " $GREYPROG -w $WHITELIST -t $TIMEOUT_DAYS -g $RESEND_WIN_HR \\"
if [ " $GREYDAEMON" = " " ] ; then
echo "	-m $MIN_RESEND_MIN -s $SAVE_INTERVAL -h $HASH_SIZE \\\$LOCALIP $CONTEXT_FILE\""
else
echo "	-m $MIN_RESEND_MIN -s $SAVE_INTERVAL \\\$LOCALIP $CONTEXT_FILE\""
fi
) > "$DESTDIR"$SERVICEDIR/greylist.$tag/run
/bin/chmod +x "$DESTDIR"$SERVICEDIR/greylist.$tag/run

# greylist log script
if [ ! -d $LOGDIR ] ; then
	/bin/mkdir -p $LOGDIR
	$chown qmaill:nofiles $LOGDIR
fi
dump_log_header greylist.$tag > "$DESTDIR"$SERVICEDIR/greylist.$tag/log/run
/bin/chmod +x "$DESTDIR"$SERVICEDIR/greylist.$tag/log/run
echo "$prog_args" > $conf_dir/.options

if [ $run_file_only -eq 1 ] ; then
	if [ -d "$conf_dir".bak ] ; then
		/bin/rm -rf $conf_dir
		/bin/mv "$conf_dir".bak $conf_dir
	fi
fi
}

create_courier()
{
if [ $# -ne 7 ] ; then
	echo "USAGE: create_courier supervise_dir type port nolastauth=0|1 legacyserver=0|1 use_ssl=0|1 infifo=fifo_path" 1>&2
	return 1
fi
SERVICEDIR=$1
stype=$2
COURIER_PORT=$3
nolastauth=$4
legacyserver=$5
imap_ssl=$6
infifo=$7

if [ " $servicetag" = " " ] ; then
	tag=$COURIER_PORT
else
	tag=$servicetag
fi
if [ " $stype" = " pop3d" ] ; then
	cdb_f="pop3"
elif [ " $stype" = " imapd" ] ; then
	cdb_f="imap"
else
	echo "USAGE: create_courier supervise_dir type port nolastauth=0|1 legacyserver=0|1 use_ssl=0|1 infifo=fifo_path" 1>&2
	return 1
fi
if [ " $proxy_port" = " " ] ;then
	proxy_type="qmail"
else
	proxy_type="proxy"
fi
if [ " $imap_ssl" = " 1" ] ; then
	if [ $nooverwrite -eq 1 -a -d "$DESTDIR"$SERVICEDIR/"$proxy_type"-$stype-ssl.$tag/variables ] ; then
		return 0
	fi
	/bin/mkdir -p "$DESTDIR"$SERVICEDIR/"$proxy_type"-$stype-ssl.$tag/log
	conf_dir="$DESTDIR"$SERVICEDIR/"$proxy_type"-$stype-ssl.$tag/variables
else
	if [ $nooverwrite -eq 1 -a -d "$DESTDIR"$SERVICEDIR/"$proxy_type"-$stype.$tag/variables ] ; then
		return 0
	fi
	/bin/mkdir -p "$DESTDIR"$SERVICEDIR/"$proxy_type"-$stype.$tag/log
	conf_dir="$DESTDIR"$SERVICEDIR/"$proxy_type"-$stype.$tag/variables
fi
if [ $down_state -eq 1 ] ; then
	$TOUCH "$DESTDIR"$SERVICEDIR/"$proxy_type"-$stype.$tag/down
fi
mkdir -p $conf_dir
$chown root:indimail $conf_dir
/bin/chmod 550 $conf_dir
link_with_global $conf_dir

if [ -d /run ] ; then
	fifo_tmpdir="/run/indimail/inquery"
elif [ -d /var/run ] ; then
	fifo_tmpdir="/var/run/indimail/inquery"
else
	fifo_tmpdir="/tmp/indimail/inquery"
fi
if [ $run_file_only -ne 1 ] ; then
	if [ " $imap_ssl" = " 1" ] ; then
		# /service/[qmail|proxy]-[pop3d|imapd]-ssl.nnn/variables
		(
		create_"$cdb_f"_config $COURIER_PORT $infifo
		create_"$cdb_f"_config_ssl
		) > $conf_dir/.courier_variables
	else
		# /service/[qmail|proxy]-[pop3d|imapd].nnn/variables
		# this will have all variables of "$cdb_f"_config_ssl also
		create_"$cdb_f"_config $COURIER_PORT $infifo > $conf_dir/.courier_variables
	fi
	echo $fifo_tmpdir > $conf_dir/FIFOTMPDIR
	echo "0660" > $conf_dir/FIFO_MODE
	if [ ! " $memory" = " " ] ; then
		echo $memory > $conf_dir/SOFT_MEM
	else
		echo 5242880 > $conf_dir/SOFT_MEM
	fi
	if [ ! " $domainlimits" = " " ] ; then
		echo  > $conf_dir/DOMAIN_LIMITS
	else
		> $conf_dir/DOMAIN_LIMITS
	fi
	if [ " $query_cache" = " " ] ; then
		> $conf_dir/QUERY_CACHE
	else
		echo > $conf_dir/QUERY_CACHE
	fi
	old=$force
	force=1
	import_variables $conf_dir $conf_dir/.courier_variables 0 root indimail 640
	export_variables $conf_dir $conf_dir/.variables root indimail 640
	force=$old
fi
(
dump_run_header
echo "HOSTNAME=\`uname -n\`"
echo ""
echo "prefix=$QmailBinPrefix"
echo "bindir=\${prefix}/bin"
echo "libexecdir=${libexecdir}"
echo ""
echo "if [ ! -d $fifo_tmpdir ] ; then"
echo "  mkdir -p $fifo_tmpdir"
echo "  chmod 2770 $fifo_tmpdir"
echo "  $chown qmaild:indimail $fifo_tmpdir"
echo "fi"
echo "exec 2>&1"
echo "exec $QmailBinPrefix/bin/envdir $envdir_opts variables sh -c \""
echo "IMAPAUTHMODULES=\\\"\\\""
echo "for f in \\\`echo \\\$IMAPMODULES\\\`"
echo "do"
echo "	IMAPAUTHMODULES=\\\"\\\$IMAPAUTHMODULES \$libexecdir/imapmodules/\\\$f\\\""
echo "done"
echo "exec $QmailBinPrefix/bin/softlimit -m \\\$SOFT_MEM -o 1024 \\"
echo "$QmailBinPrefix/bin/tcpserver -v -c ./variables/MAXDAEMONS -C \\\$MAXPERIP \\"
echo "-x $sysconfdir/tcp/tcp.$cdb_f.cdb -X \\"
if [ " $imap_ssl" = " 1" ] ; then
	echo "-o -b \\\$MAXDAEMONS -H -l \$HOSTNAME -R -u indimail -g indimail,qmail,qcerts \\\$SSLADDRESS \\\$SSLPORT \\"
	echo "\$prefix/bin/couriertls -server -tcpd \\"
else
	echo "-o -b \\\$MAXDAEMONS -H -l \$HOSTNAME -R -u indimail -g indimail,qmail,qcerts \\\$LOCALIP \\\$PORT \\"
fi
if [ " $proxy_port" = " " ] ;then
	echo "\$prefix/sbin/"$cdb_f"login \\\$IMAPAUTHMODULES \$bindir/$stype Maildir\""
else
	echo "$QmailBinPrefix/bin/proxy"$cdb_f" \$bindir/$stype Maildir\""
fi
) > $TMPDIR/courier-run.$$

if [ " $imap_ssl" = " 1" ] ; then
	/bin/mv $TMPDIR/courier-run.$$ "$DESTDIR"$SERVICEDIR/"$proxy_type"-$stype-ssl.$tag/run
	/bin/chmod +x "$DESTDIR"$SERVICEDIR/"$proxy_type"-$stype-ssl.$tag/run
else
	/bin/mv $TMPDIR/courier-run.$$ "$DESTDIR"$SERVICEDIR/"$proxy_type"-$stype.$tag/run
	/bin/chmod +x "$DESTDIR"$SERVICEDIR/"$proxy_type"-$stype.$tag/run
fi

if [ ! -d $LOGDIR ] ; then
	/bin/mkdir -p $LOGDIR
	$chown qmaill:nofiles $LOGDIR
fi
# qmail-[imapd|pop3d] log script
(
if [ " $proxy_port" = " " ] ;then
	if [ " $imap_ssl" = " 1" ] ; then
		dump_log_header $stype-ssl.$tag
	else
		dump_log_header $stype.$tag
	fi
else
	if [ " $imap_ssl" = " 1" ] ; then
		dump_log_header proxy"`echo $cdb_f|tr '[:lower:]' '[:upper:]'`"-ssl.$tag
	else
		dump_log_header proxy"`echo $cdb_f|tr '[:lower:]' '[:upper:]'`".$tag
	fi
fi
) > $TMPDIR/courierlog-run.$$
if [ " $imap_ssl" = " 1" ] ; then
	/bin/mv $TMPDIR/courierlog-run.$$ "$DESTDIR"$SERVICEDIR/"$proxy_type"-$stype-ssl.$tag/log/run
	/bin/chmod +x "$DESTDIR"$SERVICEDIR/"$proxy_type"-$stype-ssl.$tag/log/run
else
	/bin/mv $TMPDIR/courierlog-run.$$ "$DESTDIR"$SERVICEDIR/"$proxy_type"-$stype.$tag/log/run
	/bin/chmod +x "$DESTDIR"$SERVICEDIR/"$proxy_type"-$stype.$tag/log/run
fi
echo "$prog_args" > $conf_dir/.options
}

create_imap_config()
{
echo "#--------- IndiMail ---------------------------"
if [ -n "$default_domain" ] ; then
	if [ -f $sysconfdir/control/global_vars/DEFAULT_DOMAIN ] ; then
		t=$(@prefix@/bin/qcat $sysconfdir/control/global_vars/DEFAULT_DOMAIN)
		if [ ! "$t" = "$default_domain" ] ; then
			echo "DEFAULT_DOMAIN=$default_domain"
		fi
	else
		echo "DEFAULT_DOMAIN=$default_domain"
	fi
	if [ -f $sysconfdir/control/global_vars/QMAILDEFAULTHOST ] ; then
		t=$(@prefix@/bin/qcat $sysconfdir/control/global_vars/QMAILDEFAULTHOST)
		if [ ! "$t" = "$default_domain" ] ; then
			echo "QMAILDEFAULTHOST=$default_domain" # Used if mail is stored in outbox
		fi
	else
		echo "QMAILDEFAULTHOST=$default_domain" # Used if mail is stored in outbox
	fi
fi
echo "MIGRATEUSER=$QmailBinPrefix/bin/migrateuser"
echo "MIGRATEFLAG=indi.txt"
echo "TCP_FILE=$sysconfdir/tcp/tcp.imap"
echo "OPEN_SMTP=$sysconfdir/tcp/open-smtp"
echo "RELAY_TABLE=relay"
echo "MCDFILE=mcdinfo"
IMAP_PORT=$1
if [ " $IMAP_PORT" = " " ] ; then
	echo "PORT=143"
else
	echo "PORT=$IMAP_PORT"
fi
if [ -d /run ] ; then
	rundir=/run/indimail
elif [ -d /var/run ] ; then
	rundir=/var/run/indimail
else
	rundir=/tmp/indimail
fi
fifo_name=`basename $2`
infifo_dir=`dirname $2`
if [ $infifo_dir = "." ] ; then
	infifo_dir=$rundir/inlookup
fi
echo "INFIFO=$fifo_name"
echo "INFIFODIR=$infifo_dir"

if [ " $ipaddress" = " " ] ; then
	echo "LOCALIP=0"
else
	echo "LOCALIP=$ipaddress"
fi
if [ ! " $nolastauth" = " " ] ; then
	echo "NOLASTAUTHLOGGING=1"
else
	echo "NOLASTAUTHLOGGING="
fi
if [ ! " $proxy_port" = " " ] ;then
	echo "#--------- ADDED FOR Proxy IMAP ---------------"
	echo "ADMIN_HOST=localhost"
	echo "ADMIN_PORT=4000"
	echo "ADMIN_USER=admin"
	echo "ADMIN_PASS=$ADMIN_PASS"
	echo "HARD_QUOTA=$hard_quota"
	echo "DATA_TIMEOUT=1"
	echo "DESTPORT=imap:$proxy_port"
	if [ ! " $legacyserver" = " " ] ; then
		echo "LEGACY_SERVER=1"
	else
		echo "LEGACY_SERVER="
	fi
else
	echo "DESTPORT="
fi
echo "#----------------------------------------------"
echo "MIN_LOGIN_INTERVAL=0"
if [ ! " $maxdaemons" = " " ] ; then
	echo "MAXDAEMONS=$maxdaemons"
else
	echo "MAXDAEMONS=$CONCURRENCYINCOMING"
fi
if [ ! " $maxperip" = " " ] ; then
	echo "MAXPERIP=$maxperip"
else
	echo "MAXPERIP=25"
fi
MODULES=""
for i in `ls $DESTDIR"$libexecdir"/imapmodules`
do
	if [ " $i" = " authgeneric" -o " $i" = " authtest" -o " $i" = " authpam" ] ; then
		continue
	fi
	if [ -n "$MODULES" ] ; then
		MODULES="$MODULES $i"
	else
		MODULES="$i"
	fi
done
if [ -f $DESTDIR"$libexecdir"/imapmodules/authpam ] ; then
	MODULES="$MODULES authpam"
fi
echo "IMAPMODULES=\"$MODULES\""
echo "DEBUG_LOGIN=0"
IMAP_CAPABILITY="IMAP4rev1 UIDPLUS CHILDREN NAMESPACE THREAD=ORDEREDSUBJECT THREAD=REFERENCES SORT QUOTA IDLE"
echo "#SMAP_CAPABILITY=SMAP1"
echo "IMAP_KEYWORDS=1"
echo "IMAP_ACL=1"
echo "IMAP_DISABLETHREADSORT=0"
echo "IMAP_CHECK_ALL_FOLDERS=0"
echo "IMAP_OBSOLETE_CLIENT=0"
echo "IMAP_UMASK=022"
echo "IMAP_USELOCKS=0"
echo "IMAP_IDLE_TIMEOUT=60"
echo "IMAP_MAILBOX_SANITY_CHECK=1"
echo "IMAP_TRASHFOLDERNAME=Trash"
echo "IMAP_EMPTYTRASH=Trash:7"
echo "IMAP_MOVE_EXPUNGE_TO_TRASH=0"
echo "IMAP_LOG_DELETIONS=0"
echo "IMAPDEBUGFILE=imap-debug.txt"
echo "OUTBOX=.Outbox"
echo "SENDMAIL=$QmailBinPrefix/bin/sendmail"
echo "HEADERFROM=X-IMAP-Sender"
echo "IMAP_SHAREDINDEXFILE=$sysconfdir/shared/index"
echo "MAILDIRPATH=Maildir"
echo "IMAP_CAPABILITY=\"$IMAP_CAPABILITY AUTH=CRAM-MD5 AUTH=CRAM-SHA1 AUTH=CRAM-SHA256\""
echo "IMAP_CAPABILITY_TLS=\"$IMAP_CAPABILITY AUTH=PLAIN AUTH=LOGIN AUTH=CRAM-MD5 AUTH=CRAM-SHA1 AUTH=CRAM-SHA256\""

# IMAP_STARTTLS, IMAP_TLS_REQUIRED, COURIERTLS required for advertising STARTTLS
if [ $use_starttls -eq 1 ] ; then
	echo "IMAP_STARTTLS=YES"
	echo "IMAP_TLS_REQUIRED=0"
	if [ " $tlsprog" = " " ] ; then
		echo "COURIERTLS=$QmailBinPrefix/bin/couriertls"
	else
		echo "COURIERTLS=$tlsprog"
	fi
	create_imap_config_ssl
fi
}

create_imap_config_ssl()
{
if [ " $IMAP_PORT" = " " ] ; then
	echo "SSLPORT=993"
else
	echo "SSLPORT=$IMAP_PORT"
fi
if [ " $ipaddress" = " " ] ; then
	echo "SSLADDRESS=0"
else
	echo "SSLADDRESS=$ipaddress"
fi
echo "TLS_PROTOCOL=TLSv1.2"
echo "TLS_STARTTLS_PROTOCOL=TLSv1.2"
echo "# TLS_CIPHER_LIST=\"TLSv1:HIGH:!LOW:!MEDIUM:!EXP:!NULL:!aNULL@STRENGTH\""
echo "##NAME: TLS_TIMEOUT:0"
echo "# TLS_TIMEOUT is currently not implemented, and reserved for future use."
echo "# This is supposed to be an inactivity timeout, but its not yet implemented."
echo "#"
echo "TLS_CERTFILE=$sysconfdir/certs/servercert.pem"
echo "TLS_DHPARAMS=$sysconfdir/certs/dhparams.pem"
echo "# TLS_TRUSTCERTS="
echo "TLS_VERIFYPEER=NONE"
echo "TLS_CACHEFILE=$sysconfdir/certs/couriersslcache"
echo "TLS_CACHESIZE=524288"
echo "TLS_ALPN=imap"
echo "IMAP_TLS=1"
}

create_ssl_cnf()
{
if [ $# -ne 2 ] ; then
	echo "USAGE: create_ssl_cnf postmaster IMAP|POP3" 1>&2
	return 1
fi
postmaster=$1
cn=$2
echo
echo "RANDFILE = $sysconfdir/certs/servercert.rand"
echo
echo "[ req ]"
echo "default_bits = 4096"
echo "encrypt_key = yes"
echo "distinguished_name = req_dn"
echo "x509_extensions = cert_type"
echo "prompt = no"
echo "default_md = sha256"
echo
echo "[v3_req]"
echo "subjectAltName = DNS:$cn"
echo
echo "[ req_dn ]"
echo "C=IN"
echo "ST=GOA"
echo "L=Porvorim"
echo "O=IndiMail Server"
echo "OU=Automatically-generated SSL key"
echo "CN=$cn"
echo "emailAddress=$postmaster"
echo
echo "[ cert_type ]"
echo "nsCertType = server"
}

create_imap_stunnel()
{
if [ " $ipaddress" = " " ] ; then
	SSLADDRESS=0
else
	SSLADDRESS=$ipaddress
fi
echo "[imapd]"
echo "accept = $SSLADDRESS:993"
echo "protocol = imap"
echo "connect = $SSLADDRESS:143"
echo "cert = $sysconfdir/certs/servercert.pem"
}

create_pop3_stunnel()
{
if [ " $ipaddress" = " " ] ; then
	SSLADDRESS=0
else
	SSLADDRESS=$ipaddress
fi
echo "[pop3d]"
echo "accept = $SSLADDRESS:995"
echo "protocol = pop3"
echo "connect = $SSLADDRESS:110"
echo "cert = $sysconfdir/certs/servercert.pem"
}

create_pop3_config()
{
echo "#--------- IndiMail ---------------------------"
if [ -n "$default_domain" ] ; then
	if [ -f $sysconfdir/control/global_vars/DEFAULT_DOMAIN ] ; then
		t=$(@prefix@/bin/qcat $sysconfdir/control/global_vars/DEFAULT_DOMAIN)
		if [ ! "$t" = "$default_domain" ] ; then
			echo "DEFAULT_DOMAIN=$default_domain"
		fi
	else
		echo "DEFAULT_DOMAIN=$default_domain"
	fi
	if [ -f $sysconfdir/control/global_vars/QMAILDEFAULTHOST ] ; then
		t=$(@prefix@/bin/qcat $sysconfdir/control/global_vars/QMAILDEFAULTHOST)
		if [ ! "$t" = "$default_domain" ] ; then
			echo "QMAILDEFAULTHOST=$default_domain" # Used if mail is stored in outbox
		fi
	else
		echo "QMAILDEFAULTHOST=$default_domain" # Used if mail is stored in outbox
	fi
fi
echo "MIGRATEUSER=$QmailBinPrefix/bin/migrateuser"
echo "MIGRATEFLAG=indi.txt"
echo "TCP_FILE=$sysconfdir/tcp/tcp.pop3"
echo "OPEN_SMTP=$sysconfdir/tcp/open-smtp"
echo "RELAY_TABLE=relay"
echo "MCDFILE=mcdinfo"
POP3_PORT=$1
if [ " $POP3_PORT" = " " ] ; then
	echo "PORT=110"
else
	echo "PORT=$POP3_PORT"
fi
if [ -d /run ] ; then
	rundir=/run/indimail
elif [ -d /var/run ] ; then
	rundir=/var/run/indimail
else
	rundir=/tmp/indimail
fi
fifo_name=`basename $2`
infifo_dir=`dirname $2`
if [ $infifo_dir = "." ] ; then
	infifo_dir=$rundir/inlookup
fi
echo "INFIFO=$fifo_name"
echo "INFIFODIR=$infifo_dir"

if [ " $ipaddress" = " " ] ; then
	echo "LOCALIP=0"
else
	echo "LOCALIP=$ipaddress"
fi
if [ ! " $nolastauth" = " " ] ; then
	echo "NOLASTAUTHLOGGING=\"\""
else
	echo "NOLASTAUTHLOGGING="
fi
if [ ! " $proxy_port" = " " ] ;then
	echo "#--------- ADDED FOR Proxy POP3 ----------------"
	echo "ADMIN_HOST=localhost"
	echo "ADMIN_PORT=4000"
	echo "ADMIN_USER=admin"
	echo "ADMIN_PASS=$ADMIN_PASS"
	echo "HARD_QUOTA=$hardquota"
	echo "DATA_TIMEOUT=1"
	echo "DESTPORT=pop3:$proxy_port"
	if [ ! " $legacyserver" = " " ] ; then
		echo "LEGACY_SERVER=1"
	else
		echo "LEGACY_SERVER="
	fi
else
	echo "DESTPORT="
fi
echo "#-----------------------------------------------"
echo "MIN_LOGIN_INTERVAL=0"
if [ ! " $maxdaemons" = " " ] ; then
	echo "MAXDAEMONS=$maxdaemons"
else
	echo "MAXDAEMONS=$CONCURRENCYINCOMING"
fi
if [ ! " $maxperip" = " " ] ; then
	echo "MAXPERIP=$maxperip"
else
	echo "MAXPERIP=25"
fi
MODULES=""
for i in `ls $DESTDIR"$libexecdir"/imapmodules`
do
	if [ " $i" = " authgeneric" -o " $i" = " authtest" -o " $i" = " authpam" ] ; then
		continue
	fi
	if [ -n "$MODULES" ] ; then
		MODULES="$MODULES $i"
	else
		MODULES="$i"
	fi
done
if [ -f $DESTDIR"$libexecdir"/imapmodules/authpam ] ; then
	MODULES="$MODULES authpam"
fi
echo "IMAPMODULES=\"$MODULES\""
echo "DEBUG_LOGIN=0"
echo "POP3AUTH=\"LOGIN PLAIN CRAM-MD5 CRAM-SHA1 CRAM-SHA256\""

# POP3_STARTTLS, POP3_TLS_REQUIRED, COURIERTLS required for advertising STARTTLS
if [ $use_starttls -eq 1 ] ; then
	echo "POP3_STARTTLS=YES"
	echo "POP3_TLS_REQUIRED=0"
	if [ " $tlsprog" = " " ] ; then
		echo "COURIERTLS=$QmailBinPrefix/bin/couriertls"
	else
		echo "COURIERTLS=$tlsprog"
	fi
	create_pop3_config_ssl
fi
}

create_pop3_config_ssl()
{
if [ " $POP3_PORT" = " " ] ; then
	echo "SSLPORT=995"
else
	echo "SSLPORT=$POP3_PORT"
fi
if [ " $ipaddress" = " " ] ; then
	echo "SSLADDRESS=0"
else
	echo "SSLADDRESS=$ipaddress"
fi
echo "POP3AUTH_TLS=\"$POP3AUTH\""
echo "TLS_PROTOCOL=TLSv1.2"
echo "TLS_STARTTLS_PROTOCOL=TLSv1.2"
echo "# TLS_CIPHER_LIST=\"TLSv1:HIGH:!LOW:!MEDIUM:!EXP:!NULL:!aNULL@STRENGTH\""
echo "##NAME: TLS_TIMEOUT:0"
echo "# TLS_TIMEOUT is currently not implemented, and reserved for future use."
echo "# This is supposed to be an inactivity timeout, but its not yet implemented."
echo "#"
echo "TLS_CERTFILE=$sysconfdir/certs/servercert.pem"
echo "TLS_DHPARAMS=$sysconfdir/certs/dhparams.pem"
echo "# TLS_TRUSTCERTS="
echo "TLS_VERIFYPEER=NONE"
echo "TLS_CACHEFILE=$sysconfdir/certs/couriersslcache"
echo "TLS_CACHESIZE=524288"
echo "TLS_ALPN=pop3"
}

mysql_opt_reconnect()
{
mysqlPrefix=/usr
if [ -x $mysqlPrefix/sbin/mariadbd ] ; then
	mysqld=sbin/mariadbd
elif [ -x $mysqlPrefix/libexec/mysqld ] ; then
	mysqld=libexec/mysqld
elif [ -x $mysqlPrefix/sbin/mysqld ] ; then
	mysqld=sbin/mysqld
elif [ -x $mysqlPrefix/bin/mysqld ] ; then
	mysqld=bin/mysqld
else
	return 1
fi
mysql_version=`$mysqlPrefix/$mysqld --version 2>&1 | grep Ver | awk '{print $3}'`
echo $mysql_version |grep MariaDB > /dev/null 2>&1
if [ $? -eq 0 ] ; then
	mysql_version=`echo $mysql_version|cut -d- -f1`
	return 0
else
	major=$(echo $mysql_version|cut -d. -f1)
	minor1=$(echo $mysql_version|cut -d. -f2)
	minor2=$(echo $mysql_version|cut -d. -f3)
	if [ $major -lt 8 ] ; then
		return 0
	elif [ $major -gt 8 ] ; then
		return 1
	elif [ $major -eq 8 ] ; then
		if [ $minor1 -gt 0 ] ; then
			return 1
		fi
		if [ $minor2 -gt 34 ] ; then
			return 1
		fi
		return 0
	fi
fi
}

create_indisrvr()
{
if [ $# -ne 9 ] ; then
	echo "create_indisrvr bind_port [mysql_host mysql_user mysql_pass] avg_user_quota hard_quota base_path servicedir use_ssl=0|1" 1>&2
	return 1
fi
BIND_PORT=$1
CNTRL_HOST=$2
CNTRL_USER=$3
CNTRL_PASS=$4
AVG_USER_QUOTA=$5
HARD_QUOTA=$6
BASE_PATH=$7
SERVICEDIR=$8
indisrvr_ssl=$9
if [ " $indisrvr_ssl" = " 1" -a " $certfile" = " " ] ; then
	echo "Certificate not specfied" 1>&2
	return 1
fi

if [ " $servicetag" = " " ] ; then
	tag=$BIND_PORT
else
	tag=$servicetag
fi
if [ $nooverwrite -eq 1 -a -d "$DESTDIR"$SERVICEDIR/indisrvr.$tag/variables ] ; then
	return 0
fi
conf_dir="$DESTDIR"$SERVICEDIR/indisrvr.$tag/variables
if [ $run_file_only -eq 1 ] ; then
	if [ -d $conf_dir ] ; then
		/bin/mv $conf_dir "$conf_dir".bak
	fi
fi
/bin/mkdir -p $conf_dir
$chown root:indimail $conf_dir
/bin/chmod 550 $conf_dir
link_with_global $conf_dir

/bin/mkdir -p "$DESTDIR"$SERVICEDIR/indisrvr.$tag/log
if [ $down_state -eq 1 ] ; then
	$TOUCH "$DESTDIR"$SERVICEDIR/indisrvr.$tag/down
fi
if [ " $ipaddress" = " " ] ; then
	echo "0" > $conf_dir/LOCALIP
else
	echo "$ipaddress" > $conf_dir/LOCALIP
fi
if [ ! " $maxdaemons" = " " ] ; then
	echo "$maxdaemons" > $conf_dir/MAXDAEMONS
else
	echo "$CONCURRENCYINCOMING" > $conf_dir/MAXDAEMONS
fi
if [ ! " $maxperip" = " " ] ; then
	echo "$maxperip" > $conf_dir/MAXPERIP
else
	echo "10" > $conf_dir/MAXPERIP
fi
echo $sysconfdir/indimail.cnf > $conf_dir/MYSQL_READ_DEFAULT_FILE
echo $BIND_PORT   > $conf_dir/PORT
if [ -n "$CNTRL_HOST" ] ; then
	echo $CNTRL_HOST  > $conf_dir/CNTRL_HOST
	if [ -n "$CNTRL_USER" ] ; then
		echo $CNTRL_USER  > $conf_dir/CNTRL_USER
	else
		> $conf_dir/CNTRL_USER
	fi
	if [ -n "$CNTRL_PASS" ] ; then
		echo $CNTRL_PASS  > $conf_dir/CNTRL_PASSWD
	else
		> $conf_dir/CNTRL_PASSWD
	fi
else
	> $conf_dir/CNTRL_HOST
	> $conf_dir/CNTRL_USER
	> $conf_dir/CNTRL_PASSWD
fi
echo $AVG_USER_QUOTA > $conf_dir/AVG_USER_QUOTA
echo $HARD_QUOTA > $conf_dir/HARD_QUOTA
echo $base_path > $conf_dir/BASE_PATH
if [ ! " $domainlimits" = " " ] ; then
	echo > $conf_dir/DOMAIN_LIMITS
else
	> $conf_dir/DOMAIN_LIMITS
fi
if [ " $indisrvr_ssl" = " 1" ] ; then
	echo $certfile > $conf_dir/CERTFILE
else
	> $conf_dir/CERTFILE
fi
mysql_opt_reconnect
[ $? -eq 0 ] && echo 1 > $conf_dir/MYSQL_OPT_RECONNECT || > $conf_dir/MYSQL_OPT_RECONNECT
echo 300 > $conf_dir/DATA_TIMEOUT
(
dump_run_header
echo "CERTFILE=\$(@prefix@/bin/qcat variables/CERTFILE)"
echo "if [ ! -f \$CERTFILE ]; then"
echo "    echo Certificate not present"
echo "    sleep 5"
echo "    exit 1"
echo "fi"
echo ""
echo "exec 2>&1"
echo "exec $QmailBinPrefix/bin/envdir $envdir_opts variables sh -c \""
if [ " $indisrvr_ssl" = " 1" ] ; then
	echo "exec $QmailBinPrefix/bin/setuidgid -g qcerts indimail \\"
	echo "$QmailBinPrefix/sbin/indisrvr -i \\\$LOCALIP -p \\\$PORT -b \\\$MAXDAEMONS \\"
	echo "-t \\\$DATA_TIMEOUT -n \\\$CERTFILE\""
else
	echo "exec $QmailBinPrefix/bin/setuidgid indimail \\"
	echo "$QmailBinPrefix/sbin/indisrvr -i \\\$LOCALIP -p \\\$PORT -b \\\$MAXDAEMONS \\"
	echo "-t \\\$DATA_TIMEOUT\""
fi
) > "$DESTDIR"$SERVICEDIR/indisrvr.$tag/run
/bin/chmod +x "$DESTDIR"$SERVICEDIR/indisrvr.$tag/run

if [ ! -d $LOGDIR ] ; then
	/bin/mkdir -p $LOGDIR
	$chown qmaill:nofiles $LOGDIR
fi
# indisrvr log script
dump_log_header indisrvr.$tag > "$DESTDIR"$SERVICEDIR/indisrvr.$tag/log/run
/bin/chmod +x "$DESTDIR"$SERVICEDIR/indisrvr.$tag/log/run
echo "$prog_args" > $conf_dir/.options

if [ $run_file_only -eq 1 ] ; then
	if [ -d "$conf_dir".bak ] ; then
		/bin/rm -rf $conf_dir
		/bin/mv "$conf_dir".bak $conf_dir
	fi
fi
}

create_qscanq()
{
if [ $# -ne 1 ] ; then
	echo "USAGE: create_qscanq supervise_dir"
	return 1
fi
SERVICEDIR=$1
conf_dir="$DESTDIR"$SERVICEDIR/qscanq/variables
if [ $nooverwrite -eq 1 -a -d "$conf_dir" ] ; then
	return 0
fi
if [ $run_file_only -eq 1 ] ; then
	if [ -d $conf_dir ] ; then
		/bin/mv $conf_dir "$conf_dir".bak
	fi
fi
/bin/mkdir -p $conf_dir
$chown root:indimail $conf_dir
/bin/chmod 500 $conf_dir
link_with_global $conf_dir

/bin/mkdir -p "$DESTDIR"$SERVICEDIR/qscanq/log
if [ $down_state -eq 1 ] ; then
	$TOUCH "$DESTDIR"$SERVICEDIR/qscanq/down
fi

# qscanq init script
(
dump_run_header
echo "exec 2>&1"
echo "set -e"
echo ""
echo "if [ -d /run ] ; then"
echo "  svdir=/run/svscan"
echo "elif [ -d /var/run ] ; then"
echo "  svdir=/var/run/svscan"
echo "else"
echo "  svdir=$SERVICEDIR"
echo "fi"
echo "# set permissions for run-cleanq (run via cron) to allow"
echo "# svc -o /service/qscanq"
echo "if [ ! -p \$svdir/qscanq/supervise/control ] ; then"
echo "  if [ ! -d \$svdir/qscanq/supervise ] ; then"
echo "    /bin/mkdir -p \$svdir/qscanq/supervise"
echo "  fi"
echo "  /usr/bin/mkfifo \$svdir/qscanq/supervise/control"
echo "  /bin/chmod 660 \$svdir/qscanq/supervise/control"
echo "  $chown qscand:root \$svdir/qscanq/supervise/control"
echo "else"
echo "  /bin/chmod 660 \$svdir/qscanq/supervise/control"
echo "  $chown qscand:root \$svdir/qscanq/supervise/control"
echo "fi"
echo "$chown qscand:root \$svdir/qscanq/supervise"
echo "echo \"Finished initialization\""
) > "$DESTDIR"$SERVICEDIR/qscanq/init
/bin/chmod +x "$DESTDIR"$SERVICEDIR/qscanq/init

# qscanq script
(
dump_run_header
echo "exec 2>&1"
if [ " $scan_interval" = " " ] ; then
	scan_interval=10
fi
echo "exec $QmailBinPrefix/bin/setuidgid qscand $QmailBinPrefix/sbin/cleanq \\"
echo "  -l -s $scan_interval $QmailHOME/qscanq/root/scanq"
) > "$DESTDIR"$SERVICEDIR/qscanq/run
/bin/chmod +x "$DESTDIR"$SERVICEDIR/qscanq/run

if [ ! -d $LOGDIR ] ; then
	/bin/mkdir -p $LOGDIR
	$chown qmaill:nofiles $LOGDIR
fi
# qscanq log script
dump_log_header qscanq > "$DESTDIR"$SERVICEDIR/qscanq/log/run
/bin/chmod +x "$DESTDIR"$SERVICEDIR/qscanq/log/run
echo "$prog_args" > $conf_dir/.options

if [ $run_file_only -eq 1 ] ; then
	if [ -d "$conf_dir".bak ] ; then
		/bin/rm -rf $conf_dir
		/bin/mv "$conf_dir".bak $conf_dir
	fi
fi
}

create_clamd()
{
if [ $# -ne 2 ] ; then
	echo "USAGE: create_clamd clamdPrefix supervise_dir"
	return 1
fi
clamdPrefix=$1
SERVICEDIR=$2
conf_dir="$DESTDIR"$SERVICEDIR/clamd/variables
if [ $nooverwrite -eq 1 -a -d $conf_dir ] ; then
	return 0
fi
if [ $run_file_only -eq 1 ] ; then
	if [ -d $conf_dir ] ; then
		/bin/mv $conf_dir "$conf_dir".bak
	fi
fi
/bin/mkdir -p $conf_dir
/bin/chmod 555 $conf_dir

/bin/mkdir -p "$DESTDIR"$SERVICEDIR/clamd/log
if [ $down_state -eq 1 ] ; then
	$TOUCH "$DESTDIR"$SERVICEDIR/clamd/down
fi
# clamd script
(
dump_run_header
echo "exec 2>&1"
echo "CLAMD_SOCK=\`grep -w ^LocalSocket $sysconfdir/scan.conf | awk '{print \$2}'\`"
echo "if [ -z \"\$CLAMD_SOCK\" ] ; then"
echo "  echo \"LocalSocket not defined in $sysconfdir/scan.conf\" 1>&2"
echo "  sleep 60"
echo "  exit 1"
echo "fi"
echo "SOCKET_DIR=\`dirname \$CLAMD_SOCK\`"
echo "if [ ! -d \"\$SOCKET_DIR\" ] ; then"
echo "  /bin/mkdir -p \"\$SOCKET_DIR\""
echo "fi"
echo "$chown qscand:qmail \"\$SOCKET_DIR\""
echo "/bin/chmod 750 \"\$SOCKET_DIR\""
echo ""
echo "# Check for a leftover socket."
echo "if [ -e \$CLAMD_SOCK ] ; then"
echo "  echo \"run: WARNING: file \$CLAMD_SOCK exists\""
echo "  SCAN_FILE=\$0"
echo "  if $clamdPrefix/bin/clamdscan \$SCAN_FILE --quiet --no-summary"
echo "    then"
echo "    echo \"run: FATAL: clamd is already running. quitting...\""
echo "    sleep 300"
echo "    exit 1"
echo "  else"
echo "    echo \"run: INFO: clamd is not running. Deleting \$CLAMD_SOCK\""
echo "    /bin/rm -f \$CLAMD_SOCK"
echo "  fi"
echo "fi"
echo ""
echo "dbd=\`grep -w \"^DatabaseDirectory\" $sysconfdir/scan.conf | awk '{print \$2}'\`"
echo "if [ -z \"\$dbd\" ] ; then"
echo "  echo \"DatabaseDirectory not defined in $sysconfdir/scan.conf\" 1>&2"
echo "  sleep 10"
echo "  exit 1"
echo "fi"
echo "if [ ! -f \$dbd/main.cvd -a ! -f \$dbd/main.cld ] ; then"
echo "  echo \"run: FATAL: no signatures found...\""
echo "  sleep 300"
echo "  exit 1"
echo "fi"
echo "fgd=\`grep -w \"^Foreground\" $sysconfdir/scan.conf | awk '{print \$2}'\`"
echo "if [ -z \"\$fgd\" ] ; then"
echo "  echo \"Foreground not defined in $sysconfdir/scan.conf\" 1>&2"
echo "  sleep 10"
echo "  exit 1"
echo "fi"
echo ""
echo "# Run the scanner daemon."
echo "exec $clamdPrefix/sbin/clamd \\"
echo "  --config-file=$sysconfdir/scan.conf"
) > "$DESTDIR"$SERVICEDIR/clamd/run
/bin/chmod +x "$DESTDIR"$SERVICEDIR/clamd/run

if [ ! -d $LOGDIR ] ; then
	/bin/mkdir -p $LOGDIR
	$chown qmaill:nofiles $LOGDIR
fi
# clamd log script
dump_log_header clamd > "$DESTDIR"$SERVICEDIR/clamd/log/run
/bin/chmod +x "$DESTDIR"$SERVICEDIR/clamd/log/run
echo "$prog_args" > $conf_dir/.options

if [ $run_file_only -eq 1 ] ; then
	if [ -d "$conf_dir".bak ] ; then
		/bin/rm -rf $conf_dir
		/bin/mv "$conf_dir".bak $conf_dir
	fi
fi

conf_dir="$DESTDIR"$SERVICEDIR/freshclam/variables
if [ $run_file_only -eq 1 ] ; then
	if [ -d $conf_dir ] ; then
		/bin/mv $conf_dir "$conf_dir".bak
	fi
fi
/bin/mkdir -p "$DESTDIR"$SERVICEDIR/freshclam/log
if [ $down_state -eq 1 ] ; then
	$TOUCH "$DESTDIR"$SERVICEDIR/freshclam/down
fi
/bin/mkdir -p $conf_dir
/bin/chmod 555 $conf_dir
qsysconfdir=/etc # weird tha clamd and freshclam do not share the same dir for config files
# freshclam script
(
dump_run_header
echo "exec 2>&1"
echo "dbd=\`grep -w \"^DatabaseDirectory\" $qsysconfdir/freshclam.conf | awk '{print \$2}'\`"
echo "if [ -z \"\$dbd\" ] ; then"
echo "	echo \"DatabaseDirectory not defined in $qsysconfdir/freshclam.conf\" 1>&2"
echo "	sleep 10"
echo "	exit 1"
echo "fi"
echo "if [ ! -d \"\$dbd\" ] ; then"
echo "	/bin/mkdir \$dbd"
echo "	$chown qscand:qmail \$dbd"
echo "	/bin/chmod 775 \$dbd"
echo "fi"
echo "ans=\`grep -w \"^AllowSupplementaryGroups\" $qsysconfdir/freshclam.conf | awk '{print \$2}'\`"
echo "fgd=\`grep -w \"^Foreground\" $qsysconfdir/freshclam.conf | awk '{print \$2}'\`"
echo "if [ -z \"\$fgd\" ] ; then"
echo "	echo \"Foreground not defined in $qsysconfdir/freshclam.conf\" 1>&2"
echo "	sleep 10"
echo "	exit 1"
echo "fi"
echo ""
echo "cmp $sysconfdir/foxhole_all.cdb \$dbd/foxhole_all.cdb >/dev/null 2>&1"
echo "if [ \$? -ne 0 ] ; then"
echo "	echo \"Updating foxhole_all.cdb\""
echo "	/bin/cp $sysconfdir/foxhole_all.cdb \$dbd"
echo "	$chown qscand:qmail \$dbd/foxhole_all.cdb"
echo "fi"
echo ""
echo "if [ \" \$ans\" = \" yes\" -o \" \$ans\" = \" Yes\" ] ; then"
echo "  if [ \" \$fgd\" = \" yes\" -o \" \$fgd\" = \" Yes\" ] ; then"
echo "    exec $clamdPrefix/bin/freshclam -v --stdout --datadir=\$dbd -d -c 2 \\"
echo "      --config-file=$qsysconfdir/freshclam.conf"
echo "  else"
echo "    exec $clamdPrefix/bin/freshclam -v --stdout --datadir=\$dbd -f -d -c 2 \\"
echo "      --config-file=$qsysconfdir/freshclam.conf"
echo "  fi"
echo "else"
echo "  if [ \" \$fgd\" = \" yes\" -o \" \$fgd\" = \" Yes\" ] ; then"
echo "    exec $QmailBinPrefix/bin/setuidgid qscand \\"
echo "      $clamdPrefix/bin/freshclam -v --stdout --datadir=\$dbd -d -c 2 \\"
echo "      --config-file=$qsysconfdir/freshclam.conf"
echo "  else"
echo "    exec $QmailBinPrefix/bin/setuidgid qscand \\"
echo "      $clamdPrefix/bin/freshclam -v --stdout --datadir=\$dbd -f -d -c 2 \\"
echo "      --config-file=$qsysconfdir/freshclam.conf"
echo "  fi"
echo "fi"
) > "$DESTDIR"$SERVICEDIR/freshclam/run
/bin/chmod +x "$DESTDIR"$SERVICEDIR/freshclam/run

if [ ! -d $LOGDIR ] ; then
	/bin/mkdir -p $LOGDIR
	$chown qmaill:nofiles $LOGDIR
fi
# freshclam log script
dump_log_header freshclam > "$DESTDIR"$SERVICEDIR/freshclam/log/run
/bin/chmod +x "$DESTDIR"$SERVICEDIR/freshclam/log/run
echo "$prog_args" > $conf_dir/.options

if [ $run_file_only -eq 1 ] ; then
	if [ -d "$conf_dir".bak ] ; then
		/bin/rm -rf $conf_dir
		/bin/mv "$conf_dir".bak $conf_dir
	fi
fi
# fix QHPSI variable in services which require virus scanning
for q in smtpd.465 smtpd.25 smtpd.587 qmqpd.628 qmtpd.209
do
	grep qhpsi "$DESTDIR"$SERVICEDIR/qmail-"$q"/variables/.options >/dev/null 2>&1
	if [ $? -ne 0 ] ; then
		qhpsi="$clamdPrefix/bin/clamdscan %s --fdpass --quiet --no-summary"
		cur_options=$(@prefix@/bin/qcat "$DESTDIR"$SERVICEDIR/qmail-"$q"/variables/.options)
		if [ -n "$cur_options" ] ; then
			cur_options="$cur_options --qhpsi=\"$qhpsi\""
			echo "$cur_options" >  "$DESTDIR"$SERVICEDIR/qmail-"$q"/variables/.options
			echo $qhpsi > "$DESTDIR"$SERVICEDIR/qmail-"$q"/variables/QHPSI
		fi
	fi
done
# fix QHPSI for default queue
grep qhpsi "$DESTDIR"$sysconfdir/control/defaultqueue/.options > /dev/null 2>&1
if [ $? -ne 0 ] ; then
	qhpsi="$clamdPrefix/bin/clamdscan %s --fdpass --quiet --no-summary"
	cur_options=$(@prefix@/bin/qcat "$DESTDIR"$sysconfdir/control/defaultqueue/.options)
	if [ -n "$cur_options" ] ; then
		cur_options="$cur_options --qhpsi=\"$qhpsi\""
		echo "$cur_options" >  "$DESTDIR"$sysconfdir/control/defaultqueue/.options
		echo $qhpsi > "$DESTDIR"$sysconfdir/control/defaultqueue/QHPSI
	fi
fi
}

create_pwdlookup()
{
if [ $# -ne 9 ] ; then
	echo "USAGE: create_pwdlookup socket_path thread_count listen_timeout supervise_dir mysql_host mysql_user mysql_pass mysql_port mysql_socket" 1>&2
	return 1
fi
SOCKET_PATH=$1
THREAD_COUNT=$2
LISTEN_TIMEOUT=$3
SERVICEDIR=$4
if [ $nooverwrite -eq 1 -a -d "$DESTDIR"$SERVICEDIR/pwdlookup/variables ] ; then
	return 0
fi
conf_dir="$DESTDIR"$SERVICEDIR/pwdlookup/variables
if [ $run_file_only -eq 1 ] ; then
	if [ -d $conf_dir ] ; then
		/bin/mv $conf_dir "$conf_dir".bak
	fi
fi
/bin/mkdir -p $conf_dir
/bin/chmod 500 $conf_dir
link_with_global $conf_dir

/bin/mkdir -p "$DESTDIR"$SERVICEDIR/pwdlookup/log
if [ $down_state -eq 1 ] ; then
	$TOUCH "$DESTDIR"$SERVICEDIR/pwdlookup/down
fi
if [ -d /run ] ; then
	rundir="/run/indimail/pwdlookup"
elif [ -d /var/run ] ; then
	rundir="/var/run/indimail/pwdlookup"
else
	rundir="/tmp/pwdlookup"
fi
echo "$rundir/nssd.sock" > $conf_dir/NSSD_SOCKET
(
dump_run_header
echo "if [ ! -d $rundir ] ; then"
echo "  mkdir -p $rundir"
echo "fi"
echo "$chown root:indimail $rundir"
echo "chmod 775 $rundir"
echo "exec 2>&1"
echo ""
echo "exec $QmailBinPrefix/bin/envdir $envdir_opts variables \\"
echo "$QmailBinPrefix/bin/setuidgid -g qmail indimail $QmailBinPrefix/sbin/nssd -d notice"
) > "$DESTDIR"$SERVICEDIR/pwdlookup/run
/bin/chmod +x "$DESTDIR"$SERVICEDIR/pwdlookup/run
(
echo "10"
echo "mysql.3306"
) > "$DESTDIR"$SERVICEDIR/pwdlookup/wait
if [ -d "$DESTDIR"$SERVICEDIR/mysql.3306 ] ; then
	/bin/rm -rf "$DESTDIR"$SERVICEDIR/pwdlookup/down
else
	touch "$DESTDIR"$SERVICEDIR/pwdlookup/down
fi

for i in "$DESTDIR"$SERVICEDIR/qmail-send.* "$DESTDIR"$SERVICEDIR/slowq-send
do
	if [ -d "$DESTDIR"$SERVICEDIR/mysql.3306 ] ; then
		if [ ! -f "$i"/wait ] ; then
			(
			echo "10"
			echo "pwdlookup"
			) > "$i"/wait
		fi
	else
		/bin/rm -f "$i"/wait
	fi
done

if [ ! -d $LOGDIR ] ; then
	/bin/mkdir -p $LOGDIR
	$chown qmaill:nofiles $LOGDIR
fi
# pwdlookup log script
dump_log_header pwdlookup > "$DESTDIR"$SERVICEDIR/pwdlookup/log/run
/bin/chmod +x "$DESTDIR"$SERVICEDIR/pwdlookup/log/run
echo "$prog_args" > $conf_dir/.options

if [ $run_file_only -eq 1 ] ; then
	if [ -d "$conf_dir".bak ] ; then
		/bin/rm -rf $conf_dir
		/bin/mv "$conf_dir".bak $conf_dir
	fi
else
	create_pwdlookup_conf "$5" "$6" "$7" "$8" "$9" > $TMPDIR/config.cnf.$$
	if [ $? -eq 0 ] ; then
		if [ -n "$mysysconfdir" ] ; then
			sysconfdir=$mysysconfdir
		fi
		conf_file=$DESTDIR"$sysconfdir/nssd.conf"
		change_config $conf_file $TMPDIR/config.cnf.$$
		/bin/chmod 640 $conf_file
		$chown root:qmail $conf_file
	fi
fi
}

create_pwdlookup_conf()
{
if [ $# -ne 5 ] ; then
	echo "USAGE: create_pwdlookup_conf mysql_host mysql_user mysql_pass mysql_port mysql_socket" 1>&2
	return 1
fi
mysql_host=$1
mysql_user=$2
mysql_pass=$3
mysql_port=$4
mysqlSocket=$5
if [ -z "$mysqlSocket" ]; then
	if [ -n "$MYSQL_SOCKET" ] ; then
		mysqlSocket=$MYSQL_SOCKET
	elif [ -d /run ] ; then
		mysqlSocket=/run/mysqld/mysqld.sock
	elif [ -d /var/run ] ; then
		mysqlSocket=/var/run/mysqld/mysqld.sock
	elif [ -d /var/lib/mysql ] ; then
		mysqlSocket=/var/lib/mysql/mysql.sock
	else
		mysqlSocket=/run/mysqld/mysqld.sock
	fi
fi
SYSTEM=$(uname -s)
case "$SYSTEM" in
	Linux)
	echo "getpwnam    SELECT pw_name,'x',555,555,pw_gecos,pw_dir,pw_shell \\"
	echo "            FROM indimail \\"
	echo "            WHERE pw_name='%1\$s' and pw_domain='%2\$s' \\"
	echo "            LIMIT 1"
	echo "getspnam    SELECT pw_name,pw_passwd,'1','0','99999','0','0','-1','0' \\"
	echo "            FROM indimail \\"
	echo "            WHERE pw_name='%1\$s'and pw_domain='%2\$s' \\"
	echo "            LIMIT 1"
	echo "getpwent    SELECT pw_name,'x',555,555,pw_gecos,pw_dir,pw_shell \\"
	echo "            FROM indimail LIMIT 100"
	echo "getspent    SELECT pw_name,pw_passwd,'1','0','99999','0','0','-1','0' \\"
	echo "            FROM indimail"
	;;
	FreeBSD)
	echo "getpwnam    SELECT pw_name,pw_passwd,555,555,0,'',pw_gecos,pw_dir,pw_shell,0 \\"
	echo "            FROM indimail \\"
	echo "            WHERE pw_name='%1\$s' and pw_domain='%2\$s' \\"
	echo "            LIMIT 1"
	echo "getpwent    SELECT pw_name,pw_passwd,555,555,0,'',pw_gecos,pw_dir,pw_shell,0 \\"
	echo "            FROM indimail"
	;;
esac
echo ""
echo "host        $mysql_host"
echo "database    indimail"
echo "username    $mysql_user"
echo "password    $mysql_pass"
if [ -n "$mysqlSocket" ] ; then
	echo "socket      $mysqlSocket"
else
	if [ " $mysql_port" = " " ] ; then
		echo "port       3306"
	else
		echo "port       $mysql_port"
	fi
fi
if [ -d /run ] ; then
echo "pidfile     /run/indimail/pwdlookup/nssd.pid"
elif [ -d /var/run ] ; then
echo "pidfile     /var/run/indimail/pwdlookup/nssd.pid"
else
echo "pidfile     /tmp/indimail/pwdlookup/nssd.pid"
fi
echo "threads     $THREAD_COUNT"
echo "timeout     $LISTEN_TIMEOUT"
echo "facility    daemon"
echo "priority    err"
}

create_inlookup()
{
if [ $# -ne 4 ] ; then
	echo "USAGE: create_inlookup infifo thread_count activeDays supervise_dir" 1>&2
	return 1
fi
INFIFO_PATH=$1
THREAD_COUNT=$2
activeDays=$3
SERVICEDIR=$4
if [ -d /run ] ; then
	rundir=/run/indimail
elif [ -d /var/run ] ; then
	rundir=/var/run/indimail
else
	rundir=/tmp/indimail
fi
fifo_tmpdir="$rundir/inquery"
fifo_name=`basename $INFIFO_PATH`
infifo_dir=`dirname $INFIFO_PATH`
if [ $infifo_dir = "." ] ; then
	infifo_dir=$rundir/inlookup
fi

if [ $nooverwrite -eq 1 -a -d "$DESTDIR"$SERVICEDIR/inlookup.$fifo_name/variables ] ; then
	return 0
fi
conf_dir="$DESTDIR"$SERVICEDIR/inlookup.$fifo_name/variables
if [ $run_file_only -eq 1 ] ; then
	if [ -d $conf_dir ] ; then
		/bin/mv $conf_dir "$conf_dir".bak
	fi
fi
/bin/mkdir -p $conf_dir
/bin/chmod 500 $conf_dir
link_with_global $conf_dir

/bin/mkdir -p "$DESTDIR"$SERVICEDIR/inlookup.$fifo_name/log
if [ $down_state -eq 1 ] ; then
	$TOUCH "$DESTDIR"$SERVICEDIR/inlookup.$fifo_name/down
fi

echo "$fifo_name"  > $conf_dir/INFIFO
echo "$infifo_dir" > $conf_dir/INFIFODIR
echo $THREAD_COUNT > $conf_dir/THREAD_COUNT
echo "0660" > $conf_dir/FIFO_MODE
if [ " $activeDays"  = " " ] ; then
echo 0 > $conf_dir/CACHE_COUNT
else
echo "`expr $activeDays \* 86400`" > $conf_dir/CACHE_COUNT
fi
if [ ! " $CONTROLDIR" = " " ] ; then
	echo $CONTROLDIR > $conf_dir/CONTROLDIR
else
	> $conf_dir/CONTROLDIR
fi
if [ " $password_cache" = " " ] ; then
	> $conf_dir/PASSWD_CACHE
else
	echo > $conf_dir/PASSWD_CACHE
fi
if [ " $query_cache" = " " ] ; then
	> $conf_dir/QUERY_CACHE
else
	echo > $conf_dir/QUERY_CACHE
fi
if [ " $use_btree" = " " ] ; then
	> $conf_dir/USE_BTREE
else
	echo 1 > $conf_dir/USE_BTREE
fi
if [ ! " $max_btree_count" = " " ] ; then
	echo $max_btree_count > $conf_dir/MAX_BTREE_COUNT
else
	echo 0 > $conf_dir/MAX_BTREE_COUNT
fi
if [ ! " $domainlimits" = " " ] ; then
	echo  > $conf_dir/DOMAIN_LIMITS
else
	> $conf_dir/DOMAIN_LIMITS
fi
echo $sysconfdir/indimail.cnf > $conf_dir/MYSQL_READ_DEFAULT_FILE
echo "inlookup" > $conf_dir/MYSQL_READ_DEFAULT_GROUP
mysql_opt_reconnect
[ $? -eq 0 ] && echo 1 > $conf_dir/MYSQL_OPT_RECONNECT || > $conf_dir/MYSQL_OPT_RECONNECT

(
dump_run_header
echo "exec 2>&1"
echo ""
echo "if [ ! -d $infifo_dir ] ; then"
echo "  /bin/mkdir -p $infifo_dir"
echo "  $chown indimail:qmail $infifo_dir"
echo "  chmod 770 $infifo_dir"
echo "fi"
echo "if [ ! -d $fifo_tmpdir ] ; then"
echo "  /bin/mkdir -p $fifo_tmpdir"
echo "  chmod 2770 $fifo_tmpdir"
echo "  $chown qmaild:indimail $fifo_tmpdir"
echo "fi"
echo "if [ -f variables/THREAD_COUNT ] ; then"
echo "  count=\$(@prefix@/bin/qcat variables/THREAD_COUNT)"
echo "else"
echo "  count=5"
echo "fi"
echo "echo \"initializing fifo count=\$count ...\""
echo "/bin/rm -f $infifo_dir/infifo.*"
echo "for i in \$(seq 1 \$count); do f=$infifo_dir/infifo.\$i; mkfifo \$f && /bin/chmod 660 \$f && $chown qmaild:indimail \$f; done"
echo "exec $QmailBinPrefix/bin/envdir $envdir_opts variables sh -c \""
echo "exec $QmailBinPrefix/bin/setuidgid -g qmail,indimail,qcerts qmaild $QmailBinPrefix/sbin/inlookup \\"
echo "  -v -i \\\$THREAD_COUNT -c \\\$CACHE_COUNT\""
) > "$DESTDIR"$SERVICEDIR/inlookup.$fifo_name/run
/bin/chmod +x "$DESTDIR"$SERVICEDIR/inlookup.$fifo_name/run

if [ -d "$DESTDIR"$SERVICEDIR/mysql.3306 ] ; then
	/bin/rm -f "$DESTDIR"$SERVICEDIR/inlookup.$fifo_name/down
else
	touch "$DESTDIR"$SERVICEDIR/inlookup.$fifo_name/down
fi

(
echo "10"
echo "mysql.3306"
) > "$DESTDIR"$SERVICEDIR/inlookup.$fifo_name/wait

(
dump_run_header
echo "exec 2>&1"
echo "set -e"
echo ""
echo "if [ -d /run ] ; then"
echo "  svdir=/run/svscan"
echo "elif [ -d /var/run ] ; then"
echo "  svdir=/var/run/svscan"
echo "else"
echo "  svdir=$SERVICEDIR"
echo "fi"
echo "#"
echo "# do this so that vdeluser can run libexec script vdeluser"
echo "# which can send sighup to inlookup service using the svc command"
echo "#"
echo "if [ ! -p \$svdir/inlookup.$fifo_name/supervise/control ] ; then"
echo "  if [ ! -d \$svdir/inlookup.$fifo_name/supervise ] ; then"
echo "    /bin/mkdir -p \$svdir/inlookup.$fifo_name/supervise"
echo "  fi"
echo "  /usr/bin/mkfifo \$svdir/inlookup.$fifo_name/supervise/control"
echo "  /bin/chmod 660 \$svdir/inlookup.$fifo_name/supervise/control"
echo "  $chown root:indimail \$svdir/inlookup.$fifo_name/supervise/control"
echo "else"
echo "  /bin/chmod 660 \$svdir/inlookup.$fifo_name/supervise/control"
echo "  $chown root:indimail \$svdir/inlookup.$fifo_name/supervise/control"
echo "fi"
echo "$chown indimail:indimail \$svdir/inlookup.$fifo_name/supervise"
echo "echo \"Finished initialization\""
) > "$DESTDIR"$SERVICEDIR/inlookup.$fifo_name/init
/bin/chmod +x "$DESTDIR"$SERVICEDIR/inlookup.$fifo_name/init

if [ ! -d $LOGDIR ] ; then
	/bin/mkdir -p $LOGDIR
	$chown qmaill:nofiles $LOGDIR
fi
# inlookup log script
dump_log_header inlookup.$fifo_name > "$DESTDIR"$SERVICEDIR/inlookup.$fifo_name/log/run
/bin/chmod +x "$DESTDIR"$SERVICEDIR/inlookup.$fifo_name/log/run
echo "$prog_args" > $conf_dir/.options

if [ $run_file_only -eq 1 ] ; then
	if [ -d "$conf_dir".bak ] ; then
		/bin/rm -rf $conf_dir
		/bin/mv "$conf_dir".bak $conf_dir
	fi
fi
}

mariadb_ssl_rsa_setup()
{
if [ $# -ne 2 ] ; then
	echo "USAGE: mariadb_ssl_rsa_setup capath certdir" 1>&2
	return 1
fi
if [ ! -x /usr/bin/openssl ] ; then
	echo "/usr/bin/openssl: No such file or directory" 1>&2
	return 1
fi
ca_path=$1
certdir=$2
if [ -z "$ca_path" ] ; then
	ca_path="."
fi
if [ ! -d "$DESTDIR"$certdir ] ; then
	/bin/mkdir -p "$DESTDIR"$certdir
	if [ $? -ne 0 ] ; then
		return 1
	fi
else
	if [ -f "$DESTDIR"$certdir/client-cert.pem -o -f "$DESTDIR"$certdir/server-cert.pem ] ; then
		if [ $force -ne 1 ] ; then
			echo "SSL Certs exists. Remove client, server certs in "$DESTDIR"$certdir to proceed" 1>&2
			return 1
		fi
	fi
fi

if [ ! -f $ca_path/ca.pem -o ! -f $ca_path/ca-key.pem ] ; then
	# create CA certificate with x509 v3 extension
	if [ ! -d $ca_path ] ; then
		mkdir -p $ca_path
		if [ $? -ne 0 ] ; then
			echo "Failed to create $ca_path" 1>&2
			return 1
		fi
	fi
	cd $ca_path
	if [ $? -ne 0 ] ; then
		echo "unable to cd to $ca_path" 1>&2
		return 1
	fi
	echo "basicConstraints=CA:TRUE"  > cav3.ext
	# Create CA
	/usr/bin/openssl req -newkey rsa:2048 -nodes -keyout $ca_path/ca-key.pem \
		-subj /CN=MariaDB_Auto_Generated_CA_Certificate -out $ca_path/ca-req.pem
	if [ $? -ne 0 ] ; then
		echo "failed to create $ca_path/ca-req.pem" 1>&2
		/bin/rm -f $ca_path/cav3.ext
		return 1
	fi
	/usr/bin/openssl rsa -in $ca_path/ca-key.pem -out $ca_path/ca-key.pem
	if [ $? -ne 0 ] ; then
		echo "failed to create $ca_path/ca-key.pem" 1>&2
		/bin/rm -f $ca_path/cav3.ext
		return 1
	fi
	/usr/bin/openssl x509 -sha256 -days 3650 -extfile $ca_path/cav3.ext -set_serial 1 \
		-req -in $ca_path/ca-req.pem -signkey $ca_path/ca-key.pem -out $ca_path/ca.pem
	if [ $? -ne 0 ] ; then
		echo "failed to create $ca_path/ca.pem" 1>&2
		/bin/rm -f $ca_path/cav3.ext
		return 1
	fi
	/bin/rm -f $ca_path/cav3.ext
fi

cd "$DESTDIR"$certdir
if [ $? -ne 0 ] ; then
	echo "unable to cd to "$DESTDIR"$certdir" 1>&2
	return 1
fi

# create certificats with x509 v3 extension
echo "basicConstraints=CA:FALSE" > certv3.ext
# Create Server Cert
/usr/bin/openssl req -newkey rsa:2048 -nodes -keyout server-key.pem \
	-subj /CN=MariaDB_Auto_Generated_Server_Certificate -out server-req.pem
if [ $? -ne 0 ] ; then
	echo "failed to create $certdir/server-req.pem" 1>&2
	/bin/rm -f certv3.ext
	return 1
fi
/usr/bin/openssl rsa -in server-key.pem -out server-key.pem
if [ $? -ne 0 ] ; then
	echo "failed to create $certdir/server-key.pem" 1>&2
	/bin/rm -f certv3.ext
	return 1
fi
/usr/bin/openssl x509 -sha256 -days 3650 -extfile certv3.ext -set_serial 2 -req \
	-in server-req.pem -CA $ca_path/ca.pem -CAkey $ca_path/ca-key.pem -out server-cert.pem
if [ $? -ne 0 ] ; then
	echo "failed to create $certdir/server-cert.pem" 1>&2
	/bin/rm -f certv3.ext
	return 1
fi

# Create Client Cert
/usr/bin/openssl req -newkey rsa:2048 -nodes -keyout client-key.pem \
	-subj /CN=MariaDB_Auto_Generated_Client_Certificate -out client-req.pem
if [ $? -ne 0 ] ; then
	echo "failed to create $certdir/client-req.pem" 1>&2
	/bin/rm -f certv3.ext
	return 1
fi
/usr/bin/openssl rsa -in client-key.pem -out client-key.pem
if [ $? -ne 0 ] ; then
	echo "failed to create $certdir/client-key.pem" 1>&2
	/bin/rm -f certv3.ext
	return 1
fi
/usr/bin/openssl x509 -sha256 -days 3650 -extfile certv3.ext -set_serial 3 -req \
	-in client-req.pem -CA $ca_path/ca.pem -CAkey $ca_path/ca-key.pem -out client-cert.pem
if [ $? -ne 0 ] ; then
	echo "failed to create $certdir/client-cert.pem" 1>&2
	/bin/rm -f certv3.ext
	return 1
fi
/bin/rm -f certv3.ext

# Verifiy the certificates
/usr/bin/openssl verify -CAfile $ca_path/ca.pem server-cert.pem client-cert.pem

# Generate public & private key
/usr/bin/openssl genrsa  -out private_key.pem 2048
/usr/bin/openssl rsa -in private_key.pem -pubout -out public_key.pem
$chown mysql:indimail $ca_path/ca-key.pem $ca_path/ca.pem server-key.pem server-cert.pem \
	client-key.pem client-cert.pem private_key.pem public_key.pem
/bin/chmod 600 $ca_path/ca-key.pem private_key.pem server-key.pem
/bin/chmod 640 client-cert.pem client-key.pem server-cert.pem $ca_path/ca.pem
/bin/chmod 644 public_key.pem
/bin/rm -f $ca_path/ca-req.pem server-req.pem client-req.pem
echo ""
echo "Verifying server and client certs with ca.pem"
/usr/bin/openssl verify -CAfile $ca_path/ca.pem server-cert.pem client-cert.pem
echo ""
echo "ca.pem purpose"
/usr/bin/openssl x509 -in $ca_path/ca.pem -noout -purpose
echo ""
echo "client-cert.pem purpose"
/usr/bin/openssl x509 -in client-cert.pem -noout -purpose
echo ""
echo "server-cert.pem purpose"
/usr/bin/openssl x509 -in server-cert.pem -noout -purpose
return 0
}

mecho()
{
	echo "$*"
	echo " $*" 1>&2
}

wait_for_mysqld()
{
	die=$1
	count=0
	if [ " $die" = " 1" ] ; then
		/bin/echo -n "Waiting for MySQL Server to stop ." 1>&2
	else
		/bin/echo -n "Waiting for MySQL Server to start ." 1>&2
	fi
	while true
	do
		if [ $count -eq 5 ] ; then
			echo "giving up" 1>&2
			break
		fi
		if [ " $die" = " 1" ] ; then
			if [ ! -f "$DESTDIR"$databasedir/mysqld.pid ] ; then
				echo
				sleep 5
				break
			fi
		else
			if [ -s "$DESTDIR"$databasedir/mysqld.pid ] ; then
				echo
				sleep 5
				break
			fi
		fi
		/bin/echo -n "."
		count=`expr $count + 1`
		sleep 5
	done
}

set_mysql_ssl_permission()
{
	ssl_dir=$1
	if [ -z "$ssl_dir" ] ; then
		echo "usage: set_mysql_ssl_permission dir" 1>&2
		return 1
	fi
	for i in ca-key.pem ca.pem server-key.pem server-cert.pem \
		client-key.pem client-cert.pem private_key.pem public_key.pem
	do
		$chown mysql:indimail $ssl_dir/$i
	done
	for i in ca-key.pem private_key.pem server-key.pem
	do
		/bin/chmod 600 $ssl_dir/$i
	done
	for i in client-cert.pem client-key.pem server-cert.pem ca.pem
	do
		/bin/chmod 640 $ssl_dir/$i
	done
	/bin/chmod 644 $ssl_dir/public_key.pem
}

create_mysql_db()
{
if [ $# -ne 4 ] ; then
	echo "USAGE: create_mysql_db user password databasedir mysql_socket" 1>&2
	return 1
fi
user=$1
pass=$2
databasedir=$3
mysqlSocket=$4
if [ -z "$mysqlSocket" ] ; then
	if [ -n "$MYSQL_SOCKET" ] ; then
		mysqlSocket=$MYSQL_SOCKET
	elif [ -d /run ] ; then
		mysqlSocket=/run/mysqld/mysqld.sock
	elif [ -d /var/run ] ; then
		mysqlSocket=/var/run/mysqld/mysqld.sock
	elif [ -d /var/lib/mysql ] ; then
		mysqlSocket=/var/lib/mysql/mysql.sock
	else
		mysqlSocket=/run/mysqld/mysqld.sock
	fi
fi
if [ -x $mysqlPrefix/sbin/mariadbd ] ; then
	mysqld=sbin/mariadbd
elif [ -x $mysqlPrefix/libexec/mysqld ] ; then
	mysqld=libexec/mysqld
elif [ -x $mysqlPrefix/sbin/mysqld ] ; then
	mysqld=sbin/mysqld
elif [ -x $mysqlPrefix/bin/mysqld ] ; then
	mysqld=bin/mysqld
else
	echo "mysqld: No such File or directory" 1>&2
	return 1
fi
tmysql_version=`$mysqlPrefix/$mysqld --version`
echo $tmysql_version 1>&2
mysql_version=`echo $tmysql_version | awk '{print $3}' 2>&1`
echo $mysql_version | grep MariaDB > /dev/null 2>&1
if [ $? -eq 0 ] ; then
	mysql_version=`echo $mysql_version|cut -d- -f1`
	mariadb=1
else
	$mysqlPrefix/$mysqld --version | grep -E "MySQL Community Server|FreeBSD" > /dev/null 2>&1
	if [ $? -eq 0 ] ; then
		mysql_community_server=1
	else
		mysql_community_server=0
	fi
	mariadb=0
fi
mysql_version_8=0
mariadb_version_10=0
case "$mysql_version" in
	8.*)
	# the monkeys at Or$cle have again succeeded in introducing incompatibility with previous
	# database initialization methods
	mysql_version_8=1
	;;

	10.*)
	if [ $mariadb -eq 1 ] ; then
		mariadb_version_10=1
	fi
	;;

	*)
	mysql_version_8=0
	;;
esac
if [ -x $mysqlPrefix/bin/mariadb-install-db ] ; then
	install_db=$mysqlPrefix/bin/mariadb-install-db
elif [ -x $mysqlPrefix/bin/mysql_install_db ] ; then
	install_db=$mysqlPrefix/bin/mysql_install_db
elif [ -x $mysqlPrefix/scripts/mysql_install_db ] ; then
	install_db=$mysqlPrefix/scripts/mysql_install_db
else
	install_db=""
fi

echo "mysql_version=$mysql_version, mariadb=$mariadb, mysql_community_server=$mysql_community_server, mysql_version_8=$mysql_version_8, mariadb_version_10=$mariadb_version_10" 1>&2
echo "install_db=$install_db" 1>&2
if [ -d "$DESTDIR"$databasedir/data/mysql -a "$force" -eq 0 ] ; then
	echo "MySQL database exists!!!" 1>&2
	return 0
fi
t=$(dirname $mysqlSocket)
if [ ! -d $t ] ; then
	/bin/mkdir -p $t
	$chown mysql:mysql $t
fi
if [ $force -eq 1 ] ; then
	find "$DESTDIR"$databasedir/data -not -name '*'.pem -exec /bin/rm -rf {} \;
fi
if ( ! /bin/mkdir -p "$DESTDIR"$databasedir/data || ! /bin/mkdir -p "$DESTDIR"$databasedir/logs ) ; then
	echo "Unable to create "$DESTDIR"$databasedir/data or "$DESTDIR"$databasedir/logs" 1>&2
	return 1
fi
(
echo "$RCSID"
echo "# generated on $host at `date`"
echo "# by the below command"
echo "$prog_args"
echo ""
) > "$DESTDIR"$databasedir/.create_db
chmod +x "$DESTDIR"$databasedir/.create_db
$chown mysql:mysql "$DESTDIR"$databasedir
$chown -R mysql:mysql "$DESTDIR"$databasedir/logs
find "$DESTDIR"$databasedir/data -not -name '*'.pem -exec $chown mysql:mysql {} \;
if [ -x $mysqlPrefix/bin/mysql_ssl_rsa_setup ] ; then
	if [ ! -f "$DESTDIR"$databasedir/data/ca.pem ] ; then
		echo "Creating MySQL SSL/TLS Certificates" 1>&2
		echo "$mysqlPrefix/bin/mysql_ssl_rsa_setup --uid=mysql --datadir="$DESTDIR"$databasedir/data" 1>&2
		$mysqlPrefix/bin/mysql_ssl_rsa_setup --uid=mysql --datadir="$DESTDIR"$databasedir/data
		set_mysql_ssl_permission "$DESTDIR"$databasedir/data
	fi
elif [ $mariadb -eq 1 ] ; then
	if [ ! -f "$DESTDIR"$databasedir/ssl/ca.pem ] ; then
		echo "Creating MariaDB SSL/TLS Certificates" 1>&2
		echo "mariadb_ssl_rsa_setup "$DESTDIR"$databasedir/ssl "$DESTDIR"$databasedir/ssl" 1>&2
		mariadb_ssl_rsa_setup "$DESTDIR"$databasedir/ssl "$DESTDIR"$databasedir/ssl
		set_mysql_ssl_permission "$DESTDIR"$databasedir/ssl
	fi
fi
echo "Creating MySQL Database in "$DESTDIR"$databasedir/data" 1>&2
mysqld_server_opt=""
mysql_client_opt=""
if [ $mariadb -eq 1 ] ; then
	major=$(echo $tmysql_version|awk '{print $3}'|cut -d. -f1)
	minor=$(echo $tmysql_version|awk '{print $3}'|cut -d. -f2)
	t=""
	if [ $major -eq 10 ] ; then
		if [ $minor -ge 4 ] ; then
			t="--auth-root-authentication-method=normal"
		fi
	elif [ $major -ge 11 ] ; then # mariadb ssl fails when using self-signed cert
		t="--auth-root-authentication-method=normal"
		mysqld_server_opt="--skip-ssl"
		mysql_client_opt="--skip-ssl-verify-server-cert"
	elif [ $major -gt 10 ] ; then
		t="--auth-root-authentication-method=normal"
	fi
	echo "$install_db $t --user=mysql --basedir=$mysqlPrefix --datadir="$DESTDIR"$databasedir/dbtmp" 1>&2
	$install_db $t --user=mysql --basedir=$mysqlPrefix --datadir="$DESTDIR"$databasedir/dbtmp
	if [ $? -eq 0 ] ; then
		if [ ! -d "$DESTDIR"$databasedir/data ] ; then
			/bin/mv "$DESTDIR"$databasedir/dbtmp "$DESTDIR"$databasedir/data
		else
			/bin/mv "$DESTDIR"$databasedir/dbtmp/* "$DESTDIR"$databasedir/data && \
				/bin/rm -rf "$DESTDIR"$databasedir/dbtmp
		fi
	else
		/bin/rm -rf "$DESTDIR"$databasedir/dbtmp
		echo "WARNING!!! $install_db failed" 1>&2
		return 1
	fi
else
	if [ $mysql_version_8 -eq 1 ] ; then
		opt_str="--user=root"
	else
		opt_str="--user=mysql"
	fi
	/bin/echo -n "$mysqlPrefix/$mysqld --no-defaults --initialize-insecure $opt_str" 1>&2
	/bin/echo -n " --skip-networking" 1>&2
	/bin/echo -n " --log-error="$DESTDIR"$databasedir/logs/mysqld.log"  1>&2
	/bin/echo    " --datadir="$DESTDIR"$databasedir/dbtmp --socket=$mysqlSocket" 1>&2
	$mysqlPrefix/$mysqld --no-defaults --initialize-insecure $opt_str \
		--skip-networking \
		--log-error="$DESTDIR"$databasedir/logs/mysqld.log \
		--datadir="$DESTDIR"$databasedir/dbtmp --socket=$mysqlSocket
	if [ $? -eq 0 ] ; then
		if [ ! -d "$DESTDIR"$databasedir/data ] ; then
			/bin/mv "$DESTDIR"$databasedir/dbtmp "$DESTDIR"$databasedir/data
		else
			/bin/mv "$DESTDIR"$databasedir/dbtmp/* "$DESTDIR"$databasedir/data && \
				/bin/rm -rf "$DESTDIR"$databasedir/dbtmp
		fi
	else
		/bin/rm -rf "$DESTDIR"$databasedir/dbtmp
		if [ -z $install_db ] ; then
			echo "mysqld --initialize failed and couldn't locate db installation program" 1>&2
			return 1
		else
			echo "WARNING!!! mysqld --initialize-insecure failed. Running $install_db" 1>&2
			echo "$install_db $opt_str --basedir=$mysqlPrefix --datadir="$DESTDIR"$databasedir/dbtmp" 1>&2
			$install_db $opt_str --basedir=$mysqlPrefix --datadir="$DESTDIR"$databasedir/dbtmp
			if [ $? -ne 0 ] ; then
				echo "WARNING!!! $install_db too failed. Giving up !!!" 1>&2
				return 1
			fi
		fi
	fi
fi
$chown mysql:mysql "$DESTDIR"$databasedir
$chown -R mysql:mysql "$DESTDIR"$databasedir/logs
find "$DESTDIR"$databasedir/data -not -name '*'.pem -exec $chown mysql:mysql {} \;
#
# Pretty Dumb stuff in packing MySQL on openSUSE, creating /usr/my.cnf.
#
if [ -f /usr/my.cnf ] ; then
	/bin/rm /usr/my.cnf
fi
if [ $mysql_version_8 -eq 1 ] ; then
	pass_str=0
	auth_str=0
	plugin_str=0
elif [ -f "$DESTDIR"$databasedir/data/mysql/user.frm ] ; then
	# Create user mysql with all privileges
	/usr/bin/strings "$DESTDIR"$databasedir/data/mysql/user.frm | grep "^Password" > /dev/null 2>&1
	if [ $? -eq 0 ] ; then
		pass_str=1
	else
		pass_str=0
	fi
	/usr/bin/strings "$DESTDIR"$databasedir/data/mysql/user.frm | grep "^authentication_string" > /dev/null 2>&1
	if [ $? -eq 0 ] ; then
		auth_str=1
	else
		auth_str=0
	fi
	if [ $mariadb -eq 1 ] ; then
		/usr/bin/strings "$DESTDIR"$databasedir/data/mysql/user.frm | grep "^plugin" > /dev/null 2>&1
		if [ $? -eq 0 ] ; then
			plugin_str=1
		else
			plugin_str=0
		fi
	else # from another monkey at oracle
			plugin_str=2
	fi
else # hope for the best
	echo "Unable to create MySQL db in "$DESTDIR"$databasedir/data" 1>&2
	return 1
fi
if [ $mysql_version_8 -eq 1 ] ; then
	opt_str="--user=root"
else
	opt_str="--user=mysql"
fi
#
# Start MySQL daemon
#
/bin/echo -n "$mysqlPrefix/$mysqld --no-defaults --pid-file="$DESTDIR"$databasedir/mysqld.pid" 1>&2
/bin/echo -n " --skip-networking $mysqld_server_opt --datadir="$DESTDIR"$databasedir/data" 1>&2
/bin/echo -n " --log-error="$DESTDIR"$databasedir/logs/mysqld.log" 1>&2
/bin/echo    " $opt_str --socket=$mysqlSocket" 1>&2
$mysqlPrefix/$mysqld --no-defaults --pid-file="$DESTDIR"$databasedir/mysqld.pid \
	--skip-networking $mysqld_server_opt --datadir="$DESTDIR"$databasedir/data \
	--log-error="$DESTDIR"$databasedir/logs/mysqld.log \
	$opt_str --socket=$mysqlSocket &
wait_for_mysqld
if [ ! -s "$DESTDIR"$databasedir/mysqld.pid ] ; then
	echo "did not find running mysqld: "$DESTDIR"$databasedir/mysqld.pid not found" 1>&2
	return 1
fi
pid=$(@prefix@/bin/qcat "$DESTDIR"$databasedir/mysqld.pid)
if [ -x $mysqlPrefix/bin/mariadb ] ; then
	mysql=$mysqlPrefix/bin/mariadb
else
	mysql=$mysqlPrefix/bin/mysql
fi
(
# Create user mysql with all privileges
echo "Creating MySQL admin User 'mysql' for Database in "$DESTDIR"$databasedir/data" 1>&2
create_mysql_rootuser $mysql_version $mariadb $mysql_community_server $pass_str $auth_str $plugin_str
echo "Creating MySQL indimail User '$user' for Database in "$DESTDIR"$databasedir/data" 1>&2
create_mysql_user "$user" "$pass" "$mysql_version"
) | eval $mysql -u root --skip-password $mysql_client_opt -S $mysqlSocket
kill $pid
wait_for_mysqld 1
$chown mysql:mysql "$DESTDIR"$databasedir
$chown -R mysql:mysql "$DESTDIR"$databasedir/logs
find "$DESTDIR"$databasedir/data -not -name '*'.pem -exec $chown mysql:mysql {} \;
}

check_mysqld_variable()
{
if [ $# -ne 2 ] ; then
	echo "USAGE: check_mysqld_variable variable_name print" 1>&2
	return 1
fi
if [ -x $mysqlPrefix/sbin/mariadbd ] ; then
	mysqld=sbin/mariadbd
elif [ -x $mysqlPrefix/libexec/mysqld ] ; then
	mysqld=libexec/mysqld
elif [ -x $mysqlPrefix/sbin/mysqld ] ; then
	mysqld=sbin/mysqld
elif [ -x $mysqlPrefix/bin/mysqld ] ; then
	mysqld=bin/mysqld
else
	echo "mysqld: No such File or directory" 1>&2
	return 1
fi
name=$(echo $1 | cut -d= -f1)
tmp=$(echo $1 | cut -d= -f2)
if [ "$name" = "$tmp" ] ; then
	tmp=""
fi
should_print=$2
if [ -n "$tmp" ] ; then
	$mysqlPrefix/$mysqld --no-defaults --verbose --help 2>/dev/null | grep -- "$name=" >/dev/null 2>&1 || \
		$mysqlPrefix/$mysqld --no-defaults --verbose --help 2>/dev/null | grep -- "$name\[=" >/dev/null 2>&1
	ret=$?
else
	$mysqlPrefix/$mysqld --no-defaults --verbose --help 2>/dev/null | grep -- "$name " >/dev/null 2>&1
	ret=$?
fi
if [ $ret -eq 0 ] ; then
	if [ $should_print -ne 0 ] ; then
		echo "$1 \\"
	fi
	return 0
else
	return 1
fi
}

check_mysqld_ini_variable()
{
if [ $# -ne 2 ] ; then
	echo "USAGE: check_mysqld_ini_variable variable_name print" 1>&2
	return 1
fi
if [ -x $mysqlPrefix/sbin/mariadbd ] ; then
	mysqld=sbin/mariadbd
elif [ -x $mysqlPrefix/libexec/mysqld ] ; then
	mysqld=libexec/mysqld
elif [ -x $mysqlPrefix/sbin/mysqld ] ; then
	mysqld=sbin/mysqld
elif [ -x $mysqlPrefix/bin/mysqld ] ; then
	mysqld=bin/mysqld
else
	echo "mysqld: No such File or directory" 1>&2
	return 1
fi
name=`echo $1 | cut -d= -f1`
should_print=$2
$mysqlPrefix/$mysqld --no-defaults --verbose --help 2>/dev/null | grep -w "^$name " > /dev/null 2>&1
if [ $? -eq 0 ] ; then
	if [ $should_print -ne 0 ] ; then
		echo "$1"
	fi
	return 0
else
	echo $name | grep "_" > /dev/null 2>&1
	if [ $? -ne 0 ] ; then
		return 1
	fi
	# we found underscore in the variable name
	# mysqld variables have hyphen '-' to its equivalent ini variable.
	# replace '_' with '-' and perform check
	# replace underscores only in the variable name and not in the
	# variable value
	orig=`echo $name` # remove trailing space in $name, if present
	name=`echo $name|sed s{_{-{g`
	$mysqlPrefix/$mysqld --no-defaults --verbose --help 2>/dev/null |grep -w "^$name" > /dev/null 2>&1
	if [ $? -eq 0 ] ; then
		if [ $should_print -eq 1 ] ; then
			echo "$1" | sed s{"$orig"{"$name"{g
		elif [ $should_print -eq 2 ] ; then
			/bin/echo -n "# "
			echo "$1" | sed s{"$orig"{"$name"{g
		fi
		return 0
	fi
	return 1
fi
}

create_mysql_rootuser()
{
	mysql_version=$1
	mariadb=$2
	mysql_community_server=$3
	pass_str=$4
	auth_str=$5
	plugin_str=$6
	mysql_version_8=0
	mariadb_version_10=0
	if [ $mariadb -eq 1 ] ; then
		major=$(echo $mysql_version | cut -d. -f1)
	fi
	case "$mysql_version" in
		8.*)
		mysql_version_8=1
		;;
		10.*)
		if [ $mariadb -eq 1 ] ; then
			mariadb_version_10=1
		fi
		;;
	esac
	if [ $mariadb -eq 1 ] ; then
		if [ $major -gt 10 ] ; then
			mecho "ALTER USER 'root'@'localhost' identified by '$PRIV_PASS';"
		elif [ $mariadb_version_10 ] ; then
			ver2=`echo $mysql_version | cut -d. -f2`
			if [ -n "$ver2" -a $ver2 -ge 2 ] ; then
				mecho "ALTER USER 'root'@'localhost' identified by '$PRIV_PASS';"
			else
				mecho "set PASSWORD for 'root'@'localhost' = PASSWORD('$PRIV_PASS');"
			fi
		else
			mecho "set PASSWORD for 'root'@'localhost' = PASSWORD('$PRIV_PASS');"
		fi
	else
		if [ $mysql_version_8 -eq 1 ] ; then
			mecho "ALTER USER 'root'@'localhost' identified by '$PRIV_PASS';"
		else
			mecho "set PASSWORD for 'root'@'localhost' = PASSWORD('$PRIV_PASS');"
		fi
	fi
	mecho "use mysql;"
	if [ $mariadb -eq 0 ] ; then
		mecho "drop procedure if exists remove_unsafe;"
		mecho "delimiter ';;'"
		mecho "create procedure remove_unsafe()"
		mecho "begin"
		mecho "  if exists (select * from information_schema.columns where table_name = 'user' and column_name = 'password') then"
		mecho "    DELETE FROM user where user='' or password='';"
		mecho "  ELSE"
		mecho "    DELETE FROM user where user='';"
		mecho "  end if;"
		mecho "end;;"
		mecho "delimiter \';\'"
		mecho "call remove_unsafe();"
		mecho "drop procedure if exists remove_unsafe;"
	else
		mecho "DELETE FROM user where user='';"
	fi
	mecho "DROP database if exists test;"
	mecho "DELETE from db where db like 'test%';"
}

create_mysql_user()
{
	if [ ! -x /usr/bin/openssl ] ; then
		echo "/usr/bin/openssl: No such file or directory" 1>&2
		return 1
	fi
	mysql_user=$1
	mysql_pass=$2
	mysql_version=$3
	if [ $mariadb -eq 1 ] ; then
		major=$(echo $mysql_version | cut -d. -f1)
	fi
	case "$mysql_version" in
		8.*)
			mysql_version_8=1
			;;
		7.*)
			mysql_version_8=0
			;;
	esac

	# for non-indimail database, just create admin, repl and remove root user
	if [ -z "$mysql_user" ] ; then
		mecho "use mysql;"
		mecho "CREATE USER 'admin'@'%'    identified by '$ADMIN_PASS';"
		mecho "CREATE USER 'repl'@'%'     identified by 'slaveserver';"
		if [ $mariadb -eq 1 ] ; then
			if [ $major -gt 10 ] ; then
				mecho "DROP USER if exists mysql@localhost;"
			elif [ $mariadb_version_10 -eq 1 ] ; then
				ver2=`echo $mysql_version | cut -d. -f2`
				if [ -n "$ver2" -a $ver2 -gt 0 ] ; then
					mecho "DROP USER if exists mysql@localhost;"
				fi
			fi
		fi
		mecho "CREATE USER 'mysql'@'%' identified by '$PRIV_PASS';"
		mecho "GRANT ALL on *.* TO mysql;"
		if [ $mysql_version_8 -eq 1 ] ; then
			mecho "GRANT USAGE ON *.* TO mysql WITH GRANT OPTION;"
		elif [ $mariadb -eq 1 ] ; then
			if [ $mariadb_version_10 -eq 1 -o $major -gt 10 ] ; then
				mecho "GRANT USAGE ON *.* TO mysql WITH GRANT OPTION;"
			fi
		fi
		mecho "GRANT RELOAD,SHUTDOWN,PROCESS,SUPER on *.* to admin;"
		mecho "GRANT REPLICATION SLAVE on *.* to repl;"
		if [ $mysql_version_8 -eq 1 ] ; then
			c_d=""
			c_d="$c_d UPDATE global_grants set user='mysql' where user='root';"
			mecho $c_d
		fi
		mecho "FLUSH PRIVILEGES;"
		return 0
	fi
	# Bootstrap Indimail tables
	mecho "CREATE database indimail;"
	mecho "use indimail;"
	c_d=""
	c_d="$c_d CREATE TABLE mgmtaccess ("
	c_d="$c_d user  char(32) not null,"
	c_d="$c_d pass char(128) not null,"
	c_d="$c_d pw_uid int not null,"
	c_d="$c_d pw_gid int not null,"
	c_d="$c_d lastaccess int not null,"
	c_d="$c_d lastupdate int not null,"
	c_d="$c_d day char(2) not null,"
	c_d="$c_d attempts int not null,"
	c_d="$c_d status char(2) not null,"
	c_d="$c_d zztimestamp TIMESTAMP not null,"
	c_d="$c_d unique index(user));"
	mecho $c_d
	case "$host" in
		*-*-darwin*)
		TMP=$(head -c 8192 /dev/urandom | env LC_ALL=C tr -dc 'a-zA-Z0-9./' | head -c 8)
		CRYPT_PASS=$(openssl passwd -1 -salt $TMP $ADMIN_PASS)
		;;
		*)
		TMP=$(head -c 8192 /dev/urandom | env LC_TYPE=C tr -dc 'a-zA-Z0-9./' | head -c 16)
		CRYPT_PASS=$(openssl passwd -6 -salt $TMP $ADMIN_PASS)
		;;
	esac
	TMVAL=`date +'%s'`
	DAY=`date +'%d'`
	TIMESTAMP=`date +'%F %R:%S'`
	mecho "INSERT INTO mgmtaccess (user,pass,pw_uid,pw_gid,lastaccess,lastupdate,day,attempts,status,zztimestamp) VALUES ('admin','$CRYPT_PASS',0,0,$TMVAL,$TMVAL,$DAY,0,0,'$TIMESTAMP');"
	c_d=""
	c_d="$c_d CREATE TABLE indimail ("
	c_d="$c_d pw_name char(40) not null,"
	c_d="$c_d pw_domain char(67) not null,"
	c_d="$c_d pw_passwd char(128) not null,"
	c_d="$c_d pw_uid int,"
	c_d="$c_d pw_gid int,"
	c_d="$c_d pw_gecos char(48) not null,"
	c_d="$c_d pw_dir char(156),"
	c_d="$c_d pw_shell char(30),"
	c_d="$c_d scram char(255),"
	c_d="$c_d salted char(128),"
	c_d="$c_d timestamp TIMESTAMP DEFAULT CURRENT_TIMESTAMP on update CURRENT_TIMESTAMP not null,"
	c_d="$c_d primary key (pw_name, pw_domain), index pw_gecos (pw_gecos(25)), index pw_uid (pw_uid));"
	mecho $c_d
	c_d=""
	c_d="$c_d CREATE TABLE indibak ("
	c_d="$c_d pw_name char(40) not null,"
	c_d="$c_d pw_domain char(67) not null,"
	c_d="$c_d pw_passwd char(128) not null,"
	c_d="$c_d pw_uid int,"
	c_d="$c_d pw_gid int,"
	c_d="$c_d pw_gecos char(48) not null,"
	c_d="$c_d pw_dir char(156),"
	c_d="$c_d pw_shell char(30),"
	c_d="$c_d scram char(255),"
	c_d="$c_d salted char(128),"
	c_d="$c_d timestamp TIMESTAMP DEFAULT CURRENT_TIMESTAMP on update CURRENT_TIMESTAMP not null,"
	c_d="$c_d primary key (pw_name, pw_domain), index pw_gecos (pw_gecos(25)), index pw_uid (pw_uid));"
	mecho $c_d
	c_d=""
	c_d="$c_d CREATE TABLE vfilter ("
	c_d="$c_d emailid char(107) not null,"
	c_d="$c_d filter_no smallint not null,"
	c_d="$c_d filter_name char(32) not null,"
	c_d="$c_d header_name smallint not null,"
	c_d="$c_d comparision tinyint not null,"
	c_d="$c_d keyword char(64) not null,"
	c_d="$c_d destination char(156) not null,"
	c_d="$c_d bounce_action char(64) not null,"
	c_d="$c_d primary key(emailid, filter_no), unique index (emailid, header_name, comparision, keyword, destination));"
	mecho $c_d

	mecho "use mysql;"
	mecho "CREATE USER '$mysql_user'@'%' identified by '$mysql_pass';"
	mecho "CREATE USER 'admin'@'%'    identified by '$ADMIN_PASS';"
	mecho "CREATE USER 'repl'@'%'     identified by 'slaveserver';"
	if [ $mariadb -eq 1 ] ; then
		if [ $major -gt 10 ] ; then
			mecho "DROP USER if exists mysql@localhost;"
		elif [ $mariadb_version_10 -eq 1 ] ; then
			ver2=`echo $mysql_version | cut -d. -f2`
			if [ -n "$ver2" -a $ver2 -gt 0 ] ; then
				mecho "DROP USER if exists mysql@localhost;"
			fi
		fi
	fi
	mecho "CREATE USER 'mysql'@'%' identified by '$PRIV_PASS';"
	mecho "GRANT ALL on *.* TO mysql;"
	if [ $mysql_version_8 -eq 1 ] ; then
		mecho "GRANT USAGE ON *.* TO mysql WITH GRANT OPTION;"
	elif [ $mariadb -eq 1 ] ; then
		if [ $mariadb_version_10 -eq 1 -o $major -gt 10 ] ; then
			mecho "GRANT USAGE ON *.* TO mysql WITH GRANT OPTION;"
		fi
	fi
	mecho "GRANT SELECT,CREATE,ALTER,INDEX,INSERT,UPDATE,DELETE,CREATE TEMPORARY TABLES,LOCK TABLES ON indimail.* to indimail;"
	mecho "GRANT RELOAD,SHUTDOWN,PROCESS,SUPER on *.* to admin;"
	mecho "GRANT REPLICATION SLAVE on *.* to repl;"
	if [ $mysql_version_8 -eq 1 ] ; then
		c_d=""
		c_d="$c_d UPDATE global_grants set user='mysql' where user='root';"
		mecho $c_d
	fi
	mecho "FLUSH PRIVILEGES;"
}

create_mysql_service()
{
if [ $# -ne 5 ] ; then
	echo "USAGE: create_mysql_service port mysqlPrefix databasedir config_file servicedir" 1>&2
	return 1
fi
port=$1
mysqlPrefix=$2
databasedir=$3
conf_file=$4
SERVICEDIR=$5
if [ $nooverwrite -eq 1 -a -d "$DESTDIR"$SERVICEDIR/mysql.$port/variables ] ; then
	return 0
fi
if [ -x $mysqlPrefix/sbin/mariadbd ] ; then
	mysqld=sbin/mariadbd
elif [ -x $mysqlPrefix/libexec/mysqld ] ; then
	mysqld=libexec/mysqld
elif [ -x $mysqlPrefix/sbin/mysqld ] ; then
	mysqld=sbin/mysqld
elif [ -x $mysqlPrefix/bin/mysqld ] ; then
	mysqld=bin/mysqld
else
	echo "mysqld: No such File or directory" 1>&2
	return 1
fi
mysql_version=`$mysqlPrefix/$mysqld --version 2>&1 | grep Ver | awk '{print $3}'`
case "$mysql_version" in
	8.*)
		mysql_version_8=1
		;;
	7.*)
		mysql_version_8=0
		;;
esac
mysql_client_opt=""
echo $mysql_version |grep MariaDB > /dev/null 2>&1
if [ $? -eq 0 ] ; then
	major=$(echo $mysql_version|cut -d. -f1)
	if [ $major -ge 11 ] ; then # mariadb ssl fails when using self-signed cert
		mysql_client_opt="--skip-ssl-verify-server-cert"
	fi
	mariadb=1
else
	mariadb=0
fi
if [ ! -f "$DESTDIR"$conf_file ] ; then
	echo ""$DESTDIR"$conf_file: No such File or directory" 1>&2
	return 1
fi
conf_dir="$DESTDIR"$SERVICEDIR/mysql.$port/variables
if [ $run_file_only -eq 1 ] ; then
	if [ -d $conf_dir ] ; then
		/bin/mv $conf_dir "$conf_dir".bak
	fi
fi
/bin/mkdir -p "$DESTDIR"$SERVICEDIR/mysql.$port/log
if [ $down_state -eq 1 ] ; then
	$TOUCH "$DESTDIR"$SERVICEDIR/$mysql.$port/down
fi
/bin/mkdir -p $conf_dir
/bin/chmod 500 $conf_dir
if [ -z "$mysqlSocket" ]; then
	if [ -n "$MYSQL_SOCKET" ] ; then
		mysqlSocket=$MYSQL_SOCKET
	elif [ -d /run ] ; then
		mysqlSocket=/run/mysqld/mysqld.sock
	elif [ -d /var/run ] ; then
		mysqlSocket=/var/run/mysqld/mysqld.sock
	elif [ -d /var/lib/mysql ] ; then
		mysqlSocket=/var/lib/mysql/mysql.sock
	else
		mysqlSocket=/run/mysqld/mysqld.sock
	fi
fi
(
dump_run_header
echo "exec 2>&1"
echo "socket_dir=\`dirname \$(grep \"socket.* = \" $conf_file 2>/dev/null| head -1 | cut -d= -f2) 2>/dev/null\`"
echo "if [ -z \"\$socket_dir\" ] ; then"
echo "    socket_dir=`dirname $mysqlSocket`"
echo "fi"
echo "if [ ! -d \$socket_dir ] ; then"
echo "    /bin/mkdir \$socket_dir"
echo "    $chown mysql:mysql \$socket_dir"
echo "fi"
echo "MYSQL_BASE=$mysqlPrefix"
echo "DATABASE=$databasedir"
echo "PIDFILE=\$socket_dir/mysqld."$port".pid"
echo ""
echo "# update $sysconfdir/control/libmysql with the latest MySQL shared lib"
echo "$QmailBinPrefix/sbin/svctool --fixsharedlibs"
echo ""
echo "exec $QmailBinPrefix/bin/softlimit -o 1024 -p 1024 \\"
echo "$QmailBinPrefix/bin/setuidgid mysql \$MYSQL_BASE/$mysqld --defaults-file=$conf_file \\"
echo "--port=$port --basedir=\$MYSQL_BASE \\"
echo "--datadir=\$DATABASE/data \\"
#check_mysqld_variable --mysql-native-password=ON 1 # this is deprecated
check_mysqld_variable --mysqlx=OFF 1
check_mysqld_variable --memlock 1
check_mysqld_variable --ssl 1
check_mysqld_variable --require-secure-transport 1
# skip external locking is not shown in the list of variables
echo "--skip-external-locking \\"
check_mysqld_variable --delay-key-write=all 1
check_mysqld_variable --skip-name-resolve 1
echo "--sql-mode=\"NO_ENGINE_SUBSTITUTION,STRICT_TRANS_TABLES\" \\"
check_mysqld_variable --explicit-defaults-for-timestamp=TRUE 1
if [ $? -ne 0 ] ; then
	check_mysqld_variable --explicit-defaults-for-timestamp 1
fi
check_mysqld_variable --binlog-expire-logs-auto-purge=ON 1
if [ $? -ne 0 ] ; then
	check_mysqld_variable --binlog-expire-logs-auto-purge 1
	if [ $? -eq 0 ] ; then
		check_mysqld_variable --binlog-expire-logs-seconds=864000 1
	fi
else
	check_mysqld_variable --binlog-expire-logs-seconds=864000 1
fi
check_mysqld_variable --local-infile 1
#
# On Unix systems, if the --log-error option is not used,
# the errors are written to stderr (usually, the command line).
# Few mysql-community servers have a bug where if you don't have --log-error, you get
# Error-log destination "stderr" is not a file. Can not restore error log messages from previous run.
#
# check_mysqld_variable --log-error=\$DATABASE/logs/mysql-error.log 1
check_mysqld_variable --log-error-verbosity=2 1
if [ $mariadb -eq 1 ] ; then
	check_mysqld_variable --log-warnings=2 1
fi
check_mysqld_variable --log-output=NONE 1
check_mysqld_variable --general-log 1
check_mysqld_variable --general-log-file=\$DATABASE/logs/general.log 1
check_mysqld_variable --slow-query-log 1
check_mysqld_variable --slow-query-log-file=\$DATABASE/logs/slowquery.log 1
check_mysqld_variable --log-queries-not-using-indexes 1
echo "--pid-file=\$PIDFILE 2>&1"
) > "$DESTDIR"$SERVICEDIR/mysql.$port/run
/bin/chmod +x "$DESTDIR"$SERVICEDIR/mysql.$port/run

(
dump_run_header
echo "MYSQL_BASE=$mysqlPrefix"
echo ""
echo "if [ -x \$MYSQL_BASE/bin/mariadb-admin ] ; then"
echo "  mysqladmin=\$MYSQL_BASE/bin/mariadb-admin"
echo "else"
echo "  mysqladmin=\$MYSQL_BASE/bin/mysqladmin"
echo "fi"
echo "exec 2>&1"
echo "exec \$mysqladmin --defaults-file=$conf_file \\"
echo "  -u admin -p$ADMIN_PASS $mysql_client_opt shutdown"
) > "$DESTDIR"$SERVICEDIR/mysql.$port/shutdown
/bin/chmod 500 "$DESTDIR"$SERVICEDIR/mysql.$port/shutdown

if [ ! -d $LOGDIR ] ; then
	/bin/mkdir -p $LOGDIR
	$chown qmaill:nofiles $LOGDIR
fi

# mysql log script
dump_log_header mysql.$port > "$DESTDIR"$SERVICEDIR/mysql.$port/log/run
/bin/chmod +x "$DESTDIR"$SERVICEDIR/mysql.$port/log/run
echo "$prog_args" > $conf_dir/.options

if [ $run_file_only -eq 1 ] ; then
	if [ -d "$conf_dir".bak ] ; then
		/bin/rm -rf $conf_dir
		/bin/mv "$conf_dir".bak $conf_dir
	fi
fi

if [ $force -eq 1 -o ! -f $sysconfdir/logrotate.mysql ] ; then
	(
	echo "$RCSID"
	echo "# generated on $host at `date`"
	echo "# by the below command"
	echo "# $prog_args"
	echo ""
	echo "# mysql logrotate config generated by svctool"
	echo ""
	echo "# The log file name and location can be set in"
	echo "# $sysconfdir/indimail.cnf by setting the \"log-error\" option"
	echo "# in [mysqld]  section as follows:"
	echo "#"
	echo "# [mysqld]"
	echo "# log-error=/var/log/mysqld.log"
	echo "#"
	echo "# For the mysqladmin commands below to work, root account"
	echo "# password is required. Use mysql_config_editor(1) to store"
	echo "# authentication credentials in the encrypted login path file"
	echo "# ~/.mylogin.cnf"
	echo "#"
	echo "# Example usage:"
	echo "#"
	echo "#  mysql_config_editor set --login-path=client --user=root --host=localhost --password"
	echo "#"
	echo ""
	echo "$databasedir/logs/*.log {"
	echo "    create 640 mysql mysql"
	echo "    notifempty"
	echo "    daily"
	echo "    rotate 5"
	echo "    missingok"
	echo "    compress"
	echo "  postrotate"
	echo "    # just if mysqld is really running"
	echo "    if [ -x /usr/bin/mariadb-admin ] ; then"
	echo "      mysqladmin=$mysqlPrefix/bin/mariadb-admin"
	echo "    else"
	echo "      mysqladmin=$mysqlPrefix/bin/mysqladmin"
	echo "    fi"
	echo "    if test -x $mysqladmin && \\"
	echo "       $mysqladmin ping &>/dev/null"
	echo "    then"
	echo "      $mysqladmin --defaults-file=$sysconfdir/indimail.cnf \\"
	echo "        -u admin -p$ADMIN_PASS flush-logs"
	echo "    fi"
	echo "  endscript"
	echo "}"
	) > "$DESTDIR"$sysconfdir/logrotate.mysql
	chown root:root "$DESTDIR"$sysconfdir/logrotate.mysql
	/bin/chmod 600 "$DESTDIR"$sysconfdir/logrotate.mysql
fi
}

create_mysql_config()
{
if [ -x $mysqlPrefix/sbin/mariadbd ] ; then
	mysqld=sbin/mariadbd
elif [ -x $mysqlPrefix/libexec/mysqld ] ; then
	mysqld=libexec/mysqld
elif [ -x $mysqlPrefix/sbin/mysqld ] ; then
	mysqld=sbin/mysqld
elif [ -x $mysqlPrefix/bin/mysqld ] ; then
	mysqld=bin/mysqld
else
	echo "mysqld: No such File or directory" 1>&2
	return 1
fi
if [ -z "$mysqlSocket" ]; then
	if [ -n "$MYSQL_SOCKET" ] ; then
		mysqlSocket=$MYSQL_SOCKET
	elif [ -d /run ] ; then
		mysqlSocket=/run/mysqld/mysqld.sock
	elif [ -d /var/run ] ; then
		mysqlSocket=/var/run/mysqld/mysqld.sock
	elif [ -d /var/lib/mysql ] ; then
		mysqlSocket=/var/lib/mysql/mysql.sock
	else
		mysqlSocket=/run/mysqld/mysqld.sock
	fi
fi

mysql_version=`$mysqlPrefix/$mysqld --version 2>&1 | grep Ver | awk '{print $3}'`
echo $mysql_version |grep MariaDB > /dev/null 2>&1
if [ $? -eq 0 ] ; then
	mysql_version=`echo $mysql_version|cut -d- -f1`
	mariadb=1
else
	mariadb=0
fi
if [ -x $mysqlPrefix/bin/mariadb ] ; then
	mysql=$mysqlPrefix/bin/mariadb
else
	mysql=$mysqlPrefix/bin/mysql
fi
$mysql --version 2>/dev/null | grep MariaDB >/dev/null 2>&1
if [ $? -eq 0 ] ; then
	mariadb_client=1
else
	mariadb_client=0
fi

if [ -x $mysqlPrefix/bin/mysql_ssl_rsa_setup ] ; then
	if [ $force -eq 1 ] ; then
		find "$DESTDIR"$databasedir/data -name '*'.pem -exec /bin/rm -rf {} \;
	fi
	if ( ! /bin/mkdir -p "$DESTDIR"$databasedir/data ) ; then
		echo "Unable to create "$DESTDIR"$databasedir/data for MySQL Certificates" 1>&2
		return 1
	fi
	if [ ! -f "$DESTDIR"$databasedir/data/ca.pem ] ; then
		echo "Creating MySQL SSL/TLS Certificates" 1>&2
		echo "$mysqlPrefix/bin/mysql_ssl_rsa_setup --uid=mysql --datadir="$DESTDIR"$databasedir/data" 1>&2
		$mysqlPrefix/bin/mysql_ssl_rsa_setup --uid=mysql --datadir="$DESTDIR"$databasedir/data 1>&2
	else
		echo "MySQL SSL/TLS Certificates exists!!!" 1>&2
	fi
elif [ $mariadb -eq 1 ] ; then
	if [ $force -eq 1 ] ; then
		find "$DESTDIR"$databasedir/ssl -name '*'.pem -exec /bin/rm -rf {} \;
	fi
	if ( ! /bin/mkdir -p "$DESTDIR"$databasedir/ssl ) ; then
		echo "Unable to create "$DESTDIR"$databasedir/ssl for MariaDB Certificates" 1>&2
		return 1
	fi
	if [ ! -f "$DESTDIR"$databasedir/ssl/ca.pem ] ; then
		echo "Creating MariaDB SSL/TLS Certificates" 1>&2
		echo "mariadb_ssl_rsa_setup "$DESTDIR"$databasedir/ssl "$DESTDIR"$databasedir/ssl" 1>&2
		mariadb_ssl_rsa_setup "$DESTDIR"$databasedir/ssl "$DESTDIR"$databasedir/ssl 1>&2
	else
		echo "MariaDB SSL/TLS Certificates exists!!!" 1>&2
	fi
fi

echo "$RCSID"
echo "# generated on $host at `date`"
echo "# by the below command"
echo "# $prog_args"
echo ""
echo "[client]"
echo "socket    = $mysqlSocket"
echo "port      = $mysql_port"
check_mysqld_ini_variable "default_character_set=utf8mb4" 1
if [ -d "$DESTDIR"$databasedir/ssl ] ; then
	echo ""
	echo "# MySQL Client SSL configuration"
	echo "ssl-ca=$databasedir/ssl/ca.pem"
	echo "ssl-cert=$databasedir/ssl/client-cert.pem"
	echo "ssl-key=$databasedir/ssl/client-key.pem"
	echo "# This option is disabled by default"
	echo "#ssl-verify-server-cert"
	echo ""
fi
echo ""
echo "[mysqld]"
echo "#"
echo "# * Basic Settings"
echo "#"
echo
echo "#"
echo "# * IMPORTANT"
echo "#   If you make changes to these settings and your system uses apparmor, you may"
echo "#   also need to also adjust /etc/apparmor.d/usr.sbin.mysqld."
echo "#"
echo
echo "#default-authentication-plugin=mysql_native_password"
echo "#sql_mode=\"NO_ENGINE_SUBSTITUTION,NO_ZERO_DATE,NO_ZERO_IN_DATE,ERROR_FOR_DIVISION_BY_ZERO,NO_AUTO_CREATE_USER,STRICT_ALL_TABLES\""
echo "#sql_mode=\"NO_ENGINE_SUBSTITUTION,STRICT_TRANS_TABLES\""
if [ $mariadb -eq 1 -a -d "$DESTDIR"$databasedir/ssl ] ; then
	check_mysqld_variable --ssl 0
	if [ $? -eq 0 ] ; then
		echo ""
		echo "# MySQL Server SSL configuration"
		echo "# Securing the Database with ssl option and certificates"
		echo "# There is no control over the protocol level used."
		echo "# mariadb will use TLSv1.0 or better."
		echo "ssl"
		echo "ssl-ca=$databasedir/ssl/ca.pem"
		echo "ssl-cert=$databasedir/ssl/server-cert.pem"
		echo "ssl-key=$databasedir/ssl/server-key.pem"
		echo ""
	fi
else
	check_mysqld_ini_variable "ssl" 1
fi
check_mysqld_ini_variable "require_secure_transport = ON" 1
check_mysqld_ini_variable "mysqlx = OFF" 1
check_mysqld_ini_variable "explicit_defaults_for_timestamp=TRUE" 1
echo "user     = mysql"
echo "socket   = $mysqlSocket"
echo "port     = $mysql_port"
echo "basedir  = $mysqlPrefix"
echo "datadir  = $databasedir/data"
check_mysqld_ini_variable "character_set_client_handshake = FALSE" 1
check_mysqld_ini_variable "character_set_server = utf8mb4" 1
check_mysqld_ini_variable "collation_server = utf8mb4_unicode_ci" 1
echo
echo "#Description: If set to 1, LOCAL is supported for LOAD DATA INFILE statements."
echo "#If set to 0 (default), usually for security reasons, attempts to perform a"
echo "#LOAD DATA LOCAL will fail with an error message."
check_mysqld_ini_variable "local_infile = TRUE" 1
echo
echo "[inlookup]"
echo "#The number of seconds the server waits for activity on an"
echo "#interactive connection before closing it. An interactive client is"
echo "#defined as a client that uses the 'CLIENT_INTERACTIVE' option to connect"
echo "interactive_timeout=28880"
echo
echo "#The number of seconds to wait for more data from a connection"
echo "#before aborting the read. This timeout applies only to TCP/IP"
echo "#connections, not to connections made via Unix socket files, named"
echo "#pipes, or shared memory."
echo "net_read_timeout=5"
echo
echo "#The number of seconds to wait for a block to be written to a"
echo "#connection before aborting the write. This timeout applies only to"
echo "#TCP/IP connections, not to connections made via Unix socket files,"
echo "#named pipes, or shared memory."
echo "net_write_timeout=5"
echo
echo "#The number of seconds the server waits for activity on a"
echo "#non-interactive connection before closing it. This timeout applies"
echo "#only to TCP/IP and Unix socket file connections, not to"
echo "#connections made via named pipes, or shared memory."
echo "wait_timeout=28800"
}

create_svscan()
{
if [ $# -lt 1 ] ; then
	echo "USAGE: create_svscan supervise_dir [initcmd]" 1>&2
	return 1
fi
SERVICEDIR=$1
if [ $# -eq 2 ] ; then
	init_cmd=$2
else
	init_cmd=""
fi
if [ $nooverwrite -eq 1 -a -d "$DESTDIR"$SERVICEDIR/.svscan/log ] ; then
	return 0
fi
if [ $mount_resolvconf -eq 1 ] ; then
	if [ ! -x /usr/bin/unshare ] ; then
		echo "/usr/bin/unshare: No access or unshare command missing" 1>&2
		return 1
	fi
	if [ ! -x "$DESTDIR"$SERVICEDIR/dnscache/run ] ; then
		echo "djb dnscache service is required for --resolvconf" 1>&2
		return 1
	fi
fi
# Setup variables INITCMD, SCANINTERVAL, SCANLOG, UNSHARE
conf_dir="$DESTDIR"$SERVICEDIR/.svscan/variables
if [ $run_file_only -eq 1 ] ; then
	if [ -d $conf_dir ] ; then
		/bin/mv $conf_dir "$conf_dir".bak
	fi
fi
/bin/mkdir -p $conf_dir
/bin/chmod 500 $conf_dir
link_with_global $conf_dir
echo "$prog_args" > $conf_dir/.options
if [ -z $init_cmd ] ; then
	echo           > $conf_dir/INITCMD
else
	echo $init_cmd > $conf_dir/INITCMD
fi
echo "/usr/bin:/bin:/usr/local/bin:/usr/sbin:/sbin:/usr/local/sbin" > $conf_dir/PATH
if [ " $scan_interval" = " " ] ; then
	echo           "300"  > $conf_dir/SCANINTERVAL
else
	echo "$scan_interval" > $conf_dir/SCANINTERVAL
fi
echo > $conf_dir/SCANLOG
if [ -x /usr/bin/unshare -a $use_unshare -eq 1 ] ; then
	echo > $conf_dir/UNSHARE
else
	> $conf_dir/UNSHARE
fi

if [ -x /usr/bin/mount ] ; then
	mount=/usr/bin/mount
elif [ -x /bin/mount ] ; then
	mount=/bin/mount
elif [ -x /sbin/mount ] ; then
	mount=/sbin/mount
else
	echo "mount command not found" 1>&2
	return 1
fi
if [ -x /usr/bin/umount ] ; then
	umount=/usr/bin/umount
elif [ -x /bin/umount ] ; then
	umount=/bin/umount
elif [ -x /sbin/umount ] ; then
	umount=/sbin/umount
else
	echo "umount command not found" 1>&2
	return 1
fi
echo 1 > $conf_dir/AUTOSCAN
/bin/mkdir -p "$DESTDIR"$SERVICEDIR/.svscan/log
/bin/mkdir -p "$DESTDIR"$sysconfdir
if [ $mount_resolvconf -eq 1 ] ; then
	# svscan run script
	(
	dump_run_header
	echo "echo svscan \$PPID initialization PID \$\$"
	echo "exec 2>&1"
	echo "if [ -s $sysconfdir/variables/UNSHARE -a -f $sysconfdir/resolv.conf ] ; then"
	echo "  /usr/bin/mount --bind $sysconfdir/resolv.conf /etc/resolv.conf"
	echo "  /usr/bin/mount -l"
	echo "fi"
	echo ""
	echo "echo \"Environment Variable List\""
	echo "env"
	echo "echo"
	) > "$DESTDIR"$SERVICEDIR/.svscan/run
	if [ ! -f "$DESTDIR"$sysconfdir/resolv.conf -o $force -eq 1 ] ; then
		hostname=$([ -n "$HOSTNAME" ] && echo "$HOSTNAME" || uname -n)
		(
		echo "# $prog_args"
		echo "# generated at `date`"
		echo "search $hostname"
		echo "nameserver 127.0.0.1"
		) > "$DESTDIR"$sysconfdir/resolv.conf
	fi
	/bin/chmod +x "$DESTDIR"$SERVICEDIR/.svscan/run
else
	if [ -x "$DESTDIR"$SERVICEDIR/.svscan/run ] ; then
		/bin/chmod -x "$DESTDIR"$SERVICEDIR/.svscan/run
	fi
fi

(
dump_run_header
echo "exec 2>&1"
if [ $mount_resolvconf -eq 1 ] ; then
	echo "if [ -s $SERVICEDIR/variables/UNSHARE -a -f $sysconfdir/resolv.conf ] ; then"
	echo "  /usr/bin/umount /etc/resolv.conf"
	echo "fi"
fi
echo "$prefix/bin/svc -dx $SERVICEDIR/*"
) > "$DESTDIR"$SERVICEDIR/.svscan/shutdown
/bin/chmod +x "$DESTDIR"$SERVICEDIR/.svscan/shutdown

if [ ! -d $LOGDIR ] ; then
	/bin/mkdir -p $LOGDIR
	$chown qmaill:nofiles $LOGDIR
fi
# svscan log script
dump_log_header svscan > "$DESTDIR"$SERVICEDIR/.svscan/log/run
/bin/chmod +x "$DESTDIR"$SERVICEDIR/.svscan/log/run

if [ $run_file_only -eq 1 ] ; then
	if [ -d "$conf_dir".bak ] ; then
		/bin/rm -rf $conf_dir
		/bin/mv "$conf_dir".bak $conf_dir
	fi
fi

# resolvconf script using inotify
if [ $mount_resolvconf -eq 1 -a \
		-x /usr/bin/unshare -a \
		-x "$DESTDIR"$SERVICEDIR/dnscache/run -a \
		-x "$DESTDIR"$QmailBinPrefix/bin/inotify ] ; then
	conf_dir="$DESTDIR"$SERVICEDIR/resolvconf/variables
	if [ $run_file_only -eq 1 ] ; then
		if [ -d $conf_dir ] ; then
			/bin/mv $conf_dir "$conf_dir".bak
		fi
	fi
	/bin/mkdir -p "$DESTDIR"$SERVICEDIR/resolvconf/log
	if [ $down_state -eq 1 ] ; then
		$TOUCH "$DESTDIR"$SERVICEDIR/resolvconf/down
	fi
	/bin/mkdir -p $conf_dir
	/bin/chmod 500 $conf_dir
	link_with_global $conf_dir
	echo "$prog_args" > $conf_dir/.options
	# resolvconf run script
	(
	dump_run_header
	echo "trap clean_up 1 2 15"
	echo ""
	echo "function clean_up"
	echo "{"
	echo "	echo \"shutting down\""
	echo "	pkill -P \$(jobs -p) inotify"
	echo "	exit"
	echo "}"
	echo ""
	echo "exec 2>&1"
	echo "file=\$(realpath /etc/resolv.conf)"
	echo "dir=\$(dirname \$file)"
	echo "echo \"starting with resolv_conf_dir=\$dir\""
	echo "@prefix@/bin/qcat /etc/resolv.conf"
	echo "exec 2>&1"
	echo "("
	echo "/usr/bin/inotify -n \$dir | while read line"
	echo "do"
	echo "	set \$line"
	echo "	if [ \" \$1\" != \" file\" ] ; then"
	echo "		continue"
	echo "	fi"
	echo "	file=\$2"
	echo "	event=\$3"
	echo "	if [ \" \$file\" != \" resolv.conf\" ] ; then"
	echo "		continue"
	echo "	fi"
	echo "	if [ \" \$event\" != \" closed\" ] ; then"
	echo "		continue"
	echo "	fi"
	echo "	list=\`grep nameserver /etc/resolv.conf\` > /dev/null"
	echo "	if [ \$? -eq 0 ] ; then"
	echo "		echo \$list"
	echo "		echo \"$umount /etc/resolv.conf\""
	echo "		$umount /etc/resolv.conf"
	echo "		echo \"$mount --bind $sysconfdir/resolv.conf /etc/resolv.conf\""
	echo "		$mount --bind $sysconfdir/resolv.conf /etc/resolv.conf"
	echo "	fi"
	echo "done"
	echo ") &"
	echo "wait"
	) > "$DESTDIR"$SERVICEDIR/resolvconf/run
	/bin/chmod +x "$DESTDIR"$SERVICEDIR/resolvconf/run

	# resolvconf log script
	dump_log_header resolvconf > "$DESTDIR"$SERVICEDIR/resolvconf/log/run
	/bin/chmod +x "$DESTDIR"$SERVICEDIR/resolvconf/log/run
fi

if [ $run_file_only -eq 1 ] ; then
	if [ -d "$conf_dir".bak ] ; then
		/bin/rm -rf $conf_dir
		/bin/mv "$conf_dir".bak $conf_dir
	fi
fi
}

create_fetchmail()
{
if [ $# -ne 4 ] ; then
	echo "USAGE: create_fetchmail qbase queue_count first_queue_no supervise_dir" 1>&2
	return 1
fi
QUEUE_BASE=$1
QUEUE_COUNT=$2
QUEUE_START=$3
SERVICEDIR=$4

if [ $nooverwrite -eq 1 -a -d "$DESTDIR"$SERVICEDIR/fetchmail/variables ] ; then
	return 0
fi
conf_dir="$DESTDIR"$SERVICEDIR/fetchmail/variables
if [ $run_file_only -eq 1 ] ; then
	if [ -d $conf_dir ] ; then
		/bin/mv $conf_dir "$conf_dir".bak
	fi
fi
/bin/mkdir -p "$DESTDIR"$SERVICEDIR/fetchmail/log
if [ $down_state -eq 1 ] ; then
	$TOUCH "$DESTDIR"$SERVICEDIR/fetchmail/down
fi
/bin/mkdir -p $conf_dir
/bin/chmod 500 $conf_dir
link_with_global $conf_dir
if [ $no_multi -eq 1 ] ; then
	echo $qmailqueue > $conf_dir/QMAILQUEUE
	echo "$QUEUE_BASE"/queue1 > $conf_dir/QUEUEDIR
else
	> $conf_dir/QUEUEDIR
	for i in QUEUE_BASE QUEUE_COUNT QUEUE_START
	do
		/bin/rm -f $conf_dir/$i
	done
	t=$("$DESTDIR"$QmailBinPrefix/bin/envdir $conf_dir env 2>/dev/null | grep QUEUE_BASE | cut -d= -f2)
	if [ ! "$t" = "$QUEUE_BASE" ] ; then
		echo $QUEUE_BASE   > $conf_dir/QUEUE_BASE
	fi
	t=$("$DESTDIR"$QmailBinPrefix/bin/envdir $conf_dir env 2>/dev/null | grep QUEUE_COUNT | cut -d= -f2)
	if [ ! "$t" = "$QUEUE_COUNT" ] ; then
		echo $QUEUE_COUNT > $conf_dir/QUEUE_COUNT
	fi
	t=$("$DESTDIR"$QmailBinPrefix/bin/envdir $conf_dir env 2>/dev/null | grep QUEUE_START | cut -d= -f2)
	if [ ! "$t" = "$QUEUE_START" ] ; then
		echo $QUEUE_START  > $conf_dir/QUEUE_START
	fi
fi
if [ " $min_free" = " " ] ; then
	echo 52428800 > $conf_dir/MIN_FREE
else
	echo $min_free > $conf_dir/MIN_FREE
fi
if [ " $QmailBinPrefix" = " /usr" ] ; then
	echo "/bin:/usr/bin:/usr/sbin:/sbin" > $conf_dir/PATH
else
	echo "/bin:/usr/bin:$QmailBinPrefix/bin:$QmailBinPrefix/sbin" > $conf_dir/PATH
fi
if [ ! " $CONTROLDIR" = " " ] ; then
	echo $CONTROLDIR > $conf_dir/CONTROLDIR
else
	> $conf_dir/CONTROLDIR
fi
if [ $usefsync -ne 0 ] ; then
	echo 1 > $conf_dir/USE_FSYNC
fi
if [ $usefdatasync -ne 0 ] ; then
	echo 1 > $conf_dir/USE_FDATASYNC
fi
if [ $usesyncdir -ne 0 ] ; then
	echo 1 > $conf_dir/USE_SYNCDIR
fi
if [ ! " $memory" = " " ] ; then
	echo $memory > $conf_dir/SOFT_MEM
else
	echo 5242880 > $conf_dir/SOFT_MEM
fi
if [ ! " $spamfilter" = " " ] ; then
	echo "$spamfilter" > $conf_dir/SPAMFILTER
	echo "1" > $conf_dir/MAKE_SEEKABLE
	if [ ! " $spamexitcode" = " " ] ; then
		echo "$spamexitcode" > $conf_dir/SPAMEXITCODE
		if [ ! " $rejectspam" = " " ] ; then
			echo "$rejectspam" > $conf_dir/REJECTSPAM
		else
			> $conf_dir/REJECTSPAM
		fi
	else
		> $conf_dir/SPAMEXITCODE
	fi
else
	> $conf_dir/SPAMFILTER
	> $conf_dir/REJECTSPAM
	> $conf_dir/SPAMEXITCODE
fi
if [ " $qhpsi" = " " ] ; then
	> $conf_dir/QHPSI
else
	echo $qhpsi > $conf_dir/QHPSI
fi
if [ ! " $domainlimits" = " " ] ; then
	echo  > $conf_dir/DOMAIN_LIMITS
else
	> $conf_dir/DOMAIN_LIMITS
fi
case "$dkverify_option" in
	dkimstrict)
	/bin/rm -f $conf_dir/DKIMSIGN $conf_dir/DKIMVERIFY $conf_dir/DKIMQUEUE
	echo FGHIKLMNOQRTUVWjp > $conf_dir/DKIMVERIFY
	> $conf_dir/UNSIGNED_SUBJECT
	> $conf_dir/UNSIGNED_FROM
	if [ -n "$spamfilter" ] ; then
		echo "$QmailBinPrefix/sbin/qmail-spamfilter $QmailBinPrefix/sbin/qmail-dkim" \
			> $conf_dir/QMAILQUEUE
	else
		echo "$QmailBinPrefix/sbin/qmail-dkim" > $conf_dir/QMAILQUEUE
	fi
	if [ $no_multi -eq 1 ] ; then
		echo "$QUEUE_BASE"/queue1 > $conf_dir/QUEUEDIR
	fi
	;;
	dkim)
	/bin/rm -f $conf_dir/DKIMSIGN $conf_dir/DKIMVERIFY $conf_dir/DKIMQUEUE
	echo > $conf_dir/DKIMVERIFY
	echo > $conf_dir/UNSIGNED_SUBJECT
	> $conf_dir/UNSIGNED_FROM
	if [ -n "$spamfilter" ] ; then
		echo "$QmailBinPrefix/sbin/qmail-spamfilter $QmailBinPrefix/sbin/qmail-dkim" \
			> $conf_dir/QMAILQUEUE
	else
		echo "$QmailBinPrefix/sbin/qmail-dkim" > $conf_dir/QMAILQUEUE
	fi
	if [ $no_multi -eq 1 ] ; then
		echo "$QUEUE_BASE"/queue1 > $conf_dir/QUEUEDIR
	fi
	;;
	none|*)
	/bin/rm -f $conf_dir/DKIMSIGN $conf_dir/DKIMVERIFY $conf_dir/DKIMQUEUE
	if [ $no_multi -eq 1 ] ; then
		echo "$QUEUE_BASE"/queue1 > $conf_dir/QUEUEDIR
		if [ -n "$spamfilter" ] ; then
			echo "$QmailBinPrefix/sbin/qmail-spamfilter" > $conf_dir/QMAILQUEUE
		else
			echo $qmailqueue > $conf_dir/QMAILQUEUE
		fi
	else
		> $conf_dir/QUEUEDIR
		if [ -n "$spamfilter" ] ; then
			echo "$QmailBinPrefix/sbin/qmail-spamfilter" > $conf_dir/QMAILQUEUE
		else
			echo "$QmailBinPrefix/sbin/qmail-multi" > $conf_dir/QMAILQUEUE
		fi
	fi
	;;
esac
(
dump_run_header
echo "exec 2>&1"
echo "exec $QmailBinPrefix/bin/envdir $envdir_opts variables sh -c \""
echo "exec $QmailBinPrefix/bin/setuidgid indimail \\"
echo "$QmailBinPrefix/bin/softlimit -m \\\$SOFT_MEM -o 1024 \\"
if [ $silent -eq 1 ] ; then
	echo "$QmailBinPrefix/bin/fetchmail --silent --nodetach -f $sysconfdir/fetchmailrc\""
else
	echo "$QmailBinPrefix/bin/fetchmail --nodetach -f $sysconfdir/fetchmailrc\""
fi
) > "$DESTDIR"$SERVICEDIR/fetchmail/run
/bin/chmod +x "$DESTDIR"$SERVICEDIR/fetchmail/run

if [ ! -d $LOGDIR ] ; then
	/bin/mkdir -p $LOGDIR
	$chown qmaill:nofiles $LOGDIR
fi
# fetchmail log script
dump_log_header fetchmail > "$DESTDIR"$SERVICEDIR/fetchmail/log/run
/bin/chmod +x "$DESTDIR"$SERVICEDIR/fetchmail/log/run
echo "$prog_args" > $conf_dir/.options

if [ $run_file_only -eq 1 ] ; then
	if [ -d "$conf_dir".bak ] ; then
		/bin/rm -rf $conf_dir
		/bin/mv "$conf_dir".bak $conf_dir
	fi
fi

if [ ! " $logfilter" = " " ] ; then
	echo  $logfilter > $conf_dir/LOGFILTER
	if [ ! -f $SERVICEDIR/qmail-logfifo/run ] ; then
		prog_args="$QmailBinPrefix/sbin/svctool --fifologger=\"$logfilter\" --servicedir=\"$SERVICEDIR\""
		create_fifologger $logfilter $SERVICEDIR
	fi
else
	> $conf_dir/LOGFILTER
fi
}

create_poppass()
{
if [ $# -ne 4 ] ; then
	echo "USAGE: create_poppass supervise_dir poppass_port password_command use_ssl=0|1" 1>&2
	return 1
fi
SERVICEDIR=$1
pwd_cmd=$3
poppass_ssl=$4
if [ " $poppass_ssl" = " 1" -a " $certfile" = " " ] ; then
	echo "Certificate not specfied" 1>&2
	return 1
fi

if [ " $servicetag" = " " ] ; then
	tag=$2
else
	tag=$servicetag
fi

if [ $nooverwrite -eq 1 -a -d "$DESTDIR"$SERVICEDIR/qmail-poppass.$tag/variables ] ; then
	return 0
fi
conf_dir="$DESTDIR"$SERVICEDIR/qmail-poppass.$tag/variables
if [ $run_file_only -eq 1 ] ; then
	if [ -d $conf_dir ] ; then
		/bin/mv $conf_dir "$conf_dir".bak
	fi
fi
/bin/mkdir -p "$DESTDIR"$SERVICEDIR/qmail-poppass.$tag/log
if [ $down_state -eq 1 ] ; then
	$TOUCH "$DESTDIR"$SERVICEDIR/qmail-poppass.$tag/down
fi
/bin/mkdir -p $conf_dir
$chown root:indimail $conf_dir
/bin/chmod 550 $conf_dir
link_with_global $conf_dir

if [ " $ipaddress" = " " ] ; then
	echo 0 > $conf_dir/LOCALIP
else
	echo $ipaddress > $conf_dir/LOCALIP
fi
if [ ! " $memory" = " " ] ; then
	echo $memory > $conf_dir/SOFT_MEM
else
	echo 52428800 > $conf_dir/SOFT_MEM
fi
echo $2 > $conf_dir/PORT
if [ ! " $maxdaemons" = " " ] ; then
	echo "$maxdaemons" > $conf_dir/MAXDAEMONS
else
	echo $CONCURRENCYINCOMING > $conf_dir/MAXDAEMONS
fi
if [ ! " $maxperip" = " " ] ; then
	echo "$maxperip" > $conf_dir/MAXPERIP
else
	echo 10 > $conf_dir/MAXPERIP
fi
if [ -f /bin/false ] ; then
	false="/bin/false"
else
	false="/usr/bin/false"
fi
if [ ! " $pwd_cmd" = " " ] ; then
	echo "$pwd_cmd $false" > $conf_dir/PASSWORD_COMMAND
else
	echo "$QmailBinPrefix""/sbin/vsetpass $false" > $conf_dir/PASSWORD_COMMAND
fi
if [ " $poppass_ssl" = " 1" ] ; then
	echo $certfile > $conf_dir/CERTFILE
else
	> $conf_dir/CERTFILE
fi
(
dump_run_header
echo "exec 2>&1"
echo "MYUID=\`/usr/bin/id -u qmaild\`"
echo "MYGID=\`/usr/bin/id -g indimail\`"
echo "HOSTNAME=\`uname -n\`"
echo ""
echo "if [ -z \"\$MYUID\" -o -z \"\$MYGID\" ]; then"
echo "    echo UID or GID is unset in"
echo "    echo \`pwd\`/run"
echo "    sleep 5"
echo "    exit 1"
echo "fi"
echo "if [ ! -f \$CERTFILE ]; then"
echo "    echo Certificate not present"
echo "    sleep 5"
echo "    exit 1"
echo "fi"
echo ""
echo "exec $QmailBinPrefix/bin/envdir $envdir_opts variables sh -c \""
echo "exec $QmailBinPrefix/bin/softlimit -m \\\$SOFT_MEM -o 1024 \\"
echo "$QmailBinPrefix/bin/tcpserver -v -H -R -l \$HOSTNAME \\"
echo "-x $sysconfdir/tcp/tcp.poppass.cdb -X \\"
echo "-c ./variables/MAXDAEMONS -C \\\$MAXPERIP -o -b \\\$MAXDAEMONS \\"
if [ " $poppass_ssl" = " 1" ] ; then
	echo "-n \\\$CERTFILE \\"
fi
if [ -f /bin/false ] ; then
	false="/bin/false"
else
	false="/usr/bin/false"
fi
if [ " $poppass_ssl" = " 1" ] ; then
echo "-u \$MYUID -g \$MYGID,qcerts \\\$LOCALIP \\\$PORT \\"
else
echo "-u \$MYUID -g \$MYGID \\\$LOCALIP \\\$PORT \\"
fi
echo "$QmailBinPrefix/sbin/qmail-poppass \$HOSTNAME $QmailBinPrefix/sbin/vchkpass $false\""
) > $TMPDIR/poppass-run.$$
/bin/mv $TMPDIR/poppass-run.$$ "$DESTDIR"$SERVICEDIR/qmail-poppass.$tag/run
/bin/chmod +x "$DESTDIR"$SERVICEDIR/qmail-poppass.$tag/run

if [ ! -d $LOGDIR ] ; then
	/bin/mkdir -p $LOGDIR
	$chown qmaill:nofiles $LOGDIR
fi
# qmail-poppass log script
dump_log_header poppass.$tag > $TMPDIR/popass-run.$$
/bin/mv $TMPDIR/popass-run.$$ "$DESTDIR"$SERVICEDIR/qmail-poppass.$tag/log/run
/bin/chmod +x "$DESTDIR"$SERVICEDIR/qmail-poppass.$tag/log/run
echo "$prog_args" > $conf_dir/.options

if [ $run_file_only -eq 1 ] ; then
	if [ -d "$conf_dir".bak ] ; then
		/bin/rm -rf $conf_dir
		/bin/mv "$conf_dir".bak $conf_dir
	fi
fi
}

create_delivery()
{
if [ $# -ne 8 ] ; then
	echo "USAGE: create_delivery qbase queue_count first_queue_no supervise_dir ident routes=smtp|qmtp|static use_ssl utf8" 1>&2
	return 1
fi
QUEUE_BASE=$1
QUEUE_COUNT=$2
QUEUE_START=$3
SERVICEDIR=$4
QUEUE_IDENT=$5
routes=$6
qmr_ssl=$7
utf8=$8
if [ " $servicetag" = " " ] ; then
	tag=$QUEUE_IDENT
else
	tag=$servicetag
fi

# Create QUEUES
#---------------------------------------------------------------------------------------
if [ ! -d "$DESTDIR"$QUEUE_BASE ] ; then
	/bin/mkdir -p "$DESTDIR"$QUEUE_BASE
	/bin/chmod 755 "$DESTDIR"$QUEUE_BASE
	$chown root:qmail "$DESTDIR"$QUEUE_BASE
fi

if [ $nooverwrite -eq 0 -o ! -d "$DESTDIR"$QUEUE_BASE/nqueue ] ; then
	echo "Creating Queue "$DESTDIR"$QUEUE_BASE/nqueue"
	if [ -f "$DESTDIR"$QmailBinPrefix/bin/queue-fix ] ; then
		"$DESTDIR"$QmailBinPrefix/bin/queue-fix -s QUEUESPLIT -b 0 \
			"$DESTDIR"$QUEUE_BASE/nqueue > /dev/null
	else
		./queue-fix -s QUEUESPLIT -b 0 \
			"$DESTDIR"$QUEUE_BASE/nqueue > /dev/null
	fi
	QUEUE_NO=$QUEUE_START
	COUNT=1
	while true
	do
		if [ ! -d "$DESTDIR"$QUEUE_BASE/queue"$QUEUE_NO" ] ; then
			/bin/mkdir -p "$DESTDIR"$QUEUE_BASE/queue"$QUEUE_NO"
		fi
		echo "Creating Queue "$DESTDIR"$QUEUE_BASE/queue"$QUEUE_NO""
		if [ -f "$DESTDIR"$QmailBinPrefix/bin/queue-fix ] ; then
			"$DESTDIR"$QmailBinPrefix/bin/queue-fix -s QUEUESPLIT -b 0 \
				"$DESTDIR"$QUEUE_BASE/queue"$QUEUE_NO" > /dev/null
		else
			./queue-fix -s QUEUESPLIT -b 0 \
				"$DESTDIR"$QUEUE_BASE/queue"$QUEUE_NO" > /dev/null
		fi
		if [ $COUNT -eq $QUEUE_COUNT ] ; then
			break
		fi
		COUNT=`expr $COUNT + 1`
		QUEUE_NO=`expr $QUEUE_NO + 1`
	done
	echo $QUEUE_BASE > "$DESTDIR"$CONTROLDIR/queue_base
fi

# qmail-send script
conf_dir="$DESTDIR"$SERVICEDIR/qmail-send.$tag/variables
if [ $nooverwrite -eq 1 -a -d $conf_dir ] ; then
	return 0
fi
if [ $run_file_only -eq 1 ] ; then
	if [ -d $conf_dir ] ; then
		/bin/mv $conf_dir "$conf_dir".bak
	fi
fi
/bin/mkdir -p "$DESTDIR"$SERVICEDIR/qmail-send.$tag/log
if [ $down_state -eq 1 ] ; then
	$TOUCH "$DESTDIR"$SERVICEDIR/qmail-send.$tag/down
fi
/bin/mkdir -p $conf_dir
/bin/chmod 500 $conf_dir
link_with_global $conf_dir
if [ $no_multi -eq 1 ] ; then
	echo "$QUEUE_BASE"/queue1 > $conf_dir/QUEUEDIR
	> $conf_dir/QMAILQUEUE
else
	echo "$QmailBinPrefix/sbin/qmail-multi" > $conf_dir/QMAILQUEUE
	> $conf_dir/QUEUEDIR
	for i in QUEUE_BASE QUEUE_COUNT QUEUE_START
	do
		/bin/rm -f $conf_dir/$i
	done
	t=$("$DESTDIR"$QmailBinPrefix/bin/envdir $conf_dir env 2>/dev/null | grep QUEUE_BASE | cut -d= -f2)
	if [ ! "$t" = "$QUEUE_BASE" ] ; then
		echo $QUEUE_BASE   > $conf_dir/QUEUE_BASE
	fi
	t=$("$DESTDIR"$QmailBinPrefix/bin/envdir $conf_dir env 2>/dev/null | grep QUEUE_COUNT | cut -d= -f2)
	if [ ! "$t" = "$QUEUE_COUNT" ] ; then
		echo $QUEUE_COUNT > $conf_dir/QUEUE_COUNT
	fi
	t=$("$DESTDIR"$QmailBinPrefix/bin/envdir $conf_dir env 2>/dev/null | grep QUEUE_START | cut -d= -f2)
	if [ ! "$t" = "$QUEUE_START" ] ; then
		echo $QUEUE_START  > $conf_dir/QUEUE_START
	fi
fi
if [ " $min_free" = " " ] ; then
	echo 52428800 > $conf_dir/MIN_FREE
else
	echo $min_free > $conf_dir/MIN_FREE
fi
if [ -z "$msgqsize" ] ; then
	echo 8388608 > $conf_dir/MSGQSIZE
else
	echo $msgqsize > $conf_dir/MSGQSIZE
fi
if [ " $QmailBinPrefix" = " /usr" ] ; then
	echo "/bin:/usr/bin:/usr/sbin:/sbin" > $conf_dir/PATH
else
	echo "/bin:/usr/bin:$QmailBinPrefix/bin:$QmailBinPrefix/sbin" > $conf_dir/PATH
fi
if [ $qmr_ssl -eq 1 ] ; then
echo $sysconfdir/certs > $conf_dir/CERTDIR #for qmail-remote
fi
if [ $setgroups -eq 1 ] ; then
	echo 1 > $conf_dir/USE_SETGROUPS
else
	> $conf_dir/USE_SETGROUPS
fi
if [ $utf8 -eq 1 ] ; then
	echo $utf8 > $conf_dir/UTF8
else
	> $conf_dir/UTF8
fi
if [ ! " $routes" = " " ] ; then
	echo $routes > $conf_dir/ROUTES
else
	> $conf_dir/ROUTES
fi
if [ " $enable_cname_lookup" = " " ] ; then
	echo 1 > $conf_dir/DISABLE_CNAME_LOOKUP
else
	> $conf_dir/DISABLE_CNAME_LOOKUP
fi
if [ $setuser_privilege -eq 1 ] ; then
	echo 1 > $conf_dir/SETUSER_PRIVILEGES
else
	> $conf_dir/SETUSER_PRIVILEGES
fi
if [ -n "$sanitized_env" ] ; then
	echo $sanitized_env > $conf_dir/SANITIZE_ENV
else
	> $conf_dir/SANITIZE_ENV
fi
if [ ! " $CONTROLDIR" = " " ] ; then
	echo "$CONTROLDIR" > $conf_dir/CONTROLDIR
else
	> $conf_dir/CONTROLDIR
fi
if [ ! " $mailcount_limit" = " " ] ; then
	echo "$mailcount_limit" > $conf_dir/MAILCOUNT_LIMIT
else
	> $conf_dir/MAILCOUNT_LIMIT
fi
if [ ! " $mailsize_limit" = " " ] ; then
	echo "$mailsize_limit" > $conf_dir/MAILSIZE_LIMIT
else
	> $conf_dir/MAILSIZE_LIMIT
fi
if [ ! " $domainlimits" = " " ] ; then
	echo  > $conf_dir/DOMAIN_LIMITS
else
	> $conf_dir/DOMAIN_LIMITS
fi
> $conf_dir/FAST_QUOTA
echo "8192"  > $conf_dir/MAILDIRSIZE_MAX_SIZE
echo "43200" > $conf_dir/MAILDIRSIZE_MAX_AGE
if [ ! " $overquota_mailsize" = " " ] ; then
	echo $overquota_mailsize > $conf_dir/OVERQUOTA_MAILSIZE
else
	> $conf_dir/OVERQUOTA_MAILSIZE
fi
if [ $usefsync -ne 0 ] ; then
	echo 1 > $conf_dir/USE_FSYNC
fi
if [ $usefdatasync -ne 0 ] ; then
	echo 1 > $conf_dir/USE_FDATASYNC
fi
if [ $usesyncdir -ne 0 ] ; then
	echo 1 > $conf_dir/USE_SYNCDIR
fi
if [ ! " $dmemory" = " " ] ; then
	echo $dmemory > $conf_dir/SOFT_MEM
else
	echo 83886080 > $conf_dir/SOFT_MEM
fi
if [ " $persistdb" = " 1" ] ; then
	echo postmaster > $conf_dir/ROUTE_NULL_USER
	echo > $conf_dir/AUTHSELF
	mysql_opt_reconnect
	[ $? -eq 0 ] && echo 1 > $conf_dir/MYSQL_OPT_RECONNECT || > $conf_dir/MYSQL_OPT_RECONNECT
else
	> $conf_dir/ROUTE_NULL_USER
	> $conf_dir/AUTHSELF
	> $conf_dir/MYSQL_OPT_RECONNECT
fi
if [ ! " $remoteauthsmtp" = " " ] ; then
	echo $remoteauthsmtp > $conf_dir/AUTH_SMTP
else
	> $conf_dir/AUTH_SMTP
fi
if [ ! " $localfilter" = " " ] ; then
	echo "$QmailBinPrefix/sbin/spawn-filter" > $conf_dir/QMAILLOCAL
	echo "1" > $conf_dir/MAKE_SEEKABLE
else
	> $conf_dir/QMAILLOCAL
fi
if [ ! " $remotefilter" = " " ] ; then
	echo "$QmailBinPrefix/sbin/spawn-filter" > $conf_dir/QMAILREMOTE
	echo ""  > $conf_dir/RATELIMIT_DIR
	echo "1" > $conf_dir/MAKE_SEEKABLE
else
	> $conf_dir/QMAILREMOTE
fi
echo "startup.so" > $conf_dir/START_PLUGIN
#
# DEFAULT_DOMAIN will be used by vdelivermail for bounces
#
if [ -n "$default_domain" ] ; then
	if [ -f $sysconfdir/control/global_vars/DEFAULT_DOMAIN ] ; then
		t=$(@prefix@/bin/qcat $sysconfdir/control/global_vars/DEFAULT_DOMAIN)
		if [ ! "$t" = "$default_domain" ] ; then
			echo $default_domain > $conf_dir/DEFAULT_DOMAIN
		fi
	else
		echo $default_domain > $conf_dir/DEFAULT_DOMAIN
	fi
fi
echo > $conf_dir/LOGLOCK
case "$dksign_option" in
	dkim)
	/bin/rm -f $conf_dir/DKIMSIGN $conf_dir/DKIMVERIFY $conf_dir/DKIMQUEUE
	echo "$private_key" > $conf_dir/DKIMSIGN
	> $conf_dir/UNSIGNED_SUBJECT
	> $conf_dir/UNSIGNED_FROM
	if [ ! -f "$DESTDIR"$CONTROLDIR/filterargs ] ; then
		echo "*:remote:$QmailBinPrefix/sbin/qmail-dkim:DKIMQUEUE=@prefix@/bin/qcat" > "$DESTDIR"$CONTROLDIR/filterargs
	fi
	;;
	none|*)
	/bin/rm -f $conf_dir/DKIMSIGN $conf_dir/DKIMVERIFY $conf_dir/DKIMQUEUE
	;;
esac
if [ $hidehost -eq 1 ] ; then
	echo 1 > $conf_dir/HIDE_HOST
else
	> $conf_dir/HIDE_HOST
fi
(
dump_run_header
echo "exec 2>&1"
echo "if [ -s variables/CONTROLDIR ] ; then"
echo "  cntrl_dir=\$(@prefix@/bin/qcat variables/CONTROLDIR)"
echo "else"
echo "  cntrl_dir=/etc/indimail/control"
echo "fi"
echo "if [ -f \$cntrl_dir/defaultdelivery ] ; then"
echo "  defaultdelivery=\$(@prefix@/bin/qcat \$cntrl_dir/defaultdelivery)"
echo "else"
echo "  defaultdelivery=\"./Maildir/\""
echo "fi"
echo "exec $QmailBinPrefix/bin/envdir $envdir_opts variables sh -c \""
if [ "$qtype" = "static" ] ; then
	echo "exec $QmailBinPrefix/bin/softlimit -m \\\$SOFT_MEM -o 1024 \\"
else
	SYSTEM=$(uname -s)
	case "$SYSTEM" in
		FreeBSD)
		echo "if [ ! -d $QmailHOME/queue/mqueue ] ; then"
		echo "  mkdir -p $QmailHOME/queue/mqueue"
		echo "fi"
		echo "mq=\$(df $QmailHOME/queue/mqueue|grep mqueue >/dev/null)"
		echo "if [ \$? -ne 0 ] ; then"
		echo "  mount -t mqueuefs null $QmailHOME/queue/mqueue"
		echo "fi"
		;;
		Linux)
		echo "# fix for linux. Haven't understood this, but this works"
		echo "ulimit -q unlimited"
		;;
	esac
	echo "exec $QmailBinPrefix/bin/softlimit -q \\\$MSGQSIZE -m \\\$SOFT_MEM -o 1024 \\"
fi
case $qtype in
	static|*)
	echo "$QmailBinPrefix/sbin/qscheduler -s \$defaultdelivery\""
	;;
	dynamic)
	echo "$QmailBinPrefix/sbin/qscheduler -d \$defaultdelivery\""
	;;
	compat)
	echo "$QmailBinPrefix/sbin/qscheduler -cd \$defaultdelivery\""
	;;
esac
) > "$DESTDIR"$SERVICEDIR/qmail-send."$tag"/run
/bin/chmod +x "$DESTDIR"$SERVICEDIR/qmail-send."$tag"/run

if [ ! -d $LOGDIR ] ; then
	/bin/mkdir -p $LOGDIR
	$chown qmaill:nofiles $LOGDIR
fi
# qmail-send log script
/bin/mkdir -p "$DESTDIR"$SERVICEDIR/qmail-send."$tag"/log
if [ $down_state -eq 1 ] ; then
	$TOUCH "$DESTDIR"$SERVICEDIR/qmail-send.$tag/down
fi
dump_log_header deliver.$tag > "$DESTDIR"$SERVICEDIR/qmail-send."$tag"/log/run
/bin/chmod +x "$DESTDIR"$SERVICEDIR/qmail-send."$tag"/log/run
if [ -z "$skipsend" ] ; then
	echo "$prog_args" > $conf_dir/.options
fi

if [ $run_file_only -eq 1 ] ; then
	if [ -d "$conf_dir".bak ] ; then
		/bin/rm -rf $conf_dir
		/bin/mv "$conf_dir".bak $conf_dir
	fi
fi
}

create_slowq()
{
if [ $# -ne 5 ] ; then
	echo "USAGE: create_slowq qbase supervise_dir routes=smtp|qmtp|static use_ssl utf8" 1>&2
	return 1
fi
QUEUE_BASE=$1
SERVICEDIR=$2
routes=$3
qmr_ssl=$4
utf8=$5

# Create QUEUES
#---------------------------------------------------------------------------------------
if [ ! -d "$DESTDIR"$QUEUE_BASE ] ; then
	/bin/mkdir -p "$DESTDIR"$QUEUE_BASE
	/bin/chmod 755 "$DESTDIR"$QUEUE_BASE
	$chown root:qmail "$DESTDIR"$QUEUE_BASE
fi

if [ $nooverwrite -eq 0 -o ! -d "$DESTDIR"$QUEUE_BASE/slowq ] ; then
	echo "Creating Queue "$DESTDIR"$QUEUE_BASE/slowq"
	if [ -f "$DESTDIR"$QmailBinPrefix/bin/queue-fix ] ; then
		"$DESTDIR"$QmailBinPrefix/bin/queue-fix -s QUEUESPLIT -b 0 \
			-r "$DESTDIR"$QUEUE_BASE/slowq > /dev/null
	else
		./queue-fix -s QUEUESPLIT -b 0 \
			-r "$DESTDIR"$QUEUE_BASE/slowq > /dev/null
	fi
fi

# slowq-send script
conf_dir="$DESTDIR"$SERVICEDIR/slowq-send/variables
if [ $nooverwrite -eq 1 -a -d $conf_dir ] ; then
	return 0
fi
if [ $run_file_only -eq 1 ] ; then
	if [ -d $conf_dir ] ; then
		/bin/mv $conf_dir "$conf_dir".bak
	fi
fi
/bin/mkdir -p "$DESTDIR"$SERVICEDIR/slowq-send/log
if [ $down_state -eq 1 ] ; then
	$TOUCH "$DESTDIR"$SERVICEDIR/slowq-send/down
fi
/bin/mkdir -p $conf_dir
/bin/chmod 500 $conf_dir
link_with_global $conf_dir
echo "$QUEUE_BASE/slowq" > $conf_dir/QUEUEDIR
if [ " $min_free" = " " ] ; then
	echo 52428800 > $conf_dir/MIN_FREE
else
	echo $min_free > $conf_dir/MIN_FREE
fi
if [ " $QmailBinPrefix" = " /usr" ] ; then
	echo "/bin:/usr/bin:/usr/sbin:/sbin" > $conf_dir/PATH
else
	echo "/bin:/usr/bin:$QmailBinPrefix/bin:$QmailBinPrefix/sbin" > $conf_dir/PATH
fi
if [ $qmr_ssl -eq 1 ] ; then
echo $sysconfdir/certs > $conf_dir/CERTDIR #for qmail-remote
fi
if [ $setgroups -eq 1 ] ; then
	echo 1 > $conf_dir/USE_SETGROUPS
else
	> $conf_dir/USE_SETGROUPS
fi
if [ $utf8 -eq 1 ] ; then
	echo $utf8 > $conf_dir/UTF8
else
	> $conf_dir/UTF8
fi
if [ ! " $routes" = " " ] ; then
	echo $routes > $conf_dir/ROUTES
else
	> $conf_dir/ROUTES
fi
if [ " $enable_cname_lookup" = " " ] ; then
	echo 1 > $conf_dir/DISABLE_CNAME_LOOKUP
else
	> $conf_dir/DISABLE_CNAME_LOOKUP
fi
if [ $setuser_privilege -eq 1 ] ; then
	echo 1 > $conf_dir/SETUSER_PRIVILEGES
else
	> $conf_dir/SETUSER_PRIVILEGES
fi
if [ -n "$sanitized_env" ] ; then
	echo $sanitized_env > $conf_dir/SANITIZE_ENV
else
	> $conf_dir/SANITIZE_ENV
fi
if [ ! " $CONTROLDIR" = " " ] ; then
	echo "$CONTROLDIR" > $conf_dir/CONTROLDIR
else
	> $conf_dir/CONTROLDIR
fi
if [ ! " $mailcount_limit" = " " ] ; then
	echo "$mailcount_limit" > $conf_dir/MAILCOUNT_LIMIT
else
	> $conf_dir/MAILCOUNT_LIMIT
fi
if [ ! " $mailsize_limit" = " " ] ; then
	echo "$mailsize_limit" > $conf_dir/MAILSIZE_LIMIT
else
	> $conf_dir/MAILSIZE_LIMIT
fi
if [ ! " $domainlimits" = " " ] ; then
	echo  > $conf_dir/DOMAIN_LIMITS
else
	> $conf_dir/DOMAIN_LIMITS
fi
> $conf_dir/FAST_QUOTA
echo "8192"  > $conf_dir/MAILDIRSIZE_MAX_SIZE
echo "43200" > $conf_dir/MAILDIRSIZE_MAX_AGE
if [ ! " $overquota_mailsize" = " " ] ; then
	echo $overquota_mailsize > $conf_dir/OVERQUOTA_MAILSIZE
else
	> $conf_dir/OVERQUOTA_MAILSIZE
fi
if [ $usefsync -ne 0 ] ; then
	echo 1 > $conf_dir/USE_FSYNC
fi
if [ $usefdatasync -ne 0 ] ; then
	echo 1 > $conf_dir/USE_FDATASYNC
fi
if [ $usesyncdir -ne 0 ] ; then
	echo 1 > $conf_dir/USE_SYNCDIR
fi
if [ ! " $dmemory" = " " ] ; then
	echo $dmemory > $conf_dir/SOFT_MEM
else
	echo 83886080 > $conf_dir/SOFT_MEM
fi
if [ " $persistdb" = " 1" ] ; then
	echo postmaster > $conf_dir/ROUTE_NULL_USER
	echo > $conf_dir/AUTHSELF
	[ $? -eq 0 ] && echo 1 > $conf_dir/MYSQL_OPT_RECONNECT || > $conf_dir/MYSQL_OPT_RECONNECT
else
	> $conf_dir/ROUTE_NULL_USER
	> $conf_dir/AUTHSELF
	> $conf_dir/MYSQL_OPT_RECONNECT
fi
if [ ! " $remoteauthsmtp" = " " ] ; then
	echo $remoteauthsmtp > $conf_dir/AUTH_SMTP
else
	> $conf_dir/AUTH_SMTP
fi
if [ ! " $localfilter" = " " ] ; then
	echo "$QmailBinPrefix/sbin/spawn-filter" > $conf_dir/QMAILLOCAL
	echo "1" > $conf_dir/MAKE_SEEKABLE
else
	> $conf_dir/QMAILLOCAL
fi
if [ ! " $remotefilter" = " " ] ; then
	mkdir -p "$DESTDIR"$QUEUE_BASE/slowq/ratelimit
	$chown qmails:qmail "$DESTDIR"$QUEUE_BASE/slowq/ratelimit
	echo "$QmailBinPrefix/sbin/spawn-filter" > $conf_dir/QMAILREMOTE
	echo "$QUEUE_BASE/slowq/ratelimit" > $conf_dir/RATELIMIT_DIR
	echo "1" > $conf_dir/MAKE_SEEKABLE
else
	> $conf_dir/QMAILREMOTE
fi
if [ $todo_proc -eq 1 ] ; then
	echo "1" > $conf_dir/TODO_PROCESSOR
else
	> $conf_dir/TODO_PROCESSOR
fi
#
# DEFAULT_DOMAIN will be used by vdelivermail for bounces
#
if [ -n "$default_domain" ] ; then
	if [ -f $sysconfdir/control/global_vars/DEFAULT_DOMAIN ] ; then
		t=$(@prefix@/bin/qcat $sysconfdir/control/global_vars/DEFAULT_DOMAIN)
		if [ ! "$t" = "$default_domain" ] ; then
			echo $default_domain > $conf_dir/DEFAULT_DOMAIN
		fi
	else
		echo $default_domain > $conf_dir/DEFAULT_DOMAIN
	fi
fi
case "$dksign_option" in
	dkim)
	/bin/rm -f $conf_dir/DKIMSIGN $conf_dir/DKIMVERIFY $conf_dir/DKIMQUEUE
	echo "$private_key" > $conf_dir/DKIMSIGN
	> $conf_dir/UNSIGNED_SUBJECT
	> $conf_dir/UNSIGNED_FROM
	if [ ! -f "$DESTDIR"$CONTROLDIR/filterargs ] ; then
		echo "*:remote:$QmailBinPrefix/sbin/qmail-dkim:DKIMQUEUE=@prefix@/bin/qcat" > "$DESTDIR"$CONTROLDIR/filterargs
	fi
	;;
	none|*)
	/bin/rm -f $conf_dir/DKIMSIGN $conf_dir/DKIMVERIFY $conf_dir/DKIMQUEUE
	;;
esac
(
dump_run_header
echo "exec 2>&1"
echo "if [ -s variables/CONTROLDIR ] ; then"
echo "  cntrl_dir=\$(@prefix@/bin/qcat variables/CONTROLDIR)"
echo "else"
echo "  cntrl_dir=/etc/indimail/control"
echo "fi"
echo "if [ -f \$cntrl_dir/defaultdelivery ] ; then"
echo "  defaultdelivery=\$(@prefix@/bin/qcat \$cntrl_dir/defaultdelivery)"
echo "else"
echo "  defaultdelivery=\"./Maildir/\""
echo "fi"
echo "exec $QmailBinPrefix/bin/envdir $envdir_opts variables sh -c \""
echo "exec $QmailBinPrefix/bin/softlimit -m \\\$SOFT_MEM -o 1024 \\"
echo "$QmailBinPrefix/sbin/slowq-start \$defaultdelivery\""
) > "$DESTDIR"$SERVICEDIR/slowq-send/run
/bin/chmod +x "$DESTDIR"$SERVICEDIR/slowq-send/run

if [ ! -d $LOGDIR ] ; then
	/bin/mkdir -p $LOGDIR
	$chown qmaill:nofiles $LOGDIR
fi
# qmail-send log script
/bin/mkdir -p "$DESTDIR"$SERVICEDIR/slowq-send/log
if [ $down_state -eq 1 ] ; then
	$TOUCH "$DESTDIR"$SERVICEDIR/slowq-send/down
fi
dump_log_header slowq > "$DESTDIR"$SERVICEDIR/slowq-send/log/run
/bin/chmod +x "$DESTDIR"$SERVICEDIR/slowq-send/log/run
if [ -z "$skipsend" ] ; then
	echo "$prog_args" > $conf_dir/.options
fi

if [ $run_file_only -eq 1 ] ; then
	if [ -d "$conf_dir".bak ] ; then
		/bin/rm -rf $conf_dir
		/bin/mv "$conf_dir".bak $conf_dir
	fi
fi
}

mrtg_extra()
{
	systemctl is-enabled qmta-send >/dev/null
	if [ $? -eq 0 ] ; then
		echo "#-------------------------------------------------------------------"
		echo "Title[qmta-msg1]: qmta-send Messages (Success, Attempts) - $1"
		echo "MaxBytes[qmta-msg1]: 10000"
		echo "AbsMax[qmta-msg1]: 20000"
		echo "Options[qmta-msg1]: growright,unknaszero,nopercent,gauge,pngdate"
		echo "Target[qmta-msg1]: \`$libexecdir/journal2mrtg qmta-send -m\`"
		echo "PageTop[qmta-msg1]: <B>Stats for Messages</B><br>"
		echo "ShortLegend[qmta-msg1]: msg"
		echo "YLegend[qmta-msg1]: msg/hour"
		echo "Legend1[qmta-msg1]: Total Msg&nbsp;"
		echo "Legend2[qmta-msg1]: Total Attempts&nbsp;"
		echo "LegendI[qmta-msg1]: Deliveries:&nbsp;"
		echo "LegendO[qmta-msg1]: Attempts:&nbsp;"
		echo "WithPeak[qmta-msg1]: ymwd"
		echo "XSize[qmta-msg1]: 350"
		echo "YSize[qmta-msg1]: 150"
		echo ""
		echo "#-------------------------------------------------------------------"
		echo "Title[qmta-msg2]: qmta-send Message Status (Success, Failure) - $1"
		echo "MaxBytes[qmta-msg2]: 10000"
		echo "AbsMax[qmta-msg2]: 100000"
		echo "Options[qmta-msg2]: growright,unknaszero,nopercent,gauge,pngdate"
		echo "Target[qmta-msg2]: \`$libexecdir/journal2mrtg qmta-send -s\`"
		echo "PageTop[qmta-msg2]: <B>Message Status</B><BR>"
		echo "ShortLegend[qmta-msg2]: msg"
		echo "YLegend[qmta-msg2]: msg/hour"
		echo "Legend1[qmta-msg2]: msg&nbsp;"
		echo "LegendI[qmta-msg2]: success&nbsp;"
		echo "LegendO[qmta-msg2]: failures&nbsp;"
		echo "WithPeak[qmta-msg2]: ymwd"
		echo "XSize[qmta-msg2]: 350"
		echo "YSize[qmta-msg2]: 150"
		echo ""
		echo "#-------------------------------------------------------------------"
		echo "Title[qmta-conc]: qmta-send Local/Remote Concurrency - $1"
		echo "MaxBytes[qmta-conc]: 500"
		echo "AbsMax[qmta-conc]: 10000"
		echo "Options[qmta-conc]: growright,unknaszero,nopercent,gauge,pngdate"
		echo "Target[qmta-conc]: \`$libexecdir/journal2mrtg qmta-send -c\`"
		echo "PageTop[qmta-conc]: <B>Local/Remote Concurrency</B><br>"
		echo "ShortLegend[qmta-conc]: concurrency"
		echo "YLegend[qmta-conc]: max concurrency"
		echo "Legend1[qmta-conc]: Local Concurrency&nbsp;"
		echo "Legend2[qmta-conc]: Remote Concurrency&nbsp;"
		echo "LegendI[qmta-conc]: local&nbsp;"
		echo "LegendO[qmta-conc]: remote&nbsp;"
		echo "WithPeak[qmta-conc]: ymwd"
		echo "XSize[qmta-conc]: 350"
		echo "YSize[qmta-conc]: 150"
		echo ""
		echo "#-------------------------------------------------------------------"
		echo "Title[qmta-ibits]: qmta-send Bits Transfered - $1"
		echo "MaxBytes[qmta-ibits]: 1540000"
		echo "AbsMax[qmta-ibits]: 100000000"
		echo "Options[qmta-ibits]: growright,unknaszero,nopercent,gauge,pngdate"
		echo "Target[qmta-ibits]: \`$libexecdir/journal2mrtg qmta-send -b\`"
		echo "PageTop[qmta-ibits]: <B>Bits Transfered</B><br>"
		echo "ShortLegend[qmta-ibits]: bits"
		echo "YLegend[qmta-ibits]: bits/sec"
		echo "Legend1[qmta-ibits]: bits&nbsp;"
		echo "Legend2[qmta-ibits]: bits&nbsp;"
		echo "LegendI[qmta-ibits]: bits&nbsp;"
		echo "LegendO[qmta-ibits]: bits&nbsp;"
		echo "WithPeak[qmta-ibits]: ymwd"
		echo "XSize[qmta-ibits]: 350"
		echo "YSize[qmta-ibits]: 150"
		echo ""
	fi

	for i in POP3.110.pop3d.110 POP3-SSL.995.pop3d-ssl.995 \
		IMAP.143.imapd.143 IMAP-SSL.993.imapd-ssl.993 \
		PROXYIMAP.4143.proxyIMAP.4143 PROXYPOP3.4110.proxyPOP3.4110 \
		PROXYIMAP-SSL.9143.proxyIMAP-ssl.9143 PROXYPOP3-SSL.9110.proxyPOP3-ssl.9110
	do
		t1=$(echo $i | cut -d. -f1)
		t2=$(echo $i | cut -d. -f2)
		t3=$(echo $i | cut -d. -f3-)
		echo $i |grep PROXY > /dev/null
		if [ $? -eq 0 ] ; then
			s=$(echo $t3 | sed -e 's{proxy{proxy-{g' -e 's{IMAP{imapd{g' -e 's{POP3{pop3d{g')
		else
			s=qmail-"$t3"
		fi
		if [ ! -d $LOGDIR/$t3 ] ; then
			continue
		fi
		echo "#-------------------------------------------------------------------"
		echo "Title[$t1]: $t1 Concurrency (port $t2) - $1"
		echo "MaxBytes[$t1]: 100"
		echo "AbsMax[$t1]: 500"
		echo "Options[$t1]: growright,unknaszero,nopercent,gauge,pngdate"
		echo "Target[$t1]: \`$libexecdir/qmailmrtg -t $LOGDIR/$t3 $SERVICEDIR/$s\`"
		echo "PageTop[$t1]: <B>$t1 Concurrency</B><BR>"
		echo "ShortLegend[$t1]: concurrency"
		t4=$(echo $t1|sed 's{PROXY{{g')
		echo "YLegend[$t1]: max $t4"
		echo "Legend1[$t1]: Peak Connections&nbsp;"
		echo "Legend2[$t1]: Max Concurrency&nbsp;"
		echo "LegendI[$t1]: $t1 Concurrency&nbsp;"
		echo "LegendO[$t1]: Max Configured $t1&nbsp;"
		echo "WithPeak[$t1]: ymwd"
		echo "XSize[$t1]: 350"
		echo "YSize[$t1]: 150"
		echo ""
		echo "#-------------------------------------------------------------------"
		echo "Title[$t1-total]: $t1 allowed/denied (port $t2) - $1"
		echo "MaxBytes[$t1-total]: 1000"
		echo "AbsMax[$t1-total]: 100000"
		echo "Options[$t1-total]: growright,unknaszero,nopercent,gauge,pngdate"
		echo "Target[$t1-total]: \`$libexecdir/qmailmrtg -a $LOGDIR/$t3 $SERVICEDIR/$s\`"
		echo "PageTop[$t1-total]: <B>$t1 Connections</B><BR>"
		echo "ShortLegend[$t1-total]: conn/hour"
		t4=$(echo $t1|sed 's{PROXY{{g')
		echo "YLegend[$t1-total]: $t4-$t2 conn/hr"
		echo "Legend1[$t1-total]: Allowed Connections&nbsp;"
		echo "Legend2[$t1-total]: Denied Connections&nbsp;"
		echo "LegendI[$t1-total]: Allowed&nbsp;"
		echo "LegendO[$t1-total]: Denied&nbsp;"
		echo "WithPeak[$t1-total]: ymwd"
		echo "XSize[$t1-total]: 350"
		echo "YSize[$t1-total]: 150"
		echo ""
	done
	if [ -d $LOGDIR/inlookup.infifo ] ; then
		for i in user relay password limit alias host domain cached
		do
			opt=$(echo $i |cut -c1)
			echo "#-------------------------------------------------------------------"
			echo "Title[inlookup-$i]: $i lookup - $1"
			echo "MaxBytes[inlookup-$i]: 1000"
			echo "AbsMax[inlookup-$i]: 100000"
			echo "Options[inlookup-$i]: nopercent,growright,integer,gauge,pngdate"
			echo "Target[inlookup-$i]: \`$libexecdir/qmailmrtg -i $opt $LOGDIR/inlookup.infifo $SERVICEDIR/inlookup.infifo\`"
			echo "PageTop[inlookup-$i]: <B>Inlookup $i query count</B><br>"
			echo "ShortLegend[inlookup-$i]: q"
			echo "YLegend[inlookup-$i]: queries"
			echo "Legend1[inlookup-$i]: $i queries"
			echo "Legend2[inlookup-$i]: total queries"
			echo "LegendI[inlookup-$i]: $i queries"
			echo "LegendO[inlookup-$i]: total queries"
			echo "WithPeak[inlookup-$i]: ymwd"
			echo "XSize[inlookup-$i]: 350"
			echo "YSize[inlookup-$i]: 150"
			echo ""
		done
		echo "#-------------------------------------------------------------------"
		echo "Title[inlookup-cachehit]: inlookup Cache Hits - $1"
		echo "MaxBytes[inlookup-cachehit]: 1000"
		echo "AbsMax[inlookup-cachehit]: 100000"
		echo "Options[inlookup-cachehit]: nopercent,growright,integer,gauge,pngdate"
		echo "Target[inlookup-cachehit]: \`$libexecdir/qmailmrtg -i C $LOGDIR/inlookup.infifo $SERVICEDIR/inlookup.infifo\`"
		echo "PageTop[inlookup-cachehit]: <B>Inlookup Cache Hits</B><br>"
		echo "ShortLegend[inlookup-cachehit]: hits/hour"
		echo "YLegend[inlookup-cachehit]: cache hits/hour"
		echo "Legend1[inlookup-cachehit]: cache hits/hour"
		echo "Legend2[inlookup-cachehit]: queries/hour"
		echo "LegendI[inlookup-cachehit]: cache hits/hour"
		echo "LegendO[inlookup-cachehit]: queries/hour"
		echo "WithPeak[inlookup-cachehit]: ymwd"
		echo "XSize[inlookup-cachehit]: 350"
		echo "YSize[inlookup-cachehit]: 150"
		echo ""
	fi
	if [ -d $LOGDIR/dnscache ] ; then
		echo "#-------------------------------------------------------------------"
		echo "Title[dnscache]: dnscache Queries - $1"
		echo "MaxBytes[dnscache]: 1000"
		echo "AbsMax[dnscache]: 100000"
		echo "Options[dnscache]: nopercent,growright,gauge,pngdate"
		echo "Target[dnscache]: \`$libexecdir/qmailmrtg -d $LOGDIR/dnscache $SERVICEDIR/dnscache\`"
		echo "PageTop[dnscache]: <B>dnscache Queries</B><br>"
		echo "ShortLegend[dnscache]: &nbsp;"
		echo "YLegend[dnscache]: Queries/hour"
		echo "Legend1[dnscache]: Cached&nbsp;"
		echo "Legend2[dnscache]: Queries&nbsp;"
		echo "LegendI[dnscache]: Cached&nbsp;"
		echo "LegendO[dnscache]: Queries&nbsp;"
		echo "WithPeak[dnscache]: ymwd"
		echo "XSize[dnscache]: 350"
		echo "YSize[dnscache]: 150"
		echo ""
	fi
	if [ -d $LOGDIR/tinydns ] ; then
		echo "#-------------------------------------------------------------------"
		echo "Title[tinydns]: tinydns Queries - $1"
		echo "MaxBytes[tinydns]: 1000"
		echo "AbsMax[tinydns]: 100000"
		echo "Options[tinydns]: nopercent,growright,gauge,pngdate"
		echo "Target[tinydns]: \`$libexecdir/qmailmrtg -l $LOGDIR/tinydns $SERVICEDIR/tinydns\`"
		echo "PageTop[tinydns]: <B>tinydns Queries</B><br>"
		echo "ShortLegend[tinydns]: Queries"
		echo "YLegend[tinydns]: Queries/hour"
		echo "Legend1[tinydns]: Queries&nbsp;"
		echo "LegendI[tinydns]: Queries&nbsp;"
		echo "LegendO[tinydns]: &nbsp;"
		echo "WithPeak[tinydns]: ymwd"
		echo "XSize[tinydns]: 350"
		echo "YSize[tinydns]: 150"
		echo ""
	fi

	if [ -d $LOGDIR/mysql.3306 ] ; then
		if [ -x \$MYSQL_BASE/bin/mariadb-admin ] ; then
		  mysqladmin=/usr/bin/mariadb-admin
		else
		  mysqladmin=/usr/bin/mysqladmin
		fi
		echo "#"
		echo "# MySQL QUERIES"
		echo "#"
		echo "Title[mysql_queries]: MySQL query count - $1"
		echo "Target[mysql_queries]: \`$mysqladmin -u $REPL_USER -p$REPL_PASS ver|awk '/Qu/{q=\$20;s=\$7}/Up/{u=\$0}/Se/{v=\$3}END{print q\"\\n\"s\"\\n\"u\"\\nMySQL version \"v}'|sed 's/Uptime:[[:space:]]\+//'\`"
		echo "Options[mysql_queries]: nopercent, growright, integer, perminute, pngdate"
		echo "MaxBytes[mysql_queries]: 250000"
		echo "PageTop[mysql_queries]: <h1>MySQL query count</h1>"
		echo "ShortLegend[mysql_queries]: q/m"
		echo "YLegend[mysql_queries]: queries/minute"
		echo "Legend1[mysql_queries]: queries/minute"
		echo "Legend2[mysql_queries]: slow queries"
		echo "LegendI[mysql_queries]: queries/minute"
		echo "LegendO[mysql_queries]: slow queries"
		echo "XSize[mysql_queries]: 350"
		echo "YSize[mysql_queries]: 150"
		echo ""
		echo "#"
		echo "# MySQL CPU / MEMORY"
		echo "#"
		echo "Title[mysql_cpumem]: MySQL cpu / memory usage - $1"
		echo "Target[mysql_cpumem]: \`ps ax -o %cpu,%mem,comm|grep mysqld|awk 'BEGIN{cpu=0;mem=0}{cpu+=\$1;mem+=\$2}END{print cpu\"\\n\"mem\"\\nUNKNOWN\\nUNKNOWN\"}'\`"
		echo "Options[mysql_cpumem]: nopercent, growright, gauge, noinfo, unknaszero, pngdate"
		echo "MaxBytes[mysql_cpumem]: 100"
		echo "PageTop[mysql_cpumem]: <h1>MySQL cpu / memory usage</h1>"
		echo "ShortLegend[mysql_cpumem]: %"
		echo "YLegend[mysql_cpumem]: Percents"
		echo "Unscaled[mysql_cpumem]: ymwd"
		echo "LegendI[mysql_cpumem]: CPU"
		echo "LegendO[mysql_cpumem]: Memory"
		echo "Legend1[mysql_cpumem]: CPU"
		echo "Legend2[mysql_cpumem]: Memory"
		echo "XSize[mysql_cpumem]: 350"
		echo "YSize[mysql_cpumem]: 150"
	fi
	if [ -d $LOGDIR/clamd ] ; then
		echo "#-------------------------------------------------------------------"
		echo "Title[clamd]: clamd - $1"
		echo "MaxBytes[clamd]: 10000"
		echo "AbsMax[clamd]: 100000"
		echo "Options[clamd]: growright,unknaszero,nopercent,gauge,pngdate"
		echo "Target[clamd]: \`$libexecdir/qmailmrtg -C $LOGDIR/clamd $SERVICEDIR/clamd\`"
		echo "PageTop[clamd]: <B>ClamAV</B><br>"
		echo "ShortLegend[clamd]: Msg"
		echo "YLegend[clamd]: viri/hour"
		echo "Legend1[clamd]: Virus&nbsp;"
		echo "Legend2[clamd]: Errors&nbsp;"
		echo "LegendI[clamd]: found&nbsp;"
		echo "LegendO[clamd]: errors:&nbsp;"
		echo "WithPeak[clamd]: ymwd"
		echo "XSize[clamd]: 350"
		echo "YSize[clamd]: 150"
		echo ""
	fi
	for i in $(grep -E -v "lo:|virb|ztk|ztr|vbox" /proc/net/dev|tail -n +3|cut -d: -f1)
	do
		echo "# ----- interface $i -----------------------"
		echo "Title[$i]: $i bits - $1"
		echo "Options[$i]: bits,growright,pngdate"
		echo "Target[$i]: \`grep $i /proc/net/dev | sed 's/$i://' | awk '{print \$1; print \$9; print \"\"; print \"\"}'\`"
		echo "PageTop[$i]: $i bits - $i"
		echo "MaxBytes[$i]: 100000000"
		echo "Ylegend[$i]: bits"
		echo "ShortLegend[$i]: bits"
		echo "XSize[$i]: 350"
		echo "YSize[$i]: 150"
		echo "Legend1[$i]: bits&nbsp;"
		echo "LegendI[$i]: input&nbsp;"
		echo "LegendO[$i]: output&nbsp;"
		echo "WithPeak[$i]: ymwd"
		echo
	done
}

create_mrtg()
{
if [ $# -lt 2 ] ; then
	echo "create_mrtg htmldir service_dir" 1>&2
	return 1
fi
htmldir=$1
SERVICEDIR=$2
conf_dir="$DESTDIR"$SERVICEDIR/mrtg/variables
if [ $nooverwrite -eq 1 -a -d $conf_dir ] ; then
	return 0
fi
indexmaker=$(which indexmaker)
if [ -z "$indexmaker" -o ! -f /usr/bin/mrtg ] ; then
	echo "mrtg or indexmaker missing: Have you installed mrtg?" 1>&2
	return 1
fi
/bin/mkdir -p $htmldir
$chown root:0 $htmldir
/bin/chmod 755 $htmldir
# mrtg - script
/bin/mkdir -p "$DESTDIR"$SERVICEDIR/mrtg/log
if [ $down_state -eq 1 ] ; then
	$TOUCH "$DESTDIR"$SERVICEDIR/mrtg/down
fi
/bin/mkdir -p $conf_dir
/bin/chmod 500 $conf_dir
$chown root:0 $conf_dir
/bin/mkdir -p "$conf_dir"/../etc
/bin/chmod 555 $conf_dir/../etc
$chown root:0 $conf_dir/../etc
echo C > $conf_dir/LANG
hostname=$([ -n "$HOSTNAME" ] && echo "$HOSTNAME" || uname -n)
(
sed "s{MRTGHOSTNAME{$hostname{;  s{HTMLDIR{$htmldir\/{" $sysconfdir/indimail.mrtg.cfg
mrtg_extra $hostname
) > $conf_dir/../etc/indimail.mrtg.cfg
chmod 600 $conf_dir/../etc/indimail.mrtg.cfg
#
# Create index.html in htmldir
#
$indexmaker --title="IndiMail Statistics" --section=title \
	--output=$htmldir/index.html  $conf_dir/../etc/indimail.mrtg.cfg
if [ -f @qsysconfdir@/favicon.base64 ] ; then
	base64 -d < @qsysconfdir@/favicon.base64 > $htmldir/favicon.png
	sed -i '/<TITLE>IndiMail Statistics<\/TITLE>/a\'$'\n''\    <link rel="icon" type="image/x-icon" href="favicon.png">' $htmldir/index.html 
fi
# mrtg run script
(
dump_run_header
echo "exec 2>&1"
echo "while true"
echo "do"
echo "  $QmailBinPrefix/bin/envdir $envdir_opts variables \\"
echo "    /usr/bin/mrtg etc/indimail.mrtg.cfg"
echo "  status=\$?"
echo "  echo \"mrtg \`pwd\`/etc/indimail.mrtg.cfg status \$status\""
if [ " $scan_interval" = " " ] ; then
echo "  sleep 300"
else
echo "  sleep $scan_interval"
fi
echo "done"
echo "exit \$status"
) > "$DESTDIR"$SERVICEDIR/mrtg/run
/bin/chmod +x "$DESTDIR"$SERVICEDIR/mrtg/run

if [ ! -d $LOGDIR ] ; then
	/bin/mkdir -p $LOGDIR
	$chown qmaill:nofiles $LOGDIR
fi
# mrtg log script
dump_log_header mrtg > "$DESTDIR"$SERVICEDIR/mrtg/log/run
/bin/chmod +x "$DESTDIR"$SERVICEDIR/mrtg/log/run
echo "$prog_args" > $conf_dir/.options
}

rmsvc()
{
if [ $# -lt 1 ] ; then
	echo "rmsvc service_with_full_path" 1>&2
	return 1
fi
if [ -d /run ] ; then
	rundir=/run/svscan
elif [ -d /var/run ] ; then
	rundir=/var/run/svscan
fi
for i in $*
do
	if [ ! -d $i ] ; then
		echo "$i: No such file or directory" 1>&2
		continue
	fi
	dir=`dirname $i`
	svcname=`basename $i`
	first_char=`echo ${svcname} | cut -c1`
	if [ " $first_char" = " ." ] ; then
		echo "skipping $i that starts with ." 1>&2
		continue
	fi
	if [ -n "$rundir" ] ; then
		if [ ! -p $rundir/${svcname}/supervise/control ] ; then
			echo "$i/supervise not a supervise directory" 1>&2
			continue
		fi
	else
		if [ ! -p $i/supervise/control ] ; then
			echo "$i/supervise not a supervise directory" 1>&2
			continue
		fi
	fi
	/bin/mv $i $dir/"."${svcname}
	if [ $? -ne 0 ] ; then
		continue
	fi
	if [ -n "$rundir" ] ; then
		if [ -d $rundir/${svcname} -a ! -d $rundir/"."${svcname} ] ; then
			/bin/mv $rundir/${svcname} $rundir/"."${svcname}
			if [ $? -ne 0 ] ; then
				continue
			fi
		fi
		dir=$rundir
	fi
	#
	# prepend a . to the directory so that svscan will skip this directory
	#
	echo "Removed Service ${svcname}"
	"$DESTDIR"$prefix/bin/svc -dx $dir/"."${svcname}
	if [ -d $dir/"."${svcname}/log ] ; then
		echo "Removed Service ${svcname}/log"
		"$DESTDIR"$prefix/bin/svc -dx $dir/"."${svcname}/log
	fi
done
}

ensvc()
{
if [ $# -lt 1 ] ; then
	echo "ensvc service_with_full_path" 1>&2
	return 1
fi
if [ -d /run ] ; then
	rundir=/run/svscan
elif [ -d /var/run ] ; then
	rundir=/var/run/svscan
fi
for i in $*
do
	if [ ! -d $i ] ; then
		echo "$i: No such file or directory" 1>&2
		continue
	fi
	dir=`dirname $i`
	svcname=`basename $i`
	first_char=`echo ${svcname} | cut -c1`
	if [ " $first_char" = " ." ] ; then
		second_char=`echo ${svcname} | cut -c2`
		if [ " $second_char" = " ." -o -z "$second_char" ] ; then
			echo "skipping $i" 1>&2
			continue
		fi
		svcname=`echo ${svcname} | cut -c2-`
		if [ -d $dir/${svcname} ] ; then
			echo "$dir/${svcname} exists"
			continue;
		fi
		/bin/mv $i $dir/${svcname}
		if [ $? -ne 0 ] ; then
			continue
		fi
		if [ -n "$rundir" ] ; then
			/bin/rm -rf $rundir/"."${svcname}
		fi
		echo "Enabled Service ${svcname}"
	else
		echo "Service $i is not disabled (should have a '.' as the first char in ${svcname})" 1>&2
	fi
done
sleep 5
for i in $*
do
	dir=`dirname $i`
	svcname=`basename $i`
	first_char=`echo ${svcname} | cut -c1`
	if [ " $first_char" = " ." ] ; then
		second_char=`echo ${svcname} | cut -c2`
		if [ " $second_char" = " ." -o -z "$second_char" ] ; then
			echo "skipping $i" 1>&2
			continue
		fi
		svcname=`echo ${svcname} | cut -c2-`
		"$DESTDIR"$prefix/bin/svstat $dir/$svcname
		if [ -d $dir/${svcname}/log ] ; then
			"$DESTDIR"$prefix/bin/svstat $dir/${svcname}/log
		fi
	fi
done
}

refreshsvc()
{
if [ $# -lt 1 ] ; then
	echo "refreshsvc service_with_full_path" 1>&2
	return 1
fi
if [ " $1" = " all" ] ; then
	for j in `/bin/ls "$DESTDIR"$servicedir/*/variables/.options "$DESTDIR"$servicedir/.svscan/variables/.options 2>/dev/null`
	do
		# remove last 19 char to get the directory+service_name
		i=`echo $j|sed 's/.\{19\}$//'`
		svcname=`basename $i`
		if [ ! -s $i/variables/.options -a ! -s $i/.options ] ; then
			echo "$i/variables/.options or $i/.options missing" 1>&2
			continue
		fi
		if [ -f $i/variables/.norefreshsvc -a $force -eq 0 ] ; then
			if [ $silent -eq 0 ] ; then
				echo "Skip Refreshing service $svcname"
			fi
		else
			if [ $silent -eq 0 ] ; then
				echo "Refreshing service $svcname command $(@prefix@/bin/qcat $i/variables/.options)"
			fi
			sh $i/variables/.options
		fi
	done
	if [ -f "$DESTDIR"$sysconfdir/control/defaultqueue/.norefreshsvc -a $force -eq 0 ] ; then
		if [ $silent -eq 0 ] ; then
			echo "Skip Refreshing config defaultqueue"
		fi
	else
		if [ $silent -eq 0 ] ; then
			echo "Refreshing config defaultqueue command $(@prefix@/bin/qcat "$DESTDIR"$sysconfdir/control/defaultqueue/.options)"
		fi
		sh "$DESTDIR"$sysconfdir/control/defaultqueue/.options
	fi
else
	for i in $*
	do
		if [ ! -d "$DESTDIR"$i ] ; then
			echo "$i: No such file or directory" 1>&2
			continue
		fi
		svcname=`basename $i`
		if [ ! -s "$DESTDIR"$i/variables/.options -a ! -s "$DESTDIR"$i/.options ] ; then
			echo "$i/variables/.options or $i/.options missing" 1>&2
			continue
		fi
		if [ -f "$DESTDIR"$i/variables/.norefreshsvc -o -f "$DESTDIR"$i/.norefreshsvc ] ; then
			if [ $force -eq 0 ] ; then
				if [ $silent -eq 0 ] ; then
					echo "Skip Refreshing service/config $svcname"
				fi
				continue
			fi
		fi
		if [ -f "$DESTDIR"$i/variables/.options ] ; then
			if [ $silent -eq 0 ] ; then
				echo "Refreshing service $svcname command $(@prefix@/bin/qcat "$DESTDIR"$i/variables/.options)"
			fi
			sh "$DESTDIR"$i/variables/.options
		elif [ -f "$DESTDIR"$i/.options ] ; then
			if [ $silent -eq 0 ] ; then
				echo "Refreshing config $svcname command $(@prefix@/bin/qcat "$DESTDIR"$i/.options)"
			fi
			sh "$DESTDIR"$i/.options
		fi
	done
fi
}

autorefresh()
{
if [ $# -lt 2 ] ; then
	echo "autorefresh=0|1 service_with_full_path" 1>&2
	return 1
fi
svc_opt=$1
shift
if [ " $1" = " all" ] ; then
	for j in `/bin/ls "$DESTDIR"$servicedir/*/variables/.options "$DESTDIR"$servicedir/.svscan/variables/.options 2>/dev/null`
	do
		# remove last 19 char to get the directory+service_name
		i=`echo $j|sed 's/.\{19\}$//'`
		if [ ! -s $i/variables/.options -a ! -s $i/.options ] ; then
			echo "$i/variables/.options or $i/.options missing" 1>&2
			continue
		fi
		if [ $svc_opt -eq 1 ] ; then
			if [ $silent -eq 0 ] ; then
				echo "Enabling  auto refresh for $i"
			fi
			/bin/rm -f $i/variables/.norefreshsvc
		else
			if [ $silent -eq 0 ] ; then
				echo "Disabling auto refresh for $i"
			fi
			$touch $i/variables/.norefreshsvc
		fi
	done
	if [ $svc_opt -eq 1 ] ; then
		if [ $silent -eq 0 ] ; then
			echo "Enabling  auto refresh for $sysconfdir/control/defaultqueue"
		fi
		/bin/rm -f "$DESTDIR"$sysconfdir/control/defaultqueue/.norefreshsvc
	else
		if [ $silent -eq 0 ] ; then
			echo "Disabling auto refresh for $sysconfdir/control/defaultqueue"
		fi
		$touch "$DESTDIR"$sysconfdir/control/defaultqueue/.norefreshsvc
	fi
else
	for i in $*
	do
		if [ ! -d "$DESTDIR"$i ] ; then
			echo "$i: No such file or directory" 1>&2
			continue
		fi
		if [ ! -s "$DESTDIR"$i/variables/.options -a ! -s "$DESTDIR"$i/.options ] ; then
			echo "$i/variables/.options or $i/.options missing" 1>&2
			continue
		fi
		if [ $svc_opt -eq 1 ] ; then
			if [ $silent -eq 0 ] ; then
				echo "Enabling  auto refresh for $1"
			fi
			if [ -d $i/variables ] ; then
				/bin/rm -f "$DESTDIR"$i/variables/.norefreshsvc
			else
				/bin/rm -f "$DESTDIR"$i/.norefreshsvc
			fi
		else
			if [ $silent -eq 0 ] ; then
				echo "Disabling auto refresh for $i"
			fi
			if [ -d "$DESTDIR"$i/variables ] ; then
				$touch "$DESTDIR"$i/variables/.norefreshsvc
			else
				$touch "$DESTDIR"$i/.norefreshsvc
			fi
		fi
	done
fi
}

enable_service()
{
	for i in $*
	do
		#
		# upstart/systemd
		#
		if [ -x /bin/systemctl -a -f /lib/systemd/system/$i.service -a -f /bin/systemctl ] ; then
			/bin/systemctl enable "$i".service
		elif [ -x /bin/systemctl -a -f /usr/lib/systemd/system/$i.service -a -f /bin/systemctl ] ; then
			/bin/systemctl enable "$i".service
		elif [ -f /Library/LaunchDaemons/org.indimail.svscan.plist ] ; then
			launchctl load -w /Library/LaunchDaemons/org.indimail.svscan.plist
		elif [ -f /sbin/initctl -a -f /etc/init/$i.override ] ; then
			/bin/rm -f /etc/init/$i.override
		elif [ -f /etc/event.d/$i.override ] ; then
			/bin/rm -f /etc/event.d/$i.override
		elif [ -f $QmailBinPrefix/etc/rc.d/svscan ] ; then
			service svscan enable
		fi
		#
		# traditional sys v
		#
		if [ -f /etc/init.d/$i ] ; then
			if [ -f /sbin/chkconfig ] ; then
				if [ -f /etc/debian_version ] ; then
					/sbin/chkconfig --add $i 2>/dev/null
				else
					/sbin/chkconfig --add $i
				fi
			elif [ -f /sbin/rc-update ] ; then
				/sbin/rc-update add $i
			elif [ -f /usr/sbin/update-rc.d ] ; then
				/usr/sbin/update-rc.d $i defaults
				/usr/sbin/update-rc.d $i enable
			fi
		fi
	done
}

disable_service()
{
	for i in $*
	do
		#
		# upstart/systemd
		#
		if [ -x /bin/systemctl -a -d /lib/systemd/system -a -f /bin/systemctl ] ; then
			/bin/systemctl stop "$i".service
			/bin/systemctl disable "$i".service
		elif [ -x /bin/systemctl -a -d /usr/lib/systemd/system -a -f /bin/systemctl ] ; then
			/bin/systemctl stop "$i".service
			/bin/systemctl disable "$i".service
		elif [ -f /Library/LaunchDaemons/org.indimail.svscan.plist ] ; then
			launchctl unload -w /Library/LaunchDaemons/org.indimail.svscan.plist 2>/dev/null
			launchctl remove /Library/LaunchDaemons/org.indimail.svscan.plist
		elif [ -f /sbin/initctl -a -d /etc/init ] ; then
			/sbin/initctl stop $i
			if [ -f /etc/init/$i.conf ] ; then
				echo "manual" > /etc/init/$i.override
			fi
		elif [ -d /etc/event.d ] ; then
			/sbin/initctl stop $i
			echo "manual" > /etc/event.d/$i.override
		elif [ -f $QmailBinPrefix/etc/rc.d/svscan ] ; then
			service svscan disable
		else
			/usr/sbin/service $i stop
		fi
		#
		# traditional sys v
		#
		if [ -f /etc/init.d/$i ] ; then
			/etc/init.d/$i stop
			if [ -f /sbin/chkconfig ] ; then
				if [ -f /etc/debian_version ] ; then
					/sbin/chkconfig --del $i 2>/dev/null
				else
					/sbin/chkconfig --del $i
				fi
			elif [ -f /sbin/rc-update ] ; then
				/sbin/rc-update del $i
			elif [ -f /usr/sbin/update-rc.d ] ; then
				/usr/sbin/update-rc.d -f $i disable
				/usr/sbin/update-rc.d -f $i remove
			fi
		fi
	done
}

macOSgroupadd()
{
	echo "groupadd $*"
	groupid=""
	while test $# -gt 1; do
		case "$1" in
		-g)
		groupid=$2
		for i in `dscl . -list /Groups PrimaryGroupID | awk '{print $2}'`
		do
			if [ $i -eq $groupid ] ; then
				echo "groupid $groupid not unique" 1>&2
				return 1
			fi
		done
		;;
		esac
		shift
	done
	/usr/bin/dscl . -list /Groups/$1 > /dev/null 2>&1
	if [ $? -eq 0 ] ; then
		echo "group $1 exists"
		return 1
	fi
	if [ -n "$groupid" ] ; then
		echo "dscl . -create /Groups/$1 PrimaryGroupID $groupid"
		/usr/bin/dscl . -create /Groups/$1 PrimaryGroupID $groupid
	else
		echo "dseditgroup -o create $1"
		/usr/sbin/dseditgroup -o create $1
	fi
	return $?
}

macOSuseradd()
{
	echo "useradd $*"
	userid=""
	group=""
	members=""
	homedir=""
	shell=""
	password=""
	create_home=1
	while test $# -gt 1; do
		case "$1" in
		-M)
		create_home=0
		;;
		-u)
		userid=$2
		for i in `dscl . -list /Users UniqueID | awk '{print $2}'`
		do
			if [ $i -eq $userid ] ; then
				echo "userid $userid not unique" 1>&2
				return 1
			fi
		done
		;;
		-g)
		group=$2
		dscl . -read /Groups/$group PrimaryGroupID >/dev/null 2>&1
		if [ $? -ne 0 ] ; then
			echo "error with Group $group" 1>&2
			return 1
		fi
		groupid=`dscl . -read /Groups/$group PrimaryGroupID | grep -v "No such key"| awk '{print $2}'`
		;;
		-G)
		members=$2
		;;
		-d)
		homedir=$2
		;;
		-s)
		shell=$2
		;;
		-p)
		password=$2
		;;
		esac
		shift
	done
	dscl . -list /Users/$1 > /dev/null 2>&1
	if [ $? -eq 0 ] ; then
		echo "user $1 exists"
		return 1
	fi
	echo "dscl . -create /Users/$1 UniqueID $userid"
	dscl . -create /Users/$1 UniqueID $userid
	echo "dscl . -create /Users/$1 home $homedir"
	dscl . -create /Users/$1 home $homedir
	echo "dscl . -create /Users/$1 PrimaryGroupID $groupid"
	dscl . -create /Users/$1 PrimaryGroupID $groupid
	echo "dscl . -create /Users/$1 UserShell $shell"
	dscl . -create /Users/$1 UserShell $shell
	echo "dscl . -create /Users/$1 RealName $1"
	dscl . -create /Users/$1 RealName $1
	if [ ! " $password" = " " ] ; then
		echo "dscl . -passwd /Users/$1 xxxxxxxx"
		dscl . -passwd /Users/$1 $password
	fi
	if [ ! " $members" = " " ] ; then
		IFS=,
		j=`echo $members`
		unset IFS
		for i in $j
		do
			echo "dscl . -append /Groups/$i GroupMembership $1"
			dscl . -append /Groups/$i GroupMembership $1 >/dev/null 2>&1
		done
	fi
	if [ $create_home -eq 1 ] ; then
		if [ ! -d $homedir ] ; then
			mkdir -p $homedir
			chmod 700 $homedir
			chown $homedir $userid:$groupid
		fi
	fi
}

linuxuseradd()
{
	userid=""
	group=""
	gecos=""
	members=""
	homedir=""
	shell=""
	password=""
	create_home=1
	nolastlog=0
	args=""

	case "$host" in
		*-*-freebsd*)
		pw=/usr/sbin/pw
		opt1=""
		opt3="-n"
		;;
		*)
		pw=""
		opt1="-r -M"
		opt3=""
		;;
	esac

	while test $# -gt 1; do
		case "$1" in
		-M)
		if [ -f /etc/debian_version -o -n "$pw" ] ; then
			opt2=""
		elif [ -f /etc/alpine-release ] ; then
			opt2="-H"
		else
			opt2="-M"
		fi
		;;
		-u)
		userid=$2
		;;
		-g)
		group=$2
		;;
		-G)
		members=$2
		;;
		-c)
		gecos=$2
		;;
		-d)
		homedir=$2
		;;
		-s)
		shell=$2
		;;
		-p)
		password=$2
		;;
		-l)
		if [ ! -f /etc/alpine-release ] ; then
			nolastlog=1
		fi
		;;
		esac
		shift
	done
	getent passwd $1 >/dev/null && echo "user $1 exists" && return 0
	echo "creating user $1"
	if [ -n "$userid" ] ; then
		args="$args -u $userid"
	fi
	if [ -n "$shell" ] ; then
		args="$args -s $shell"
	fi
	if [ -f /etc/alpine-release ] ; then
		if [ -n "$group" ] ; then
			args="$args -G $group"
		fi
		if [ -n "$homedir" ] ; then
			args="$args -h $homedir"
		fi
		if [ -n "$gecos" ] ; then
			args="$args -g \"$gecos\""
		fi
		echo /usr/sbin/adduser $opt2 $args -D -S $1
		eval /usr/sbin/adduser $opt2 $args -D -S $1
		if [ $? -eq 0 -a -n "$members" ] ; then
			echo /usr/sbin/adduser $1 $members
			eval /usr/sbin/adduser $1 $members
		fi
	else
		if [ -n "$group" ] ; then
			args="$args -g $group"
		fi
		if [ -n "$homedir" ] ; then
			args="$args -d $homedir"
		fi
		if [ -n "$members" ] ; then
			args="$args -G $members"
		fi
		if [ -n "$gecos" ] ; then
			args="$args -c \"$gecos\""
		fi
		if [ -n "$pw" ] ; then
			echo $pw useradd $opt2 $args $opt3 $1
			eval $pw useradd $opt2 $args $opt3 $1
		else
			[ $nolastlog -eq 1 ] && echo /usr/sbin/useradd -l $opt1 $opt2 $args $1 || \
				echo /usr/sbin/useradd $opt1 $opt2 $args $1
			[ $nolastlog -eq 1 ] && eval /usr/sbin/useradd -l $opt1 $opt2 $args $1 || \
				eval /usr/sbin/useradd $opt1 $opt2 $args $1
		fi
	fi
	return $?
}

linuxgroupadd()
{
	args=""
	case "$host" in
		*-*-freebsd*)
		pw=/usr/sbin/pw
		opt1=""
		opt2=""
		opt3="-n"
		;;
		*)
		pw=""
		opt1="-r"
		opt2=""
		opt3=""
		;;
	esac

	groupid=""
	while test $# -gt 1; do
		case "$1" in
		-g)
		groupid=$2
		;;
		esac
		shift
	done
	if [ -n "$groupid" ] ; then
		args="$args -g $groupid"
	fi
	getent group $1 >/dev/null && echo "group $1 exists" && return 0
	echo "creating group $1"
	if [ -f /etc/alpine-release ] ; then
		echo /usr/sbin/addgroup -S $args $1
		/usr/sbin/addgroup -S $args $1
	else
		if [ -n "$pw" ] ; then
			echo $pw groupadd $args $opt3 $1
			$pw groupadd $args $opt3 $1
		else
			echo /usr/sbin/groupadd $opt1 $args $1
			/usr/sbin/groupadd $opt1 $args $1
		fi
	fi
	return $?
}

create_users()
{
	if [ -f /sbin/nologin ] ; then
		safe_shell=/sbin/nologin
	elif [ -f /usr/bin/false ] ; then
		safe_shell=/usr/bin/false
	elif [ -f /bin/false ] ; then
		safe_shell=/bin/false
	else
		safe_shell=/usr/bin/false
	fi
	(
	# mysql
	case "$host" in
		*-*-darwin*)
		/usr/bin/dscl . -list Groups/mysql >/dev/null 2>&1
		if [ $? -ne 0 ]; then
			echo "creating group mysql"
			macOSgroupadd -g 74 mysql
		fi
		/usr/bin/dscl . -list Users/mysql >/dev/null 2>&1
		if [ $? -ne 0 ]; then
			echo "creating user mysql"
			macOSuseradd -M -u 74 -g mysql -d /var/empty -s $safe_shell mysql
		fi
		;;
		*)
		linuxgroupadd mysql
		linuxuseradd -c "mysql user" -g mysql -d /var/lib/mysql -s $safe_shell mysql
		if [ $? -ne 0 ] ; then
			exit 1
		fi
		;;
	esac

	######### indimail #####################
	userid=555
	groupid=555
	case "$host" in
		*-*-darwin*)
		/usr/bin/dscl . -list Groups/indimail >/dev/null 2>&1
		if [ $? -ne 0 ]; then
			echo "creating group indimail"
			macOSgroupadd -g $groupid indimail
		fi
		/usr/bin/dscl . -list Users/indimail >/dev/null 2>&1
		if [ $? -ne 0 ]; then
			echo "creating user indimail"
			if [ -f /bin/bash ] ; then
				macOSuseradd -M -u $userid -g indimail -d $QmailHOME -s /bin/bash indimail
			elif [ -f /usr/local/bin/bash ] ; then
				macOSuseradd -M -u $userid -g indimail -d $QmailHOME -s /usr/local/bin/bash indimail
			else
				macOSuseradd -M -u $userid -g indimail -d $QmailHOME -s /bin/sh indimail
			fi
		fi
		;;
		*)
		#SALT=`date +'%s'`
		#CRYPT_PASS=$(openssl passwd -6 -salt $SALT $ADMIN_PASS)
		# echo $ADMIN_PASS | $pw $useradd $opt2 -g indimail -d $QmailHOME -s /bin/sh $opt3 indimail -h 0
		getent group $groupid >/dev/null
		if [ $? -eq 2 ] ; then
			linuxgroupadd -g $groupid indimail
		else
			linuxgroupadd indimail
		fi
		getent passwd $userid >/dev/null
		if [ $? -eq 2 ] ; then
			linuxuseradd -c "indmail user" -M -u $userid -g indimail -d $QmailHOME -s /bin/sh indimail
		else
			linuxuseradd -c "indmail user" -M -g indimail -d $QmailHOME -s /bin/sh indimail
		fi
		;;
	esac

	# qmail, nofiles, qscand, qcerts group
	for i in qmail nofiles qscand qcerts; do
		case "$host" in
		*-*-darwin*)
		groupid=`dscl . -list /Groups PrimaryGroupID | awk '{print $2}' | sort -n | tail -1`
		groupid=`expr $groupid + 1`
		/usr/bin/dscl . -list Groups/$i >/dev/null 2>&1
		if [ $? -ne 0 ]; then
			echo "creating group $i"
			macOSgroupadd -g $groupid $i
			if [ "$i" = "qcerts" ] ; then
				/usr/bin/dscl . -list Users/apache >/dev/null 2>&1
				if [ $? -eq 0 ] ; then
					echo "added supplementary group qcerts to apache"
					/usr/bin/dscl . append /Groups/qcerts GroupMembership apache
				fi
			fi
		fi
		;;
		*)
		linuxgroupadd $i
		if [ "$i" = "qcerts" ] ; then
			/usr/bin/getent group apache > /dev/null && /usr/sbin/usermod -aG qcerts apache
		fi
		;;
		esac
	done
	# alias user
	case "$host" in
		*-*-darwin*)
		userid=`expr $userid + 1`
		/usr/bin/dscl . -list Users/alias >/dev/null 2>&1
		if [ $? -ne 0 ]; then
			echo "Creating user alias"
			macOSuseradd -M -u $userid -g nofiles -d $QmailHOME/alias -s $safe_shell alias
		fi
		;;
		*)
		linuxuseradd -c "qmail alias user" -M -g nofiles -d $QmailHOME/alias -s $safe_shell alias
		;;
	esac
	# qmaill user
	case "$host" in
		*-*-darwin*)
		userid=`dscl . -list /Users UniqueID | awk '{print $2}' | sort -n |tail -1`
		userid=`expr $userid + 1`
		/usr/bin/dscl . -list Users/qmaill >/dev/null 2>&1
		if [ $? -ne 0 ]; then
			echo "Creating qmail log user qmaill"
			macOSuseradd -M -u $userid -g nofiles -d $LOGDIR -s $safe_shell qmaill
		fi
		;;
		*)
		linuxuseradd -c "qmail log user qmaill" -M -g nofiles -d $LOGDIR -s $safe_shell qmaill
		;;
	esac
	# qmaild, qmailp user
	for i in qmaild qmailp; do
		case "$i" in
			qmaild)
			t="qmail daemon user $i"
			;;
			qmailp)
			t="qmail password fetch user $i"
			;;
		esac
		case "$host" in
			*-*-darwin*)
			userid=`dscl . -list /Users UniqueID | awk '{print $2}' | sort -n |tail -1`
			userid=`expr $userid + 1`
			/usr/bin/dscl . -list Users/$i >/dev/null 2>&1
			if [ $? -ne 0 ]; then
				echo "Creating $t"
				if [ "$i" = "qmaild" ] ; then
					macOSuseradd -M -u $userid -g nofiles -d $QmailHOME -s $safe_shell -G qcerts $i
				else
					macOSuseradd -M -u $userid -g nofiles -d $QmailHOME -s $safe_shell $i
				fi
			fi
			;;
			*)
			if [ "$i" = "qmaild" ] ; then
				linuxuseradd -c "$t" -M -g nofiles -d $QmailHOME -s $safe_shell -G qcerts $i
			else
				linuxuseradd -c "$t" -M -g nofiles -d $QmailHOME -s $safe_shell $i
			fi
			;;
		esac
	done
	# qmailq, qmailr, qmails user
	for i in qmailq qmailr qmails; do
		case "$i" in
			qmailq)
			t="qmail-queue user $i"
			;;
			qmailr)
			t="qmail-remote user $i"
			;;
			qmails)
			t="qmail-send user $i"
			;;
		esac
		case "$host" in
			*-*-darwin*)
			userid=`dscl . -list /Users UniqueID | awk '{print $2}' | sort -n |tail -1`
			userid=`expr $userid + 1`
			/usr/bin/dscl . -list Users/$i >/dev/null 2>&1
			if [ $? -ne 0 ]; then
				echo "Creating $t"
				if [ "$i" = "qmailr" -o "$i" = "qmails" ] ; then
					macOSuseradd -M -u $userid -g qmail -d $QmailHOME -s $safe_shell -G qcerts $i
				else
					macOSuseradd -M -u $userid -g qmail -d $QmailHOME -s $safe_shell $i
				fi
			fi
			;;
			*)
			if [ "$i" = "qmailr" -o "$i" = "qmails" ] ; then
				linuxuseradd -c "$t" -M -g qmail -d $QmailHOME -s $safe_shell -G qcerts $i
			else
				linuxuseradd -c "$t" -M -g qmail -d $QmailHOME -s $safe_shell $i
			fi
			;;
		esac
	done
	# qscand user
	case "$host" in
		*-*-darwin*)
		userid=`dscl . -list /Users UniqueID | awk '{print $2}' | sort -n |tail -1`
		userid=`expr $userid + 1`
		/usr/bin/dscl . -list Users/qscand >/dev/null 2>&1
		if [ $? -ne 0 ]; then
			echo "creating user qscand"
			macOSuseradd -M -u $userid -g qscand -d $QmailHOME/qscanq -G qmail -s $safe_shell qscand
		fi
		;;
		*)
		linuxuseradd -c "indimail virus scan user" -M -g qscand -d $QmailHOME/qscanq -G qmail -s $safe_shell qscand
		;;
	esac
	case "$host" in
		*-*-darwin*)
		defaults read /Library/Preferences/com.apple.loginwindow SHOWOTHERUSERS_MANAGED >/dev/null
		if [ $? -eq 0 ] ; then
			case "$line" in
				indimail|alias|qmaild|qmaill|qmailp|qmailq|qmailr|qmails|qscand|mysql)
				dscl . create /Users/$line IsHidden 1
				;;
			esac
		else
			loginwindow="/Library/Preferences/com.apple.loginwindow"
			defaults read $loginwindow HiddenUsersList | while read line
			do
				if [ " $line" = " (" -o " $line" = " )" ] ; then
					continue
				fi
				line=`echo $line | cut -d, -f1`
				case "$line" in
					indimail|alias|qmaild|qmaill|qmailp|qmailq|qmailr|qmails|qscand|mysql)
					echo $line
					;;
					*)
					defaults write $loginwindow HiddenUsersList -array-add $line
					;;
				esac
			done
		fi
		;;
	esac
	) 2>$TMPDIR/svctool.user.$$
	if [ -s $TMPDIR/svctool.user.$$ ] ; then
		@prefix@/bin/qcat $TMPDIR/svctool.user.$$
		/bin/rm -f $TMPDIR/svctool.user.$$
		return 1
	else
		/bin/rm -f $TMPDIR/svctool.user.$$
		return 0
	fi
}

delUsers()
{
	case "$host" in
		*-*-freebsd*)
		pw=/usr/sbin/pw
		;;
		*)
		pw=""
		;;
	esac
	(
	case "$host" in
		*-*-darwin*)
		for i in indimail qmail nofiles qscand; do
			echo "deleting group $i"
			dscl . -delete /Groups/$i
		done
		for i in indimail alias qmaild qmaill qmailp qmailq qmailr qmails qscand; do
			echo "deleting user $i"
			dscl . -delete /Users/$i
		done
		;;
		*)
		for i in indimail qmail nofiles qscand; do
			echo "deleting group $i"
			$pw groupdel $i
		done
		for i in indimail alias qmaild qmaill qmailp qmailq qmailr qmails qscand; do
			echo "deleting user $i"
			$pw userdel $i
		done
		;;
	esac
	) 2>$TMPDIR/svctool.user.$$
	if [ -s $TMPDIR/svctool.user.$$ ] ; then
		@prefix@/bin/qcat $TMPDIR/svctool.user.$$
		/bin/rm -f $TMPDIR/svctool.user.$$
		return 1
	else
		/bin/rm -f $TMPDIR/svctool.user.$$
		return 0
	fi
}

create_snmpd_conf()
{
	if [ ! -d /etc/snmp ] ; then
		mkdir /etc/snmp
		chown root:0 /etc/snmp
	fi
	snmpdconf=@qsysconfdir@/snmpd.conf
	flag=0
	if [ -L /etc/snmp/snmpd.conf ] ; then
		t=$(readlink /etc/snmp/snmpd.conf)
		if [ "$t" != $snmpdconf ] ; then
			flag=1
		fi
	elif [ -f /etc/snmp/snmpd.conf ] ; then
		flag=1
	fi
	if [ $flag -eq 1 ] ; then
		echo "/etc/snmp/snmpd.conf exists!!!. Merge manually with $snmpdconf" 1>&2
	fi
	hostname=$([ -n "$HOSTNAME" ] && echo "$HOSTNAME" || uname -n)
	(
	echo "# snmpd.conf:"
	echo ""
	echo "###############################################################################"
	echo "# Access Control"
	echo "###############################################################################"
	echo "# First, map the community name \"public\" into a \"security name\""
	echo "rocommunity indimail"
	echo ""
	echo "#       sec.name  source          community"
	echo "#com2sec notConfigUser  default       public"
	echo "com2sec local     localhost           public"
	echo "com2sec mynetwork 192.168.2.0/24      public"
	echo ""
	echo "####"
	echo "# Second, map the security name into a group name:"
	echo ""
	echo "#       groupName      securityModel securityName"
	echo "group MyRWGroup v1         local"
	echo "group MyRWGroup v2c        local"
	echo "group MyRWGroup usm        local"
	echo "group MyROGroup v1         mynetwork"
	echo "group MyROGroup v2c        mynetwork"
	echo "group MyROGroup usm        mynetwork"
	echo ""
	echo "####"
	echo "# Third, create a view for us to let the group have rights to:"
	echo ""
	echo "# Make at least  snmpwalk -v 1 localhost -c public system fast again."
	echo "#       name           incl/excl     subtree         mask(optional)"
	echo "view    all           included   .1                   80"
	echo ""
	echo "####"
	echo "# Finally, grant the group read-only access to the systemview view."
	echo ""
	echo "#       group          context sec.model sec.level prefix read   write  notif"
	echo "access MyROGroup \"\"      any       noauth    exact  all    none   none"
	echo "access MyRWGroup \"\"      any       noauth    exact  all    all    none"
	echo ""
	echo ""
	echo "###############################################################################"
	echo "# System contact information"
	echo "#"
	echo ""
	echo "# It is also possible to set the sysContact and sysLocation system"
	echo "# variables through the snmpd.conf file:"
	echo ""
	echo "syslocation IndiMail ($hostname), Mail Server"
	echo "syscontact IndiMail <postmaster@$hostname>"
	echo ""
	echo "# proc: Check for processes that should be running."
	echo "#     proc NAME [MAX=0] [MIN=0]"
	echo "#   "
	echo "#     NAME:  the name of the process to check for.  It must match"
	echo "#            exactly (ie, http will not find httpd processes)."
	echo "#     MAX:   the maximum number allowed to be running.  Defaults to 0."
	echo "#     MIN:   the minimum number to be running.  Defaults to 0."
	echo "#   "
	echo "#   The results are reported in the prTable section of the UCD-SNMP-MIB tree"
	echo "#   Special Case:  When the min and max numbers are both 0, it assumes"
	echo "#   you want a max of infinity and a min of 1."
	echo ""
	echo "proc  svscan 1 "
	echo ""
	echo "# load: Check for unreasonable load average values."
	echo "#   Watch the load average levels on the machine."
	echo "#   "
	echo "#    load [1MAX=12.0] [5MAX=12.0] [15MAX=12.0]"
	echo "#   "
	echo "#    1MAX:   If the 1 minute load average is above this limit at query"
	echo "#            time, the errorFlag will be set."
	echo "#    5MAX:   Similar, but for 5 min average."
	echo "#    15MAX:  Similar, but for 15 min average."
	echo "#   "
	echo "#   The results are reported in the laTable section of the UCD-SNMP-MIB tree"
	echo ""
	echo "load  20 10 5"
	) > $snmpdconf.bak
	chmod 400 $snmpdconf.bak
	chown root:0 $snmpdconf.bak
	if [ -f $snmpdconf ] ; then
		diff $snmpdconf $snmpdconf.bak >/dev/null
		if [ $? -ne 0 ] ; then
			change_config $snmpdconf $snmpdconf.bak
			echo "$snmpdconf changed: Restart snmpd!!!" 1>&2
		fi
	else
		change_config $snmpdconf $snmpdconf.bak
	fi
	if [ $flag -eq 0 -a ! -L /etc/snmp/snmpd.conf ] ; then
		ln -s $snmpdconf /etc/snmp/snmpd.conf
	fi
}

default_alias_config()
{
	t=$QmailHOME/alias/Maildir
	if [ ! -d $t -o ! -d $t/new -o ! -d $t/cur -o ! -d $t/tmp ] ; then
		echo "Creating catch-all alias ($t) for all system users"
	fi
	if [ ! -d $QmailHOME/alias ] ; then
		mkdir -p $QmailHOME/alias
	fi
	if [ -x "$DESTDIR"$QmailBinPrefix/bin/maildirmake ] ; then
		"$DESTDIR"$QmailBinPrefix/bin/maildirmake "$DESTDIR"$t >/dev/null 2>&1
	else
		/bin/mkdir -p "$DESTDIR"$t/tmp >/dev/null 2>&1
		/bin/mkdir -p "$DESTDIR"$t/new >/dev/null 2>&1
		/bin/mkdir -p "$DESTDIR"$t/cur >/dev/null 2>&1
	fi
	if [ $? -eq 0 ] ; then
		$chown -R alias:qmail "$DESTDIR"$t
		/bin/chmod -R 775 "$DESTDIR"$t
	fi
	if [ $force -eq 1 -o ! -f "$DESTDIR"$QmailHOME/alias/.qmail-postmaster ] ; then
		/bin/rm -f "$DESTDIR"$QmailHOME/alias/.qmail-postmaster
		first_char=`echo $postmaster | cut -c1`
		if [ " $first_char" = " /" ] ; then
			if [ -x "$DESTDIR"$QmailBinPrefix/bin/maildirmake ] ; then
				"$DESTDIR"$QmailBinPrefix/bin/maildirmake "$DESTDIR"$postmaster >/dev/null 2>&1
			else
				/bin/mkdir -p "$DESTDIR"$postmaster/tmp >/dev/null 2>&1
				/bin/mkdir -p "$DESTDIR"$postmaster/new >/dev/null 2>&1
				/bin/mkdir -p "$DESTDIR"$postmaster/cur >/dev/null 2>&1
			fi
			if [ $? -eq 0 ] ; then
				$chown -R alias:qmail "$DESTDIR"$postmaster
				/bin/chmod -R 775 "$DESTDIR"$postmaster
			fi
		fi
		echo $postmaster > "$DESTDIR"$QmailHOME/alias/.qmail-postmaster
		$chown -R alias:qmail "$DESTDIR"$QmailHOME/alias/.qmail-postmaster
	fi
	cd "$DESTDIR"$QmailHOME/alias
	if [ $? -eq 0 ] ; then
		if [ ! -f .qmail-root -o $force -eq 1 ] ; then
			/bin/rm -f .qmail-root
			/bin/ln -s .qmail-postmaster .qmail-root
		fi
		if [ ! -f .qmail-mailer-daemon -o $force -eq 1 ] ; then
			/bin/rm -f .qmail-mailer-daemon
			/bin/ln -s .qmail-postmaster .qmail-mailer-daemon
		fi
	fi
}

create_qmail_config()
{
	default_qmail_control
	if [ -n "$postmaster" ] ; then
		default_alias_config
	fi
}

create_startup()
{
	case "$host" in
		*-*-darwin*)
		if [ -n "$DESTDIR" ] ; then
			mkdir -p "$DESTDIR"/etc
		fi
		if [ ! -f /etc/synthetic.conf ] ; then
			echo "Creating /service in /etc/synthetic.conf"
			printf "service\t/System/Volumes/data@qsysconfdir@/sv\n" > /etc/synthetic.conf
		else
			grep "^service" /etc/synthetic.conf >/dev/null
			if [ $? -ne 0 ] ; then
				echo "Creating /service in /etc/synthetic.conf"
				if [ -z "$DESTDIR" ] ; then
					printf "service\t/System/Volumes/data@qsysconfdir@/sv\n" >> /etc/synthetic.conf
				else
				(
					printf "service\t/System/Volumes/data@qsysconfdir@/sv\n"
					@prefix@/bin/qcat /etc/synthetic.conf
				) > "$DESTDIR"/etc/synthetic.conf
				fi
			else
				echo "/service exists in /etc/synthetic.conf" 1>&2
			fi
		fi
		;;
		*)
		if [ ! -d /service -a ! -L /service ] ; then
			if [ -d /run ] ; then
				ln -s /run/svscan /service
			elif [ -d /var/run ] ; then
				ln -s /var/run/svscan /service
			elif [ "$servicedir" != "/service" ] ; then
				ln -s $servicedir /service
			fi
		fi
		;;
	esac
	if [ -x "$DESTDIR"$QmailBinPrefix/sbin/initsvc -a ! -x /bin/busybox ] ; then
		"$DESTDIR"$QmailBinPrefix/sbin/initsvc -on
	else
		if [ -x /bin/systemctl -a -d /lib/systemd/system ] ; then
			cmp -s "$DESTDIR"$shareddir/boot/systemd /lib/systemd/system/svscan.service >/dev/null 2>&1
			if [ $? -ne 0 ] ; then
				echo "svscan startup enabled in systemd /lib/systemd/system/svscan.service"
				/bin/cp "$DESTDIR"$shareddir/boot/systemd /lib/systemd/system/svscan.service
				/bin/systemctl daemon-reload && echo "installed new svscan service"
			fi
			state=$(/bin/systemctl is-enabled svscan.service)
			if [ $? -ne 0 -a "$state" = "disabled" ] ; then
				/bin/systemctl enable svscan.service
			fi
		elif [ -x /bin/systemctl -a -d /usr/lib/systemd/system ] ; then
			cmp -s "$DESTDIR"$shareddir/boot/systemd /usr/lib/systemd/system/svscan.service >/dev/null 2>&1
			if [ $? -ne 0 ] ; then
				echo "svscan startup enabled in systemd /usr/lib/systemd/system/svscan.service"
				/bin/cp "$DESTDIR"$shareddir/boot/systemd /usr/lib/systemd/system/svscan.service
				/bin/systemctl daemon-reload && echo "installed new svscan service"
			fi
			state=$(/bin/systemctl is-enabled svscan.service)
			if [ $? -ne 0 -a "$state" = "disabled" ] ; then
				/bin/systemctl enable svscan.service
			fi
		elif [ -f /sbin/initctl -a -d /etc/init ] ; then
			cmp -s "$DESTDIR"$shareddir/boot/upstart /etc/init/svscan.conf >/dev/null 2>&1
			if [ $? -ne 0 ] ; then
				echo "svscan startup enabled in /etc/init/svscan.conf"
				/bin/cp "$DESTDIR"$shareddir/boot/upstart /etc/init/svscan.conf
			fi
		elif [ -d /etc/event.d ] ; then
			cmp -s "$DESTDIR"$shareddir/boot/upstart /etc/event.d/upstart >/dev/null 2>&1
			if [ $? -ne 0 ] ; then
				echo "svscan startup enabled in /etc/event.d/upstart"
				/bin/cp "$DESTDIR"$shareddir/boot/upstart /etc/event.d
			fi
		elif [ -d /etc/rc.d -a -f /etc/rc.subr ] ; then
			cmp -s "$DESTDIR"$shareddir/boot/svscan $QmailBinPrefix/etc/rc.d/svscan >/dev/null 2>&1
			if [ $? -ne 0 ] ; then
				echo "svscan startup enabled in rc $QmailBinPrefix/etc/rc.d/svscan"
				/bin/cp "$DESTDIR"$shareddir/boot/svscan $QmailBinPrefix/etc/rc.d/svscan
				/bin/chmod 755 $QmailBinPrefix/etc/rc.d/svscan
				service svscan enable
			fi
		elif [ -d /Library/LaunchDaemons ] ; then
			cmp -s "$DESTDIR"$shareddir/boot/svscan.plist /Library/LaunchDaemons/org.indimail.svscan.plist >/dev/null 2>&1
			if [ $? -ne 0 ] ; then
				echo "svscan startup enabled in /Library/LaunchDaemons/org.indimail.svscan.plist"
				/bin/cp "$DESTDIR"$shareddir/boot/svscan.plist /Library/LaunchDaemons/org.indimail.svscan.plist
			fi
		elif [ -f /sbin/openrc-run -a -d /etc/init.d ] ; then
			cmp -s "$DESTDIR"$shareddir/boot/openrc /etc/init.d/svscan >/dev/null 2>&1
			if [ $? -ne 0 ] ; then
				if [ $silent -eq 0 ] ; then
					echo "svscan startup enabled in rc /etc/init.d/svscan"
				fi
				/bin/cp "$DESTDIR"$shareddir/boot/openrc /etc/init.d/svscan
				/bin/chmod 755 /etc/init.d/svscan
				service svscan enable
			fi
		elif [ -f /etc/inittab ] ; then
			grep ".*:.*:respawn:.*svscanboot" /etc/inittab >/dev/null
			if [ $? -eq 0 ] ; then
	  			grep ".*:.*:respawn:.*svscanboot" /etc/inittab |grep respawn >/dev/null
				if [ $? -eq 0 ] ; then
					echo "svscan startup enabled in /etc/inittab"
				else
					echo "svscan startup configured in /etc/inittab but not enabled"
				fi
			fi
		fi
	fi
	if [ ! -f /sbin/openrc-run -a -d /etc/init.d ] ; then
		cmp -s "$DESTDIR"$QmailBinPrefix/bin/qmailctl /etc/init.d/svscan >/dev/null 2>&1
		if [ $? -ne 0 ] ; then
			/bin/cp "$DESTDIR"$QmailBinPrefix/bin/qmailctl /etc/init.d/svscan
		fi
	fi
	if [ -f /etc/init.d/svscan ] ; then
		if [ -f /sbin/chkconfig ] ; then
			if [ -f /etc/debian_version ] ; then
				/sbin/chkconfig --add svscan 2>/dev/null
			else
				/sbin/chkconfig --add svscan
			fi
		elif [ -f /usr/sbin/chkconfig ] ; then
			/usr/sbin/chkconfig --add svscan 2>/dev/null
		elif [ -f /usr/sbin/update-rc.d ] ; then
			/usr/sbin/update-rc.d svscan start 14 2 3 4 5 . stop 91 0 1 6 .
		fi
	fi
}

remove_startup()
{
	echo "Giving svscan exactly 5 seconds to exit nicely" 1>&2
	if [ -f "$DESTDIR"$QmailBinPrefix/sbin/initsvc -a ! -x /bin/busybox ] ; then
		"$DESTDIR"$QmailBinPrefix/sbin/initsvc -off
	fi
	if [ -f /lib/systemd/system/svscan.service -a -x /bin/systemctl ] ; then
		/bin/systemctl stop svscan
		/bin/systemctl disable svscan.service
		/bin/rm -f /lib/systemd/system/svscan.service
		/bin/systemctl daemon-reload && echo "removed svscan service"
	elif [ -f /usr/lib/systemd/system/svscan.service -a -x /bin/systemctl ] ; then
		/bin/systemctl stop svscan
		/bin/systemctl disable svscan.service
		/bin/rm -f /usr/lib/systemd/system/svscan
		/bin/systemctl daemon-reload && echo "removed svscan service"
	elif [ -f /etc/init/svscan.conf ] ; then
		/sbin/initctl emit qmailstop > /dev/null
		/bin/rm -f /etc/event.d/svscan
	elif [ -f /etc/event.d/svscan ] ; then
		/sbin/initctl emit qmailstop > /dev/null
		/bin/rm -f /etc/event.d/svscan
	elif [ -f /sbin/openrc-run ] ; then
		service svscan stop
		if [ -f /etc/init.d/svscan ] ; then
			service svscan disable
		fi
		/bin/rm -f /etc/init.d/svscan
	elif [ -f /etc/inittab ] ; then
		service svscan stop
		sed -i '/svscan/d' /etc/inittab
		kill -1 1
	elif [ -d /etc/rc.d -a -f /etc/rc.subr ] ; then
		service svscan stop
		if [ -f $prefix/etc/rc.d/svscan ] ; then
			service svscan disable && service svscan delete
			/bin/rm -f $QmailBinPrefix/etc/rc.d/svscan
		fi
	fi
	if [ -f /etc/init.d/svscan ] ; then
		if [ -f /sbin/chkconfig ] ; then
			if [ -f /etc/debian_version ] ; then
				/sbin/chkconfig --del svscan 2>/dev/null
			else
				/sbin/chkconfig --del svscan
			fi
		elif [ -f /usr/sbin/chkconfig ] ; then
			/usr/sbin/chkconfig --del svscan 2>/dev/null
		elif [ -f /usr/sbin/update-rc.d ] ; then
			/usr/sbin/update-rc.d -f svscan remove
		fi
		/bin/rm -f /etc/init.d/svscan
	fi
}

add_alternatives()
{
	move="/usr/lib/sendmail"
	if [ -L /usr/sbin ] ; then
		link=$(readlink /usr/sbin)
		if [ ! " $link" = " bin" -a ! " $link" = " /usr/bin" ] ; then
			move="$move /usr/sbin/sendmail"
		fi
	else
		move="$move /usr/sbin/sendmail"
	fi
	if [ -x /usr/sbin/alternatives -o -x /usr/sbin/update-alternatives -o -x /usr/bin/update-alternatives ] ; then
		for i in $move; do
			if [ -f $i -a ! -L $i ]; then
				echo "! $i is a file, should be a link. Fixing..." 1>&2
				echo /bin/mv $i $i.old
				/bin/mv $i $i.old
			fi
		done
		if [ -f /etc/pam.d/smtp -a ! -L /etc/pam.d/smtp ] ; then
			echo "! /etc/pam.d/smtp is a file, should be a link. Fixing..." 1>&2
			echo /bin/mv /etc/pam.d/smtp /etc/pam.d/smtp.old
			/bin/mv /etc/pam.d/smtp /etc/pam.d/smtp.old
		fi
		if [ -x /usr/sbin/alternatives ] ; then
			alternatives_cmd=/usr/sbin/alternatives
		elif [ -x /usr/sbin/update-alternatives ] ; then
			alternatives_cmd=/usr/sbin/update-alternatives
		else
			alternatives_cmd=/usr/bin/update-alternatives
		fi
		if [ -f /etc/os-release ] ; then
			openSUSE=$(grep openSUSE /etc/os-release 2>/dev/null)
		else
			openSUSE=""
		fi
		if [ -z "$openSUSE" -a ! -f /etc/SuSE-release -a ! -f /etc/debian_version ] ; then
			init_script="--initscript svscan"
		else
			init_script=""
		fi
		cmd="$alternatives_cmd"
		cmd="$cmd --install /usr/sbin/sendmail  mta            $QmailBinPrefix/bin/sendmail 120"
		cmd="$cmd --slave   /usr/bin/mailq      mta-mailq      $QmailBinPrefix/bin/qmail-qread"
		cmd="$cmd --slave   /usr/bin/rmail      mta-rmail      $QmailBinPrefix/bin/irmail"
		cmd="$cmd --slave   /usr/bin/newaliases mta-newaliases $QmailBinPrefix/bin/inewaliases"
		cmd="$cmd --slave   /usr/lib/sendmail   mta-sendmail   $QmailBinPrefix/bin/sendmail"
		if [ -f $mandir/man1/inewaliases.1.gz ] ; then
			cmd="$cmd --slave $mandir/man1/newaliases.1.gz mta-newaliasesman $mandir/man1/inewaliases.1.gz"
		fi
		if [ -f $mandir/man8/qmail-qread.8.gz ] ; then
			cmd="$cmd --slave $mandir/man1/mailq.1.gz mta-mailqman $mandir/man8/qmail-qread.8.gz"
		fi
		if [ -f $mandir/man8/isendmail.8.gz ] ; then
			cmd="$cmd --slave $mandir/man8/sendmail.8.gz mta-sendmailman $mandir/man8/isendmail.8.gz"
		fi
		if [ -f $mandir/man8/irmail.8.gz ] ; then
			cmd="$cmd --slave $mandir/man8/rmail.8.gz mta-rmailman $mandir/man8/irmail.8.gz"
		fi
		if [ -f /etc/pam.d/pam-multi ] ; then
			cmd="$cmd --slave /etc/pam.d/smtp mta-pam /etc/pam.d/pam-multi"
		fi
		if [ -n "$init_script" ] ; then
			cmd="$cmd $init_script"
		fi
		echo $cmd
		eval $cmd
	elif [ -f /usr/sbin/mailwrapper ] ; then
		if [ ! -f /etc/mail/mailer.conf.indimail-mta ] ; then
			echo /bin/mv /etc/mail/mailer.conf /etc/mail/mailer.conf.indimail-mta
			/bin/mv /etc/mail/mailer.conf /etc/mail/mailer.conf.indimail-mta
		fi
		echo "creating /etc/mail/mailer.conf"
		(
		echo "#"
		echo "# Execute the "real" sendmail program, named /usr/local/bin/sendmail"
		echo "#"
		echo "sendmail   $QmailBinPrefix/bin/sendmail"
		echo "mailq      $QmailBinPrefix/bin/qmail-qread"
		echo "newaliases $QmailBinPrefix/bin/inewaliases"
		echo "hoststat   $QmailBinPrefix/sbin/qmail-tcpto"
		echo "purgestat  $QmailBinPrefix/sbin/qmail-tcpok"
		) > /etc/mail/mailer.conf
	else
		for i in $move; do
			if [ -f $i -a ! -L $i ]; then
				echo "! $i is a file, should be a link. Fixing..."
				echo /bin/mv $i $i.old
				echo /bin/ln -s $QmailBinPrefix/bin/sendmail $i
				/bin/mv $i $i.old
				/bin/ln -s $QmailBinPrefix/bin/sendmail $i
			elif [ -L $i ];then
				link=$(readlink $i)
				if [ x"$link" != x"$QmailBinPrefix/bin/sendmail" ] ; then
					echo /bin/mv $i $i.old
					echo /bin/ln -s $QmailBinPrefix/bin/sendmail $i
					/bin/mv $i $i.old
					/bin/ln -s $QmailBinPrefix/bin/sendmail $i
				fi
			elif [ ! -f $i ];then
				echo "! $i is missing. Fixing..."
				echo /bin/ln -s $QmailBinPrefix/bin/sendmail $i
				/bin/ln -s $QmailBinPrefix/bin/sendmail $i
			fi
		done
		if [ -f /etc/pam.d/smtp -a ! -L /etc/pam.d/smtp ] ; then
			echo "! /etc/pam.d/smtp is a file, should be a link. Fixing..."
			echo /bin/mv /etc/pam.d/smtp /etc/pam.d/smtp.old
			echo /bin/ln -s /etc/pam.d/pam.multi /etc/pam.d/smtp
			/bin/mv /etc/pam.d/smtp /etc/pam.d/smtp.old
			/bin/ln -s /etc/pam.d/pam.multi /etc/pam.d/smtp
		fi
	fi
}

remove_alternatives()
{
	if [ -x /usr/sbin/alternatives ] ; then
		echo /usr/sbin/alternatives --remove mta $QmailBinPrefix/bin/sendmail
		echo /usr/sbin/alternatives --auto mta
		/usr/sbin/alternatives --remove mta $QmailBinPrefix/bin/sendmail
		/usr/sbin/alternatives --auto mta
	elif [ -x /usr/sbin/update-alternatives ] ; then
		echo /usr/sbin/update-alternatives --remove mta $QmailBinPrefix/bin/sendmail
		echo /usr/sbin/update-alternatives --auto mta
		/usr/sbin/update-alternatives --remove mta $QmailBinPrefix/bin/sendmail
		/usr/sbin/update-alternatives --auto mta
	elif [ -x /usr/sbin/mailwrapper ] ; then
		if [  -f /etc/mail/mailer.conf.indimail-mta ] ; then
			echo /bin/mv /etc/mail/mailer.conf.indimail-mta /etc/mail/mailer.conf
			/bin/mv /etc/mail/mailer.conf.indimail-mta /etc/mail/mailer.conf
		else
			echo "unable to restore original mailer.conf" 1>&2
		fi
	else
		for i in /usr/lib/sendmail /usr/sbin/sendmail /etc/pam.d/smtp; do
			if [ -f $i.old -o -L $i.old ]; then
				echo "restoring $i"
				echo /bin/rm -f $i
				echo /bin/mv $i.old $i
				/bin/rm -f $i
				/bin/mv $i.old $i
			fi
		done
	fi
}

selinux_module()
{
	config_name=$1
	policy_file=$2
	# selinux
	status=0
	if [ ! -x /usr/sbin/selinuxenabled ] ; then
		exit 1
	fi
	/usr/sbin/selinuxenabled
	if [ $? -ne 0 ] ; then
		exit 1
	fi
	if [ -x /usr/bin/checkmodule -a -x /usr/bin/semodule_package -a -f "$policy_file".te ] ; then
		echo "Creating selinux module `date`"
		/usr/bin/checkmodule -M -m -o "$policy_file".mod   "$policy_file".te
		if [ $status -eq 0 ] ; then
			status=$?
		fi
		if [ -f $policy_file.fc ] ; then
			echo "/usr/bin/semodule_package -o "$policy_file".pp -m "$policy_file".mod -f "$policy_file".fc `date`"
			/usr/bin/semodule_package -o "$policy_file".pp -m "$policy_file".mod -f "$policy_file".fc
		else
			echo "/usr/bin/semodule_package -o "$policy_file".pp -m "$policy_file".mod `date`"
			/usr/bin/semodule_package -o "$policy_file".pp -m "$policy_file".mod
		fi
		if [ $status -eq 0 ] ; then
			status=$?
		fi
		if [ -x /usr/sbin/semodule -a $status -eq 0 ] ; then
			echo "enabling selinux module "$policy_file".pp `date`"
			/usr/sbin/semodule -i "$policy_file".pp
			if [ $status -eq 0 ] ; then
				status=$?
			fi
		fi
	fi
	if [ " $config_name" = " selinux" ] ; then
		exit $status
	fi

	if [ " $config_name" = " qselinux" ] ; then
		echo "selinux settings for indimail-mta `date`"
		if [ -x /usr/sbin/setsebool ] ; then
			echo "setting clamd_use_jit `date`"
			/usr/sbin/setsebool -P clamd_use_jit on
			if [ $status -eq 0 ] ; then
				status=$?
			fi
			echo "setting antivirus_can_scan_system `date`"
			/usr/sbin/setsebool -P antivirus_can_scan_system on
			if [ $status -eq 0 ] ; then
				status=$?
			fi
		fi
		if [ -x /usr/bin/chcon ] ; then
			echo "changing security context (antivirus_db_t) for $indimaildir/clamd `date`"
			/usr/bin/chcon -R -u system_u -r object_r -t antivirus_db_t  $indimaildir/clamd
			if [ $status -eq 0 ] ; then
				status=$?
			fi
		fi
		if [ " $QmailBinPrefix" = " $indimaildir" ] ; then
			for i in sendmail envdir qmail-inject
			do
				if [ -x /usr/bin/chcon ] ; then
					echo "changing security context for $QmailBinPrefix/bin/$i `date`"
					/usr/bin/chcon -t bin_t $QmailBinPrefix/bin/$i
					if [ $status -eq 0 ] ; then
						status=$?
					fi
				fi
				if [ -x /usr/sbin/semanage ] ; then
					echo "Manage file context mapping (bin_t) definitions for $QmailBinPrefix/bin/$i `date`"
					/usr/sbin/semanage fcontext -a -t bin_t $QmailBinPrefix/bin/$i
					if [ $status -eq 0 ] ; then
						status=$?
					fi
				fi
				if [ -x /usr/sbin/restorecon ] ; then
					echo "/usr/sbin/restorecon -Rv $QmailBinPrefix/$i `date`"
					/usr/sbin/restorecon -Rv $QmailBinPrefix/$i
					if [ $status -eq 0 ] ; then
						status=$?
					fi
				fi
			done
		fi
	fi

	if [ " $config_name" = " iselinux" ] ; then
		echo "selinux settings for indimail `date`"
		if [ -x /usr/sbin/setsebool ] ; then
			echo "setsebool -P httpd_can_network_connect 1 `date`"
			setsebool -P httpd_can_network_connect 1
		fi
		if [ -x /usr/bin/chcon ] ; then
			echo "changing security context (mysqld_db_t) for $indimaildir/mysqldb/data `date`"
			/usr/bin/chcon -R -u system_u -r object_r -t mysqld_db_t  $indimaildir/mysqldb/data
			if [ $status -eq 0 ] ; then
				status=$?
			fi
			echo "changing security context (mysqld_log_t) for $indimaildir/mysqldb/logs `date`"
			/usr/bin/chcon -R -u system_u -r object_r -t mysqld_log_t $indimaildir/mysqldb/logs
			if [ $status -eq 0 ] ; then
				status=$?
			fi
			echo "changing security context (mysqld_etc_t) for $sysconfdir/indimail.cnf `date`"
			/usr/bin/chcon -R -u system_u -r object_r -t mysqld_etc_t $sysconfdir/indimail.cnf
			if [ $status -eq 0 ] ; then
				status=$?
			fi
			if [ -f /tmp/mysql.sock ] ; then
				echo "changing security context (mysqld_var_run_t) for /tmp/mysql.sock `date`"
				/usr/bin/chcon -R -u system_u -r object_r -t mysqld_var_run_t /tmp/mysql.sock
				if [ $status -eq 0 ] ; then
					status=$?
				fi
			fi
		fi
		if [ -x /usr/sbin/restorecon ] ; then
			for i in $indimaildir/mysqldb/data $indimaildir/mysqldb/logs $sysconfdir/indimail.cnf \
				$indimaildir/clamd $sysconfdir $servicedir
			do
				echo "/usr/sbin/restorecon -Rv $i `date`"
				/usr/sbin/restorecon -Rv $i
				if [ $status -eq 0 ] ; then
					status=$?
				fi
			done
		fi
	fi
	if [ -x /usr/bin/chcon ] ; then
		echo "changing security context (etc_t) for $sysconfdir `date`"
		/usr/bin/chcon -R -u system_u -r object_r -t etc_t $sysconfdir
		if [ $status -eq 0 ] ; then
			status=$?
		fi
		echo "changing security context (svc_svc_t) for $servicedir `date`"
		/usr/bin/chcon -R -u system_u -r object_r -t svc_svc_t $servicedir
		if [ $status -eq 0 ] ; then
			status=$?
		fi
	fi
	exit $status
}

create_clamd_conf()
{
echo "# Also log clean files. Useful in debugging but drastically increases the"
echo "# log size."
echo "# Default: no"
echo "LogClean yes"
echo ""
echo "# Log additional information about the infected file, such as its"
echo "# size and hash, together with the virus name."
echo "ExtendedDetectionInfo yes"
echo ""
echo "# Path to the database directory."
echo "# Default: hardcoded (depends on installation options)"
echo "DatabaseDirectory /var/indimail/clamd"
echo ""
echo "# Path to a local socket file the daemon will listen on."
echo "# Default: disabled (must be specified by a user)"
echo "LocalSocket /var/run/clamd.scan/clamd.sock"
echo ""
echo "# Sets the group ownership on the unix socket."
echo "# Default: disabled (the primary group of the user running clamd)"
echo "LocalSocketGroup qmail"
echo ""
echo "# Sets the permissions on the unix socket to the specified mode."
echo "# Default: disabled (socket is world accessible)"
echo "LocalSocketMode 660"
echo ""
echo "# Remove stale socket after unclean shutdown."
echo "# Default: yes"
echo "FixStaleSocket yes"
echo ""
echo "# Enable non-blocking (multi-threaded/concurrent) database reloads."
echo "# This feature will temporarily load a second scanning engine while scanning"
echo "# continues using the first engine. Once loaded, the new engine takes over."
echo "# The old engine is removed as soon as all scans using the old engine have"
echo "# completed."
echo "# This feature requires more RAM, so this option is provided in case users are"
echo "# willing to block scans during reload in exchange for lower RAM requirements."
echo "# Default: yes"
echo "ConcurrentDatabaseReload no"
echo ""
echo "# Run as another user (clamd must be started by root for this option to work)"
echo "# Default: don't drop privileges"
echo "User qscand"
echo ""
echo "# Stop daemon when libclamav reports out of memory condition."
echo "ExitOnOOM yes"
echo ""
echo "# Don't fork into background."
echo "# Default: no"
echo "Foreground yes"
}

create_freshclam_conf()
{
echo "# Path to the database directory."
echo "# WARNING: It must match clamd.conf's directive!"
echo "# Default: hardcoded (depends on installation options)"
echo "DatabaseDirectory /var/indimail/clamd"
echo ""
echo ""
echo "# By default when started freshclam drops privileges and switches to the"
echo "# \"clamav\" user. This directive allows you to change the database owner."
echo "# Default: clamav (may depend on installation options)"
echo "DatabaseOwner qscand"
echo ""
echo "# database.clamav.net is now the primary domain name to be used world-wide."
echo "# Now that CloudFlare is being used as our Content Delivery Network (CDN), "
echo "# this one domain name works world-wide to direct freshclam to the closest "
echo "# geographic endpoint."
echo "DatabaseMirror database.clamav.net"
echo ""
echo "# How many attempts to make before giving up."
echo "# Default: 3 (per mirror)"
echo "MaxAttempts 5"
echo ""
echo "# Don't fork into background."
echo "# Default: no"
echo "Foreground yes"
}

create_bogofilter_conf()
{
if [ ! -f "$DESTDIR"$sysconfdir/bogofilter.cf.example ] ; then
	echo ""$DESTDIR"$sysconfdir/bogofilter.cf.example: No such file or directory" 1>&2
	return 1
fi
@prefix@/bin/qcat "$DESTDIR"$sysconfdir/bogofilter.cf.example | sed \
	-e 's,##bogofilter_dir=,bogofilter_dir=,g' \
	-e 's,#spam_header_name=,spam_header_name=,g' \
	-e 's,#spam_header_place=,spam_header_place=,g' \
	-e 's,##spamicity_tags = Yes\, No\, Unsure,spamicity_tags = Yes\, No\, Unsure,g' \
	-e 's,##spamicity_formats = %0.6f\, %0.6f\, %0.6f,spamicity_formats = %0.6f\, %0.6f\, %0.6f,g' \
	-e 's,#header_format = %h: %c\, tests=bogofilter\, spamicity=%p\, version=%v,header_format = %h: %c\, spamicity=%p\, cutoff=%o\, ham_cutoff=%O\, version=%v,g' \
	-e 's,#log_update_format = register-%r\, %w words\, %m messages,log_update_format = register-%r %w words %m msg,g' \
	-e 's,##log_header_format = %h: %c\, spamicity=%f\, ipaddr=%A\, queueID=%Q\, msgID=%I\, subject=%s\, version=%v,log_header_format = %h: %c\, spamicity=%p\, cutoff=%o\, ham_cutoff=%O\, queueID=%Q\, msgID=%I\, ipaddr=%A,g' \
	-e 's,##ham_cutoff  = 0.00,ham_cutoff  = 0.00,g' \
	-e 's,##spam_cutoff = 0.99,spam_cutoff = 0.99,g'
}

repair_tables()
{
	MYSQL_DB="indimail"
	if [ -x $mysqlPrefix/bin/mariadb ] ; then
		mysql=$mysqlPrefix/bin/mariadb
	else
		mysql=$mysqlPrefix/bin/mysql
	fi
	if [ -f $HOME/.mylogin.cnf ] ; then
	(
	$mysql --login-path=admin -s --database=$MYSQL_DB \
		--execute="show tables" | while read i
	do
		$mysql --login-path=admin -s --database=$MYSQL_DB \
			--execute="check table $i"
	done
	) | awk -v mysql=$mysql \
		-v database=$MYSQL_DB '{
		table=$1
		operation=$2
		msg_type=$3
		msg_text=$4
		if (FNR == 1)
			printf("%s %-30s %-9s %-8s %s\n", "S/N", "Table", "Operation", "msg_type", "msg_text");
		printf("%03d %-30s %-9s %-8s %s\n", FNR, table, operation, msg_type, msg_text);
		if (msg_text != "OK")
		{
			cmd=sprintf("%s --login-path=admin -s --database=%s \
				--execute=\"repair table %s;\"", \
				mysql, database, table);
			system(cmd);
		}
	}'
	else
	(
	$mysql -s -u $PRIV_USER -p$PRIV_PASS --database=$MYSQL_DB \
		--execute="show tables" | while read i
	do
		$mysql -s -u $PRIV_USER -p$PRIV_PASS --database=$MYSQL_DB \
			--execute="check table $i"
	done
	) | awk -v mysql=$mysql -v mysql_user=$PRIV_USER -v mysql_pass=$PRIV_PASS \
		-v database=$MYSQL_DB '{
		table=$1
		operation=$2
		msg_type=$3
		msg_text=$4
		if (FNR == 1)
			printf("%s %-30s %-9s %-8s %s\n", "S/N", "Table", "Operation", "msg_type", "msg_text");
		printf("%03d %-30s %-9s %-8s %s\n", FNR, table, operation, msg_type, msg_text);
		if (msg_text != "OK")
		{
			cmd=sprintf("%s -s -u %s -p%s --database=%s \
				--execute=\"repair table %s;\"", \
				mysql, mysql_user, mysql_pass, database, table);
			system(cmd);
		}
	}'
	fi
}

report ()
{
	for i in $*
	do
		case $i in
			all)
				report_option="all"
			;;
			zoverall|zddist|zsuccesses|zdeferrals|zfailures|zsenders|zrecipients| zrhosts| \
				zrxdelay| zsendmail|zsuids|zsmtp|zspam|all)
			;;
			*)
				echo "Invalid option $i" 1>&2
				usage 1
			;;
		esac
	done
	if [ ! -d $LOGDIR ] ; then
		/bin/mkdir -p $LOGDIR
		$chown qmaill:nofiles $LOGDIR
	fi
	if [ ! -d $LOGDIR/reports ] ; then
		/bin/mkdir -p $LOGDIR/reports
		$chown qmaill:nofiles $LOGDIR/reports
	fi
	(
	if [ -f $LOGDIR/reports/send_pend ] ; then
		@prefix@/bin/qcat $LOGDIR/reports/send_pend
		/bin/rm -f $LOGDIR/reports/send_pend
	fi
	@prefix@/bin/qcat $LOGDIR/deliver.25/current
	) | "$DESTDIR"$libexecdir/matchup 5>$LOGDIR/reports/pend_tmp > $LOGDIR/reports/deliver25
	if [ -s $LOGDIR/reports/pend_tmp ] ; then
		/bin/mv $LOGDIR/reports/pend_tmp $LOGDIR/reports/send_pend
	else
		/bin/rm -f $LOGDIR/reports/pend_tmp
	fi

	(
	if [ -f $LOGDIR/reports/smtp25_pend ] ; then
		@prefix@/bin/qcat $LOGDIR/reports/smtp25_pend
		/bin/rm -f $LOGDIR/reports/smtp25_pend
	fi
	@prefix@/bin/qcat $LOGDIR/smtpd.25/current
	) | "$DESTDIR"$libexecdir/smtp-matchup 5>$LOGDIR/reports/pend_tmp | \
	"$DESTDIR"$QmailBinPrefix/bin/tai64nunix > $LOGDIR/reports/smtp25
	if [ -s $LOGDIR/reports/pend_tmp ] ; then
		/bin/mv $LOGDIR/reports/pend_tmp $LOGDIR/reports/smtp25_pend
	else
		/bin/rm -f $LOGDIR/reports/pend_tmp
	fi
	(
	if [ -f $LOGDIR/reports/smtp587_pend ] ; then
		@prefix@/bin/qcat $LOGDIR/reports/smtp587_pend
		/bin/rm -f $LOGDIR/reports/smtp587_pend
	fi
	@prefix@/bin/qcat $LOGDIR/smtpd.587/current
	) | "$DESTDIR"$libexecdir/smtp-matchup 5>$LOGDIR/reports/pend_tmp | \
	"$DESTDIR"$QmailBinPrefix/bin/tai64nunix > $LOGDIR/reports/smtp587
	if [ -s $LOGDIR/reports/pend_tmp ] ; then
		/bin/mv $LOGDIR/reports/pend_tmp $LOGDIR/reports/smtp587_pend
	else
		/bin/rm -f $LOGDIR/reports/pend_tmp
	fi

	spamheader_name=`grep ^spam_header_name $sysconfdir/bogofilter.cf | cut -d= -f2`
	if [ " $spamheader_name" = " " ] ; then
		> $LOGDIR/reports/spam
	else
		> $LOGDIR/reports/spam
		@prefix@/bin/qcat $LOGDIR/logfifo/current | grep $spamheader_name| "$DESTDIR"$QmailBinPrefix/bin/tai64nunix \
		> $LOGDIR/reports/spam
	fi

	if [ -s $LOGDIR/reports/deliver25 -o -s $LOGDIR/reports/smtp25 -o -s $LOGDIR/reports/smtp587 ] ; then
		EXT=`date +'%d/%m/%y %H:%M:%S'`
		(
		if [ " $report_option" = " all" ] ; then
			for i in zoverall zddist zrhosts zrxdelay \
				zfailures zdeferrals zsenders \
				zrecipients zsuids zsmtp zspam
			do
				echo "-- Report $i ----------------"
				if [ " $i" = " zsmtp" ] ; then
					if [ -s $LOGDIR/reports/smtp25 ] ; then
						echo "---- Port 25 -----------------"
						"$DESTDIR"$libexecdir/$i < $LOGDIR/reports/smtp25
					fi
					if [ -s $LOGDIR/reports/smtp587 ] ; then
						echo "---- Port 587 ----------------"
						"$DESTDIR"$libexecdir/$i < $LOGDIR/reports/smtp587
					fi
				elif [ " $i" = " zspam" ] ; then
					if [ -s $LOGDIR/reports/spam ] ; then
						"$DESTDIR"$libexecdir/$i < $LOGDIR/reports/spam
					fi
				else
					if [ -s $LOGDIR/reports/deliver25 ] ; then
						"$DESTDIR"$libexecdir/$i < $LOGDIR/reports/deliver25
					fi
				fi
				echo
			done
		else
			for i in $*
			do
				echo "-- Report $i ----------------"
				if [ " $i" = " zsmtp" ] ; then
					if [ -s $LOGDIR/reports/smtp25 ] ; then
						echo "---- Port 25 -----------------"
						"$DESTDIR"$libexecdir/$i < $LOGDIR/reports/smtp25
					fi
					if [ -s $LOGDIR/reports/smtp587 ] ; then
						echo "---- Port 587 ----------------"
						"$DESTDIR"$libexecdir/$i < $LOGDIR/reports/smtp587
					fi
				elif [ " $i" = " zspam" ] ; then
					if [ -s $LOGDIR/reports/spam ] ; then
						"$DESTDIR"$libexecdir/$i < $LOGDIR/reports/spam
					fi
				else
					if [ -s $LOGDIR/reports/deliver25 ] ; then
						"$DESTDIR"$libexecdir/$i < $LOGDIR/reports/deliver25
					fi
				fi
				echo
			done
		fi
		) > $TMPDIR/report.$$
		if [ " $postmaster" = " " ] ; then
			@prefix@/bin/qcat $TMPDIR/report.$$
			/bin/rm -f $TMPDIR/report.$$
		else
			subject="Mail Statistics for `/usr/bin/head -1 $CONTROLDIR/me` on $EXT"
			date=`/bin/date -R`
			if [ " $attach_report" = " " ] ; then
				(
				echo "To: $postmaster"
				echo "From: \"IndiMail Reporter\" <Mailer-Daemon>"
				echo "Date: $date"
				echo "Subject: $subject"
				echo
				echo ATTENTION: Mail Administrators
				echo
				echo Mail Statistics for $(@prefix@/bin/qcat $CONTROLDIR/me)
				echo on $date
				echo from host `"$DESTDIR"$libexecdir/hostname`
				echo
				echo "Since I am good only at reporting, please do not reply"
				echo "to me."
				echo
				echo "Thank you for using IndiMail"
				echo
				echo "I am enclosing a copy of the Statistics below"
				echo
				@prefix@/bin/qcat $TMPDIR/report.$$
				/bin/rm -f $TMPDIR/report.$$
				) | "$DESTDIR"$QmailBinPrefix/bin/sendmail -t
			else
				(
				echo ATTENTION: Mail Administrators
				echo
				echo Mail Statistics for `/usr/bin/head -1 $CONTROLDIR/me`
				echo on $date
				echo from host `"$DESTDIR"$libexecdir/hostname`
				echo
				echo "I have attached a copy of the Statistics in zip file format."
				echo "Since I am good only at reporting, please do not reply"
				echo "to me."
				echo
				echo "Thank you for using IndiMail"
				echo
				) > $TMPDIR/descfile.$$
				/usr/bin/zip -q -j -m $TMPDIR/report.$$.zip $TMPDIR/report.$$
				QMAILUSER=postmaster
				if [ -z "$default_domain" ] ; then
					if [ -f $sysconfdir/control/global_vars/DEFAULT_DOMAIN ] ; then
						QMAILHOST=$(qcat $sysconfdir/control/global_vars/DEFAULT_DOMAIN)
					else
						QMAILHOST=$([ -n "$HOSTNAME" ] && echo "$HOSTNAME" || uname -n)
					fi
				fi
				QMAILNAME="IndiMail Reporter"
				export QMAILNAME
				"$DESTDIR"$QmailBinPrefix/bin/mpack -s "$subject" -d $TMPDIR/descfile.$$ \
				-c "application/zip" $TMPDIR/report.$$.zip $postmaster
				/bin/rm -f $TMPDIR/descfile.$$ $TMPDIR/report.$$.zip
			fi
		fi
		/bin/rm -f $LOGDIR/reports/deliver25 $LOGDIR/reports/smtp25 \
			$LOGDIR/reports/smtp587 $LOGDIR/reports/spam
	fi
}

dump_config()
{
if [ $# -ne 2 ] ; then
	echo "USAGE: dump_config servicedir controldir" 1>&2
	return 1
fi
servicedir=$1
CONTROLDIR=$2
export CONTROLDIR
if [ ! -d $servicedir ] ; then
	echo "servicedir: No such file or directory" 1>&2
	return 1
fi
echo "Service Configuration ================================================"
echo Service Directory $servicedir
cd $servicedir
scount=1
for j in `/bin/ls -a $servicedir`
do
	if [ " $j" = " ." -o " $j" = " .." ] ; then
		continue
	fi
	printf "Service %05d %s\n" $scount $j
	scount=`expr $scount + 1`
	if [ -f $j/run ] ; then
		echo "Run File [$j/run]"
		@prefix@/bin/qcat $j/run
		/bin/echo -n "- EOF -----------------"
		printf "%-20s" "$j/run"
		echo "-----------------------"
		echo
	fi
	if [ -f $j/log/run ] ; then
		echo "Log Run File [$j/log/run]"
		@prefix@/bin/qcat $j/log/run
		/bin/echo -n "- EOF -----------------"
		printf "%-20s" "$j/log/run"
		echo "-----------------------"
		echo
	fi
	if [ -f $j/shutdown ] ; then
		echo "Shutdown File [$j/shutdown]"
		@prefix@/bin/qcat $j/shutdown
		/bin/echo -n "- EOF -----------------"
		printf "%-20s" "$j/shutdown"
		echo "-----------------------"
		echo
	fi
	if [ -d $j/variables ] ; then
		count=1
		for k in `ls $j/variables`
		do
			if [ $count -eq 1 ] ; then
				echo "Variables List"
			fi
			if [ -s $j/variables/$k ] ; then
				printf "%-3d %s\nValue:[" $count $k
				val=$(@prefix@/bin/qcat $j/variables/$k)
				if [ " $val" = " " ] ; then
					/bin/echo -n "SET"
				else
					/bin/echo -n $val
				fi
				printf "]\n"
			else
				printf "%-3d %s\nValue:[UNSET]\n" $count $k
			fi
			count=`expr $count + 1`
			echo
		done
	fi
	echo
done
echo
echo "Service variables ====================================================="
print_all_variables

echo
echo "Control Configuration ================================================="
echo
"$DESTDIR"$QmailBinPrefix/bin/qmail-showctl

echo
echo "IndiMail Assign File:"
@prefix@/bin/qcat "$DESTDIR"$sysconfdir/users/assign
echo
}

# backup /service, system-startup config, indimail configuration
sort_backup()
{
if [ $# -lt 1 ] ; then
	echo "sort_backup service_dir" 1>&2
	return 1
fi
if [ -f /etc/event.d/svscan ] ; then
	file=/etc/event.d/svscan
elif [ -f /etc/init/svscan.conf ] ; then
	file=/etc/init/svscan.conf
elif [ -f /lib/systemd/system/svscan.service ] ; then
	file=/lib/systemd/system/svscan.service
	if [ -d "$file".d ] ; then
		file="$file $file.d"
	fi
elif [ -f /usr/lib/systemd/system/svscan.service ] ; then
	file=/usr/lib/systemd/system/svscan.service
	if [ -d "$file".d ] ; then
		file="$file $file.d"
	fi
fi
# who cuts the barber
if [ -f $sysconfdir/backup.conf ] ; then
	file="$file $sysconfdir/backup.conf $(@prefix@/bin/qcat $sysconfdir/backup.conf)"
fi
servicedir=$1
(
for i in $servicedir $sysconfdir $QmailHOME/domains $QmailHOME/autoturn \
	/etc/inittab /etc/init.d/svscan /etc/passwd /etc/group /etc/hosts \
	/etc/sysconfig/network /etc/nsswitch.conf /etc/resolv.conf \
	/etc/security /etc/alternativces  $QmailHOME/.bashrc \
	$QmailHOME/.bash_profile $QmailHOME/.indimail /root/indimail_config.txt \
	$file
do
	if [ -f $i -o -d $i ] ; then
		echo "$i"
	fi
done
) | grep -v '^#' | sort -u
}

tls_cert_check()
{
if [ ! -x /usr/bin/openssl ] ; then
	echo "/usr/bin/openssl: No such file or directory" 1>&2
	return 1
fi
error=0
if [ $# -eq 1 ] ; then
	files=$1
else
	files="$sysconfdir/certs/servercert.pem $sysconfdir/certs/clientcert.pem"
fi
for cert in $files
do
	if [ -f $cert ]; then
		echo "Checking $cert"
		#First, check that it's a valid cert for the task
		TEMP_PURPOSE=`/usr/bin/openssl x509 -in $cert -noout -purpose 2>/dev/null`
		if [ "$?" != "0" ]; then
			error=1
			dir=`dirname $cert`
			fil=`basename $cert`
			$mv -f $cert $dir/BROKEN-${fil}
			if [ $? -eq 0 ] ; then
				echo $TEMP_PURPOSE 1>&2
				echo "$cert is a broken cert. Disabled" 1>&2
			fi
		fi

		#Now check it hasn't expired
		TEMP_DATE=`/usr/bin/openssl x509 -in $cert -noout -dates 2>/dev/null|grep -i after|cut -d= -f2`
		case "$host" in
			*-*-darwin*|*-*-freebsd*)
			EXPIRE_IN_SECS=`date -j -f "%b %d %T %Y %Z" "$TEMP_DATE" "+%s"`
			;;
			*)
			EXPIRE_IN_SECS=`date +%s --date $TEMP_DATE 2>/dev/null`
			;;
		esac
		if [ "`echo $EXPIRE_IN_SECS|grep -E '^[0-9]+$'`" != "" ]; then
			NOW_IN_SECS=`date +%s 2>/dev/null`
			if [ "`echo $NOW_IN_SECS|grep -E '^[0-9]+$'`" != "" ]; then
				if [ $NOW_IN_SECS -gt $EXPIRE_IN_SECS ]; then
					error=1
					echo expiry $TEMP_DATE 1>&2
					dir=`dirname $cert`
					fil=`basename $cert`
					$mv -f $cert $dir/EXPIRED-${fil}
					if [ $? -eq 0 ] ; then
						echo "$cert has EXPIRED. Disabling" 1>&2
					fi
				fi
			fi
		fi

		if [ "`echo $cert|grep server`" != "" ];then
			if [ "`echo $TEMP_PURPOSE|grep -Ei '(any purpose|server).* yes'`" = "" ]; then
				error=1
				echo "Purpose $TEMP_PURPOSE" 1>&2
				dir=`dirname $cert`
				fil=`basename $cert`
				$mv -f $cert $dir/NOT-A-SERVER-CERT-${fil}
				if [ $? -eq 0 ] ; then
					echo "$cert is NOT a server cert. Disabled" 1>&2
				fi
			fi
		fi
		if [ "`echo $cert|grep client`" != "" ];then
			if [ "`echo $TEMP_PURPOSE|grep -Ei '(any purpose|client).* yes'`" = "" ]; then
				error=1
				echo "Purpose $TEMP_PURPOSE" 1>&2
				dir=`dirname $cert`
				fil=`basename $cert`
				$mv -f $cert $dir/NOT-A-CLIENT-CERT-${fil}
				if [ $? -eq 0 ] ; then
					echo "$cert is NOT a client cert. Disabled" 1>&2
				fi
			fi
		fi
	fi
done
if [ $error -eq 0 ] ; then
	echo "All Certificates found OK. Press ENTER to view Certificates"
	read key
	(
	for cert in $files; do
		echo "==== Certificate $cert ========="
		if [ -L $cert ] ; then
			echo "Linked to $(readlink $cert)"
			ls -ld $cert
		else
	 		openssl x509 -in $cert -text
		fi
	 	echo ================================================================
		echo
	done
	) | $MORE
fi
}

import_variables()
{
if [ $# -ne 6 ] ; then
	echo "usage: import_variables dir envfile restore_flag owner group mode" 1>&2
	exit 1
fi
dir=$1
env_file=$2
restore_flag=$3
owner=$4
group=$5
mode=$6
if [ $silent -eq 0 ] ; then
	if [ $restore_flag -eq 1 ] ; then
		printf "Restoring %-40s from %s\n" $dir $env_file
		old=$silent
		silent=1
	else
		printf "importing %-40s from %s\n" $dir $env_file
	fi
fi
# remove comment, leading space and blank lines
sed -e 's{^[ \t]*#.*{{g ; s{^[ \t]*{{g ; s{#.*{{ ;  /^$/d' $env_file | while read line
do
	if [ -z "$line" ] ; then
		continue
	fi
	file=`echo $line | cut -d= -f1`
	value=`echo $line | cut -d= -f2- | tr -d \"`
	# when importing variables
	if [ $restore_flag -eq 0 -a $force -eq 0 -a -f $dir/$file ] ; then
		if [ $silent -eq 0 ] ; then
			echo "importing variables without --force. not replacing variable $file" 1>&2
		fi
		continue
	fi
	if [ -z "$value" ] ; then
		value=`echo $line | cut -d= -f2-`
		if [ -z "$value" ] ; then
			# variable=
			# unset/remove variable
			if [ -s $dir/$file -o ! -f $dir/$file ] ; then
				if [ $silent -eq 0 ] ; then
					echo "unset env variable $file"
				fi
				> $dir/$file
			fi
		else
			# variable=""
			# set variable with empty value
			value=`echo $value | tr -d \"`
			check_update_if_diff $dir/$file "$value"
			if [ $? -ne 0 -a $silent -eq 0 ] ; then
				echo "set env variable $file=[$value]"
			fi
		fi
	else
		# variable="some value"
		# set variable=$value
		check_update_if_diff $dir/$file "$value"
		if [ $? -ne 0 -a $silent -eq 0 ] ; then
			echo "set env variable $file=[$value]"
		fi
	fi
	if [ -n "$mode" ] ; then
		/bin/chmod $mode $dir/$file
	fi
	if [ -n "$owner" -a -n "$group" ] ; then
		$chown $owner:$group $dir/$file
	elif [ -n "$owner" ] ; then
		$chown $owner $dir/$file
	fi
done
if [ $restore_flag -eq 1 ] ; then
	silent=$old
fi

# remove variables not originally present
# This will restore variables to original state
if [ $restore_flag -eq 1 ] ; then
	out=`mktemp -dt svcXXXXXXXXXXXX`
	if [ $? -ne 0 ] ; then
		echo "unable to create temp dir" 1>&2
		return 1
	fi
	@prefix@/bin/qcat $env_file | while read line
	do
		file=`echo $line | cut -d= -f1`
		$touch $out/$file
	done
	for j in `/bin/ls $dir`
	do
		if [ ! -f $out/$j ] ; then
			/bin/rm -f $dir/$j
		fi
	done
	/bin/rm -rf $out
fi
return 0
}

export_variables()
{
if [ $# -ne 5 ] ; then
	echo "usage: export_variables($#) dir envfile owner group mode" 1>&2
	exit 1
fi
dir=$1
file=$2
owner=$3
group=$4
mode=$5
if [ ! -d $dir ] ; then
	echo "$dir: No such file or directory" 1>&2
	exit 1
fi
if [ $? -ne 0 ] ; then
	echo "$dir: unable to chdir" 1>&2
	exit 1
fi
if [ $force -eq 0 -a -f $file ] ; then
	echo "$file exists and --force not specified" 1>&2
	return 1
fi
(
for i in `/bin/ls $dir`
do
	if [ -s $dir/$i ] ; then
		value=$(@prefix@/bin/qcat $dir/$i)
		if [ -n "$value" ] ; then
			echo $i="$value"
		else
			echo $i=\"\"
		fi
	else
		echo "$i="
	fi
done
) > $file
if [ -s $file ] ; then
	/bin/chmod $mode $file
	$chown $owner:$group $file
else
	/bin/rm -f $file
fi
}

save_all_variables()
{
if [ -z "$owner" ] ; then
	owner=root
fi
if [ -z "$group" ] ; then
	group=0
fi
if [ -z $mode ] ; then
	mode=0640
fi
for i in `/bin/ls -d "$DESTDIR"$servicedir/*/variables "$DESTDIR"$servicedir/.svscan/variables 2>/dev/null`
do
	if [ $silent -eq 0 ] ; then
		printf "export %-35s %s %s %s\n" "`dirname $i`" $owner $group $mode
	fi
	export_variables $i $i/.variables $owner $group $mode
done
dir="$DESTDIR"$sysconfdir/control/defaultqueue
if [ -d $dir ] ; then
	if [ $silent -eq 0 ] ; then
		printf "export %-35s %s %s %s\n" $dir $owner $group $mode
	fi
	export_variables $dir $dir/.variables "indimail" "qmail" 0644
fi
}

restore_all_variables()
{
if [ -z "$owner" ] ; then
	owner=root
fi
if [ -z "$group" ] ; then
	group=0
fi
if [ -z $mode ] ; then
	mode=0640
fi
old=$force
force=1
for j in `/bin/ls "$DESTDIR"$servicedir/*/variables/.variables "$DESTDIR"$servicedir/.svscan/variables/.variables 2>/dev/nul`
do
	# remove last 21 char to get the directory+service_name
	i=`echo $j|sed 's/.\{21\}$//'`
	if [ ! -s $i/variables/.variables -a ! -s $i/.variables ] ; then
		echo "$i/variables/.variables or $i/.variables missing" 1>&2
		continue
	fi
	import_variables $i/variables $i/variables/.variables 1 $owner $group $mode
done
dir="$DESTDIR"$sysconfdir/control/defaultqueue
import_variables $dir $dir/.variables 1 "indimail" "qmail" 0644
force=$old
}

do_recontrol()
{
if [ -z "$CONTROLDIR" ] ; then
	CONTROLDIR=$sysconfdir/control
fi
defaultHost=$([ -n "$HOSTNAME" ] && echo "$HOSTNAME" || uname -n)
if [ -n "$default_domain" ] ; then
	defaultDomain=$default_domain
else
	defaultDomain=$(echo $defaultHost | sed 's/^\([^\.]*\)\.\([^\.]*\)\./\2\./')
fi
if [ -z "$defaultHost" -o -z "$defaultDomain" ] ; then
	echo "host or domain cannot be null" 1>&2
	exit 1
fi
orig=$(qcat "$DESTDIR"$CONTROLDIR/me)

# config-fast - me, defaulthost, envnoathost, defaultdomain,
# plusdomain, locals, rcpthosts, virtualdomains
/usr/libexec/indimail/config-fast --quiet $defaultDomain
SYSTEM=$(uname -s)
out=$(mktemp -t recontrolXXXXXX)
for i in nodnscheck locals rcpthosts smtpgreeting me localiphost
do
	if [ -f "$DESTDIR"$CONTROLDIR/$i ] ; then
		if [ "$i" = "nodnscheck" ] ; then
			sed -e "s{^@"$orig"{@"$defaultHost"{g" "$DESTDIR"$CONTROLDIR/$i > $out
		else
			sed -e "s{^$orig{$defaultHost{g" "$DESTDIR"$CONTROLDIR/$i > $out
		fi
		diff "$DESTDIR"$CONTROLDIR/$i $out > /dev/null
		if [ $? -ne 0 ] ; then
			# get permission of the original file before
			# updating it
			case $SYSTEM in
    			Darwin|FreeBSD)
				perm=$(stat -f %Op /etc/hosts|cut -c4-)
				;;
				Linux)
				perm=$(stat -c %a "$DESTDIR"$CONTROLDIR/$i)
				;;
				*)
				perms=0644
				;;
			esac
			echo "updating $DESTDIR""$CONTROLDIR/$i with $defaultHost"
			/bin/cp $out "$DESTDIR"$CONTROLDIR/$i
			if [ -n $perm ] ; then
				chmod $perm "$DESTDIR"$CONTROLDIR/$i
			else
				chmod 0644 "$DESTDIR"$CONTROLDIR/$i
			fi
		fi
		/bin/rm -f $out
	fi
done
}

create_cert()
{
	if [ $# -ne 3 ] ; then
		echo "svctool: create_cert postmaster common_name validity_days" 1>&2
			exit 1
	fi
	postmaster=$1
	common_name=$2
	no_of_days=$3
	if [ -n "$certdir" ] ; then
		CERTDIR=$certdir
	else
		CERTDIR=$sysconfdir/certs
	fi
	if [ -z "$postmaster" ] ; then
		echo "postmaster not specified" 1>&2
		exit 1
	fi
	if [ -z "$common_name" ] ; then
		echo "CN not specified" 1>&2
		exit 1
	fi
	if [ ! -x /usr/bin/openssl ] ; then
		echo "/usr/bin/openssl: No such file or directory" 1>&2
		exit 1
	fi
	rand_file=$DESTDIR"$CERTDIR/servercert.rand"
	if [ ! -f $rand_file -o $force -eq 1 ] ; then
		if [ ! -d "$DESTDIR"$CERTDIR ] ; then
			mkdir -p "$DESTDIR"$CERTDIR
		fi
		head -c 512 /dev/urandom > $rand_file
		chown indimail:qcerts $rand_file
		chmod 644 $rand_file
	fi
	conf_file=$DESTDIR"$CERTDIR/servercert.cnf"
	if [ ! -f $conf_file -o $force -eq 1 ] ; then
		echo "Creating SSL Configuration email=$postmaster cn=$common_name"
		create_ssl_cnf $postmaster $common_name > $TMPDIR/config.cnf.$$
		/bin/mkdir -p "$DESTDIR"$CERTDIR
		change_config $conf_file $TMPDIR/config.cnf.$$
		/bin/chmod 640 $conf_file
		$chown root:qcerts $conf_file
	fi
	if [ -f "$DESTDIR"$CERTDIR/servercert.pem -a $force -ne 1 ] ; then
		echo ""$DESTDIR"$CERTDIR/servercert.pem exists. Remove to create new" 1>&2
		exit 1
	fi
	echo "Creating openssl Certificate for $no_of_days days in $CERTDIR/servercert.pem"
	/usr/bin/openssl req -new -x509 -nodes -days $no_of_days -out "$DESTDIR"$CERTDIR/servercert.pem \
		-keyout "$DESTDIR"$CERTDIR/servercert.pem -config $conf_file
	if [ $? -ne 0 ] ; then
		echo "Failed to create openssl x509 certificate" 1>&2
		exit 1
	fi
	/bin/chmod 640 "$DESTDIR"$CERTDIR/servercert.pem
	$chown root:qcerts "$DESTDIR"$CERTDIR/servercert.pem
	if [ -f "$DESTDIR"$CERTDIR/clientcert.pem ] ; then
		t=$(readlink "$DESTDIR"$CERTDIR/clientcert.pem)
		t=$(basename $t)
		if [ ! "$t" = "servercert.pem" ] ; then
			/bin/rm -f "$DESTDIR"$CERTDIR/clientcert.pem
		fi
	fi
	if [ ! -f "$DESTDIR"$CERTDIR/clientcert.pem ] ; then
		cd "$DESTDIR"$CERTDIR
		/bin/ln -s servercert.pem \
			clientcert.pem 2>/dev/null
		if [ $? -ne 0 ] ; then
			echo "Failed to link clientcert.pem to servercert.pem" 1>&2
			exit 1
		fi
	fi
	echo "Updating temporary RSA and DH keys"
	if [ -x "$DESTDIR"$libexecdir/update_tmprsadh ] ; then
		"$DESTDIR"$libexecdir/update_tmprsadh --certdir="$DESTDIR"$CERTDIR
	elif [ -x $libexecdir/update_tmprsadh ] ; then
		$libexecdir/update_tmprsadh --certdir="$DESTDIR"$CERTDIR
	else
		echo "$libexecdir/update_tmprsadh: No such file or directory" 1>&2
		echo "generate RSA/DH parameters manually" 1>&2
	fi

	updated=0
	if [ $updatecerts -eq 1 ] ; then
		svc_list=""
		echo "Fixing CERTDIR variable for services"
		for i in @servicedir@/*/variables/CERTDIR
		do
			j=$(@prefix@/bin/qcat $i 2>/dev/null)
			if [ ! " $j" = " $CERTDIR" ] ; then
				updated=1
				echo "Fixed $i"
				echo $CERTDIR > $i
				x=$(echo $i | cut -d'/' -f1,2,3)
				if [ -z "$svc_list" ] ; then
					svc_list="$x"
				else
					svc_list="$svc_list $x"
				fi
			else
				echo "$i: OK"
			fi
		done

		echo "Updating CERT as $CERTDIR/servercert.pem"
		for i in `ls @servicedir@/*/variables/*CERTFILE \
			@servicedir@/*/variables/CLIENTCERT \
			@servicedir@/*/variables/SERVERCERT 2>/dev/null`
		do
			x=$(echo $i | cut -d'/' -f1,2,3)
			echo $svc_list | grep $x >/dev/null
			if [ $? -ne 0 ] ; then
				if [ -z "$svc_list" ] ; then
					svc_list="$x"
				else
					svc_list="$svc_list $x"
				fi
			fi
			j=$(@prefix@/bin/qcat $i)
			if [ ! " $j" = " $CERTDIR/servercert.pem" ] ; then
				echo "Fixed $i"
				updated=1
				echo "$CERTDIR/servercert.pem" > $i
			else
				echo "$i: OK"
			fi
		done
		if [ -f /usr/bin/imapd ] ; then
			echo "Updating TLS CACHE as $CERTDIR/couriersslcache"
			for i in `ls @servicedir@/*/variables/TLS_CACHEFILE 2>/dev/null`
			do
				j=$(@prefix@/bin/qcat $i)
				if [ ! " $j" = " $CERTDIR/couriersslcache" ] ; then
					echo "Fixed $i"
					echo "$CERTDIR/couriersslcache" > $i
					updated=1
					x=$(echo $i | cut -d'/' -f1,2,3)
					echo $svc_list | grep $x >/dev/null
					if [ $? -ne 0 ] ; then
						if [ -z "$svc_list" ] ; then
							svc_list="$x"
						else
							svc_list="$svc_list $x"
						fi
					fi
				else
					echo "$i: OK"
				fi
			done
		fi
	else
		if [ -f /usr/bin/imapd ] ; then
			echo "--update-certs not given. Skipping updating Services for CERTDIR, CERTFILE and TLS_CACHE" 1>&2
		else
			echo "--update-certs not given. Skipping updating Services for CERTDIR, CERTFILE" 1>&2
		fi
	fi

	echo "====== Certificate $CERTDIR/servercert.pem ========="
	/usr/bin/openssl x509 -in "$DESTDIR"$CERTDIR/servercert.pem -noout -text
	echo "================================================================"

	# restart services
	if [ $updated -eq 1 ] ; then
		echo "Restarting services using $CERTDIR/servercert.pem"
		for i in $svc_list; do echo "Restarting service $i"; done
		svc -r $svc_list
	fi
}

print_all_variables()
{
	for i in $servicedir/*
	do
		if [ -d $i/variables ] ; then
			vars=$(envdir -c $i/variables env 2>/dev/null)
			if [ -n "$vars" ] ; then
				echo "------ environment variables for $i ------------"
				envdir -c $i/variables env
				echo
				if [ -L $i/variables/.envdir ] ; then
					link_name=$(readlink $i/variables/.envdir)
					echo "NOTE: additional variables were obtained from $link_name"
					echo
				elif [ -f $i/variables/.envdir ] ; then
					echo "NOTE: additional variables were obtained from .envdir"
					@prefix@/bin/qcat $i/variables/.envdir
					echo
				fi
				if [ -f $i/variables/.envfile ] ; then
					echo "NOTE: additional variables were obtained from .envfile"
					@prefix@/bin/qcat $i/variables/.envfile
					echo
				fi
			else
				echo "------ $i ------------"
				echo "NOTE: This service has no configured variables"
				echo
			fi
		fi
	done
	for i in $sysconfdir/control/defaultqueue $sysconfdir/control/global_vars \
		$sysconfdir/ezmlm/global_vars
	do
		if [ -d $i ] ; then
			vars=$(envdir -c $i env 2>/dev/null)
			if [ -n "$vars" ] ; then
				if [ -L $i/.envdir -o -f $i/.envdir -o -f $i/.envfile ] ; then
					echo "------ $i ------------"
				fi
				if [ -L $i/.envdir ] ; then
					link_name=$(readlink $i/.envdir)
					echo "NOTE: additional variables were obtained from $link_name"
					echo
				elif [ -f $i/.envdir ] ; then
					echo "NOTE: additional variables were obtained from .envdir"
					@prefix@/bin/qcat $i/.envdir
					echo
				fi
				if [ -f $i/.envfile ] ; then
					echo "NOTE: additional variables were obtained from .envfile"
					@prefix@/bin/qcat $i/.envfile
					echo
				fi
				echo "------ environment variables for $i ------------"
				envdir -c $i env
				echo
			else
				echo "------ $i ------------"
				echo "NOTE: This directory has no configured variables"
				echo
			fi
		fi
	done
}

################################# Main ##################################
if test $# -eq 0; then
	usage 1
fi
if [ $(id -u) -ne 0 ] ; then
	echo "svctool is not meant to be run by mere mortals. Use sudo to get superpowers"
	exit 100
fi

chown=$(which chown)

if [ -f $sysconfdir/svctool.cnf ] ; then
	perms=$(stat -c "%u:%g:%a" $sysconfdir/svctool.cnf)
	if [ $? -ne 0 ] ; then
		exit 100
	fi
	if [ ! $perms = "0:0:400" ] ; then
		echo "fixing permissions of svctool.cnf"
		$chown root:0 $sysconfdir/svctool.cnf
		/bin/chmod 400 $sysconfdir/svctool.cnf
	fi
	. $sysconfdir/svctool.cnf
fi
[ -z "$MYSQL_PASS" ] && MYSQL_PASS="ssh-1.5-"
[ -z "$PRIV_PASS" ]  && PRIV_PASS="4-57343-"
[ -z "$ADMIN_PASS" ] && ADMIN_PASS="benhur20"
[ -z "$REPL_PASS" ] && REPL_PASS="slaveserver"
[ -z "$TMPDIR" ] && TMPDIR="/tmp/indimail"
[ ! -d "$TMPDIR" ] && mkdir -m 0700 -p $TMPDIR
if [ ! -d $TMPDIR ] ; then
	echo "$TMPDIR: No such file or directory" 1>&2
	exit 1
fi

prog_args="$QmailBinPrefix/sbin/svctool"
force=0
no_multi=0
qtype="static"
silent=0
down_state=0
use_unshare=0
use_ssl=0
use_starttls=0
forcetls=0
nooverwrite=0
run_file_only=0
setgroups=0
utf8=0
usefsync=0
usefdatasync=0
usesyncdir=0
mount_resolvconf=0
valid_for=366
todo_proc=0
enablecram=0
databytes=$DATABYTES
qmailqueue="/usr/sbin/qmail-queue"
qmailsmtpd=$QmailBinPrefix"/sbin/qmail-smtpd"
envdir_opts="-c"
updatecerts=0
setuser_privilege=0
sanitized_env=""
chkrecipient=0
chksender=0
chkrelay=0
hidehost=0
secureauth=0
death=86400
barelf=0
if [ " $CONTROLDIR" = " " ] ; then
	cntrldir=$sysconfdir/control
else
	slash=`echo $CONTROLDIR | cut -c1`
	if [ " $slash" = " /" ] ; then
		cntrldir=$CONTROLDIR
	else
		cntrldir=$sysconfdir/$CONTROLDIR
	fi
fi
CONTROLDIR=$cntrldir
export CONTROLDIR
if [ -f $cntrldir/servicedir.conf ] ; then
	servicedir=$(@prefix@/bin/qcat $cntrldir/servicedir.conf)
else
	servicedir=@servicedir@
fi
if [ -x /bin/touch ] ; then
	touch=/bin/touch
elif [ -x /usr/bin/touch ] ; then
	touch=/usr/bin/touch
else
	touch=/bin/touch
fi
while test $# -gt 0; do
	case "$1" in
	-*=*)
	optarg=`echo "$1" | sed 's/[-_a-zA-Z0-9]*=//'`
	optval=`echo "$1" | cut -d'=' -f1`
	prog_args="$prog_args $optval=\"$optarg\""
	;;
	*)
	optarg=
	prog_args="$prog_args $1"
	;;
	esac

	case "$1" in
	--qbase=*)
	qbase=$optarg
	;;
	--verbose)
	verbose="-v"
	;;
	--qcount=*)
	qcount=$optarg
	;;
	--qstart=*)
	qstart=$optarg
	;;
	--min-free=*)
	min_free=$optarg
	;;
	--deliverylimit-count=*)
	mailcount_limit=$optarg
	;;
	--deliverylimit-size=*)
	mailsize_limit=$optarg
	;;
	--overquota_mailsize=*)
	overquota_mailsize=$optarg
	;;
	--servicedir=*)
	servicedir=$optarg
	;;
	--qmaildir=*)
	qmaildir=$optarg
	QmailHOME=$qmaildir
	export QmailHOME
	;;
	--cntrldir=*)
	slash=`echo $optarg | cut -c1`
	if [ " $slash" = " /" ] ; then
		cntrldir=$optarg
	else
		cntrldir=$sysconfdir/$optarg
	fi
	CONTROLDIR=$cntrldir
	export CONTROLDIR
	;;
	--skipsend)
	skipsend=1
	;;
	--hide-host)
	hidehost=1
	;;
	--qmailqueue=*)
	qmailqueue=$optarg
	;;
	--qmailsmtpd=*)
	qmailsmtpd=$optarg
	;;
	--envdiropts=*)
	envdir_opts=$optarg
	;;
	--databytes=*)
	databytes=$optarg
	;;
	--scanint=*)
	scan_interval=$optarg
	;;
	--memory=*)
	memory=$optarg
	;;
	--dmemory=*)
	dmemory=$optarg
	;;
	--msgqsize=*)
	msgqsize=$optarg
	;;
	--password-cache)
	password_cache=1
	;;
	--query-cache)
	query_cache=1
	;;
	--use-btree)
	use_btree=1
	;;
	--max-btree-count=*)
	max_btree_count=$optarg
	;;
	--servicetag=*)
	servicetag=$optarg
	;;
	--qregex)
	qregex=1
	;;
	--qhpsi=*)
	qhpsi=$optarg
	;;
	--virus-filter=*)
	viruscheck=$optarg
	;;
	--virus-filter)
	viruscheck=1
	;;
	--no-multi)
	no_multi=1
	;;
	--qtype=*)
	case $qtype in
		static|dynamic|compat)
		qtype=$optarg
		;;
		*)
		echo qtype must be static, dynamic or compat 1>&2
		exit 1
		;;
	esac
	;;
	--content-filter=*)
	bodycheck=$optarg
	;;
	--content-filter)
	bodycheck=1
	;;
	--dnscheck)
	dnscheck=1
	;;
	--helofqdn)
	helofqdn=1
	;;
	--helocheck)
	helocheck=1
	;;
	--fsync)
	usefsync=1
	;;
	--fdatasync)
	usefdatasync=1
	;;
	--syncdir)
	usesyncdir=1
	;;
	--debug-level=*)
	debug_level=$optarg
	;;
	--spamfilter=*)
	spamfilter=$optarg
	echo $spamfilter | grep '/' >/dev/null 2>&1
	if [ $? -ne 0 ] ; then
		echo spamfilter must be an absolute path 1>&2
		exit 1
	fi
	;;
	--logfilter=*)
	logfilter=$optarg
	;;
	--localfilter)
	localfilter=1
	;;
	--remotefilter)
	remotefilter=1
	;;
	--todo-proc)
	todo_proc=1
	;;
	--rejectspam=*)
	rejectspam=$optarg
	;;
	--spamexitcode=*)
	spamexitcode=$optarg
	;;
	--masquerade)
	masquerade=1
	;;
	--logdir=*)
	LOGDIR=$optarg
	export LOGDIR
	;;
	--indimaildir=*)
	indimaildir=$optarg
	export indimaildir
	;;
	--domainlimits)
	domainlimits=1
	;;
	--chkrecipient)
	chkrecipient=1
	;;
	--chksender)
	chksender=1
	;;
	--antispoof)
	antispoof=1
	;;
	--cugmail)
	cugmail=1
	;;
	--secureauth)
	secureauth=1
	;;
	--forceauthsmtp)
	forceauthsmtp=1
	;;
	--enablecram)
	enableram=1
	;;
	--authsmtp)
	authsmtp=1
	;;
	--remote-authsmtp=*)
	remoteauthsmtp=$optarg
	;;
	--min-resend-min=*)
	min_resend_min=$optarg
	;;
	--resend-win-hr=*)
	resend_win_hr=$optarg
	;;
	--timeout-days=*)
	timeout_days=$optarg
	;;
	--context-file=*)
	echo $optarg | grep '/' >/dev/null 2>&1
	if [ $? -eq 0 ] ; then
		echo context file cannot have absolute path 1>&2
		exit 1
	fi
	context_file=$optarg
	;;
	--save-interval=*)
	save_interval=$optarg
	;;
	--hash-size=*)
	hash_size=$optarg
	;;
	--whitelist=*)
	echo $optarg | grep '/' >/dev/null 2>&1
	if [ $? -eq 0 ] ; then
		echo whitelist file cannot have absolute path 1>&2
		exit 1
	fi
	whitelist=$optarg
	;;
	--authall)
	authall=1
	;;
	--chkrelay)
	chkrelay=1
	;;
	--paranoid)
	paranoid=1
	;;
	--dmasquerade)
	dmasquerade=1
	;;
	--force)
	force=1
	;;
	--routes=*)
	routes=$optarg
	;;
	--utf8)
	utf8=1
	;;
	--setgroups)
	setgroups=1
	;;
	--cname-lookup)
	enable_cname_lookup="yes"
	;;
	--setuser-priv)
	setuser_privilege=1
	;;
	--sanitize-env=*)
	sanitized_env=$optarg
	;;
	--odmr)
	odmr=1
	;;
	--rbl=*)
	rbl=1
	rbl_list="$optarg $rbl_list"
	;;
	--proxy=*)
	proxy_port=$optarg
	;;
	--localip=*)
	ipaddress=$optarg
	;;
	--maxdaemons=*)
	maxdaemons=$optarg
	;;
	--setpassword=*)
	password_cmd=$optarg
	;;
	--certfile=*)
	certfile=$optarg
	;;
	--maxperip=*)
	maxperip=$optarg
	;;
	--nolastauth)
	nolastauth=1
	;;
	--legacyserver)
	legacyserver=1
	;;
	--smtp-plugin)
	enable_smtp_plugin=1
	;;
	--starttls) # e.g. port 25, 587
	use_starttls=1
	use_ssl=0
	;;
	--ssl) # e.g. port 465
	use_starttls=0
	use_ssl=1
	;;
	--forcetls)
	forcetls=1
	;;
	--tlsprog=*)
	tlsprog=$optarg
	echo $tlsprog | grep '/' >/dev/null 2>&1
	if [ $? -ne 0 ] ; then
		echo tlsprog must be an absolute path 1>&2
		exit 1
	fi
	;;
	--persistdb)
	persistdb=1
	;;
	--mysqlhost=*)
	mysql_host=$optarg
	;;
	--mysqluser=*)
	mysql_user=$optarg
	;;
	--mysqlpass=*)
	mysql_pass=$optarg
	;;
	--mysqlport=*)
	mysql_port=$optarg
	;;
	--mysqlsocket=*)
	mysqlSocket=$optarg
	;;
	--avguserquota=*)
	avg_user_quota=$optarg
	;;
	--hardquota=*)
	hard_quota=$optarg
	;;
	--base_path=*)
	base_path=$optarg
	;;
	--default-domain=*)
	default_domain=$optarg
	;;
	--databasedir=*)
	databasedir=$optarg
	;;
	--capath=*)
	ca_path=$optarg
	;;
	--certdir=*)
	certdir=$optarg
	;;
	--update-certs)
	updatecerts=1
	;;
	--mysqlPrefix=*)
	mysqlPrefix=$optarg
	;;
	--clamdPrefix=*)
	clamdPrefix=$optarg
	;;
	--sysconfdir=*)
	mysysconfdir=$optarg
	;;
	--dbdir=*)
	dbdir=$optarg
	;;
	--postmaster=*)
	postmaster=$optarg
	;;
	--common_name=*)
	common_name=$optarg
	;;
	--validity_days=*)
	valid_for=$optarg
	;;
	--attach)
	if [ -f /usr/bin/zip ] ; then
		attach_report=1
	else
		echo "/usr/bin/zip is missing on your system" 1>&2
		echo "will not use attachments to send out reports" 1>&2
		echo "Press ENTER to continue" 1>&2
		read key
	fi
	;;

	--showctl|--showctl=*)
	if [ -z "$optarg" ] ; then
		CONTROLDIR=control
	else
		CONTROLDIR=$optarg
	fi
	export CONTROLDIR
	echo $CONTROLDIR | grep '/' >/dev/null 2>&1
	if [ $? -eq 0 ] ; then
		if [ ! -d $CONTROLDIR ] ; then
			echo "$CONTROLDIR: No such file or directory" 1>&2
			exec "$DESTDIR"$QmailBinPrefix/bin/qmail-showctl
		fi
	else
		if [ ! -d /etc/indimail/$CONTROLDIR ] ; then
			echo "/etc/indimail/$CONTROLDIR: No such file or directory" 1>&2
		else
			exec "$DESTDIR"$QmailBinPrefix/bin/qmail-showctl
		fi
	fi
	exit 1
	;;

	--fixsharedlibs)
	if [ -z "$CONTROLDIR" ] ; then
		CONTROLDIR=$sysconfdir/control
	fi
	case "$host" in
		*-*-darwin**)
		libdir=/opt/local/lib
		if [ -n "$DESTDIR" ] ; then
			indlib=`ls -d "$DESTDIR""$libdir"*/libindimail.*.dylib 2>/dev/null|sed -e "s}$DESTDIR}}g"`
			if [ -z "$indlib" ] ; then
				indlib=`ls -d "$libdir"*/libindimail.*.dylib 2>/dev/null`
			fi
		else
			indlib=`ls -d "$libdir"*/libindimail.*.dylib 2>/dev/null`
		fi
		;;
		*)
		libdir=$QmailBinPrefix/lib
		if [ -n "$DESTDIR" ] ; then
			if [ -r /etc/debian_version ] ; then
				indlib=`ls -d "$DESTDIR""$libdir"/*-linux-gnu/libindimail.so.*.*.* 2>/dev/null`
			else
				indlib=`ls -d "$DESTDIR""$libdir"*/libindimail.so.*.*.* 2>/dev/null|sed -e "s}$DESTDIR}}g"`
			fi
			if [ -z "$indlib" ] ; then
				if [ -r /etc/debian_version ] ; then
					indlib=`ls -d "$libdir"/*-linux-gnu/libindimail.so.*.*.* 2>/dev/null`
				else
					indlib=`ls -d "$libdir"*/libindimail.so.*.*.* 2>/dev/null`
				fi
			fi
		else
			if [ -r /etc/debian_version ] ; then
				indlib=`ls -d "$libdir"/*-linux-gnu/libindimail.so.*.*.* 2>/dev/null`
			else
				indlib=`ls -d "$libdir"*/libindimail.so.*.*.* 2>/dev/null`
			fi
		fi
		;;
	esac
	if [ -n "$indlib" ] ; then
		check_update_if_diff "$DESTDIR"$CONTROLDIR/libindimail $indlib
	elif [ -f "$DESTDIR"$CONTROLDIR/libindimail ] ; then
		echo "removing "$DESTDIR"$CONTROLDIR/libindimail"
		/bin/rm -f "$DESTDIR"$CONTROLDIR/libindimail
	fi
	check_libmysqlclient_lib
	nolog=1
	exit $?
	;;
	--queuefix=*)
	"$DESTDIR"$QmailBinPrefix/bin/queue-fix $optarg
	exit 0
	;;

	--threads=*)
	thread_count=$optarg
	;;

	--activeDays=*)
	activeDays=$optarg
	;;

	--timeout=*)
	timeout=$optarg
	;;

	--nolog)
	nolog=1
	;;

	# create new qmail assign file
	--wipe)
	wipe_option=1
	;;

	--silent)
	silent=1
	;;

	--destdir=*)
	DESTDIR=$optarg
	;;

	--dkverify=*)
	dkverify_option=$optarg
	;;
	--dksign=*)
	dksign_option=$optarg
	;;
	--private_key=*)
	private_key=$optarg
	;;
	--down)
	down_state=1
	;;
	--unshare)
	use_unshare=1
	;;

	--smtp=*)
	option=1
	smtp_port=$optarg
	;;

	--imap=*)
	option=2
	imap_port=$optarg
	;;

	--pop3=*)
	option=3
	pop3_port=$optarg
	;;

	--rmsvc=*)
	option=4
	service=$optarg
	;;

	--ensvc=*)
	option=5
	service=$optarg
	;;

	--infifo=*)
	infifo=$optarg
	;;

	--inlookup=*)
	option=6
	infifo=$optarg
	;;

	--indisrvr=*)
	option=7
	bind_port=$optarg
	;;

	--mysql=*)
	option=8
	mysql_port=$optarg
	;;

	--fetchmail)
	option=9
	;;

	--nooverwrite)
	nooverwrite=1
	;;

	--config=*)
	if [ " $option" = " " ] ; then
		option=10
		config_name=$optarg
	else
		conf_file=$optarg
	fi
	;;

	--module=*)
	if [ -z "$config_name" ] ; then
		echo "--config option not specified" 1>&2
		exit 1
	fi
	module_name=$optarg
	;;

	--check-install)
	option=11
	;;

	--backup=*)
	backupdir=$optarg
	option=13
	;;

	--repair-tables)
	option=14
	;;

	--report=*)
	option=15
	report_option=$optarg
	;;

	--qscanq)
	option=16
	;;

	--clamd)
	option=17
	;;

	--poppass=*)
	poppass_port=$optarg
	option=18
	;;

	--svscanlog)
	option=19
	;;

	--resolvconf)
	mount_resolvconf=1
	;;

	--death=*)
	death=$optarg
	;;

	--barelf)
	barelf=1
	;;

	--initcmd=*)
	svscan_init_cmd=$optarg
	;;

	--pwdlookup=*)
	option=20
	socket_path=$optarg
	;;

	--dumpconfig)
	option=21
	;;

	--greylist=*)
	option=22
	grey_port=$optarg
	;;
	--use-greydaemon)
	use_greydaemon=1
	;;

	--qmtp=*)
	option=23
	qmtp_port=$optarg
	;;
	--qmqp=*)
	option=24
	qmqp_port=$optarg
	;;

	--queueParam=*)
	option=25
	queuedef=$optarg
	echo $queuedef | grep '/' >/dev/null 2>&1
	if [ $? -eq 0 ] ; then
		echo queuedef cannot be an absolute path 1>&2
		exit 1
	fi
	;;

	--delivery=*)
	option=26
	queue_ident=$optarg
	;;

	--udplogger=*)
	option=27
	udp_port=$optarg
	;;

	--fifologger=*)
	option=28
	logfilter=$optarg
	;;

	--mrtg=*)
	option=29
	htmldir=$optarg
	;;

	--shared-objects=*)
	shared_objects=$optarg
	;;

	--use-dlmopen=*)
	use_dlmopen=$optarg
	;;

	--check-certs)
	cert_file=""
	option=30
	;;

	--check-certs=*)
	cert_file=$optarg
	option=30
	;;
	--skip-sendmail)
	skip_sendmail_check=1
	;;

	--enable-service)
	option=31
	service=$optarg
	break
	;;

	--disable-service)
	option=32
	service=$optarg
	break
	;;

	--tlsa=*)
	option=33
	dane_port=$optarg
	;;

	--refreshsvc=*)
	option=34
	service_name=$optarg
	;;

	--run-file-only)
	if [ $option -eq 34 ] ; then
		run_file_only=1
	fi
	;;

	--autorefresh=*)
	option=35
	service_name=$optarg
	;;

	--mysqlupgrade)
	if [ " $mysqlPrefix" = " " ] ; then
		echo "MySQL Installation Prefix directory not specified" 1>&2
		usage 1
	fi
	if [ -x $mysqlPrefix/bin/mysql_upgrade ] ; then
		echo "Running MySQL Upgrade to fix MySQL 8.0 Montrosity"
		$mysqlPrefix/bin/mysql_upgrade -u mysql -p"$PRIV_PASS"
	fi
	;;

	--mode=*)
	group=$optarg
	;;
	--owner=*)
	owner=$optarg
	;;
	--group=*)
	group=$optarg
	;;
	--envdir=*)
	envdir=$optarg
	;;
	--service-name=*)
	service_name=$optarg
	;;
	--variable-value=*)
	variable_value=$optarg
	;;
	--set-variable=*)
	variable_name=$optarg
	option=36
	;;
	--modify-variable=*)
	variable_name=$optarg
	option=37
	;;
	--unset-variable=*)
	variable_name=$optarg
	option=38
	;;
	--remove-variable=*)
	variable_name=$optarg
	option=39
	;;
	--restore-variables)
	option=40
	;;
	--import-variables=*)
	option=41
	env_file=$optarg
	;;
	--export-variables=*)
	option=42
	env_file=$optarg
	;;
	--save-variables)
	option=43
	;;
	--restore-all-variables)
	option=44
	;;
	--print-variables)
	option=45
	;;
	--print-all-variables)
	option=46
	;;
	--slowq)
	option=47
	;;

	--version)
	echo "$RCSID"
	exit 0
	;;

	--help)
	usage 0
	;;

	*)
	echo "invalid option [$1]" 1>&2
	read key
	usage 1
	;;
	esac
	shift
done

do_exit()
{
	if [ ! " $prog_args" = " " ] ; then
		if [ " $nolog" = " " ] ; then
			if [ ! -d "$DESTDIR"$LOGDIR ] ; then
				/bin/mkdir -p "$DESTDIR"$LOGDIR
			fi
			echo "`date` $prog_args" >> "$DESTDIR"$LOGDIR/svctool.log
		fi
	fi
	exit 0
}

#
# Main
#
case $option in
	1) # SMTP Service
	if [ " $servicedir" = " " ] ; then
		echo "Supervise Directory not specified" 1>&2
		usage 1
	elif [ " $smtp_port" = " " ] ; then
		echo "SMTP PORT not specified" 1>&2
		usage 1
	elif [ $smtp_port -lt 1 ] ; then
		echo "SMTP PORT should be a positive number" 1>&2
		usage 1
	fi
	if [ -z "$odmr" ] ; then
		case "$dkverify_option" in
			dkim)
			;;
			none|"")
			;;
			*)
			echo "invalid --dkverify=$dkverify_option" 1>&2
			usage 1
			;;
		esac
		case "$dksign_option" in
			dkim)
			if [ " $private_key" = " " ] ; then
				echo "--private_key not specfied" 1>&2
				usage 1
			fi
			;;
			none|"")
			;;
			*)
			echo "invalid --dksign=$dksign_option" 1>&2
			usage 1
			;;
		esac
		if [ -z "$skipsend" ] ; then
			if [ -z "$qbase" ] ; then
				echo "Queue Directory not specified" 1>&2
				usage 1
			elif [ -z "$qcount" ] ; then
				echo "Queue Count not specified" 1>&2
				usage 1
			elif [ -z "$qstart" ] ; then
				echo "Start Queue Number not specified" 1>&2
				usage 1
			elif [ $qcount -lt 1 ] ; then
				echo "No of Queues should be a positive number" 1>&2
				usage 1
			elif [ $qstart -lt 1 ] ; then
				echo "Start Queue Number should be a positive number" 1>&2
				usage 1
			fi
			echo "Creating $qcount queues"
			if [ $use_starttls -ne 0 -o $use_ssl -ne 0 ] ; then
				create_delivery "$qbase" "$qcount" "$qstart" "$servicedir" "$smtp_port" "$routes" "1" "$utf8"
			else
				create_delivery "$qbase" "$qcount" "$qstart" "$servicedir" "$smtp_port" "$routes" "0" "$utf8"
			fi
		fi
	fi
	if [ " $odmr" = " " ] ; then
	echo "Creating SMTP Listener Port $smtp_port, Service $servicedir, Queue Base $qbase, Queue Count $qcount, Queue Start $qstart"
	else
	echo "Creating SMTP Listener Port $smtp_port, Service $servicedir"
	fi
	create_smtp "$qbase" "$qcount" "$qstart" "$servicedir" "$smtp_port" "$use_ssl" "$forcetls" "$infifo" "$utf8"
	;;

	2) # IMAP Service
	if [ " $imap_port" = " " ] ; then
		echo "IMAP PORT not specified" 1>&2
		usage 1
	elif [ " $servicedir" = " " ] ; then
		echo "Supervise Directory not specified" 1>&2
		usage 1
	elif [ $imap_port -lt 1 ] ; then
		echo "IMAP PORT should be a positive number" 1>&2
		usage 1
	fi
	if [ ! " $proxy" = " " ] ; then
		if [ " $hard_quota" = " " ] ; then
			echo "User Hard Quota not specified" 1>&2
			usage 1
		elif [ $hard_quota -lt 500000 ] ; then
			echo "Hard Quota should be greater than 500k" 1>&2
			usage 1
		fi
	fi
	echo "Creating IMAP4 Listener Port $imap_port, Service $servicedir infifo $infifo"
	create_courier "$servicedir" "imapd" "$imap_port" "$nolastauth" "$legacyserver" "$use_ssl" "$infifo"
	;;

	3) # POP3 Service
	if [ " $pop3_port" = " " ] ; then
		echo "POP3 PORT not specified" 1>&2
		usage 1
	elif [ " $servicedir" = " " ] ; then
		echo "Supervise Directory not specified" 1>&2
		usage 1
	elif [ $pop3_port -lt 1 ] ; then
		echo "POP3 PORT should be a positive number" 1>&2
		usage 1
	fi
	if [ ! " $proxy" = " " ] ; then
		if [ " $hard_quota" = " " ] ; then
			echo "User Hard Quota not specified" 1>&2
			usage 1
		elif [ $hard_quota -lt 500000 ] ; then
			echo "Hard Quota should be greater than 500k" 1>&2
			usage 1
		fi
	fi
	echo "Creating POP3 Listener Port $pop3_port, Service $servicedir infifo $infifo"
	create_courier "$servicedir" "pop3d" "$pop3_port" "$nolastauth" "$legacyserver" "$use_ssl" "$infifo"
	;;

	4) # remove service
	rmsvc "$service"
	;;

	5) # enable servie
	ensvc "$service"
	;;

	6) # inlookup service
	if [ " $servicedir" = " " ] ; then
		echo "Supervise Directory not specified" 1>&2
		usage 1
	elif [ " $infifo" = " " ] ; then
		echo "INFIFO not specified" 1>&2
		usage 1
	elif [ " $thread_count" = " " ] ; then
		echo "Thread Count not specified" 1>&2
		usage 1
	elif [ " $activeDays" = " " ] ; then
		activeDays=7
	fi
	echo "Creating InLookup Daemon on Fifo $infifo, Service $servicedir, Threads $thread_count ActiveDays=$activeDays"
	create_inlookup "$infifo" "$thread_count" "$activeDays" "$servicedir"
	;;

	7) # indisrvr service
	if [ " $servicedir" = " " ] ; then
		echo "Supervise Directory not specified" 1>&2
		usage 1
	elif [ " $bind_port" = " " ] ; then
		echo "Port not specified" 1>&2
		usage 1
	elif [ " $avg_user_quota" = " " ] ; then
		echo "Average User Quota not specified" 1>&2
		usage 1
	elif [ " $hard_quota" = " " ] ; then
		echo "User Hard Quota not specified" 1>&2
		usage 1
	elif [ " $base_path" = " " ] ; then
		echo "Base Path not specified" 1>&2
		usage 1
	elif [ $bind_port -lt 1 ] ; then
		echo "BIND PORT should be a positive number" 1>&2
		usage 1
	elif [ $avg_user_quota -lt 500000 ] ; then
		echo "Average User Quota should be greater than 500k" 1>&2
		usage 1
	elif [ $hard_quota -lt 500000 ] ; then
		echo "Hard Quota should be greater than 500k" 1>&2
		usage 1
	fi
	if [ -n "$mysql_host" ] ; then
		if [ " $mysql_user" = " " ] ; then
			echo "MySQL User not specified" 1>&2
			usage 1
		elif [ " $mysql_pass" = " " ] ; then
			echo "MySQL Passwd not specified" 1>&2
			usage 1
		fi
	fi
	if [ -n "$mysysconfdir" ] ; then
		sysconfdir=$mysysconfdir
	fi
	echo "Creating IndiMail Admin Service Port $bind_port, Service $servicedir, Base Path $base_path"
	create_indisrvr "$bind_port" "$mysql_host" "$mysql_user" "$mysql_pass" "$avg_user_quota" "$hard_quota" \
		"$base_path" "$servicedir" "$use_ssl"
	;;

	8) # MySQL service
	if [ " $servicedir" = " " ] ; then
		echo "Supervise Directory not specified" 1>&2
		usage 1
	elif [ " $databasedir" = " " ] ; then
		echo "Database directory not specified" 1>&2
		usage 1
	elif [ " $mysqlPrefix" = " " ] ; then
		echo "MySQL Installation Prefix directory not specified" 1>&2
		usage 1
	elif [ " $conf_file" = " " ] ; then
		echo "Config file not specified" 1>&2
		usage 1
	elif [ " $mysql_port" = " " ] ; then
		echo "Port not specified" 1>&2
		usage 1
	elif [ $mysql_port -lt 1 ] ; then
		echo "BIND PORT should be a positive number" 1>&2
		usage 1
	fi
	echo "Creating MySQL Service Port $mysql_port, Service $servicedir, conf file $conf_file"
	create_mysql_service "$mysql_port" "$mysqlPrefix" "$databasedir" "$conf_file" "$servicedir"
	;;

	9) # fetchmail service
	if [ -z "$qbase" ] ; then
		qbase=$indimaildir/queue
	fi
	if [ -z "$qcount" ] ; then
		qcount=5
	fi
	if [ -z "$qstart" ] ; then
		qstart=1
	fi
	if [ " $servicedir" = " " ] ; then
		echo "Supervise Directory not specified" 1>&2
		usage 1
	elif [ $qcount -lt 1 ] ; then
		echo "No of Queues should be a positive number" 1>&2
		usage 1
	elif [ $qstart -lt 1 ] ; then
		echo "Start Queue Number should be a positive number" 1>&2
		usage 1
	fi
	echo "Creating FetchMail Daemon Service $servicedir, Queue Base $qbase, Queue Count $qcount, Qstart $qstart"
	create_fetchmail "$qbase" "$qcount" "$qstart" "$servicedir"
	;;

	10) # --config
	# 1  MySQL Config creation
	# 2  MySQL Database creation
	# 3  MariaDB SSL/RSA cert creation
	# 4  Create default indimail control files
	# 5  Create default qmail control files
	# 6  create indimail users from system (/etc/passwd,etc)
	# 7  remove indimail users from system (/etc/passwd, etc)
	# 8  nssd config creation
	# 9  snmpd config creation
	# 10 create clamd, freschclam config
	# 11 create foxhole_all.cdb
	# 12 create bogofilter config and wordlist.db
	# 13 create CERTS
	# 14 add startup for indimail at boot
	# 15 remove startup for indimail from boot
	# 16 add indimail-mta as alternative mta
	# 17 remove indimail-mta as alternative mta
	# 18 enable selinux module for indimail
	# 19 enable selinux module for indimail-mta
	# 20 enable svscan in inittab
	# 21 recontrol

	case $config_name in
		mysql)
		if [ " $mysqlPrefix" = " " ] ; then
			echo "MySQL Installation Prefix directory not specified" 1>&2
			usage 1
		elif [ " $databasedir" = " " ] ; then
			echo "Database directory not specified" 1>&2
			usage 1
			exit 1
		fi
		if [ " $mysql_port" = " " ]; then
			mysql_port=3306
		fi
		if [ -z "$mysqlSocket" ]; then
			if [ -n "$MYSQL_SOCKET" ] ; then
				mysqlSocket=$MYSQL_SOCKET
			elif [ -d /run ] ; then
				mysqlSocket=/run/mysqld/mysqld.sock
			elif [ -d /var/run ] ; then
				mysqlSocket=/var/run/mysqld/mysqld.sock
			elif [ -d /var/lib/mysql ] ; then
				echo "svctool: possible wrong MySQL socket selected" 1>&2
				mysqlSocket=/var/lib/mysql/mysql.sock
			else
				mysqlSocket=/run/mysqld/mysqld.sock
			fi
		fi
		if [ -x $mysqlPrefix/sbin/mariadbd ] ; then
			mysqld=sbin/mariadbd
		elif [ -x $mysqlPrefix/libexec/mysqld ] ; then
			mysqld=libexec/mysqld
		elif [ -x $mysqlPrefix/sbin/mysqld ] ; then
			mysqld=sbin/mysqld
		elif [ -x $mysqlPrefix/bin/mysqld ] ; then
			mysqld=bin/mysqld
		else
			echo "mysqld: No such File or directory" 1>&2
			exit 1
		fi
		if [ $nooverwrite -eq 1 ] ; then
			echo "conf_file exists" 1>&2
			exit 1
		fi
		if [ -n "$mysysconfdir" ] ; then
			sysconfdir=$mysysconfdir
		fi
		conf_file="$DESTDIR"$sysconfdir/indimail.cnf
		mysql_version=`$mysqlPrefix/$mysqld --version 2>&1 | grep Ver | awk '{print $3}'`
		echo "Creating MySQL config $conf_file for MySQL $mysql_version prefix=$mysqlPrefix socket=$mysqlSocket port=$mysql_port dbdir=$databasedir"
		echo conf_file=$conf_file
		create_mysql_config > $TMPDIR/config.cnf.$$
		conf_dir=`dirname $conf_file`
		if [ ! -d $conf_dir ] ; then
			mkdir -p $conf_dir
		fi
		change_config $conf_file $TMPDIR/config.cnf.$$ 1
		if [ -n "$mysysconfdir" ] ; then
			do_exit
		fi
		echo $mysql_version |grep MariaDB > /dev/null 2>&1
		if [ $? -eq 0 ] ; then
			mysql_version=`echo $mysql_version|cut -d- -f1`
			mariadb=1
		else
			mariadb=0
		fi
		if [ $mariadb -eq 1 ] ; then
			if [ -d /etc/my.cnf.d ] ; then
				/bin/ln -sf $sysconfdir/indimail.cnf /etc/my.cnf.d/indimail.cnf
				if [ -f /etc/my.cnf ] ; then
					grep '^!includedir /etc/my.cnf.d' /etc/my.cnf > /dev/null 2>&1
					if [ $? -ne 0 ] ; then
						echo '!includedir /etc/my.cnf.d' >> /etc/my.cnf
					fi
				fi
			elif  [ -d /etc/mysql ] ; then
				/bin/ln -sf $sysconfdir/indimail.cnf /etc/mysql/indimail.cnf
				if [ -f /etc/my.cnf ] ; then
					grep '^!includedir /etc/mysql' /etc/my.cnf > /dev/null 2>&1
					if [ $? -ne 0 ] ; then
						echo '!includedir /etc/mysql' >> /etc/my.cnf
					fi
				fi
			else
				/bin/ln -sf $sysconfdir/indimail.cnf /etc/indimail.cnf
			fi
		else
			if [ -d /etc/my.cnf.d -a ! -d /etc/mysql -a ! -h /etc/mysqlf ] ; then
				/bin/ln -s /etc/my.cnf.d /etc/mysql
				/bin/ln -sf $sysconfdir/indimail.cnf /etc/my.cnf.d/indimail.cnf
				if [ -f /etc/my.cnf ] ; then
					grep '^!includedir /etc/my.cnf.d' /etc/my.cnf > /dev/null 2>&1
					if [ $? -ne 0 ] ; then
						echo '!includedir /etc/my.cnf.d' >> /etc/my.cnf
					fi
				fi
			elif  [ -d /etc/mysql ] ; then
				/bin/ln -sf $sysconfdir/indimail.cnf /etc/mysql/indimail.cnf
				if [ -f /etc/my.cnf ] ; then
					grep '^!includedir /etc/mysql' /etc/my.cnf > /dev/null 2>&1
					if [ $? -ne 0 ] ; then
						echo '!includedir /etc/mysql' >> /etc/my.cnf
					fi
				fi
			else
				/bin/ln -sf $sysconfdir/indimail.cnf /etc/indimail.cnf
			fi
		fi
		if [ -d "$DESTDIR"$databasedir/ssl ] ; then
			set_mysql_ssl_permission "$DESTDIR"$databasedir/ssl
		elif [ -d "$DESTDIR"$databasedir/data ] ; then
			set_mysql_ssl_permission "$DESTDIR"$databasedir/data
		fi
		if [ -d /etc/security/limits.d -a ! -f /etc/security/limits.d/51-mysql.conf ] ; then
			echo "mysql hard nproc unlimited" > /etc/security/limits.d/51-mysql.conf
		fi
		;;

		mysqldb)
		if [ " $mysqlPrefix" = " " ] ; then
			echo "MySQL Installation Prefix directory not specified" 1>&2
			usage 1
		elif [ " $databasedir" = " " ] ; then
			echo "Database directory not specified" 1>&2
			usage 1
			exit 1
		elif [ " $base_path" = " " ] ; then
			echo "Base Path not specified" 1>&2
			usage 1
			exit 1
		fi
		if [ -z "$mysql_user" -a -x "$DESTDIR"$QmailBinPrefix/bin/vadddomain ] ; then
			echo "MySQL user not specified. Using default user $MYSQL_USER" 1>&2
			mysql_user=$MYSQL_USER
		fi
		if [ -z "$mysql_pass" -a -x "$DESTDIR"$QmailBinPrefix/bin/vadddomain ] ; then
			echo "MySQL password not specified. Using default password $MYSQL_PASS" 1>&2
			mysql_pass=$MYSQL_PASS
		fi
		if [ -z "$mysqlSocket" ]; then
			if [ -n "$MYSQL_SOCKET" ] ; then
				mysqlSocket=$MYSQL_SOCKET
			elif [ -d /run ] ; then
				mysqlSocket=/run/mysqld/mysqld.sock
			elif [ -d /var/run ] ; then
				mysqlSocket=/var/run/mysqld/mysqld.sock
			elif [ -d /var/lib/mysql ] ; then
				mysqlSocket=/var/lib/mysql/mysql.sock
			else
				mysqlSocket=/run/mysqld/mysqld.sock
			fi
		fi
		create_mysql_db "$mysql_user" "$mysql_pass" "$databasedir" "$mysqlSocket"
		if [ -d "$DESTDIR"$databasedir/ssl ] ; then
			set_mysql_ssl_permission "$DESTDIR"$databasedir/ssl
		elif [ -d "$DESTDIR"$databasedir/data ] ; then
			set_mysql_ssl_permission "$DESTDIR"$databasedir/data
		fi
		;;

		ssl_rsa)
		if [ " $certdir" = " " ] ; then
			echo "Certificate directory not specified" 1>&2
			usage 1
			exit 1
		elif [ " $ca_path" = " " ] ; then
			echo "CA certificate path not specified" 1>&2
			usage 1
			exit 1
		fi
		echo "Creating MariaDB SSL/TLS Certificates"
		mariadb_ssl_rsa_setup "$ca_path" "$DESTDIR"$certdir
		if [ -d "$DESTDIR"$databasedir/ssl ] ; then
			set_mysql_ssl_permission "$DESTDIR"$databasedir/ssl
		elif [ -d "$DESTDIR"$databasedir/data ] ; then
			set_mysql_ssl_permission "$DESTDIR"$databasedir/data
		fi
		;;

		indimail)
		default_indimail_control
		;;

		qmail)
		create_qmail_config # calls default_qmail_control
		;;

		users)
		nscd_up=`ps ax |grep nscd |grep -v grep|wc -l`
		if [ $nscd_up -ge 1 ] ; then
			if [ -x /etc/init.d/nscd ] ; then
				/etc/init.d/nscd stop
			elif [ -f /etc/systemd/system/multi-user.target/nscd.service ] ; then
				/bin/systemctl stop nscd.service
			fi
		fi
		create_users
		status=$?
		if [ $nscd_up -ge 1 ] ; then
			if [ -x /etc/init.d/nscd ] ; then
				/etc/init.d/nscd start
			elif [ -f /etc/systemd/system/multi-user.target/nscd.service ] ; then
				/bin/systemctl start nscd.service
			fi
		fi
		exit $status
		;;

		rmusers)
		delUsers
		;;

		snmpdconf)
		create_snmpd_conf
		;;

		nssd)
		if [ " $mysql_host" = " " ] ; then
			echo "MySQL Host not specified" 1>&2
			usage 1
		elif [ " $mysql_user" = " " ] ; then
			echo "MySQL User not specified" 1>&2
			usage 1
		elif [ " $mysql_pass" = " " ] ; then
			echo "MySQL Passwd not specified" 1>&2
			usage 1
		elif [ -z "$mysql_port" -a -z "$mysqlSocket" ] ; then
			echo "MySQL Port/Socket not specified" 1>&2
			usage 1
		elif [ " $thread_count" = " " ] ; then
			echo "Thread Count not specified" 1>&2
			usage 1
		elif [ " $timeout" = " " ] ; then
			echo "Listen Timeout not specified" 1>&2
			usage 1
		fi
		THREAD_COUNT=$thread_count
		LISTEN_TIMEOUT=$timeout
		create_pwdlookup_conf "$mysql_host" "$mysql_user" "$mysql_pass" "$mysql_port" "$mysqlSocket" > $TMPDIR/config.cnf.$$
		if [ $? -eq 0 ] ; then
			if [ -n "$mysysconfdir" ] ; then
				sysconfdir=$mysysconfdir
			fi
			conf_file=$DESTDIR"$sysconfdir/nssd.conf"
			change_config $conf_file $TMPDIR/config.cnf.$$
			/bin/chmod 640 $conf_file
			$chown root:qmail $conf_file
		fi
		;;

		clamd)
		if [ -n "$mysysconfdir" ] ; then
			sysconfdir=$mysysconfdir
		fi
		if [ ! -d $sysconfdir ] ; then
			/bin/mkdir -p $sysconfdir
		fi
		if [ -f /usr/sbin/clamd ] ; then
			conf_file=$sysconfdir/scan.conf
		else
			echo "/usr/sbin/clamd not found" 1>&2
			exit 1
		fi
		if [ ! -d /etc/clamd.d ] ; then
			if [ ! -f $sysconfdir/scan.conf ] ; then
				create_clamd_conf > $sysconfdir/scan.conf
			fi
			mkdir -p /etc/clamd.d
			if [ $? ne 0 ] ; then
				exit 1
			fi
			/bin/ln -sr $sysconfdir/scan.conf \
				/etc/clamd.d/scan.conf 2>/dev/null
			if [ $? -ne 0 ] ; then
				cd /etc/clamd.d
				/bin/ln -s $sysconfdir/scan.conf
			fi
		elif [ ! -f /etc/clamd.d/scan.conf ] ; then
			if [ ! -f $sysconfdir/scan.conf ] ; then
				create_clamd_conf > $sysconfdir/scan.conf
			fi
			/bin/ln -sr $sysconfdir/scan.conf \
				/etc/clamd.d/scan.conf 2>/dev/null
			if [ $? -ne 0 ] ; then
				cd /etc/clamd.d
				/bin/ln -s $sysconfdir/scan.conf
			fi
		fi
		if [ -f /etc/clamd.d/scan.conf.rpmnew ] ; then
			mv /etc/clamd.d/scan.conf.rpmnew /etc/clamd.d/scan.conf.BAK
			f=/etc/clamd.d/scan.conf.rpmnew
		elif [ -f /etc/clamd.d/scan.conf -a ! -L /etc/clamd.d/scan.conf ] ; then
			mv /etc/clamd.d/scan.conf /etc/clamd.d/scan.conf.BAK
			f=/etc/clamd.d/scan.conf.BAK
		else
			f=""
		fi
		if [ -n "$f" ] ; then
			@prefix@/bin/qcat $f | sed \
				-e 's,#LogClean,LogClean,g' \
				-e 's,#LocalSocket,LocalSocket,g' \
				-e 's,LogSyslog,#LogSyslog,g' \
				-e 's,#DatabaseDirectory.*,DatabaseDirectory /var/indimail/clamd,g' \
				-e 's,LocalSocketGroup.*,LocalSocketGroup qmail,g'  \
				-e 's,#LocalSocketMode,LocalSocketMode,g' \
				-e 's,#FixStaleSocket,FixStaleSocket,g' \
				-e 's,User.*,User qscand,g'  \
				-e 's,#Foreground,Foreground,g' \
				-e '#ConcurrentDatabaseReload,ConcurrentDatabaseReload,g' \
				-e '0,/^#Example$/d' > $TMPDIR/config.cnf.$$
			if [ -s $TMPDIR/config.cnf.$$ ] ; then
				/bin/rm -f $conf_file
				change_config $conf_file $TMPDIR/config.cnf.$$
				/bin/rm -f /etc/clamd.d/scan.conf.rpmnew
			else
				echo "error generating scan.conf" 1>&1
				exit 1
			fi
		fi
		if [ -f /etc/clamd.d/scan.conf -a ! -L /etc/clamd.d/scan.conf ] ; then
			mv /etc/clamd.d/scan.conf /etc/clamd.d/scan.conf.BAK
			if [ -f $sysconfdir/scan.conf ] ; then
				/bin/ln -sr $sysconfdir/scan.conf
					/etc/clamd.d/scan.conf 2>/dev/null
				if [ $? -ne 0 ] ; then
					cd /etc/clamd.d
					/bin/ln -s $sysconfdir/scan.conf
				fi
			fi
		fi
		if [ ! -f /etc/clamd.d/scan.conf ] ; then
			echo "clamd config /etc/clamd.d/scan.conf not found" 1>&2
			exit 1
		fi
		dbd=`grep -w "^DatabaseDirectory" $conf_file | awk '{print $2}'`
		if [ -z "$dbd" ] ; then
			echo "DatabaseDirectory not defined in $conf_file" 1>&2
			exit 1
		fi
		if [ ! -d $dbd ] ; then
			/bin/mkdir -p $dbd
			if [ $? -eq 0 ] ; then
				$chown qscand:qmail $dbd
				if [ $? -ne 0 ] ; then
					exit 1
				fi
			else
				exit 1
			fi
		fi
		if [ -d /etc/tmpfiles.d ] ; then
			if [ -d /run ] ; then
				echo "d /run/clamd.scan 0770 qscand qmail" > $TMPDIR/config.cnf.$$
			elif [ -d /var/run ] ; then
				echo "d /var/run/clamd.scan 0770 qscand qmail" > $TMPDIR/config.cnf.$$
			fi
			if [ -s $TMPDIR/config.cnf.$$ ] ; then
				conf_file="/etc/tmpfiles.d/clamd.scan.conf"
				change_config $conf_file $TMPDIR/config.cnf.$$
			fi
		fi
		if [ -f /usr/bin/freshclam ] ; then
			conf_file=$sysconfdir/freshclam.conf
		else
			echo "/usr/bin/freshclam not found" 1>&2
			exit 1
		fi
		if [ -f /etc/freshclam.conf.rpmnew ] ; then
			mv /etc/freshclam.conf.rpmnew /etc/freshclam.conf.BAK
			f=/etc/freshclam.conf.rpmnew
		elif [ -f /etc/freshclam.conf -a ! -L /etc/freshclam.conf ] ; then
			mv /etc/freshclam.conf /etc/freshclam.conf.BAK
			f=/etc/freshclam.conf.BAK
		else
			f=""
		fi
		if [ -n "$f" ] ; then
			@prefix@/bin/qcat $f | sed \
				-e 's,#LogClean,LogClean,g' \
				-e 's,LogSyslog,#LogSyslog,g' \
				-e 's,#DatabaseDirectory.*,DatabaseDirectory /var/indimail/clamd,g' \
				-e 's,#DatabaseOwner.*,DatabaseOwner qscand,g'  \
				-e 's,#Foreground,Foreground,g' \
				-e '0,/^#Example$/d' > $TMPDIR/config.cnf.$$
			if [ ! -s $TMPDIR/config.cnf.$$ ] ; then
				create_freshclam_conf > $TMPDIR/config.cnf.$$
			fi
			if [ -s $TMPDIR/config.cnf.$$ ] ; then
				/bin/rm -f $conf_file
				change_config $conf_file $TMPDIR/config.cnf.$$ >/dev/null
				/bin/rm -f /etc/freshclam.conf.rpmnew
			else
				echo "error generating $conf_file" 1>&1
				exit 1
			fi
		fi
		if [ ! -f /etc/freshclam.conf ] ; then
			/bin/ln -sr $sysconfdir/freshclam.conf \
				/etc/freshclam.conf 2>/dev/null
			if [ $? -ne 0 ] ; then
				cd /etc
				/bin/ln -s $sysconfdir/freshclam.conf
			fi
		fi
		if [ -f /usr/lib/tmpfiles.d/clamd.scan.conf ] ; then
			echo "d /run/clamd.scan 0750 qscand qmail" > /usr/lib/tmpfiles.d/clamd.scan.conf
		fi
		;;

		foxhole)
		if [ ! -d "$DESTDIR"$sysconfdir ] ; then
			/bin/mkdir -p "$DESTDIR"$sysconfdir
		fi
		if [ -f "$DESTDIR"$QmailBinPrefix/sbin/clamd ] ; then
			conf_file="$DESTDIR"$sysconfdir/scan.conf
		else
			conf_file="$DESTDIR"$sysconfdir/scan.conf.disabled
		fi
		dbd=`grep -w "^DatabaseDirectory" $conf_file | awk '{print $2}'`
		if [ -z "$dbd" ] ; then
			echo "DatabaseDirectory not defined in $conf_file" 1>&2
			exit 1
		fi
		/bin/mkdir -p $dbd
		if [ $? -eq 0 ] ; then
			$chown qscand:qmail $dbd
			if [ $? -ne 0 ] ; then
				exit 1
			fi
		else
			exit 1
		fi
		if [ -f "$DESTDIR"$sysconfdir/foxhole_all.cdb -a -d $dbd ] ; then
			/bin/cp "$DESTDIR"$sysconfdir/foxhole_all.cdb $dbd/foxhole_all.cdb
			$chown qscand:qmail $dbd/foxhole_all.cdb
		fi
		;;

		bogofilter)
		if [ -n "$mysysconfdir" ] ; then
			if [ ! -f $mysysconfdir/bogofilter.cf.example -a -f $sysconfdir/bogofilter.cf.example ] ; then
				if [ ! -d $mysysconfdir ] ; then
					mkdir -p $mysysconfdir
				fi
				/bin/cp -p $sysconfdir/bogofilter.cf.example $mysysconfdir
			fi
			sysconfdir=$mysysconfdir
		fi
		conf_file="$DESTDIR"$sysconfdir/bogofilter.cf
		create_bogofilter_conf > $TMPDIR/config.cnf.$$
		if [ $? -eq 0 ] ; then
			change_config $conf_file $TMPDIR/config.cnf.$$ >/dev/null
		else
			/bin/rm -f $TMPDIR/config.cnf.$$
		fi
		if [ ! -f "$DESTDIR"$sysconfdir/wordlist.db ] ; then
			# create empty wordlist.db
			"$DESTDIR"$QmailBinPrefix/bin/bogoutil -l "$DESTDIR"$sysconfdir/wordlist.db < /dev/null
			$chown indimail:indimail "$DESTDIR"$sysconfdir/wordlist.db
		fi
		;;

		cert)
		create_cert "$postmaster" "$common_name" $valid_for
		;;

		add-boot)
		case "$host" in
			*-*-darwin*)
			if [ -z "$servicedir" ] ; then
				echo "Supervise Directory not specified" 1>&2
				usage 1
			fi
			;;
		esac
		create_startup
		;;

		rm-boot)
		remove_startup
		;;

		add-alt)
		add_alternatives
		;;
		remove-alt)
		remove_alternatives
		;;

		iselinux)
		selinux_module iselinux $sysconfdir/indimail
		;;
		qselinux)
		selinux_module qselinux $sysconfdir/indimail-mta
		;;
		selinux)
		selinux_module selinux $sysconfdir/$module_name
		;;

		inittab)
		if [ " $servicedir" = " " ] ; then
			echo "Supervise Directory not specified" 1>&2
			usage 1
		fi
		grep "svscanboot" /etc/inittab >/dev/null
		if [ $? -ne 0 ]; then
			if [ -f /etc/debian_release ] ; then
				echo "SV:2345:respawn:$libexecdir/svscanboot $servicedir" >> /etc/inittab
			elif [ -f /etc/alpine-release ] ; then
				echo "::respawn:$libexecdir/svscanboot $servicedir" >> /etc/inittab
			else
				echo "SV:345:respawn:$libexecdir/svscanboot $servicedir" >> /etc/inittab
			fi
			if [ $? -eq 0 ] ; then
				kill -1 1
			fi
		else
			grep "svscanboot" /etc/inittab |grep respawn >/dev/null
			if [ $? -ne 0 ]; then
				grep -v "svscanboot" /etc/inittab > /etc/inittab.svctool.$$
				if [ $? -eq 0 ] ; then
					if [ -f /etc/debian_release ] ; then
						echo "SV:345:respawn:$libexecdir/svscanboot $servicedir" >> /etc/inittab.svctool.$$
					elif [ -f /etc/alpine-release ] ; then
						echo "::respawn:$libexecdir/svscanboot $servicedir" >> /etc/inittab
					else
						echo "SV:2345:respawn:$libexecdir/svscanboot $servicedir" >> /etc/inittab.svctool.$$
					fi
				fi
				if [ $? -eq 0 ] ; then
					/bin/mv /etc/inittab.svctool.$$ /etc/inittab
					kill -1 1
				fi
			fi
		fi
		;;

		recontrol)
		do_recontrol
		;;

		*)
		echo "Invalid configuration option $optarg" 1>&2
		usage 1
		;;
	esac
	;;

	11) # check install
	if [ -z "$qbase" ] ; then
		qbase=$indimaildir/queue
	fi
	if [ -z "$qcount" ] ; then
		qcount=5
	fi
	if [ -z "$qstart" ] ; then
		qstart=1
	fi
	if [ -z "$servicedir" ] ; then
		echo "Supervise Directory not specified" 1>&2
		usage 1
	fi
	check_installation "$qbase" "$qcount" "$qstart" "$servicedir" "$verbose"
	;;

	13) # backup - mysqldump, dump_config, tar
	if [ " $servicedir" = " " ] ; then
		echo "Supervise Directory not specified" 1>&2
		usage 1
	elif [ " $mysqlPrefix" = " " ] ; then
		echo "MySQL Installation Prefix directory not specified" 1>&2
		usage 1
	elif [ " $backupdir" = " " ] ; then
		echo "Backup directory not specified" 1>&2
		usage 1
	fi
	if [ ! -d $backupdir ] ; then
		echo "$backupdir: No such file or directory" 1>&2
		exit 1
	fi
	EXT=`date +'%d%m%y-%H%M%S'`
	dump_config "$servicedir" "$cntrldir" > /root/indimail_config.txt
	# take backup of config files
	tar cf - `sort_backup $servicedir` | gzip -c > $backupdir/indimail-backup.$EXT.tar.gz
	/bin/rm -f /root/indimail_config.txt
	if [ -x $mysqlPrefix/bin/mariadb-dump ] ; then
		mariadb=1
		mysqldump=$mysqlPrefix/bin/mariadb-dump
	else
		mariadb=0
		mysqldump=$mysqlPrefix/bin/mysqldump
	fi
	major=$(mysqld --version |awk '{print $3}'|cut -d. -f1)
	if [ -f $HOME/.mylogin.cnf ] ; then
		if [ $mariadb -eq 1 ] ; then
			$mysqldump --login-path=admin --compress --flush-logs \
				--add-drop-table --extended-insert --lock-tables \
				--verbose -A | gzip -c > $backupdir/dbexport.$EXT.gz
		else
			$mysqldump --login-path=admin --compression-algorithms=zlib \
				--add-drop-table --extended-insert --lock-tables \
				--verbose -A | gzip -c > $backupdir/dbexport.$EXT.gz
		fi
	else
		if [ $mariadb -eq 1 ] ; then
			$mysqldump -u mysql -p$PRIV_PASS --compress --flush-logs \
				--add-drop-table --extended-insert --lock-tables \
				--verbose -A | gzip -c > $backupdir/dbexport.$EXT.gz
		else
			$mysqldump -u mysql -p$PRIV_PASS --compression-algorithms=zlib \
				--add-drop-table --extended-insert --lock-tables \
				--verbose -A | gzip -c > $backupdir/dbexport.$EXT.gz
		fi
	fi
	;;

	14) # repair tables
	if [ " $mysqlPrefix" = " " ] ; then
		echo "MySQL Installation Prefix directory not specified" 1>&2
		usage 1
	fi
	repair_tables
	;;

	15) # reports
	report $report_option
	;;

	16) # create_qscanq
	if [ " $servicedir" = " " ] ; then
		echo "Supervise Directory not specified" 1>&2
		usage 1
	fi
	echo "Creating qscanq virus scanner, Service $servicedir"
	create_qscanq $servicedir
	;;

	17) # --clamd
	if [ " $servicedir" = " " ] ; then
		echo "Supervise Directory not specified" 1>&2
		usage 1
	elif [ " $clamdPrefix" = " " ] ; then
		echo "clamd Installation Prefix directory not specified" 1>&2
		usage 1
	fi
	if [ -n "$mysysconfdir" ] ; then
		sysconfdir=$mysysconfdir
	fi
	echo "Creating Virus Scanner Service ClamdPrefix $clamdPrefix, Service $servicedir, Sysconf $sysconfdir"
	create_clamd $clamdPrefix $servicedir
	;;

	18) # create_poppass
	if [ " $poppass_port" = " " ] ; then
		echo "poppass PORT not specified" 1>&2
		usage 1
	elif [ " $servicedir" = " " ] ; then
		echo "Supervise Directory not specified" 1>&2
		usage 1
	elif [ " $password_cmd" = " " ] ; then
		echo "Setpassword Command not specified" 1>&2
		usage 1
	elif [ $poppass_port -lt 1 ] ; then
		echo "POP3 PORT should be a positive number" 1>&2
		usage 1
	elif [ $use_ssl -eq 1 -a " $certfile" = " " ] ; then
		echo "--ssl option given and Certficate not specified" 1>&2
		usage 1
	fi
	echo "Creating poppass Listener Port $poppass_port, Service $servicedir use_ssl=$use_ssl"
	create_poppass "$servicedir" "$poppass_port" "$password_cmd" "$use_ssl"
	;;

	19) # create_svscan
	if [ " $servicedir" = " " ] ; then
		echo "Supervise Directory not specified" 1>&2
		usage 1
	fi
	echo "Creating svscan log Service Service $servicedir"
	create_svscan $servicedir $svscan_init_cmd
	;;

	20) # create_pwdlookup service
	if [ " $servicedir" = " " ] ; then
		echo "Supervise Directory not specified" 1>&2
		usage 1
	elif [ " $socket_path" = " " ] ; then
		echo "socket path not specified" 1>&2
		usage 1
	elif [ " $thread_count" = " " ] ; then
		echo "Thread Count not specified" 1>&2
		usage 1
	elif [ " $timeout" = " " ] ; then
		echo "Listen Timeout not specified" 1>&2
		usage 1
	elif [ " $mysql_host" = " " ] ; then
		echo "MySQL Host not specified" 1>&2
		usage 1
	elif [ " $mysql_user" = " " ] ; then
		echo "MySQL User not specified" 1>&2
		usage 1
	elif [ " $mysql_pass" = " " ] ; then
		echo "MySQL Passwd not specified" 1>&2
		usage 1
	elif [ " $mysql_port" = " " -a -z "$mysqlSocket" ] ; then
		echo "MySQL Port/Socket not specified" 1>&2
		usage 1
	fi
	echo "Creating pwdLookup Daemon on socket $socket_path, thread_count $thread_count"
	echo "  listen_timeout $timeout, Service $servicedir"
	echo "  mysql_user $mysql_user, mysql_pass $mysql_pass"
	if [ -n "$mysql_port" -a -n "$mysqlSocket" ] ; then
		echo "  mysql_port $mysql_port, mysql_socket $mysqlSocket"
	elif [ -n "$mysqlSocket" ] ; then
		echo "  mysql_socket $mysqlSocket"
	elif [ -n "$mysql_port" ] ; then
		echo "  mysql_port $mysql_port"
	fi
	create_pwdlookup "$socket_path" "$thread_count" "$timeout" "$servicedir" \
		"$mysql_host" "$mysql_user" "$mysql_pass" "$mysql_port" "$mysqlSocket"
	;;

	21) # dump config
	if [ " $servicedir" = " " ] ; then
		echo "Supervise Directory not specified" 1>&2
		usage 1
	fi
	dump_config "$servicedir" "$cntrldir"
	;;

	22) # create_greylist
	if [ " $servicedir" = " " ] ; then
		echo "Supervise Directory not specified" 1>&2
		usage 1
	fi
	create_greylist "$servicedir" "$grey_port" "$min_resend_min" " $resend_win_hr" "$timeout_days" "$context_file" "$save_interval" "$hash_size" "$whitelist" "$use_greydaemon"
	;;

	23) # create_qmtp
	if [ " $servicedir" = " " ] ; then
		echo "Supervise Directory not specified" 1>&2
		usage 1
	elif [ " $qmtp_port" = " " ] ; then
		echo "QMTP PORT not specified" 1>&2
		usage 1
	elif [ $qmtp_port -lt 1 ] ; then
		echo "QMTP PORT should be a positive number" 1>&2
		usage 1
	fi
	if [ " $qbase" = " " ] ; then
		echo "Queue Directory not specified" 1>&2
		usage 1
	elif [ " $qcount" = " " ] ; then
		echo "Queue Count not specified" 1>&2
		usage 1
	elif [ " $qstart" = " " ] ; then
		echo "Start Queue Number not specified" 1>&2
		usage 1
	elif [ $qcount -lt 1 ] ; then
		echo "No of Queues should be a positive number" 1>&2
		usage 1
	elif [ $qstart -lt 1 ] ; then
		echo "Start Queue Number should be a positive number" 1>&2
		usage 1
	fi
	echo "Creating QMTP Listener Port $qmtp_port, Service $servicedir, Queue Base $qbase, Queue Count $qcount, Queue Start $qstart"
	create_qmtp_or_qmqp "qmtpd" "$qbase" "$qcount" "$qstart" "$servicedir" "$qmtp_port"
	;;

	24) # create_qmqp
	if [ " $servicedir" = " " ] ; then
		echo "Supervise Directory not specified" 1>&2
		usage 1
	elif [ " $qmqp_port" = " " ] ; then
		echo "QMQP PORT not specified" 1>&2
		usage 1
	elif [ $qmqp_port -lt 1 ] ; then
		echo "QMQP PORT should be a positive number" 1>&2
		usage 1
	fi
	if [ " $qbase" = " " ] ; then
		echo "Queue Directory not specified" 1>&2
		usage 1
	elif [ " $qcount" = " " ] ; then
		echo "Queue Count not specified" 1>&2
		usage 1
	elif [ " $qstart" = " " ] ; then
		echo "Start Queue Number not specified" 1>&2
		usage 1
	elif [ $qcount -lt 1 ] ; then
		echo "No of Queues should be a positive number" 1>&2
		usage 1
	elif [ $qstart -lt 1 ] ; then
		echo "Start Queue Number should be a positive number" 1>&2
		usage 1
	fi
	echo "Creating QMQP Listener Port $qmqp_port, Service $servicedir, Queue Base $qbase, Queue Count $qcount, Queue Start $qstart"
	create_qmtp_or_qmqp "qmqpd" "$qbase" "$qcount" "$qstart" "$servicedir" "$qmqp_port"
	;;

	25) # create_queuedef
	if [ " $queuedef" = " " ] ; then
		echo "queue environment directory not specified" 1>&2
		usage 1
	elif [ " $qbase" = " " ] ; then
		echo "Queue Directory not specified" 1>&2
		usage 1
	elif [ " $qcount" = " " ] ; then
		echo "Queue Count not specified" 1>&2
		usage 1
	elif [ " $qstart" = " " ] ; then
		echo "Start Queue Number not specified" 1>&2
		usage 1
	elif [ $qcount -lt 1 ] ; then
		echo "No of Queues should be a positive number" 1>&2
		usage 1
	elif [ $qstart -lt 1 ] ; then
		echo "Start Queue Number should be a positive number" 1>&2
		usage 1
	fi
	case "$dkverify_option" in
		dkim)
		;;
		none|"")
		;;
		*)
		echo "invalid --dkverify=$dkverify_option" 1>&2
		usage 1
		;;
	esac
	case "$dksign_option" in
		dkim)
		if [ " $private_key" = " " ] ; then
			echo "--private_key not specfied" 1>&2
			usage 1
		fi
		;;
		none|"")
		;;
		*)
		echo "invalid --dksign=$dksign_option" 1>&2
		usage 1
		;;
	esac
	echo "Creating queue envdir $queuedef"
	create_queuedef "$qbase" "$qcount" "$qstart" "$queuedef"
	;;

	26) # create_delivery
	if [ " $servicedir" = " " ] ; then
		echo "Supervise Directory not specified" 1>&2
		usage 1
	elif [ " $queue_ident" = " " ] ; then
		echo "Queue IDENT not specified" 1>&2
		usage 1
	fi
	if [ " $qbase" = " " ] ; then
		echo "Queue Directory not specified" 1>&2
		usage 1
	elif [ " $qcount" = " " ] ; then
		echo "Queue Count not specified" 1>&2
		usage 1
	elif [ " $qstart" = " " ] ; then
		echo "Start Queue Number not specified" 1>&2
		usage 1
	elif [ $qcount -lt 1 ] ; then
		echo "No of Queues should be a positive number" 1>&2
		usage 1
	elif [ $qstart -lt 1 ] ; then
		echo "Start Queue Number should be a positive number" 1>&2
		usage 1
	fi
	case "$dkverify_option" in
		dkim)
		;;
		none|"")
		;;
		*)
		echo "invalid --dkverify=$dkverify_option" 1>&2
		usage 1
		;;
	esac
	case "$dksign_option" in
		dkim)
		if [ " $private_key" = " " ] ; then
			echo "--private_key not specfied" 1>&2
			usage 1
		fi
		;;
		none|"")
		;;
		*)
		echo "invalid --dksign=$dksign_option" 1>&2
		usage 1
		;;
	esac
	echo "Creating delivery only $qcount queues"
	if [ $use_starttls -ne 0 -o $use_ssl -ne 0 ] ; then
		create_delivery "$qbase" "$qcount" "$qstart" "$servicedir" "$queue_ident" "$routes" 1 $utf8
	else
		create_delivery "$qbase" "$qcount" "$qstart" "$servicedir" "$queue_ident" "$routes" 0 $utf8
	fi
	;;

	27) # create_udplogger
	if [ " $servicedir" = " " ] ; then
		echo "Supervise Directory not specified" 1>&2
		usage 1
	elif [ " $udp_port" = " " ] ; then
		echo "udplogger PORT not specified" 1>&2
		usage 1
	fi
	create_udplogger "$udp_port" "$servicedir"
	;;

	28) # create_fifologger
	if [ " $servicedir" = " " ] ; then
		echo "Supervise Directory not specified" 1>&2
		usage 1
	elif [ " $logfilter" = " " ] ; then
		echo "fifo path not specified" 1>&2
		usage 1
	fi
	echo "Creating fifologger $logfilter in $servicedir"
	create_fifologger $logfilter $servicedir
	;;

	29) # create mrtg
	if [ " $servicedir" = " " ] ; then
		echo "Supervise Directory not specified" 1>&2
		usage 1
	elif [ " $htmldir" = " " ] ; then
		echo "your web html dir not specified" 1>&2
		usage 1
	fi
	echo "Creating indimail mrtg $hmtldir in $servicedir"
	create_mrtg $htmldir $servicedir
	;;

	30) # tls certificate check
	if [ -z "$cert_file" ] ; then
		tls_cert_check
	else
		tls_cert_check $cert_file
	fi
	;;

	31) # enable service
	shift
	enable_service $*
	;;

	32) # disable service
	shift
	disable_service $*
	;;

	33) # create dane
	if [ " $servicedir" = " " ] ; then
		echo "Supervise Directory not specified" 1>&2
		usage 1
	fi
	create_dane "$servicedir" "$dane_port" "$timeout_days" "$context_file" "$save_interval" "$hash_size" "$whitelist"
	;;

	34) # refresh services
	if [ " $servicedir" = " " ] ; then
		echo "Supervise Directory not specified" 1>&2
		usage 1
	fi
	refreshsvc $service_name
	;;

	35) # set norefresh service flag
	if [ " $servicedir" = " " ] ; then
		echo "Supervise Directory not specified" 1>&2
		usage 1
	fi
	autorefresh $service_name
	;;

	36|37|38|39) # set, modify, unset variables
	if [ -z "$servicedir" ] ; then
		echo "Supervise Directory not specified" 1>&2
		usage 1
	fi
	if [ -z "$service_name" ] ; then
		echo "service name not specified" 1>&2
		usage 1
	fi
	if [ -z "$variable_name" ] ; then
		echo "variable name not specified" 1>&2
		usage 1
	fi
	if [ ! -d $servicedir/$service_name ] ; then
		echo "$servicedir/$service_name: No such file or directory" 1>&2
		exit 1
	fi
	case $option in
		36) #set variable
		if [ $force -ne 1 -a -f $servicedir/$service_name/variables/$variable_name ] ; then
			echo "$variable_name exists. Use --force to overwrite" 1>&2
			usage 1
		fi
		;;
		37) # modify variable
		if [ $force -ne 1 -a ! -f $servicedir/$service_name/variables/$variable_name ] ; then
			echo "$variable_name does not exists. Use --force to create" 1>&2
			usage 1
		fi
		;;
		38) # unset variable
		echo "unsetting env variable $variable_name"
		> $servicedir/$service_name/variables/$variable_name
		;;
		39) # delete variable
		echo "removing env variable $variable_name"
		/bin/rm -f $servicedir/$service_name/variables/$variable_name
		;;
	esac
	if [ -z "$owner" ] ; then
		owner=root
	fi
	if [ -z "$group" ] ; then
		group=0
	fi
	if [ -z $mode ] ; then
		mode=0640
	fi
	dir=$servicedir/$service_name
	if [ $option -eq 36 -o $option -eq 37 ] ; then
		if [ -n "$variable_value" ] ; then
			echo "setting env variable $variable_name=[$variable_value]"
			echo $variable_value > $dir/variables/$variable_name
		else
			echo "setting env variable $variable_name"
			echo > $dir/variables/$variable_name
		fi
	fi
	old=$force
	force=1
	if [ $silent -eq 0 ] ; then
		printf "export %-35s %s %s %s\n" "`basename $dir`" $owner $group $mode
	fi
	export_variables $dir/variables $dir/variables/.variables $owner $group $mode
	force=$old
	if [ $option -ne 39 ] ; then
		dir=$servicedir/$service_name
		if [ -n "$mode" ] ; then
			/bin/chmod $mode $dir/variables/$variable_name
		fi
		if [ -n "$owner" -a -n "$group" ] ; then
			$chown $owner:$group $dir/variables/$variable_name
		elif [ -n "$owner" ] ; then
			$chown $owner $dir/variables/$variable_name
		fi
	fi
	;;

	40|41|42) # restore variables, import variables, export variables
	if [ -z "$servicedir" ] ; then
		echo "Supervise Directory not specified" 1>&2
		usage 1
	fi
	if [ -z "$service_name" ] ; then
		echo "service name not specified" 1>&2
		usage 1
	fi
	if [ ! -d $servicedir/$service_name ] ; then
		echo "$servicedir/$service_name: No such file or directory" 1>&2
		exit 1
	fi
	if [ -z "$owner" ] ; then
		owner=root
	fi
	if [ -z "$group" ] ; then
		group=0
	fi
	if [ -z $mode ] ; then
		mode=0640
	fi
	if [ $option -eq 40 ] ; then
		file=$servicedir/$service_name/variables/.variables
		if [ ! -f $file ] ; then
			echo "env file for $service_name (.variables) doesn't exist" 1>&2
			usage 1
		fi
	elif [ $option -eq 41 ] ; then
		file=$env_file
		if [ ! -f $file ] ; then
			echo "$file: No such file or directory" 1>&2
			usage 1
		fi
	else
		file=$(basename $env_file)
	fi
	if [ $option -eq 40 ] ; then   # restore variables to original state
		import_variables $servicedir/$service_name/variables $file 1 $owner $group $mode
	elif [ $option -eq 41 ] ; then # import new variables
		import_variables $servicedir/$service_name/variables $file 0 $owner $group $mode
	else
		dir="$servicedir/$service_name"
		if [ $silent -eq 0 ] ; then
			printf "export %-35s %s %s %s\n" "$dir" $owner $group $mode
		fi
		if [ ! "$dir/variables/$file" = "$env_file" ] ; then
			old=$force
			force=1
			export_variables $dir/variables $env_file $owner $group $mode
			force=$old
		else
			export_variables $dir/variables $dir/variables/$file $owner $group $mode
		fi
	fi
	;;

	43)
	if [ -z "$servicedir" ] ; then
		echo "Supervise Directory not specified" 1>&2
		usage 1
	fi
	save_all_variables
	;;
	44)
	if [ -z "$servicedir" ] ; then
		echo "Supervise Directory not specified" 1>&2
		usage 1
	fi
	restore_all_variables
	;;
	45)
	if [ -z "$servicedir" ] ; then
		echo "Supervise Directory not specified" 1>&2
		usage 1
	fi
	if [ -n "$service_name" -a -n "$envdir" ] ; then
		echo "you cannot specify both service name and envdir both" 1>&2
		exit 1
	fi
	if [ -z "$service_name" -a -z "$envdir" ] ; then
		echo "service name or envdir has to be specified" 1>&2
		usage 1
	fi
	if [ -n "$service_name" -a ! -d $servicedir/$service_name ] ; then
		echo "$servicedir/$service_name: No such file or directory" 1>&2
		exit 1
	fi
	if [ -n "$envdir" -a ! -d "$envdir" ] ; then
		echo "$envdir: No such directory" 1>&2
		exit 1
	fi
	if [ -n "$service_name" ] ; then
		i=$servicedir/$service_name/variables
	elif [ -n "$envdir" ] ; then
		i=$envdir
	fi
	if [ ! -d $i ] ; then
		(
		echo "------ $servicedir/$service_name ------------"
		echo "NOTE: This service has no configured variables"
		) 1>&2
		exit 1
	fi
	if [ -n "$service_name" ] ; then
		echo "------ environment variables for $servicedir/$service_name ------------"
	elif [ -n "$envdir" ] ; then
		echo "------ environment variables for $envdir ------------"
	fi
	envdir -c $i env
	echo
	if [ -L $i/.envdir ] ; then
		link_name=$(readlink $i/.envdir)
		echo "NOTE: additional variables were obtained from $link_name"
		echo
	elif [ -f $i/.envdir ] ; then
		echo "NOTE: additional variables were obtained from .envdir"
		@prefix@/bin/qcat $i/.envdir
		echo
	fi
	if [ -f $i/.envfile ] ; then
		echo "NOTE: additional variables were obtained from .envfile"
		@prefix@/bin/qcat $i/.envfile
		echo
	fi
	;;

	46)
	if [ -z "$servicedir" ] ; then
		echo "Supervise Directory not specified" 1>&2
		usage 1
	fi
	print_all_variables
	;;

	47) # create_delivery
	if [ " $servicedir" = " " ] ; then
		echo "Supervise Directory not specified" 1>&2
		usage 1
	fi
	if [ " $qbase" = " " ] ; then
		echo "Queue Directory not specified" 1>&2
		usage 1
	fi
	case "$dkverify_option" in
		dkim)
		;;
		none|"")
		;;
		*)
		echo "invalid --dkverify=$dkverify_option" 1>&2
		usage 1
		;;
	esac
	case "$dksign_option" in
		dkim)
		if [ " $private_key" = " " ] ; then
			echo "--private_key not specfied" 1>&2
			usage 1
		fi
		;;
		none|"")
		;;
		*)
		echo "invalid --dksign=$dksign_option" 1>&2
		usage 1
		;;
	esac
	echo "Creating delivery only slowq"
	if [ $use_starttls -ne 0 -o $use_ssl -ne 0 ] ; then
		create_slowq "$qbase" "$servicedir" "$routes" 1 $utf8
	else
		create_slowq "$qbase" "$servicedir" "$routes" 0 $utf8
	fi
	;;

	*)
	if [ " $option" = " " ] ; then
		echo "No Options Provided" 1>&2
		read key
	else
		echo "Invalid Option [$option]" 1>&2
	fi
	;;
esac
do_exit
exit 0
