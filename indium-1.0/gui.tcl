
############################
# code to load stock images


if {![info exist vTcl(sourcing)]} {
proc vTcl:rename {name} {

    regsub -all "\\." $name "_" ret
    regsub -all "\\-" $ret "_" ret
    regsub -all " " $ret "_" ret
    regsub -all "/" $ret "__" ret

    return [string tolower $ret]
}

proc vTcl:image:create_new_image {filename description type} {

	global vTcl env

	# image already existing ?
	if [info exists vTcl(images,files)] {

		set index [lsearch -exact $vTcl(images,files) $filename]

		if {$index != "-1"} {
			# cool, no more work to do
			return
		}
	}

	# wait a minute... does the file actually exist?
	if {! [file exists $filename] } {

		# try current directory
		set script [file dirname [info script]]
		set filename [file join $script [file tail $filename] ]

		# puts "looking for $filename..."
	}

	if {! [file exists $filename] } {

		set description "file not found!"

		set object [image create bitmap -data {
		    #define open_width 16
		    #define open_height 16
		    static char open_bits[] = {
			0x7F, 0xFE,
			0x41, 0x82,
			0x21, 0x81,
			0x41, 0x82,
			0x21, 0x81,
			0x21, 0x81,
			0x21, 0x81,
			0x91, 0x80,
			0x21, 0x81,
			0x91, 0x80,
			0x21, 0x81,
			0x21, 0x81,
			0x21, 0x81,
			0x41, 0x82,
			0x41, 0x82,
			0x7F, 0xFE};}]

	} else {

		set object [image create  [vTcl:image:get_creation_type $filename]  -file $filename]
	}

	set reference [vTcl:rename $filename]

	set vTcl(images,$reference,image)       $object
	set vTcl(images,$reference,description) $description
	set vTcl(images,$reference,type)        $type
	set vTcl(images,filename,$object)       $filename

	lappend vTcl(images,files) $filename

	# return image name in case caller might want it
	return $object
}

proc vTcl:image:get_image {filename} {

	global vTcl
	set reference [vTcl:rename $filename]

	# let's do some checking first
	if {! [info exists vTcl(images,$reference,image)] } {

		# well, the path may be wrong; in that case check
		# only the filename instead, without the path

		set imageTail [file tail $filename]

		foreach oneFile $vTcl(images,files) {

			if { [file tail $oneFile] == $imageTail } {

				set reference [vTcl:rename $oneFile]
				break
			}
		}
	}

	return $vTcl(images,$reference,image)
}

proc vTcl:image:get_creation_type {filename} {

	set ext [file extension $filename]
	set ext [string tolower $ext]

	switch $ext {

		.ppm -
		.gif    {return photo}
		.xbm    {return bitmap}

		default {return photo}
	}
}

vTcl:image:create_new_image "/usr/local/vtcl-1.2.2/images/edit/copy.gif" \
    "" \
    "stock"
vTcl:image:create_new_image "/usr/local/vtcl-1.2.2/images/edit/cut.gif" \
    "" \
    "stock"
vTcl:image:create_new_image "/usr/local/vtcl-1.2.2/images/edit/paste.gif" \
    "" \
    "stock"
vTcl:image:create_new_image "/usr/local/vtcl-1.2.2/images/edit/new.gif" \
    "" \
    "stock"
vTcl:image:create_new_image "/usr/local/vtcl-1.2.2/images/edit/open.gif" \
    "" \
    "stock"
vTcl:image:create_new_image "/usr/local/vtcl-1.2.2/images/edit/save.gif" \
    "" \
    "stock"
vTcl:image:create_new_image "/usr/local/vtcl-1.2.2/images/edit/replace.gif" \
    "" \
    "stock"
}
############################
# code to load user images

############################
# code to load stock fonts


if {![info exist vTcl(sourcing)]} {
set vTcl(fonts,counter) 0
proc vTcl:font:add_font {font_descr font_type newkey} {

     global vTcl

     incr vTcl(fonts,counter)
     set newfont [eval font create $font_descr]

     lappend vTcl(fonts,objects) $newfont

     # each font has its unique key so that when a project is
     # reloaded, the key is used to find the font description

     if {$newkey == ""} {
          set newkey vTcl:font$vTcl(fonts,counter)
     }

     set vTcl(fonts,$newfont,type)                      $font_type
     set vTcl(fonts,$newfont,key)                       $newkey
     set vTcl(fonts,$vTcl(fonts,$newfont,key),object)   $newfont

     # in case caller needs it
     return $newfont
}

proc vTcl:font:get_font {key} {

	global vTcl

	return $vTcl(fonts,$key,object)
}

vTcl:font:add_font \
    "-family helvetica -size 12 -weight normal -slant roman -underline 0 -overstrike 0" \
    stock \
    vTcl:font1
vTcl:font:add_font \
    "-family helvetica -size 12 -weight normal -slant roman -underline 1 -overstrike 0" \
    stock \
    underline
vTcl:font:add_font \
    "-family courier -size 12 -weight normal -slant roman -underline 0 -overstrike 0" \
    stock \
    vTcl:font3
vTcl:font:add_font \
    "-family times -size 12 -weight normal -slant roman -underline 0 -overstrike 0" \
    stock \
    vTcl:font4
vTcl:font:add_font \
    "-family helvetica -size 12 -weight bold -slant roman -underline 0 -overstrike 0" \
    stock \
    vTcl:font5
vTcl:font:add_font \
    "-family courier -size 12 -weight bold -slant roman -underline 0 -overstrike 0" \
    stock \
    vTcl:font6
vTcl:font:add_font \
    "-family times -size 12 -weight bold -slant roman -underline 0 -overstrike 0" \
    stock \
    vTcl:font7
vTcl:font:add_font \
    "-family lucida -size 18 -weight normal -slant roman -underline 0 -overstrike 0" \
    stock \
    vTcl:font8
vTcl:font:add_font \
    "-family lucida -size 18 -weight normal -slant italic -underline 0 -overstrike 0" \
    stock \
    vTcl:font9
}
############################
# code to load user fonts

vTcl:font:add_font \
    "-family fixed -size 9 -weight normal -slant roman -underline 0 -overstrike 0" \
    user \
    vTcl:font10
#############################################################################
# Visual Tcl v1.22 Project
#

#################################
# GLOBAL VARIABLES
#
global algo; 
global askuserspec; 
global compress; 
global compressval; 
global configs; 
global connwait; 
global fork; 
global gateway; 
global ipverconnect; 
global launcher; 
global noagentforward; 
global noexec; 
global nopriv; 
global quiet; 
global sshverconnect; 
global stricthost; 
global termicon; 
global verbose; 
global widget; 
    set widget(.vTcl.fontmgr.listbox) {.vTcl.fontmgr.fra28.cpd29.01}
    set widget(.vTcl.fontmgr.text) {.vTcl.fontmgr.cpd43.03}
    set widget(addent) {.top17.fra44.fra19.fra28.02}
    set widget(agentent) {.top17.fra44.fra19.fra27.02}
    set widget(askeduser) {.top21.fra22.ent24}
    set widget(askpassent) {.top17.fra44.fra19.fra29.02}
    set widget(backbutton) {.top53.fra29.but33}
    set widget(browserent) {.top17.fra44.fra19.fra18.02}
    set widget(cfgfileent) {.top17.fra21.fra24.ent21}
    set widget(commandent) {.top17.fra21.fra24.ent26}
    set widget(comment) {.top17.fra21.ent23}
    set widget(commentent) {.top17.ent32}
    set widget(comprlev) {.top17.fra21.fra25.02}
    set widget(configsbut) {.top17.fra30.fra19.but24}
    set widget(indadminbut) {.top17.fra30.fra19.but25}
    set widget(confirmframe) {.top53.fra42}
    set widget(connectsbut) {.top17.fra30.fra19.but20}
    set widget(contbutton) {.top53.fra29.but31}
    set widget(defsites) {.top17.fra46.fra24.fra25.01}
    set widget(distconnlist) {.top53.fra34.cpd35.01}
    set widget(distkeyent) {.top17.fra35.ent40}
    set widget(distkeyentry) {.top53.fra20.fra26.ent28}
    set widget(distkeylist) {.top53.fra20.cpd22.01}
    set widget(distuserlist) {.top53.fra39.01.02}
    set widget(finishmessage) {.top53.fra42.mes44}
    set widget(host) {.top17.fra21.ent25}
    set widget(hostent) {.top17.fra21.fra24.ent20}
    set widget(hostframe) {.top53.fra34}
    set widget(hostkeyview) {.top19.cpd21.03}
    set widget(identityent) {.top17.fra21.fra24.ent19}
    set widget(identpath) {.top52.fra18.ent26}
    set widget(idents) {.top17.fra35.fra20.fra22.01}
    set widget(inport) {.top17.fra21.ent27}
    set widget(keydisthost) {.top17.fra35.fra17.cpd29.01}
    set widget(keydistkey) {.top17.fra35.fra17.ent23}
    set widget(keydistuser) {.top17.fra35.fra17.fra17.01}
    set widget(keyframe) {.top53.fra20}
    set widget(keygenent) {.top17.fra44.fra19.fra26.02}
    set widget(keylist) {.top52.fra34.cpd17.01}
    set widget(keysbut) {.top17.fra30.fra19.but23}
    set widget(knownhosts) {.top50.fra51.cpd54.01}
    set widget(lfcomment) {.top43.lab25}
    set widget(lfcommentent) {.top43.fra45.ent34}
    set widget(lfhost) {.top43.fra45.ent17}
    set widget(lfin) {.top43.fra45.ent46}
    set widget(lforwards) {.top43.cpd44.01}
    set widget(lfout) {.top43.fra45.ent48}
    set widget(messageline) {.top53.lab17}
    set widget(newaddr) {.top40.fra21.ent25}
    set widget(newtit) {.top40.fra21.ent23}
    set widget(newuser) {.top40.fra21.ent27}
    set widget(outport) {.top17.fra21.ent29}
    set widget(pe) {.top17.fra21.fra48.fra19.ent20}
    set widget(portent) {.top17.fra21.fra24.ent24}
    set widget(profileent) {.top17.fra21.fra48.fra19.ent20}
    set widget(profiles) {.top17.fra21.fra48.cpd18.01}
    set widget(profilesbut) {.top17.fra30.fra19.but22}
    set widget(proplabel) {.top20.lab36}
    set widget(remotekeysfile) {.top40.fra54.cpd57.03}
    set widget(remotercfile) {.top40.fra42.cpd49.03}
    set widget(remoteshostsfile) {.top40.fra41.cpd50.03}
    set widget(rev,.top17.cpd18.03) {textfield}
    set widget(rev,.top17.cpd31.01) {textfield}
    set widget(rev,.top17.ent32) {commentent}
    set widget(rev,.top17.fra17.lab18) {status}
    set widget(rev,.top17.fra17.lab19) {statusagent}
    set widget(rev,.top17.fra21.ent23) {comment}
    set widget(rev,.top17.fra21.ent25) {host}
    set widget(rev,.top17.fra21.ent27) {inport}
    set widget(rev,.top17.fra21.ent29) {outport}
    set widget(rev,.top17.fra21.fra24.ent18) {titleent}
    set widget(rev,.top17.fra21.fra24.ent19) {identityent}
    set widget(rev,.top17.fra21.fra24.ent20) {hostent}
    set widget(rev,.top17.fra21.fra24.ent21) {cfgfileent}
    set widget(rev,.top17.fra21.fra24.ent22) {userent}
    set widget(rev,.top17.fra21.fra24.ent24) {portent}
    set widget(rev,.top17.fra21.fra24.ent26) {commandent}
    set widget(rev,.top17.fra21.fra24.ent42) {subsysent}
    set widget(rev,.top17.fra21.fra25.02) {comprlev}
    set widget(rev,.top17.fra21.fra25.fra23.ent26) {identityent}
    set widget(rev,.top17.fra21.fra48.cpd18.01) {profiles}
    set widget(rev,.top17.fra21.fra48.fra19.ent20) {pe}
    set widget(rev,.top17.fra27.fra30.cpd31.01) {scphosts}
    set widget(rev,.top17.fra27.fra30.cpd32.01) {scpusers}
    set widget(rev,.top17.fra30.but26) {sscreenbut}
    set widget(rev,.top17.fra30.fra19.but20) {connectsbut}
    set widget(rev,.top17.fra30.fra19.but21) {scpbut}
    set widget(rev,.top17.fra30.fra19.but22) {profilesbut}
    set widget(rev,.top17.fra30.fra19.but23) {keysbut}
    set widget(rev,.top17.fra30.fra19.but24) {configsbut}
    set widget(rev,.top17.fra30.fra19.but25) {indadminbut}
    set widget(rev,.top17.fra35.ent40) {distkeyent}
    set widget(rev,.top17.fra35.fra17.cpd29.01) {keydisthost}
    set widget(rev,.top17.fra35.fra17.ent23) {keydistkey}
    set widget(rev,.top17.fra35.fra17.fra17.01) {keydistuser}
    set widget(rev,.top17.fra35.fra17.fra33.ent35) {keydistuser}
    set widget(rev,.top17.fra35.fra20.fra22.01) {idents}
    set widget(rev,.top17.fra44.ent25) {sshbinent}
    set widget(rev,.top17.fra44.fra17.ent19) {xterment}
    set widget(rev,.top17.fra44.fra19.fra17.ent20) {scanent}
    set widget(rev,.top17.fra44.fra19.fra18.02) {browserent}
    set widget(rev,.top17.fra44.fra19.fra20.ent22) {sshent}
    set widget(rev,.top17.fra44.fra19.fra26.02) {keygenent}
    set widget(rev,.top17.fra44.fra19.fra27.02) {agentent}
    set widget(rev,.top17.fra44.fra19.fra28.02) {addent}
    set widget(rev,.top17.fra44.fra19.fra29.02) {askpassent}
    set widget(rev,.top17.fra44.fra19.fra32.02) {xterment}
    set widget(rev,.top17.fra44.fra19.fra34.ent36) {scpent}
    set widget(rev,.top17.fra44.fra29.ent25) {sshent}
    set widget(rev,.top17.fra46.fra24.fra25.01) {defsites}
    set widget(rev,.top17.fra46.fra26.fra27.01) {specsites}
    set widget(rev,.top19.cpd20.03) {viewkey}
    set widget(rev,.top19.cpd21.03) {hostkeyview}
    set widget(rev,.top20.lab36) {proplabel}
    set widget(rev,.top21.fra22.ent24) {askeduser}
    set widget(rev,.top27.fra26.01) {sspecsites}
    set widget(rev,.top34.fra35.cpd38.01) {scpdirsl}
    set widget(rev,.top34.fra35.cpd39.01) {scpfilesl}
    set widget(rev,.top34.fra37.cpd40.01) {scpdirsr}
    set widget(rev,.top34.fra37.cpd41.01) {scpfilesr}
    set widget(rev,.top40.fra21.ent23) {newtit}
    set widget(rev,.top40.fra21.ent25) {newaddr}
    set widget(rev,.top40.fra21.ent27) {newuser}
    set widget(rev,.top40.fra41.cpd50.03) {remoteshostsfile}
    set widget(rev,.top40.fra42.cpd49.03) {remotercfile}
    set widget(rev,.top40.fra54.cpd57.03) {remotekeysfile}
    set widget(rev,.top43.cpd44.01) {lforwards}
    set widget(rev,.top43.fra45.ent17) {lfhost}
    set widget(rev,.top43.fra45.ent34) {lfcommentent}
    set widget(rev,.top43.fra45.ent46) {lfin}
    set widget(rev,.top43.fra45.ent48) {lfout}
    set widget(rev,.top43.lab25) {lfcomment}
    set widget(rev,.top50.fra51.cpd54.01) {knownhosts}
    set widget(rev,.top51.cpd53.01) {rforwards}
    set widget(rev,.top51.fra36.01) {rfin}
    set widget(rev,.top51.fra36.011) {rfcommentent}
    set widget(rev,.top51.fra36.03) {rfout}
    set widget(rev,.top51.fra36.08) {rfhost}
    set widget(rev,.top51.fra54.ent18) {rfhost}
    set widget(rev,.top51.fra54.ent55) {rfin}
    set widget(rev,.top51.fra54.ent57) {rfout}
    set widget(rev,.top51.lab24) {rfcomment}
    set widget(rev,.top52.fra18.ent26) {identpath}
    set widget(rev,.top52.fra34.cpd17.01) {keylist}
    set widget(rev,.top53.fra20) {keyframe}
    set widget(rev,.top53.fra20.cpd22.01) {distkeylist}
    set widget(rev,.top53.fra20.fra26.ent28) {distkeyentry}
    set widget(rev,.top53.fra29.but31) {contbutton}
    set widget(rev,.top53.fra29.but33) {backbutton}
    set widget(rev,.top53.fra34) {hostframe}
    set widget(rev,.top53.fra34.cpd35.01) {distconnlist}
    set widget(rev,.top53.fra39) {userframe}
    set widget(rev,.top53.fra39.01.02) {distuserlist}
    set widget(rev,.top53.fra42) {confirmframe}
    set widget(rev,.top53.fra42.mes44) {finishmessage}
    set widget(rev,.top53.lab17) {messageline}
    set widget(rev,.vTcl.fontmgr.cpd43.03) {.vTcl.fontmgr.text}
    set widget(rev,.vTcl.fontmgr.fra28.cpd29.01) {.vTcl.fontmgr.listbox}
    set widget(rfcomment) {.top51.lab24}
    set widget(rfcommentent) {.top51.fra36.011}
    set widget(rfhost) {.top51.fra36.08}
    set widget(rfin) {.top51.fra36.01}
    set widget(rforwards) {.top51.cpd53.01}
    set widget(rfout) {.top51.fra36.03}
    set widget(scanent) {.top17.fra44.fra19.fra17.ent20}
    set widget(scpbut) {.top17.fra30.fra19.but21}
    set widget(scpdirsl) {.top34.fra35.cpd38.01}
    set widget(scpdirsr) {.top34.fra37.cpd40.01}
    set widget(scpent) {.top17.fra44.fra19.fra34.ent36}
    set widget(scpfilesl) {.top34.fra35.cpd39.01}
    set widget(scpfilesr) {.top34.fra37.cpd41.01}
    set widget(scphosts) {.top17.fra27.fra30.cpd31.01}
    set widget(scpusers) {.top17.fra27.fra30.cpd32.01}
    set widget(specsites) {.top17.fra46.fra26.fra27.01}
    set widget(sscreenbut) {.top17.fra30.but26}
    set widget(sshbinent) {.top17.fra44.ent25}
    set widget(sshent) {.top17.fra44.fra19.fra20.ent22}
    set widget(sspecsites) {.top27.fra26.01}
    set widget(status) {.top17.fra17.lab18}
    set widget(statusagent) {.top17.fra17.lab19}
    set widget(subsysent) {.top17.fra21.fra24.ent42}
    set widget(textfield) {.top17.cpd31.01}
    set widget(titleent) {.top17.fra21.fra24.ent18}
    set widget(userent) {.top17.fra21.fra24.ent22}
    set widget(userframe) {.top53.fra39}
    set widget(viewkey) {.top19.cpd20.03}
    set widget(xterment) {.top17.fra44.fra19.fra32.02}
global x11forward; 

#################################
# USER DEFINED PROCEDURES
#

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

proc vTclWindow. {base {container 0}} {
    if {$base == ""} {
        set base .
    }
    ###################
    # CREATING WIDGETS
    ###################
    if {!$container} {
    wm focusmodel $base passive
    wm geometry $base 115x1+0+0
    wm maxsize $base 1009 738
    wm minsize $base 115 1
    wm overrideredirect $base 0
    wm resizable $base 1 1
    wm withdraw $base
    wm title $base "vt.tcl"
    }
    ###################
    # SETTING GEOMETRY
    ###################
}

