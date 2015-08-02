#! /usr/bin/perl
# Script to train bogofilter from mboxes
# by Boris 'pi' Piwinger <3.14@piology.org>
# with many useful additions by David Relson <relson@osagesoftware.com>

# Program locations
my $bogofilter="bogofilter";
my $bogoutil="bogoutil";
my $bf_compact="bf_compact";
my $catcommand="bzcat -f";

# Not correct number of parameters
my $commandlineoptions=($ARGV[0]=~/^-(?=[^c]*c?[^c]*$)(?=[^f]*f?[^f]*$)(?=[^n]*n?[^n]*$)(?=[^s]*s?[^s]*$)[cfns]*v{0,2}[cfns]*$/);
unless (scalar(@ARGV)-$commandlineoptions==3 || scalar(@ARGV)-$commandlineoptions==4) {
  print <<END;

bogominitrain.pl version 1.6
  requires bogofilter 0.94.4 or later

Usage:
  bogominitrain.pl [-[f][v[v]][s]] <database-directory> <ham-mboxes>\\
    <spam-mboxes> [bogofilter-options]

  database-directory is the directory containing your wordlist. It
  will be created as needed.  ham-mboxes and spam-mboxes are the
  mboxes containing the mails; they will be shell-expanded.
  bogofilter-options are given to bogofilter literally.

  Uses a "train on error" process to build minimal wordlists that can
  correctly score all messages.

  It may be a good idea to run this script command several times.  Use
  the '-f' option to run the script until no scoring errors occur
  (training to exhaustion). The '-n' option will prevent messages from
  being added more than once; this may leave errors at the end.

  To improve bogofilter's accuracy, use bogofilter's -o option to
  create a "security margin" around your normal cutoff during
  training.  The script will train so that the messages will avoid
  this interval, i.e., all messages in your training mboxes will be
  marked as ham or spam with values far from your production cutoff.
  For example you might want to use spam_cutoff=0.5 and '-o 0.8,0.2'
  as bogofilter options.  If you would rather use tri-state mode, you
  can just center this around 0.5 and again use '-o 0.8,0.2'.

  To correct the classification of a message, just move it to the
  correct mbox and repeat the full training process (which will add a
  few messages to the existing database).

Example:
  bogominitrain.pl -fnv .bogofilter 'ham*' 'spam*' '-c train.cf'

Options:
  -v   This switch produces info on messages used for training.
  -vv  Also lists messages not used for training.
  -f   Runs the program until no errors remain.
  -n   Prevents repetitions.
  -s   Saves the messages used for training to files
       bogominitrain.ham.* and bogominitrain.spam.*
  -c   Compacts the database at the end.
  Note: If you need to use more than one option, you must combine them.

END
  exit;
}

