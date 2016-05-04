FROM ubuntu:15.10

MAINTAINER cprogrammer

# Freshen
RUN apt-get -y update
RUN apt-get install -y wget openssh-server man-db vim net-tools less lsof strace heirloom-mailx
RUN wget -q http://download.opensuse.org/repositories/home:/indimail/xUbuntu_15.10/Release.key -O - | apt-key add -
RUN echo 'deb http://download.opensuse.org/repositories/home:/indimail/xUbuntu_15.10/ /' > /etc/apt/sources.list.d/indimail.list
RUN apt-get -y update
RUN DEBIAN_FRONTEND=noninteractive apt-get -q -y install indimail
RUN echo 127.0.0.1 > /var/indimail/control/localiphost
