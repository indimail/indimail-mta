#!/bin/sh
#
#
mail_path=/home/mail
qcount=5
qbase=@indimaildir@/queue
tcpserver_plugin=0
add_boot=0
check_install=0
servicedir=@servicedir@
indimaildir=@indimaildir@
prefix=@prefix@
sysconfdir=@sysconfdir@
controldir=@sysconfdir@/control
libexecdir=@libexecdir@
mysql_user="indimail"
mysql_pass="ssh-1.5-"
chown=$(which chown)
mkdir=$(which mkdir)
cp=$(which cp)

#
# End USER Configuration OPTIONS
#

# $Id: create_services.sh,v 2.131 2024-02-22 01:04:24+05:30 Cprogrammer Exp mbhangui $

usage()
{
  echo "create_service [--servicedir=path] [--qbase=path] [--mbase=path] --mysqlPrefix=path \\
  destdir=path] [--shared-objects] [--add-boot] [--chk-install] [--nomysql] [--help]

  servicedir     - path of Supervise service directory
  qbase          - path for createing IndiMail's queue collection
  mbase          - path where user's home maildir will be created
  mysqlPrefix    - Prefix for MySQL installation
  destdir        - Staging directory for creating service
  shared-objects - Enable loading of shared objects in tcpserver for SMTP
  add-boot       - Enable indimail/indimail-mta in system startup
  chk-install    - Check indimail/indimail-mta installation
  nomysql        - Do not configure MySQL service
"
  if [ $1 -ne 0 ] ; then
    echo "Press ENTER or Cntrl C to quit"
    read key
  fi
  exit 1
}

create_ssl_cnf()
{
if [ $# -ne 3 ] ; then
  echo "USAGE: create_ssl_cnf dir postmaster cn" 1>&2
  return 1
fi
dir=$1
postmaster=$2
cn=$3
echo
echo "RANDFILE = "$dir"$sysconfdir/certs/servercert.rand"
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

noproxy=0
nocourierimap=0
nofetchmail=0
nobogofilter=0
noindimail=0
nonssd=0
nomysql=0
nodksignatures=0
destdir_given=0
dkimkeyfn=default
if [ -x /usr/sbin/mysqld -o -x /usr/bin/mysqld ] ; then # Linux
  mysql_base=/usr
elif [ -x /usr/local/libexec/mysqld ] ; then # FreeBSD
  mysql_base=/usr/local
elif [ -x /usr/local/mysql/bin/mysqld ] ; then # Darwin
  mysql_base=/usr/local/mysql
else
  nomysql=1
fi

ID=$(id -u)
default_domain=$(echo $([ -n "$HOSTNAME" ] && echo "$HOSTNAME" || uname -n) | sed 's/^\([^\.]*\)\.\([^\.]*\)\./\2\./')
while test $# -gt 0; do
    case "$1" in
    -*=*) optarg=`echo "$1" | sed 's/[-_a-zA-Z0-9]*=//'` ;;
    *) optarg= ;;
    esac

    case "$1" in
    --servicedir=*)
    servicedir=$optarg
    ;;

    --qcount=*)
    qcount=$optarg
    ;;

    --qbase=*)
    qbase=$optarg
    ;;

    --mbase=*)
    mail_path=$optarg
    ;;

    --mysqlPrefix=*)
    mysql_base=$optarg
    ;;

    --no-courier-imap)
    nocourierimap=1
    ;;

    --no-proxy)
    noproxy=1
    ;;

    --no-bogofilter)
    nobogofilter=1
    ;;

    --nomysql)
    nomysql=1
    nonssd=1
    ;;

    --no-nssd)
    nonssd=1
    ;;

    --no-fetchmail)
    nofetchmail=1
    ;;

    --chk-install)
    check_install=1
    ;;

    --shared-objects)
    tcpserver_plugin=1
    ;;

    --add-boot)
    add_boot=1
    ;;

    --destdir=*)
    extra_opt="--destdir=$optarg"
    dir_prefix=$optarg
    destdir_given=1
    ;;
    --help)
    usage 0
    ;;

    *)
    echo "invalid option [$1]"
    read key
    usage 0
    ;;
    esac

    shift
done

if [ $destdir_given -eq 0 -a $ID -ne 0 ] ; then
  echo "create_services is not meant to be run by mere mortals. Use sudo $0 to get superpowers"
  exit 100
fi

if [ ! -x $dir_prefix$prefix/bin/vadddomain -a ! -x $prefix/bin/vadddomain ] ; then
  noindimail=1
  nonssd=1
fi
if [ $noindimail -eq 0 ] ; then
  if [ $nomysql -eq 1 ] ; then
    if [ " $qbase" = " " -o " $mail_path" = " " ] ; then
      usage 1
    fi
  else
    if [ " $qbase" = " " -o " $mail_path" = " " -o " $mysql_base" = " " ] ; then
      usage 1
    fi
  fi