proc vTclWindow.top17 {base {container 0}} {
    if {$base == ""} {
        set base .top17
    }
    if {[winfo exists $base] && (!$container)} {
        wm deiconify $base; return
    }
    ###################
    # CREATING WIDGETS
    ###################
    if {!$container} {
    toplevel $base -class Toplevel \
        -borderwidth "4" -menu "$base.m17" -highlightcolor "#000000" 
    wm focusmodel $base passive
    wm geometry $base 465x464
    wm maxsize $base 1009 738
    wm minsize $base 115 1
    wm overrideredirect $base 0
    wm resizable $base 1 1
    wm deiconify $base
    wm title $base "secpanel"
    }
    menu $base.m17 \
        -activeborderwidth "1" -borderwidth "1" -cursor "" -tearoff "0" -background LightSeaGreen
    $base.m17 add cascade \
        -menu "$base.m17.men18" -label "Program" -background orange
    $base.m17 add cascade \
        -menu "$base.m17.men31" -label "Config"  -background orange
    $base.m17 add cascade \
        -menu "$base.m17.men19" -label "Help"  -background orange
    menu $base.m17.men18 \
        -activeborderwidth "1" -borderwidth "1" -tearoff "0" 
    $base.m17.men18 add command \
        -command "historyman show" -label "Command history" 
    $base.m17.men18 add command \
        -command "command_trace view" -label "View trace window" 
    $base.m17.men18 add separator
    $base.m17.men18 add command \
        -command "do_exit" -label "Exit" 
    menu $base.m17.men19 \
        -activeborderwidth "1" -borderwidth "1" -tearoff "0"
    $base.m17.men19 add command \
        -label "Help Index (no help yet)" 
    $base.m17.men19 add cascade \
        -menu "$base.m17.men19.men17" -label "Update checks" 
    $base.m17.men19 add separator
    $base.m17.men19 add command \
        -command "about" -label "About secpanel" 
    menu $base.m17.men19.men20 \
        -activeborderwidth "1" -borderwidth "1" -tearoff "0" 
    menu $base.m17.men19.men17 \
        -activeborderwidth "1" -borderwidth "1" -tearoff "0" 
    $base.m17.men19.men17 add command \
        -command "check_sources ssh" -label "Check for SSH updates" \
        -state "disabled" 
    $base.m17.men19.men17 add command \
        -command "check_sources sp" -label "See secpanel homepage" 
    menu $base.m17.men31 \
        -activeborderwidth "1" -borderwidth "1" -tearoff "0" 
    $base.m17.men31 add checkbutton \
        -variable "configs(protectdata)" -command "save_globals protectdata" \
        -label "Protect Data (not yet fully funct.)" 
    $base.m17.men31 add separator
    $base.m17.men31 add command \
        -command "colorman 1" -label "Program colors" 
    $base.m17.men31 add command \
        -command "fontman show" -label "Program fonts" 
    $base.m17.men31 add checkbutton \
        -variable "configs(wingeom)" -command "save_globals geom" \
        -label "Remember window positions" 
    $base.m17.men31 add checkbutton \
        -variable "configs(startsat)" -command "save_globals startsat" \
        -label "Start into sat mode" 
    frame $base.fra21 \
        -borderwidth "1" -height "75" -highlightcolor "#000000" -width "125" 
    frame $base.fra21.fra23 \
        -height "75" -highlightcolor "#000000" -width "125" 
    checkbutton $base.fra21.fra23.01 \
        -anchor "w" -borderwidth "1" -highlightcolor "#000000" \
        -text "No agent forwarding" -variable "noagentforward" -bg orange
    checkbutton $base.fra21.fra23.02 \
        -anchor "w" -borderwidth "1" -highlightcolor "#000000" \
        -text "No X11 forwarding" -variable "x11forward" -bg orange
    checkbutton $base.fra21.fra23.che17 \
        -anchor "w" -borderwidth "1" -highlightcolor "#000000" \
        -text "No priv. source port" -variable "nopriv" -bg orange
    checkbutton $base.fra21.fra23.che20 \
        -anchor "w" -borderwidth "1" -highlightcolor "#000000" \
        -text "Verbose" -variable "verbose" -bg orange
    checkbutton $base.fra21.fra23.che21 \
        -anchor "w" -borderwidth "1" -highlightcolor "#000000" -text "Quiet" \
        -variable "quiet" -bg orange
    checkbutton $base.fra21.fra23.che22 \
        -anchor "w" -borderwidth "1" -highlightcolor "#000000" \
        -text "Fork into background" -variable "fork" -bg orange
    checkbutton $base.fra21.fra23.che39 \
        -anchor "w" -borderwidth "1" -highlightcolor "#000000" \
        -text "Strict hostkey check" -variable "stricthost" -bg orange
    checkbutton $base.fra21.fra23.che29 \
        -anchor "w" -borderwidth "1" -highlightcolor "#000000" \
        -text "Wait after connection" -variable "connwait" -bg orange
    checkbutton $base.fra21.fra23.che30 \
        -anchor "w" -borderwidth "1" -highlightcolor "#000000" \
        -text "Terminal iconified" -variable "termicon" -bg orange
    frame $base.fra21.fra24 \
        -height "75" -highlightcolor "#000000" -width "125" 
    label $base.fra21.fra24.lab17 \
        -anchor "w" -borderwidth "1" -highlightcolor "#000000" -text "Title:" 
    entry $base.fra21.fra24.ent18 \
        -borderwidth "1" -highlightcolor "#000000" -width "15" 
    label $base.fra21.fra24.lab19 \
        -anchor "w" -borderwidth "0" -highlightcolor "#000000" \
        -relief "raised" -text "Host:" 
    entry $base.fra21.fra24.ent20 \
        -borderwidth "1" -highlightcolor "#000000" -width "15" 
    label $base.fra21.fra24.lab21 \
        -anchor "w" -borderwidth "0" -highlightcolor "#000000" \
        -relief "raised" -text "User:" 
    entry $base.fra21.fra24.ent22 \
        -borderwidth "1" -highlightcolor "#000000" -width "15" 
    label $base.fra21.fra24.lab23 \
        -anchor "w" -borderwidth "0" -highlightcolor "#000000" \
        -relief "raised" -text "Port:" 
    entry $base.fra21.fra24.ent24 \
        -borderwidth "1" -highlightcolor "#000000" -width "15" 
    label $base.fra21.fra24.lab25 \
        -anchor "w" -borderwidth "0" -highlightcolor "#000000" \
        -relief "raised" -text "Exec:" 
    entry $base.fra21.fra24.ent26 \
        -borderwidth "1" -highlightcolor "#000000" -width "15" 
    checkbutton $base.fra21.fra24.che17 \
        -anchor "w" -borderwidth "1" -highlightcolor "#000000" -text "Ask" \
        -variable "askuserspec" -bg orange
    label $base.fra21.fra24.lab41 \
        -anchor "w" -borderwidth "0" -highlightcolor "#000000" \
        -relief "raised" -text "Subsys:" 
    entry $base.fra21.fra24.ent42 \
        -borderwidth "1" -highlightcolor "#000000" -width "15" 
    checkbutton $base.fra21.fra24.che43 \
        -anchor "w" -borderwidth "1" -highlightcolor "#000000" \
        -text "No Exec" -variable "noexec" -bg orange
    label $base.fra21.fra24.lab18 \
        -anchor "w" -borderwidth "0" -highlightcolor "#000000" \
        -text "Identity:" 
    entry $base.fra21.fra24.ent19 \
        -borderwidth "1" -highlightcolor "#000000" -width "15" 
    label $base.fra21.fra24.lab20 \
        -anchor "w" -borderwidth "0" -highlightcolor "#000000" \
        -text "Cfg-file:" 
    entry $base.fra21.fra24.ent21 \
        -borderwidth "1" -highlightcolor "#000000" -width "15" 
    button $base.fra21.fra24.but22 \
        -command "choosefile {} {} {} identityent" -highlightcolor "#000000" \
        -padx "9" -pady "3" -relief "groove" 
    button $base.fra21.fra24.but23 \
        -command "choosefile {} {} {} cfgfileent" -highlightcolor "#000000" \
        -padx "9" -pady "3" -relief "groove" 
    label $base.fra21.fra24.lab24 \
        -anchor "w" -borderwidth "1" -highlightcolor "#000000" -text "Algo:" 
    menubutton $base.fra21.fra24.men25 \
        -highlightcolor "#000000" -indicatoron "1" \
        -menu "$base.fra21.fra24.men25.01" -padx "4" -pady "3" \
        -relief "groove" -text "idea" -textvariable "algo" 
    menu $base.fra21.fra24.men25.01 \
        -activeborderwidth "1" -borderwidth "1" -cursor "" -tearoff "0" 
    $base.fra21.fra24.men25.01 add radiobutton \
        -value "default" -variable "algo" -label "default" 
    $base.fra21.fra24.men25.01 add radiobutton \
        -value "idea" -variable "algo" -label "idea" 
    $base.fra21.fra24.men25.01 add radiobutton \
        -value "des" -variable "algo" -label "des" 
    $base.fra21.fra24.men25.01 add radiobutton \
        -value "3des" -variable "algo" -label "3des" 
    $base.fra21.fra24.men25.01 add radiobutton \
        -value "blowfish" -variable "algo" -label "blowfish" 
    $base.fra21.fra24.men25.01 add radiobutton \
        -value "arcfour" -variable "algo" -label "arcfour" 
    $base.fra21.fra24.men25.01 add radiobutton \
        -value "none" -variable "algo" -label "none" 
    checkbutton $base.fra21.fra24.che26 \
        -anchor "w" -borderwidth "1" -highlightcolor "#000000" \
        -text "Compression" -variable "compress" -bg orange
    scale $base.fra21.fra24.sca27 \
        -borderwidth "1" -highlightcolor "#000000" -orient "horizontal" \
        -showvalue "0" -sliderlength "20" -sliderrelief "groove" -to "9.0" \
        -troughcolor "#e2e2de" -variable "compressval" 
    label $base.fra21.fra24.lab28 \
        -borderwidth "0" -highlightcolor "#000000" -text "0" \
        -textvariable "compressval" 
    frame $base.fra21.fra26 \
        -height "75" -highlightcolor "#000000" -width "125" 
    button $base.fra21.fra26.01 \
        -anchor "w" -command "open_forwardings l" -highlightcolor "#000000" \
        -padx "9" -pady "3" -relief "groove" -text "Local forwards" -bg orange
    button $base.fra21.fra26.02 \
        -anchor "w" -command "open_forwardings r" -highlightcolor "#000000" \
        -padx "9" -pady "3" -relief "groove" -text "Remote forwards" -bg orange
    checkbutton $base.fra21.fra26.03 \
        -anchor "w" -borderwidth "1" -highlightcolor "#000000" \
        -text "Run as gateway" -variable "gateway" -bg orange
    frame $base.fra21.fra48 \
        -borderwidth "2" -relief "groove" -height "75" \
        -highlightcolor "#000000" -width "110" 
    frame $base.fra21.fra48.cpd18 \
        -relief "raised" -height "30" -highlightcolor "#000000" -width "3" 
    listbox $base.fra21.fra48.cpd18.01 \
        -borderwidth "1" -height "3" -highlightcolor "#000000" -width "10" \
        -xscrollcommand "$base.fra21.fra48.cpd18.02 set" \
        -yscrollcommand "$base.fra21.fra48.cpd18.03 set" 
    bind $base.fra21.fra48.cpd18.01 <Double-Button-1> {
        load_profile ssh
    }
    scrollbar $base.fra21.fra48.cpd18.02 \
        -borderwidth "1" -command "$base.fra21.fra48.cpd18.01 xview" \
        -cursor "left_ptr" -highlightcolor "#000000" -orient "horizontal" \
        -troughcolor "#e2e2de" -width "10" 
    scrollbar $base.fra21.fra48.cpd18.03 \
        -borderwidth "1" -command "$base.fra21.fra48.cpd18.01 yview" \
        -cursor "left_ptr" -highlightcolor "#000000" -troughcolor "#e2e2de" \
        -width "10" 
    frame $base.fra21.fra48.fra19 \
        -height "75" -highlightcolor "#000000" -width "125" 
    entry $base.fra21.fra48.fra19.ent20 \
        -borderwidth "1" -highlightcolor "#000000" -width "2" 
    label $base.fra21.fra48.fra19.lab21 \
        -borderwidth "1" -highlightcolor "#000000" -text "Profile Name" 
    frame $base.fra21.fra48.fra40 \
        -height "75" -highlightcolor "#000000" -width "125" 
    button $base.fra21.fra48.fra40.but41 \
        -command "save_profile" -highlightcolor "#000000" -padx "9" -pady "3" \
        -relief "groove" -text "Save" -bg orange
    button $base.fra21.fra48.fra40.but42 \
        -command "delete_profile" -highlightcolor "#000000" -padx "9" \
        -pady "3" -relief "groove" -text "Delete" -bg IndianRed
    button $base.fra21.fra48.fra40.but43 \
        -command "load_profile ssh" -highlightcolor "#000000" -padx "9" \
        -pady "3" -relief "groove" -text "Load" -bg orange
    button $base.fra21.fra48.fra40.but44 \
        -command "clear_profiles" -highlightcolor "#000000" -padx "9" \
        -pady "3" -relief "groove" -text "New" -bg orange
    frame $base.fra21.fra35 \
        -height "75" -highlightcolor "#000000" -width "125" 
    radiobutton $base.fra21.fra35.rad36 \
        -anchor "e" -borderwidth "1" -highlightcolor "#000000" -text "IPv4" \
        -value "4" -variable "ipverconnect" -bg orange
    radiobutton $base.fra21.fra35.rad37 \
        -anchor "w" -borderwidth "1" -highlightcolor "#000000" -text "IPv6" \
        -value "6" -variable "ipverconnect" -bg orange
    radiobutton $base.fra21.fra35.rad38 \
        -anchor "e" -borderwidth "1" -highlightcolor "#000000" -text "SSH 1" \
        -value "1" -variable "sshverconnect" -bg orange
    radiobutton $base.fra21.fra35.rad39 \
        -anchor "w" -borderwidth "1" -highlightcolor "#000000" -text "SSH 2" \
        -value "2" -variable "sshverconnect" -bg orange
    frame $base.fra30 \
        -height "75" -highlightcolor "#000000" -width "125" 
    frame $base.fra30.fra19 \
        -height "75" -highlightcolor "#000000" -width "125" 
    button $base.fra30.fra19.but20 \
        -command "changetab connect" -highlightcolor "#000000" -padx "9" \
        -pady "3" -relief "groove" -bg black
    bind $base.fra30.fra19.but20 <Enter> {
        showstatus "Lists of configured SSH connections"
    }
    bind $base.fra30.fra19.but20 <Leave> {
        showstatus ""
    }
    button $base.fra30.fra19.but22 \
        -command "changetab ssh" -highlightcolor "#000000" -padx "9" \
        -pady "3" -relief "groove" -bg black
    bind $base.fra30.fra19.but22 <Enter> {
        showstatus "Manage connection profiles"
    }
    bind $base.fra30.fra19.but22 <Leave> {
        showstatus ""
    }
    button $base.fra30.fra19.but23 \
        -command "changetab key" -highlightcolor "#000000" -padx "9" \
        -pady "3" -relief "groove" -bg black
    bind $base.fra30.fra19.but23 <Enter> {
        showstatus "Manage your keypairs"
    }
    bind $base.fra30.fra19.but23 <Leave> {
        showstatus ""
    }
    button $base.fra30.fra19.but24 \
        -command "changetab terminal" -highlightcolor "#000000" -padx "9" \
        -pady "3" -relief "groove" -bg black
    bind $base.fra30.fra19.but24 <Enter> {
        showstatus "Configurations"
    }
    bind $base.fra30.fra19.but24 <Leave> {
        showstatus ""
    }
    button $base.fra30.fra19.but25 \
        -command "indiadmin" -padx "9" -pady "3" -relief "groove" -bg black
    bind $base.fra30.fra19.but25 <Enter> {
        showstatus "IndiMail Administration"
    }
    bind $base.fra30.fra19.but25 <Leave> {
        showstatus ""
    }
    button $base.fra30.but26 \
        -borderwidth "0" -command "changetab small" -highlightcolor "#000000" \
        -padx "9" -pady "3" -relief "flat" -bg orange
    bind $base.fra30.but26 <Enter> {
        showstatus "Switch to satellite GUI"
    }
    bind $base.fra30.but26 <Leave> {
        showstatus ""
    }
    frame $base.fra35 \
        -borderwidth "1" -height "75" -highlightcolor "#000000" -width "125" 
    frame $base.fra35.fra17 \
        -borderwidth "2" -relief "groove" -height "75" \
        -highlightcolor "#000000" -width "125" 
    button $base.fra35.fra17.but17 \
        -command "keygen 0" -highlightcolor "#000000" -padx "9" -pady "3" \
        -relief "groove" -text "Manage keypairs" -bg orange
    button $base.fra35.fra17.but19 \
        -command "keygen dist" -highlightcolor "#000000" -relief "groove" \
        -text "Distribute public keys" -bg orange
    button $base.fra35.fra17.but18 \
        -command "hostkey edit" -highlightcolor "#000000" -padx "9" -pady "3" \
        -relief "groove" -text "Manage hostkeys" -bg orange
    label $base.fra35.fra17.lab19 \
        -anchor "w" -borderwidth "0" -highlightcolor "#000000" \
        -text "Key handling" -font "helvetica 10 bold"
    frame $base.fra35.fra20 \
        -borderwidth "2" -relief "groove" -height "75" \
        -highlightcolor "#000000" -width "125" 
    button $base.fra35.fra20.but22 \
        -command "manage_agent launch" -highlightcolor "#000000" -padx "9" \
        -pady "3" -relief "groove" -text "Launch agent" -bg orange
    button $base.fra35.fra20.but26 \
        -command "manage_agent remident" -highlightcolor "#000000" -padx "9" \
        -pady "3" -relief "groove" -text "Remove identity" 
    checkbutton $base.fra35.fra20.che28 \
        -borderwidth "1" -command "save_globals agent" \
        -highlightcolor "#000000" -text "Launch agent at startup" \
        -variable "launcher" -bg orange
    frame $base.fra35.fra20.fra22 \
        -borderwidth "1" -height "30" -highlightcolor "#000000" -width "30" 
    listbox $base.fra35.fra20.fra22.01 \
        -borderwidth "1" -height "3" -highlightcolor "#000000" -width "18" \
        -xscrollcommand "$base.fra35.fra20.fra22.02 set" \
        -yscrollcommand "$base.fra35.fra20.fra22.03 set" 
    scrollbar $base.fra35.fra20.fra22.02 \
        -borderwidth "1" -command "$base.fra35.fra20.fra22.01 xview" \
        -cursor "left_ptr" -highlightcolor "#000000" -orient "horizontal" \
        -troughcolor "#e2e2de" -width "10" 
    scrollbar $base.fra35.fra20.fra22.03 \
        -borderwidth "1" -command "$base.fra35.fra20.fra22.01 yview" \
        -cursor "left_ptr" -highlightcolor "#000000" -troughcolor "#e2e2de" \
        -width "10" 
    button $base.fra35.fra20.but20 \
        -command "manage_agent addident" -highlightcolor "#000000" -padx "9" \
        -pady "3" -relief "groove" -text "Add identity" -bg orange
    button $base.fra35.fra20.but17 \
        -command "manage_agent kill" -highlightcolor "#000000" -padx "9" \
        -pady "3" -relief "groove" -text "Kill agent" -bg IndianRed
    label $base.fra35.fra20.lab28 \
        -anchor "w" -borderwidth "1" -highlightcolor "#000000" \
        -text "SSH-Agent" -font "helvetica 10 bold" -bg orange
    button $base.fra35.fra20.but18 \
        -command "manage_agent chdef" -highlightcolor "#000000" -padx "9" \
        -pady "3" -relief "groove" -text "Set def. ident." -bg orange
    button $base.fra35.fra20.but188 \
        -command "manage_agent rmdef" -highlightcolor "#000000" -padx "9" \
        -pady "3" -relief "groove" -text "Remove def. ident." -bg orange
    button $base.fra35.fra20.but30 \
        -command "manage_agent info" -highlightcolor "#000000" -padx "9" \
        -pady "3" -relief "groove" -text "Agent info" -bg orange
    frame $base.fra44 \
        -borderwidth "1" -height "75" -highlightcolor "#000000" 
    frame $base.fra44.fra27 \
        -relief "groove" -height "75" -highlightcolor "#000000" 
    button $base.fra44.fra27.but28 \
        -command "save_globals bins" -highlightcolor "#000000" -padx "9" \
        -pady "3" -relief "groove" -text "Save" -bg orange
    frame $base.fra44.fra19 \
        -height "75" -highlightcolor "#000000" 
    frame $base.fra44.fra19.fra20 \
        -borderwidth "1" -height "75" -highlightcolor "#000000" 
    label $base.fra44.fra19.fra20.lab21 \
        -anchor "w" -borderwidth "1" -highlightcolor "#000000" -text "SSH" 
    entry $base.fra44.fra19.fra20.ent22 \
        -borderwidth "1" -highlightcolor "#000000" 
    button $base.fra44.fra19.fra20.but23 \
        -borderwidth "0" -command "browsebin ssh" -highlightcolor "#000000" \
        -padx "9" -pady "3" -relief "flat" -bg orange
    frame $base.fra44.fra19.fra26 \
        -borderwidth "1" -height "75" -highlightcolor "#000000" 
    label $base.fra44.fra19.fra26.01 \
        -anchor "w" -borderwidth "1" -highlightcolor "#000000" \
        -text "SSH-Keygen" 
    entry $base.fra44.fra19.fra26.02 \
        -borderwidth "1" -highlightcolor "#000000" 
    button $base.fra44.fra19.fra26.03 \
        -borderwidth "0" -command "browsebin keygen" \
        -highlightcolor "#000000" -padx "9" -pady "3" -relief "flat" -bg orange
    frame $base.fra44.fra19.fra27 \
        -borderwidth "1" -height "75" -highlightcolor "#000000" 
    label $base.fra44.fra19.fra27.01 \
        -anchor "w" -highlightcolor "#000000" -text "SSH-Agent" 
    entry $base.fra44.fra19.fra27.02 \
        -borderwidth "1" -highlightcolor "#000000" 
    button $base.fra44.fra19.fra27.03 \
        -borderwidth "0" -command "browsebin agent" -highlightcolor "#000000" \
        -padx "9" -pady "3" -relief "flat" -bg orange
    frame $base.fra44.fra19.fra28 \
        -borderwidth "1" -height "75" -highlightcolor "#000000" 
    label $base.fra44.fra19.fra28.01 \
        -anchor "w" -highlightcolor "#000000" -text "SSH-Add" 
    entry $base.fra44.fra19.fra28.02 \
        -borderwidth "1" -highlightcolor "#000000" 
    button $base.fra44.fra19.fra28.03 \
        -borderwidth "0" -command "browsebin add" -highlightcolor "#000000" \
        -padx "9" -pady "3" -relief "flat" -bg orange
    frame $base.fra44.fra19.fra29 \
        -borderwidth "1" -height "75" -highlightcolor "#000000" 
    label $base.fra44.fra19.fra29.01 \
        -anchor "w" -borderwidth "1" -highlightcolor "#000000" \
        -text "SSH-Askpass" 
    entry $base.fra44.fra19.fra29.02 \
        -borderwidth "1" -highlightcolor "#000000" 
    button $base.fra44.fra19.fra29.03 \
        -borderwidth "0" -command "browsebin askpass" \
        -highlightcolor "#000000" -padx "9" -pady "3" -relief "flat" -bg orange
    frame $base.fra44.fra19.fra32 \
        -borderwidth "1" -height "75" -highlightcolor "#000000" 
    label $base.fra44.fra19.fra32.01 \
        -anchor "w" -borderwidth "1" -highlightcolor "#000000" \
        -text "XTerminal" 
    menubutton $base.fra44.fra19.fra32.men35 \
        -highlightcolor "#000000" -indicatoron "1" \
        -menu "$base.fra44.fra19.fra32.men35.01" -padx "4" -pady "3" \
        -relief "groove" -textvariable "configs(termver)" -bg orange
    menu $base.fra44.fra19.fra32.men35.01 \
        -activeborderwidth "1" -borderwidth "1" -cursor "" -tearoff "0" 
    frame $base.fra44.fra19.fra30 \
        -borderwidth "1" -height "75" -highlightcolor "#000000" 
    label $base.fra44.fra19.fra30.01 \
        -anchor "w" -borderwidth "1" -highlightcolor "#000000" -text "SFTP" 
    menubutton $base.fra44.fra19.fra30.men34 \
        -indicatoron "1" -menu "$base.fra44.fra19.fra30.men34.m" -padx "4" \
        -pady "3" -relief "groove" -textvariable "configs(sftpbin)" -bg orange
    menu $base.fra44.fra19.fra30.men34.m \
        -activeborderwidth "1" -borderwidth "1" -cursor "" -tearoff "0" 
    frame $base.fra44.fra19.fra18 \
        -borderwidth "1" -height "75" -highlightcolor "#000000" 
    label $base.fra44.fra19.fra18.01 \
        -anchor "w" -borderwidth "1" -highlightcolor "#000000" \
        -text "WWW browser" 
    entry $base.fra44.fra19.fra18.02 \
        -borderwidth "1" -highlightcolor "#000000" 
    button $base.fra44.fra19.fra18.03 \
        -borderwidth "0" -command "browsebin browser" \
        -highlightcolor "#000000" -padx "9" -pady "3" -relief "flat" -bg orange
    frame $base.fra44.fra19.fra34 \
        -relief "groove" -height "75" -width "125" 
    label $base.fra44.fra19.fra34.lab35 \
        -borderwidth "0" -relief "raised" -text "SCP" -anchor "w"
    entry $base.fra44.fra19.fra34.ent36 \
        -borderwidth "1" 
    button $base.fra44.fra19.fra34.but34 \
        -borderwidth "0" -command "browsebin scp" -highlightcolor "#000000" \
        -padx "9" -pady "3" -relief "flat" -bg orange
    label $base.fra44.lab30 \
        -anchor "w" -borderwidth "1" -highlightcolor "#000000" \
        -text "Programs used by secpanel" -font "helvetica 10 bold" 
    frame $base.fra46 \
        -borderwidth "1" -height "75" -highlightcolor "#000000" 
    frame $base.fra46.fra26 \
        -borderwidth "1" -height "75" -highlightcolor "#000000" -width "125" 
    frame $base.fra46.fra26.fra27 \
        -relief "raised" -height "30" -highlightcolor "#000000" -width "30" 
    listbox $base.fra46.fra26.fra27.01 \
        -borderwidth "1" -height "6" -highlightcolor "#000000" \
        -selectmode "extended" \
        -xscrollcommand "$base.fra46.fra26.fra27.02 set" \
        -yscrollcommand "$base.fra46.fra26.fra27.03 set" 
    bind $base.fra46.fra26.fra27.01 <Button-3> {
        set li [.top17.fra46.fra26.fra27.01 nearest %y]
	.top17.fra46.fra26.fra27.01 selection clear 0 end
	.top17.fra46.fra26.fra27.01 activate $li
	.top17.fra46.fra26.fra27.01 selection set $li
        showmenu
    }
    bind $base.fra46.fra26.fra27.01 <Double-Button-1> {
        connect
    }
    bind $base.fra46.fra26.fra27.01 <Enter> {
        showstatus "Right click mouse to handle entries"
    }
    bind $base.fra46.fra26.fra27.01 <Leave> {
        showstatus ""
    }
    scrollbar $base.fra46.fra26.fra27.02 \
        -borderwidth "1" -command "$base.fra46.fra26.fra27.01 xview" \
        -cursor "left_ptr" -highlightcolor "#000000" -orient "horizontal" \
        -troughcolor "#e2e2de" -width "10" 
    scrollbar $base.fra46.fra26.fra27.03 \
        -borderwidth "1" -command "$base.fra46.fra26.fra27.01 yview" \
        -cursor "left_ptr" -highlightcolor "#000000" -troughcolor "#e2e2de" \
        -width "10" 
    frame $base.fra46.fra26.fra29 \
        -borderwidth "1" -height "75" -highlightcolor "#000000" -width "125" 
    frame $base.fra46.fra26.fra29.fra28 \
        -height "75" -highlightcolor "#000000" -width "125" 
    button $base.fra46.fra26.fra29.fra28.but29 \
        -command "newconn 1" -highlightcolor "#000000" -padx "9" -pady "3" \
        -relief "groove" -text "New" -bg orange
    button $base.fra46.fra26.fra29.fra28.but30 \
        -command "propconn specsites" -highlightcolor "#000000" -padx "9" \
        -pady "3" -relief "groove" -text "Edit" -bg orange
    button $base.fra46.fra26.fra29.fra28.but31 \
        -command "delconn" -highlightcolor "#000000" -padx "9" -pady "3" \
        -relief "groove" -text "Delete" -bg orange
    frame $base.fra46.fra26.fra29.fra36 \
        -relief "groove" -height "75" -width "125" 
    button $base.fra46.fra26.fra29.fra36.but37 \
        -command "connect specsites" -highlightcolor "#000000" -padx "9" \
        -pady "3" -relief "groove" -text "Connect" -bg orange
    button $base.fra46.fra26.fra29.fra36.but38 \
        -command "connect multi" -highlightcolor "#000000" -padx "9" \
        -pady "3" -relief "groove" -text "Multi" -bg orange
    button $base.fra46.fra26.fra29.fra36.but39 \
        -command "connect custom" -highlightcolor "#000000" -padx "9" \
        -pady "3" -relief "groove" -state "disabled" -text "Custom" -bg orange
    button $base.fra46.fra26.fra29.fra36.but34 \
        -command "connect_sftp" -relief "groove" -text "SFTP" -bg orange
    label $base.fra46.fra26.lab34 \
        -anchor "w" -borderwidth "1" -highlightcolor "#000000" \
        -text "Connections" 
    frame $base.fra17 \
        -borderwidth "1" -height "75" -highlightcolor "#000000" -width "125" 
    label $base.fra17.lab18 \
        -anchor "w" -borderwidth "1" -highlightcolor "#000000" -width "30" 
    label $base.fra17.lab19 \
        -anchor "w" -borderwidth "1" -highlightcolor "#000000" -width "30" 
    ###################
    # SETTING GEOMETRY
    ###################
    grid columnconf $base 0 -weight 1
    grid rowconf $base 1 -weight 1

    grid rowconf $base.fra21 0 -weight 1
    grid columnconf $base.fra21 0 -weight 1
    grid columnconf $base.fra21 1 -weight 1

    grid $base.fra21.fra23 \
        -in $base.fra21 -column "0" -row "1" -columnspan "1" -rowspan "1" \
        -ipadx "2" -ipady "2" -padx "2" -pady "2" -sticky "new" 
    grid $base.fra21.fra23.01 \
        -in $base.fra21.fra23 -column "0" -row "0" -columnspan "1" \
        -rowspan "1" -sticky "ew" 
    grid $base.fra21.fra23.02 \
        -in $base.fra21.fra23 -column "0" -row "1" -columnspan "1" \
        -rowspan "1" -sticky "ew" 
    grid $base.fra21.fra23.che17 \
        -in $base.fra21.fra23 -column "0" -row "3" -columnspan "1" \
        -rowspan "1" -sticky "ew" 
    grid $base.fra21.fra23.che20 \
        -in $base.fra21.fra23 -column "0" -row "6" -columnspan "1" \
        -rowspan "1" -sticky "ew" 
    grid $base.fra21.fra23.che21 \
        -in $base.fra21.fra23 -column "0" -row "5" -columnspan "1" \
        -rowspan "1" -sticky "ew" 
    grid $base.fra21.fra23.che22 \
        -in $base.fra21.fra23 -column "0" -row "4" -columnspan "1" \
        -rowspan "1" -sticky "ew" 
    grid $base.fra21.fra23.che39 \
        -in $base.fra21.fra23 -column "0" -row "2" -columnspan "1" \
        -rowspan "1" -sticky "ew" 
    grid $base.fra21.fra23.che29 \
        -in $base.fra21.fra23 -column "0" -row "7" -columnspan "1" \
        -rowspan "1" -sticky "ew" 
    grid $base.fra21.fra23.che30 \
        -in $base.fra21.fra23 -column "0" -row "8" -columnspan "1" \
        -rowspan "1" -sticky "ew" 
    grid $base.fra21.fra24 \
        -in $base.fra21 -column "1" -row "1" -columnspan "1" -rowspan "1" \
        -ipadx "2" -ipady "2" -padx "2" -pady "2" -sticky "new" 
    grid columnconf $base.fra21.fra24 1 -weight 1
    grid $base.fra21.fra24.lab17 \
        -in $base.fra21.fra24 -column "0" -row "1" -columnspan "1" \
        -rowspan "1" -sticky "ew" 
    grid $base.fra21.fra24.ent18 \
        -in $base.fra21.fra24 -column "1" -row "1" -columnspan "2" \
        -rowspan "1" -sticky "ew" 
    grid $base.fra21.fra24.lab19 \
        -in $base.fra21.fra24 -column "0" -row "2" -columnspan "1" \
        -rowspan "1" -sticky "ew" 
    grid $base.fra21.fra24.ent20 \
        -in $base.fra21.fra24 -column "1" -row "2" -columnspan "2" \
        -rowspan "1" -sticky "ew" 
    grid $base.fra21.fra24.lab21 \
        -in $base.fra21.fra24 -column "0" -row "3" -columnspan "1" \
        -rowspan "1" -sticky "ew" 
    grid $base.fra21.fra24.ent22 \
        -in $base.fra21.fra24 -column "1" -row "3" -columnspan "1" \
        -rowspan "1" -sticky "ew" 
    grid $base.fra21.fra24.lab23 \
        -in $base.fra21.fra24 -column "0" -row "4" -columnspan "1" \
        -rowspan "1" -sticky "ew" 
    grid $base.fra21.fra24.ent24 \
        -in $base.fra21.fra24 -column "1" -row "4" -columnspan "2" \
        -rowspan "1" -sticky "ew" 
    grid $base.fra21.fra24.lab25 \
        -in $base.fra21.fra24 -column "0" -row "5" -columnspan "1" \
        -rowspan "1" -sticky "ew" 
    grid $base.fra21.fra24.ent26 \
        -in $base.fra21.fra24 -column "1" -row "5" -columnspan "1" \
        -rowspan "1" -sticky "ew" 
    grid $base.fra21.fra24.che17 \
        -in $base.fra21.fra24 -column "2" -row "3" -columnspan "1" \
        -rowspan "1" -sticky "ew" 
    grid $base.fra21.fra24.lab41 \
        -in $base.fra21.fra24 -column "0" -row "6" -columnspan "1" \
        -rowspan "1" -sticky "ew" 
    grid $base.fra21.fra24.ent42 \
        -in $base.fra21.fra24 -column "1" -row "6" -columnspan "2" \
        -rowspan "1" -sticky "ew" 
    grid $base.fra21.fra24.che43 \
        -in $base.fra21.fra24 -column "2" -row "5" -columnspan "1" \
        -rowspan "1" -sticky "ew" 
    grid $base.fra21.fra24.lab18 \
        -in $base.fra21.fra24 -column "0" -row "7" -columnspan "1" \
        -rowspan "1" -sticky "ew" 
    grid $base.fra21.fra24.ent19 \
        -in $base.fra21.fra24 -column "1" -row "7" -columnspan "1" \
        -rowspan "1" -sticky "ew" 
    grid $base.fra21.fra24.lab20 \
        -in $base.fra21.fra24 -column "0" -row "8" -columnspan "1" \
        -rowspan "1" -sticky "ew" 
    grid $base.fra21.fra24.ent21 \
        -in $base.fra21.fra24 -column "1" -row "8" -columnspan "1" \
        -rowspan "1" -sticky "ew" 
    grid $base.fra21.fra24.but22 \
        -in $base.fra21.fra24 -column "2" -row "7" -columnspan "1" \
        -rowspan "1" -sticky "w" 
    grid $base.fra21.fra24.but23 \
        -in $base.fra21.fra24 -column "2" -row "8" -columnspan "1" \
        -rowspan "1" -sticky "w" 
    grid $base.fra21.fra24.lab24 \
        -in $base.fra21.fra24 -column "0" -row "9" -columnspan "1" \
        -rowspan "1" -sticky "ew" 
    grid $base.fra21.fra24.men25 \
        -in $base.fra21.fra24 -column "1" -row "9" -columnspan "2" \
        -rowspan "1" -padx "2" -sticky "ew" 
    grid $base.fra21.fra24.che26 \
        -in $base.fra21.fra24 -column "0" -row "10" -columnspan "2" \
        -rowspan "1" -sticky "ew" 
    grid $base.fra21.fra24.sca27 \
        -in $base.fra21.fra24 -column "2" -row "10" -columnspan "1" \
        -rowspan "1" -sticky "ew" 
    grid $base.fra21.fra24.lab28 \
        -in $base.fra21.fra24 -column "1" -row "10" -columnspan "1" \
        -rowspan "1" -sticky "e" 
    grid $base.fra21.fra26 \
        -in $base.fra21 -column "1" -row "3" -columnspan "1" -rowspan "1" \
        -ipadx "2" -ipady "2" -padx "2" -pady "2" -sticky "nesw" 
    grid columnconf $base.fra21.fra26 0 -weight 1
    grid rowconf $base.fra21.fra26 0 -weight 1
    grid $base.fra21.fra26.01 \
        -in $base.fra21.fra26 -column "0" -row "0" -columnspan "1" \
        -rowspan "1" -sticky "ew" 
    grid $base.fra21.fra26.02 \
        -in $base.fra21.fra26 -column "0" -row "1" -columnspan "1" \
        -rowspan "1" -sticky "ew" 
    grid $base.fra21.fra26.03 \
        -in $base.fra21.fra26 -column "3" -row "0" -columnspan "1" \
        -rowspan "1" -sticky "ew" 
    grid $base.fra21.fra48 \
        -in $base.fra21 -column "0" -row "0" -columnspan "2" -rowspan "1" \
        -ipadx "2" -ipady "2" -padx "2" -pady "2" -sticky "nesw" 
    grid columnconf $base.fra21.fra48 1 -weight 1
    grid columnconf $base.fra21.fra48 2 -weight 1
    grid columnconf $base.fra21.fra48 3 -weight 1
    grid rowconf $base.fra21.fra48 1 -weight 1
    grid $base.fra21.fra48.cpd18 \
        -in $base.fra21.fra48 -column "2" -row "0" -columnspan "2" \
        -rowspan "2" -sticky "nesw" 
    grid columnconf $base.fra21.fra48.cpd18 0 -weight 1
    grid rowconf $base.fra21.fra48.cpd18 0 -weight 1
    grid $base.fra21.fra48.cpd18.01 \
        -in $base.fra21.fra48.cpd18 -column "0" -row "0" -columnspan "1" \
        -rowspan "1" -sticky "nesw" 
    grid $base.fra21.fra48.cpd18.02 \
        -in $base.fra21.fra48.cpd18 -column "0" -row "1" -columnspan "1" \
        -rowspan "1" -sticky "ew" 
    grid $base.fra21.fra48.cpd18.03 \
        -in $base.fra21.fra48.cpd18 -column "1" -row "0" -columnspan "1" \
        -rowspan "1" -sticky "ns" 
    grid $base.fra21.fra48.fra19 \
        -in $base.fra21.fra48 -column "0" -row "0" -columnspan "1" \
        -rowspan "1" -ipadx "2" -ipady "2" -padx "2" -pady "2" -sticky "new" 
    grid columnconf $base.fra21.fra48.fra19 0 -weight 1
    grid $base.fra21.fra48.fra19.ent20 \
        -in $base.fra21.fra48.fra19 -column "0" -row "1" -columnspan "2" \
        -rowspan "1" -sticky "new" 
    grid $base.fra21.fra48.fra19.lab21 \
        -in $base.fra21.fra48.fra19 -column "0" -row "0" -columnspan "1" \
        -rowspan "1" -sticky "ew" 
    grid $base.fra21.fra48.fra40 \
        -in $base.fra21.fra48 -column "1" -row "0" -columnspan "1" \
        -rowspan "1" -ipadx "2" -ipady "2" -padx "2" -pady "2" -sticky "n" 
    grid $base.fra21.fra48.fra40.but41 \
        -in $base.fra21.fra48.fra40 -column "0" -row "0" -columnspan "1" \
        -rowspan "1" -sticky "ew" 
    grid $base.fra21.fra48.fra40.but42 \
        -in $base.fra21.fra48.fra40 -column "0" -row "1" -columnspan "1" \
        -rowspan "1" -sticky "ew" 
    grid $base.fra21.fra48.fra40.but43 \
        -in $base.fra21.fra48.fra40 -column "1" -row "0" -columnspan "1" \
        -rowspan "1" -sticky "ew" 
    grid $base.fra21.fra48.fra40.but44 \
        -in $base.fra21.fra48.fra40 -column "1" -row "1" -columnspan "1" \
        -rowspan "1" -sticky "ew" 
    grid $base.fra21.fra35 \
        -in $base.fra21 -column "0" -row "3" -columnspan "1" -rowspan "1" \
        -ipadx "2" -ipady "2" -padx "2" -pady "2" -sticky "nesw" 
    grid columnconf $base.fra21.fra35 0 -weight 1
    grid columnconf $base.fra21.fra35 1 -weight 1
    grid rowconf $base.fra21.fra35 0 -weight 1
    grid rowconf $base.fra21.fra35 1 -weight 1
    grid $base.fra21.fra35.rad36 \
        -in $base.fra21.fra35 -column "0" -row "1" -columnspan "1" \
        -rowspan "1" -sticky "ew" 
    grid $base.fra21.fra35.rad37 \
        -in $base.fra21.fra35 -column "1" -row "1" -columnspan "1" \
        -rowspan "1" -sticky "ew" 
    grid $base.fra21.fra35.rad38 \
        -in $base.fra21.fra35 -column "0" -row "0" -columnspan "1" \
        -rowspan "1" -sticky "ew" 
    grid $base.fra21.fra35.rad39 \
        -in $base.fra21.fra35 -column "1" -row "0" -columnspan "1" \
        -rowspan "1" -sticky "ew" 
    grid $base.fra30 \
        -in $base -column "0" -row "0" -columnspan "1" -rowspan "1" -padx "2" \
        -pady "2" -sticky "ew" 
    grid columnconf $base.fra30 0 -weight 1
    grid columnconf $base.fra30 1 -weight 1
    grid $base.fra30.fra19 \
        -in $base.fra30 -column "0" -row "0" -columnspan "1" -rowspan "1" \
        -sticky "w" 
    grid $base.fra30.fra19.but20 \
        -in $base.fra30.fra19 -column "0" -row "0" -columnspan "1" \
        -rowspan "1" 
    grid $base.fra30.fra19.but22 \
        -in $base.fra30.fra19 -column "2" -row "0" -columnspan "1" \
        -rowspan "1" 
    grid $base.fra30.fra19.but23 \
        -in $base.fra30.fra19 -column "3" -row "0" -columnspan "1" \
        -rowspan "1" 
    grid $base.fra30.fra19.but24 \
        -in $base.fra30.fra19 -column "4" -row "0" -columnspan "1" \
        -rowspan "1" 
    grid $base.fra30.fra19.but25 \
        -in $base.fra30.fra19 -column "5" -row "0" -columnspan "1" \
        -rowspan "1" 
    grid $base.fra30.but26 \
        -in $base.fra30 -column "1" -row "0" -columnspan "1" -rowspan "1" \
        -sticky "e" 

    grid rowconf $base.fra35 1 -weight 1
    grid columnconf $base.fra35 0 -weight 1
    grid columnconf $base.fra35 1 -weight 1

    grid $base.fra35.fra17 \
        -in $base.fra35 -column "0" -row "1" -columnspan "1" -rowspan "1" \
        -ipadx "2" -ipady "2" -padx "2" -pady "2" -sticky "nesw" 
    grid columnconf $base.fra35.fra17 0 -weight 1
    grid rowconf $base.fra35.fra17 5 -weight 1
    grid rowconf $base.fra35.fra17 7 -weight 1
    grid $base.fra35.fra17.but17 \
        -in $base.fra35.fra17 -column "0" -row "2" -columnspan "2" \
        -rowspan "1" -padx "2" -pady "2" -sticky "ew" 
    grid $base.fra35.fra17.but19 \
        -in $base.fra35.fra17 -column "0" -row "3" -columnspan "1" \
        -rowspan "1" -padx "2" -pady "2" -sticky "ew" 
    grid $base.fra35.fra17.but18 \
        -in $base.fra35.fra17 -column "0" -row "1" -columnspan "1" \
        -rowspan "1" -padx "2" -pady "2" -sticky "ew" 
    grid $base.fra35.fra17.lab19 \
        -in $base.fra35.fra17 -column "0" -row "0" -columnspan "1" \
        -rowspan "1" -padx "2" -pady "2" -sticky "ew" 
    grid $base.fra35.fra20 \
        -in $base.fra35 -column "1" -row "1" -columnspan "1" -rowspan "1" \
        -ipadx "2" -ipady "2" -padx "2" -pady "2" -sticky "nesw" 
    grid columnconf $base.fra35.fra20 0 -weight 1
    grid rowconf $base.fra35.fra20 6 -weight 1
    grid $base.fra35.fra20.but22 \
        -in $base.fra35.fra20 -column "0" -row "1" -columnspan "1" \
        -rowspan "1" -sticky "ew" 
    grid $base.fra35.fra20.but26 \
        -in $base.fra35.fra20 -column "0" -row "7" -columnspan "1" \
        -rowspan "1" -sticky "ew" 
    grid $base.fra35.fra20.che28 \
        -in $base.fra35.fra20 -column "0" -row "2" -columnspan "1" \
        -rowspan "1" -sticky "ew" 
    grid $base.fra35.fra20.fra22 \
        -in $base.fra35.fra20 -column "0" -row "6" -columnspan "1" \
        -rowspan "1" -sticky "nesw" 
    grid columnconf $base.fra35.fra20.fra22 0 -weight 1
    grid rowconf $base.fra35.fra20.fra22 0 -weight 1
    grid $base.fra35.fra20.fra22.01 \
        -in $base.fra35.fra20.fra22 -column "0" -row "0" -columnspan "1" \
        -rowspan "1" -sticky "nesw" 
    grid $base.fra35.fra20.fra22.02 \
        -in $base.fra35.fra20.fra22 -column "0" -row "1" -columnspan "1" \
        -rowspan "1" -sticky "ew" 
    grid $base.fra35.fra20.fra22.03 \
        -in $base.fra35.fra20.fra22 -column "1" -row "0" -columnspan "1" \
        -rowspan "1" -sticky "ns" 
    grid $base.fra35.fra20.but20 \
        -in $base.fra35.fra20 -column "0" -row "3" -columnspan "1" \
        -rowspan "1" -sticky "ew" 
    grid $base.fra35.fra20.but17 \
        -in $base.fra35.fra20 -column "0" -row "9" -columnspan "1" \
        -rowspan "1" -sticky "ew" 
    grid $base.fra35.fra20.lab28 \
        -in $base.fra35.fra20 -column "0" -row "0" -columnspan "1" \
        -rowspan "1" -padx "2" -pady "2" -sticky "new" 
    grid $base.fra35.fra20.but18 \
        -in $base.fra35.fra20 -column "0" -row "4" -columnspan "1" \
        -rowspan "1" -sticky "ew" 
    grid $base.fra35.fra20.but188 \
        -in $base.fra35.fra20 -column "0" -row "5" -columnspan "1" \
        -rowspan "1" -sticky "ew" 
    grid $base.fra35.fra20.but30 \
        -in $base.fra35.fra20 -column "0" -row "8" -columnspan "1" \
        -rowspan "1" -sticky "ew" 

    grid rowconf $base.fra44 2 -weight 1
    grid columnconf $base.fra44 0 -weight 1

    grid rowconf $base.fra44 2 -weight 1
    grid columnconf $base.fra44 0 -weight 1
    grid $base.fra44.fra27 \
        -in $base.fra44 -column "0" -row "3" -columnspan "2" -rowspan "1" \
        -ipadx "2" -ipady "2" -padx "4" -pady "2" -sticky "esw" 
    grid columnconf $base.fra44.fra27 0 -weight 1
    grid $base.fra44.fra27.but28 \
        -in $base.fra44.fra27 -column "0" -row "2" -columnspan "2" \
        -rowspan "1" -sticky "ew" 
    grid $base.fra44.fra19 \
        -in $base.fra44 -column "0" -row "2" -columnspan "2" -rowspan "1" \
        -ipady "10" -padx "2" -pady "10" -sticky "new" 
    grid columnconf $base.fra44.fra19 0 -weight 1
    grid columnconf $base.fra44.fra19 1 -weight 1
    grid $base.fra44.fra19.fra20 \
        -in $base.fra44.fra19 -column "0" -row "0" -columnspan "1" \
        -rowspan "1" -ipadx "2" -ipady "2" -padx "2" -sticky "ew" 
    grid columnconf $base.fra44.fra19.fra20 0 -weight 1
    grid $base.fra44.fra19.fra20.lab21 \
        -in $base.fra44.fra19.fra20 -column "0" -row "0" -columnspan "1" \
        -rowspan "1" -sticky "ew" 
    grid $base.fra44.fra19.fra20.ent22 \
        -in $base.fra44.fra19.fra20 -column "0" -row "1" -columnspan "2" \
        -rowspan "1" -sticky "ew" 
    grid $base.fra44.fra19.fra20.but23 \
        -in $base.fra44.fra19.fra20 -column "1" -row "0" -columnspan "1" \
        -rowspan "1" 
    grid $base.fra44.fra19.fra26 \
        -in $base.fra44.fra19 -column "0" -row "1" -columnspan "1" \
        -rowspan "1" -ipadx "2" -ipady "2" -padx "2" -sticky "ew" 
    grid columnconf $base.fra44.fra19.fra26 0 -weight 1
    grid $base.fra44.fra19.fra26.01 \
        -in $base.fra44.fra19.fra26 -column "0" -row "0" -columnspan "1" \
        -rowspan "1" -sticky "ew" 
    grid $base.fra44.fra19.fra26.02 \
        -in $base.fra44.fra19.fra26 -column "0" -row "1" -columnspan "2" \
        -rowspan "1" -sticky "ew" 
    grid $base.fra44.fra19.fra26.03 \
        -in $base.fra44.fra19.fra26 -column "1" -row "0" -columnspan "1" \
        -rowspan "1" 
    grid $base.fra44.fra19.fra27 \
        -in $base.fra44.fra19 -column "0" -row "2" -columnspan "1" \
        -rowspan "1" -ipadx "2" -ipady "2" -padx "2" -sticky "ew" 
    grid columnconf $base.fra44.fra19.fra27 0 -weight 1
    grid $base.fra44.fra19.fra27.01 \
        -in $base.fra44.fra19.fra27 -column "0" -row "0" -columnspan "1" \
        -rowspan "1" -sticky "ew" 
    grid $base.fra44.fra19.fra27.02 \
        -in $base.fra44.fra19.fra27 -column "0" -row "1" -columnspan "2" \
        -rowspan "1" -sticky "ew" 
    grid $base.fra44.fra19.fra27.03 \
        -in $base.fra44.fra19.fra27 -column "1" -row "0" -columnspan "1" \
        -rowspan "1" 
    grid $base.fra44.fra19.fra28 \
        -in $base.fra44.fra19 -column "1" -row "1" -columnspan "1" \
        -rowspan "1" -ipadx "2" -ipady "2" -padx "2" -sticky "ew" 
    grid columnconf $base.fra44.fra19.fra28 0 -weight 1
    grid $base.fra44.fra19.fra28.01 \
        -in $base.fra44.fra19.fra28 -column "0" -row "0" -columnspan "1" \
        -rowspan "1" -sticky "ew" 
    grid $base.fra44.fra19.fra28.02 \
        -in $base.fra44.fra19.fra28 -column "0" -row "1" -columnspan "2" \
        -rowspan "1" -sticky "ew" 
    grid $base.fra44.fra19.fra28.03 \
        -in $base.fra44.fra19.fra28 -column "1" -row "0" -columnspan "1" \
        -rowspan "1" 
    grid $base.fra44.fra19.fra29 \
        -in $base.fra44.fra19 -column "1" -row "0" -columnspan "1" \
        -rowspan "1" -ipadx "2" -ipady "2" -padx "2" -sticky "ew" 
    grid columnconf $base.fra44.fra19.fra29 0 -weight 1
    grid $base.fra44.fra19.fra29.01 \
        -in $base.fra44.fra19.fra29 -column "0" -row "0" -columnspan "1" \
        -rowspan "1" -sticky "ew" 
    grid $base.fra44.fra19.fra29.02 \
        -in $base.fra44.fra19.fra29 -column "0" -row "1" -columnspan "2" \
        -rowspan "1" -sticky "ew" 
    grid $base.fra44.fra19.fra29.03 \
        -in $base.fra44.fra19.fra29 -column "1" -row "0" -columnspan "1" \
        -rowspan "1" 
    grid $base.fra44.fra19.fra32 \
        -in $base.fra44.fra19 -column "0" -row "4" -columnspan "1" \
        -rowspan "1" -ipadx "2" -ipady "2" -padx "2" -sticky "ew" 
    grid columnconf $base.fra44.fra19.fra32 0 -weight 1
    grid $base.fra44.fra19.fra32.01 \
        -in $base.fra44.fra19.fra32 -column "0" -row "0" -columnspan "1" \
        -rowspan "1" -sticky "ew" 
    grid $base.fra44.fra19.fra32.men35 \
        -in $base.fra44.fra19.fra32 -column "0" -row "1" -columnspan "1" \
        -rowspan "1" -padx "2" -sticky "ew" 
    grid $base.fra44.fra19.fra30 \
        -in $base.fra44.fra19 -column "1" -row "4" -columnspan "1" \
        -rowspan "1" -ipadx "2" -ipady "2" -padx "2" -sticky "ew" 
    grid columnconf $base.fra44.fra19.fra30 0 -weight 1
    grid $base.fra44.fra19.fra30.01 \
        -in $base.fra44.fra19.fra30 -column "0" -row "0" -columnspan "1" \
        -rowspan "1" -sticky "ew" 
    grid $base.fra44.fra19.fra30.men34 \
        -in $base.fra44.fra19.fra30 -column "0" -row "1" -columnspan "1" \
        -rowspan "1" -sticky "ew" 
    grid $base.fra44.fra19.fra18 \
        -in $base.fra44.fra19 -column "1" -row "2" -columnspan "1" \
        -rowspan "1" -ipadx "2" -ipady "2" -padx "2" -sticky "ew" 
    grid columnconf $base.fra44.fra19.fra18 0 -weight 1
    grid $base.fra44.fra19.fra18.01 \
        -in $base.fra44.fra19.fra18 -column "0" -row "0" -columnspan "1" \
        -rowspan "1" -sticky "ew" 
    grid $base.fra44.fra19.fra18.02 \
        -in $base.fra44.fra19.fra18 -column "0" -row "1" -columnspan "2" \
        -rowspan "1" -sticky "ew" 
    grid $base.fra44.fra19.fra18.03 \
        -in $base.fra44.fra19.fra18 -column "1" -row "0" -columnspan "1" \
        -rowspan "1" 
    grid $base.fra44.fra19.fra34 \
        -in $base.fra44.fra19 -column "0" -row "3" -columnspan "1" \
        -rowspan "1" -ipadx "2" -ipady "2" -padx "2" -sticky "ew" 
    grid columnconf $base.fra44.fra19.fra34 0 -weight 1
    grid $base.fra44.fra19.fra34.lab35 \
        -in $base.fra44.fra19.fra34 -column "0" -row "0" -columnspan "1" \
        -rowspan "1" -sticky "ew" 
    grid $base.fra44.fra19.fra34.ent36 \
        -in $base.fra44.fra19.fra34 -column "0" -row "1" -columnspan "2" \
        -rowspan "1" -sticky "ew" 
    grid $base.fra44.fra19.fra34.but34 \
        -in $base.fra44.fra19.fra34 -column "1" -row "0" -columnspan "1" \
        -rowspan "1" 
    grid $base.fra44.lab30 \
        -in $base.fra44 -column "0" -row "0" -columnspan "2" -rowspan "1" \
        -padx "2" -pady "2" -sticky "new" 
    grid $base.fra46 \
        -in $base -column "0" -row "1" -columnspan "1" -rowspan "1" \
        -sticky "nesw" 
    grid columnconf $base.fra46 0 -weight 1
    grid rowconf $base.fra46 0 -weight 1
    grid $base.fra46.fra26 \
        -in $base.fra46 -column "0" -row "0" -columnspan "1" -rowspan "1" \
        -ipadx "2" -ipady "2" -padx "2" -pady "2" -sticky "nesw" 
    grid columnconf $base.fra46.fra26 0 -weight 1
    grid rowconf $base.fra46.fra26 1 -weight 1
    grid $base.fra46.fra26.fra27 \
        -in $base.fra46.fra26 -column "0" -row "1" -columnspan "1" \
        -rowspan "1" -padx "2" -pady "2" -sticky "nesw" 
    grid columnconf $base.fra46.fra26.fra27 0 -weight 1
    grid rowconf $base.fra46.fra26.fra27 0 -weight 1
    grid $base.fra46.fra26.fra27.01 \
        -in $base.fra46.fra26.fra27 -column "0" -row "0" -columnspan "1" \
        -rowspan "1" -sticky "nesw" 
    grid $base.fra46.fra26.fra27.02 \
        -in $base.fra46.fra26.fra27 -column "0" -row "1" -columnspan "1" \
        -rowspan "1" -sticky "ew" 
    grid $base.fra46.fra26.fra27.03 \
        -in $base.fra46.fra26.fra27 -column "1" -row "0" -columnspan "1" \
        -rowspan "1" -sticky "ns" 
    grid $base.fra46.fra26.fra29 \
        -in $base.fra46.fra26 -column "1" -row "1" -columnspan "1" \
        -rowspan "1" -ipadx "2" -ipady "2" -padx "2" -pady "2" -sticky "ns" 
    grid rowconf $base.fra46.fra26.fra29 4 -weight 1
    grid $base.fra46.fra26.fra29.fra28 \
        -in $base.fra46.fra26.fra29 -column "0" -row "4" -columnspan "1" \
        -rowspan "1" -sticky "esw" 
    grid columnconf $base.fra46.fra26.fra29.fra28 0 -weight 1
    grid $base.fra46.fra26.fra29.fra28.but29 \
        -in $base.fra46.fra26.fra29.fra28 -column "0" -row "0" \
        -columnspan "1" -rowspan "1" -sticky "ew" 
    grid $base.fra46.fra26.fra29.fra28.but30 \
        -in $base.fra46.fra26.fra29.fra28 -column "0" -row "1" \
        -columnspan "1" -rowspan "1" -sticky "ew" 
    grid $base.fra46.fra26.fra29.fra28.but31 \
        -in $base.fra46.fra26.fra29.fra28 -column "0" -row "2" \
        -columnspan "1" -rowspan "1" -sticky "ew" 
    grid $base.fra46.fra26.fra29.fra36 \
        -in $base.fra46.fra26.fra29 -column "0" -row "0" -columnspan "1" \
        -rowspan "1" 
    grid $base.fra46.fra26.fra29.fra36.but37 \
        -in $base.fra46.fra26.fra29.fra36 -column "0" -row "0" \
        -columnspan "1" -rowspan "1" -sticky "ew" 
    grid $base.fra46.fra26.fra29.fra36.but38 \
        -in $base.fra46.fra26.fra29.fra36 -column "0" -row "1" \
        -columnspan "1" -rowspan "1" -sticky "ew" 
    grid $base.fra46.fra26.fra29.fra36.but39 \
        -in $base.fra46.fra26.fra29.fra36 -column "0" -row "2" \
        -columnspan "1" -rowspan "1" -sticky "ew" 
    grid $base.fra46.fra26.fra29.fra36.but34 \
        -in $base.fra46.fra26.fra29.fra36 -column "0" -row "3" \
        -columnspan "1" -rowspan "1" -sticky "ew" 
    grid $base.fra46.fra26.lab34 \
        -in $base.fra46.fra26 -column "0" -row "0" -columnspan "2" \
        -rowspan "1" -sticky "ew" 
    grid $base.fra17 \
        -in $base -column "0" -row "2" -columnspan "1" -rowspan "1" -padx "2" \
        -pady "2" -sticky "ew" 
    grid columnconf $base.fra17 0 -weight 1
    grid columnconf $base.fra17 1 -weight 1
    grid $base.fra17.lab18 \
        -in $base.fra17 -column "0" -row "0" -columnspan "1" -rowspan "1" \
        -sticky "ew" 
    grid $base.fra17.lab19 \
        -in $base.fra17 -column "1" -row "0" -columnspan "1" -rowspan "1" \
        -sticky "ew" 
}

