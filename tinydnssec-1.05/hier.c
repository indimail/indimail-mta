#include "auto_home.h"
void c(const char *,const char *,const char *,int,int,int);
void h(const char *, int,int,int);
void d(const char *,const char *,int,int,int);
void l(const char *,const char *,const char *);

void hier()
{
 char *mandir = "/usr/share/man";

  h("/etc/indimail",-1,-1,02755);
  c("/etc","indimail","dnsroots.global",-1,-1,0644);
#if 0
  l("/etc/indimail/tinydns",  "dnsroots.global", "/etc/indimail");
  l("/etc/indimail/dnscache", "dnsroots.global", "/etc/indimail");
#endif

  h(auto_home,-1,-1,02755);
  h(mandir,-1,-1,0755);
  d(mandir,"man1",-1,-1,0755);
  d(mandir,"man5",-1,-1,0755);
  d(mandir,"man8",-1,-1,0755);
  d(auto_home,"bin",-1,-1,02755);

  c(auto_home,"bin","dnscache-conf",-1,-1,0755);
  c(auto_home,"bin","tinydns-conf",-1,-1,0755);
  c(auto_home,"bin","walldns-conf",-1,-1,0755);
  c(auto_home,"bin","rbldns-conf",-1,-1,0755);
  c(auto_home,"bin","pickdns-conf",-1,-1,0755);
  c(auto_home,"bin","axfrdns-conf",-1,-1,0755);

  c(auto_home,"bin","dnscache",-1,-1,0755);
  c(auto_home,"bin","tinydns",-1,-1,0755);
  c(auto_home,"bin","walldns",-1,-1,0755);
  c(auto_home,"bin","rbldns",-1,-1,0755);
  c(auto_home,"bin","pickdns",-1,-1,0755);
  c(auto_home,"bin","axfrdns",-1,-1,0755);

  c(auto_home,"bin","tinydns-get",-1,-1,0755);
  c(auto_home,"bin","tinydns-data",-1,-1,0755);
  c(auto_home,"bin","tinydns-edit",-1,-1,0755);
  c(auto_home,"bin","rbldns-data",-1,-1,0755);
  c(auto_home,"bin","pickdns-data",-1,-1,0755);
  c(auto_home,"bin","axfr-get",-1,-1,0755);

  c(auto_home,"bin","dnsip",-1,-1,0755);
  c(auto_home,"bin","dnsip6",-1,-1,0755);
  c(auto_home,"bin","dnsnamex",-1,-1,0755);
  c(auto_home,"bin","dnsipq",-1,-1,0755);
  c(auto_home,"bin","dnsip6q",-1,-1,0755);
  c(auto_home,"bin","dnsname",-1,-1,0755);
  c(auto_home,"bin","dnstxt",-1,-1,0755);
  c(auto_home,"bin","dnsmx",-1,-1,0755);
  c(auto_home,"bin","dnsfilter",-1,-1,0755);
  c(auto_home,"bin","random-ip",-1,-1,0755);
  c(auto_home,"bin","dnsqr",-1,-1,0755);
  c(auto_home,"bin","dnsq",-1,-1,0755);
  c(auto_home,"bin","dnstrace",-1,-1,0755);
  c(auto_home,"bin","dnstracesort",-1,-1,0755);
  c(auto_home,"bin","dnsgetroot",-1,-1,0755);
  c(mandir,"man1","dnsfilter.1",-1,-1,0644);
  c(mandir,"man1","dnsip.1",-1,-1,0644);
  c(mandir,"man1","dnsipq.1",-1,-1,0644);
  c(mandir,"man1","dnsmx.1",-1,-1,0644);
  c(mandir,"man1","dnsname.1",-1,-1,0644);
  c(mandir,"man1","dnsq.1",-1,-1,0644);
  c(mandir,"man1","dnsqr.1",-1,-1,0644);
  c(mandir,"man1","dnstrace.1",-1,-1,0644);
  c(mandir,"man1","dnstracesort.1",-1,-1,0644);
  c(mandir,"man1","dnstxt.1",-1,-1,0644);
  c(mandir,"man1","tinydns-get.1",-1,-1,0644);
  c(mandir,"man5","qualification.5",-1,-1,0644);
  c(mandir,"man8","axfrdns.8",-1,-1,0644);
  c(mandir,"man8","axfrdns-conf.8",-1,-1,0644);
  c(mandir,"man8","axfr-get.8",-1,-1,0644);
  c(mandir,"man8","dnscache.8",-1,-1,0644);
  c(mandir,"man8","dnscache-conf.8",-1,-1,0644);
  c(mandir,"man8","rbldns.8",-1,-1,0644);
  c(mandir,"man8","rbldns-conf.8",-1,-1,0644);
  c(mandir,"man8","rbldns-data.8",-1,-1,0644);
  c(mandir,"man8","tinydns.8",-1,-1,0644);
  c(mandir,"man8","tinydns-conf.8",-1,-1,0644);
  c(mandir,"man8","tinydns-data.8",-1,-1,0644);
  c(mandir,"man8","tinydns-edit.8",-1,-1,0644);
  c(mandir,"man8","tinydns-get.1",-1,-1,0644);
  c(mandir,"man8","tinydns-sign.8",-1,-1,0644);
  c(mandir,"man8","walldns.8",-1,-1,0644);
  c(mandir,"man8","walldns-conf.8",-1,-1,0644);
}