else
  if [ " $qbase" = " " ] ; then
    usage 1
  fi
fi

if [ -x $dir_prefix$prefix/sbin/svctool ] ; then
  svctool=$dir_prefix$prefix/sbin/svctool
elif [ -x /usr/sbin/svctool ] ; then
  svctool=/usr/sbin/svctool
elif [ -x /usr/local/sbin/svctool ] ; then
  svctool=/usr/local/sbin/svctool
else
  echo "Unable to locate svctool" 1>&2
  exit 1
fi
# some setup
$mkdir -p $dir_prefix$prefix/bin
$mkdir -p $dir_prefix$prefix/sbin
$mkdir -p $dir_prefix$indimaildir
$mkdir -p $dir_prefix$controldir
$mkdir -p $dir_prefix$sysconfdir/users
$mkdir -p $dir_prefix$sysconfdir/control
$mkdir -p $dir_prefix$libexecdir
$mkdir -p $dir_prefix$servicedir
if [ ! -x $dir_prefix$libexecdir/config-fast -a -x $libexecdir/config-fast ] ; then
  $cp $libexecdir/config-fast $dir_prefix$libexecdir
fi
if [ ! -x $dir_prefix$libexecdir/ipmeprint -a -x $libexecdir/ipmeprint ] ; then
  $cp $libexecdir/ipmeprint $dir_prefix$libexecdir
fi
if [ ! -x $dir_prefix$libexecdir/update_tmprsadh -a -x $libexecdir/update_tmprsadh ] ; then
  $cp $libexecdir/update_tmprsadh $dir_prefix$libexecdir
fi
if [ ! -x $dir_prefix$prefix/bin/queue-fix -a -x $prefix/bin/queue-fix ] ; then
  $cp $prefix/bin/queue-fix $dir_prefix$prefix/bin/queue-fix
fi
if [ ! -x $dir_prefix$prefix/bin/tcprules -a -x $prefix/bin/tcprules ] ; then
  $cp $prefix/bin/tcprules $dir_prefix$prefix/bin/tcprules
fi
if [ ! -x $dir_prefix$prefix/bin/svc -a -x $prefix/bin/svc ] ; then
  $cp $prefix/bin/svc $dir_prefix$prefix/bin/svc
fi
if [ ! -x $dir_prefix$prefix/bin/bogoutil -a -x $prefix/bin/bogoutil ] ; then
  $cp $prefix/bin/bogoutil $dir_prefix$prefix/bin/bogoutil
fi
if [ ! -f $dir_prefix$sysconfdir/bogofilter.cf.example ] ; then
  if [ -f $sysconfdir/bogofilter.cf.example ] ; then
    $cp $sysconfdir/bogofilter.cf.example $dir_prefix$sysconfdir/bogofilter.cf.example
  fi
fi
if [ ! -f $dir_prefix$sysconfdir/users/assign ] ; then
  echo "." > $dir_prefix$sysconfdir/users/assign
fi
######################

if [ ! -x $dir_prefix$prefix/bin/fetchmail -a ! -x $prefix/bin/fetchmail ] ; then
  nofetchmail=1
fi
if [ ! -x $dir_prefix$prefix/bin/bogofilter -a ! -x $prefix/bin/bogofilter ] ; then
  nobogofilter=1
fi
if [ $nomysql -eq 0 ] ; then
  if [ ! -x $dir_prefix$prefix/sbin/nssd -a ! -x $prefix/sbin/nssd ] ; then
    nonssd=1
  fi
fi
if [ ! -x $dir_prefix$prefix/bin/imapd -a ! -x $prefix/bin/imapd ] ; then
  nocourierimap=1
fi
if [ ! -x $dir_prefix$libexecdir/imapmodules/authindi -a ! -x $libexecdir/imapmodues/authindi ] ; then
  noproxy=1
fi
#
# create users
#
if [ $ID -eq 0 ] ; then
  if [ $noindimail -eq 0 ] ; then
    echo "Creating indimail Users"
  else
    echo "Creating indimail-mta Users"
  fi
  [ $ID -eq 0 ] && $svctool --config=users $extra_opt
fi
#
# create qmail config
[ $ID -eq 0 ] && $svctool --config=qmail --postmaster=$indimaildir/alias/Maildir/ \
  --default-domain=$default_domain $extra_opt
# rebuild cdb
for i in smtp qmtp qmqp
do
  for j in `/bin/ls $sysconfdir/tcp/tcp*.$i 2>/dev/null`
  do
    t1=`date -r $j +'%s'`
    if [ -f $j.cdb ] ; then
      t2=`date -r $j.cdb +'%s'`
    else
      t2=0
    fi
    if [ $t1 -gt $t2 ] ; then
      echo "Creating CDB $j.cdb"
      $dir_prefix"$prefix"/bin/tcprules $j.cdb $j.tmp < $j && /bin/chmod 664 $j.cdb \
        && [ $ID -eq 0 ] && chown indimail:indimail $j.cdb
    fi
  done