proc vTclWindow.top18 {base {container 0}} {
    if {$base == ""} {
        set base .top18
    }
    if {[winfo exists $base] && (!$container)} {
        wm deiconify $base; return
    }
    ###################
    # CREATING WIDGETS
    ###################
    if {!$container} {
    toplevel $base -class Toplevel  -borderwidth "10" -highlightcolor "#000000" 
    wm focusmodel $base passive
    wm geometry $base 250x150
    wm maxsize $base 1265 994
    wm minsize $base 115 1
    wm overrideredirect $base 0
    wm resizable $base 1 1
    wm title $base "secpanel - Confirm"
    }
    message $base.mes19  -aspect "300" -padx "5" -pady "2" 
    frame $base.fra20  -borderwidth "1" -height "75" -highlightcolor "#000000" -width "125" 
    button $base.fra20.but21  -command "set questres 1" -highlightcolor "#000000" -padx "9"  -pady "3" -relief "groove" -text "Yes" -bg orange
    button $base.fra20.but22  -command "set questres 0" -highlightcolor "#000000" -padx "9"  -pady "3" -relief "groove" -text "No" -bg IndianRed
    ###################
    # SETTING GEOMETRY
    ###################
    grid columnconf $base 0 -weight 1
    grid rowconf $base 0 -weight 1
    grid $base.mes19  -in $base -column "0" -row "0" -columnspan "1" -rowspan "1"  -sticky "nesw" 
    grid $base.fra20  -in $base -column "0" -row "1" -columnspan "1" -rowspan "1"  -sticky "ew" 
    grid columnconf $base.fra20 0 -weight 1
    grid columnconf $base.fra20 1 -weight 1
    grid $base.fra20.but21  -in $base.fra20 -column "0" -row "0" -columnspan "1" -rowspan "1"  -sticky "ew" 
    grid $base.fra20.but22  -in $base.fra20 -column "1" -row "0" -columnspan "1" -rowspan "1"  -sticky "ew"
}