# Check input
my $compact=1 if ($commandlineoptions && $ARGV[0]=~s/c//);
my $force=1 if ($commandlineoptions && $ARGV[0]=~s/f//);
my $norepetitions=1 if ($commandlineoptions && $ARGV[0]=~s/n//);
my ($safe,$safeham,$safespam)=(1,"bogominitrain.ham","bogominitrain.spam")
   if ($commandlineoptions && $ARGV[0]=~s/s//);
my $verbose=1 if ($commandlineoptions && $ARGV[0]=~s/^-v/-/);
my $vverbose=1 if ($commandlineoptions && $ARGV[0] eq "-v");
shift (@ARGV) if ($commandlineoptions);
my ($dir,$ham,$spam,$options) = @ARGV;
$bogofilter.=" $options -d $dir";
die ("$dir is not a directory or not accessible.\n") unless (-d $dir && -r $dir && -w $dir && -x $dir);
`$bogofilter -n < /dev/null` unless (-s "$dir/wordlist.db");
my $ham_total=`$catcommand $ham 2>/dev/null |grep -c "^From "`;
my $spam_total=`$catcommand $spam 2>/dev/null |grep -c "^From "`;
my ($fp,$fn,$hamadd,$spamadd,%trainedham,%trainedspam);
my $runs=0;
my @status=("S","H","U","E");

print "\nStarting with this database:\n";
print `$bogoutil -w $dir/wordlist.db .MSG_COUNT`,"\n";

do { # Start force loop
  my $starttime=time;
  $runs++;
  open (HAM,  "$catcommand $ham 2>/dev/null |")  || die("Cannot open ham: $!\n");
  open (SPAM, "$catcommand $spam 2>/dev/null |") || die("Cannot open spam: $!\n");

  # Loop through all the mail
  my ($lasthamline,$lastspamline,$hamcount,$spamcount,$skipham,$skipspam) = ("","",0,0,0,0);
  ($hamadd,$spamadd)=(0,0);
  do {

    # Read one mail from ham box and test, train as needed
    unless (eof(HAM) || $hamcount*$spam_total > $spamcount*$ham_total) {
      my $mail=$lasthamline;
      $lasthamline="";
      while (defined(my $line=<HAM>)) {
        if ($line=~/^From /) {$lasthamline=$line; last;}
        $mail.=$line;
      }
      if ($mail) {
        $hamcount++;
        open (TEMP, "| $bogofilter") || die "Cannot pipe to bogofilter: $!";
        print TEMP $mail;
        close (TEMP);
        my$status=$status[$?>>8];
        unless ($status eq "H") {
          unless ($norepetitions && $trainedham{$hamcount}) {
            open (TEMP, "| $bogofilter -n") || die "Cannot pipe to bogofilter: $!";
            print TEMP $mail;
            close (TEMP);
            $hamadd++;
            $trainedham{$hamcount}++;
            print "$status -- Training ham message $hamcount",
                  $trainedham{$hamcount}>1&&" ($trainedham{$hamcount})",
                  ".\n" if ($verbose);
            if ($safe) {
              open (TEMP, ">>$safeham.$runs") || die "Cannot write to $safeham.$runs: $!";
              print TEMP $mail;
              close (TEMP);
            }
          } else {$skipham++;print "$status -- Skipping ham message $hamcount.\n" if ($verbose);}
        } else {print "$status -- Not training ham message $hamcount.\n" if ($vverbose);}
      }
    }

    # Read one mail from spam box and test, train as needed
    unless (eof(SPAM) || $spamcount*$ham_total > $hamcount*$spam_total) {
      my $mail=$lastspamline;
      $lastspamline="";
      while (!eof(SPAM) && defined(my $line=<SPAM>)) {
        if ($line=~/^From /) {$lastspamline=$line; last;}
        $mail.=$line;
      }
      if ($mail) {
        $spamcount++;
        open (TEMP, "| $bogofilter") || die "Cannot pipe to bogofilter: $!";
        print TEMP $mail;
        close (TEMP);
        my$status=$status[$?>>8];
        unless ($status eq "S") {
          unless ($norepetitions && $trainedspam{$spamcount}) {
            open (TEMP, "| $bogofilter -s") || die "Cannot pipe to bogofilter: $!";
            print TEMP $mail;
            close (TEMP);
            $spamadd++;
            $trainedspam{$spamcount}++;
            print "$status -- Training spam message $spamcount",
                  $trainedspam{$spamcount}>1&&" ($trainedspam{$spamcount})",
                  ".\n" if ($verbose);
            if ($safe) {
              open (TEMP, ">>$safespam.$runs") || die "Cannot write to $safespam.$runs: $!";
              print TEMP $mail;
              close (TEMP);
            }
          } else {$skipspam++;print "$status -- Skipping spam message $spamcount.\n" if ($verbose);}
        } else {print "$status -- Not training spam message $spamcount.\n" if ($vverbose);}
      }
    }

  } until (eof(HAM) && eof(SPAM));
  close (HAM);
  close (SPAM);

  print "\nEnd of run #$runs (in ",time-$starttime,"s):\n";
  print "Read $hamcount ham mail",$hamcount!=1&&"s"," and $spamcount spam mail",$spamcount!=1&&"s",".\n";
  print "Added $hamadd ham mail",$hamadd!=1&&"s",$skipham>0&&" (skipping $skipham)",
        " and $spamadd spam mail",$spamadd!=1&&"s",$skipspam>0&&" (skipping $skipspam)",
        " to the database.\n";
  print `$bogoutil -w $dir/wordlist.db .MSG_COUNT`;
  unless ($hamadd+$spamadd==0) {
    $starttime=time;
    $fn=$spamcount>0 && `$catcommand $spam | $bogofilter -TM | grep -cv ^S` || "0\n";
    print "\nFalse negatives: $fn";
    $fp=$hamcount>0 && `$catcommand $ham | $bogofilter -TM | grep -cv ^H` || "0\n";
    print "False positives: $fp\n";
    print "Verification done in ",time-$starttime,"s.\n\n";
  }
} until ($fn+$fp==0 || $hamadd+$spamadd==0 || !$force);
print "\n$runs run",$runs>1&&"s"," needed to close off.\n" if ($force);
if ($compact) {
  print "Compacting database ...\n";
  system("$bf_compact $dir && rm -rf $dir.old");
}

