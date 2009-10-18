#!/usr/bin/wish
#############################################################################
# Visual Tcl v1.20 Project
#

#################################
# GLOBAL VARIABLES
#
global widget; 
    set widget(askeduser) {.top21.fra22.ent24}
    set widget(backbutton) {.top17.fra29.but33}
    set widget(contbutton) {.top17.fra29.but31}
    set widget(finishframe) {.top17.fra42}
    set widget(finishmessage) {.top17.fra42.mes44}
    set widget(hostframe) {.top17.fra34}
    set widget(hostlist) {.top17.fra34.cpd35.01}
    set widget(keyentry) {.top17.fra20.fra26.ent28}
    set widget(keyframe) {.top17.fra20}
    set widget(keylist) {.top17.fra20.cpd22.01}
    set widget(messageline) {.top17.lab17}
    set widget(rev,.top17.fra20) {keyframe}
    set widget(rev,.top17.fra20.cpd22.01) {keylist}
    set widget(rev,.top17.fra20.fra26.ent28) {keyentry}
    set widget(rev,.top17.fra29.but31) {contbutton}
    set widget(rev,.top17.fra29.but33) {backbutton}
    set widget(rev,.top17.fra34) {hostframe}
    set widget(rev,.top17.fra34.cpd35.01) {hostlist}
    set widget(rev,.top17.fra39) {userframe}
    set widget(rev,.top17.fra39.01.02) {userlist}
    set widget(rev,.top17.fra42) {finishframe}
    set widget(rev,.top17.fra42.mes44) {finishmessage}
    set widget(rev,.top17.lab17) {messageline}
    set widget(rev,.top21.fra22.ent24) {askeduser}
    set widget(userframe) {.top17.fra39}
    set widget(userlist) {.top17.fra39.01.02}

#################################
# USER DEFINED PROCEDURES
#

set returnstatus [catch {set indimaildir [eval {exec grep ^indimail /etc/passwd | awk -F: {{print $6}}}]} result]
if {$returnstatus != 0} {
	puts stdout "failed to get home dir for indimail"
	exit 1
}
set libdir $indimaildir/lib/secpanel
set bindir $indimaildir/bin


proc init {argc argv} {
    global env configs
    
    source "$env(HOME)/.indimail/config"
    initconfigs
    
    array set gshorts {fore foreground back background}
    
    foreach gcolval [array names gshorts] {
	if [info exists configs($gcolval)] {
	    option add *$gshorts($gcolval) $configs($gcolval)
	}
    }
    
    array set shorts {entfore Entry.foreground entback Entry.background \
			  listfore Listbox.foreground listback Listbox.background}
    
    foreach colval [array names shorts] {
	if [info exists configs($colval)] {
	    option add *$shorts($colval) $configs($colval)
	    if {$colval == "listfore"} {
		option add *Text.foreground $configs($colval)		
	    }
	    if {$colval == "listback"} {
		option add *Text.background $configs($colval)		
	    }
	}
    }
    
    set fb ""
    set fi ""
    
    if [info exists configs(fontbold)] {
	if $configs(fontbold) {
	    set fb "bold"
	}
    }
    
    if [info exists configs(fontitalic)] {
	if $configs(fontitalic) {
	    set fi "italic"
	}
    }
    
    if {[info exists configs(fontfam)] && [info exists configs(fontsize)]} {
	option add *Font "\"$configs(fontfam)\" $configs(fontsize) $fb $fi"
    }
}

proc initconfigs {} {
    global configs

    foreach {bintag binprog} \
	    {sshbin ssh xtermbin xterm} {
	if {! [info exists configs($bintag)]} {
	    set configs($bintag) $binprog
	}
    }

    if {! [info exists configs(sshver)]} {
	set configs(sshver) "OpenSSH"
    }

    if {! [info exists configs(termver)]} {
	set configs(termver) "Xterm"
    }
}

init $argc $argv

proc getuser {} {
    global widget userres
    set userres [$widget(askeduser) get]
}