proc vTclWindow.top19 {base {container 0}} {
    if {$base == ""} {
        set base .top19
    }
    if {[winfo exists $base] && (!$container)} {
        wm deiconify $base; return
    }
    ###################
    # CREATING WIDGETS
    ###################
    if {!$container} {
    toplevel $base -class Toplevel  -borderwidth "4" -menu "$base.m17" -highlightcolor "#000000" 
    wm focusmodel $base passive
    wm geometry $base 666x183
    wm maxsize $base 1009 738
    wm minsize $base 115 1
    wm overrideredirect $base 0
    wm resizable $base 1 1
    wm title $base "secpanel - View key"
    }
    label $base.lab19  -anchor "w" -borderwidth "1" -highlightcolor "#000000"  -text "Fingerprint" 
    label $base.lab39  -anchor "w" -borderwidth "0" -highlightcolor "#000000"  -relief "raised" -text "Bubblebabble digest" 
    button $base.but41  -command "Window destroy .top19" -highlightcolor "#000000" -padx "9"  -pady "3" -relief "groove" -text "Close" -bg IndianRed
    menu $base.m17  -activeborderwidth "1" -borderwidth "1" -cursor "" -tearoff "0" 
    frame $base.fra22  -borderwidth "2" -relief "groove" -height "75"  -highlightcolor "#000000" -width "125" 
    label $base.fra22.lab23  -anchor "w" -borderwidth "0" -highlightcolor "#000000"  -relief "raised" 
    label $base.fra22.lab24  -anchor "w" -borderwidth "0" -highlightcolor "#000000"  -relief "raised" 
    text $base.tex17  -borderwidth "0" -height "1" -highlightcolor "#000000"  -width "8" 
    text $base.tex18  -borderwidth "0" -cursor "fleur" -height "1"  -highlightcolor "#000000" -width "8" 
    ###################
    # SETTING GEOMETRY
    ###################
    grid columnconf $base 0 -weight 1
    grid rowconf $base 2 -weight 1
    grid rowconf $base 4 -weight 1
    grid $base.lab19  -in $base -column "0" -row "1" -columnspan "2" -rowspan "1" -padx "2"  -sticky "ew" 
    grid $base.lab39  -in $base -column "0" -row "3" -columnspan "2" -rowspan "1" -padx "2"  -sticky "ew" 
    grid $base.but41  -in $base -column "0" -row "5" -columnspan "2" -rowspan "1" -padx "2"  -pady "2" -sticky "ew" 
    grid $base.fra22  -in $base -column "0" -row "0" -columnspan "1" -rowspan "1"  -sticky "ew" 
    grid columnconf $base.fra22 0 -weight 1
    grid columnconf $base.fra22 1 -weight 1
    grid $base.fra22.lab23  -in $base.fra22 -column "0" -row "0" -columnspan "1" -rowspan "1"  -sticky "ew" 
    grid $base.fra22.lab24  -in $base.fra22 -column "1" -row "0" -columnspan "1" -rowspan "1"  -sticky "ew" 
    grid $base.tex17  -in $base -column "0" -row "2" -columnspan "1" -rowspan "1" -padx "2"  -pady "2" -sticky "nesw" 
    grid $base.tex18  -in $base -column "0" -row "4" -columnspan "1" -rowspan "1" -padx "2"  -pady "2" -sticky "nesw"
}

