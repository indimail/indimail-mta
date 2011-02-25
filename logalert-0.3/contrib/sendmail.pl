#!/usr/bin/perl
#
#############################################
# Send mail using Net:SMTP perl module
# Does not require any mail service on the machine
#############################################


#############################################
# Modules
############################################
use strict;
use Getopt::Std;
use vars qw( $VERSION $DEBUG $DEBUG_FILE $PROGNAME %msgError %globalOpts );
use Net::SMTP;

############################################
# Variables
############################################
$VERSION = '0.5';
$DEBUG=0;
$PROGNAME=$0;
############################################

# Define here message errors expected
%msgError = ( 
   
    'findFile' => 'Could not find file',
    'fileOpen' => 'Could not open file',
    'fileExists' => 'File not found or empty',
    'noSubj' => 'Subject not found or empty',
    'getHelp' => 'Try -h for help',
    'mailFailed' => 'SendMail has failed!',
    'mailOK' => 'Mail was sent successfully!',
    'syntax' => "$PROGNAME version $VERSION
syntax: $0 -s -t -f -H [-u -p -h] [ <message/text> ]
\t-s subject
\t-H mailhost server
\t-t To: email list destination 
\t-c Cc: email list destination \n
\t-f From: orig/recip address\n
\t If you use SMTP authentication:\n
\t-u user [AUTH]
\t-p password [AUTH]
\t<message/text> message itself or a text file. if not defined, stdin is used.

\t Note: email lists must have ',' as a separator .
"

);

%globalOpts = (

    CONFFILE => $PROGNAME . '.conf',
    MAILHOST => 'localhost',
    MAILSUBJ => $PROGNAME . ' - Auto Email Sender',
    MAILTO => $ENV{'USER'} . '@localhost',
    MAILBODY => $PROGNAME . ' - Auto Email Sender ' . "\n" ,
    MAILFROM => $ENV{'USER'} . '@localhost',
    MAILAUTHUSER => '',
    MAILAUTHPASS => '',
    MAILCC => '',
    MAILBCC => ''
);

############################################
# Procedures
############################################

############################################
# sub treatError
# Description: a common procedure to treat errors
#              and exits abnormaly.
# Arguments:
#    1 - msg text to be displayed
#    2 - additional arg (file name, instruction, etc) - optional
############################################
sub treatError {

    return unless @_;
    my $text = shift;
    my $arg = shift || '.';

    print STDOUT 'Error: ' . $text . ': ' . $arg . "\n\n" . $msgError{'getHelp'} . "\n";
    exit 1 ;
} 

############################################
# sub myDebug
# Description: a common procedure to treat debug msgs.
#    It uses global vars $DEBUG and $DEBUG_FILE.
#    $DEBUG_FILE will only be used if a file name was
#    defined by the user and all debug info will be 
#    logged into this file ($DEBUG is set to 2). 
#    Otherwise debug info is set to STDOUT.
# Arguments:
#    1 - debug msg text 
############################################
sub myDebug {
  
    return unless @_;
    my $text = shift;
    my $curDate = `date '+[%x %X] '`;
 
    chop $curDate;
    if($DEBUG > 0) {

      if($DEBUG == 1) {
         print STDOUT 'Debug: ' . $text . "\n";
      } elsif ($DEBUG == 2) {
         if(open(DFILE,">>$DEBUG_FILE")) {
              print DFILE $curDate . $text . "\n";
              close(DFILE);
         } else {
              print STDERR "Debug: " . $msgError{'openFile'} . ' -> STDOUT' . "\n";
              $DEBUG = 1;
         } 
      }
    }
}

############################################
# sub freakout
# Description: scream and die!
############################################
sub freakout {
                                                                                               
    my $msg = shift;
                                                                                               
    &myDebug("[!] freakout : $msg\n");
    print STDERR "[!] freakout : $msg\n";
    exit(1);
                                                                                               
}

############################################
# sub checkMail
# Description: checa validade da sintaxe do email
############################################

sub checkMail() {
                                                                                               
    return unless @_;
    my $mail = shift;
                                                                                               
   ( ( ($mail =~ /localhost/) || ($mail =~ /\w@[\w.]+\.[a-z]+/) ) && (return 1) ) || (return 0)                                                                                               
}


############################################
# sub sendMail()
# Description: sends an email using a SMTP server
#
############################################