done

echo "Creating @logdir@"
if [ ! -d $dir_prefix"@logdir@" ] ; then
  $mkdir -p $dir_prefix"@logdir@"
fi
[ $ID -eq 0 ] && $chown -R qmaill:nofiles @logdir@

#
# MySQL
#
if [ $ID -eq 0 -a $nomysql -eq 0 ] ; then
  echo "Attempting to Stop & Disable MySQL service"
  if [ -f /usr/local/etc/rc.d/mysql-server ] ; then
    if [ -f /var/db/mysql/FreeBSD.org.pid ] ; then
      /usr/local/etc/rc.d/mysql-server stop
    fi
    service mysql-server enabled
    if [ $? -eq 0 ] ; then
      service mysql-server disable
    fi
  elif [ -f /Library/LaunchDaemons/com.oracle.oss.mysql.mysqld.plist ] ; then
    launchctl unload -w /Library/LaunchDaemons/com.oracle.oss.mysql.mysqld.plist 2>/dev/null
    launchctl remove /Library/LaunchDaemons/com.oracle.oss.mysql.mysqld.plist
  elif [ -x /bin/systemctl -a -f /lib/systemd/system/mysqld.service ] ; then
    /bin/systemctl stop mysqld.service
    /bin/systemctl disable mysqld.service
  elif [ -x /bin/systemctl -a -f /lib/systemd/system/mariadb.service ] ; then
    /bin/systemctl stop mariadb.service
    /bin/systemctl disable mariadb.service
  elif [ -f /etc/init.d/mysqld ] ; then
    /etc/init.d/mysqld stop
    if [ -x /sbin/chkconfig ] ; then
      chkconfig mysqld off
    elif [ -x /sbin/update-rc.d ] ; then
      update-rc.d -f mysqld disable
    elif [ -f /sbin/rc-update ] ; then
      /sbin/rc-update del mysqld
    #else bhagwan bharose
    fi
  elif [ -f /etc/init.d/mysql ] ; then
    /etc/init.d/mysql stop
    if [ -x /sbin/chkconfig ] ; then
      chkconfig mysql off
    elif [ -x /sbin/update-rc.d ] ; then
      update-rc.d -f mysql disable
    elif [ -f /sbin/rc-update ] ; then
      /sbin/rc-update del mysql
    #else bhagwan bharose
    fi
  else
    echo "Couldn't automatically stop mysqld process" 1>&2
  fi
  ps_list=`ps ax|grep mysqld|grep -v grep`
  if [ -n "$count" ] ; then
    echo "mysqld process is running. Please stop before resuming this script" 1>&2
    exit 0
  fi
  [ $ID -eq 0 ] && $svctool --config=mysql   --mysqlPrefix=$mysql_base \
      --databasedir=$indimaildir/mysqldb $extra_opt
  if [ $noindimail -eq 1 ] ; then
    mysql_user=""
    mysql_pass=""
  fi
  [ $ID -eq 0 ] && $svctool --config=mysqldb --mysqlPrefix=$mysql_base \
    --mysqluser="$mysql_user" --mysqlpass="$mysql_pass" \
    --databasedir=$indimaildir/mysqldb --base_path=$mail_path \
    $extra_opt
  [ $ID -eq 0 ] && $svctool --mysql=3306 --servicedir=$servicedir \
    --mysqlPrefix=$mysql_base --databasedir=$indimaildir/mysqldb \
    --config=$sysconfdir/indimail.cnf $extra_opt
fi
if [ $noindimail -eq 0 ] ; then
  if [ -n "$MYSQL_SOCKET" ] ; then
    mysqlSocket=$MYSQL_SOCKET
  elif [ -d /run ] ; then
    mysqlSocket=/run/mysqld/mysqld.sock
  elif [ -d /var/run ] ; then
    mysqlSocket=/var/run/mysqld/mysqld.sock
  elif [ -d /var/lib/mysql ] ; then
    mysqlSocket=/var/mysql/mysql.sock
  else
    mysqlSocket=/run/mysqld/mysqld.sock
  fi
  $prefix/sbin/ischema -u
  [ $ID -eq 0 ] && $svctool --config=indimail --mysqlhost=localhost \
    --mysqluser=$mysql_user --mysqlpass=$mysql_pass --mysqlsocket=$mysqlSocket $extra_opt
fi