proc vTclWindow.top20 {base {container 0}} {
    if {$base == ""} {
        set base .top20
    }
    if {[winfo exists $base] && (!$container)} {
        wm deiconify $base; return
    }
    ###################
    # CREATING WIDGETS
    ###################
    if {!$container} {
    toplevel $base -class Toplevel  -borderwidth "4" -highlightcolor "#000000" 
    wm focusmodel $base passive
    wm geometry $base 292x175
    wm maxsize $base 785 570
    wm minsize $base 115 1
    wm overrideredirect $base 0
    wm resizable $base 1 1
    wm title $base "secpanel - Change Key-Properties"
    }
    frame $base.fra21  -borderwidth "1" -height "75" -highlightcolor "#000000" -width "125" 
    label $base.fra21.lab25  -anchor "w" -borderwidth "1" -highlightcolor "#000000"  -text "Old password:" 
    entry $base.fra21.ent26  -borderwidth "1" -highlightcolor "#000000" -show "*" 
    label $base.fra21.lab27  -anchor "w" -borderwidth "1" -highlightcolor "#000000"  -text "New password:" 
    label $base.fra21.lab28  -anchor "w" -borderwidth "1" -highlightcolor "#000000"  -text "New password:" 
    entry $base.fra21.ent30  -borderwidth "1" -highlightcolor "#000000" -show "*" 
    entry $base.fra21.ent31  -borderwidth "1" -highlightcolor "#000000" -show "*" 
    label $base.fra21.lab19  -anchor "w" -borderwidth "1" -highlightcolor "#000000"  -text "Comment:" 
    entry $base.fra21.ent20  -borderwidth "1" -highlightcolor "#000000" 
    frame $base.fra22  -borderwidth "2" -relief "groove" -height "75"  -highlightcolor "#000000" -width "125" 
    button $base.fra22.but23  -command "Window destroy .top20" -highlightcolor "#000000" -padx "9"  -pady "3" -relief "groove" -text "Cancel" -bg IndianRed
    button $base.fra22.but33  -command "keygen chpwd" -highlightcolor "#000000" -padx "9" -pady "3"  -relief "groove" -text "Save" -bg orange
    label $base.lab35  -anchor "w" -borderwidth "1" -highlightcolor "#000000"  -text "Changing properties of" 
    label $base.lab36  -anchor "w" -borderwidth "1" -highlightcolor "#000000" 
    ###################
    # SETTING GEOMETRY
    ###################
    grid columnconf $base 0 -weight 1
    grid rowconf $base 2 -weight 1
    grid $base.fra21  -in $base -column "0" -row "2" -columnspan "1" -rowspan "1"  -ipadx "2" -ipady "2" -padx "2" -pady "2" -sticky "nesw" 
    grid columnconf $base.fra21 1 -weight 1
    grid $base.fra21.lab25  -in $base.fra21 -column "0" -row "0" -columnspan "1" -rowspan "1"  -sticky "ew" 
    grid $base.fra21.ent26  -in $base.fra21 -column "1" -row "0" -columnspan "1" -rowspan "1"  -padx "2" -sticky "ew" 
    grid $base.fra21.lab27  -in $base.fra21 -column "0" -row "1" -columnspan "1" -rowspan "1"  -sticky "ew" 
    grid $base.fra21.lab28  -in $base.fra21 -column "0" -row "2" -columnspan "1" -rowspan "1"  -sticky "ew" 
    grid $base.fra21.ent30  -in $base.fra21 -column "1" -row "1" -columnspan "1" -rowspan "1"  -padx "2" -sticky "ew" 
    grid $base.fra21.ent31  -in $base.fra21 -column "1" -row "2" -columnspan "1" -rowspan "1"  -padx "2" -sticky "ew" 
    grid $base.fra21.lab19  -in $base.fra21 -column "0" -row "3" -columnspan "1" -rowspan "1"  -sticky "ew" 
    grid $base.fra21.ent20  -in $base.fra21 -column "1" -row "3" -columnspan "1" -rowspan "1"  -padx "2" -sticky "ew" 
    grid $base.fra22  -in $base -column "0" -row "3" -columnspan "1" -rowspan "1"  -ipadx "2" -ipady "2" -padx "2" -pady "2" -sticky "ew" 
    grid $base.fra22.but23  -in $base.fra22 -column "0" -row "0" -columnspan "1" -rowspan "1" 
    grid $base.fra22.but33  -in $base.fra22 -column "1" -row "0" -columnspan "1" -rowspan "1" 
    grid $base.lab35  -in $base -column "0" -row "0" -columnspan "1" -rowspan "1" -padx "2"  -sticky "ew" 
    grid $base.lab36  -in $base -column "0" -row "1" -columnspan "1" -rowspan "1" -padx "2"  -sticky "ew"
}

proc vTclWindow.top21 {base {container 0}} {
    if {$base == ""} {
        set base .top21
    }
    if {[winfo exists $base] && (!$container)} {
        wm deiconify $base; return
    }
    ###################
    # CREATING WIDGETS
    ###################
    if {!$container} {
    toplevel $base -class Toplevel  -borderwidth "4" -highlightcolor "#000000" 
    wm focusmodel $base passive
    wm geometry $base 199x124
    wm maxsize $base 1265 994
    wm minsize $base 115 1
    wm overrideredirect $base 0
    wm resizable $base 1 1
    wm title $base "secpanel - Ask for user"
    }
    frame $base.fra22  -borderwidth "2" -relief "groove" -height "75"  -highlightcolor "#000000" -width "125" 
    message $base.fra22.mes23  -anchor "w" -aspect "300" -padx "5" -pady "2"  -text "Give login to use for this connection:" 
    entry $base.fra22.ent24  -borderwidth "1" -highlightcolor "#000000" 
    bind $base.fra22.ent24 <Key-Return> {
        getuser
    }
    button $base.but25  -command "set userres #####" -highlightcolor "#000000" -padx "9"  -pady "3" -relief "groove" -text "Cancel" -bg IndianRed
    button $base.but26  -command "getuser" -highlightcolor "#000000" -padx "9" -pady "3"  -relief "groove" -text "Connect" -bg orange
    ###################
    # SETTING GEOMETRY
    ###################
    grid columnconf $base 0 -weight 1
    grid columnconf $base 1 -weight 1
    grid rowconf $base 0 -weight 1
    grid $base.fra22  -in $base -column "0" -row "0" -columnspan "2" -rowspan "1"  -ipadx "2" -ipady "2" -padx "2" -pady "2" -sticky "nesw" 
    grid columnconf $base.fra22 0 -weight 1
    grid rowconf $base.fra22 0 -weight 1
    grid $base.fra22.mes23  -in $base.fra22 -column "0" -row "0" -columnspan "1" -rowspan "1"  -sticky "nesw" 
    grid $base.fra22.ent24  -in $base.fra22 -column "0" -row "1" -columnspan "1" -rowspan "1"  -sticky "ew" 
    grid $base.but25  -in $base -column "0" -row "1" -columnspan "1" -rowspan "1" -padx "2"  -pady "2" -sticky "ew" 
    grid $base.but26  -in $base -column "1" -row "1" -columnspan "1" -rowspan "1" -padx "2"  -pady "2" -sticky "ew"
}

proc vTclWindow.top22 {base {container 0}} {
    if {$base == ""} {
        set base .top22
    }
    if {[winfo exists $base] && (!$container)} {
        wm deiconify $base; return
    }
    ###################
    # CREATING WIDGETS
    ###################
    if {!$container} {
    toplevel $base -class Toplevel  -borderwidth "4" -highlightcolor "#000000" 
    wm focusmodel $base passive
    wm geometry $base 250x150
    wm maxsize $base 1265 994
    wm minsize $base 115 1
    wm overrideredirect $base 0
    wm resizable $base 1 1
    wm title $base "secpanel - Message"
    }
    message $base.mes23  -aspect "300" -padx "5" -pady "2" 
    label $base.lab26  -bitmap "info" -highlightcolor "#000000" -text "label" 
    button $base.but17  -command "Window destroy .top22" -highlightcolor "#000000" -padx "9"  -pady "3" -relief "groove" -text "OK" -bg orange
    ###################
    # SETTING GEOMETRY
    ###################
    grid columnconf $base 1 -weight 1
    grid rowconf $base 0 -weight 1
    grid $base.mes23  -in $base -column "1" -row "0" -columnspan "1" -rowspan "1" -padx "4"  -pady "4" -sticky "nesw" 
    grid $base.lab26  -in $base -column "0" -row "0" -columnspan "1" -rowspan "1"  -ipadx "4" -ipady "4" -padx "4" -pady "4" -sticky "nesw" 
    grid $base.but17  -in $base -column "0" -row "1" -columnspan "2" -rowspan "1" -padx "2"  -pady "2" -sticky "ew"
}

proc vTclWindow.top23 {base {container 0}} {
    if {$base == ""} {
        set base .top23
    }
    if {[winfo exists $base] && (!$container)} {
        wm deiconify $base; return
    }
    ###################
    # CREATING WIDGETS
    ###################
    if {!$container} {
    toplevel $base -class Toplevel  -borderwidth "4" -highlightcolor "#000000" 
    wm focusmodel $base passive
    wm geometry $base 408x288
    wm maxsize $base 1265 994
    wm minsize $base 115 1
    wm overrideredirect $base 0
    wm resizable $base 1 1
    wm title $base "secpanel - GUI font settings"
    }
    frame $base.fra25  -borderwidth "2" -relief "groove" -height "75"  -highlightcolor "#000000" -width "125" 
    button $base.fra25.but32  -command "fontman save" -highlightcolor "#000000" -relief "groove"  -text "Save" -bg orange
    button $base.fra25.but36  -command "Window destroy .top23" -highlightcolor "#000000"  -relief "groove" -text "Cancel" -bg IndianRed
    frame $base.fra18  -borderwidth "2" -relief "groove" -height "75"  -highlightcolor "#000000" -width "125" 
    checkbutton $base.fra18.che19  -borderwidth "1" -command "fontman ul" -highlightcolor "#000000"  -text "Use system fonts" -variable "configs(sysfonts)" -bg orange
    label $base.lab30  -borderwidth "0" -highlightcolor "#000000" -text "Example text" 
    frame $base.fra36  -height "75" -highlightcolor "#000000" -width "125" 
    frame $base.fra36.cpd37  -height "30" -highlightcolor "#000000" -width "30" 
    listbox $base.fra36.cpd37.01  -borderwidth "1" -highlightcolor "#000000"  -xscrollcommand "$base.fra36.cpd37.02 set"  -yscrollcommand "$base.fra36.cpd37.03 set" 
    bind $base.fra36.cpd37.01 <Button-1> {
        .top23.fra36.cpd37.01 activate @%x,%y
fontman ul
    }
    scrollbar $base.fra36.cpd37.02  -borderwidth "1" -command "$base.fra36.cpd37.01 xview"  -cursor "left_ptr" -highlightcolor "#000000" -orient "horizontal"  -troughcolor "#e2e2de" -width "10" 
    scrollbar $base.fra36.cpd37.03  -borderwidth "1" -command "$base.fra36.cpd37.01 yview"  -cursor "left_ptr" -highlightcolor "#000000" -troughcolor "#e2e2de"  -width "10" 
    frame $base.fra36.cpd38  -height "30" -highlightcolor "#000000" -width "30" 
    listbox $base.fra36.cpd38.01  -borderwidth "1" -height "2" -highlightcolor "#000000" -width "4"  -xscrollcommand "$base.fra36.cpd38.02 set"  -yscrollcommand "$base.fra36.cpd38.03 set" 
    bind $base.fra36.cpd38.01 <Button-1> {
        .top23.fra36.cpd38.01 activate @%x,%y
fontman ul
    }
    scrollbar $base.fra36.cpd38.02  -borderwidth "1" -command "$base.fra36.cpd38.01 xview"  -cursor "left_ptr" -highlightcolor "#000000" -orient "horizontal"  -troughcolor "#e2e2de" -width "10" 
    scrollbar $base.fra36.cpd38.03  -borderwidth "1" -command "$base.fra36.cpd38.01 yview"  -cursor "left_ptr" -highlightcolor "#000000" -troughcolor "#e2e2de"  -width "10" 
    frame $base.fra36.fra39  -height "75" -highlightcolor "#000000" -width "125" 
    checkbutton $base.fra36.fra39.che40  -anchor "w" -borderwidth "1" -command "fontman ul"  -highlightcolor "#000000" -text "Bold" -variable "configs(fontbold)" -bg orange
    checkbutton $base.fra36.fra39.che41  -anchor "w" -borderwidth "1" -command "fontman ul"  -highlightcolor "#000000" -text "Italic"  -variable "configs(fontitalic)" -bg orange
    ###################
    # SETTING GEOMETRY
    ###################
    grid columnconf $base 0 -weight 1
    grid rowconf $base 0 -weight 1
    grid $base.fra25  -in $base -column "0" -row "3" -columnspan "1" -rowspan "1"  -ipadx "2" -ipady "2" -padx "2" -pady "2" -sticky "ew" 
    grid $base.fra25.but32  -in $base.fra25 -column "1" -row "0" -columnspan "1" -rowspan "1" 
    grid $base.fra25.but36  -in $base.fra25 -column "0" -row "0" -columnspan "1" -rowspan "1" 
    grid $base.fra18  -in $base -column "0" -row "2" -columnspan "1" -rowspan "1"  -ipadx "2" -ipady "2" -padx "2" -pady "2" -sticky "ew" 
    grid $base.fra18.che19  -in $base.fra18 -column "0" -row "0" -columnspan "1" -rowspan "1" 
    grid $base.lab30  -in $base -column "0" -row "1" -columnspan "1" -rowspan "1"  -ipadx "2" -ipady "2" -padx "2" -pady "2" -sticky "ew" 
    grid $base.fra36  -in $base -column "0" -row "0" -columnspan "1" -rowspan "1"  -ipadx "2" -ipady "2" -padx "2" -pady "2" -sticky "nesw" 
    grid columnconf $base.fra36 0 -weight 1
    grid rowconf $base.fra36 0 -weight 1
    grid $base.fra36.cpd37  -in $base.fra36 -column "0" -row "0" -columnspan "1" -rowspan "1"  -sticky "nesw" 
    grid columnconf $base.fra36.cpd37 0 -weight 1
    grid rowconf $base.fra36.cpd37 0 -weight 1
    grid $base.fra36.cpd37.01  -in $base.fra36.cpd37 -column "0" -row "0" -columnspan "1"  -rowspan "1" -sticky "nesw" 
    grid $base.fra36.cpd37.02  -in $base.fra36.cpd37 -column "0" -row "1" -columnspan "1"  -rowspan "1" -sticky "ew" 
    grid $base.fra36.cpd37.03  -in $base.fra36.cpd37 -column "1" -row "0" -columnspan "1"  -rowspan "1" -sticky "ns" 
    grid $base.fra36.cpd38  -in $base.fra36 -column "1" -row "0" -columnspan "1" -rowspan "1"  -sticky "ns" 
    grid columnconf $base.fra36.cpd38 0 -weight 1
    grid rowconf $base.fra36.cpd38 0 -weight 1
    grid $base.fra36.cpd38.01  -in $base.fra36.cpd38 -column "0" -row "0" -columnspan "1"  -rowspan "1" -sticky "nesw" 
    grid $base.fra36.cpd38.02  -in $base.fra36.cpd38 -column "0" -row "1" -columnspan "1"  -rowspan "1" -sticky "ew" 
    grid $base.fra36.cpd38.03  -in $base.fra36.cpd38 -column "1" -row "0" -columnspan "1"  -rowspan "1" -sticky "ns" 
    grid $base.fra36.fra39  -in $base.fra36 -column "2" -row "0" -columnspan "1" -rowspan "1"  -ipadx "2" -ipady "2" -padx "2" -pady "2" -sticky "n" 
    grid $base.fra36.fra39.che40  -in $base.fra36.fra39 -column "0" -row "0" -columnspan "1"  -rowspan "1" -sticky "ew" 
    grid $base.fra36.fra39.che41  -in $base.fra36.fra39 -column "0" -row "1" -columnspan "1"  -rowspan "1" -sticky "ew"
}

proc vTclWindow.top24 {base {container 0}} {
    if {$base == ""} {
        set base .top24
    }
    if {[winfo exists $base] && (!$container)} {
        wm deiconify $base; return
    }
    ###################
    # CREATING WIDGETS
    ###################
    if {!$container} {
    toplevel $base -class Toplevel  -borderwidth "4" -highlightcolor "#000000" 
    wm focusmodel $base passive
    wm geometry $base 500x200
    wm maxsize $base 1265 930
    wm minsize $base 115 1
    wm overrideredirect $base 0
    wm resizable $base 1 1
    wm title $base "secpanel - Command Trace"
    }
    frame $base.cpd25  -relief "raised" -height "30" -highlightcolor "#000000" -width "30" 
    scrollbar $base.cpd25.01  -borderwidth "1" -command "$base.cpd25.03 xview" -cursor "left_ptr"  -highlightcolor "#000000" -orient "horizontal" -troughcolor "#e2e2de"  -width "10" 
    scrollbar $base.cpd25.02  -borderwidth "1" -command "$base.cpd25.03 yview" -cursor "left_ptr"  -highlightcolor "#000000" -troughcolor "#e2e2de" -width "10" 
    text $base.cpd25.03  -borderwidth "1"  -height "1" -highlightcolor "#000000"  -width "8" -xscrollcommand "$base.cpd25.01 set"  -yscrollcommand "$base.cpd25.02 set" 
    button $base.but26  -command "destroyfilter .top24" -highlightcolor "#000000"  -relief "groove" -text "Close" -bg IndianRed
    ###################
    # SETTING GEOMETRY
    ###################
    grid columnconf $base 0 -weight 1
    grid rowconf $base 0 -weight 1
    grid $base.cpd25  -in $base -column "0" -row "0" -columnspan "1" -rowspan "1"  -sticky "nesw" 
    grid columnconf $base.cpd25 0 -weight 1
    grid rowconf $base.cpd25 0 -weight 1
    grid $base.cpd25.01  -in $base.cpd25 -column "0" -row "1" -columnspan "1" -rowspan "1"  -sticky "ew" 
    grid $base.cpd25.02  -in $base.cpd25 -column "1" -row "0" -columnspan "1" -rowspan "1"  -sticky "ns" 
    grid $base.cpd25.03  -in $base.cpd25 -column "0" -row "0" -columnspan "1" -rowspan "1"  -sticky "nesw" 
    grid $base.but26  -in $base -column "0" -row "1" -columnspan "1" -rowspan "1" -padx "2"  -pady "2" -sticky "ew"
}

proc vTclWindow.top25 {base {container 0}} {
    if {$base == ""} {
        set base .top25
    }
    if {[winfo exists $base] && (!$container)} {
        wm deiconify $base; return
    }
    ###################
    # CREATING WIDGETS
    ###################
    if {!$container} {
    toplevel $base -class Toplevel  -borderwidth "4" -highlightcolor "#000000" 
    wm focusmodel $base passive
    wm geometry $base 204x120
    wm maxsize $base 1009 738
    wm minsize $base 115 1
    wm overrideredirect $base 0
    wm resizable $base 1 1
    wm title $base "secpanel - About"
    }
    frame $base.fra26  -borderwidth "1" -height "75" -highlightcolor "#000000" -width "125" 
    message $base.fra26.mes28  -aspect "550" -borderwidth "1" -justify "center"  -padx "5" -pady "2"  -text "Author: Steffen Leich-Nienhaus
steffen.leich _at_ gmail.com" 
    label $base.fra26.lab21  -borderwidth "1" -highlightcolor "#000000" 
    frame $base.fra27  -borderwidth "1" -height "75" -highlightcolor "#000000" -width "125" 
    button $base.fra27.but29  -command "Window destroy .top25" -highlightcolor "#000000" -padx "9"  -pady "3" -relief "groove" -text "Close" -bg IndianRed
    ###################
    # SETTING GEOMETRY
    ###################
    grid columnconf $base 0 -weight 1
    grid rowconf $base 0 -weight 1
    grid $base.fra26  -in $base -column "0" -row "0" -columnspan "1" -rowspan "1"  -ipadx "2" -ipady "2" -padx "2" -pady "2" -sticky "nesw" 
    grid $base.fra26.mes28  -in $base.fra26 -column "0" -row "1" -columnspan "1" -rowspan "1"  -sticky "ew" 
    grid $base.fra26.lab21  -in $base.fra26 -column "0" -row "0" -columnspan "1" -rowspan "1"  -sticky "ew" 
    grid $base.fra27  -in $base -column "0" -row "1" -columnspan "1" -rowspan "1"  -ipadx "2" -ipady "2" -padx "2" -pady "2" -sticky "ew" 
    grid columnconf $base.fra27 0 -weight 1
    grid $base.fra27.but29  -in $base.fra27 -column "0" -row "0" -columnspan "1" -rowspan "1"  -padx "2" -pady "2" -sticky "ew"
}

