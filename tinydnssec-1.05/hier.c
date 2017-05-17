#include "auto_home.h"
extern char *mandir;
void c(const char *,const char *,const char *,int,int,int);
void h(const char *, int,int,int);
void d(const char *,const char *,int,int,int);
void l(const char *,const char *,const char *);

void hier(char *inst_dir)
{
  const char *auto_h = auto_home;

  if (inst_dir && *inst_dir)
    auto_h = inst_dir;
  h("/etc/indimail",-1,-1,02755);
  c("/etc","indimail","dnsroots.global",-1,-1,0644);

  h(auto_h,-1,-1,0755);
  h(mandir,-1,-1,0755);
  d(mandir,"man1",-1,-1,0755);
  d(mandir,"man5",-1,-1,0755);
  d(mandir,"man7",-1,-1,0755);
  d(mandir,"man8",-1,-1,0755);
  d(auto_h,"bin",-1,-1,0755);
  d(auto_h,"sbin",-1,-1,0755);

  c(auto_h,"sbin","dnscache-conf",-1,-1,0755);
  c(auto_h,"sbin","tinydns-conf",-1,-1,0755);
  c(auto_h,"sbin","walldns-conf",-1,-1,0755);
  c(auto_h,"sbin","dqcache-conf",-1,-1,0755);
  c(auto_h,"sbin","curvedns-conf",-1,-1,0755);
  c(auto_h,"sbin","rbldns-conf",-1,-1,0755);
  c(auto_h,"sbin","pickdns-conf",-1,-1,0755);
  c(auto_h,"sbin","axfrdns-conf",-1,-1,0755);

  c(auto_h,"bin","dnscache",-1,-1,0755);
  c(auto_h,"bin","tinydns",-1,-1,0755);
  c(auto_h,"bin","walldns",-1,-1,0755);
  c(auto_h,"bin","rbldns",-1,-1,0755);
  c(auto_h,"bin","pickdns",-1,-1,0755);
  c(auto_h,"bin","axfrdns",-1,-1,0755);

  c(auto_h,"bin","tinydns-get",-1,-1,0755);
  c(auto_h,"bin","tinydns-data",-1,-1,0755);
  c(auto_h,"bin","tinydns-edit",-1,-1,0755);
  c(auto_h,"bin","rbldns-data",-1,-1,0755);
  c(auto_h,"bin","pickdns-data",-1,-1,0755);
  c(auto_h,"bin","axfr-get",-1,-1,0755);

  c(auto_h,"bin","dnsip",-1,-1,0755);
  c(auto_h,"bin","dnsip6",-1,-1,0755);
  c(auto_h,"bin","dnsnamex",-1,-1,0755);
  c(auto_h,"bin","dnsipq",-1,-1,0755);
  c(auto_h,"bin","dnsip6q",-1,-1,0755);
  c(auto_h,"bin","dnsname",-1,-1,0755);
  c(auto_h,"bin","dnstxt",-1,-1,0755);
  c(auto_h,"bin","dnsmx",-1,-1,0755);
  c(auto_h,"bin","dnsfilter",-1,-1,0755);
  c(auto_h,"bin","random-ip",-1,-1,0755);
  c(auto_h,"bin","dnsqr",-1,-1,0755);
  c(auto_h,"bin","dnsq",-1,-1,0755);
  c(auto_h,"bin","dnstrace",-1,-1,0755);
  c(auto_h,"bin","dnstracesort",-1,-1,0755);
  c(auto_h,"bin","dnsgetroot",-1,-1,0755);
  c(auto_h,"bin","tinydns-sign",-1,-1,0755);
  c(mandir,"man1","dnsfilter.1",-1,-1,0644);
  c(mandir,"man1","dnsip.1",-1,-1,0644);
  c(mandir,"man1","dnsip6.1",-1,-1,0644);
  c(mandir,"man1","dnsipq.1",-1,-1,0644);
  c(mandir,"man1","dnsip6q.1",-1,-1,0644);
  c(mandir,"man1","dnsmx.1",-1,-1,0644);
  c(mandir,"man1","dnsname.1",-1,-1,0644);
  c(mandir,"man1","dnsnamex.1",-1,-1,0644);
  c(mandir,"man1","dnsq.1",-1,-1,0644);
  c(mandir,"man1","dnsqr.1",-1,-1,0644);
  c(mandir,"man1","dnstrace.1",-1,-1,0644);
  c(mandir,"man1","dnstracesort.1",-1,-1,0644);
  c(mandir,"man1","dnstxt.1",-1,-1,0644);
  c(mandir,"man1","tinydns-get.1",-1,-1,0644);
  c(mandir,"man1","random-ip.1",-1,-1,0644);
  c(mandir,"man1","dnsgetroot.1",-1,-1,0644);
  c(mandir,"man5","qualification.5",-1,-1,0644);
  c(mandir,"man7","djbdns.7",-1,-1,0644);
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
  c(mandir,"man8","pickdns.8",-1,-1,0644);
  c(mandir,"man8","pickdns-data.8",-1,-1,0644);
  c(mandir,"man8","pickdns-conf.8",-1,-1,0644);
  c(mandir,"man8","dqcache-conf.8",-1,-1,0644);
  c(mandir,"man8","curvedns-conf.8",-1,-1,0644);
}
