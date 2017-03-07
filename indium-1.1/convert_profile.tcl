#

###############################################################
# SecPanel helper
# Converting from old profile seetings to new version (0.5)
###############################################################

proc cp {} {
    global env

    if {! [file exists "$env(HOME)/.indimail/sites"]} {
	puts "No sites-file to convert"
	return
    }

    set sf [open "$env(HOME)/.indimail/sites"]

    while {[gets $sf line] >= 0} {
	set nf [open "$env(HOME)/.indimail/profiles/newfiles.tmp" w]

	set sitetitle [lindex [split $line \#] 0]
	set sitehost [lindex [split $line \#] 1]
	set siteuser [lindex [split $line \#] 2]

	set defs [open "$env(HOME)/.indimail/default.profile"]
	while {[gets $defs defline] >= 0} {
	    switch -regexp $defline {
		"^set title .*" {
		    regsub -all " " $sitetitle "" filetitle
		    puts $nf "set title \"$sitetitle\""
		}
		"^set host .*" {
		    puts $nf "set host \"$sitehost\""
		}
		"^set user .*" {
		    puts $nf "set user \"$siteuser\""
		}
		default {
		    puts $nf $defline
		}
	    }
	}
	close $defs
	close $nf
	file copy -force "$env(HOME)/.indimail/profiles/newfiles.tmp" "$env(HOME)/.indimail/profiles/$filetitle.profile"
	unset filetitle
	file delete -force "$env(HOME)/.indimail/profiles/newfiles.tmp"
    }
    close $sf
}

proc do_backup {} {
    global env
    set butime [clock seconds]
    file copy -force "$env(HOME)/.indimail" "$env(HOME)/secpanel_backup$butime"
    puts "Put Backup of $env(HOME)/.indimail to $env(HOME)/secpanel_backup$butime"
}

proc moveprofs {} {
    global env
    file delete -force "$env(HOME)/.indimail/default.profile"
    foreach file [glob -nocomplain "$env(HOME)/.indimail/*.profile"] {
	file copy -force "$file" "$env(HOME)/.indimail/profiles"
	file delete -force "$file"
    }
    file delete -force "$env(HOME)/.indimail/sites"
}

puts "Doing profile conversion..."
do_backup
file mkdir "$env(HOME)/.indimail/profiles"
cp
moveprofs