proc vTclWindow.top26 {base {container 0}} {
    if {$base == ""} {
        set base .top26
    }
    if {[winfo exists $base] && (!$container)} {
        wm deiconify $base; return
    }
    ###################
    # CREATING WIDGETS
    ###################
    if {!$container} {
    toplevel $base -class Toplevel  -borderwidth "4" -highlightcolor "#000000" 
    wm focusmodel $base passive
    wm geometry $base 420x240
    wm maxsize $base 1265 994
    wm minsize $base 115 1
    wm overrideredirect $base 0
    wm resizable $base 1 1
    wm title $base "secpanel - Command History"
    }
    frame $base.cpd27  -borderwidth "1" -height "30" -highlightcolor "#000000" -width "30" 
    scrollbar $base.cpd27.01  -borderwidth "1" -command "$base.cpd27.03 xview" -cursor "left_ptr"  -highlightcolor "#000000" -orient "horizontal" -troughcolor "#e2e2de"  -width "10" 
    scrollbar $base.cpd27.02  -borderwidth "1" -command "$base.cpd27.03 yview" -cursor "left_ptr"  -highlightcolor "#000000" -troughcolor "#e2e2de" -width "10" 
    text $base.cpd27.03  -borderwidth "1" -height "1" -highlightcolor "#000000"  -width "8" -wrap "none" -xscrollcommand "$base.cpd27.01 set"  -yscrollcommand "$base.cpd27.02 set" 
    frame $base.fra17  -relief "groove" -height "75" -highlightcolor "#000000" -width "125" 
    radiobutton $base.fra17.rad25  -anchor "w" -borderwidth "1" -command "historyman print"  -highlightcolor "#000000" -indicatoron "0" -relief "raised"  -selectcolor "#8c8cff" -text "Keydist" -value "5" -variable "histjob" 
    radiobutton $base.fra17.rad26  -anchor "w" -borderwidth "1" -command "historyman print"  -highlightcolor "#000000" -indicatoron "0" -relief "ridge"  -selectcolor "#8c8cff" -text "Keygen" -value "4" -variable "histjob" 
    radiobutton $base.fra17.rad28  -anchor "w" -borderwidth "1" -command "historyman print"  -highlightcolor "#000000" -indicatoron "0" -selectcolor "#8c8cff"  -text "SFTP connect" -value "2" -variable "histjob" 
    radiobutton $base.fra17.rad29  -anchor "w" -borderwidth "1" -command "historyman print"  -highlightcolor "#000000" -indicatoron "0" -relief "sunken"  -selectcolor "#8c8cff" -text "SSH connect" -value "1"  -variable "histjob" 
    radiobutton $base.fra17.rad37  -anchor "w" -borderwidth "1" -command "historyman print"  -highlightcolor "#000000" -indicatoron "0" -relief "raised"  -selectcolor "#8c8cff" -text "Agent" -value "6" -variable "histjob" 
    button $base.but18  -command "destroyfilter .top26" -highlightcolor "#000000"  -relief "groove" -text "Close" 
    menubutton $base.men17  -highlightcolor "#000000" -indicatoron "1" -menu "$base.men17.m"  -padx "4" -pady "3" -relief "groove" -text "Clear hist." 
    menu $base.men17.m  -activeborderwidth "1" -borderwidth "1" -cursor "" -tearoff "0" 
    $base.men17.m add command  -command "historyman clear 30" -label "Keep last 30 days" 
    $base.men17.m add command  -command "historyman clear 7" -label "Keep last 7 days" 
    $base.men17.m add command  -command "historyman clear 1" -label "Keep one day" 
    $base.men17.m add command  -command "historyman clear 0" -label "Keep nothing" 
    button $base.but28  -command "historyman report" -highlightcolor "#000000"  -justify "left" -relief "groove" -text "Save logs" 
    ###################
    # SETTING GEOMETRY
    ###################
    grid columnconf $base 1 -weight 1
    grid rowconf $base 1 -weight 1
    grid $base.cpd27  -in $base -column "1" -row "0" -columnspan "1" -rowspan "3"  -sticky "nesw" 
    grid columnconf $base.cpd27 0 -weight 1
    grid rowconf $base.cpd27 0 -weight 1
    grid $base.cpd27.01  -in $base.cpd27 -column "0" -row "1" -columnspan "1" -rowspan "1"  -sticky "ew" 
    grid $base.cpd27.02  -in $base.cpd27 -column "1" -row "0" -columnspan "1" -rowspan "1"  -sticky "ns" 
    grid $base.cpd27.03  -in $base.cpd27 -column "0" -row "0" -columnspan "1" -rowspan "1"  -sticky "nesw" 
    grid $base.fra17  -in $base -column "0" -row "0" -columnspan "1" -rowspan "1" -padx "2"  -pady "2" -sticky "new" 
    grid columnconf $base.fra17 0 -weight 1
    grid $base.fra17.rad25  -in $base.fra17 -column "0" -row "4" -columnspan "1" -rowspan "1"  -sticky "ew" 
    grid $base.fra17.rad26  -in $base.fra17 -column "0" -row "3" -columnspan "1" -rowspan "1"  -sticky "ew" 
    grid $base.fra17.rad28  -in $base.fra17 -column "0" -row "1" -columnspan "1" -rowspan "1"  -sticky "ew" 
    grid $base.fra17.rad29  -in $base.fra17 -column "0" -row "0" -columnspan "1" -rowspan "1"  -sticky "ew" 
    grid $base.fra17.rad37  -in $base.fra17 -column "0" -row "5" -columnspan "1" -rowspan "1"  -sticky "ew" 
    grid $base.but18  -in $base -column "0" -row "3" -columnspan "2" -rowspan "1" -padx "2"  -pady "2" -sticky "ew" 
    grid $base.men17  -in $base -column "0" -row "2" -columnspan "1" -rowspan "1"  -sticky "esw" 
    grid $base.but28  -in $base -column "0" -row "1" -columnspan "1" -rowspan "1"  -sticky "esw"
}

proc vTclWindow.top27 {base {container 0}} {
    if {$base == ""} {
        set base .top27
    }
    if {[winfo exists $base] && (!$container)} {
        wm deiconify $base; return
    }
    ###################
    # CREATING WIDGETS
    ###################
    if {!$container} {
    toplevel $base -class Toplevel  -highlightcolor "#000000" 
    wm focusmodel $base passive
    wm geometry $base 115x59
    wm maxsize $base 1265 994
    wm minsize $base 115 1
    wm overrideredirect $base 0
    wm resizable $base 1 1
    wm title $base "secpanel - Satellite"
    }
    frame $base.fra26  -height "75" -highlightcolor "#000000" -width "125" 
    menubutton $base.fra26.01  -anchor "w" -highlightcolor "#000000" -indicatoron "1"  -menu "$base.fra26.01.02" -padx "4" -pady "3" -relief "groove"  -text "Connections" 
    button $base.but27  -command "changetab big" -highlightcolor "#000000" -relief "groove" 
    ###################
    # SETTING GEOMETRY
    ###################
    grid columnconf $base 0 -weight 1
    grid rowconf $base 2 -weight 1
    grid $base.fra26  -in $base -column "0" -row "0" -columnspan "2" -rowspan "1"  -sticky "ew" 
    grid columnconf $base.fra26 0 -weight 1
    grid $base.fra26.01  -in $base.fra26 -column "0" -row "1" -columnspan "1" -rowspan "1"  -sticky "ew" 
    grid $base.but27  -in $base -column "0" -row "1" -columnspan "1" -rowspan "1"  -sticky "w"
}

proc vTclWindow.top32 {base {container 0}} {
    if {$base == ""} {
        set base .top32
    }
    if {[winfo exists $base] && (!$container)} {
        wm deiconify $base; return
    }
    ###################
    # CREATING WIDGETS
    ###################
    if {!$container} {
    toplevel $base -class Toplevel  -borderwidth "4" -highlightcolor "#000000" 
    wm focusmodel $base passive
    wm geometry $base 266x258
    wm maxsize $base 1009 738
    wm minsize $base 115 1
    wm overrideredirect $base 0
    wm resizable $base 1 1
    wm title $base "secpanel - GUI Colors"
    }
    frame $base.fra33  -borderwidth "1" -height "75" -highlightcolor "#000000" -width "125" 
    button $base.fra33.but37  -anchor "w" -command "colorman fore" -height "1"  -highlightcolor "#000000" -padx "9" -pady "3" -relief "groove"  -width "2" 
    button $base.fra33.but38  -anchor "w" -command "colorman back" -highlightcolor "#000000"  -padx "9" -pady "3" -relief "groove" 
    button $base.fra33.but39  -anchor "w" -command "colorman entfore" -highlightcolor "#000000"  -padx "9" -pady "3" -relief "groove" 
    button $base.fra33.but40  -anchor "w" -command "colorman entback" -highlightcolor "#000000"  -padx "9" -pady "3" -relief "groove" 
    button $base.fra33.but41  -anchor "w" -command "colorman listfore" -highlightcolor "#000000"  -padx "9" -pady "3" -relief "groove" 
    button $base.fra33.but42  -anchor "w" -command "colorman listback" -highlightcolor "#000000"  -padx "9" -pady "3" -relief "groove" 
    label $base.fra33.lab17  -anchor "w" -borderwidth "1" -highlightcolor "#000000"  -text "Foreground:" 
    label $base.fra33.lab18  -anchor "w" -borderwidth "1" -highlightcolor "#000000"  -text "Background:" 
    label $base.fra33.lab19  -anchor "w" -borderwidth "1" -highlightcolor "#000000"  -text "Entries - Fore:" 
    label $base.fra33.lab20  -anchor "w" -borderwidth "1" -highlightcolor "#000000"  -text "Entries - Back:" 
    label $base.fra33.lab21  -anchor "w" -borderwidth "1" -highlightcolor "#000000"  -text "Lists - Fore:" 
    label $base.fra33.lab22  -anchor "w" -borderwidth "1" -highlightcolor "#000000"  -text "Lists - Back:" 
    checkbutton $base.fra33.che17  -anchor "e" -borderwidth "1" -highlightcolor "#000000"  -text "Default" -variable "foredef" 
    checkbutton $base.fra33.che23  -anchor "e" -borderwidth "1" -highlightcolor "#000000"  -text "Default" -variable "backdef" 
    checkbutton $base.fra33.che24  -anchor "e" -borderwidth "1" -highlightcolor "#000000"  -text "Default" -variable "entforedef" 
    checkbutton $base.fra33.che25  -anchor "e" -borderwidth "1" -highlightcolor "#000000"  -text "Default" -variable "entbackdef" 
    checkbutton $base.fra33.che26  -anchor "e" -borderwidth "1" -highlightcolor "#000000"  -text "Default" -variable "listforedef" 
    checkbutton $base.fra33.che27  -anchor "e" -borderwidth "1" -highlightcolor "#000000"  -text "Default" -variable "listbackdef" 
    frame $base.fra34  -borderwidth "2" -relief "groove" -height "75"  -highlightcolor "#000000" -width "125" 
    button $base.fra34.but35  -command "Window destroy .top32" -highlightcolor "#000000" -padx "9"  -pady "3" -relief "groove" -text "Cancel" 
    button $base.fra34.but36  -command "colorman save" -highlightcolor "#000000" -padx "9"  -pady "3" -relief "groove" -text "Save" 
    label $base.lab18  -anchor "w" -borderwidth "1" -highlightcolor "#000000"  -text "Define colors or use system defaults" 
    ###################
    # SETTING GEOMETRY
    ###################
    grid columnconf $base 0 -weight 1
    grid rowconf $base 0 -weight 1
    grid $base.fra33  -in $base -column "0" -row "1" -columnspan "1" -rowspan "1"  -ipadx "2" -ipady "2" -padx "2" -pady "2" -sticky "nesw" 
    grid columnconf $base.fra33 1 -weight 1
    grid $base.fra33.but37  -in $base.fra33 -column "1" -row "0" -columnspan "1" -rowspan "1"  -padx "2" -sticky "ew" 
    grid $base.fra33.but38  -in $base.fra33 -column "1" -row "1" -columnspan "1" -rowspan "1"  -padx "2" -sticky "ew" 
    grid $base.fra33.but39  -in $base.fra33 -column "1" -row "2" -columnspan "1" -rowspan "1"  -padx "2" -sticky "ew" 
    grid $base.fra33.but40  -in $base.fra33 -column "1" -row "3" -columnspan "1" -rowspan "1"  -padx "2" -sticky "ew" 
    grid $base.fra33.but41  -in $base.fra33 -column "1" -row "4" -columnspan "1" -rowspan "1"  -padx "2" -sticky "ew" 
    grid $base.fra33.but42  -in $base.fra33 -column "1" -row "5" -columnspan "1" -rowspan "1"  -padx "2" -sticky "ew" 
    grid $base.fra33.lab17  -in $base.fra33 -column "0" -row "0" -columnspan "1" -rowspan "1"  -sticky "ew" 
    grid $base.fra33.lab18  -in $base.fra33 -column "0" -row "1" -columnspan "1" -rowspan "1"  -sticky "ew" 
    grid $base.fra33.lab19  -in $base.fra33 -column "0" -row "2" -columnspan "1" -rowspan "1"  -sticky "ew" 
    grid $base.fra33.lab20  -in $base.fra33 -column "0" -row "3" -columnspan "1" -rowspan "1"  -sticky "ew" 
    grid $base.fra33.lab21  -in $base.fra33 -column "0" -row "4" -columnspan "1" -rowspan "1"  -sticky "ew" 
    grid $base.fra33.lab22  -in $base.fra33 -column "0" -row "5" -columnspan "1" -rowspan "1"  -sticky "ew" 
    grid $base.fra33.che17  -in $base.fra33 -column "2" -row "0" -columnspan "1" -rowspan "1"  -padx "2" -sticky "ew" 
    grid $base.fra33.che23  -in $base.fra33 -column "2" -row "1" -columnspan "1" -rowspan "1"  -padx "2" -sticky "ew" 
    grid $base.fra33.che24  -in $base.fra33 -column "2" -row "2" -columnspan "1" -rowspan "1"  -padx "2" -sticky "ew" 
    grid $base.fra33.che25  -in $base.fra33 -column "2" -row "3" -columnspan "1" -rowspan "1"  -padx "2" -sticky "ew" 
    grid $base.fra33.che26  -in $base.fra33 -column "2" -row "4" -columnspan "1" -rowspan "1"  -padx "2" -sticky "ew" 
    grid $base.fra33.che27  -in $base.fra33 -column "2" -row "5" -columnspan "1" -rowspan "1"  -padx "2" -sticky "ew" 
    grid $base.fra34  -in $base -column "0" -row "2" -columnspan "1" -rowspan "1"  -ipadx "2" -ipady "2" -padx "2" -pady "2" -sticky "ew" 
    grid $base.fra34.but35  -in $base.fra34 -column "0" -row "0" -columnspan "1" -rowspan "1" 
    grid $base.fra34.but36  -in $base.fra34 -column "1" -row "0" -columnspan "1" -rowspan "1" 
    grid $base.lab18  -in $base -column "0" -row "0" -columnspan "1" -rowspan "1"  -sticky "ew"
}

proc vTclWindow.top33 {base {container 0}} {
    if {$base == ""} {
        set base .top33
    }
    if {[winfo exists $base] && (!$container)} {
        wm deiconify $base; return
    }
    ###################
    # CREATING WIDGETS
    ###################
    if {!$container} {
    toplevel $base -class Toplevel  -borderwidth "4" -highlightcolor "#000000" 
    wm focusmodel $base passive
    wm geometry $base 624x219
    wm maxsize $base 1265 994
    wm minsize $base 115 1
    wm overrideredirect $base 0
    wm resizable $base 1 1
    wm title $base "secpanel - Agent info"
    }
    frame $base.fra34  -borderwidth "2" -relief "groove" -height "75"  -highlightcolor "#000000" -width "125" 
    label $base.fra34.lab36  -anchor "e" -borderwidth "0" -highlightcolor "#000000"  -relief "raised" -text "SSH_AUTH_SOCK:" 
    label $base.fra34.lab37  -anchor "e" -borderwidth "0" -highlightcolor "#000000"  -relief "raised" -text "SSH_AGENT_PID:" 
    label $base.fra34.lab38  -anchor "w" -borderwidth "0" -highlightcolor "#000000" 
    label $base.fra34.lab39  -anchor "w" -borderwidth "0" -highlightcolor "#000000"  -relief "raised" 
    button $base.but40  -command "destroyfilter .top33" -highlightcolor "#000000"  -relief "groove" -text "Close" 
    frame $base.cpd36  -relief "raised" -height "30" -highlightcolor "#000000" -width "30" 
    scrollbar $base.cpd36.01  -borderwidth "1" -command "$base.cpd36.03 xview" -cursor "left_ptr"  -highlightcolor "#000000" -orient "horizontal" -troughcolor "#e2e2de"  -width "10" 
    scrollbar $base.cpd36.02  -borderwidth "1" -command "$base.cpd36.03 yview" -cursor "left_ptr"  -highlightcolor "#000000" -troughcolor "#e2e2de" -width "10" 
    text $base.cpd36.03  -borderwidth "1"  -height "1"  -highlightcolor "#000000" -width "8"  -xscrollcommand "$base.cpd36.01 set"  -yscrollcommand "$base.cpd36.02 set" 
    ###################
    # SETTING GEOMETRY
    ###################
    grid columnconf $base 0 -weight 1
    grid rowconf $base 1 -weight 1
    grid $base.fra34  -in $base -column "0" -row "0" -columnspan "1" -rowspan "1"  -ipadx "2" -ipady "2" -padx "2" -pady "2" -sticky "nesw" 
    grid columnconf $base.fra34 1 -weight 1
    grid $base.fra34.lab36  -in $base.fra34 -column "0" -row "0" -columnspan "1" -rowspan "1"  -padx "4" -sticky "nesw" 
    grid $base.fra34.lab37  -in $base.fra34 -column "0" -row "1" -columnspan "1" -rowspan "1"  -padx "4" -sticky "nesw" 
    grid $base.fra34.lab38  -in $base.fra34 -column "1" -row "0" -columnspan "1" -rowspan "1"  -padx "2" -sticky "nesw" 
    grid $base.fra34.lab39  -in $base.fra34 -column "1" -row "1" -columnspan "1" -rowspan "1"  -padx "2" -sticky "nesw" 
    grid $base.but40  -in $base -column "0" -row "2" -columnspan "1" -rowspan "1" -pady "2"  -sticky "ew" 
    grid $base.cpd36  -in $base -column "0" -row "1" -columnspan "1" -rowspan "1" -padx "2"  -pady "2" -sticky "nesw" 
    grid columnconf $base.cpd36 0 -weight 1
    grid rowconf $base.cpd36 0 -weight 1
    grid $base.cpd36.01  -in $base.cpd36 -column "0" -row "1" -columnspan "1" -rowspan "1"  -sticky "ew" 
    grid $base.cpd36.02  -in $base.cpd36 -column "1" -row "0" -columnspan "1" -rowspan "1"  -sticky "ns" 
    grid $base.cpd36.03  -in $base.cpd36 -column "0" -row "0" -columnspan "1" -rowspan "1"  -sticky "nesw"
}

proc vTclWindow.top40 {base {container 0}} {
    if {$base == ""} {
        set base .top40
    }
    if {[winfo exists $base] && (!$container)} {
        wm deiconify $base; return
    }
    ###################
    # CREATING WIDGETS
    ###################
    if {!$container} {
    toplevel $base -class Toplevel  -borderwidth "4" -menu "$base.m52" 
    wm focusmodel $base passive
    wm geometry $base 589x379
    wm maxsize $base 1284 1002
    wm minsize $base 115 1
    wm overrideredirect $base 0
    wm resizable $base 1 1
    wm title $base "secpanel - Remote Account Manager"
    }
    frame $base.fra41  -relief "groove" -height "75" -width "125" 
    frame $base.fra41.cpd50  -relief "raised" -height "30" -width "30" 
    scrollbar $base.fra41.cpd50.01  -borderwidth "1" -command "$base.fra41.cpd50.03 xview"  -orient "horizontal" -width "10" 
    scrollbar $base.fra41.cpd50.02  -borderwidth "1" -command "$base.fra41.cpd50.03 yview" -width "10" 
    text $base.fra41.cpd50.03  -borderwidth "1" -height "1" -width "8"  -xscrollcommand "$base.fra41.cpd50.01 set"  -yscrollcommand "$base.fra41.cpd50.02 set" 
    frame $base.fra42  -relief "groove" -height "75" -width "125" 
    frame $base.fra42.cpd49  -relief "raised" -height "30" -width "30" 
    scrollbar $base.fra42.cpd49.01  -borderwidth "1" -command "$base.fra42.cpd49.03 xview"  -orient "horizontal" -width "10" 
    scrollbar $base.fra42.cpd49.02  -borderwidth "1" -command "$base.fra42.cpd49.03 yview" -width "10" 
    text $base.fra42.cpd49.03  -borderwidth "1" -height "1" -width "8"  -xscrollcommand "$base.fra42.cpd49.01 set"  -yscrollcommand "$base.fra42.cpd49.02 set" 
    frame $base.fra43  -borderwidth "2" -relief "groove" -height "75" -width "125" 
    button $base.fra43.but47  -command "manage_remote cancel" -relief "groove" -text "Do nothing" 
    button $base.fra43.but48  -command "manage_remote write" -relief "groove"  -text "Write settings to remote account" 
    label $base.lab51  -anchor "w" -borderwidth "0" -font "helvetica 12 bold"  -text "Remote Account Manager for..." 
    menu $base.m52  -activeborderwidth "1" -borderwidth "1" -cursor "" 
    label $base.lab53  -anchor "w" -borderwidth "0" -relief "raised"  -text "Remote ~/.ssh/authorized_keys file" 
    frame $base.fra54  -relief "groove" -height "75" -width "125" 
    frame $base.fra54.cpd57  -relief "raised" -height "30" -width "30" 
    scrollbar $base.fra54.cpd57.01  -borderwidth "1" -command "$base.fra54.cpd57.03 xview"  -orient "horizontal" -width "10" 
    scrollbar $base.fra54.cpd57.02  -borderwidth "1" -command "$base.fra54.cpd57.03 yview" -width "10" 
    text $base.fra54.cpd57.03  -borderwidth "1" -height "1" -width "6" -wrap "none"  -xscrollcommand "$base.fra54.cpd57.01 set"  -yscrollcommand "$base.fra54.cpd57.02 set" 
    label $base.lab55  -anchor "w" -borderwidth "0" -relief "raised"  -text "Remote ~/.shosts file" 
    label $base.lab56  -anchor "w" -borderwidth "0" -relief "raised"  -text "Remote ~/.ssh/rc file" 
    ###################
    # SETTING GEOMETRY
    ###################
    grid columnconf $base 0 -weight 1
    grid rowconf $base 2 -weight 1
    grid rowconf $base 4 -weight 1
    grid rowconf $base 6 -weight 1
    grid $base.fra41  -in $base -column "0" -row "4" -columnspan "1" -rowspan "1"  -sticky "nesw" 
    grid columnconf $base.fra41 0 -weight 1
    grid rowconf $base.fra41 0 -weight 1
    grid $base.fra41.cpd50  -in $base.fra41 -column "0" -row "0" -columnspan "1" -rowspan "1"  -sticky "nesw" 
    grid columnconf $base.fra41.cpd50 0 -weight 1
    grid rowconf $base.fra41.cpd50 0 -weight 1
    grid $base.fra41.cpd50.01  -in $base.fra41.cpd50 -column "0" -row "1" -columnspan "1"  -rowspan "1" -sticky "ew" 
    grid $base.fra41.cpd50.02  -in $base.fra41.cpd50 -column "1" -row "0" -columnspan "1"  -rowspan "1" -sticky "ns" 
    grid $base.fra41.cpd50.03  -in $base.fra41.cpd50 -column "0" -row "0" -columnspan "1"  -rowspan "1" -sticky "nesw" 
    grid $base.fra42  -in $base -column "0" -row "6" -columnspan "1" -rowspan "1"  -sticky "nesw" 
    grid columnconf $base.fra42 0 -weight 1
    grid rowconf $base.fra42 0 -weight 1
    grid $base.fra42.cpd49  -in $base.fra42 -column "0" -row "0" -columnspan "1" -rowspan "1"  -sticky "nesw" 
    grid columnconf $base.fra42.cpd49 0 -weight 1
    grid rowconf $base.fra42.cpd49 0 -weight 1
    grid $base.fra42.cpd49.01  -in $base.fra42.cpd49 -column "0" -row "1" -columnspan "1"  -rowspan "1" -sticky "ew" 
    grid $base.fra42.cpd49.02  -in $base.fra42.cpd49 -column "1" -row "0" -columnspan "1"  -rowspan "1" -sticky "ns" 
    grid $base.fra42.cpd49.03  -in $base.fra42.cpd49 -column "0" -row "0" -columnspan "1"  -rowspan "1" -sticky "nesw" 
    grid $base.fra43  -in $base -column "0" -row "8" -columnspan "1" -rowspan "1"  -ipadx "2" -ipady "2" -sticky "ew" 
    grid $base.fra43.but47  -in $base.fra43 -column "0" -row "0" -columnspan "1" -rowspan "1" 
    grid $base.fra43.but48  -in $base.fra43 -column "1" -row "0" -columnspan "1" -rowspan "1" 
    grid $base.lab51  -in $base -column "0" -row "0" -columnspan "1" -rowspan "1"  -sticky "ew" 
    grid $base.lab53  -in $base -column "0" -row "1" -columnspan "1" -rowspan "1" -pady "2"  -sticky "ew" 
    grid $base.fra54  -in $base -column "0" -row "2" -columnspan "1" -rowspan "1"  -sticky "nesw" 
    grid columnconf $base.fra54 0 -weight 1
    grid rowconf $base.fra54 0 -weight 1
    grid $base.fra54.cpd57  -in $base.fra54 -column "0" -row "0" -columnspan "1" -rowspan "1"  -sticky "nesw" 
    grid columnconf $base.fra54.cpd57 0 -weight 1
    grid rowconf $base.fra54.cpd57 0 -weight 1
    grid $base.fra54.cpd57.01  -in $base.fra54.cpd57 -column "0" -row "1" -columnspan "1"  -rowspan "1" -sticky "ew" 
    grid $base.fra54.cpd57.02  -in $base.fra54.cpd57 -column "1" -row "0" -columnspan "1"  -rowspan "1" -sticky "ns" 
    grid $base.fra54.cpd57.03  -in $base.fra54.cpd57 -column "0" -row "0" -columnspan "1"  -rowspan "1" -sticky "nesw" 
    grid $base.lab55  -in $base -column "0" -row "3" -columnspan "1" -rowspan "1" -padx "2"  -pady "2" -sticky "ew" 
    grid $base.lab56  -in $base -column "0" -row "5" -columnspan "1" -rowspan "1" -pady "2"  -sticky "ew"
}