if [ $nodksignatures -eq 0 ] ; then
  if [ -x $dir_prefix$prefix/bin/dknewkey ] ; then
    ver_opt="dkim"
    sign_opt="dkim"
    mkdir -p "$dir_prefix"$controldir/domainkeys
    $dir_prefix$prefix/bin/dknewkey -b 1024 $dkimkeyfn
  else
    ver_opt="none"
    sign_opt="none"
  fi
else
  ver_opt="none"
  sign_opt="none"
fi

a_arch=`uname -m`
if [ " $a_arch" = " x86_64" -o " $a_arch" = " amd64" ] ; then
  fetchmail_mem=144857600
  smtp_soft_mem=104857600
  qmtp_soft_mem=104857600
  qmqp_soft_mem=104857600
  send_soft_mem=104857600
  imap_pop3_mem=524288000
  imapspop3_mem=524288000
  poppass_mem=104857600
else
  fetchmail_mem=72428800
  smtp_soft_mem=52428800
  qmtp_soft_mem=52428800
  qmqp_soft_mem=52428800
  send_soft_mem=52428800
  imap_pop3_mem=52428800
  imapspop3_mem=52428800
  poppass_mem=52428800
fi
if [ -d /run ] ; then
  logfifo="/run/indimail/logfifo"
  mkdir -p /run/indimail
elif [ -d /var/run ] ; then
  logfifo="/var/run/indimail/logfifo"
  mkdir -p /var/run/indimail
else
  logfifo="/tmp/logfifo"
fi
#
# SMTP
#
orig_bogofilter=$nobogofilter

if [ -f /usr/sbin/clamd -a -f /usr/bin/clamdscan ] ; then
  clamav_os=1
  clamdPrefix="/usr"
  if [ -d /etc/clamav ] ; then
    mysysconfdir=/etc/clamav
  elif [ -d /etc/clamd.d ] ; then
    mysysconfdir=/etc/clamd.d
  elif [ -d $sysconfdir ] ; then
    mysysconfdir=$sysconfdir
  else
    mysysconfdir=/etc
  fi
  qhpsi="/usr/bin/clamdscan %s --config=$mysysconfdir/scan.conf --fdpass --quiet --no-summary"
else
  clamav_os=0
fi
for port in 465 25 587
do
  if [ $port -eq 465 ] ; then
    e_opt="--skipsend --authsmtp --ssl --utf8"
    e_opt="$e_opt --rbl=-rzen.spamhaus.org --rbl=-rdnsbl-1.uceprotect.net"
  elif [ $port -eq 587 ] ; then
    nobogofilter=1
    e_opt="--skipsend --forceauthsmtp --antispoof --forcetls --utf8 --secureauth"
  else
    e_opt="--remote-authsmtp=plain --localfilter --remotefilter"
    e_opt="$e_opt --deliverylimit-count=-1 --deliverylimit-size=-1"
    e_opt="$e_opt --rbl=-rzen.spamhaus.org --rbl=-rdnsbl-1.uceprotect.net"
    e_opt="$e_opt --dmemory=$send_soft_mem --setgroups --utf8 --setuser-priv"
  fi
  if [ $tcpserver_plugin -eq 1 ] ; then
    e_opt="$e_opt --shared-objects=1 --use-dlmopen=1"
  fi
  append_opt=""
  if [ $nobogofilter -eq 0 ] ; then
    append_opt="--spamfilter=\"$prefix/bin/bogofilter -p -d $sysconfdir\"" 
    append_opt="$append_opt --logfilter=$logfifo --rejectspam=0 --spamexitcode=0"
  fi
  if [ $clamav_os -eq 1 ] ; then
    append_opt="$append_opt --qhpsi=\"$qhpsi\""
  fi
  [ $ID -eq 0 ] && eval $svctool --smtp=$port --servicedir=$servicedir \
    --qbase=$qbase --qcount=$qcount --qstart=1 \
    --query-cache --dnscheck --password-cache \
    --cntrldir=control --localip=0 --maxdaemons=75 --maxperip=25 --persistdb \
    --starttls --memory=$smtp_soft_mem --chkrecipient --chkrelay --masquerade \
    --min-free=52428800 --content-filter --virus-filter \
    --dmasquerade --infifo=infifo \
    --dkverify=$ver_opt \
    --dksign=$sign_opt --private_key=$controldir/domainkeys/%/$dkimkeyfn \
    $append_opt $e_opt $extra_opt
  nobogofilter=$orig_bogofilter
done

[ $ID -eq 0 ] && $svctool --smtp=366 --odmr --servicedir=$servicedir --skipsend \
  --forceauthsmtp --starttls --query-cache --password-cache \
  --infifo=infifo --memory=$smtp_soft_mem $extra_opt

append_opt=""
if [ $clamav_os -eq 1 ] ; then
  append_opt="--qhpsi=\"$qhpsi\""
