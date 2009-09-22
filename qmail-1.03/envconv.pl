#!/usr/bin/perl
# Written by Jonas Pasche 
while (<STDIN>) {
  if( $_ =~ /^([^#][^=]+)=(.*)/) {
    open (F, ">$1");
    $v = $2;
    if( $v =~ /^"(.*)"$/ ) {
      print F $1;
    } else {
      print F $v;
    }
    close (F);
  }
}