proc vTclWindow.top43 {base {container 0}} {
    if {$base == ""} {
        set base .top43
    }
    if {[winfo exists $base] && (!$container)} {
        wm deiconify $base; return
    }
    ###################
    # CREATING WIDGETS
    ###################
    if {!$container} {
    toplevel $base -class Toplevel  -borderwidth "4" -highlightcolor "#000000" 
    wm focusmodel $base passive
    wm geometry $base 243x350
    wm maxsize $base 1009 738
    wm minsize $base 115 1
    wm overrideredirect $base 0
    wm resizable $base 1 1
    wm title $base "secpanel - Local forwardings"
    }
    frame $base.cpd44  -borderwidth "1" -highlightcolor "#000000" -width "60" 
    listbox $base.cpd44.01  -borderwidth "1" -height "5" -highlightcolor "#000000"  -xscrollcommand "$base.cpd44.02 set"  -yscrollcommand "$base.cpd44.03 set" 
    bind $base.cpd44.01 <Button-1> {
        .top43.cpd44.01 activate @%x,%y
        showcomm l
    }
    scrollbar $base.cpd44.02  -borderwidth "1" -command "$base.cpd44.01 xview" -cursor "left_ptr"  -highlightcolor "#000000" -orient "horizontal" -troughcolor "#e2e2de"  -width "10" 
    scrollbar $base.cpd44.03  -borderwidth "1" -command "$base.cpd44.01 yview" -cursor "left_ptr"  -highlightcolor "#000000" -troughcolor "#e2e2de" -width "10" 
    frame $base.fra45  -borderwidth "2" -relief "groove" -height "75"  -highlightcolor "#000000" -width "125" 
    entry $base.fra45.ent46  -borderwidth "1" -highlightcolor "#000000" -width "10" 
    label $base.fra45.lab47  -borderwidth "1" -highlightcolor "#000000" -relief "raised" -text ">" 
    entry $base.fra45.ent48  -borderwidth "1" -highlightcolor "#000000" -width "10" 
    bind $base.fra45.ent48 <Key-Return> {
        add_forw l
    }
    menubutton $base.fra45.men49  -highlightcolor "#000000" -indicatoron "1"  -menu "$base.fra45.men49.m" -padx "4" -pady "3" -relief "groove"  -text "Ports" 
    menu $base.fra45.men49.m  -activeborderwidth "1" -borderwidth "1" -cursor "" -tearoff "0" 
    $base.fra45.men49.m add command  -command "insprot 110 l" -label "pop3" -state "active" 
    $base.fra45.men49.m add command  -command "insprot 143 l" -label "imap" 
    $base.fra45.men49.m add command  -command "insprot 80 l" -label "http" 
    $base.fra45.men49.m add command  -command "insprot 25 l" -label "smtp" 
    $base.fra45.men49.m add command  -command "insprot 23 l" -label "telnet" 
    $base.fra45.men49.m add command  -command "insprot 5901 l" -label "vnc" 
    button $base.fra45.but21  -command "add_forw l" -highlightcolor "#000000" -padx "9" -pady "3"  -relief "groove" -text "Add" 
    button $base.fra45.but17  -command "del_forw l" -highlightcolor "#000000" -padx "9" -pady "3"  -relief "groove" -text "Delete" 
    entry $base.fra45.ent17  -borderwidth "1" -highlightcolor "#000000" 
    label $base.fra45.lab17  -anchor "w" -borderwidth "1" -highlightcolor "#000000"  -text "Host - leave empty for target host:" 
    label $base.fra45.lab33  -anchor "w" -borderwidth "1" -highlightcolor "#000000"  -text "Comment:" 
    entry $base.fra45.ent34  -borderwidth "1" -highlightcolor "#000000" 
    label $base.fra45.lab35  -anchor "w" -borderwidth "1" -highlightcolor "#000000" -text "Ports:" 
    button $base.but22  -command "Window destroy .top43" -highlightcolor "#000000" -padx "9"  -pady "3" -relief "groove" -text "Cancel" 
    button $base.but17  -command "save_forwards l" -highlightcolor "#000000" -padx "9"  -pady "3" -relief "groove" -text "OK" 
    label $base.lab25  -anchor "w" -highlightcolor "#000000" 
    label $base.lab26  -anchor "w" -borderwidth "1" -highlightcolor "#000000"  -text "Local forwardings" 
    ###################
    # SETTING GEOMETRY
    ###################
    grid columnconf $base 0 -weight 1
    grid columnconf $base 1 -weight 1
    grid rowconf $base 1 -weight 1
    grid $base.cpd44  -in $base -column "0" -row "1" -columnspan "2" -rowspan "1" -padx "2"  -pady "2" -sticky "nesw" 
    grid columnconf $base.cpd44 0 -weight 1
    grid rowconf $base.cpd44 0 -weight 1
    grid $base.cpd44.01  -in $base.cpd44 -column "0" -row "0" -columnspan "1" -rowspan "1"  -sticky "nesw" 
    grid $base.cpd44.02  -in $base.cpd44 -column "0" -row "1" -columnspan "1" -rowspan "1"  -sticky "ew" 
    grid $base.cpd44.03  -in $base.cpd44 -column "1" -row "0" -columnspan "1" -rowspan "1"  -sticky "ns" 
    grid $base.fra45  -in $base -column "0" -row "3" -columnspan "2" -rowspan "1"  -ipadx "4" -ipady "4" -padx "4" -pady "4" -sticky "ew" 
    grid columnconf $base.fra45 0 -weight 1
    grid columnconf $base.fra45 2 -weight 1
    grid $base.fra45.ent46  -in $base.fra45 -column "0" -row "3" -columnspan "1" -rowspan "1"  -padx "4" -sticky "ew" 
    grid $base.fra45.lab47  -in $base.fra45 -column "1" -row "3" -columnspan "1" -rowspan "1" 
    grid $base.fra45.ent48  -in $base.fra45 -column "2" -row "3" -columnspan "1" -rowspan "1"  -padx "4" -sticky "ew" 
    grid $base.fra45.men49  -in $base.fra45 -column "2" -row "6" -columnspan "1" -rowspan "1" 
    grid $base.fra45.but21  -in $base.fra45 -column "1" -row "6" -columnspan "1" -rowspan "1" 
    grid $base.fra45.but17  -in $base.fra45 -column "0" -row "6" -columnspan "1" -rowspan "1" 
    grid $base.fra45.ent17  -in $base.fra45 -column "0" -row "1" -columnspan "3" -rowspan "1"  -padx "4" -sticky "ew" 
    grid $base.fra45.lab17  -in $base.fra45 -column "0" -row "0" -columnspan "3" -rowspan "1"  -sticky "ew" 
    grid $base.fra45.lab33  -in $base.fra45 -column "0" -row "4" -columnspan "3" -rowspan "1"  -sticky "ew" 
    grid $base.fra45.ent34  -in $base.fra45 -column "0" -row "5" -columnspan "3" -rowspan "1"  -padx "4" -sticky "ew" 
    grid $base.fra45.lab35  -in $base.fra45 -column "0" -row "2" -columnspan "3" -rowspan "1"  -sticky "ew" 
    grid $base.but22  -in $base -column "0" -row "4" -columnspan "1" -rowspan "1" 
    grid $base.but17  -in $base -column "1" -row "4" -columnspan "1" -rowspan "1" 
    grid $base.lab25  -in $base -column "0" -row "2" -columnspan "2" -rowspan "1" -padx "2"  -sticky "ew" 
    grid $base.lab26  -in $base -column "0" -row "0" -columnspan "2" -rowspan "1" -padx "2"  -pady "2" -sticky "new"
}

proc vTclWindow.top50 {base {container 0}} {
    if {$base == ""} {
        set base .top50
    }
    if {[winfo exists $base] && (!$container)} {
        wm deiconify $base; return
    }
    ###################
    # CREATING WIDGETS
    ###################
    if {!$container} {
    toplevel $base -class Toplevel  -borderwidth "4" -highlightcolor "#000000" 
    wm focusmodel $base passive
    wm geometry $base 315x236
    wm maxsize $base 1009 738
    wm minsize $base 115 1
    wm overrideredirect $base 0
    wm resizable $base 1 1
    wm title $base "secpanel - Hostkeys"
    }
    frame $base.fra51  -borderwidth "1" -height "75" -highlightcolor "#000000" -width "125" 
    frame $base.fra51.fra53  -height "75" -highlightcolor "#000000" -width "125" 
    button $base.fra51.fra53.but56  -command "hostkey view" -highlightcolor "#000000" -padx "9" -pady "3"  -relief "groove" -state "disabled" -text "View" 
    button $base.fra51.fra53.but57  -command "hostkey delete" -highlightcolor "#000000" -padx "9"  -pady "3" -relief "groove" -text "Delete" 
    button $base.fra51.fra53.but18  -command "hostkey export" -highlightcolor "#000000" -padx "9"  -pady "3" -relief "groove" -state "disabled" -text "Export" 
    frame $base.fra51.cpd54  -relief "raised" -height "30" -highlightcolor "#000000" -width "30" 
    listbox $base.fra51.cpd54.01  -borderwidth "1" -highlightcolor "#000000"  -xscrollcommand "$base.fra51.cpd54.02 set"  -yscrollcommand "$base.fra51.cpd54.03 set" 
    scrollbar $base.fra51.cpd54.02  -borderwidth "1" -command "$base.fra51.cpd54.01 xview"  -cursor "left_ptr" -highlightcolor "#000000" -orient "horizontal"  -troughcolor "#e2e2de" -width "10" 
    scrollbar $base.fra51.cpd54.03  -borderwidth "1" -command "$base.fra51.cpd54.01 yview"  -cursor "left_ptr" -highlightcolor "#000000" -troughcolor "#e2e2de"  -width "10" 
    label $base.fra51.lab17  -anchor "w" -borderwidth "1" -highlightcolor "#000000"  -text "Collected hostkeys, keytype assumed:" 
    label $base.fra51.lab177  -anchor "w" -borderwidth "1" -highlightcolor "#000000"  -textvariable "hostkeytype" 
    button $base.but38  -command "Window destroy .top50" -highlightcolor "#000000" -padx "9"  -pady "3" -relief "groove" -text "Close" 
    ###################
    # SETTING GEOMETRY
    ###################
    grid columnconf $base 0 -weight 1
    grid rowconf $base 0 -weight 1
    grid $base.fra51  -in $base -column "0" -row "0" -columnspan "1" -rowspan "1"  -ipadx "2" -ipady "2" -padx "2" -pady "2" -sticky "nesw" 
    grid columnconf $base.fra51 0 -weight 1
    grid rowconf $base.fra51 2 -weight 1
    grid $base.fra51.fra53  -in $base.fra51 -column "1" -row "2" -columnspan "1" -rowspan "1"  -ipadx "2" -padx "2" -sticky "n" 
    grid $base.fra51.fra53.but56  -in $base.fra51.fra53 -column "0" -row "0" -columnspan "1"  -rowspan "1" -sticky "ew" 
    grid $base.fra51.fra53.but57  -in $base.fra51.fra53 -column "0" -row "2" -columnspan "1"  -rowspan "1" -sticky "ew" 
    grid $base.fra51.fra53.but18  -in $base.fra51.fra53 -column "0" -row "1" -columnspan "1"  -rowspan "1" 
    grid $base.fra51.cpd54  -in $base.fra51 -column "0" -row "2" -columnspan "1" -rowspan "1"  -sticky "nesw" 
    grid columnconf $base.fra51.cpd54 0 -weight 1
    grid rowconf $base.fra51.cpd54 0 -weight 1
    grid $base.fra51.cpd54.01  -in $base.fra51.cpd54 -column "0" -row "0" -columnspan "1"  -rowspan "1" -sticky "nesw" 
    grid $base.fra51.cpd54.02  -in $base.fra51.cpd54 -column "0" -row "1" -columnspan "1"  -rowspan "1" -sticky "ew" 
    grid $base.fra51.cpd54.03  -in $base.fra51.cpd54 -column "1" -row "0" -columnspan "1"  -rowspan "1" -sticky "ns" 
    grid $base.fra51.lab17  -in $base.fra51 -column "0" -row "0" -columnspan "2" -rowspan "1"  -sticky "ew" 
    grid $base.fra51.lab177  -in $base.fra51 -column "0" -row "1" -columnspan "2" -rowspan "1"  -sticky "ew" 
    grid $base.but38  -in $base -column "0" -row "1" -columnspan "1" -rowspan "1" -padx "2"  -pady "2" -sticky "ew"
}

proc vTclWindow.top51 {base {container 0}} {
    if {$base == ""} {
        set base .top51
    }
    if {[winfo exists $base] && (!$container)} {
        wm deiconify $base; return
    }
    ###################
    # CREATING WIDGETS
    ###################
    if {!$container} {
    toplevel $base -class Toplevel  -borderwidth "4" -menu "$base.m20" -highlightcolor "#000000" 
    wm focusmodel $base passive
    wm geometry $base 243x349
    wm maxsize $base 1009 738
    wm minsize $base 115 1
    wm overrideredirect $base 0
    wm resizable $base 1 1
    wm title $base "secpanel - Remote forwardings"
    }
    frame $base.cpd53  -borderwidth "1" -height "30" -highlightcolor "#000000" -width "30" 
    listbox $base.cpd53.01  -borderwidth "1" -height "5" -highlightcolor "#000000"  -xscrollcommand "$base.cpd53.02 set"  -yscrollcommand "$base.cpd53.03 set" 
    bind $base.cpd53.01 <Button-1> {
        .top51.cpd53.01 activate @%x,%y
        showcomm r
    }
    scrollbar $base.cpd53.02  -borderwidth "1" -command "$base.cpd53.01 xview" -cursor "left_ptr"  -highlightcolor "#000000" -orient "horizontal" -troughcolor "#e2e2de"  -width "10" 
    scrollbar $base.cpd53.03  -borderwidth "1" -command "$base.cpd53.01 yview" -cursor "left_ptr"  -highlightcolor "#000000" -troughcolor "#e2e2de" -width "10" 
    button $base.but21  -command "Window destroy .top51" -highlightcolor "#000000" -padx "9"  -pady "3" -relief "groove" -text "Cancel" 
    button $base.but18  -command "save_forwards r" -highlightcolor "#000000" -padx "9"  -pady "3" -relief "groove" -text "OK" 
    label $base.lab24  -anchor "w" -highlightcolor "#000000" 
    frame $base.fra36  -borderwidth "2" -relief "groove" -height "75"  -highlightcolor "#000000" -width "125" 
    entry $base.fra36.01  -borderwidth "1" -highlightcolor "#000000" -width "10" 
    label $base.fra36.02  -borderwidth "1" -highlightcolor "#000000" -relief "raised" -text ">" 
    entry $base.fra36.03  -borderwidth "1" -highlightcolor "#000000" -width "10" 
    bind $base.fra36.03 <Key-Return> {
        add_forw r
    }
    menubutton $base.fra36.04  -highlightcolor "#000000" -indicatoron "1" -menu "$base.fra36.04.05"  -padx "4" -pady "3" -relief "groove" -text "Ports" 
    menu $base.fra36.04.05  -activeborderwidth "1" -borderwidth "1" -cursor "" -tearoff "0" 
    $base.fra36.04.05 add command  -command "insprot 110 r" -label "pop3" -state "active" 
    $base.fra36.04.05 add command  -command "insprot 143 r" -label "imap" 
    $base.fra36.04.05 add command  -command "insprot 80 r" -label "http" 
    $base.fra36.04.05 add command  -command "insprot 25 r" -label "smtp" 
    $base.fra36.04.05 add command  -command "insprot 23 r" -label "telnet" 
    $base.fra36.04.05 add command  -command "insprot 5901 r" -label "vnc" 
    button $base.fra36.06  -command "add_forw r" -highlightcolor "#000000" -padx "9" -pady "3"  -relief "groove" -text "Add" 
    button $base.fra36.07  -command "del_forw r" -highlightcolor "#000000" -padx "9" -pady "3"  -relief "groove" -text "Delete" 
    entry $base.fra36.08  -borderwidth "1" -highlightcolor "#000000" 
    label $base.fra36.09  -anchor "w" -borderwidth "1" -highlightcolor "#000000"  -text "Host - leave empty for local host:" 
    label $base.fra36.010  -anchor "w" -borderwidth "1" -highlightcolor "#000000"  -text "Comment:" 
    entry $base.fra36.011  -borderwidth "1" -highlightcolor "#000000" 
    label $base.fra36.012  -anchor "w" -borderwidth "1" -highlightcolor "#000000" -text "Ports:" 
    label $base.lab27  -anchor "w" -borderwidth "1" -highlightcolor "#000000"  -text "Remote forwardings" 
    ###################
    # SETTING GEOMETRY
    ###################
    grid columnconf $base 0 -weight 1
    grid columnconf $base 1 -weight 1
    grid rowconf $base 1 -weight 1
    grid $base.cpd53  -in $base -column "0" -row "1" -columnspan "2" -rowspan "1" -padx "2"  -pady "2" -sticky "nesw" 
    grid columnconf $base.cpd53 0 -weight 1
    grid rowconf $base.cpd53 0 -weight 1
    grid $base.cpd53.01  -in $base.cpd53 -column "0" -row "0" -columnspan "1" -rowspan "1"  -sticky "nesw" 
    grid $base.cpd53.02  -in $base.cpd53 -column "0" -row "1" -columnspan "1" -rowspan "1"  -sticky "ew" 
    grid $base.cpd53.03  -in $base.cpd53 -column "1" -row "0" -columnspan "1" -rowspan "1"  -sticky "ns" 
    grid $base.but21  -in $base -column "0" -row "4" -columnspan "1" -rowspan "1" 
    grid $base.but18  -in $base -column "1" -row "4" -columnspan "1" -rowspan "1" 
    grid $base.lab24  -in $base -column "0" -row "2" -columnspan "2" -rowspan "1" -padx "2"  -sticky "ew" 
    grid $base.fra36  -in $base -column "0" -row "3" -columnspan "2" -rowspan "1"  -ipadx "4" -ipady "4" -padx "4" -pady "4" -sticky "ew" 
    grid columnconf $base.fra36 0 -weight 1
    grid columnconf $base.fra36 2 -weight 1
    grid $base.fra36.01  -in $base.fra36 -column "0" -row "3" -columnspan "1" -rowspan "1"  -padx "4" -sticky "ew" 
    grid $base.fra36.02  -in $base.fra36 -column "1" -row "3" -columnspan "1" -rowspan "1" 
    grid $base.fra36.03  -in $base.fra36 -column "2" -row "3" -columnspan "1" -rowspan "1"  -padx "4" -sticky "ew" 
    grid $base.fra36.04  -in $base.fra36 -column "2" -row "6" -columnspan "1" -rowspan "1" 
    grid $base.fra36.06  -in $base.fra36 -column "1" -row "6" -columnspan "1" -rowspan "1" 
    grid $base.fra36.07  -in $base.fra36 -column "0" -row "6" -columnspan "1" -rowspan "1" 
    grid $base.fra36.08  -in $base.fra36 -column "0" -row "1" -columnspan "3" -rowspan "1"  -padx "4" -sticky "ew" 
    grid $base.fra36.09  -in $base.fra36 -column "0" -row "0" -columnspan "3" -rowspan "1"  -sticky "ew" 
    grid $base.fra36.010  -in $base.fra36 -column "0" -row "4" -columnspan "3" -rowspan "1"  -sticky "ew" 
    grid $base.fra36.011  -in $base.fra36 -column "0" -row "5" -columnspan "3" -rowspan "1"  -padx "4" -sticky "ew" 
    grid $base.fra36.012  -in $base.fra36 -column "0" -row "2" -columnspan "3" -rowspan "1"  -sticky "ew" 
    grid $base.lab27  -in $base -column "0" -row "0" -columnspan "2" -rowspan "1" -padx "2"  -pady "2" -sticky "new"
}