fi
[ $ID -eq 0 ] && eval $svctool --queueParam=defaultqueue \
  --qbase=$qbase --qcount=$qcount --qstart=1 \
  --min-free=52428800 --virus-filter \
  --dkverify="none" --dksign="none" \
  $append_opt $extra_opt

[ $ID -eq 0 ] && eval $svctool --slowq --servicedir=$servicedir --qbase=$qbase \
  --cntrldir=control --todo-proc --persistdb --starttls \
  --dmemory=$send_soft_mem --min-free=52428800 --dkverify=$ver_opt \
  --dksign=$sign_opt --private_key=$controldir/domainkeys/%/$dkimkeyfn \
  --remote-authsmtp=plain --localfilter --remotefilter \
  --deliverylimit-count="-1" --deliverylimit-size="-1" --setgroups --utf8 --setuser-priv
#
# Greylist
#
[ $ID -eq 0 ] && $svctool --greylist=1999 --servicedir=$servicedir --min-resend-min=2 \
  --resend-win-hr=24 --timeout-days=30 --context-file=greylist.context \
  --hash-size=65536 --save-interval=5 --whitelist=greylist.white $extra_opt

# qmail-dane tlsa daemon
[ $ID -eq 0 ] && $svctool --tlsa=1998 --servicedir=$servicedir --timeout-days=30 \
  --context-file=tlsa.context --hash-size=65536 --save-interval=5 \
  --whitelist=tlsa.white $extra_opt
#
# qmtpd service
#
[ $ID -eq 0 ] && $svctool --qmtp=209 --servicedir=$servicedir \
  --qbase=$qbase --qcount=$qcount --qstart=1 \
  --cntrldir=control --localip=0 --maxdaemons=75 --maxperip=25 \
  --memory=$qmtp_soft_mem --min-free=52428800 $extra_opt
#
# qmqpd service
#
[ $ID -eq 0 ] && $svctool --qmqp=628 --servicedir=$servicedir \
  --qbase=$qbase --qcount=$qcount --qstart=1 \
  --cntrldir=control --localip=0 --maxdaemons=75 --maxperip=25 \
  --memory=$qmqp_soft_mem --min-free=52428800 $extra_opt

#
# fetchmail
#
if [ $nofetchmail -eq 0 ] ; then
  append_opt=""
  if [ $nobogofilter -eq 0 ] ; then
    append_opt="--spamfilter=\"$prefix/bin/bogofilter -p -d $sysconfdir\"" 
    append_opt="$append_opt --logfilter=$logfifo --rejectspam=0 --spamexitcode=0"
  fi
  if [ $clamav_os -eq 1 ] ; then
    append_opt="$append_opt --qhpsi=\"$qhpsi\""
  fi
  [ $ID -eq 0 ] && eval $svctool --fetchmail --servicedir=$servicedir \
    --qbase=$qbase --qcount=$qcount --qstart=1 \
    --cntrldir=control --memory=$fetchmail_mem --fsync \
    --syncdir --dkverify=$ver_opt \
    $append_opt $extra_opt
  [ $ID -eq 0 ] && touch $dir_prefix$servicedir/fetchmail/down
fi

#
# IMAP/POP3
#
if [ $nocourierimap -eq 0 ] ; then
  [ $ID -eq 0 ] && $svctool --imap=143 --servicedir=$servicedir \
    --starttls --localip=0 --maxdaemons=40 --maxperip=25 --infifo=infifo \
    --query-cache --memory=$imap_pop3_mem $extra_opt
  [ $ID -eq 0 ] && $svctool --imap=993 --servicedir=$servicedir --localip=0 \
    --maxdaemons=40 --maxperip=25 --infifo=infifo \
    --query-cache --memory=$imapspop3_mem --ssl $extra_opt
  [ $ID -eq 0 ] && $svctool --pop3=110 --servicedir=$servicedir --localip=0 \
    --starttls --maxdaemons=40 --maxperip=25 --infifo=infifo \
    --query-cache --memory=$imap_pop3_mem $extra_opt
  [ $ID -eq 0 ] && $svctool --pop3=995 --servicedir=$servicedir --localip=0 \
    --maxdaemons=40 --maxperip=25 --infifo=infifo \
    --query-cache --memory=$imapspop3_mem --ssl $extra_opt
