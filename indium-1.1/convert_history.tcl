# convert_history.tcl
#
# converts SecPanel's old history file format to new format
# implemented for SecPanel 0.40 and newer

proc ch {} {
    global env

    set hfile "$env(HOME)/.indimail/history"
    set hfile_backup "$env(HOME)/.indimail/history_backup"
    set thfile "$env(HOME)/.indimail/history.temp"

    if {! [file exists $hfile]} {
	puts "No history-file to convert"
	return
    }

    file copy -force $hfile $hfile_backup

    set hf [open "$hfile" r]
    set thf [open "$thfile" w]

    while {[gets $hf line] >= 0} {
	regsub " - " $line ";" newline
	
	set type [lindex [split $newline "\#"] 0]
	set date [lindex [split [lindex [split $newline "#"] 1] ";"] 0]
	set action [lindex [split [lindex [split $newline "#"] 1] ";"] 1]

	if [catch {set newdate [clock scan $date]} err] {
	    puts "Found history file already updated..."
	    file copy -force $hfile_backup $hfile
	    return
	}

	puts $thf "$type\#[clock scan $date]\#$action"
    }

    close $thf
    close $hf

    file copy -force $thfile $hfile
    file delete -force $thfile

    puts "Finished converting history"
}

ch
