#!/bin/sh
#  -*- tcl -*-
# Executing wish #\
exec wish "$0" "$@"



#############################################################################
# Visual Tcl v1.22 Project
#

#################################
# GLOBAL VARIABLES
#
global widget; 

#################################
# USER DEFINED PROCEDURES
#

proc {main} {argc argv} {
    global mode
    set mode [lindex $argv 0]
    
    if [catch {exec which gpg} err] {
	.top34.fra36.ent41 insert 0 "No GnuPG for encryption found!"
	.top34.fra36.ent41 config -show "" -state disabled
	.top34.fra37.but39 config -command exit
    }
}

proc direader {dir ts} {
    global env

    set return ""
    set tarfile [open "$env(HOME)/.secpanel/.tarfiles" w]
    foreach f  [lsort [glob -nocomplain $dir/* $dir/.*]] {
	if {[regexp "\/\\.\\.$" $f] || [regexp "\/\\.$" $f]} {
	    continue
	}
	puts $tarfile $f
    }
    close $tarfile

    return $return
}

proc {returnpw} {} {
    global widget env mode
    set ts [clock clicks]
    exec echo [.top34.fra36.ent41 get] > $env(HOME)/.secpanel/.pw_$ts
    
    if {$mode == "de"} {
	catch {exec gpg -q --no-verbose --passphrase-file $env(HOME)/.secpanel/.pw_$ts -d $env(HOME)/.secpanel/spdata.lck | tar tzfv -} err
	exec echo $err > $env(HOME)/.secpanel/err
    } elseif {$mode == "en"} {
	direader "$env(HOME)/.secpanel" $ts
	catch {exec tar -c -z -f - -T $env(HOME)/.secpanel/.tarfiles | gpg -q --no-verbose -c --passphrase-file pf > $env(HOME)/.secpanel/spdata.lck} err
	exec echo $err > $env(HOME)/.secpanel/err
    } else {
	direader "$env(HOME)/.secpanel" $ts
	puts "no valid operation mode..."
	exit
    }
    
    file delete -force "$env(HOME)/.secpanel/.pw_$ts"
    exit
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

proc init {argc argv} {

}

init $argc $argv

#################################
# VTCL GENERATED GUI PROCEDURES
#

proc vTclWindow. {base {container 0}} {
    if {$base == ""} {
        set base .
    }
    ###################
    # CREATING WIDGETS
    ###################
    if {!$container} {
    wm focusmodel $base passive
    wm geometry $base 200x200+0+0
    wm maxsize $base 1284 1002
    wm minsize $base 115 1
    wm overrideredirect $base 0
    wm resizable $base 1 1
    wm withdraw $base
    wm title $base "vt"
    }
    ###################
    # SETTING GEOMETRY
    ###################
}

proc vTclWindow.top34 {base {container 0}} {
    if {$base == ""} {
        set base .top34
    }
    if {[winfo exists $base] && (!$container)} {
        wm deiconify $base; return
    }
    ###################
    # CREATING WIDGETS
    ###################
    if {!$container} {
    toplevel $base -class Toplevel \
        -borderwidth "4" 
    wm focusmodel $base passive
    wm geometry $base 204x92+344+247
    wm maxsize $base 1284 1002
    wm minsize $base 115 1
    wm overrideredirect $base 0
    wm resizable $base 1 1
    wm deiconify $base
    wm title $base "SecPanel - Data Protection"
    }
    frame $base.fra35 \
        -relief "groove" -height "75" -width "125" 
    label $base.fra35.lab40 \
        -anchor "nw" -borderwidth "0" -relief "raised" \
        -text "Enter password:" 
    frame $base.fra36 \
        -relief "groove" -height "75" -width "125" 
    entry $base.fra36.ent41 -show "*\#*"
    bind $base.fra36.ent41 <Key-Return> {
        returnpw
    }

    frame $base.fra37 \
        -borderwidth "2" -relief "groove" -height "75" -width "125" 
    button $base.fra37.but38 \
        -relief "groove" -text "Cancel" -command exit 
    button $base.fra37.but39 \
        -command "returnpw" -relief "groove" -text "Continue" 
    ###################
    # SETTING GEOMETRY
    ###################
    grid columnconf $base 0 -weight 1
    grid rowconf $base 0 -weight 1
    grid $base.fra35 \
        -in $base -column "0" -row "0" -columnspan "1" -rowspan "1" -padx "2" \
        -pady "2" -sticky "nesw" 
    grid columnconf $base.fra35 0 -weight 1
    grid rowconf $base.fra35 0 -weight 1
    grid $base.fra35.lab40 \
        -in $base.fra35 -column "0" -row "0" -columnspan "1" -rowspan "1" \
        -sticky "nesw" 
    grid $base.fra36 \
        -in $base -column "0" -row "1" -columnspan "1" -rowspan "1" -padx "2" \
        -pady "2" -sticky "ew" 
    grid columnconf $base.fra36 0 -weight 1
    grid $base.fra36.ent41 \
        -in $base.fra36 -column "0" -row "0" -columnspan "1" -rowspan "1" \
        -sticky "ew" 
    grid $base.fra37 \
        -in $base -column "0" -row "2" -columnspan "1" -rowspan "1" -padx "2" \
        -pady "2" -sticky "ew" 
    grid $base.fra37.but38 \
        -in $base.fra37 -column "0" -row "0" -columnspan "1" -rowspan "1" 
    grid $base.fra37.but39 \
        -in $base.fra37 -column "1" -row "0" -columnspan "1" -rowspan "1" 
}

Window show .
Window show .top34

tkwait visibility .top34
focus .top34.fra36.ent41
grab .top34

main $argc $argv
