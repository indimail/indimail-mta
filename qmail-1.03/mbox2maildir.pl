#!/usr/bin/perl
#
# mbox2maildir: coverts mbox file to maildir directory - the reverse of
# maildir2mbox from the qmail distribution.
#
# Usage: mbox2maildir uses the same environment variables as maildir2mbox:
# MAILDIR is the name of your maildir directory; MAIL is the name of your
# mbox file; MAILTMP is ignored.  MAIL is deleted after the conversion.
#
# WARNING: there is no locking; don't run more than one of these!  you
# have been warned.
#
# based on convert-and-create by Russell Nelson <nelson@qmail.org>
# kludged into this by Ivan Kohler <ivan@voicenet.com> 97-sep-17
 
require 'stat.pl';
 
local $SIG{HUP} = 'IGNORE';
local $SIG{INT} = 'IGNORE';
local $SIG{QUIT} = 'IGNORE'; 
local $SIG{TERM} = 'IGNORE';
local $SIG{TSTP} = 'IGNORE';
 
($name, $passwd, $uid, $gid, $quota, $comment, $gcos, $dir, $shell) =
  getpwuid($<);
 
die "fatal: home dir $dir doesn't exist\n" unless -e $dir;
&Stat($dir);
die "fatal: $name is $uid, but $dir is owned by $st_uid\n" if $uid != $st_uid;
 
chdir($dir) or die "fatal: unable to chdir to $dir\n";
$spoolname = "$ENV{MAILDIR}";
-d $spoolname or mkdir $spoolname,0700
  or die("fatal: $spoolname doesn't exist and can't be created.\n");
 
chdir($spoolname) or die("fatal: unable to chdir to $spoolname.\n");
-d "tmp" or mkdir("tmp",0700) or die("fatal: unable to make tmp/ subdir\n");
-d "new" or mkdir("new",0700) or die("fatal: unable to make new/ subdir\n");
-d "cur" or mkdir("cur",0700) or die("fatal: unable to make cur/ subdir\n");
 
open(SPOOL, "<$ENV{MAIL}")
  or die "Unable to open $ENV{$MAIL}\n";
$i = time;
while(<SPOOL>) {
  if (/^From /) {
    $fn = sprintf("new/%d.$$.mbox", $i);
    open(OUT, ">$fn") or die("fatal: unable to create new message");
    $i++;
    next;
  }
  s/^>From /From /;
  print OUT or die("fatal: unable to write to new message");
}
close(SPOOL);
close(OUT);
unlink("$ENV{MAIL}");
