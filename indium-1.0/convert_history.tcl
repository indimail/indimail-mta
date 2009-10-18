# convert_history.tcl
#
# converts SecPanel's old history file format to new format
# implemented for SecPanel 0.40


set hfile "$env(HOME)/.secpanel/history"
set thfile "$env(HOME)/.secpanel/history.temp"

if {! [file exists $hfile]} {
    puts "No history-file to convert"
    return
}

set hf [open "$hfile" r]
set thf [open "$thfile" w]

while {[gets $hf line] >= 0} {
    regsub " - " $line ";" newline
    
    set type [lindex [split $newline "\#"] 0]
    set date [lindex [split [lindex [split $newline "#"] 1] ";"] 0]
    set action [lindex [split [lindex [split $newline "#"] 1] ";"] 1]

    puts $thf "$type\#[clock scan $date]\#$action"
}

close $thf
close $hf

file copy -force $thfile $hfile
file delete -force $thfile

puts "Finished converting history"