proc askforuser {"mode c"} {
    global userres widget
    set old [focus]
    Window show .top21

    tkwait visibility .top21
    focus $widget(askeduser)
    grab .top21
    tkwait variable userres
    grab release .top21
    focus $old
    Window destroy .top21
    return $userres
}

proc historyman {"categ {}" "text {}"} {
    global env
    set hf [open "$env(HOME)/.indimail/history" a]
    puts $hf "$categ#[clock seconds]#$text"
    close $hf
}

proc {distkey} {key host user} {
    global env widget libdir configs
    
    if {$user == "<ASKFORUSER>"} {
	set user [askforuser]
	if {$user == "#####"} {
	    return
	}
    }
    
    switch -regexp $configs(termver) {
	"GNOME-Term" {
	    set titlepar "-t"
	    set quotepar "\""
	}
	"KDE-Term" {
	}
	"Xterm|Rxvt|Aterm" {
	    set titlepar "-title"
	    set quotepar ""	    
	}
	"Eterm" {
	    set titlepar "-T"
	    set quotepar ""	    
	}
    }

    set dfile "$env(HOME)/.indimail/.runfiles/keydist.[clock clicks]"
    set df [open "$dfile" w]

    set actstring "exec $configs(xtermbin) $titlepar \"SecPanel Key-Distribution\"  -e $quotepar $libdir/secpanel.dist $host $user $key $configs(sshbin) $quotepar"
    puts $df $actstring

    close $df
    exec chmod +x $dfile
    exec $dfile &
    historyman 5 "$key -> $user@$host"
}

proc {do_cancel} {} {
    exit -1
}

proc {go} {mode direct} {
    global widget key host user
    switch -exact $mode {
	"key" {
	    switchtab "key"
	    $widget(contbutton) config -command "go host f" -text "Continue" -state "active"
	    $widget(backbutton) config -state "disabled" -command ""
	    showmessage ""
	}
	"host" {
	    if {$direct == "f"} {
		set key [$widget(keyentry) get]
		if {$key == ""} {
		    showmessage "Please choose key!"
		    return
		}
	    }
	    switchtab "host"
	    $widget(contbutton) config -command "go user f" -text "Continue" -state "active"
	    $widget(backbutton) config -command "go key b" -state "active"
	    showmessage ""
	}
	"user" {
	    if {$direct == "f"} {
		if {[selection own] != $widget(hostlist)} {
		    showmessage "Please choose host!"
		    return
		}
		set host [$widget(hostlist) get active]
	    }
	    switchtab "user"
	    $widget(contbutton) config -command "go finish f" -text "Finish" -state "active"
	    $widget(backbutton) config -command "go host b" -state "active"
	    showmessage ""
	}
	"finish" {
	    if {$direct == "f"} {
		if {[selection own] != $widget(userlist)} {
		    showmessage "Please choose user!"
		    return
		}
		set user [$widget(userlist) get active]
	    }
	    switchtab "finish"
	    $widget(contbutton) config -state "disabled"
	    $widget(backbutton) config -state "disabled"
	    showmessage ""

	    $widget(finishmessage) config -text "Distributing\n$key\nto account\n$user@$host"
	    distkey "\"$key\"" "$host" "$user"
	}
    }
}