sub sendMail() {
                                                                                               
    my $mail;
    my $curMail;
    my $status = 1;
    my @mailList;
    my @ccList;
    my $date = `date +'%x %X'`;
    my $Header;

                                                                                               
    chomp $date;
                                                                                               
    $Header = "Date: $date\n";
    $Header .= "From: $globalOpts{MAILFROM}\n";
    $Header .= "To: $globalOpts{MAILTO}\n";

    $Header .= "Cc: $globalOpts{MAILCC}\n" if($globalOpts{MAILCC} ne '' );
                                                                                               
    foreach $mail (split(/\s*,\s*/,$globalOpts{MAILTO})) {
                                                                                               
        ($mail =~ /\w/) || next ;
                                                                                               
        if(&checkMail($mail)) {
                push @mailList,$mail ;
                &myDebug('Adicionando email ' . $mail);
        } else {
                &myDebug($msgError{'mailSyntax'} . $mail);
        }
    }

    foreach $mail (split(/\s*,\s*/,$globalOpts{MAILCC})) {
                                                                                               
        ($mail =~ /\w/) || next ;
                                                                                               
        if(&checkMail($mail)) {
                push @ccList,$mail ;
                &myDebug('Adicionando email ' . $mail);
        } else {
                &myDebug($msgError{'mailSyntax'} . $mail);
        }
     }

                                                                                               
    $mail = $globalOpts{MAILFROM};
                                                                                               
    if(! ( ((scalar @mailList) > 0) && (&checkMail($mail)) ) ) {
           &myDebug($msgError{'mailSyntax'});
           return 0 ;
    }
                                                                                               
        $Header .= "Subject: $globalOpts{MAILSUBJ}\n";
        $Header .= "X-Mailer: Auto service ($VERSION)\n";
        $Header .= "Organization: Auto\n";
        $Header .= "X-Powered-By: Auto\n";
        $Header .= "Message-ID: <".time().".".rand(999999).">\n";
        $Header .= "Content-Type: TEXT/PLAIN; charset=US-ASCII\n";
                                                                                               
                                                                                               
    my $smtp = Net::SMTP->new($globalOpts{MAILHOST});

    if($smtp) {
                                                                                               
            if($globalOpts{MAILAUTHUSER} && $globalOpts{MAILAUTHPASS}) {
                  $smtp->auth($globalOpts{MAILAUTHUSER},$globalOpts{MAILAUTHPASS});
            }

       eval {
            $smtp->mail($mail);
            $smtp->to(@mailList);
	    $smtp->cc(@ccList) if($globalOpts{MAILCC} ne '' );
            $smtp->data();
            $smtp->datasend($Header . "\n");
            $smtp->datasend($globalOpts{MAILBODY} . "\n");
            $smtp->dataend();
            $smtp->quit();
        };
                                                                                               
       if($@) {
           &myDebug($msgError{'mailFailed'});
           $status = 0;
       } else {
           &myDebug($msgError{'mailOK'});
       }
                                                                                               
       $smtp->quit;
                                                                                               
     } else {
         &myDebug($msgError{'mailHostFailed'} . $globalOpts{MAILHOST});
         $status = 0;
     }
                                                                                               
    return $status;
}


    
############################################
# sub readConfFile
# Description: reads configuration file and sets global
#     variable $globalOpts.
# Arguments:
#     none
############################################
sub readConfFile {

    ( -s $globalOpts{CONFFILE} ) || &treatError($msgError{'fileExists'},$globalOpts{CONFFILE});

}

############################################
# sub setOptions
# Description: sets options passed by the user.
#     You must define which options are available
#     in Getopt::Std format.
# Arguments:
#     none
############################################
sub setOptions {

    my %opt={};
    
    getopts('H:t:f:s:u:p:c:hD',\%opt);

    if(exists $opt{'h'}) {
          print STDOUT $msgError{'syntax'};
          exit(0);
    }

    if(exists $opt{'D'}) {
        $DEBUG = 1;
     }

    if(exists $opt{'s'}) {
      ($opt{'s'} ne '') ?
        $globalOpts{MAILSUBJ} =  $opt{'s'}
        : &treatError($msgError{'noSubj'},$opt{'s'});
    }

    if(exists $opt{'t'}) {
      ($opt{'t'} ne '') ?
        $globalOpts{MAILTO} =  $opt{'t'}
        : &treatError($msgError{'noSubj'},$opt{'t'});
    }

    if(exists $opt{'f'}) {
      ($opt{'f'} ne '') ?
        $globalOpts{MAILFROM} =  $opt{'f'}
        : &treatError("mail from .. ?!",$opt{'f'});
    }

    if(exists $opt{'c'}) {
      ($opt{'c'} ne '') ?
        $globalOpts{MAILCC} =  $opt{'c'}
        : &treatError("cc list .. ?!",$opt{'c'});
    }

    if(exists $opt{'u'}) {
      ($opt{'u'} ne '') ?
        $globalOpts{MAILAUTHUSER} =  $opt{'u'}
        : &treatError("No user set!",$opt{'u'});
    }

    if(exists $opt{'H'}) {
      ($opt{'H'} ne '') ?
        $globalOpts{MAILHOST} =  $opt{'H'}
        : &treatError("No user set!",$opt{'u'});
    }


    if(exists $opt{'p'}) {
      ($opt{'p'} ne '') ?
        $globalOpts{MAILAUTHPASS} =  $opt{'p'}
        : &treatError("No password set!",$opt{'p'});
    }

}

############################################
# sub getFromFile
# Description: get text from a file
############################################
sub getFromFile {

    my $fileName = shift;
    my $text;

    open(FEEDME,"$fileName") || die $msgError{'openFile'} . "$fileName" ;

    while(<FEEDME>) {
         $text .= $_;
    }
    close(FEEDME);

    return $text;

}

############################################
# sub getFromSTDINPUT
# Description: get text from a standart input
############################################
sub getFromSTDINPUT {

    my $text;

    while(<>) {

      $text .= $_;
    }

    return $text;

} 


############################################
# Main - that's up to you :-D
############################################


my $numArgs = scalar @ARGV;

  (scalar @ARGV < 1 ) && die $msgError{'syntax'} . "\n"; 

  setOptions();

  --$numArgs;

  if(!($ARGV[0])) {
     
      $globalOpts{MAILBODY} = getFromSTDINPUT();

  } else {

      if(-s $ARGV[0]) {
          $globalOpts{MAILBODY} = getFromFile($ARGV[0]); 

      } else {  # its the message itself!
          $globalOpts{MAILBODY} = $ARGV[0]; 
      }

  }

  &sendMail();