fi
if [ $noproxy -eq 0 ] ; then
  [ $ID -eq 0 ] && $svctool --imap=4143 --servicedir=$servicedir --localip=0 --infifo=infifo \
    --maxdaemons=40 --maxperip=25 --query-cache \
    --memory=$imap_pop3_mem --proxy=143 --starttls --tlsprog=$prefix/bin/sslerator \
    $extra_opt
  [ $ID -eq 0 ] && $svctool --imap=9143 --servicedir=$servicedir --localip=0 --infifo=infifo \
    --maxdaemons=40 --maxperip=25 --query-cache \
    --memory=$imapspop3_mem --proxy=143 --ssl $extra_opt
  [ $ID -eq 0 ] && $svctool --pop3=4110 --servicedir=$servicedir --localip=0 --infifo=infifo \
    --maxdaemons=40 --maxperip=25 --query-cache \
    --memory=$imap_pop3_mem --proxy=110 --starttls --tlsprog=$prefix/bin/sslerator \
    $extra_opt
  [ $ID -eq 0 ] && $svctool --pop3=9110 --servicedir=$servicedir --localip=0 --infifo=infifo \
    --maxdaemons=40 --maxperip=25 --query-cache \
    --memory=$imapspop3_mem --proxy=110 --ssl $extra_opt
fi

#
# IndiMail Daemons
#
if [ $noindimail -eq 0 ] ; then
  [ $ID -eq 0 ] && $svctool --indisrvr=4000 --servicedir=$servicedir \
    --localip=0 --maxdaemons=40 --maxperip=25 --avguserquota=2097152 \
    --certfile=$sysconfdir/certs/servercert.pem --ssl \
    --hardquota=52428800 --base_path=$mail_path $extra_opt
  [ $ID -eq 0 ] && $svctool --inlookup=infifo --servicedir=$servicedir --cntrldir=control --threads=5 \
    --activeDays=60 --query-cache --password-cache --use-btree \
    --max-btree-count=10000 $extra_opt
  
  if [ $nonssd -eq 0 -a $nomysql -eq 0 ] ; then
    if [ -d /run ] ; then
      nssd_sock=/run/indimail/nssd.sock
    elif [ -d /var/run ] ; then
      nssd_sock=/var/run/indimail/nssd.sock
    else
      nssd_sock=/tmp/nssd.sock
    fi
    if [ -n "$MYSQL_SOCKET" ] ; then
      mysqlSocket=$MYSQL_SOCKET
    elif [ -d /run ] ; then
      mysqlSocket=/run/mysqld/mysqld.sock
    elif [ -d /var/run ] ; then
      mysqlSocket=/var/run/mysqld/mysqld.sock
    elif [ -d /var/lib/mysql ] ; then
      mysqlSocket=/var/mysql/mysql.sock
    else
      mysqlSocket=/run/mysqld/mysqld.sock
    fi
    [ $ID -eq 0 ] && $svctool --pwdlookup=$nssd_sock --threads=5 --timeout=-1 \
      --mysqlhost=localhost --mysqluser=$mysql_user --mysqlpass=$mysql_pass \
      --mysqlsocket=$mysqlSocket --servicedir=$servicedir $extra_opt
  fi
  #
  # poppassd protocol
  #
  [ $ID -eq 0 ] && $svctool --poppass=106 --localip=0 --maxdaemons=40 --maxperip=25 \
    --memory=$poppass_mem \
    --certfile=$sysconfdir/certs/servercert.pem --ssl \
    --setpassword=$prefix/sbin/vsetpass --servicedir=$servicedir $extra_opt
fi
#
# virus clamav/spam filter
#
if [ $clamav_os -eq 1 ] ; then
  [ $ID -eq 0 ] && $svctool --qscanq --servicedir=$servicedir --clamdPrefix=$prefix \
    --scanint=200 $extra_opt
  [ $ID -eq 0 ] && $svctool --config=clamd $extra_opt
  [ $ID -eq 0 ] && $svctool --config=foxhole
  [ $ID -eq 0 ] && $svctool --clamd --servicedir=$servicedir --clamdPrefix=$prefix \
    --sysconfdir=$sysconfdir
  if [ $clamav_os -eq 1 ] ; then
    [ $ID -eq 0 ] && touch $dir_prefix$servicedir/clamd/down
    [ $ID -eq 0 ] && touch $dir_prefix$servicedir/freshclam/down
  fi
fi

qcat <<EOF > "$dir_prefix"$sysconfdir/control/signatures
# Windows executables seen in active virii
TVqQAAMAA:
TVpQAAIAA
# Additional windows executable signatures not yet seen in virii
TVpAALQAc
TVpyAXkAX
TVrmAU4AA
TVrhARwAk
TVoFAQUAA
TVoAAAQAA
TVoIARMAA
TVouARsAA
TVrQAT8AA
# .ZIPfile signature seen in SoBig.E and mydoom:
#UEsDBBQAA:SoBig.e Virus
#UEsDBAoAAA:mydoom Virus
# .GIF file found in a previous Microsoft virus making the rounds.
R0lGODlhaAA7APcAAP///+rp6puSp6GZrDUjUUc6Zn53mFJMdbGvvVtXh2xre8bF1x8cU4yLprOy:Virus in .gif files
# http://www.gossamer-threads.com/lists/qmail/users/114447
UEsDBAoAAQAAAEBHYzCf4kJRDDAAAAAwAAAKAAAAc3ZtaXhlLmV4ZcuI1rOkjfn48VwCkMYHRTfM
EOF