proc vTclWindow.top52 {base {container 0}} {
    if {$base == ""} {
        set base .top52
    }
    if {[winfo exists $base] && (!$container)} {
        wm deiconify $base; return
    }
    ###################
    # CREATING WIDGETS
    ###################
    if {!$container} {
    toplevel $base -class Toplevel  -borderwidth "4" -highlightcolor "#000000" 
    wm focusmodel $base passive
    wm geometry $base 353x420
    wm maxsize $base 1265 994
    wm minsize $base 115 1
    wm overrideredirect $base 0
    wm resizable $base 1 1
    wm title $base "secpanel - Key management"
    }
    frame $base.fra18  -height "75" -highlightcolor "#000000" -width "125" 
    label $base.fra18.lab23  -anchor "w" -borderwidth "1" -highlightcolor "#000000"  -text "Comment" 
    entry $base.fra18.ent24  -borderwidth "1" -highlightcolor "#000000" 
    label $base.fra18.lab25  -anchor "w" -borderwidth "1" -highlightcolor "#000000"  -text "Keyfile" 
    entry $base.fra18.ent26  -borderwidth "1" -highlightcolor "#000000" 
    button $base.fra18.but27  -command "keygen chpath" -height "1" -highlightcolor "#000000"  -padx "9" -pady "3" -relief "groove" 
    label $base.fra18.lab28  -anchor "w" -borderwidth "1" -highlightcolor "#000000"  -text "Passphrase" 
    entry $base.fra18.ent29  -borderwidth "1" -highlightcolor "#000000" -show "*" 
    checkbutton $base.fra18.che30  -borderwidth "1" -highlightcolor "#000000" -text "No passphrase"  -variable "nopass" 
    button $base.fra18.but54  -command "keygen gen" -highlightcolor "#000000" -padx "9" -pady "3"  -relief "groove" -text "Generate keypair" 
    entry $base.fra18.ent25  -borderwidth "1" -highlightcolor "#000000" -show "*" 
    label $base.fra18.lab26  -anchor "w" -borderwidth "1" -highlightcolor "#000000"  -text "Repeat pass." 
    checkbutton $base.fra18.che17  -anchor "w" -borderwidth "1" -highlightcolor "#000000"  -justify "left" -text "Enter passphrase
in textmode"  -variable "passintext" 
    menubutton $base.fra18.men30  -highlightcolor "#000000" -indicatoron "1"  -menu "$base.fra18.men30.m" -padx "4" -pady "3" -relief "groove"  -textvariable "keytype" 
    menu $base.fra18.men30.m  -activeborderwidth "1" -borderwidth "1" -cursor "" -tearoff "0" 
    $base.fra18.men30.m add radiobutton  -value "SSH2 RSA" -variable "keytype" -label "SSH2 RSA" 
    $base.fra18.men30.m add radiobutton  -value "SSH2 DSA" -variable "keytype" -label "SSH2 DSA" 
    $base.fra18.men30.m add radiobutton  -value "SSH1 RSA1" -variable "keytype" -label "SSH1 RSA1" 
    label $base.fra18.lab31  -anchor "w" -borderwidth "0" -highlightcolor "#000000"  -relief "raised" -text "Keytype" 
    label $base.lab32  -anchor "w" -borderwidth "1" -highlightcolor "#000000"  -text "Key-generation" 
    label $base.lab33  -anchor "w" -borderwidth "1" -highlightcolor "#000000"  -text "Editing, conversion and information" 
    frame $base.fra34  -height "75" -highlightcolor "#000000" -width "125" 
    frame $base.fra34.cpd17  -borderwidth "1" -height "30" -highlightcolor "#000000" -width "30" 
    listbox $base.fra34.cpd17.01  -borderwidth "1" -highlightcolor "#000000"  -xscrollcommand "$base.fra34.cpd17.02 set"  -yscrollcommand "$base.fra34.cpd17.03 set" 
    bind $base.fra34.cpd17.01 <Double-Button-1> {
        keygen info
    }
    scrollbar $base.fra34.cpd17.02  -borderwidth "1" -command "$base.fra34.cpd17.01 xview"  -cursor "left_ptr" -highlightcolor "#000000" -orient "horizontal"  -troughcolor "#e2e2de" -width "10" 
    scrollbar $base.fra34.cpd17.03  -borderwidth "1" -command "$base.fra34.cpd17.01 yview"  -cursor "left_ptr" -highlightcolor "#000000" -troughcolor "#e2e2de"  -width "10" 
    frame $base.fra34.fra20  -borderwidth "1" -height "75" -highlightcolor "#000000" -width "125" 
    button $base.fra34.fra20.but21  -command "keygen info" -highlightcolor "#000000" -padx "9" -pady "3"  -relief "groove" -text "Show information" 
    button $base.fra34.fra20.but22  -command "keygen 1" -highlightcolor "#000000" -padx "9" -pady "3"  -relief "groove" -text "Change passphrase" 
    checkbutton $base.fra34.fra20.che23  -anchor "w" -borderwidth "1" -highlightcolor "#000000"  -justify "left" -text "Change passphrase
in textmode"  -variable "pwtextmode" 
    button $base.fra34.fra20.but24  -command "keygen del" -highlightcolor "#000000" -padx "9" -pady "3"  -relief "groove" -text "Delete key" 
    menubutton $base.fra34.fra20.men33  -highlightcolor "#000000" -menu "$base.fra34.fra20.men33.m" -padx "4"  -pady "3" -relief "groove" -state "disabled"  -text "Convert key to..." 
    menu $base.fra34.fra20.men33.m  -activeborderwidth "1" -borderwidth "1" -cursor "" -tearoff "0" 
    $base.fra34.fra20.men33.m add command  -label "SECSH Public Key File Format" 
    $base.fra34.fra20.men33.m add command  -label "OpenSSH compatible" 
    $base.fra34.fra20.men33.m add command  -label "OpenSSH public key" 
    button $base.fra34.fra20.but17  -command "update_keylist" -highlightcolor "#000000" -relief "groove"  -text "Refresh list" 
    button $base.but37  -command "Window destroy .top52" -highlightcolor "#000000" -padx "9"  -pady "3" -relief "groove" -text "Close" 
    ###################
    # SETTING GEOMETRY
    ###################
    grid columnconf $base 0 -weight 1
    grid rowconf $base 3 -weight 1
    grid $base.fra18  -in $base -column "0" -row "1" -columnspan "2" -rowspan "1"  -ipadx "2" -ipady "2" -padx "2" -pady "2" -sticky "ew" 
    grid columnconf $base.fra18 1 -weight 1
    grid $base.fra18.lab23  -in $base.fra18 -column "0" -row "2" -columnspan "1" -rowspan "1"  -sticky "ew" 
    grid $base.fra18.ent24  -in $base.fra18 -column "1" -row "2" -columnspan "1" -rowspan "1"  -pady "2" -sticky "ew" 
    grid $base.fra18.lab25  -in $base.fra18 -column "0" -row "3" -columnspan "1" -rowspan "1"  -sticky "ew" 
    grid $base.fra18.ent26  -in $base.fra18 -column "1" -row "3" -columnspan "1" -rowspan "1"  -pady "2" -sticky "ew" 
    grid $base.fra18.but27  -in $base.fra18 -column "2" -row "3" -columnspan "1" -rowspan "1"  -sticky "nsw" 
    grid $base.fra18.lab28  -in $base.fra18 -column "0" -row "4" -columnspan "1" -rowspan "1"  -sticky "ew" 
    grid $base.fra18.ent29  -in $base.fra18 -column "1" -row "4" -columnspan "1" -rowspan "1"  -pady "2" -sticky "ew" 
    grid $base.fra18.che30  -in $base.fra18 -column "2" -row "4" -columnspan "1" -rowspan "1"  -sticky "w" 
    grid $base.fra18.but54  -in $base.fra18 -column "2" -row "6" -columnspan "2" -rowspan "1" 
    grid $base.fra18.ent25  -in $base.fra18 -column "1" -row "5" -columnspan "1" -rowspan "1"  -pady "2" -sticky "ew" 
    grid $base.fra18.lab26  -in $base.fra18 -column "0" -row "5" -columnspan "1" -rowspan "1"  -sticky "ew" 
    grid $base.fra18.che17  -in $base.fra18 -column "2" -row "5" -columnspan "1" -rowspan "1"  -sticky "ew" 
    grid $base.fra18.men30  -in $base.fra18 -column "1" -row "6" -columnspan "1" -rowspan "1"  -sticky "ew" 
    grid $base.fra18.lab31  -in $base.fra18 -column "0" -row "6" -columnspan "1" -rowspan "1"  -sticky "ew" 
    grid $base.lab32  -in $base -column "0" -row "0" -columnspan "2" -rowspan "1" -padx "2"  -pady "2" -sticky "ew" 
    grid $base.lab33  -in $base -column "0" -row "2" -columnspan "1" -rowspan "1" -padx "2"  -pady "2" -sticky "ew" 
    grid $base.fra34  -in $base -column "0" -row "3" -columnspan "1" -rowspan "1"  -ipadx "2" -ipady "2" -padx "2" -pady "2" -sticky "nesw" 
    grid columnconf $base.fra34 0 -weight 1
    grid rowconf $base.fra34 0 -weight 1
    grid rowconf $base.fra34 1 -weight 1
    grid $base.fra34.cpd17  -in $base.fra34 -column "0" -row "0" -columnspan "1" -rowspan "1"  -sticky "nesw" 
    grid columnconf $base.fra34.cpd17 0 -weight 1
    grid rowconf $base.fra34.cpd17 0 -weight 1
    grid $base.fra34.cpd17.01  -in $base.fra34.cpd17 -column "0" -row "0" -columnspan "1"  -rowspan "1" -sticky "nesw" 
    grid $base.fra34.cpd17.02  -in $base.fra34.cpd17 -column "0" -row "1" -columnspan "1"  -rowspan "1" -sticky "ew" 
    grid $base.fra34.cpd17.03  -in $base.fra34.cpd17 -column "1" -row "0" -columnspan "1"  -rowspan "1" -sticky "ns" 
    grid $base.fra34.fra20  -in $base.fra34 -column "1" -row "0" -columnspan "1" -rowspan "1"  -ipadx "2" -ipady "2" -padx "2" -pady "2" -sticky "n" 
    grid $base.fra34.fra20.but21  -in $base.fra34.fra20 -column "0" -row "0" -columnspan "1"  -rowspan "1" -sticky "ew" 
    grid $base.fra34.fra20.but22  -in $base.fra34.fra20 -column "0" -row "1" -columnspan "1"  -rowspan "1" -sticky "ew" 
    grid $base.fra34.fra20.che23  -in $base.fra34.fra20 -column "0" -row "2" -columnspan "2"  -rowspan "1" -sticky "ew" 
    grid $base.fra34.fra20.but24  -in $base.fra34.fra20 -column "0" -row "4" -columnspan "1"  -rowspan "1" -sticky "ew" 
    grid $base.fra34.fra20.men33  -in $base.fra34.fra20 -column "0" -row "3" -columnspan "1"  -rowspan "1" -sticky "ew" 
    grid $base.fra34.fra20.but17  -in $base.fra34.fra20 -column "0" -row "5" -columnspan "1"  -rowspan "1" -sticky "ew" 
    grid $base.but37  -in $base -column "0" -row "4" -columnspan "2" -rowspan "1" -padx "2"  -pady "2" -sticky "ew"
}

proc vTclWindow.top53 {base {container 0}} {
    if {$base == ""} {
        set base .top53
    }
    if {[winfo exists $base] && (!$container)} {
        wm deiconify $base; return
    }
    ###################
    # CREATING WIDGETS
    ###################
    if {!$container} {
    toplevel $base -class Toplevel  -borderwidth "4" -highlightcolor "#000000" 
    wm focusmodel $base passive
    wm geometry $base 436x294
    wm maxsize $base 1265 930
    wm minsize $base 115 1
    wm overrideredirect $base 0
    wm resizable $base 1 1
    wm title $base "secpanel - Key distribution"
    }
    frame $base.fra18  -height "75" -highlightcolor "#000000" -width "125" 
    label $base.fra18.lab21  -anchor "w" -borderwidth "0" -font "Helvetica 12 bold"  -highlightcolor "#000000" -text "secpanel key distribution" 
    frame $base.fra20  -height "75" -highlightcolor "#000000" -width "125" 
    frame $base.fra20.cpd22  -height "30" -highlightcolor "#000000" -width "30" 
    listbox $base.fra20.cpd22.01  -borderwidth "1" -highlightcolor "#000000"  -xscrollcommand "$base.fra20.cpd22.02 set"  -yscrollcommand "$base.fra20.cpd22.03 set" 
    bind $base.fra20.cpd22.01 <Button-1> {
        .top53.fra20.cpd22.01 activate @%x,%y
	.top53.fra20.fra26.ent28 delete 0 end
	if {[.top53.fra20.cpd22.01 get active] != ""} {
	    .top53.fra20.fra26.ent28 insert end "$sshdir/[.top53.fra20.cpd22.01 get active].pub"
	}
    }
    bind $base.fra20.cpd22.01 <Double-Button-1> {
        distwizard host f
    }
    scrollbar $base.fra20.cpd22.02  -borderwidth "1" -command "$base.fra20.cpd22.01 xview"  -cursor "left_ptr" -highlightcolor "#000000" -orient "horizontal"  -troughcolor "#e2e2de" -width "10" 
    scrollbar $base.fra20.cpd22.03  -borderwidth "1" -command "$base.fra20.cpd22.01 yview"  -cursor "left_ptr" -highlightcolor "#000000" -troughcolor "#e2e2de"  -width "10" 
    label $base.fra20.lab23  -borderwidth "0" -highlightcolor "#000000"  -text "Select which key to distribute" 
    frame $base.fra20.fra26  -height "75" -highlightcolor "#000000" -width "125" 
    button $base.fra20.fra26.but27  -command "seldistkey" -highlightcolor "#000000" -relief "groove"  -text "Browse" 
    entry $base.fra20.fra26.ent28  -borderwidth "1" -highlightcolor "#000000" 
    label $base.fra20.lab32  -anchor "w" -borderwidth "0" -highlightcolor "#000000"  -text "Available keys:" 
    frame $base.fra29  -height "75" -highlightcolor "#000000" -width "125" 
    button $base.fra29.but30  -command "Window destroy .top53" -highlightcolor "#000000"  -relief "groove" -text "Cancel" 
    button $base.fra29.but31  -command "distwizard host f" -highlightcolor "#000000"  -relief "groove" -state "active" -text "Continue" 
    button $base.fra29.but33  -highlightcolor "#000000" -relief "groove" -state "disabled"  -text "Back" 
    frame $base.fra34  -height "75" -highlightcolor "#000000" -width "125" 
    frame $base.fra34.cpd35  -borderwidth "1" -height "30" -highlightcolor "#000000" -width "30" 
    listbox $base.fra34.cpd35.01  -borderwidth "1" -height "0" -highlightcolor "#000000"  -selectmode "extended" -xscrollcommand "$base.fra34.cpd35.02 set"  -yscrollcommand "$base.fra34.cpd35.03 set" 
    bind $base.fra34.cpd35.01 <Double-Button-1> {
        distwizard confirm f
    }
    scrollbar $base.fra34.cpd35.02  -borderwidth "1" -command "$base.fra34.cpd35.01 xview"  -cursor "left_ptr" -highlightcolor "#000000" -orient "horizontal"  -troughcolor "#e2e2de" -width "10" 
    scrollbar $base.fra34.cpd35.03  -borderwidth "1" -command "$base.fra34.cpd35.01 yview"  -cursor "left_ptr" -highlightcolor "#000000" -troughcolor "#e2e2de"  -width "10" 
    label $base.fra34.lab36  -anchor "w" -borderwidth "0" -highlightcolor "#000000"  -justify "left" -text "Select connection to distribute the key to:" 
    frame $base.fra42  -height "75" -highlightcolor "#000000" -width "125" 
    message $base.fra42.mes44  -aspect "500" -padx "2" -pady "2" -text "message" 
    label $base.fra42.lab17  -bitmap "question" -borderwidth "0" -highlightcolor "#000000"  -text "label" 
    label $base.lab17  -anchor "w" -borderwidth "0" -highlightcolor "#000000" 
    ###################
    # SETTING GEOMETRY
    ###################
    grid columnconf $base 0 -weight 1
    grid rowconf $base 1 -weight 1
    grid $base.fra18  -in $base -column "0" -row "0" -columnspan "1" -rowspan "1"  -ipadx "2" -ipady "2" -padx "2" -pady "2" -sticky "ew" 
    grid columnconf $base.fra18 0 -weight 1
    grid $base.fra18.lab21  -in $base.fra18 -column "0" -row "1" -columnspan "1" -rowspan "1"  -sticky "ew" 
    grid $base.fra20  -in $base -column "0" -row "1" -columnspan "1" -rowspan "1"  -ipadx "2" -ipady "2" -padx "2" -pady "2" -sticky "nesw" 
    grid columnconf $base.fra20 1 -weight 1
    grid rowconf $base.fra20 1 -weight 1
    grid $base.fra20.cpd22  -in $base.fra20 -column "1" -row "1" -columnspan "1" -rowspan "2"  -sticky "nesw" 
    grid columnconf $base.fra20.cpd22 0 -weight 1
    grid rowconf $base.fra20.cpd22 0 -weight 1
    grid $base.fra20.cpd22.01  -in $base.fra20.cpd22 -column "0" -row "0" -columnspan "1"  -rowspan "1" -sticky "nesw" 
    grid $base.fra20.cpd22.02  -in $base.fra20.cpd22 -column "0" -row "1" -columnspan "1"  -rowspan "1" -sticky "ew" 
    grid $base.fra20.cpd22.03  -in $base.fra20.cpd22 -column "1" -row "0" -columnspan "1"  -rowspan "1" -sticky "ns" 
    grid $base.fra20.lab23  -in $base.fra20 -column "0" -row "0" -columnspan "1" -rowspan "1"  -sticky "ew" 
    grid $base.fra20.fra26  -in $base.fra20 -column "0" -row "1" -columnspan "1" -rowspan "1"  -ipadx "2" -ipady "2" -padx "2" -sticky "new" 
    grid columnconf $base.fra20.fra26 0 -weight 1
    grid $base.fra20.fra26.but27  -in $base.fra20.fra26 -column "0" -row "1" -columnspan "1"  -rowspan "1" -pady "2" -sticky "w" 
    grid $base.fra20.fra26.ent28  -in $base.fra20.fra26 -column "0" -row "0" -columnspan "1"  -rowspan "1" -sticky "ew" 
    grid $base.fra20.lab32  -in $base.fra20 -column "1" -row "0" -columnspan "1" -rowspan "1"  -sticky "ew" 
    grid $base.fra29  -in $base -column "0" -row "3" -columnspan "1" -rowspan "1"  -ipadx "2" -ipady "2" -padx "2" -pady "2" -sticky "ew" 
    grid columnconf $base.fra29 0 -weight 1
    grid columnconf $base.fra29 1 -weight 1
    grid $base.fra29.but30  -in $base.fra29 -column "0" -row "0" -columnspan "1" -rowspan "1"  -sticky "w" 
    grid $base.fra29.but31  -in $base.fra29 -column "2" -row "0" -columnspan "1" -rowspan "1"  -sticky "e" 
    grid $base.fra29.but33  -in $base.fra29 -column "1" -row "0" -columnspan "1" -rowspan "1"  -sticky "e" 

    grid columnconf $base.fra34 0 -weight 1
    grid rowconf $base.fra34 1 -weight 1

    grid $base.fra34.cpd35  -in $base.fra34 -column "0" -row "1" -columnspan "1" -rowspan "1"  -sticky "nesw" 
    grid columnconf $base.fra34.cpd35 0 -weight 1
    grid rowconf $base.fra34.cpd35 0 -weight 1
    grid $base.fra34.cpd35.01  -in $base.fra34.cpd35 -column "0" -row "0" -columnspan "1"  -rowspan "1" -sticky "nesw" 
    grid $base.fra34.cpd35.02  -in $base.fra34.cpd35 -column "0" -row "1" -columnspan "1"  -rowspan "1" -sticky "ew" 
    grid $base.fra34.cpd35.03  -in $base.fra34.cpd35 -column "1" -row "0" -columnspan "1"  -rowspan "1" -sticky "ns" 
    grid $base.fra34.lab36  -in $base.fra34 -column "0" -row "0" -columnspan "1" -rowspan "1"  -sticky "new" 

    grid columnconf $base.fra42 0 -weight 1
    grid rowconf $base.fra42 1 -weight 1

    grid $base.fra42.mes44  -in $base.fra42 -column "0" -row "1" -columnspan "1" -rowspan "1"  -sticky "nesw" 
    grid $base.fra42.lab17  -in $base.fra42 -column "1" -row "1" -columnspan "1" -rowspan "1"  -ipadx "8" -ipady "8" -padx "8" -pady "8" 
    grid $base.lab17  -in $base -column "0" -row "2" -columnspan "1" -rowspan "1" -padx "2"  -sticky "ew"
}

Window show .
Window show .top17

