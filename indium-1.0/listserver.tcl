#!/bin/sh
# Executing TCL-interpreter #\
echo -e "Trying interpreters...\n" && no_exit_on_failed_exec=1 && export no_exit_on_failed_exec && shopt -s execfail 2>/dev/null; true && { echo "Trying tclsh..."; exec tclsh "$0" "$@" 2>/dev/null || echo "Trying tcl..."; exec tcl "$0" "$@" 2>/dev/null || echo -e "No valuable interpreter on $(hostname) for the ListServer!\n\nYOU NEED TCL INSTALLED ON THE REMOTE SYSTEM\n\nExiting..."; }

#############################################################
# SecPanel - ListServer
# Service for remote file selection and scp
# Version SecPanel 0.40
# Author: Steffen Leich <secpanel@pingx.net>
#############################################################

global clientpass, authenticated, busy;

# gets set with each listserver-launch
set clientpass <PASS-XXXXX>

set authenticated 0
set busy 0

proc scpAuth {sock} {
    global pass clientpass authenticated

    if {! $authenticated} {
	gets $sock passandport
	
	set serverpass [lindex [split $passandport] 1]
	if {$serverpass != $clientpass} {
	    puts "Wrong authentication from ListClient (SecPanel)\nHad to reject connection"
	    close $sock
	    return
	} else {
	    set authenticated 1
	    puts "SecPanel sent good authentication.\nNow serving..."
	    GiveList $sock
	}
    } else {
	GiveList $sock
    }
}

proc Accept {sock addr port} {
    global main busy
    puts "Connection from $addr on port $port"

    if {! $busy} {
	fconfigure $sock -buffering line
	fileevent $sock readable "scpAuth $sock"
	set busy 1
    } else {
	puts "*** SECOND CONNECTION FROM $addr:$port - HAD TO REJECT CONNECTION! ***"
	close $sock
    }	
}

proc GiveList {sock} {
    global main env
    if {[eof $sock] || [catch {gets $sock input}]} {
	close $sock
	puts "closed connection to $sock"
    } else {
	set dir [lindex [split $input \t] 0]
	set hmode [lindex [split $input \t] 1]

	if {$dir == "++InitListing"} {
	    if [info exists env(HOME)] {
		set dir $env(HOME)
	    } else {
		set dir "/"
	    }
	}

	# exit command
	if {$dir == "++CloseYourSelf"} {
	    close $sock
	    close $main
	    puts "SecPanel ListServer exiting..."
	    after 1000
	    exit
	}
	
	# make dir command
	if [string match "++MakeDir *" $dir] {
	    puts "making dir"
	    file mkdir "[string range $dir 10 end]"
	    return
	}

	# delete command
	if [string match "++DelFile *" $dir] {
	    set delfile "[string range $dir 10 end]"
	    puts "Deleting $delfile..."
	    if {[catch {file delete $delfile} err] > 0} {
		puts "$err"
	    }
	    return
	}

	# long listing command
	set dolist 0
	if [string match "++Listingl *" $dir] {
	    set ls "l"
	    set range 11
	    set dolist 1
	} elseif [string match "++Listingla *" $dir] {
	    set ls "la"
	    set range 12
	    set dolist 1
	}

	if {$dolist} {
	    set fl [open "| ls -$ls [quote_space [string range $dir $range end]]" r]
	    while {[gets $fl line] >= 0} {
		puts $sock $line
	    }
	    close $fl
	    puts $sock "+++++"
	    return
	}

	# default listing command
	if $hmode {
	    set gpat "glob -nocomplain \"$dir/.*\" \"$dir/*\""
	} else {
	    set gpat "glob -nocomplain \"$dir/*\""
	}
	
	# Checking rights and existance...

	puts $sock "+++++$dir"
	foreach f [lsort [eval $gpat]] {
	    if {[file tail "$f"] == ".." || [file tail "$f"] == "."} {
		continue
	    }
	    if [file isdirectory "$f"] {
		puts $sock "++ $f"
	    } else {
		puts $sock "$f"
	    }
	}
	puts $sock "+++++"
    }
}

proc quote_space {in} {
    regsub -all " " $in "\\ " out
    return $out
}

proc hostinfo {} {
    puts "-------------------------------------------------------------"
    puts "We are running [info nameofexecutable] on [info hostname]"
    puts "-------------------------------------------------------------\n"
}

hostinfo

set kc 0
while {[catch {set main [socket -server Accept -myaddr 127.0.0.1 <listserverport-XXXXX>]} err] > 0} {
    puts "Listserver error: $err"
    if {! $kc} {
	if [file exists "$env(HOME)/.listserver.pid"] {
	    puts "Trying to remove a previous ListServer..."
	    set pf [open "$env(HOME)/.listserver.pid" r]
	    exec kill -9 [read -nonewline $pf]
	    close $pf
	    set kc 1
	}
    } else {
	exit
    }
}

puts "SecPanel ListServer waiting for connections..."
set pf [open "$env(HOME)/.listserver.pid" w]
puts -nonewline $pf [pid]
close $pf

# Tell SecPanel that we are prepared to send filelistings
set callbacksocket [socket localhost <controlremoteport-XXXXX>]
fconfigure $callbacksocket -buffering line
puts $callbacksocket "opengui $clientpass"
puts "Sent CallBack to SecPanel"
close $callbacksocket

vwait forever