[ $ID -eq 0 ] && $chown qscand:qscand "$dir_prefix"$sysconfdir/control/signatures

if [ $nobogofilter -eq 0 ] ; then
  [ $ID -eq 0 ] && $svctool --config=bogofilter $extra_opt
fi
#
# Misc/Configuration
#

# create certs
if [ ! -f $sysconfdir/certs/servercert.pem ] ; then
  create_ssl_cnf "" $postmaster@$default_domain $default_domain > \
    "$dir_prefix"$sysconfdir"/certs/servercert.cnf"
  if [ -n "$dir_prefix" ] ; then
    create_ssl_cnf $dir_prefix $postmaster@$default_domain $default_domain > \
      "$dir_prefix"$sysconfdir"/certs/servercert.cnf"
    [ $ID -eq 0 ] && $svctool --postmaster=postmaster@$default_domain --config=cert \
      --common_name=$default_domain $extra_opt
  else
    [ $ID -eq 0 ] && $svctool --postmaster=postmaster@$default_domain --config=cert \
      --common_name=$default_domain $extra_opt
  fi
fi

# fifolog service
[ $ID -eq 0 ] && $svctool --fifologger=$logfifo --servicedir=$servicedir $extra_opt

# udplogger service
[ $ID -eq 0 ] && $svctool --udplogger=3000 --localip=0 --timeout=10 --servicedir=$servicedir

# mrtg service
if [ -x /usr/bin/mrtg -a -f "$dir_prefix"$sysconfdir/indimail.mrtg.cfg ] ; then
  [ $ID -eq 0 ] && $svctool --config=snmpdconf
  [ $ID -eq 0 ] && $svctool --mrtg=/var/www/html/mailmrtg --servicedir=$servicedir $extra_opt
fi

# create svscan logging in indimail log directory
[ $ID -eq 0 ] && $svctool --svscanlog --servicedir=$servicedir $extra_opt

# turn off automatic refresh for services during first time installation
svc_list=""
for i in clamd greylist.1999 qmail-qmqpd.628 \
  qmail-qmtpd.209 qmail-smtpd.25 qmail-smtpd.366 .svscan \
  qmail-smtpd.465 qmail-smtpd.587 qscanq udplogger.3000 \
  qmail-daned.1998 qmail-imapd.143 qmail-imapd-ssl.993 \
  qmail-pop3d.110 qmail-pop3d-ssl.995 qmail-poppass.106 \
  fetchmail slowq-send qmail-send.25 qmail-logfifo freshclam \
  indisrvr.4000 inlookup.infifo mrtg mysql.3306 proxy-imapd.4143 \
  proxy-imapd-ssl.9143 proxy-pop3d.4110 proxy-pop3d-ssl.9110 \
  pwdlookup
do
  if [ -d "$dir_prefix"$servicedir/$i ] ; then
    svc_list="$svc_list $servicedir/$i"
    # save variables
    [ $ID -eq 0 ] && $svctool --servicedir=$servicedir --service-name=$i \
      --export-variables=$servicedir/$i/variables/.variables  --force
  fi
done
if [ -d "$dir_prefix"$sysconfdir/control/defaultqueue ] ; then
  svc_list="$svc_list $sysconfdir/control/defaultqueue"
fi
if [ -n "$svc_list" ] ; then
  echo "Setting nofreshsvc options"
  [ $ID -eq 0 ] && $svctool --servicedir=$servicedir --autorefresh="0 $svc_list" $extra_opt
fi

[ $ID -eq 0 ] && $svctool --servicedir=$servicedir --config=qselinux $extra_opt
if [ $noindimail -eq 0 ] ; then
  [ $ID -eq 0 ] && $svctool --servicedir=$servicedir --config=iselinux $extra_opt
fi
if [ -f "$dir_prefix"$sysconfdir/indimail-mta.cron -a -d /etc/cron.d ] ; then
  echo "adding indimail-mta cron entries"
  $cp "$dir_prefix"$sysconfdir/indimail-mta.cron /etc/cron.d
fi
if [ -f "$dir_prefix"$sysconfdir/indimail.cron -a -d /etc/cron.d ] ; then
  echo "adding indimail cron entries"
  $cp "$dir_prefix"$sysconfdir/indimail.cron /etc/cron.d
fi
if [ $noindimail -eq 0 ] ; then
  [ $ID -eq 0 ] && $svctool --fixsharedlibs $extra_opt
fi

if [ $add_boot -eq 1 ] ; then
  # Install svscan to be started on system boot
  echo "adding svscan startup"
  [ $ID -eq 0 ] && $svctool --config=add-boot $extra_opt
  echo "configuring indimail-mta as alternatives"
  [ $ID -eq 0 ] && $svctool --config=add-alt