proc {list_hosts_users} {} {
global widget env
    
    set sitefile "$env(HOME)/.indimail/sites"
    if [file exists $sitefile] {
	set sf [open $sitefile r]
	while {[gets $sf line] >= 0} {
	    set els [split $line \#]
	    set sites([lindex $els 0]) "[lindex $els 1]#[lindex $els 2]"
	}
	close $sf
    }

    $widget(keylist) delete 0 end
    $widget(userlist) delete 0 end

    set disthosts [list]
    set distusers [list]

    lappend distusers $env(USER)

    foreach s [array names sites] {
	set he [lindex [split $sites($s) '#'] 0]
	set ue [lindex [split $sites($s) '#'] 1]
	set hfound 0
	set ufound 0

	set usercheck 1

	if {$ue == ""} {
	    set usercheck 0
	}

	foreach hd $disthosts {
	    if {$he == $hd} {
		set hfound 1
		break
	    }
	}

	if {$hfound != 1} {
	    lappend disthosts $he
	}

	if {$usercheck} {
	    foreach ud $distusers {
		if {$ue == $ud} {
		    set ufound 1
		    break
		}
	    }
	    if {$ufound != 1} {
		lappend distusers $ue
	    }
	}
    }

    set profiles [glob -nocomplain "$env(HOME)/.indimail/*.profile"]
    foreach prof $profiles {
	if {[file rootname [file tail $prof]] == "default"} {
	    continue
	}
	source $prof

	set he $host
	set ue $user
	set hfound 0
	set ufound 0

	foreach hd $disthosts {
	    if {$he == $hd} {
		set hfound 1
		break
	    }
	}
	if {$hfound != 1} {
	    lappend disthosts $he
	}

	foreach ud $distusers {
	    if {$ue == $ud} {
		set ufound 1
		break
	    }
	}
	if {$ufound != 1} {
	    lappend distusers $ue
	}
	
	unset lfs
    }

    foreach hent [lsort $disthosts] {
	$widget(hostlist) insert end $hent
    }

    foreach uent [lsort $distusers] {
	$widget(userlist) insert end $uent
    }
}

proc {showmessage} {text} {
global widget
    $widget(messageline) config -text "$text"
}

proc {switchtab} {mode} {
global widget
    foreach f {key host user} {
	if {[grid info $widget([set f]frame)] != ""} {
	    grid remove $widget([set f]frame)
	}
    }
    
    grid $widget([set mode]frame) -in .top17  -column 0 -row 1 -columnspan 1 -rowspan 1  -ipadx 2 -ipady 2 -padx 2 -pady 2 -sticky nesw
}

proc {main} {argc argv} {
go key f
    list_hosts_users
}

proc {Window} {args} {
global vTcl
    set cmd [lindex $args 0]
    set name [lindex $args 1]
    set newname [lindex $args 2]
    set rest [lrange $args 3 end]
    if {$name == "" || $cmd == ""} {return}
    if {$newname == ""} {
        set newname $name
    }
    set exists [winfo exists $newname]
    switch $cmd {
        show {
            if {$exists == "1" && $name != "."} {wm deiconify $name; return}
            if {[info procs vTclWindow(pre)$name] != ""} {
                eval "vTclWindow(pre)$name $newname $rest"
            }
            if {[info procs vTclWindow$name] != ""} {
                eval "vTclWindow$name $newname $rest"
            }
            if {[info procs vTclWindow(post)$name] != ""} {
                eval "vTclWindow(post)$name $newname $rest"
            }
        }
        hide    { if $exists {wm withdraw $newname; return} }
        iconify { if $exists {wm iconify $newname; return} }
        destroy { if $exists {destroy $newname; return} }
    }
}

#################################
# VTCL GENERATED GUI PROCEDURES
#

proc vTclWindow. {base} {
    if {$base == ""} {
        set base .
    }
    ###################
    # CREATING WIDGETS
    ###################
    wm focusmodel $base passive
    wm geometry $base 1x1+0+0
    wm maxsize $base 1009 738
    wm minsize $base 1 1
    wm overrideredirect $base 0
    wm resizable $base 1 1
    wm withdraw $base
    wm title $base "vt.tcl"
    ###################
    # SETTING GEOMETRY
    ###################
}

proc vTclWindow.top17 {base} {
	variable fname
    if {$base == ""} {
        set base .top17
    }
    if {[winfo exists $base]} {
        wm deiconify $base; return
    }
    ###################
    # CREATING WIDGETS
    ###################
    toplevel $base -class Toplevel \
        -borderwidth "4" 
    wm focusmodel $base passive
    wm geometry $base 394x289+162+281
    wm maxsize $base 1265 930
    wm minsize $base 1 1
    wm overrideredirect $base 0
    wm resizable $base 1 1
    wm deiconify $base
    wm title $base "SecPanel - Key distribution"
    frame $base.fra18 \
        -borderwidth "2" -height "75" -relief "groove" -width "125" 
    label $base.fra18.lab21 \
        -borderwidth "0" -font "{Helvetica} -16 bold" \
        -text "SecPanel key distribution" 
    frame $base.fra20 \
        -height "75" -width "125" 
    frame $base.fra20.cpd22 \
        -height "30" -width "30" 
    listbox $base.fra20.cpd22.01 \
        -borderwidth "1" \
        -font "-Adobe-Helvetica-Medium-R-Normal-*-*-120-*-*-*-*-*-*" \
        -xscrollcommand ".top17.fra20.cpd22.02 set" \
        -yscrollcommand ".top17.fra20.cpd22.03 set" 
    scrollbar $base.fra20.cpd22.02 \
        -borderwidth "1" -command ".top17.fra20.cpd22.01 xview" \
        -orient "horizontal" -width "10" 
    scrollbar $base.fra20.cpd22.03 \
        -borderwidth "1" -command ".top17.fra20.cpd22.01 yview" -width "10" 
    label $base.fra20.lab23 \
        -borderwidth "0" -text "Select which key to distribute" 
    frame $base.fra20.fra26 \
        -borderwidth "2" -height "75" -relief "groove" -width "125" 
    button $base.fra20.fra26.but27 \
        -borderwidth "1" -text "Browse" -command {set fname [tk_getOpenFile -initialdir "$env(HOME)/.ssh/"]}
    entry $base.fra20.fra26.ent28 \
        -borderwidth "1" -textvariable fname
    label $base.fra20.lab32 \
        -anchor "w" -borderwidth "0" -text "Available keys:" 
    frame $base.fra29 \
        -borderwidth "2" -height "75" -relief "groove" -width "125" 
    button $base.fra29.but30 \
        -borderwidth "1" -command "do_cancel" -text "Cancel" 
    button $base.fra29.but31 \
        -borderwidth "1" -command "go host f" -state "active" \
        -text "Continue" 
    button $base.fra29.but33 \
        -borderwidth "1" -state "disabled" -text "Back" 
    frame $base.fra34 \
        -height "75" -width "125" 
    frame $base.fra34.cpd35 \
        -borderwidth "1" -height "30" -width "30" 
    listbox $base.fra34.cpd35.01 \
        -borderwidth "1" \
        -font "-Adobe-Helvetica-Medium-R-Normal-*-*-120-*-*-*-*-*-*" \
        -xscrollcommand ".top17.fra34.cpd35.02 set" \
        -yscrollcommand ".top17.fra34.cpd35.03 set" 
    scrollbar $base.fra34.cpd35.02 \
        -borderwidth "1" -command ".top17.fra34.cpd35.01 xview" \
        -orient "horizontal" -width "10" 
    scrollbar $base.fra34.cpd35.03 \
        -borderwidth "1" -command ".top17.fra34.cpd35.01 yview" -width "10" 
    label $base.fra34.lab36 \
        -anchor "w" -borderwidth "0" -justify "left" \
        -text "Select host to
distribute the key to:" 
    frame $base.fra39 \
        -height "75" -width "125" 
    frame $base.fra39.01 \
        -borderwidth "1" -height "30" -width "30" 
    listbox $base.fra39.01.02 \
        -borderwidth "1" \
        -font "-Adobe-Helvetica-Medium-R-Normal-*-*-120-*-*-*-*-*-*" \
        -xscrollcommand ".top17.fra39.01.03 set" \
        -yscrollcommand ".top17.fra39.01.04 set" 
    scrollbar $base.fra39.01.03 \
        -borderwidth "1" -command ".top17.fra39.01.02 xview" \
        -orient "horizontal" -width "10" 
    scrollbar $base.fra39.01.04 \
        -borderwidth "1" -command ".top17.fra39.01.02 yview" -width "10" 
    label $base.fra39.05 \
        -anchor "w" -borderwidth "0" -justify "left" \
        -text "Select user to
distribute the key as:" 
    frame $base.fra42 \
        -height "75" -width "125" 
    message $base.fra42.mes44 \
        -padx "2" -pady "2" -text "message" -aspect "500" 
    label $base.lab17 \
        -anchor "w" -borderwidth "0" 
    ###################
    # SETTING GEOMETRY
    ###################
    grid columnconf $base 0 -weight 1
    grid rowconf $base 1 -weight 1
    grid $base.fra18 \
        -in ".top17" -column "0" -row "0" -columnspan "1" -rowspan "1" \
        -ipadx "2" -ipady "2" -padx "2" -pady "2" -sticky "ew" 
    grid $base.fra18.lab21 \
        -in ".top17.fra18" -column "0" -row "1" -columnspan "1" -rowspan "1" 
    grid $base.fra20 \
        -in ".top17" -column "0" -row "1" -columnspan "1" -rowspan "1" \
        -ipadx "2" -ipady "2" -padx "2" -pady "2" -sticky "nesw" 
    grid columnconf $base.fra20 1 -weight 1
    grid rowconf $base.fra20 1 -weight 1
    grid rowconf $base.fra20 2 -weight 1
    grid $base.fra20.cpd22 \
        -in ".top17.fra20" -column "1" -row "1" -columnspan "1" -rowspan "2" \
        -sticky "nesw" 
    grid columnconf $base.fra20.cpd22 0 -weight 1
    grid rowconf $base.fra20.cpd22 0 -weight 1
    grid $base.fra20.cpd22.01 \
        -in ".top17.fra20.cpd22" -column "0" -row "0" -columnspan "1" \
        -rowspan "1" -sticky "nesw" 
    grid $base.fra20.cpd22.02 \
        -in ".top17.fra20.cpd22" -column "0" -row "1" -columnspan "1" \
        -rowspan "1" -sticky "ew" 
    grid $base.fra20.cpd22.03 \
        -in ".top17.fra20.cpd22" -column "1" -row "0" -columnspan "1" \
        -rowspan "1" -sticky "ns" 
    grid $base.fra20.lab23 \
        -in ".top17.fra20" -column "0" -row "0" -columnspan "1" -rowspan "1" \
        -sticky "ew" 
    grid $base.fra20.fra26 \
        -in ".top17.fra20" -column "0" -row "1" -columnspan "1" -rowspan "1" \
        -sticky "new" 
    grid columnconf $base.fra20.fra26 0 -weight 1
    grid $base.fra20.fra26.but27 \
        -in ".top17.fra20.fra26" -column "0" -row "1" -columnspan "1" \
        -rowspan "1" -sticky "w" 
    grid $base.fra20.fra26.ent28 \
        -in ".top17.fra20.fra26" -column "0" -row "0" -columnspan "1" \
        -rowspan "1" -sticky "ew" 
    grid $base.fra20.lab32 \
        -in ".top17.fra20" -column "1" -row "0" -columnspan "1" -rowspan "1" \
        -sticky "ew" 
    grid $base.fra29 \
        -in ".top17" -column "0" -row "3" -columnspan "1" -rowspan "1" \
        -ipadx "2" -ipady "2" -padx "2" -pady "2" -sticky "ew" 
    grid columnconf $base.fra29 0 -weight 1
    grid columnconf $base.fra29 1 -weight 1
    grid $base.fra29.but30 \
        -in ".top17.fra29" -column "0" -row "0" -columnspan "1" -rowspan "1" \
        -sticky "w" 
    grid $base.fra29.but31 \
        -in ".top17.fra29" -column "2" -row "0" -columnspan "1" -rowspan "1" \
        -sticky "e" 
    grid $base.fra29.but33 \
        -in ".top17.fra29" -column "1" -row "0" -columnspan "1" -rowspan "1" \
        -sticky "e" 
    grid $base.fra34.cpd35 \
        -in ".top17.fra34" -column "1" -row "0" -columnspan "1" -rowspan "1" \
        -sticky "nesw" 
    grid columnconf $base.fra34.cpd35 0 -weight 1
    grid rowconf $base.fra34.cpd35 0 -weight 1
    grid $base.fra34.cpd35.01 \
        -in ".top17.fra34.cpd35" -column "0" -row "0" -columnspan "1" \
        -rowspan "1" -sticky "nesw" 
    grid $base.fra34.cpd35.02 \
        -in ".top17.fra34.cpd35" -column "0" -row "1" -columnspan "1" \
        -rowspan "1" -sticky "ew" 
    grid $base.fra34.cpd35.03 \
        -in ".top17.fra34.cpd35" -column "1" -row "0" -columnspan "1" \
        -rowspan "1" -sticky "ns" 
    grid $base.fra34.lab36 \
        -in ".top17.fra34" -column "0" -row "0" -columnspan "1" -rowspan "1" \
        -sticky "n" 
    grid $base.fra39.01 \
        -in ".top17.fra39" -column "1" -row "0" -columnspan "1" -rowspan "1" \
        -sticky "nesw" 
    grid columnconf $base.fra39.01 0 -weight 1
    grid rowconf $base.fra39.01 0 -weight 1
    grid $base.fra39.01.02 \
        -in ".top17.fra39.01" -column "0" -row "0" -columnspan "1" \
        -rowspan "1" -sticky "nesw" 
    grid $base.fra39.01.03 \
        -in ".top17.fra39.01" -column "0" -row "1" -columnspan "1" \
        -rowspan "1" -sticky "ew" 
    grid $base.fra39.01.04 \
        -in ".top17.fra39.01" -column "1" -row "0" -columnspan "1" \
        -rowspan "1" -sticky "ns" 
    grid $base.fra39.05 \
        -in ".top17.fra39" -column "0" -row "0" -columnspan "1" -rowspan "1" \
        -sticky "n" 
    grid $base.fra42.mes44 \
        -in ".top17.fra42" -column "0" -row "1" -columnspan "1" -rowspan "1" \
        -sticky "nesw" 
    grid $base.lab17 \
        -in ".top17" -column "0" -row "2" -columnspan "1" -rowspan "1" \
        -padx "2" -sticky "ew" 
}

proc vTclWindow.top21 {base} {
    if {$base == ""} {
        set base .top21
    }
    if {[winfo exists $base]} {
        wm deiconify $base; return
    }
    ###################
    # CREATING WIDGETS
    ###################
    toplevel $base -class Toplevel  -borderwidth "4" 
    wm focusmodel $base passive
    wm geometry $base 199x124
    wm maxsize $base 1265 994
    wm minsize $base 1 1
    wm overrideredirect $base 0
    wm resizable $base 1 1
    wm title $base "SecPanel - Ask for user"
    frame $base.fra22  -borderwidth "2" -height "75" -relief "groove" -width "125" 
    message $base.fra22.mes23  -anchor "w" -aspect "300" -padx "5" -pady "2"  -text "Give login to use for this connection:" 
    entry $base.fra22.ent24  -borderwidth "1" 
    bind $base.fra22.ent24 <Key-Return> {
        getuser
    }
    button $base.but25  -command "set userres #####" -padx "9" -pady "3" -relief "groove"  -text "Cancel" 
    button $base.but26  -command "getuser" -padx "9" -pady "3" -relief "groove"  -text "Connect" 
    ###################
    # SETTING GEOMETRY
    ###################
    grid columnconf $base 0 -weight 1
    grid columnconf $base 1 -weight 1
    grid rowconf $base 0 -weight 1
    grid $base.fra22  -in ".top21" -column "0" -row "0" -columnspan "2" -rowspan "1"  -ipadx "2" -ipady "2" -padx "2" -pady "2" -sticky "nesw" 
    grid columnconf $base.fra22 0 -weight 1
    grid rowconf $base.fra22 0 -weight 1
    grid $base.fra22.mes23  -in ".top21.fra22" -column "0" -row "0" -columnspan "1" -rowspan "1"  -sticky "nesw" 
    grid $base.fra22.ent24  -in ".top21.fra22" -column "0" -row "1" -columnspan "1" -rowspan "1"  -sticky "ew" 
    grid $base.but25  -in ".top21" -column "0" -row "1" -columnspan "1" -rowspan "1"  -padx "2" -pady "2" -sticky "ew" 
    grid $base.but26  -in ".top21" -column "1" -row "1" -columnspan "1" -rowspan "1"  -padx "2" -pady "2" -sticky "ew"
}


Window show .
Window show .top17

main $argc $argv