fi

if [ $check_install -eq 1 ] ; then
  [ $ID -eq 0 ] && $svctool --check-install --servicedir=$servicedir \
    --qbase=$qbase --qcount=$qcount --qstart=1 $extra_opt
fi

#
# $Log: create_services.sh,v $
# Revision 2.131  2024-02-22 01:04:24+05:30  Cprogrammer
# replace cat with qcat
#
# Revision 2.130  2024-01-02 18:41:13+05:30  Cprogrammer
# renamed cronlist.q, cronlist.i to indimail-mta.cron, indimail.cron
#
# Revision 2.129  2023-12-03 12:27:46+05:30  Cprogrammer
# use infifo service in ODMR service for auth
#
# Revision 2.128  2023-10-24 21:46:04+05:30  Cprogrammer
# added --secureauth option for smtp port 587
# added --setuser-priv for slowq service
#
# Revision 2.127  2023-05-31 20:34:11+05:30  Cprogrammer
# create defaultqueue parameters without DKIM signing
#
# Revision 2.126  2023-05-20 23:13:10+05:30  Cprogrammer
# run ischema only if noindimail=0
#
# Revision 2.125  2023-05-20 19:43:57+05:30  Cprogrammer
# fix for alpine linux
#
# Revision 2.124  2023-05-20 12:08:38+05:30  Cprogrammer
# allow creation of mysql db even if indimail is not installed
#
# Revision 2.123  2023-04-23 16:27:42+05:30  Cprogrammer
# pass username and password argument for mysqldb creation
#
# Revision 2.122  2023-04-06 22:42:30+05:30  Cprogrammer
# added command to create /etc/indimail/snmpd.conf
#
# Revision 2.121  2023-03-11 16:07:54+05:30  Cprogrammer
# add --forceauthsmtp, --starttls options for odmr service on port 366
#
# Revision 2.120  2023-03-03 19:09:57+05:30  Cprogrammer
# run svctool only when running as uid 0
#
# Revision 2.119  2023-03-03 17:04:18+05:30  Cprogrammer
# exit if not running as root
#
# Revision 2.118  2023-02-14 17:58:06+05:30  Cprogrammer
# added --setgroups to set USE_SETGROUPS env variable for qmail-start
#
# Revision 2.117  2022-11-07 20:45:29+05:30  Cprogrammer
# removed domainkeys for new installs
#
# Revision 2.116  2022-09-27 22:05:02+05:30  Cprogrammer
# Create slowq-send service with todo processor turned on
#
# Revision 2.115  2022-09-19 22:21:54+05:30  Cprogrammer
# run ischema -u to update indimail schema to current
#
# Revision 2.114  2022-04-06 09:11:01+05:30  Cprogrammer
# set fsync, syncdir as global variables instead of local variables
#
# Revision 2.113  2022-03-06 18:48:10+05:30  Cprogrammer
# fix for FreeBSD (date usage)
#
# Revision 2.112  2021-08-23 17:34:11+05:30  Cprogrammer
# fixed default_domain variable
#
# Revision 2.111  2021-08-20 13:08:28+05:30  Cprogrammer
# remove host component from default domain
#
# Revision 2.110  2021-08-19 19:54:16+05:30  Cprogrammer
# changes for modified dknewkey
#
# Revision 2.109  2021-08-11 18:15:25+05:30  Cprogrammer
# removed not needed mkdir during build process
#
# Revision 2.108  2021-08-01 09:55:10+05:30  Cprogrammer
# install mrtg service only if mrtg binary is present
#
# Revision 2.107  2021-07-30 12:21:54+05:30  Cprogrammer
# use rc-update to add delete services with openrc
#
# Revision 2.106  2021-07-28 10:28:04+05:30  Cprogrammer
# reordered commands
#
# Revision 2.105  2021-07-27 18:33:59+05:30  Cprogrammer
# removed use of --default-domain to set local variable. Use global_vars for default domain
#
# Revision 2.104  2021-07-22 17:12:17+05:30  Cprogrammer
# removed use if initsvc to add boot entry
#
# Revision 2.103  2021-07-21 23:59:29+05:30  Cprogrammer
# fixed ps command for alpine/busybox
#
# Revision 2.102  2021-07-21 23:35:09+05:30  Cprogrammer
# fixed for mysql socket
#
# Revision 2.102  2021-07-21 22:27:28+05:30  Cprogrammer
# use common variable for mysql socket
#
# Revision 2.101  2021-07-21 16:15:00+05:30  Cprogrammer
# added services to no autorefresh list
#
# Revision 2.100  2021-07-21 00:01:42+05:30  Cprogrammer
# skip cert creation if existing
#
#
