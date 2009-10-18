#!/usr/bin/wish
#############################################################################
# Visual Tcl v1.20 Project
#

#################################
# GLOBAL VARIABLES
#
global agentforward; 
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
global noexec; 
global nopriv; 
global quiet; 
global sshverconnect; 
global stricthost; 
global termicon; 
global verbose; 
global widget; 
    set widget(addent) {.top17.fra44.fra19.fra28.02}
    set widget(agentent) {.top17.fra44.fra19.fra27.02}
    set widget(askeduser) {.top21.fra22.ent24}
    set widget(askpassent) {.top17.fra44.fra19.fra29.02}
    set widget(browserent) {.top17.fra44.fra19.fra18.02}
    set widget(cfgfileent) {.top17.fra21.fra24.ent21}
    set widget(commandent) {.top17.fra21.fra24.ent26}
    set widget(comment) {.top17.fra21.ent23}
    set widget(commentent) {.top17.ent32}
    set widget(comprlev) {.top17.fra21.fra25.02}
    set widget(configsbut) {.top17.fra30.fra19.but24}
    set widget(madadminbut) {.top17.fra30.fra19.but25}
    set widget(connectsbut) {.top17.fra30.fra19.but20}
    set widget(defsites) {.top17.fra46.fra24.fra25.01}
    set widget(distkeyent) {.top17.fra35.ent40}
    set widget(host) {.top17.fra21.ent25}
    set widget(hostent) {.top17.fra21.fra24.ent20}
    set widget(hostkeyview) {.top19.cpd21.03}
    set widget(identityent) {.top17.fra21.fra24.ent19}
    set widget(identpath) {.top52.fra18.ent26}
    set widget(idents) {.top17.fra35.fra20.fra22.01}
    set widget(inport) {.top17.fra21.ent27}
    set widget(keydisthost) {.top17.fra35.fra17.cpd29.01}
    set widget(keydistkey) {.top17.fra35.fra17.ent23}
    set widget(keydistuser) {.top17.fra35.fra17.fra17.01}
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
    set widget(rev,.top17.fra30.fra19.but25) {madadminbut}
    set widget(rev,.top17.fra35.ent40) {distkeyent}
    set widget(rev,.top17.fra35.fra17.cpd29.01) {keydisthost}
    set widget(rev,.top17.fra35.fra17.ent23) {keydistkey}
    set widget(rev,.top17.fra35.fra17.fra17.01) {keydistuser}
    set widget(rev,.top17.fra35.fra17.fra33.ent35) {keydistuser}
    set widget(rev,.top17.fra35.fra20.fra22.01) {idents}
    set widget(rev,.top17.fra44.ent25) {sshbinent}
    set widget(rev,.top17.fra44.fra17.ent19) {xterment}
    set widget(rev,.top17.fra44.fra19.fra18.02) {browserent}
    set widget(rev,.top17.fra44.fra19.fra20.ent22) {sshent}
    set widget(rev,.top17.fra44.fra19.fra26.02) {keygenent}
    set widget(rev,.top17.fra44.fra19.fra27.02) {agentent}
    set widget(rev,.top17.fra44.fra19.fra28.02) {addent}
    set widget(rev,.top17.fra44.fra19.fra29.02) {askpassent}
    set widget(rev,.top17.fra44.fra19.fra30.02) {scpent}
    set widget(rev,.top17.fra44.fra19.fra32.02) {xterment}
    set widget(rev,.top17.fra44.fra29.ent25) {sshent}
    set widget(rev,.top17.fra46.fra24.fra25.01) {defsites}
    set widget(rev,.top17.fra46.fra26.fra27.01) {specsites}
    set widget(rev,.top19.cpd20.03) {viewkey}
    set widget(rev,.top19.cpd21.03) {hostkeyview}
    set widget(rev,.top20.lab36) {proplabel}
    set widget(rev,.top21.fra22.ent24) {askeduser}
    set widget(rev,.top27.fra26.01) {sspecsites}
    set widget(rev,.top27.fra26.03) {sdefsites}
    set widget(rev,.top34.fra35.cpd38.01) {scpdirsl}
    set widget(rev,.top34.fra35.cpd39.01) {scpfilesl}
    set widget(rev,.top34.fra37.cpd40.01) {scpdirsr}
    set widget(rev,.top34.fra37.cpd41.01) {scpfilesr}
    set widget(rev,.top40.fra21.ent23) {newtit}
    set widget(rev,.top40.fra21.ent25) {newaddr}
    set widget(rev,.top40.fra21.ent27) {newuser}
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
    set widget(rfcomment) {.top51.lab24}
    set widget(rfcommentent) {.top51.fra36.011}
    set widget(rfhost) {.top51.fra36.08}
    set widget(rfin) {.top51.fra36.01}
    set widget(rforwards) {.top51.cpd53.01}
    set widget(rfout) {.top51.fra36.03}
    set widget(scpbut) {.top17.fra30.fra19.but21}
    set widget(scpdirsl) {.top34.fra35.cpd38.01}
    set widget(scpdirsr) {.top34.fra37.cpd40.01}
    set widget(scpent) {.top17.fra44.fra19.fra30.02}
    set widget(scpfilesl) {.top34.fra35.cpd39.01}
    set widget(scpfilesr) {.top34.fra37.cpd41.01}
    set widget(scphosts) {.top17.fra27.fra30.cpd31.01}
    set widget(scpusers) {.top17.fra27.fra30.cpd32.01}
    set widget(sdefsites) {.top27.fra26.03}
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
    if {$base == ""} {
        set base .top17
    }
    if {[winfo exists $base]} {
        wm deiconify $base; return
    }
    ###################
    # CREATING WIDGETS
    ###################
    toplevel $base -class Toplevel -borderwidth "4" -menu ".top17.m17" 
    wm focusmodel $base passive
    wm geometry $base 430x463
    wm maxsize $base 1009 738
    wm minsize $base 1 1
    wm overrideredirect $base 0
    wm resizable $base 1 1
    wm deiconify $base
    wm title $base "SecPanel"
    menu $base.m17 \
        -borderwidth "1" -cursor "" -tearoff "0" -background LightSeaGreen -bd 2
    $base.m17 add cascade \
        -menu ".top17.m17.men18" -label "Program" -background orange
    $base.m17 add cascade \
        -menu ".top17.m17.men31" -label "Config" -background orange
    $base.m17 add cascade \
        -menu ".top17.m17.men19" -label "Help" -background orange
    menu $base.m17.men18 \
        -borderwidth "1" -tearoff "0" 
    $base.m17.men18 add command \
        -command "historyman show" -label "Command history" 
    $base.m17.men18 add separator
    $base.m17.men18 add command \
        -command "do_exit" -label "Exit" 
    menu $base.m17.men19 \
        -borderwidth "1" -tearoff "0" 
    $base.m17.men19 add command \
        -label "Help Index (no help yet)" 
    $base.m17.men19 add cascade \
        -menu ".top17.m17.men19.men20" -label "SSH Manuals" 
    $base.m17.men19 add cascade \
        -menu ".top17.m17.men19.men17" -label "Update checks" 
    $base.m17.men19 add separator
    $base.m17.men19 add command \
        -command "about" -label "About SecPanel" 
    menu $base.m17.men19.men20 \
        -borderwidth "1" -tearoff "0" 
    $base.m17.men19.men20 add command \
        -command "showman ssh" -label "ssh" 
    $base.m17.men19.men20 add command \
        -command "showman sshd" -label "sshd" 
    $base.m17.men19.men20 add command \
        -command "showman ssh-keygen" -label "ssh-keygen" 
    $base.m17.men19.men20 add command \
        -command "showman ssh-agent" -label "ssh-agent" 
    $base.m17.men19.men20 add command \
        -command "showman ssh-add" -label "ssh-add" 
    menu $base.m17.men19.men17 \
        -borderwidth "1" -tearoff "0" 
    $base.m17.men19.men17 add command \
        -command "check_sources ssh" -label "Check for SSH updates" \
        -state "disabled" 
    $base.m17.men19.men17 add command \
        -command "check_sources sp" -label "See SecPanel homepage" 
    menu $base.m17.men31 \
        -borderwidth "1" -tearoff "0" 
    $base.m17.men31 add command \
        -command "colorman 1" -label "Program colors" 
    $base.m17.men31 add command \
        -command "fontman show" -label "Program fonts" 
    $base.m17.men31 add checkbutton \
        -variable "configs(wingeom)" -command "save_globals geom" \
        -label "Remember window geometry" 
    frame $base.fra21 \
        -borderwidth "1" -height "75" -width "125" 
    frame $base.fra21.fra23 \
        -height "75" -width "125" 
    checkbutton $base.fra21.fra23.01 \
        -anchor "w" -borderwidth "1" -text "No agent forwarding" \
        -variable "agentforward" -bg orange
    checkbutton $base.fra21.fra23.02 \
        -anchor "w" -borderwidth "1" -text "No X11 forwarding" \
        -variable "x11forward" -bg orange
    checkbutton $base.fra21.fra23.che17 \
        -anchor "w" -borderwidth "1" -text "No priv. source port" -variable "nopriv" -bg orange
    checkbutton $base.fra21.fra23.che20 \
        -anchor "w" -borderwidth "1" -text "Verbose" -variable "verbose" -bg orange
    checkbutton $base.fra21.fra23.che21 \
        -anchor "w" -borderwidth "1" -text "Quiet" -variable "quiet" -bg orange
    checkbutton $base.fra21.fra23.che22 \
        -anchor "w" -borderwidth "1" -text "Fork into background" -variable "fork" -bg orange
    checkbutton $base.fra21.fra23.che39 \
        -borderwidth "1" -text "Strict hostkey check" -variable "stricthost" -bg orange
    checkbutton $base.fra21.fra23.che29 \
        -anchor "w" -borderwidth "1" -text "Wait after action" -variable "connwait" -bg orange
    checkbutton $base.fra21.fra23.che30 \
        -anchor "w" -borderwidth "1" -text "Terminal iconified" -variable "termicon" -bg orange
    frame $base.fra21.fra24 \
        -height "75" -width "125" 
    label $base.fra21.fra24.lab17 \
        -anchor "w" -borderwidth "1" -text "Title:" 
    entry $base.fra21.fra24.ent18 \
        -borderwidth "1" -width "15" 
    label $base.fra21.fra24.lab19 \
        -anchor "w" -borderwidth "0" -relief "raised" -text "Host:" 
    entry $base.fra21.fra24.ent20 \
        -borderwidth "1" -width "15" 
    label $base.fra21.fra24.lab21 \
        -anchor "w" -borderwidth "0" -relief "raised" -text "User:" 
    entry $base.fra21.fra24.ent22 \
        -borderwidth "1" -width "15" 
    label $base.fra21.fra24.lab23 \
        -anchor "w" -borderwidth "0" -relief "raised" -text "Port:" 
    entry $base.fra21.fra24.ent24 \
        -borderwidth "1" -width "15" 
    label $base.fra21.fra24.lab25 \
        -anchor "w" -borderwidth "0" -relief "raised" -text "Exec:" 
    entry $base.fra21.fra24.ent26 \
        -borderwidth "1" -width "15" 
    checkbutton $base.fra21.fra24.che17 \
        -anchor "w" -borderwidth "1" -text "Ask" -variable "askuserspec" -bg orange
    label $base.fra21.fra24.lab41 \
        -anchor "w" -borderwidth "0" -relief "raised" -text "Subsys:" 
    entry $base.fra21.fra24.ent42 \
        -borderwidth "1" -width "15" 
    checkbutton $base.fra21.fra24.che43 \
        -anchor "w" -borderwidth "1" -text "No Exec" -variable "noexec" -bg orange
    label $base.fra21.fra24.lab18 \
        -anchor "w" -borderwidth "0" -text "Identity:" 
    entry $base.fra21.fra24.ent19 \
        -borderwidth "1" -width "15" 
    label $base.fra21.fra24.lab20 \
        -anchor "w" -borderwidth "0" -text "Cfg-file:" 
    entry $base.fra21.fra24.ent21 \
        -borderwidth "1" -width "15" 
    button $base.fra21.fra24.but22 \
        -command "choosefile {} {} {} identityent" -padx "9" -pady "3" \
        -relief "groove" 
    button $base.fra21.fra24.but23 \
        -command "choosefile {} {} {} cfgfileent" -padx "9" -pady "3" \
        -relief "groove" 
    label $base.fra21.fra24.lab24 \
        -anchor "w" -borderwidth "1" -foreground "#000000" -text "Algo:" 
    menubutton $base.fra21.fra24.men25 \
        -indicatoron "1" -menu ".top17.fra21.fra24.men25.01" -padx "4" -pady "3" -relief "groove" -text "idea" -textvariable "algo" -bg orange
    menu $base.fra21.fra24.men25.01 \
        -borderwidth "1" -cursor "" -tearoff "0" 
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
        -anchor "w" -borderwidth "1" -text "Compression" -variable "compress" -bg orange
    scale $base.fra21.fra24.sca27 \
        -borderwidth "1" -orient "horizontal" -showvalue "0" \
        -sliderlength "20" -sliderrelief "groove" -to "9.0" \
        -variable "compressval" 
    label $base.fra21.fra24.lab28 \
        -borderwidth "0" -text "0" -textvariable "compressval" 
    frame $base.fra21.fra26 \
        -height "75" -width "125" 
    button $base.fra21.fra26.01 \
        -anchor "w" -command "open_forwardings l" -padx "9" -pady "3" \
        -relief "groove" -text "Local forwards" -bg orange
    button $base.fra21.fra26.02 \
        -anchor "w" -command "open_forwardings r" -padx "9" -pady "3" \
        -relief "groove" -text "Remote forwards" -bg orange
    checkbutton $base.fra21.fra26.03 \
        -anchor "w" -borderwidth "1" -text "Run as gateway" -variable "gateway" -bg orange
    frame $base.fra21.fra48 \
        -borderwidth "2" -height "75" -relief "groove" -width "110" 
    frame $base.fra21.fra48.cpd18 \
        -height "30" -relief "raised" -width "3" 
    listbox $base.fra21.fra48.cpd18.01 \
        -borderwidth "1" -height "3" -width "10" \
        -xscrollcommand ".top17.fra21.fra48.cpd18.02 set" \
        -yscrollcommand ".top17.fra21.fra48.cpd18.03 set" 
    bind $base.fra21.fra48.cpd18.01 <Double-Button-1> {
        load_profile ssh
    }
    scrollbar $base.fra21.fra48.cpd18.02 \
        -borderwidth "1" -command ".top17.fra21.fra48.cpd18.01 xview" \
        -orient "horizontal" -width "10" 
    scrollbar $base.fra21.fra48.cpd18.03 \
        -borderwidth "1" -command ".top17.fra21.fra48.cpd18.01 yview" \
        -width "10" 
    frame $base.fra21.fra48.fra19 \
        -height "75" -width "125" 
    entry $base.fra21.fra48.fra19.ent20 \
        -borderwidth "1" -width "2" 
    label $base.fra21.fra48.fra19.lab21 \
        -borderwidth "1" -text "Profile Name" 
    frame $base.fra21.fra48.fra40 \
        -height "75" -width "125" 
    button $base.fra21.fra48.fra40.but41 \
        -command "save_profile" -padx "9" -pady "3" -relief "groove" \
        -text "Save" -bg orange
    button $base.fra21.fra48.fra40.but42 \
        -command "delete_profile" -padx "9" -pady "3" -relief "groove" \
        -text "Delete" -bg orange
    button $base.fra21.fra48.fra40.but43 \
        -command "load_profile ssh" -padx "9" -pady "3" -relief "groove" \
        -text "Load" -bg orange
    button $base.fra21.fra48.fra40.but44 \
        -command "clear_profiles" -padx "9" -pady "3" -relief "groove" \
        -text "New" -bg orange
    frame $base.fra21.fra35 \
        -height "75" -width "125" 
    radiobutton $base.fra21.fra35.rad36 \
        -anchor "e" -borderwidth "1" -text "IPv4" -value "4" -variable "ipverconnect" -bg orange
    radiobutton $base.fra21.fra35.rad37 \
        -anchor "w" -borderwidth "1" -text "IPv6" -value "6" -variable "ipverconnect" -bg orange
    radiobutton $base.fra21.fra35.rad38 \
        -anchor "e" -borderwidth "1" -text "SSH 1" -value "1" -variable "sshverconnect" -bg orange
    radiobutton $base.fra21.fra35.rad39 \
        -anchor "w" -borderwidth "1" -text "SSH 2" -value "2" -variable "sshverconnect" -bg orange
    frame $base.fra30 \
        -height "75" -width "125" 
    frame $base.fra30.fra19 \
        -height "75" -width "125" 
    button $base.fra30.fra19.but20 \
        -command "changetab connect" -padx "9" -pady "3" -relief "groove" 
    bind $base.fra30.fra19.but20 <Enter> {
        showstatus "Lists of configured SSH connections"
    }
    bind $base.fra30.fra19.but20 <Leave> {
        showstatus ""
    }
    button $base.fra30.fra19.but21 \
        -command "changetab scp" -padx "9" -pady "3" -relief "groove" 
    bind $base.fra30.fra19.but21 <Enter> {
        showstatus "Manage SCP transfers"
    }
    bind $base.fra30.fra19.but21 <Leave> {
        showstatus ""
    }
    button $base.fra30.fra19.but22 \
        -command "changetab ssh" -padx "9" -pady "3" -relief "groove" 
    bind $base.fra30.fra19.but22 <Enter> {
        showstatus "Manage connection profiles"
    }
    bind $base.fra30.fra19.but22 <Leave> {
        showstatus ""
    }
    button $base.fra30.fra19.but23 \
        -command "changetab key" -padx "9" -pady "3" -relief "groove" 
    bind $base.fra30.fra19.but23 <Enter> {
        showstatus "Manage your keypairs"
    }
    bind $base.fra30.fra19.but23 <Leave> {
        showstatus ""
    }
    button $base.fra30.fra19.but24 \
        -command "changetab terminal" -padx "9" -pady "3" -relief "groove" 
    bind $base.fra30.fra19.but24 <Enter> {
        showstatus "Configurations"
    }
    bind $base.fra30.fra19.but24 <Leave> {
        showstatus ""
    }
    button $base.fra30.fra19.but25 \
        -command "indiadmin" -padx "9" -pady "3" -relief "groove" 
    bind $base.fra30.fra19.but25 <Enter> {
        showstatus "IndiMail Administration"
    }
    bind $base.fra30.fra19.but25 <Leave> {
        showstatus ""
    }
    button $base.fra30.but26 \
        -command "changetab small" -padx "9" -pady "3" -relief "groove" 
    bind $base.fra30.but26 <Enter> {
        showstatus "Switch to satellite GUI"
    }
    bind $base.fra30.but26 <Leave> {
        showstatus ""
    }
    frame $base.fra35 \
        -borderwidth "1" -height "75" -width "125" 
    frame $base.fra35.fra17 \
        -borderwidth "2" -height "75" -relief "groove" -width "125" 
    frame $base.fra35.fra17.cpd29 \
        -borderwidth "1" -height "30" -width "30" 
    listbox $base.fra35.fra17.cpd29.01 \
        -borderwidth "1" -height "3" -width "18" \
        -xscrollcommand ".top17.fra35.fra17.cpd29.02 set" \
        -yscrollcommand ".top17.fra35.fra17.cpd29.03 set" 
    bind $base.fra35.fra17.cpd29.01 <Button-1> {
        .top17.fra35.fra17.cpd29.01 activate @%x,%y
	updateDistLabel
    }
    scrollbar $base.fra35.fra17.cpd29.02 \
        -borderwidth "1" -command ".top17.fra35.fra17.cpd29.01 xview" \
        -orient "horizontal" -width "10" 
    scrollbar $base.fra35.fra17.cpd29.03 \
        -borderwidth "1" -command ".top17.fra35.fra17.cpd29.01 yview" \
        -width "10" 
    label $base.fra35.fra17.lab39 \
        -anchor "nw" -borderwidth "1" -height "2" -justify "left" \
        -text "Dist. to host:" -width "20" 
    frame $base.fra35.fra17.fra17 \
        -borderwidth "1" -height "30" -width "30" 
    listbox $base.fra35.fra17.fra17.01 \
        -borderwidth "1" -height "3" -width "18" \
        -xscrollcommand ".top17.fra35.fra17.fra17.02 set" \
        -yscrollcommand ".top17.fra35.fra17.fra17.03 set" 
    bind $base.fra35.fra17.fra17.01 <Button-1> {
        .top17.fra35.fra17.fra17.01 activate @%x,%y
	updateDistLabel
    }
    scrollbar $base.fra35.fra17.fra17.02 \
        -borderwidth "1" -command ".top17.fra35.fra17.fra17.01 xview" \
        -orient "horizontal" -width "10" 
    scrollbar $base.fra35.fra17.fra17.03 \
        -borderwidth "1" -command ".top17.fra35.fra17.fra17.01 yview" \
        -width "10" 
    label $base.fra35.fra17.lab19 \
        -anchor "w" -borderwidth "1" -text "Dist. as user:" 
    entry $base.fra35.fra17.ent23 \
        -borderwidth "1" -width "16" 
    button $base.fra35.fra17.but24 \
        -command "seldistkey" -padx "9" -pady "3" -relief "groove" \
        -text "Select key" -bg orange
    label $base.fra35.fra17.lab29 \
        -anchor "w" -borderwidth "1" -foreground "#000000" \
        -text "Key-Distribution" 
    frame $base.fra35.fra17.fra33 \
        -borderwidth "1" -height "75" -width "125" 
    button $base.fra35.fra17.fra33.but34 \
        -command "distkey" -padx "9" -pady "3" -relief "groove" \
        -text "Distribute key" -bg orange
    button $base.fra35.fra17.fra33.but35 \
        -command "edit_account" -relief "groove" -state "disabled" \
        -text "Edit account" -bg orange
    frame $base.fra35.fra20 \
        -borderwidth "2" -height "75" -relief "groove" -width "125" 
    button $base.fra35.fra20.but22 \
        -command "manage_agent launch" -padx "9" -pady "3" -relief "groove" \
        -text "Launch agent" -bg orange
    button $base.fra35.fra20.but26 \
        -command "manage_agent remident" -padx "9" -pady "3" -relief "groove" \
        -text "Remove identity" -bg orange
    checkbutton $base.fra35.fra20.che28 \
        -borderwidth "1" -command "save_globals agent" \
        -text "Launch agent at startup" -variable "launcher" 
    frame $base.fra35.fra20.fra22 \
        -borderwidth "1" -height "30" -width "30" 
    listbox $base.fra35.fra20.fra22.01 \
        -borderwidth "1" -height "3" -width "18" \
        -xscrollcommand ".top17.fra35.fra20.fra22.02 set" \
        -yscrollcommand ".top17.fra35.fra20.fra22.03 set" 
    scrollbar $base.fra35.fra20.fra22.02 \
        -borderwidth "1" -command ".top17.fra35.fra20.fra22.01 xview" \
        -orient "horizontal" -width "10" 
    scrollbar $base.fra35.fra20.fra22.03 \
        -borderwidth "1" -command ".top17.fra35.fra20.fra22.01 yview" \
        -width "10" 
    button $base.fra35.fra20.but20 \
        -command "manage_agent addident" -padx "9" -pady "3" -relief "groove" \
        -text "Add identity" -bg orange
    button $base.fra35.fra20.but17 \
        -command "manage_agent kill" -padx "9" -pady "3" -relief "groove" \
        -text "Kill agent" -bg orange
    label $base.fra35.fra20.lab28 \
        -anchor "w" -borderwidth "1" -foreground "#000000" -text "SSH-Agent" 
    button $base.fra35.fra20.but18 \
        -command "manage_agent chdef" -padx "9" -pady "3" -relief "groove" \
        -text "Set Default" -bg orange
    button $base.fra35.fra20.but30 \
        -command "manage_agent info" -relief "groove" -text "Agent info" -bg orange
    frame $base.fra35.fra40 \
        -borderwidth "2" -height "75" -relief "groove" -width "125" 
    button $base.fra35.fra40.but43 \
        -command "keygen 0" -padx "9" -pady "3" -relief "groove" \
        -text "Manage keypairs" -bg orange
    label $base.fra35.fra40.lab30 \
        -anchor "w" -borderwidth "1" -foreground "#000000" -text "Keypairs" 
    frame $base.fra35.fra47 \
        -borderwidth "2" -height "75" -relief "groove" -width "125" 
    label $base.fra35.fra47.lab48 \
        -anchor "w" -borderwidth "1" -foreground "#000000" -text "Hostkeys" 
    button $base.fra35.fra47.but49 \
        -command "hostkey edit" -padx "9" -pady "3" -relief "groove" \
        -text "Manage hostkeys" -bg orange
    frame $base.fra44 \
        -borderwidth "1" -height "75" 
    frame $base.fra44.fra27 \
        -borderwidth "2" -height "75" -relief "groove" 
    button $base.fra44.fra27.but28 \
        -command "save_globals bins" -padx "9" -pady "3" -relief "groove" \
        -text "Save" -bg orange
    frame $base.fra44.fra19 \
        -height "75" 
    frame $base.fra44.fra19.fra20 \
        -borderwidth "1" -height "75" 
    label $base.fra44.fra19.fra20.lab21 \
        -anchor "w" -borderwidth "1" -text "SSH" 
    entry $base.fra44.fra19.fra20.ent22 \
        -borderwidth "1" 
    button $base.fra44.fra19.fra20.but23 \
        -command "browsebin ssh" -padx "9" -pady "3" -relief "groove" 
    frame $base.fra44.fra19.fra26 \
        -borderwidth "1" -height "75" 
    label $base.fra44.fra19.fra26.01 \
        -anchor "w" -borderwidth "1" -text "SSH-Keygen" 
    entry $base.fra44.fra19.fra26.02 \
        -borderwidth "1" 
    button $base.fra44.fra19.fra26.03 \
        -command "browsebin keygen" -padx "9" -pady "3" -relief "groove" 
    frame $base.fra44.fra19.fra27 \
        -borderwidth "1" -height "75" 
    label $base.fra44.fra19.fra27.01 \
        -anchor "w" -text "SSH-Agent" 
    entry $base.fra44.fra19.fra27.02 \
        -borderwidth "1" 
    button $base.fra44.fra19.fra27.03 \
        -command "browsebin agent" -padx "9" -pady "3" -relief "groove" 
    frame $base.fra44.fra19.fra28 \
        -borderwidth "1" -height "75" 
    label $base.fra44.fra19.fra28.01 \
        -anchor "w" -text "SSH-Add" 
    entry $base.fra44.fra19.fra28.02 \
        -borderwidth "1" 
    button $base.fra44.fra19.fra28.03 \
        -command "browsebin add" -padx "9" -pady "3" -relief "groove" 
    frame $base.fra44.fra19.fra29 \
        -borderwidth "1" -height "75" 
    label $base.fra44.fra19.fra29.01 \
        -anchor "w" -borderwidth "1" -text "SSH-Askpass" 
    entry $base.fra44.fra19.fra29.02 \
        -borderwidth "1" 
    button $base.fra44.fra19.fra29.03 \
        -command "browsebin askpass" -padx "9" -pady "3" -relief "groove" 
    frame $base.fra44.fra19.fra32 \
        -borderwidth "1" -height "75" 
    label $base.fra44.fra19.fra32.01 \
        -anchor "w" -borderwidth "1" -text "XTerminal" 
    entry $base.fra44.fra19.fra32.02 \
        -borderwidth "1" 
    button $base.fra44.fra19.fra32.03 \
        -command "browsebin xterm" -padx "9" -pady "3" -relief "groove" 
    frame $base.fra44.fra19.fra32.fra38 \
        -borderwidth "1" -height "75" 
    label $base.fra44.fra19.fra32.fra38.lab39 \
        -anchor "w" -borderwidth "1" -text "Terminal-type" 
    menubutton $base.fra44.fra19.fra32.fra38.men40 \
        -indicatoron "1" -menu ".top17.fra44.fra19.fra32.fra38.men40.01" \
        -padx "4" -pady "3" -relief "groove" -text "menu" -textvariable "configs(termver)" -bg orange
    menu $base.fra44.fra19.fra32.fra38.men40.01 \
        -borderwidth "1" -cursor "" -tearoff "0"
    $base.fra44.fra19.fra32.fra38.men40.01 add radiobutton \
        -value "Xterm" -variable "configs(termver)" -label "Xterm" 
    $base.fra44.fra19.fra32.fra38.men40.01 add radiobutton \
        -value "Eterm" -variable "configs(termver)" -label "Eterm" 
    $base.fra44.fra19.fra32.fra38.men40.01 add radiobutton \
        -value "Aterm" -variable "configs(termver)" -label "Aterm" 
    $base.fra44.fra19.fra32.fra38.men40.01 add radiobutton \
        -value "Rxvt" -variable "configs(termver)" -label "Rxvt" 
    $base.fra44.fra19.fra32.fra38.men40.01 add radiobutton \
        -value "KDE-Term" -variable "configs(termver)" -label "KDE-Term" \
        -state "disabled" 
    $base.fra44.fra19.fra32.fra38.men40.01 add radiobutton \
        -value "GNOME-Term" -variable "configs(termver)" -label "GNOME-Term" 
    frame $base.fra44.fra19.fra30 \
        -borderwidth "1" -height "75" 
    label $base.fra44.fra19.fra30.01 \
        -anchor "w" -borderwidth "1" -text "Secure Copy" 
    entry $base.fra44.fra19.fra30.02 \
        -borderwidth "1" 
    button $base.fra44.fra19.fra30.03 \
        -command "browsebin scp" -padx "9" -pady "3" -relief "groove" 
    frame $base.fra44.fra19.fra18 \
        -borderwidth "1" -height "75" 
    label $base.fra44.fra19.fra18.01 \
        -anchor "w" -borderwidth "1" -text "WWW browser" 
    entry $base.fra44.fra19.fra18.02 \
        -borderwidth "1" 
    button $base.fra44.fra19.fra18.03 \
        -command "browsebin browser" -padx "9" -pady "3" -relief "groove" 
    label $base.fra44.lab30 \
        -anchor "w" -borderwidth "1" -foreground "#000000" \
        -text "Programs used by SecPanel" 
    menubutton $base.fra44.men31 \
        -anchor "w" -indicatoron "1" -menu ".top17.fra44.men31.m" -padx "4" \
        -pady "3" -relief "groove" -text "menu" -textvariable "configs(sshver)" -bg orange
    menu $base.fra44.men31.m \
        -borderwidth "1" -cursor "" -tearoff "0" 
    $base.fra44.men31.m add radiobutton \
        -value "SSH.com" -variable "configs(sshver)" -label "SSH.com"
    $base.fra44.men31.m add radiobutton \
        -value "OpenSSH" -variable "configs(sshver)" -label "OpenSSH"
    $base.fra44.men31.m add command \
        -command "probeversion" -label "Probe version" 
    label $base.fra44.lab32 \
        -anchor "w" -borderwidth "1" \
        -text "Version of all the SSH programs: " 
    frame $base.fra46 \
        -borderwidth "1" -height "75" 
    frame $base.fra46.fra24 \
        -borderwidth "2" -height "75" -relief "groove" 
    frame $base.fra46.fra24.fra25 \
        -height "30" -relief "raised" -width "30" 
    listbox $base.fra46.fra24.fra25.01 \
        -borderwidth "1" -height "6" \
        -xscrollcommand ".top17.fra46.fra24.fra25.02 set" \
        -yscrollcommand ".top17.fra46.fra24.fra25.03 set" 
    bind $base.fra46.fra24.fra25.01 <Button-3> {
        set li [.top17.fra46.fra24.fra25.01 nearest %y]
		.top17.fra46.fra24.fra25.01 activate $li
		.top17.fra46.fra24.fra25.01 selection clear 0 end
		.top17.fra46.fra24.fra25.01 selection set $li
        showmenu def
    }
    bind $base.fra46.fra24.fra25.01 <Double-Button-1> {
        connect def
    }
    bind $base.fra46.fra24.fra25.01 <Enter> {
        showstatus "Right click mouse to handle entries"
    }
    bind $base.fra46.fra24.fra25.01 <Leave> {
        showstatus ""
    }
    scrollbar $base.fra46.fra24.fra25.02 \
        -borderwidth "1" -command ".top17.fra46.fra24.fra25.01 xview" \
        -orient "horizontal" -width "10" 
    scrollbar $base.fra46.fra24.fra25.03 \
        -borderwidth "1" -command ".top17.fra46.fra24.fra25.01 yview" \
        -width "10" 
    frame $base.fra46.fra24.fra28 \
        -borderwidth "1" -height "75" -width "125" 
    button $base.fra46.fra24.fra28.but39 \
        -command "connect def" -padx "9" -pady "3" -relief "groove" \
        -text "Connect" -bg orange
    frame $base.fra46.fra24.fra28.fra23 \
        -height "75" -width "125" 
    button $base.fra46.fra24.fra28.fra23.but24 \
        -command "newconn def 1" -padx "9" -pady "3" -relief "groove" \
        -text "New" -bg orange
    button $base.fra46.fra24.fra28.fra23.but25 \
        -command "propconn def 1" -padx "9" -pady "3" -relief "groove" \
        -text "Edit" -bg orange
    button $base.fra46.fra24.fra28.fra23.but26 \
        -command "delconn def" -padx "9" -pady "3" -relief "groove" \
        -text "Delete" -bg IndianRed
    label $base.fra46.fra24.lab33 \
        -anchor "w" -borderwidth "1" -foreground "#000000" \
        -text "Connections using the default profile" 
    frame $base.fra46.fra26 \
        -borderwidth "2" -height "75" -relief "groove" -width "125" 
    frame $base.fra46.fra26.fra27 \
        -height "30" -relief "raised" -width "30" 
    listbox $base.fra46.fra26.fra27.01 \
        -borderwidth "1" -height "6" \
        -xscrollcommand ".top17.fra46.fra26.fra27.02 set" \
        -yscrollcommand ".top17.fra46.fra26.fra27.03 set" 
    bind $base.fra46.fra26.fra27.01 <Button-3> {
        set li [.top17.fra46.fra26.fra27.01 nearest %y]
	.top17.fra46.fra26.fra27.01 selection clear 0 end
	.top17.fra46.fra26.fra27.01 activate $li
	.top17.fra46.fra26.fra27.01 selection set $li
        showmenu spec
    }
    bind $base.fra46.fra26.fra27.01 <Double-Button-1> {
        connect spec
    }
    bind $base.fra46.fra26.fra27.01 <Enter> {
        showstatus "Right click mouse to handle entries"
    }
    bind $base.fra46.fra26.fra27.01 <Leave> {
        showstatus ""
    }
    scrollbar $base.fra46.fra26.fra27.02 \
        -borderwidth "1" -command ".top17.fra46.fra26.fra27.01 xview" \
        -orient "horizontal" -width "10" 
    scrollbar $base.fra46.fra26.fra27.03 \
        -borderwidth "1" -command ".top17.fra46.fra26.fra27.01 yview" \
        -width "10" 
    frame $base.fra46.fra26.fra29 \
        -borderwidth "1" -height "75" -width "125" 
    button $base.fra46.fra26.fra29.but38 \
        -command "connect spec" -padx "9" -pady "3" -relief "groove" \
        -text "Connect" -bg orange
    frame $base.fra46.fra26.fra29.fra28 \
        -height "75" -width "125" 
    button $base.fra46.fra26.fra29.fra28.but29 \
        -command "newconn spec 1" -padx "9" -pady "3" -relief "groove" \
        -text "New" -bg orange
    button $base.fra46.fra26.fra29.fra28.but30 \
        -command "propconn spec 1" -padx "9" -pady "3" -relief "groove" \
        -text "Edit" -bg orange
    button $base.fra46.fra26.fra29.fra28.but31 \
        -command "delconn spec" -padx "9" -pady "3" -relief "groove" \
        -text "Delete" -bg IndianRed
    label $base.fra46.fra26.lab34 \
        -anchor "w" -borderwidth "1" -foreground "#000000" \
        -text "Connections using special profiles" 
    frame $base.fra17 \
        -borderwidth "1" -height "75" -width "125" 
    label $base.fra17.lab18 \
        -anchor "w" -borderwidth "1" -relief "sunken" -width "30" 
    label $base.fra17.lab19 \
        -anchor "w" -borderwidth "1" -relief "sunken" -width "30" 
    frame $base.fra27 \
        -height "75" -width "125" 
    frame $base.fra27.fra30 \
        -borderwidth "1" -height "75" -width "125" 
    frame $base.fra27.fra30.cpd31 \
        -borderwidth "1" -height "30" -width "30" 
    listbox $base.fra27.fra30.cpd31.01 \
        -borderwidth "1" -height "8" \
        -xscrollcommand ".top17.fra27.fra30.cpd31.02 set" \
        -yscrollcommand ".top17.fra27.fra30.cpd31.03 set" 
    bind $base.fra27.fra30.cpd31.01 <Button-1> {
        .top17.fra27.fra30.cpd31.01 activate @%x,%y
        updateSCPLabel
    }
    scrollbar $base.fra27.fra30.cpd31.02 \
        -borderwidth "1" -command ".top17.fra27.fra30.cpd31.01 xview" \
        -orient "horizontal" -width "10" 
    scrollbar $base.fra27.fra30.cpd31.03 \
        -borderwidth "1" -command ".top17.fra27.fra30.cpd31.01 yview" \
        -width "10" 
    frame $base.fra27.fra30.cpd32 \
        -borderwidth "1" -height "30" -width "30" 
    listbox $base.fra27.fra30.cpd32.01 \
        -borderwidth "1" -height "8" \
        -xscrollcommand ".top17.fra27.fra30.cpd32.02 set" \
        -yscrollcommand ".top17.fra27.fra30.cpd32.03 set" 
    bind $base.fra27.fra30.cpd32.01 <Button-1> {
        .top17.fra27.fra30.cpd32.01 activate @%x,%y
	updateSCPLabel
    }
    scrollbar $base.fra27.fra30.cpd32.02 \
        -borderwidth "1" -command ".top17.fra27.fra30.cpd32.01 xview" \
        -orient "horizontal" -width "10" 
    scrollbar $base.fra27.fra30.cpd32.03 \
        -borderwidth "1" -command ".top17.fra27.fra30.cpd32.01 yview" \
        -width "10" 
    label $base.fra27.fra30.lab51 \
        -anchor "w" -borderwidth "1" -text "Hosts" 
    label $base.fra27.fra30.lab52 \
        -anchor "w" -borderwidth "1" -text "Accounts" 
    frame $base.fra27.fra30.fra18 \
        -height "75" -width "125" 
    button $base.fra27.fra30.fra18.but19 \
        -anchor "w" -command "scpman open" -height "2" -justify "left" \
        -padx "9" -pady "3" -relief "groove" \
        -text "Connect to user@host (choose from the lists)" \
        -wraplength "300"
    button $base.fra27.fra30.fra18.but20 \
        -command "newconn def 1" -height "2" -relief "groove" \
        -text "New connection" -bg orange
    frame $base.fra27.fra17 \
        -borderwidth "1" -height "75" -width "125" 
    label $base.fra27.fra17.lab18 \
        -anchor "w" -borderwidth "1" -text "SCP-Options" 
    checkbutton $base.fra27.fra17.che20 \
        -anchor "w" -borderwidth "1" -text "Suppress transfer statistics" \
        -variable "configs(scpstats)" -bg orange
    checkbutton $base.fra27.fra17.che21 \
        -anchor "w" -borderwidth "1" -text "Preserve file attributes" \
        -variable "configs(scppres)" -bg orange
    checkbutton $base.fra27.fra17.che22 \
        -anchor "w" -borderwidth "1" -text "Verbose" \
        -variable "configs(scpverb)" -bg orange
    checkbutton $base.fra27.fra17.che23 \
        -anchor "w" -borderwidth "1" -text "Compression" \
        -variable "configs(scpcomp)" -bg orange
    button $base.fra27.fra17.but17 \
        -command "save_globals scp" -padx "9" -pady "3" -relief "groove" \
        -text "Save options as default" -bg orange
    checkbutton $base.fra27.fra17.che17 \
        -anchor "w" -borderwidth "1" -text "Show hidden files" \
        -variable "configs(scpshowhidden)" -bg orange
    label $base.fra27.fra17.lab17 \
        -anchor "w" -borderwidth "1" -text "ListServer-Options" 
    checkbutton $base.fra27.fra17.che18 \
        -anchor "w" -borderwidth "1" -text "Verbose connection" \
        -variable "configs(lsverbose)" -bg orange
    checkbutton $base.fra27.fra17.che19 \
        -anchor "w" -borderwidth "1" -text "Wait after closing" \
        -variable "configs(lswait)" -bg orange
    radiobutton $base.fra27.fra17.rad20 \
        -anchor "w" -borderwidth "1" -state "disabled" -text "PERL" -value "perl" -variable "configs(lsinterpret)" -bg orange
    radiobutton $base.fra27.fra17.rad21 \
        -anchor "w" -borderwidth "1" -text "TCL" -value "tcl" -variable "configs(lsinterpret)" -bg orange
    label $base.fra27.fra17.lab23 \
        -anchor "w" -borderwidth "1" -text "Interpreter to use:" 
    frame $base.fra27.fra17.fra18 \
        -height "75" -width "125" 
    entry $base.fra27.fra17.fra18.ent19 \
        -borderwidth "1" -textvariable "configs(scpport)" -width "10" 
    label $base.fra27.fra17.fra18.lab20 \
        -borderwidth "0" -text "Port:" 
    ###################
    # SETTING GEOMETRY
    ###################
    grid columnconf $base 0 -weight 1
    grid rowconf $base 1 -weight 1

    grid columnconf $base.fra21 0 -weight 1
    grid rowconf $base.fra21 0 -weight 1

    grid $base.fra21.fra23 \
        -in ".top17.fra21" -column "0" -row "1" -columnspan "1" -rowspan "1" \
        -ipadx "2" -ipady "2" -padx "2" -pady "2" -sticky "new" 
    grid $base.fra21.fra23.01 \
        -in ".top17.fra21.fra23" -column "0" -row "0" -columnspan "1" \
        -rowspan "1" -sticky "ew" 
    grid $base.fra21.fra23.02 \
        -in ".top17.fra21.fra23" -column "0" -row "1" -columnspan "1" \
        -rowspan "1" -sticky "ew" 
    grid $base.fra21.fra23.che17 \
        -in ".top17.fra21.fra23" -column "0" -row "3" -columnspan "1" \
        -rowspan "1" -sticky "ew" 
    grid $base.fra21.fra23.che20 \
        -in ".top17.fra21.fra23" -column "0" -row "6" -columnspan "1" \
        -rowspan "1" -sticky "ew" 
    grid $base.fra21.fra23.che21 \
        -in ".top17.fra21.fra23" -column "0" -row "5" -columnspan "1" \
        -rowspan "1" -sticky "ew" 
    grid $base.fra21.fra23.che22 \
        -in ".top17.fra21.fra23" -column "0" -row "4" -columnspan "1" \
        -rowspan "1" -sticky "ew" 
    grid $base.fra21.fra23.che39 \
        -in ".top17.fra21.fra23" -column "0" -row "2" -columnspan "2" \
        -rowspan "1" -sticky "w" 
    grid $base.fra21.fra23.che29 \
        -in ".top17.fra21.fra23" -column "0" -row "7" -columnspan "1" \
        -rowspan "1" -sticky "ew" 
    grid $base.fra21.fra23.che30 \
        -in ".top17.fra21.fra23" -column "0" -row "8" -columnspan "1" \
        -rowspan "1" -sticky "ew" 
    grid $base.fra21.fra24 \
        -in ".top17.fra21" -column "1" -row "1" -columnspan "1" -rowspan "1" \
        -ipadx "2" -ipady "2" -padx "2" -pady "2" -sticky "new" 
    grid columnconf $base.fra21.fra24 1 -weight 1
    grid $base.fra21.fra24.lab17 \
        -in ".top17.fra21.fra24" -column "0" -row "1" -columnspan "1" \
        -rowspan "1" -sticky "ew" 
    grid $base.fra21.fra24.ent18 \
        -in ".top17.fra21.fra24" -column "1" -row "1" -columnspan "2" \
        -rowspan "1" -sticky "ew" 
    grid $base.fra21.fra24.lab19 \
        -in ".top17.fra21.fra24" -column "0" -row "2" -columnspan "1" \
        -rowspan "1" -sticky "ew" 
    grid $base.fra21.fra24.ent20 \
        -in ".top17.fra21.fra24" -column "1" -row "2" -columnspan "2" \
        -rowspan "1" -sticky "ew" 
    grid $base.fra21.fra24.lab21 \
        -in ".top17.fra21.fra24" -column "0" -row "3" -columnspan "1" \
        -rowspan "1" -sticky "ew" 
    grid $base.fra21.fra24.ent22 \
        -in ".top17.fra21.fra24" -column "1" -row "3" -columnspan "1" \
        -rowspan "1" -sticky "ew" 
    grid $base.fra21.fra24.lab23 \
        -in ".top17.fra21.fra24" -column "0" -row "4" -columnspan "1" \
        -rowspan "1" -sticky "ew" 
    grid $base.fra21.fra24.ent24 \
        -in ".top17.fra21.fra24" -column "1" -row "4" -columnspan "2" \
        -rowspan "1" -sticky "ew" 
    grid $base.fra21.fra24.lab25 \
        -in ".top17.fra21.fra24" -column "0" -row "5" -columnspan "1" \
        -rowspan "1" -sticky "ew" 
    grid $base.fra21.fra24.ent26 \
        -in ".top17.fra21.fra24" -column "1" -row "5" -columnspan "1" \
        -rowspan "1" -sticky "ew" 
    grid $base.fra21.fra24.che17 \
        -in ".top17.fra21.fra24" -column "2" -row "3" -columnspan "1" \
        -rowspan "1" -sticky "ew" 
    grid $base.fra21.fra24.lab41 \
        -in ".top17.fra21.fra24" -column "0" -row "6" -columnspan "1" \
        -rowspan "1" -sticky "ew" 
    grid $base.fra21.fra24.ent42 \
        -in ".top17.fra21.fra24" -column "1" -row "6" -columnspan "2" \
        -rowspan "1" -sticky "ew" 
    grid $base.fra21.fra24.che43 \
        -in ".top17.fra21.fra24" -column "2" -row "5" -columnspan "1" \
        -rowspan "1" -sticky "ew" 
    grid $base.fra21.fra24.lab18 \
        -in ".top17.fra21.fra24" -column "0" -row "7" -columnspan "1" \
        -rowspan "1" -sticky "ew" 
    grid $base.fra21.fra24.ent19 \
        -in ".top17.fra21.fra24" -column "1" -row "7" -columnspan "1" \
        -rowspan "1" -sticky "ew" 
    grid $base.fra21.fra24.lab20 \
        -in ".top17.fra21.fra24" -column "0" -row "8" -columnspan "1" \
        -rowspan "1" -sticky "ew" 
    grid $base.fra21.fra24.ent21 \
        -in ".top17.fra21.fra24" -column "1" -row "8" -columnspan "1" \
        -rowspan "1" -sticky "ew" 
    grid $base.fra21.fra24.but22 \
        -in ".top17.fra21.fra24" -column "2" -row "7" -columnspan "1" \
        -rowspan "1" -sticky "w" 
    grid $base.fra21.fra24.but23 \
        -in ".top17.fra21.fra24" -column "2" -row "8" -columnspan "1" \
        -rowspan "1" -sticky "w" 
    grid $base.fra21.fra24.lab24 \
        -in ".top17.fra21.fra24" -column "0" -row "9" -columnspan "1" \
        -rowspan "1" -sticky "ew" 
    grid $base.fra21.fra24.men25 \
        -in ".top17.fra21.fra24" -column "1" -row "9" -columnspan "2" \
        -rowspan "1" -padx "2" -sticky "ew" 
    grid $base.fra21.fra24.che26 \
        -in ".top17.fra21.fra24" -column "0" -row "10" -columnspan "2" \
        -rowspan "1" -sticky "ew" 
    grid $base.fra21.fra24.sca27 \
        -in ".top17.fra21.fra24" -column "2" -row "10" -columnspan "1" \
        -rowspan "1" -sticky "ew" 
    grid $base.fra21.fra24.lab28 \
        -in ".top17.fra21.fra24" -column "1" -row "10" -columnspan "1" \
        -rowspan "1" -sticky "e" 
    grid $base.fra21.fra26 \
        -in ".top17.fra21" -column "1" -row "3" -columnspan "1" -rowspan "1" \
        -ipadx "2" -ipady "2" -padx "2" -pady "2" -sticky "nesw" 
    grid columnconf $base.fra21.fra26 0 -weight 1
    grid rowconf $base.fra21.fra26 0 -weight 1
    grid $base.fra21.fra26.01 \
        -in ".top17.fra21.fra26" -column "0" -row "0" -columnspan "1" \
        -rowspan "1" -sticky "ew" 
    grid $base.fra21.fra26.02 \
        -in ".top17.fra21.fra26" -column "0" -row "1" -columnspan "1" \
        -rowspan "1" -sticky "ew" 
    grid $base.fra21.fra26.03 \
        -in ".top17.fra21.fra26" -column "3" -row "0" -columnspan "1" \
        -rowspan "1" -sticky "ew" 
    grid $base.fra21.fra48 \
        -in ".top17.fra21" -column "0" -row "0" -columnspan "2" -rowspan "1" \
        -ipadx "2" -ipady "2" -padx "2" -pady "2" -sticky "nesw" 
    grid columnconf $base.fra21.fra48 1 -weight 1
    grid columnconf $base.fra21.fra48 2 -weight 1
    grid columnconf $base.fra21.fra48 3 -weight 1
    grid rowconf $base.fra21.fra48 1 -weight 1
    grid $base.fra21.fra48.cpd18 \
        -in ".top17.fra21.fra48" -column "2" -row "0" -columnspan "2" \
        -rowspan "2" -sticky "nesw" 
    grid columnconf $base.fra21.fra48.cpd18 0 -weight 1
    grid rowconf $base.fra21.fra48.cpd18 0 -weight 1
    grid $base.fra21.fra48.cpd18.01 \
        -in ".top17.fra21.fra48.cpd18" -column "0" -row "0" -columnspan "1" \
        -rowspan "1" -sticky "nesw" 
    grid $base.fra21.fra48.cpd18.02 \
        -in ".top17.fra21.fra48.cpd18" -column "0" -row "1" -columnspan "1" \
        -rowspan "1" -sticky "ew" 
    grid $base.fra21.fra48.cpd18.03 \
        -in ".top17.fra21.fra48.cpd18" -column "1" -row "0" -columnspan "1" \
        -rowspan "1" -sticky "ns" 
    grid $base.fra21.fra48.fra19 \
        -in ".top17.fra21.fra48" -column "0" -row "0" -columnspan "1" \
        -rowspan "1" -ipadx "2" -ipady "2" -padx "2" -pady "2" -sticky "new" 
    grid columnconf $base.fra21.fra48.fra19 0 -weight 1
    grid $base.fra21.fra48.fra19.ent20 \
        -in ".top17.fra21.fra48.fra19" -column "0" -row "1" -columnspan "2" \
        -rowspan "1" -sticky "new" 
    grid $base.fra21.fra48.fra19.lab21 \
        -in ".top17.fra21.fra48.fra19" -column "0" -row "0" -columnspan "1" \
        -rowspan "1" -sticky "ew" 
    grid $base.fra21.fra48.fra40 \
        -in ".top17.fra21.fra48" -column "1" -row "0" -columnspan "1" \
        -rowspan "1" -ipadx "2" -ipady "2" -padx "2" -pady "2" -sticky "n" 
    grid $base.fra21.fra48.fra40.but41 \
        -in ".top17.fra21.fra48.fra40" -column "0" -row "0" -columnspan "1" \
        -rowspan "1" -sticky "ew" 
    grid $base.fra21.fra48.fra40.but42 \
        -in ".top17.fra21.fra48.fra40" -column "0" -row "1" -columnspan "1" \
        -rowspan "1" -sticky "ew" 
    grid $base.fra21.fra48.fra40.but43 \
        -in ".top17.fra21.fra48.fra40" -column "1" -row "0" -columnspan "1" \
        -rowspan "1" -sticky "ew" 
    grid $base.fra21.fra48.fra40.but44 \
        -in ".top17.fra21.fra48.fra40" -column "1" -row "1" -columnspan "1" \
        -rowspan "1" -sticky "ew" 
    grid $base.fra21.fra35 \
        -in ".top17.fra21" -column "0" -row "3" -columnspan "1" -rowspan "1" \
        -ipadx "2" -ipady "2" -padx "2" -pady "2" -sticky "nesw" 
    grid columnconf $base.fra21.fra35 0 -weight 1
    grid columnconf $base.fra21.fra35 1 -weight 1
    grid rowconf $base.fra21.fra35 0 -weight 1
    grid rowconf $base.fra21.fra35 1 -weight 1
    grid $base.fra21.fra35.rad36 \
        -in ".top17.fra21.fra35" -column "0" -row "1" -columnspan "1" \
        -rowspan "1" -sticky "ew" 
    grid $base.fra21.fra35.rad37 \
        -in ".top17.fra21.fra35" -column "1" -row "1" -columnspan "1" \
        -rowspan "1" -sticky "ew" 
    grid $base.fra21.fra35.rad38 \
        -in ".top17.fra21.fra35" -column "0" -row "0" -columnspan "1" \
        -rowspan "1" -sticky "ew" 
    grid $base.fra21.fra35.rad39 \
        -in ".top17.fra21.fra35" -column "1" -row "0" -columnspan "1" \
        -rowspan "1" -sticky "ew" 
    grid $base.fra30 \
        -in ".top17" -column "0" -row "0" -columnspan "1" -rowspan "1" \
        -padx "2" -pady "2" -sticky "ew" 
    grid columnconf $base.fra30 0 -weight 1
    grid columnconf $base.fra30 1 -weight 1
    grid $base.fra30.fra19 \
        -in ".top17.fra30" -column "0" -row "0" -columnspan "1" -rowspan "1" \
        -sticky "w" 
    grid $base.fra30.fra19.but20 \
        -in ".top17.fra30.fra19" -column "0" -row "0" -columnspan "1" \
        -rowspan "1" 
    grid $base.fra30.fra19.but21 \
        -in ".top17.fra30.fra19" -column "1" -row "0" -columnspan "1" \
        -rowspan "1" 
    grid $base.fra30.fra19.but22 \
        -in ".top17.fra30.fra19" -column "2" -row "0" -columnspan "1" \
        -rowspan "1" 
    grid $base.fra30.fra19.but23 \
        -in ".top17.fra30.fra19" -column "3" -row "0" -columnspan "1" \
        -rowspan "1" 
    grid $base.fra30.fra19.but24 \
        -in ".top17.fra30.fra19" -column "4" -row "0" -columnspan "1" \
        -rowspan "1" 
    grid $base.fra30.fra19.but25 \
        -in ".top17.fra30.fra19" -column "5" -row "0" -columnspan "1" \
        -rowspan "1" 
    grid $base.fra30.but26 \
        -in ".top17.fra30" -column "1" -row "0" -columnspan "1" -rowspan "1" \
        -sticky "e" 

    grid columnconf $base.fra35 0 -weight 1

    grid columnconf $base.fra35 1 -weight 1
    grid rowconf $base.fra35 1 -weight 1

    grid $base.fra35.fra17 \
        -in ".top17.fra35" -column "0" -row "1" -columnspan "1" -rowspan "1" \
        -ipadx "2" -ipady "2" -padx "2" -pady "2" -sticky "nesw" 
    grid columnconf $base.fra35.fra17 0 -weight 1
    grid rowconf $base.fra35.fra17 5 -weight 1
    grid rowconf $base.fra35.fra17 7 -weight 1
    grid $base.fra35.fra17.cpd29 \
        -in ".top17.fra35.fra17" -column "0" -row "5" -columnspan "2" \
        -rowspan "1" -sticky "nesw" 
    grid columnconf $base.fra35.fra17.cpd29 0 -weight 1
    grid rowconf $base.fra35.fra17.cpd29 0 -weight 1
    grid $base.fra35.fra17.cpd29.01 \
        -in ".top17.fra35.fra17.cpd29" -column "0" -row "0" -columnspan "1" \
        -rowspan "1" -sticky "nesw" 
    grid $base.fra35.fra17.cpd29.02 \
        -in ".top17.fra35.fra17.cpd29" -column "0" -row "1" -columnspan "1" \
        -rowspan "1" -sticky "ew" 
    grid $base.fra35.fra17.cpd29.03 \
        -in ".top17.fra35.fra17.cpd29" -column "1" -row "0" -columnspan "1" \
        -rowspan "1" -sticky "ns" 
    grid $base.fra35.fra17.lab39 \
        -in ".top17.fra35.fra17" -column "0" -row "3" -columnspan "2" \
        -rowspan "1" -sticky "ew" 
    grid $base.fra35.fra17.fra17 \
        -in ".top17.fra35.fra17" -column "0" -row "7" -columnspan "2" \
        -rowspan "1" -sticky "nesw" 
    grid columnconf $base.fra35.fra17.fra17 0 -weight 1
    grid rowconf $base.fra35.fra17.fra17 0 -weight 1
    grid $base.fra35.fra17.fra17.01 \
        -in ".top17.fra35.fra17.fra17" -column "0" -row "0" -columnspan "1" \
        -rowspan "1" -sticky "nesw" 
    grid $base.fra35.fra17.fra17.02 \
        -in ".top17.fra35.fra17.fra17" -column "0" -row "1" -columnspan "1" \
        -rowspan "1" -sticky "ew" 
    grid $base.fra35.fra17.fra17.03 \
        -in ".top17.fra35.fra17.fra17" -column "1" -row "0" -columnspan "1" \
        -rowspan "1" -sticky "ns" 
    grid $base.fra35.fra17.lab19 \
        -in ".top17.fra35.fra17" -column "0" -row "6" -columnspan "2" \
        -rowspan "1" -sticky "ew" 
    grid $base.fra35.fra17.ent23 \
        -in ".top17.fra35.fra17" -column "0" -row "8" -columnspan "1" \
        -rowspan "1" -sticky "ew" 
    grid $base.fra35.fra17.but24 \
        -in ".top17.fra35.fra17" -column "1" -row "8" -columnspan "1" \
        -rowspan "1" 
    grid $base.fra35.fra17.lab29 \
        -in ".top17.fra35.fra17" -column "0" -row "0" -columnspan "2" \
        -rowspan "1" -padx "2" -pady "2" -sticky "new" 
    grid $base.fra35.fra17.fra33 \
        -in ".top17.fra35.fra17" -column "0" -row "1" -columnspan "2" \
        -rowspan "1" -sticky "ew" 
    grid columnconf $base.fra35.fra17.fra33 0 -weight 1
    grid columnconf $base.fra35.fra17.fra33 1 -weight 1
    grid $base.fra35.fra17.fra33.but34 \
        -in ".top17.fra35.fra17.fra33" -column "0" -row "0" -columnspan "1" \
        -rowspan "1" 
    grid $base.fra35.fra17.fra33.but35 \
        -in ".top17.fra35.fra17.fra33" -column "1" -row "0" -columnspan "1" \
        -rowspan "1" 
    grid $base.fra35.fra20 \
        -in ".top17.fra35" -column "1" -row "1" -columnspan "1" -rowspan "1" \
        -ipadx "2" -ipady "2" -padx "2" -pady "2" -sticky "nesw" 
    grid columnconf $base.fra35.fra20 0 -weight 1
    grid rowconf $base.fra35.fra20 4 -weight 1
    grid $base.fra35.fra20.but22 \
        -in ".top17.fra35.fra20" -column "0" -row "1" -columnspan "2" \
        -rowspan "1" -sticky "ew" 
    grid $base.fra35.fra20.but26 \
        -in ".top17.fra35.fra20" -column "0" -row "5" -columnspan "2" \
        -rowspan "1" -sticky "ew" 
    grid $base.fra35.fra20.che28 \
        -in ".top17.fra35.fra20" -column "0" -row "2" -columnspan "2" \
        -rowspan "1" -sticky "ew" 
    grid $base.fra35.fra20.fra22 \
        -in ".top17.fra35.fra20" -column "0" -row "4" -columnspan "2" \
        -rowspan "1" -sticky "nesw" 
    grid columnconf $base.fra35.fra20.fra22 0 -weight 1
    grid rowconf $base.fra35.fra20.fra22 0 -weight 1
    grid $base.fra35.fra20.fra22.01 \
        -in ".top17.fra35.fra20.fra22" -column "0" -row "0" -columnspan "1" \
        -rowspan "1" -sticky "nesw" 
    grid $base.fra35.fra20.fra22.02 \
        -in ".top17.fra35.fra20.fra22" -column "0" -row "1" -columnspan "1" \
        -rowspan "1" -sticky "ew" 
    grid $base.fra35.fra20.fra22.03 \
        -in ".top17.fra35.fra20.fra22" -column "1" -row "0" -columnspan "1" \
        -rowspan "1" -sticky "ns" 
    grid $base.fra35.fra20.but20 \
        -in ".top17.fra35.fra20" -column "0" -row "3" -columnspan "1" \
        -rowspan "1" -sticky "ew" 
    grid $base.fra35.fra20.but17 \
        -in ".top17.fra35.fra20" -column "0" -row "6" -columnspan "1" \
        -rowspan "1" -sticky "ew" 
    grid $base.fra35.fra20.lab28 \
        -in ".top17.fra35.fra20" -column "0" -row "0" -columnspan "2" \
        -rowspan "1" -padx "2" -pady "2" -sticky "new" 
    grid $base.fra35.fra20.but18 \
        -in ".top17.fra35.fra20" -column "1" -row "3" -columnspan "1" \
        -rowspan "1" 
    grid $base.fra35.fra20.but30 \
        -in ".top17.fra35.fra20" -column "1" -row "6" -columnspan "1" \
        -rowspan "1" -sticky "ew" 
    grid $base.fra35.fra40 \
        -in ".top17.fra35" -column "0" -row "0" -columnspan "1" -rowspan "1" \
        -ipadx "2" -ipady "2" -padx "2" -pady "2" -sticky "nesw" 
    grid columnconf $base.fra35.fra40 0 -weight 1
    grid rowconf $base.fra35.fra40 4 -weight 1
    grid $base.fra35.fra40.but43 \
        -in ".top17.fra35.fra40" -column "0" -row "3" -columnspan "2" \
        -rowspan "1" 
    grid $base.fra35.fra40.lab30 \
        -in ".top17.fra35.fra40" -column "0" -row "0" -columnspan "2" \
        -rowspan "1" -padx "2" -pady "2" -sticky "new" 
    grid $base.fra35.fra47 \
        -in ".top17.fra35" -column "1" -row "0" -columnspan "1" -rowspan "1" \
        -ipadx "2" -ipady "2" -padx "2" -pady "2" -sticky "nesw" 
    grid columnconf $base.fra35.fra47 0 -weight 1
    grid rowconf $base.fra35.fra47 1 -weight 1
    grid $base.fra35.fra47.lab48 \
        -in ".top17.fra35.fra47" -column "0" -row "0" -columnspan "1" \
        -rowspan "1" -padx "2" -pady "2" -sticky "ew" 
    grid $base.fra35.fra47.but49 \
        -in ".top17.fra35.fra47" -column "0" -row "1" -columnspan "1" \
        -rowspan "1"

    grid columnconf $base.fra44 0 -weight 1
    grid rowconf $base.fra44 2 -weight 1    
 
    grid $base.fra44.fra27 \
        -in ".top17.fra44" -column "0" -row "3" -columnspan "2" -rowspan "1" \
        -ipadx "2" -ipady "2" -padx "4" -pady "2" -sticky "esw" 
    grid $base.fra44.fra27.but28 \
        -in ".top17.fra44.fra27" -column "0" -row "2" -columnspan "2" \
        -rowspan "1" 
    grid $base.fra44.fra19 \
        -in ".top17.fra44" -column "0" -row "2" -columnspan "2" -rowspan "1" \
        -ipady "10" -padx "2" -pady "10" -sticky "new" 
    grid columnconf $base.fra44.fra19 0 -weight 1
    grid columnconf $base.fra44.fra19 1 -weight 1
    grid $base.fra44.fra19.fra20 \
        -in ".top17.fra44.fra19" -column "0" -row "0" -columnspan "1" \
        -rowspan "1" -ipadx "2" -ipady "2" -padx "2" -sticky "ew" 
    grid columnconf $base.fra44.fra19.fra20 0 -weight 1
    grid $base.fra44.fra19.fra20.lab21 \
        -in ".top17.fra44.fra19.fra20" -column "0" -row "0" -columnspan "1" \
        -rowspan "1" -sticky "ew" 
    grid $base.fra44.fra19.fra20.ent22 \
        -in ".top17.fra44.fra19.fra20" -column "0" -row "1" -columnspan "2" \
        -rowspan "1" -sticky "ew" 
    grid $base.fra44.fra19.fra20.but23 \
        -in ".top17.fra44.fra19.fra20" -column "1" -row "0" -columnspan "1" \
        -rowspan "1" 
    grid $base.fra44.fra19.fra26 \
        -in ".top17.fra44.fra19" -column "0" -row "1" -columnspan "1" \
        -rowspan "1" -ipadx "2" -ipady "2" -padx "2" -sticky "ew" 
    grid columnconf $base.fra44.fra19.fra26 0 -weight 1
    grid $base.fra44.fra19.fra26.01 \
        -in ".top17.fra44.fra19.fra26" -column "0" -row "0" -columnspan "1" \
        -rowspan "1" -sticky "ew" 
    grid $base.fra44.fra19.fra26.02 \
        -in ".top17.fra44.fra19.fra26" -column "0" -row "1" -columnspan "2" \
        -rowspan "1" -sticky "ew" 
    grid $base.fra44.fra19.fra26.03 \
        -in ".top17.fra44.fra19.fra26" -column "1" -row "0" -columnspan "1" \
        -rowspan "1" 
    grid $base.fra44.fra19.fra27 \
        -in ".top17.fra44.fra19" -column "0" -row "2" -columnspan "1" \
        -rowspan "1" -ipadx "2" -ipady "2" -padx "2" -sticky "ew" 
    grid columnconf $base.fra44.fra19.fra27 0 -weight 1
    grid $base.fra44.fra19.fra27.01 \
        -in ".top17.fra44.fra19.fra27" -column "0" -row "0" -columnspan "1" \
        -rowspan "1" -sticky "ew" 
    grid $base.fra44.fra19.fra27.02 \
        -in ".top17.fra44.fra19.fra27" -column "0" -row "1" -columnspan "2" \
        -rowspan "1" -sticky "ew" 
    grid $base.fra44.fra19.fra27.03 \
        -in ".top17.fra44.fra19.fra27" -column "1" -row "0" -columnspan "1" \
        -rowspan "1" 
    grid $base.fra44.fra19.fra28 \
        -in ".top17.fra44.fra19" -column "1" -row "1" -columnspan "1" \
        -rowspan "1" -ipadx "2" -ipady "2" -padx "2" -sticky "ew" 
    grid columnconf $base.fra44.fra19.fra28 0 -weight 1
    grid $base.fra44.fra19.fra28.01 \
        -in ".top17.fra44.fra19.fra28" -column "0" -row "0" -columnspan "1" \
        -rowspan "1" -sticky "ew" 
    grid $base.fra44.fra19.fra28.02 \
        -in ".top17.fra44.fra19.fra28" -column "0" -row "1" -columnspan "2" \
        -rowspan "1" -sticky "ew" 
    grid $base.fra44.fra19.fra28.03 \
        -in ".top17.fra44.fra19.fra28" -column "1" -row "0" -columnspan "1" \
        -rowspan "1" 
    grid $base.fra44.fra19.fra29 \
        -in ".top17.fra44.fra19" -column "1" -row "0" -columnspan "1" \
        -rowspan "1" -ipadx "2" -ipady "2" -padx "2" -sticky "ew" 
    grid columnconf $base.fra44.fra19.fra29 0 -weight 1
    grid $base.fra44.fra19.fra29.01 \
        -in ".top17.fra44.fra19.fra29" -column "0" -row "0" -columnspan "1" \
        -rowspan "1" -sticky "ew" 
    grid $base.fra44.fra19.fra29.02 \
        -in ".top17.fra44.fra19.fra29" -column "0" -row "1" -columnspan "2" \
        -rowspan "1" -sticky "ew" 
    grid $base.fra44.fra19.fra29.03 \
        -in ".top17.fra44.fra19.fra29" -column "1" -row "0" -columnspan "1" \
        -rowspan "1" 
    grid $base.fra44.fra19.fra32 \
        -in ".top17.fra44.fra19" -column "0" -row "3" -columnspan "1" \
        -rowspan "1" -ipadx "2" -ipady "2" -padx "2" -sticky "ew" 
    grid columnconf $base.fra44.fra19.fra32 0 -weight 1
    grid $base.fra44.fra19.fra32.01 \
        -in ".top17.fra44.fra19.fra32" -column "0" -row "0" -columnspan "1" \
        -rowspan "1" -sticky "ew" 
    grid $base.fra44.fra19.fra32.02 \
        -in ".top17.fra44.fra19.fra32" -column "0" -row "2" -columnspan "1" \
        -rowspan "1" -sticky "ew" 
    grid $base.fra44.fra19.fra32.03 \
        -in ".top17.fra44.fra19.fra32" -column "1" -row "2" -columnspan "1" \
        -rowspan "1" -sticky "ew" 
    grid $base.fra44.fra19.fra32.fra38 \
        -in ".top17.fra44.fra19.fra32" -column "0" -row "1" -columnspan "2" \
        -rowspan "1" -sticky "ew" 
    grid columnconf $base.fra44.fra19.fra32.fra38 0 -weight 1
    grid columnconf $base.fra44.fra19.fra32.fra38 1 -weight 1
    grid $base.fra44.fra19.fra32.fra38.lab39 \
        -in ".top17.fra44.fra19.fra32.fra38" -column "0" -row "1" \
        -columnspan "1" -rowspan "1" -sticky "w" 
    grid $base.fra44.fra19.fra32.fra38.men40 \
        -in ".top17.fra44.fra19.fra32.fra38" -column "1" -row "1" \
        -columnspan "1" -rowspan "1" -padx "2" -sticky "ew" 
    grid $base.fra44.fra19.fra30 \
        -in ".top17.fra44.fra19" -column "1" -row "2" -columnspan "1" \
        -rowspan "1" -ipadx "2" -ipady "2" -padx "2" -sticky "ew" 
    grid columnconf $base.fra44.fra19.fra30 0 -weight 1
    grid $base.fra44.fra19.fra30.01 \
        -in ".top17.fra44.fra19.fra30" -column "0" -row "0" -columnspan "1" \
        -rowspan "1" -sticky "ew" 
    grid $base.fra44.fra19.fra30.02 \
        -in ".top17.fra44.fra19.fra30" -column "0" -row "1" -columnspan "2" \
        -rowspan "1" -sticky "ew" 
    grid $base.fra44.fra19.fra30.03 \
        -in ".top17.fra44.fra19.fra30" -column "1" -row "0" -columnspan "1" \
        -rowspan "1" 
    grid $base.fra44.fra19.fra18 \
        -in ".top17.fra44.fra19" -column "1" -row "3" -columnspan "1" \
        -rowspan "1" -ipadx "2" -ipady "2" -padx "2" -sticky "ew" 
    grid columnconf $base.fra44.fra19.fra18 0 -weight 1
    grid $base.fra44.fra19.fra18.01 \
        -in ".top17.fra44.fra19.fra18" -column "0" -row "0" -columnspan "1" \
        -rowspan "1" -sticky "ew" 
    grid $base.fra44.fra19.fra18.02 \
        -in ".top17.fra44.fra19.fra18" -column "0" -row "1" -columnspan "2" \
        -rowspan "1" -sticky "ew" 
    grid $base.fra44.fra19.fra18.03 \
        -in ".top17.fra44.fra19.fra18" -column "1" -row "0" -columnspan "1" \
        -rowspan "1" 
    grid $base.fra44.lab30 \
        -in ".top17.fra44" -column "0" -row "0" -columnspan "2" -rowspan "1" \
        -padx "2" -pady "2" -sticky "new" 
    grid $base.fra44.men31 \
        -in ".top17.fra44" -column "1" -row "1" -columnspan "1" -rowspan "1" \
        -sticky "w" 
    grid $base.fra44.lab32 \
        -in ".top17.fra44" -column "0" -row "1" -columnspan "1" -rowspan "1" \
        -padx "2" -pady "2" -sticky "ew" 
    grid $base.fra46 \
        -in ".top17" -column "0" -row "1" -columnspan "1" -rowspan "1" \
        -sticky "nesw" 
    grid columnconf $base.fra46 0 -weight 1
    grid rowconf $base.fra46 0 -weight 1
    grid rowconf $base.fra46 1 -weight 1
    grid $base.fra46.fra24 \
        -in ".top17.fra46" -column "0" -row "0" -columnspan "1" -rowspan "1" \
        -ipadx "2" -ipady "2" -padx "2" -pady "2" -sticky "nesw" 
    grid columnconf $base.fra46.fra24 0 -weight 1
    grid rowconf $base.fra46.fra24 1 -weight 1
    grid $base.fra46.fra24.fra25 \
        -in ".top17.fra46.fra24" -column "0" -row "1" -columnspan "1" \
        -rowspan "1" -padx "2" -pady "2" -sticky "nesw" 
    grid columnconf $base.fra46.fra24.fra25 0 -weight 1
    grid rowconf $base.fra46.fra24.fra25 0 -weight 1
    grid $base.fra46.fra24.fra25.01 \
        -in ".top17.fra46.fra24.fra25" -column "0" -row "0" -columnspan "1" \
        -rowspan "1" -sticky "nesw" 
    grid $base.fra46.fra24.fra25.02 \
        -in ".top17.fra46.fra24.fra25" -column "0" -row "1" -columnspan "1" \
        -rowspan "1" -sticky "ew" 
    grid $base.fra46.fra24.fra25.03 \
        -in ".top17.fra46.fra24.fra25" -column "1" -row "0" -columnspan "1" \
        -rowspan "1" -sticky "ns" 
    grid $base.fra46.fra24.fra28 \
        -in ".top17.fra46.fra24" -column "1" -row "1" -columnspan "1" \
        -rowspan "1" -ipadx "2" -ipady "2" -padx "2" -pady "2" -sticky "ns" 
    grid rowconf $base.fra46.fra24.fra28 4 -weight 1
    grid $base.fra46.fra24.fra28.but39 \
        -in ".top17.fra46.fra24.fra28" -column "0" -row "3" -columnspan "1" \
        -rowspan "1" -sticky "ew" 
    grid $base.fra46.fra24.fra28.fra23 \
        -in ".top17.fra46.fra24.fra28" -column "0" -row "4" -columnspan "1" \
        -rowspan "1" -sticky "esw" 
    grid columnconf $base.fra46.fra24.fra28.fra23 0 -weight 1
    grid $base.fra46.fra24.fra28.fra23.but24 \
        -in ".top17.fra46.fra24.fra28.fra23" -column "0" -row "0" \
        -columnspan "1" -rowspan "1" -sticky "ew" 
    grid $base.fra46.fra24.fra28.fra23.but25 \
        -in ".top17.fra46.fra24.fra28.fra23" -column "0" -row "1" \
        -columnspan "1" -rowspan "1" -sticky "ew" 
    grid $base.fra46.fra24.fra28.fra23.but26 \
        -in ".top17.fra46.fra24.fra28.fra23" -column "0" -row "2" \
        -columnspan "1" -rowspan "1" -sticky "ew" 
    grid $base.fra46.fra24.lab33 \
        -in ".top17.fra46.fra24" -column "0" -row "0" -columnspan "2" \
        -rowspan "1" -sticky "ew" 
    grid $base.fra46.fra26 \
        -in ".top17.fra46" -column "0" -row "1" -columnspan "1" -rowspan "1" \
        -ipadx "2" -ipady "2" -padx "2" -pady "2" -sticky "nesw" 
    grid columnconf $base.fra46.fra26 0 -weight 1
    grid rowconf $base.fra46.fra26 1 -weight 1
    grid $base.fra46.fra26.fra27 \
        -in ".top17.fra46.fra26" -column "0" -row "1" -columnspan "1" \
        -rowspan "1" -padx "2" -pady "2" -sticky "nesw" 
    grid columnconf $base.fra46.fra26.fra27 0 -weight 1
    grid rowconf $base.fra46.fra26.fra27 0 -weight 1
    grid $base.fra46.fra26.fra27.01 \
        -in ".top17.fra46.fra26.fra27" -column "0" -row "0" -columnspan "1" \
        -rowspan "1" -sticky "nesw" 
    grid $base.fra46.fra26.fra27.02 \
        -in ".top17.fra46.fra26.fra27" -column "0" -row "1" -columnspan "1" \
        -rowspan "1" -sticky "ew" 
    grid $base.fra46.fra26.fra27.03 \
        -in ".top17.fra46.fra26.fra27" -column "1" -row "0" -columnspan "1" \
        -rowspan "1" -sticky "ns" 
    grid $base.fra46.fra26.fra29 \
        -in ".top17.fra46.fra26" -column "1" -row "1" -columnspan "1" \
        -rowspan "1" -ipadx "2" -ipady "2" -padx "2" -pady "2" -sticky "ns" 
    grid rowconf $base.fra46.fra26.fra29 4 -weight 1
    grid $base.fra46.fra26.fra29.but38 \
        -in ".top17.fra46.fra26.fra29" -column "0" -row "3" -columnspan "1" \
        -rowspan "1" -sticky "ew" 
    grid $base.fra46.fra26.fra29.fra28 \
        -in ".top17.fra46.fra26.fra29" -column "0" -row "4" -columnspan "1" \
        -rowspan "1" -sticky "esw" 
    grid columnconf $base.fra46.fra26.fra29.fra28 0 -weight 1
    grid $base.fra46.fra26.fra29.fra28.but29 \
        -in ".top17.fra46.fra26.fra29.fra28" -column "0" -row "0" \
        -columnspan "1" -rowspan "1" -sticky "ew" 
    grid $base.fra46.fra26.fra29.fra28.but30 \
        -in ".top17.fra46.fra26.fra29.fra28" -column "0" -row "1" \
        -columnspan "1" -rowspan "1" -sticky "ew" 
    grid $base.fra46.fra26.fra29.fra28.but31 \
        -in ".top17.fra46.fra26.fra29.fra28" -column "0" -row "2" \
        -columnspan "1" -rowspan "1" -sticky "ew" 
    grid $base.fra46.fra26.lab34 \
        -in ".top17.fra46.fra26" -column "0" -row "0" -columnspan "2" \
        -rowspan "1" -sticky "ew" 
    grid $base.fra17 \
        -in ".top17" -column "0" -row "2" -columnspan "1" -rowspan "1" \
        -padx "2" -pady "2" -sticky "ew" 
    grid columnconf $base.fra17 0 -weight 1
    grid columnconf $base.fra17 1 -weight 1
    grid $base.fra17.lab18 \
        -in ".top17.fra17" -column "0" -row "0" -columnspan "1" -rowspan "1" \
        -sticky "ew" 
    grid $base.fra17.lab19 \
        -in ".top17.fra17" -column "1" -row "0" -columnspan "1" -rowspan "1" \
        -sticky "ew"

    grid columnconf $base.fra27 0 -weight 1
    grid rowconf $base.fra27 0 -weight 1
 
    grid $base.fra27.fra30 \
        -in ".top17.fra27" -column "0" -row "0" -columnspan "1" -rowspan "1" \
        -ipadx "2" -ipady "2" -padx "2" -pady "2" -sticky "nesw" 
    grid columnconf $base.fra27.fra30 0 -weight 1
    grid rowconf $base.fra27.fra30 1 -weight 1
    grid $base.fra27.fra30.cpd31 \
        -in ".top17.fra27.fra30" -column "0" -row "1" -columnspan "1" \
        -rowspan "1" -sticky "nesw" 
    grid columnconf $base.fra27.fra30.cpd31 0 -weight 1
    grid rowconf $base.fra27.fra30.cpd31 0 -weight 1
    grid $base.fra27.fra30.cpd31.01 \
        -in ".top17.fra27.fra30.cpd31" -column "0" -row "0" -columnspan "1" \
        -rowspan "1" -sticky "nesw" 
    grid $base.fra27.fra30.cpd31.02 \
        -in ".top17.fra27.fra30.cpd31" -column "0" -row "1" -columnspan "1" \
        -rowspan "1" -sticky "ew" 
    grid $base.fra27.fra30.cpd31.03 \
        -in ".top17.fra27.fra30.cpd31" -column "1" -row "0" -columnspan "1" \
        -rowspan "1" -sticky "ns" 
    grid $base.fra27.fra30.cpd32 \
        -in ".top17.fra27.fra30" -column "1" -row "1" -columnspan "1" \
        -rowspan "1" -sticky "nesw" 
    grid columnconf $base.fra27.fra30.cpd32 0 -weight 1
    grid rowconf $base.fra27.fra30.cpd32 0 -weight 1
    grid $base.fra27.fra30.cpd32.01 \
        -in ".top17.fra27.fra30.cpd32" -column "0" -row "0" -columnspan "1" \
        -rowspan "1" -sticky "nesw" 
    grid $base.fra27.fra30.cpd32.02 \
        -in ".top17.fra27.fra30.cpd32" -column "0" -row "1" -columnspan "1" \
        -rowspan "1" -sticky "ew" 
    grid $base.fra27.fra30.cpd32.03 \
        -in ".top17.fra27.fra30.cpd32" -column "1" -row "0" -columnspan "1" \
        -rowspan "1" -sticky "ns" 
    grid $base.fra27.fra30.lab51 \
        -in ".top17.fra27.fra30" -column "0" -row "0" -columnspan "1" \
        -rowspan "1" -padx "2" -pady "2" -sticky "ew" 
    grid $base.fra27.fra30.lab52 \
        -in ".top17.fra27.fra30" -column "1" -row "0" -columnspan "1" \
        -rowspan "1" -padx "2" -pady "2" -sticky "ew" 
    grid $base.fra27.fra30.fra18 \
        -in ".top17.fra27.fra30" -column "0" -row "4" -columnspan "2" \
        -rowspan "1" -sticky "ew" 
    grid columnconf $base.fra27.fra30.fra18 0 -weight 1
    grid $base.fra27.fra30.fra18.but19 \
        -in ".top17.fra27.fra30.fra18" -column "0" -row "0" -columnspan "1" \
        -rowspan "1" -padx "4" -pady "4" -sticky "ew" 
    grid $base.fra27.fra30.fra18.but20 \
        -in ".top17.fra27.fra30.fra18" -column "1" -row "0" -columnspan "1" \
        -rowspan "1" 
    grid $base.fra27.fra17 \
        -in ".top17.fra27" -column "0" -row "1" -columnspan "1" -rowspan "1" \
        -ipadx "2" -ipady "2" -padx "2" -pady "2" -sticky "ew" 
    grid $base.fra27.fra17.lab18 \
        -in ".top17.fra27.fra17" -column "0" -row "0" -columnspan "1" \
        -rowspan "1" -padx "2" -sticky "ew" 
    grid $base.fra27.fra17.che20 \
        -in ".top17.fra27.fra17" -column "0" -row "1" -columnspan "1" \
        -rowspan "1" -padx "2" -sticky "ew" 
    grid $base.fra27.fra17.che21 \
        -in ".top17.fra27.fra17" -column "0" -row "2" -columnspan "1" \
        -rowspan "1" -padx "2" -sticky "ew" 
    grid $base.fra27.fra17.che22 \
        -in ".top17.fra27.fra17" -column "0" -row "3" -columnspan "1" \
        -rowspan "1" -padx "2" -sticky "ew" 
    grid $base.fra27.fra17.che23 \
        -in ".top17.fra27.fra17" -column "0" -row "4" -columnspan "1" \
        -rowspan "1" -padx "2" -sticky "ew" 
    grid $base.fra27.fra17.but17 \
        -in ".top17.fra27.fra17" -column "1" -row "6" -columnspan "2" \
        -rowspan "1" -sticky "ew" 
    grid $base.fra27.fra17.che17 \
        -in ".top17.fra27.fra17" -column "1" -row "1" -columnspan "2" \
        -rowspan "1" -padx "2" -sticky "ew" 
    grid $base.fra27.fra17.lab17 \
        -in ".top17.fra27.fra17" -column "1" -row "0" -columnspan "2" \
        -rowspan "1" -padx "2" -sticky "ew" 
    grid $base.fra27.fra17.che18 \
        -in ".top17.fra27.fra17" -column "1" -row "2" -columnspan "2" \
        -rowspan "1" -padx "2" -sticky "ew" 
    grid $base.fra27.fra17.che19 \
        -in ".top17.fra27.fra17" -column "1" -row "3" -columnspan "2" \
        -rowspan "1" -padx "2" -sticky "ew" 
    grid $base.fra27.fra17.rad20 \
        -in ".top17.fra27.fra17" -column "1" -row "5" -columnspan "1" \
        -rowspan "1" -sticky "ew" 
    grid $base.fra27.fra17.rad21 \
        -in ".top17.fra27.fra17" -column "2" -row "5" -columnspan "1" \
        -rowspan "1" -sticky "w" 
    grid $base.fra27.fra17.lab23 \
        -in ".top17.fra27.fra17" -column "1" -row "4" -columnspan "2" \
        -rowspan "1" -padx "2" -sticky "ew" 
    grid $base.fra27.fra17.fra18 \
        -in ".top17.fra27.fra17" -column "0" -row "5" -columnspan "1" \
        -rowspan "1" -padx "2" -sticky "w" 
    grid $base.fra27.fra17.fra18.ent19 \
        -in ".top17.fra27.fra17.fra18" -column "1" -row "0" -columnspan "1" \
        -rowspan "1" -padx "4" 
    grid $base.fra27.fra17.fra18.lab20 \
        -in ".top17.fra27.fra17.fra18" -column "0" -row "0" -columnspan "1" \
        -rowspan "1" 
}

proc vTclWindow.top18 {base} {
    if {$base == ""} {
        set base .top18
    }
    if {[winfo exists $base]} {
        wm deiconify $base; return
    }
    ###################
    # CREATING WIDGETS
    ###################
    toplevel $base -class Toplevel  -borderwidth "10" 
    wm focusmodel $base passive
    wm geometry $base 250x150
    wm maxsize $base 1265 994
    wm minsize $base 1 1
    wm overrideredirect $base 0
    wm resizable $base 1 1
    wm title $base "SecPanel - Confirm"
    message $base.mes19  -aspect "300" -padx "5" -pady "2" 
    frame $base.fra20  -borderwidth "1" -height "75" -width "125" 
    button $base.fra20.but21  -command "set questres 1" -padx "9" -pady "3" -relief "groove"  -text "Yes" -bg orange
    button $base.fra20.but22  -command "set questres 0" -padx "9" -pady "3" -relief "groove"  -text "No"  -bg IndianRed
    ###################
    # SETTING GEOMETRY
    ###################
    grid columnconf $base 0 -weight 1
    grid rowconf $base 0 -weight 1
    grid $base.mes19  -in ".top18" -column "0" -row "0" -columnspan "1" -rowspan "1"  -sticky "nesw" 
    grid $base.fra20  -in ".top18" -column "0" -row "1" -columnspan "1" -rowspan "1"  -sticky "ew" 
    grid columnconf $base.fra20 0 -weight 1
    grid columnconf $base.fra20 1 -weight 1
    grid $base.fra20.but21  -in ".top18.fra20" -column "0" -row "0" -columnspan "1" -rowspan "1"  -sticky "ew" 
    grid $base.fra20.but22  -in ".top18.fra20" -column "1" -row "0" -columnspan "1" -rowspan "1"  -sticky "ew"
}

proc vTclWindow.top19 {base} {
    if {$base == ""} {
        set base .top19
    }
    if {[winfo exists $base]} {
        wm deiconify $base; return
    }
    ###################
    # CREATING WIDGETS
    ###################
    toplevel $base -class Toplevel  -borderwidth "4" -menu ".top19.m17" 
    wm focusmodel $base passive
    wm geometry $base 666x163
    wm maxsize $base 1009 738
    wm minsize $base 1 1
    wm overrideredirect $base 0
    wm resizable $base 1 1
    wm title $base "SecPanel - View key"
    label $base.lab19  -anchor "w" -borderwidth "1" -text "Fingerprint" 
    label $base.lab39  -anchor "w" -borderwidth "0" -relief "raised"  -text "Bubblebabble digest" 
    button $base.but41  -command "Window destroy .top19" -padx "9" -pady "3" -relief "groove"  -text "Close" -bg IndianRed
    menu $base.m17  -cursor "" -tearoff "0" 
    frame $base.fra22  -borderwidth "2" -height "75" -relief "groove" -width "125" 
    label $base.fra22.lab23  -anchor "w" -borderwidth "0" -relief "raised" 
    label $base.fra22.lab24  -anchor "w" -borderwidth "0" -relief "raised" 
    text $base.tex17  -borderwidth "0" -font "fixed" -height "1" -width "8" 
    text $base.tex18  -borderwidth "0" -cursor "fleur" -font "fixed" -height "1" -width "8" 
    ###################
    # SETTING GEOMETRY
    ###################
    grid columnconf $base 0 -weight 1
    grid rowconf $base 2 -weight 1
    grid rowconf $base 4 -weight 1
    grid $base.lab19  -in ".top19" -column "0" -row "1" -columnspan "2" -rowspan "1"  -sticky "ew" 
    grid $base.lab39  -in ".top19" -column "0" -row "3" -columnspan "2" -rowspan "1"  -sticky "ew" 
    grid $base.but41  -in ".top19" -column "0" -row "5" -columnspan "2" -rowspan "1"  -padx "2" -pady "2" -sticky "e" 
    grid $base.fra22  -in ".top19" -column "0" -row "0" -columnspan "1" -rowspan "1"  -sticky "ew" 
    grid columnconf $base.fra22 0 -weight 1
    grid columnconf $base.fra22 1 -weight 1
    grid $base.fra22.lab23  -in ".top19.fra22" -column "0" -row "0" -columnspan "1" -rowspan "1"  -sticky "ew" 
    grid $base.fra22.lab24  -in ".top19.fra22" -column "1" -row "0" -columnspan "1" -rowspan "1"  -sticky "ew" 
    grid $base.tex17  -in ".top19" -column "0" -row "2" -columnspan "1" -rowspan "1"  -sticky "nesw" 
    grid $base.tex18  -in ".top19" -column "0" -row "4" -columnspan "1" -rowspan "1"  -sticky "nesw"
}

proc vTclWindow.top20 {base} {
    if {$base == ""} {
        set base .top20
    }
    if {[winfo exists $base]} {
        wm deiconify $base; return
    }
    ###################
    # CREATING WIDGETS
    ###################
    toplevel $base -class Toplevel  -borderwidth "4" 
    wm focusmodel $base passive
    wm geometry $base 292x175
    wm maxsize $base 785 570
    wm minsize $base 1 1
    wm overrideredirect $base 0
    wm resizable $base 1 1
    wm title $base "SecPanel - Change Key-Properties"
    frame $base.fra21  -borderwidth "1" -height "75" -width "125" 
    label $base.fra21.lab25  -anchor "w" -borderwidth "1" -text "Old password:" 
    entry $base.fra21.ent26  -borderwidth "1" -show "*" 
    label $base.fra21.lab27  -anchor "w" -borderwidth "1" -text "New password:" 
    label $base.fra21.lab28  -anchor "w" -borderwidth "1" -text "New password:" 
    entry $base.fra21.ent30  -borderwidth "1" -show "*" 
    entry $base.fra21.ent31  -borderwidth "1" -show "*" 
    label $base.fra21.lab19  -anchor "w" -borderwidth "1" -text "Comment:" 
    entry $base.fra21.ent20  -borderwidth "1" -show "*" 
    frame $base.fra22  -borderwidth "2" -height "75" -relief "groove" -width "125" 
    button $base.fra22.but23  -command "Window destroy .top20" -padx "9" -pady "3" -relief "groove"  -text "Cancel" -bg IndianRed
    button $base.fra22.but33  -command "keygen chpwd" -padx "9" -pady "3" -relief "groove"  -text "Save" -bg orange
    label $base.lab35  -anchor "w" -borderwidth "1" -text "Changing properties of" 
    label $base.lab36  -anchor "w" -borderwidth "1" 
    ###################
    # SETTING GEOMETRY
    ###################
    grid columnconf $base 0 -weight 1
    grid rowconf $base 2 -weight 1
    grid $base.fra21  -in ".top20" -column "0" -row "2" -columnspan "1" -rowspan "1"  -ipadx "2" -ipady "2" -padx "2" -pady "2" -sticky "nesw" 
    grid columnconf $base.fra21 1 -weight 1
    grid $base.fra21.lab25  -in ".top20.fra21" -column "0" -row "0" -columnspan "1" -rowspan "1"  -sticky "ew" 
    grid $base.fra21.ent26  -in ".top20.fra21" -column "1" -row "0" -columnspan "1" -rowspan "1"  -padx "2" -sticky "ew" 
    grid $base.fra21.lab27  -in ".top20.fra21" -column "0" -row "1" -columnspan "1" -rowspan "1"  -sticky "ew" 
    grid $base.fra21.lab28  -in ".top20.fra21" -column "0" -row "2" -columnspan "1" -rowspan "1"  -sticky "ew" 
    grid $base.fra21.ent30  -in ".top20.fra21" -column "1" -row "1" -columnspan "1" -rowspan "1"  -padx "2" -sticky "ew" 
    grid $base.fra21.ent31  -in ".top20.fra21" -column "1" -row "2" -columnspan "1" -rowspan "1"  -padx "2" -sticky "ew" 
    grid $base.fra21.lab19  -in ".top20.fra21" -column "0" -row "3" -columnspan "1" -rowspan "1"  -sticky "ew" 
    grid $base.fra21.ent20  -in ".top20.fra21" -column "1" -row "3" -columnspan "1" -rowspan "1"  -padx "2" -sticky "ew" 
    grid $base.fra22  -in ".top20" -column "0" -row "3" -columnspan "1" -rowspan "1"  -ipadx "2" -ipady "2" -padx "2" -pady "2" -sticky "ew" 
    grid $base.fra22.but23  -in ".top20.fra22" -column "0" -row "0" -columnspan "1" -rowspan "1" 
    grid $base.fra22.but33  -in ".top20.fra22" -column "1" -row "0" -columnspan "1" -rowspan "1" 
    grid $base.lab35  -in ".top20" -column "0" -row "0" -columnspan "1" -rowspan "1"  -padx "2" -sticky "ew" 
    grid $base.lab36  -in ".top20" -column "0" -row "1" -columnspan "1" -rowspan "1"  -padx "2" -sticky "ew"
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
    button $base.but25  -command "set userres #####" -padx "9" -pady "3" -relief "groove"  -text "Cancel" -bg IndianRead
    button $base.but26  -command "getuser" -padx "9" -pady "3" -relief "groove"  -text "Connect" -bg orange
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

proc vTclWindow.top22 {base} {
    if {$base == ""} {
        set base .top22
    }
    if {[winfo exists $base]} {
        wm deiconify $base; return
    }
    ###################
    # CREATING WIDGETS
    ###################
    toplevel $base -class Toplevel  -borderwidth "4" 
    wm focusmodel $base passive
    wm geometry $base 250x150
    wm maxsize $base 1265 994
    wm minsize $base 1 1
    wm overrideredirect $base 0
    wm resizable $base 1 1
    wm title $base "SecPanel - Message"
    message $base.mes23  -aspect "300" -padx "5" -pady "2" 
    label $base.lab26  -bitmap "info" -text "label" 
    button $base.but17  -command "Window destroy .top22" -padx "9" -pady "3" -relief "groove"  -text "OK" -bg orange
    ###################
    # SETTING GEOMETRY
    ###################
    grid columnconf $base 1 -weight 1
    grid rowconf $base 0 -weight 1
    grid $base.mes23  -in ".top22" -column "1" -row "0" -columnspan "1" -rowspan "1"  -padx "4" -pady "4" -sticky "nesw" 
    grid $base.lab26  -in ".top22" -column "0" -row "0" -columnspan "1" -rowspan "1"  -ipadx "4" -ipady "4" -padx "4" -pady "4" -sticky "nesw" 
    grid $base.but17  -in ".top22" -column "0" -row "1" -columnspan "2" -rowspan "1"  -padx "2" -pady "2" -sticky "ew"
}

proc vTclWindow.top23 {base} {
    if {$base == ""} {
        set base .top23
    }
    if {[winfo exists $base]} {
        wm deiconify $base; return
    }
    ###################
    # CREATING WIDGETS
    ###################
    toplevel $base -class Toplevel  -borderwidth "4" 
    wm focusmodel $base passive
    wm geometry $base 408x288
    wm maxsize $base 1265 994
    wm minsize $base 1 1
    wm overrideredirect $base 0
    wm resizable $base 1 1
    wm title $base "SecPanel - GUI font settings"
    frame $base.fra25  -borderwidth "2" -height "75" -relief "groove" -width "125" 
    button $base.fra25.but32  -command "fontman save" -relief "groove" -text "Save" -bg orange
    button $base.fra25.but36  -command "Window destroy .top23" -relief "groove" -text "Cancel" -bg IndianRed
    frame $base.fra18  -borderwidth "2" -height "75" -relief "groove" -width "125" 
    checkbutton $base.fra18.che19  -borderwidth "1" -text "Use system fonts" -variable "che19" 
    label $base.lab30  -borderwidth "0" -text "Example text" 
    frame $base.fra36  -height "75" -width "125" 
    frame $base.fra36.cpd37  -height "30" -width "30" 
    listbox $base.fra36.cpd37.01 -xscrollcommand ".top23.fra36.cpd37.02 set"  -yscrollcommand ".top23.fra36.cpd37.03 set" 
    bind $base.fra36.cpd37.01 <Button-1> {
        .top23.fra36.cpd37.01 activate @%x,%y
fontman ul
    }
    scrollbar $base.fra36.cpd37.02  -borderwidth "1" -command ".top23.fra36.cpd37.01 xview"  -orient "horizontal" -width "10" 
    scrollbar $base.fra36.cpd37.03  -borderwidth "1" -command ".top23.fra36.cpd37.01 yview" -width "10" 
    frame $base.fra36.cpd38  -height "30" -width "30" 
    listbox $base.fra36.cpd38.01 -height "2" -width "4" -xscrollcommand ".top23.fra36.cpd38.02 set"  -yscrollcommand ".top23.fra36.cpd38.03 set" 
    bind $base.fra36.cpd38.01 <Button-1> {
        .top23.fra36.cpd38.01 activate @%x,%y
fontman ul
    }
    scrollbar $base.fra36.cpd38.02  -borderwidth "1" -command ".top23.fra36.cpd38.01 xview"  -orient "horizontal" -width "10" 
    scrollbar $base.fra36.cpd38.03  -borderwidth "1" -command ".top23.fra36.cpd38.01 yview" -width "10" 
    frame $base.fra36.fra39  -height "75" -width "125" 
    checkbutton $base.fra36.fra39.che40  -anchor "w" -borderwidth "1" -text "Bold"  -variable "configs(fontbold)" 
    checkbutton $base.fra36.fra39.che41  -anchor "w" -borderwidth "1" -text "Italic"  -variable "configs(fontitalic)" 
    ###################
    # SETTING GEOMETRY
    ###################
    grid columnconf $base 0 -weight 1
    grid rowconf $base 0 -weight 1
    grid $base.fra25  -in ".top23" -column "0" -row "3" -columnspan "1" -rowspan "1"  -ipadx "2" -ipady "2" -padx "2" -pady "2" -sticky "ew" 
    grid $base.fra25.but32  -in ".top23.fra25" -column "1" -row "0" -columnspan "1" -rowspan "1" 
    grid $base.fra25.but36  -in ".top23.fra25" -column "0" -row "0" -columnspan "1" -rowspan "1" 
    grid $base.fra18  -in ".top23" -column "0" -row "2" -columnspan "1" -rowspan "1"  -ipadx "2" -ipady "2" -padx "2" -pady "2" -sticky "ew" 
    grid $base.fra18.che19  -in ".top23.fra18" -column "0" -row "0" -columnspan "1" -rowspan "1" 
    grid $base.lab30  -in ".top23" -column "0" -row "1" -columnspan "1" -rowspan "1"  -ipadx "2" -ipady "2" -padx "2" -pady "2" -sticky "ew" 
    grid $base.fra36  -in ".top23" -column "0" -row "0" -columnspan "1" -rowspan "1"  -ipadx "2" -ipady "2" -padx "2" -pady "2" -sticky "nesw" 
    grid columnconf $base.fra36 0 -weight 1
    grid rowconf $base.fra36 0 -weight 1
    grid $base.fra36.cpd37  -in ".top23.fra36" -column "0" -row "0" -columnspan "1" -rowspan "1"  -sticky "nesw" 
    grid columnconf $base.fra36.cpd37 0 -weight 1
    grid rowconf $base.fra36.cpd37 0 -weight 1
    grid $base.fra36.cpd37.01  -in ".top23.fra36.cpd37" -column "0" -row "0" -columnspan "1"  -rowspan "1" -sticky "nesw" 
    grid $base.fra36.cpd37.02  -in ".top23.fra36.cpd37" -column "0" -row "1" -columnspan "1"  -rowspan "1" -sticky "ew" 
    grid $base.fra36.cpd37.03  -in ".top23.fra36.cpd37" -column "1" -row "0" -columnspan "1"  -rowspan "1" -sticky "ns" 
    grid $base.fra36.cpd38  -in ".top23.fra36" -column "1" -row "0" -columnspan "1" -rowspan "1"  -sticky "ns" 
    grid columnconf $base.fra36.cpd38 0 -weight 1
    grid rowconf $base.fra36.cpd38 0 -weight 1
    grid $base.fra36.cpd38.01  -in ".top23.fra36.cpd38" -column "0" -row "0" -columnspan "1"  -rowspan "1" -sticky "nesw" 
    grid $base.fra36.cpd38.02  -in ".top23.fra36.cpd38" -column "0" -row "1" -columnspan "1"  -rowspan "1" -sticky "ew" 
    grid $base.fra36.cpd38.03  -in ".top23.fra36.cpd38" -column "1" -row "0" -columnspan "1"  -rowspan "1" -sticky "ns" 
    grid $base.fra36.fra39  -in ".top23.fra36" -column "2" -row "0" -columnspan "1" -rowspan "1"  -ipadx "2" -ipady "2" -padx "2" -pady "2" -sticky "n" 
    grid $base.fra36.fra39.che40  -in ".top23.fra36.fra39" -column "0" -row "0" -columnspan "1"  -rowspan "1" -sticky "ew" 
    grid $base.fra36.fra39.che41  -in ".top23.fra36.fra39" -column "0" -row "1" -columnspan "1"  -rowspan "1" -sticky "ew"
}

proc vTclWindow.top25 {base} {
    if {$base == ""} {
        set base .top25
    }
    if {[winfo exists $base]} {
        wm deiconify $base; return
    }
    ###################
    # CREATING WIDGETS
    ###################
    toplevel $base -class Toplevel  -borderwidth "4" 
    wm focusmodel $base passive
    wm geometry $base 204x120
    wm maxsize $base 1009 738
    wm minsize $base 1 1
    wm overrideredirect $base 0
    wm resizable $base 1 1
    wm title $base "SecPanel - About"
    frame $base.fra26  -borderwidth "1" -height "75" -width "125" 
    message $base.fra26.mes28  -anchor "w" -aspect "550" -borderwidth "1" -justify "center"  -padx "5" -pady "2"  -text "Author: Steffen Leich
<secpanel@pingx.net>" 
    label $base.fra26.lab21  -borderwidth "1" 
    frame $base.fra27  -borderwidth "1" -height "75" -width "125" 
    button $base.fra27.but29  -command "Window destroy .top25" -padx "9" -pady "3" -relief "groove"  -text "Close" -bg IndianRed
    ###################
    # SETTING GEOMETRY
    ###################
    grid columnconf $base 0 -weight 1
    grid rowconf $base 0 -weight 1
    grid $base.fra26  -in ".top25" -column "0" -row "0" -columnspan "1" -rowspan "1"  -ipadx "2" -ipady "2" -padx "2" -pady "2" -sticky "nesw" 
    grid $base.fra26.mes28  -in ".top25.fra26" -column "0" -row "1" -columnspan "1" -rowspan "1"  -sticky "ew" 
    grid $base.fra26.lab21  -in ".top25.fra26" -column "0" -row "0" -columnspan "1" -rowspan "1"  -sticky "ew" 
    grid $base.fra27  -in ".top25" -column "0" -row "1" -columnspan "1" -rowspan "1"  -ipadx "2" -ipady "2" -padx "2" -pady "2" -sticky "ew" 
    grid $base.fra27.but29  -in ".top25.fra27" -column "0" -row "0" -columnspan "1" -rowspan "1"
}

proc vTclWindow.top26 {base} {
    if {$base == ""} {
        set base .top26
    }
    if {[winfo exists $base]} {
        wm deiconify $base; return
    }
    ###################
    # CREATING WIDGETS
    ###################
    toplevel $base -class Toplevel  -borderwidth "4" 
    wm focusmodel $base passive
    wm geometry $base 485x254
    wm maxsize $base 1265 994
    wm minsize $base 1 1
    wm overrideredirect $base 0
    wm resizable $base 1 1
    wm title $base "SecPanel - History"
    frame $base.cpd27  -borderwidth "1" -height "30" -width "30" 
    scrollbar $base.cpd27.01  -borderwidth "1" -command ".top26.cpd27.03 xview"  -orient "horizontal" -width "10" 
    scrollbar $base.cpd27.02  -borderwidth "1" -command ".top26.cpd27.03 yview" -width "10" 
    text $base.cpd27.03  -borderwidth "1" -font "fixed" -height "1" -width "8" -wrap "none"  -xscrollcommand ".top26.cpd27.01 set"  -yscrollcommand ".top26.cpd27.02 set" 
    frame $base.fra17  -height "75" -relief "groove" -width "125" 
    radiobutton $base.fra17.rad25  -anchor "w" -borderwidth "1" -command "historyman print"  -disabledforeground "#8c8cff" -indicatoron "0" -relief "raised"  -selectcolor "#8c8cff" -text "Keydist" -value "5" -variable "histjob" 
    radiobutton $base.fra17.rad26  -anchor "w" -borderwidth "1" -command "historyman print"  -indicatoron "0" -relief "ridge" -selectcolor "#8c8cff"  -text "Keygen" -value "4" -variable "histjob" 
    radiobutton $base.fra17.rad27  -anchor "w" -borderwidth "1" -command "historyman print"  -indicatoron "0" -selectcolor "#8c8cff" -text "SCP transfer"  -value "3" -variable "histjob" 
    radiobutton $base.fra17.rad28  -anchor "w" -borderwidth "1" -command "historyman print"  -indicatoron "0" -selectcolor "#8c8cff" -text "SCP connect"  -value "2" -variable "histjob" 
    radiobutton $base.fra17.rad29  -anchor "w" -borderwidth "1" -command "historyman print"  -indicatoron "0" -selectcolor "#8c8cff" -text "SSH connect"  -value "1" -variable "histjob" 
    radiobutton $base.fra17.rad37  -anchor "w" -borderwidth "1" -command "historyman print"  -disabledforeground "#8c8cff" -indicatoron "0" -relief "raised"  -selectcolor "#8c8cff" -text "Agent" -value "6" -variable "histjob" 
    button $base.but18  -command "Window destroy .top26" -relief "groove" -text "Close" -bg IndianRed
    menubutton $base.men17  -indicatoron "1" -menu ".top26.men17.m" -padx "4" -pady "3"  -relief "groove" -text "Clear hist." 
    menu $base.men17.m  -borderwidth "1" -cursor "" -tearoff "0" 
    $base.men17.m add command  -command "historyman clear 30" -label "Keep last 30 days" 
    $base.men17.m add command  -command "historyman clear 7" -label "Keep last 7 days" 
    $base.men17.m add command  -command "historyman clear 1" -label "Keep one day" 
    $base.men17.m add command  -command "historyman clear 0" -label "Keep nothing" 
    button $base.but28  -justify "left" -relief "groove" -text "Generate HTML" -state disabled
    ###################
    # SETTING GEOMETRY
    ###################
    grid columnconf $base 1 -weight 1
    grid rowconf $base 1 -weight 1
    grid $base.cpd27  -in ".top26" -column "1" -row "0" -columnspan "1" -rowspan "3"  -sticky "nesw" 
    grid columnconf $base.cpd27 0 -weight 1
    grid rowconf $base.cpd27 0 -weight 1
    grid $base.cpd27.01  -in ".top26.cpd27" -column "0" -row "1" -columnspan "1" -rowspan "1"  -sticky "ew" 
    grid $base.cpd27.02  -in ".top26.cpd27" -column "1" -row "0" -columnspan "1" -rowspan "1"  -sticky "ns" 
    grid $base.cpd27.03  -in ".top26.cpd27" -column "0" -row "0" -columnspan "1" -rowspan "1"  -sticky "nesw" 
    grid $base.fra17  -in ".top26" -column "0" -row "0" -columnspan "1" -rowspan "1"  -padx "2" -pady "2" -sticky "new" 
    grid columnconf $base.fra17 0 -weight 1
    grid $base.fra17.rad25  -in ".top26.fra17" -column "0" -row "4" -columnspan "1" -rowspan "1"  -sticky "ew" 
    grid $base.fra17.rad26  -in ".top26.fra17" -column "0" -row "3" -columnspan "1" -rowspan "1"  -sticky "ew" 
    grid $base.fra17.rad27  -in ".top26.fra17" -column "0" -row "2" -columnspan "1" -rowspan "1"  -sticky "ew" 
    grid $base.fra17.rad28  -in ".top26.fra17" -column "0" -row "1" -columnspan "1" -rowspan "1"  -sticky "ew" 
    grid $base.fra17.rad29  -in ".top26.fra17" -column "0" -row "0" -columnspan "1" -rowspan "1"  -sticky "ew" 
    grid $base.fra17.rad37  -in ".top26.fra17" -column "0" -row "5" -columnspan "1" -rowspan "1"  -sticky "ew" 
    grid $base.but18  -in ".top26" -column "0" -row "3" -columnspan "2" -rowspan "1" 
    grid $base.men17  -in ".top26" -column "0" -row "2" -columnspan "1" -rowspan "1"  -sticky "esw" 
    grid $base.but28  -in ".top26" -column "0" -row "1" -columnspan "1" -rowspan "1"  -sticky "esw"
}

proc vTclWindow.top27 {base} {
    if {$base == ""} {
        set base .top27
    }
    if {[winfo exists $base]} {
        wm deiconify $base; return
    }
    ###################
    # CREATING WIDGETS
    ###################
    toplevel $base -class Toplevel
    wm focusmodel $base passive
    # wm geometry $base 134x115
    wm maxsize $base 1265 994
    wm minsize $base 1 1
    wm overrideredirect $base 0
    wm resizable $base 1 1
    wm title $base "SecPanel - Satellite"
    label $base.lab34  -anchor "w" -background "#d6d7d6" -borderwidth "0" -text "SSH-Agent" 
    frame $base.fra26  -height "75" -width "125" 
    menubutton $base.fra26.01  -anchor "w" -indicatoron "1" -menu ".top27.fra26.01.02" -padx "4"  -pady "3" -relief "groove" -text "Specials" 
    menubutton $base.fra26.03  -anchor "w" -indicatoron "1" -menu ".top27.fra26.03.04" -padx "4"  -pady "3" -relief "groove" -text "Defaults" 
    button $base.but27  -command "changetab big" -relief "groove" -text "Restore" -bg orange
    ###################
    # SETTING GEOMETRY
    ###################
    grid columnconf $base 0 -weight 1
    grid rowconf $base 2 -weight 1
    grid $base.lab34  -in ".top27" -column "0" -row "2" -columnspan "1" -rowspan "1"  -sticky "nesw" 
    grid $base.fra26  -in ".top27" -column "0" -row "0" -columnspan "1" -rowspan "1"  -sticky "ew" 
    grid columnconf $base.fra26 0 -weight 1
    grid $base.fra26.01  -in ".top27.fra26" -column "0" -row "1" -columnspan "1" -rowspan "1"  -sticky "ew" 
    grid $base.fra26.03  -in ".top27.fra26" -column "0" -row "0" -columnspan "1" -rowspan "1"  -sticky "ew" 
    grid $base.but27  -in ".top27" -column "0" -row "1" -columnspan "1" -rowspan "1"  -sticky "ew"
}

proc vTclWindow.top32 {base} {
    if {$base == ""} {
        set base .top32
    }
    if {[winfo exists $base]} {
        wm deiconify $base; return
    }
    ###################
    # CREATING WIDGETS
    ###################
    toplevel $base -class Toplevel  -borderwidth "4" 
    wm focusmodel $base passive
    wm geometry $base 266x258
    wm maxsize $base 1009 738
    wm minsize $base 1 1
    wm overrideredirect $base 0
    wm resizable $base 1 1
    wm title $base "SecPanel - Colors"
    frame $base.fra33  -borderwidth "1" -height "75" -width "125" 
    button $base.fra33.but37  -anchor "w" -command "colorman fore" -height "1" -padx "9" -pady "3"  -relief "groove" -width "2" 
    button $base.fra33.but38  -anchor "w" -command "colorman back" -padx "9" -pady "3"  -relief "groove" 
    button $base.fra33.but39  -anchor "w" -command "colorman entfore" -padx "9" -pady "3"  -relief "groove" 
    button $base.fra33.but40  -anchor "w" -command "colorman entback" -padx "9" -pady "3"  -relief "groove" 
    button $base.fra33.but41  -anchor "w" -command "colorman listfore" -padx "9" -pady "3"  -relief "groove" 
    button $base.fra33.but42  -anchor "w" -command "colorman listback" -padx "9" -pady "3"  -relief "groove" 
    label $base.fra33.lab17  -anchor "w" -borderwidth "1" -text "Foreground:" 
    label $base.fra33.lab18  -anchor "w" -borderwidth "1" -text "Background:" 
    label $base.fra33.lab19  -anchor "w" -borderwidth "1" -text "Entries - Fore:" 
    label $base.fra33.lab20  -anchor "w" -borderwidth "1" -text "Entries - Back:" 
    label $base.fra33.lab21  -anchor "w" -borderwidth "1" -text "Lists - Fore:" 
    label $base.fra33.lab22  -anchor "w" -borderwidth "1" -text "Lists - Back:" 
    checkbutton $base.fra33.che17  -anchor "e" -borderwidth "1" -text "Default" -variable "foredef" 
    checkbutton $base.fra33.che23  -anchor "e" -borderwidth "1" -text "Default" -variable "backdef" 
    checkbutton $base.fra33.che24  -anchor "e" -borderwidth "1" -text "Default" -variable "entforedef" 
    checkbutton $base.fra33.che25  -anchor "e" -borderwidth "1" -text "Default" -variable "entbackdef" 
    checkbutton $base.fra33.che26  -anchor "e" -borderwidth "1" -text "Default" -variable "listforedef" 
    checkbutton $base.fra33.che27  -anchor "e" -borderwidth "1" -text "Default" -variable "listbackdef" 
    frame $base.fra34  -borderwidth "2" -height "75" -relief "groove" -width "125" 
    button $base.fra34.but35  -command "Window destroy .top32" -padx "9" -pady "3" -relief "groove"  -text "Cancel" 
    button $base.fra34.but36  -command "colorman save" -padx "9" -pady "3" -relief "groove"  -text "Save" 
    label $base.lab18  -anchor "w" -borderwidth "1"  -text "Define colors or use external defined" 
    ###################
    # SETTING GEOMETRY
    ###################
    grid columnconf $base 0 -weight 1
    grid rowconf $base 0 -weight 1
    grid $base.fra33  -in ".top32" -column "0" -row "1" -columnspan "1" -rowspan "1"  -ipadx "2" -ipady "2" -padx "2" -pady "2" -sticky "nesw" 
    grid columnconf $base.fra33 1 -weight 1
    grid $base.fra33.but37  -in ".top32.fra33" -column "1" -row "0" -columnspan "1" -rowspan "1"  -padx "2" -sticky "ew" 
    grid $base.fra33.but38  -in ".top32.fra33" -column "1" -row "1" -columnspan "1" -rowspan "1"  -padx "2" -sticky "ew" 
    grid $base.fra33.but39  -in ".top32.fra33" -column "1" -row "2" -columnspan "1" -rowspan "1"  -padx "2" -sticky "ew" 
    grid $base.fra33.but40  -in ".top32.fra33" -column "1" -row "3" -columnspan "1" -rowspan "1"  -padx "2" -sticky "ew" 
    grid $base.fra33.but41  -in ".top32.fra33" -column "1" -row "4" -columnspan "1" -rowspan "1"  -padx "2" -sticky "ew" 
    grid $base.fra33.but42  -in ".top32.fra33" -column "1" -row "5" -columnspan "1" -rowspan "1"  -padx "2" -sticky "ew" 
    grid $base.fra33.lab17  -in ".top32.fra33" -column "0" -row "0" -columnspan "1" -rowspan "1"  -sticky "ew" 
    grid $base.fra33.lab18  -in ".top32.fra33" -column "0" -row "1" -columnspan "1" -rowspan "1"  -sticky "ew" 
    grid $base.fra33.lab19  -in ".top32.fra33" -column "0" -row "2" -columnspan "1" -rowspan "1"  -sticky "ew" 
    grid $base.fra33.lab20  -in ".top32.fra33" -column "0" -row "3" -columnspan "1" -rowspan "1"  -sticky "ew" 
    grid $base.fra33.lab21  -in ".top32.fra33" -column "0" -row "4" -columnspan "1" -rowspan "1"  -sticky "ew" 
    grid $base.fra33.lab22  -in ".top32.fra33" -column "0" -row "5" -columnspan "1" -rowspan "1"  -sticky "ew" 
    grid $base.fra33.che17  -in ".top32.fra33" -column "2" -row "0" -columnspan "1" -rowspan "1"  -padx "2" -sticky "ew" 
    grid $base.fra33.che23  -in ".top32.fra33" -column "2" -row "1" -columnspan "1" -rowspan "1"  -padx "2" -sticky "ew" 
    grid $base.fra33.che24  -in ".top32.fra33" -column "2" -row "2" -columnspan "1" -rowspan "1"  -padx "2" -sticky "ew" 
    grid $base.fra33.che25  -in ".top32.fra33" -column "2" -row "3" -columnspan "1" -rowspan "1"  -padx "2" -sticky "ew" 
    grid $base.fra33.che26  -in ".top32.fra33" -column "2" -row "4" -columnspan "1" -rowspan "1"  -padx "2" -sticky "ew" 
    grid $base.fra33.che27  -in ".top32.fra33" -column "2" -row "5" -columnspan "1" -rowspan "1"  -padx "2" -sticky "ew" 
    grid $base.fra34  -in ".top32" -column "0" -row "2" -columnspan "1" -rowspan "1"  -ipadx "2" -ipady "2" -padx "2" -pady "2" -sticky "ew" 
    grid $base.fra34.but35  -in ".top32.fra34" -column "0" -row "0" -columnspan "1" -rowspan "1" 
    grid $base.fra34.but36  -in ".top32.fra34" -column "1" -row "0" -columnspan "1" -rowspan "1" 
    grid $base.lab18  -in ".top32" -column "0" -row "0" -columnspan "1" -rowspan "1"  -sticky "ew"
}

proc vTclWindow.top33 {base} {
    if {$base == ""} {
        set base .top33
    }
    if {[winfo exists $base]} {
        wm deiconify $base; return
    }
    ###################
    # CREATING WIDGETS
    ###################
    toplevel $base -class Toplevel  -borderwidth "4" 
    wm focusmodel $base passive
    wm geometry $base 551x214
    wm maxsize $base 1265 994
    wm minsize $base 1 1
    wm overrideredirect $base 0
    wm resizable $base 1 1
    wm title $base "SecPanel - Agent-info"
    frame $base.fra34  -borderwidth "2" -height "75" -relief "groove" -width "125" 
    label $base.fra34.lab36  -anchor "e" -borderwidth "0" -relief "raised" -text "SSH_AUTH_SOCK:" 
    label $base.fra34.lab37  -anchor "e" -borderwidth "0" -relief "raised" -text "SSH_AGENT_PID:" 
    label $base.fra34.lab38  -anchor "w" -borderwidth "0" 
    label $base.fra34.lab39  -anchor "w" -borderwidth "0" -relief "raised" 
    button $base.but40  -command "Window destroy .top33" -relief "groove" -text "Close" 
    frame $base.cpd36  -height "30" -relief "raised" -width "30" 
    scrollbar $base.cpd36.01  -borderwidth "1" -command ".top33.cpd36.03 xview"  -orient "horizontal" -width "10" 
    scrollbar $base.cpd36.02  -borderwidth "1" -command ".top33.cpd36.03 yview" -width "10" 
    text $base.cpd36.03  -borderwidth "1" -font "fixed" -height "1" -width "8"  -xscrollcommand ".top33.cpd36.01 set"  -yscrollcommand ".top33.cpd36.02 set" 
    ###################
    # SETTING GEOMETRY
    ###################
    grid columnconf $base 0 -weight 1
    grid rowconf $base 1 -weight 1
    grid $base.fra34  -in ".top33" -column "0" -row "0" -columnspan "1" -rowspan "1"  -ipadx "2" -ipady "2" -padx "2" -pady "2" -sticky "nesw" 
    grid columnconf $base.fra34 1 -weight 1
    grid $base.fra34.lab36  -in ".top33.fra34" -column "0" -row "0" -columnspan "1" -rowspan "1"  -padx "4" -sticky "nesw" 
    grid $base.fra34.lab37  -in ".top33.fra34" -column "0" -row "1" -columnspan "1" -rowspan "1"  -padx "4" -sticky "nesw" 
    grid $base.fra34.lab38  -in ".top33.fra34" -column "1" -row "0" -columnspan "1" -rowspan "1"  -padx "2" -sticky "nesw" 
    grid $base.fra34.lab39  -in ".top33.fra34" -column "1" -row "1" -columnspan "1" -rowspan "1"  -padx "2" -sticky "nesw" 
    grid $base.but40  -in ".top33" -column "0" -row "2" -columnspan "1" -rowspan "1"  -pady "2" -sticky "ew" 
    grid $base.cpd36  -in ".top33" -column "0" -row "1" -columnspan "1" -rowspan "1"  -padx "2" -pady "2" -sticky "nesw" 
    grid columnconf $base.cpd36 0 -weight 1
    grid rowconf $base.cpd36 0 -weight 1
    grid $base.cpd36.01  -in ".top33.cpd36" -column "0" -row "1" -columnspan "1" -rowspan "1"  -sticky "ew" 
    grid $base.cpd36.02  -in ".top33.cpd36" -column "1" -row "0" -columnspan "1" -rowspan "1"  -sticky "ns" 
    grid $base.cpd36.03  -in ".top33.cpd36" -column "0" -row "0" -columnspan "1" -rowspan "1"  -sticky "nesw"
}

proc vTclWindow.top34 {base} {
    if {$base == ""} {
        set base .top34
    }
    if {[winfo exists $base]} {
        wm deiconify $base; return
    }
    ###################
    # CREATING WIDGETS
    ###################
    toplevel $base -class Toplevel  -borderwidth "8" 
    wm focusmodel $base passive
    wm geometry $base 583x460
    wm maxsize $base 1265 994
    wm minsize $base 1 1
    wm overrideredirect $base 0
    wm resizable $base 1 1
    wm title $base "SecPanel - SCP interface"
    frame $base.fra35  -borderwidth "1" -height "75" -width "125" 
    frame $base.fra35.cpd38  -borderwidth "1" -height "30" -width "30" 
    listbox $base.fra35.cpd38.01  -borderwidth "1" -selectmode "extended"  -xscrollcommand ".top34.fra35.cpd38.02 set"  -yscrollcommand ".top34.fra35.cpd38.03 set" 
    bind $base.fra35.cpd38.01 <Double-Button-1> {
        scpswitchdir %y l
    }
    scrollbar $base.fra35.cpd38.02  -borderwidth "1" -command ".top34.fra35.cpd38.01 xview"  -orient "horizontal" -width "10" 
    scrollbar $base.fra35.cpd38.03  -borderwidth "1" -command ".top34.fra35.cpd38.01 yview" -width "10" 
    frame $base.fra35.cpd39  -borderwidth "1" -height "30" -width "30" 
    listbox $base.fra35.cpd39.01  -borderwidth "1" -selectmode "extended"  -xscrollcommand ".top34.fra35.cpd39.02 set"  -yscrollcommand ".top34.fra35.cpd39.03 set" 
    scrollbar $base.fra35.cpd39.02  -borderwidth "1" -command ".top34.fra35.cpd39.01 xview"  -orient "horizontal" -width "10" 
    scrollbar $base.fra35.cpd39.03  -borderwidth "1" -command ".top34.fra35.cpd39.01 yview" -width "10" 
    entry $base.fra35.ent45  -borderwidth "1" -width "10" 
    bind $base.fra35.ent45 <Key-Return> {
        scpswitchdir ent l
    }
    menubutton $base.fra35.men19  -indicatoron "1" -menu ".top34.fra35.men19.m" -padx "4" -pady "3"  -relief "groove" -text "Tools" 
    menu $base.fra35.men19.m  -borderwidth "1" -cursor "" -tearoff "0" 
    $base.fra35.men19.m add command  -command "scpswitchdir ent l" -label "Refresh" 
    $base.fra35.men19.m add command  -command "scptools mkdir l" -label "Make dir" 
    $base.fra35.men19.m add command  -command "scptools delete l" -label "Delete marked" 
    $base.fra35.men19.m add command  -command "scptools list l" -label "Long listing" 
    frame $base.fra37  -borderwidth "1" -height "75" -width "125" 
    frame $base.fra37.cpd40  -borderwidth "1" -height "30" -width "30" 
    listbox $base.fra37.cpd40.01  -borderwidth "1" -selectmode "extended"  -xscrollcommand ".top34.fra37.cpd40.02 set"  -yscrollcommand ".top34.fra37.cpd40.03 set" 
    bind $base.fra37.cpd40.01 <Double-Button-1> {
        scpswitchdir %y r
    }
    scrollbar $base.fra37.cpd40.02  -borderwidth "1" -command ".top34.fra37.cpd40.01 xview"  -orient "horizontal" -width "10" 
    scrollbar $base.fra37.cpd40.03  -borderwidth "1" -command ".top34.fra37.cpd40.01 yview" -width "10" 
    frame $base.fra37.cpd41  -borderwidth "1" -height "30" -width "30" 
    listbox $base.fra37.cpd41.01  -borderwidth "1" -selectmode "extended"  -xscrollcommand ".top34.fra37.cpd41.02 set"  -yscrollcommand ".top34.fra37.cpd41.03 set" 
    scrollbar $base.fra37.cpd41.02  -borderwidth "1" -command ".top34.fra37.cpd41.01 xview"  -orient "horizontal" -width "10" 
    scrollbar $base.fra37.cpd41.03  -borderwidth "1" -command ".top34.fra37.cpd41.01 yview" -width "10" 
    entry $base.fra37.ent44  -borderwidth "1" -width "10" 
    bind $base.fra37.ent44 <Key-Return> {
        scpswitchdir ent r
    }
    menubutton $base.fra37.men20  -indicatoron "1" -menu ".top34.fra37.men20.m" -padx "4" -pady "3"  -relief "groove" -text "Tools" 
    menu $base.fra37.men20.m  -borderwidth "1" -cursor "" -tearoff "0" 
    $base.fra37.men20.m add command  -command "scpswitchdir ent r" -label "Refresh" 
    $base.fra37.men20.m add command  -command "scptools mkdir r" -label "Make dir" 
    $base.fra37.men20.m add command  -command "scptools delete r" -label "Delete marked" 
    $base.fra37.men20.m add command  -command "scptools list r" -label "Long listing" 
    frame $base.fra42  -borderwidth "2" -height "75" -relief "groove" -width "125" 
    button $base.fra42.but43  -command "scpman close" -padx "9" -pady "3" -relief "groove"  -text "Close connection" 
    frame $base.fra46  -borderwidth "2" -height "75" -relief "groove" -width "125" 
    label $base.fra46.lab47  -anchor "w" -borderwidth "1" -height "1" 
    frame $base.fra48  -borderwidth "1" -height "75" -width "125" 
    button $base.fra48.but49  -command "scptransfer cptoremote" -padx "9" -pady "3"  -relief "groove" -text ">" 
    button $base.fra48.but50  -command "scptransfer cptolocal" -padx "9" -pady "3" -relief "groove"  -text "<" 
    frame $base.fra17  -borderwidth "2" -height "75" -relief "groove" -width "125" 
    label $base.fra17.lab18  -anchor "w" -borderwidth "1" -height "1" 
    ###################
    # SETTING GEOMETRY
    ###################
    grid columnconf $base 0 -weight 1
    grid columnconf $base 2 -weight 1
    grid rowconf $base 1 -weight 1
    grid $base.fra35  -in ".top34" -column "0" -row "1" -columnspan "1" -rowspan "1"  -ipadx "2" -ipady "2" -padx "2" -pady "2" -sticky "nesw" 
    grid columnconf $base.fra35 0 -weight 1
    grid rowconf $base.fra35 1 -weight 1
    grid rowconf $base.fra35 2 -weight 1
    grid $base.fra35.cpd38  -in ".top34.fra35" -column "0" -row "1" -columnspan "2" -rowspan "1"  -padx "2" -pady "2" -sticky "nesw" 
    grid columnconf $base.fra35.cpd38 0 -weight 1
    grid rowconf $base.fra35.cpd38 0 -weight 1
    grid $base.fra35.cpd38.01  -in ".top34.fra35.cpd38" -column "0" -row "0" -columnspan "1"  -rowspan "1" -sticky "nesw" 
    grid $base.fra35.cpd38.02  -in ".top34.fra35.cpd38" -column "0" -row "1" -columnspan "1"  -rowspan "1" -sticky "ew" 
    grid $base.fra35.cpd38.03  -in ".top34.fra35.cpd38" -column "1" -row "0" -columnspan "1"  -rowspan "1" -sticky "ns" 
    grid $base.fra35.cpd39  -in ".top34.fra35" -column "0" -row "2" -columnspan "2" -rowspan "1"  -padx "2" -pady "2" -sticky "nesw" 
    grid columnconf $base.fra35.cpd39 0 -weight 1
    grid rowconf $base.fra35.cpd39 0 -weight 1
    grid $base.fra35.cpd39.01  -in ".top34.fra35.cpd39" -column "0" -row "0" -columnspan "1"  -rowspan "1" -sticky "nesw" 
    grid $base.fra35.cpd39.02  -in ".top34.fra35.cpd39" -column "0" -row "1" -columnspan "1"  -rowspan "1" -sticky "ew" 
    grid $base.fra35.cpd39.03  -in ".top34.fra35.cpd39" -column "1" -row "0" -columnspan "1"  -rowspan "1" -sticky "ns" 
    grid $base.fra35.ent45  -in ".top34.fra35" -column "0" -row "0" -columnspan "1" -rowspan "1"  -padx "2" -pady "2" -sticky "ew" 
    grid $base.fra35.men19  -in ".top34.fra35" -column "1" -row "0" -columnspan "1" -rowspan "1" 
    grid $base.fra37  -in ".top34" -column "2" -row "1" -columnspan "1" -rowspan "1"  -ipadx "2" -ipady "2" -padx "2" -pady "2" -sticky "nesw" 
    grid columnconf $base.fra37 0 -weight 1
    grid rowconf $base.fra37 1 -weight 1
    grid rowconf $base.fra37 2 -weight 1
    grid $base.fra37.cpd40  -in ".top34.fra37" -column "0" -row "1" -columnspan "2" -rowspan "1"  -padx "2" -pady "2" -sticky "nesw" 
    grid columnconf $base.fra37.cpd40 0 -weight 1
    grid rowconf $base.fra37.cpd40 0 -weight 1
    grid $base.fra37.cpd40.01  -in ".top34.fra37.cpd40" -column "0" -row "0" -columnspan "1"  -rowspan "1" -sticky "nesw" 
    grid $base.fra37.cpd40.02  -in ".top34.fra37.cpd40" -column "0" -row "1" -columnspan "1"  -rowspan "1" -sticky "ew" 
    grid $base.fra37.cpd40.03  -in ".top34.fra37.cpd40" -column "1" -row "0" -columnspan "1"  -rowspan "1" -sticky "ns" 
    grid $base.fra37.cpd41  -in ".top34.fra37" -column "0" -row "2" -columnspan "2" -rowspan "1"  -padx "2" -pady "2" -sticky "nesw" 
    grid columnconf $base.fra37.cpd41 0 -weight 1
    grid rowconf $base.fra37.cpd41 0 -weight 1
    grid $base.fra37.cpd41.01  -in ".top34.fra37.cpd41" -column "0" -row "0" -columnspan "1"  -rowspan "1" -sticky "nesw" 
    grid $base.fra37.cpd41.02  -in ".top34.fra37.cpd41" -column "0" -row "1" -columnspan "1"  -rowspan "1" -sticky "ew" 
    grid $base.fra37.cpd41.03  -in ".top34.fra37.cpd41" -column "1" -row "0" -columnspan "1"  -rowspan "1" -sticky "ns" 
    grid $base.fra37.ent44  -in ".top34.fra37" -column "0" -row "0" -columnspan "1" -rowspan "1"  -padx "2" -pady "2" -sticky "ew" 
    grid $base.fra37.men20  -in ".top34.fra37" -column "1" -row "0" -columnspan "1" -rowspan "1" 
    grid $base.fra42  -in ".top34" -column "0" -row "2" -columnspan "3" -rowspan "1"  -ipadx "2" -ipady "2" -padx "2" -pady "2" -sticky "ew" 
    grid $base.fra42.but43  -in ".top34.fra42" -column "0" -row "0" -columnspan "1" -rowspan "1" 
    grid $base.fra46  -in ".top34" -column "0" -row "0" -columnspan "1" -rowspan "1"  -ipadx "2" -ipady "2" -padx "2" -pady "2" -sticky "ew" 
    grid columnconf $base.fra46 0 -weight 1
    grid $base.fra46.lab47  -in ".top34.fra46" -column "0" -row "0" -columnspan "1" -rowspan "1"  -sticky "ew" 
    grid $base.fra48  -in ".top34" -column "1" -row "1" -columnspan "1" -rowspan "1" 
    grid $base.fra48.but49  -in ".top34.fra48" -column "0" -row "0" -columnspan "1" -rowspan "1" 
    grid $base.fra48.but50  -in ".top34.fra48" -column "0" -row "1" -columnspan "1" -rowspan "1" 
    grid $base.fra17  -in ".top34" -column "2" -row "0" -columnspan "1" -rowspan "1"  -ipadx "2" -ipady "2" -padx "2" -pady "2" -sticky "ew" 
    grid columnconf $base.fra17 0 -weight 1
    grid $base.fra17.lab18  -in ".top34.fra17" -column "0" -row "0" -columnspan "1" -rowspan "1"  -sticky "ew"
}

proc vTclWindow.top40 {base} {
    if {$base == ""} {
        set base .top40
    }
    if {[winfo exists $base]} {
        wm deiconify $base; return
    }
    ###################
    # CREATING WIDGETS
    ###################
    toplevel $base -class Toplevel  -borderwidth "4" 
    wm focusmodel $base passive
    wm geometry $base 293x147
    wm maxsize $base 1009 738
    wm minsize $base 1 1
    wm overrideredirect $base 0
    wm resizable $base 1 1
    wm title $base "SecPanel - Connection Entry"
    frame $base.fra45  -borderwidth "2" -height "75" -relief "groove" -width "125" 
    button $base.fra45.but46  -command "Window destroy .top40" -padx "9" -pady "3" -relief "groove"  -text "Cancel" 
    button $base.fra45.but47  -command "newconn def 2" -padx "9" -pady "3" -relief "groove"  -text "Save" 
    frame $base.fra21  -borderwidth "1" -height "75" -width "125" 
    label $base.fra21.lab22  -anchor "w" -borderwidth "1" -text "Title:" 
    entry $base.fra21.ent23  -borderwidth "1" -width "0" 
    label $base.fra21.lab24  -anchor "w" -borderwidth "1" -text "Adress:" 
    entry $base.fra21.ent25  -borderwidth "1" -width "0" 
    label $base.fra21.lab26  -anchor "w" -borderwidth "1" -text "User:" 
    entry $base.fra21.ent27  -borderwidth "1" -width "0" 
    checkbutton $base.fra21.che18  -borderwidth "1" -text "Always ask" -variable "askuserdef" 
    label $base.lab24  -anchor "w" -borderwidth "1" -foreground "#000000"  -text "Connection entry" 
    ###################
    # SETTING GEOMETRY
    ###################
    grid columnconf $base 0 -weight 1
    grid rowconf $base 2 -weight 1
    grid $base.fra45  -in ".top40" -column "0" -row "2" -columnspan "1" -rowspan "1"  -ipadx "2" -ipady "2" -padx "3" -pady "3" -sticky "nesw" 
    grid $base.fra45.but46  -in ".top40.fra45" -column "0" -row "0" -columnspan "1" -rowspan "1" 
    grid $base.fra45.but47  -in ".top40.fra45" -column "1" -row "0" -columnspan "1" -rowspan "1" 
    grid $base.fra21  -in ".top40" -column "0" -row "1" -columnspan "1" -rowspan "1"  -ipadx "2" -ipady "2" -padx "2" -pady "2" -sticky "ew" 
    grid columnconf $base.fra21 1 -weight 1
    grid $base.fra21.lab22  -in ".top40.fra21" -column "0" -row "0" -columnspan "1" -rowspan "1"  -sticky "ew" 
    grid $base.fra21.ent23  -in ".top40.fra21" -column "1" -row "0" -columnspan "2" -rowspan "1"  -sticky "ew" 
    grid $base.fra21.lab24  -in ".top40.fra21" -column "0" -row "1" -columnspan "1" -rowspan "1"  -sticky "ew" 
    grid $base.fra21.ent25  -in ".top40.fra21" -column "1" -row "1" -columnspan "2" -rowspan "1"  -sticky "ew" 
    grid $base.fra21.lab26  -in ".top40.fra21" -column "0" -row "2" -columnspan "1" -rowspan "1"  -sticky "ew" 
    grid $base.fra21.ent27  -in ".top40.fra21" -column "1" -row "2" -columnspan "1" -rowspan "1"  -sticky "ew" 
    grid $base.fra21.che18  -in ".top40.fra21" -column "2" -row "2" -columnspan "1" -rowspan "1" 
    grid $base.lab24  -in ".top40" -column "0" -row "0" -columnspan "1" -rowspan "1"  -padx "2" -pady "2" -sticky "new"
}

proc vTclWindow.top43 {base} {
    if {$base == ""} {
        set base .top43
    }
    if {[winfo exists $base]} {
        wm deiconify $base; return
    }
    ###################
    # CREATING WIDGETS
    ###################
    toplevel $base -class Toplevel  -borderwidth "4" 
    wm focusmodel $base passive
    wm geometry $base 243x350
    wm maxsize $base 1009 738
    wm minsize $base 1 1
    wm overrideredirect $base 0
    wm resizable $base 1 1
    wm title $base "SecPanel - Local forwardings"
    frame $base.cpd44  -borderwidth "1" -width "60" 
    listbox $base.cpd44.01  -borderwidth "1" -height "5" -xscrollcommand ".top43.cpd44.02 set"  -yscrollcommand ".top43.cpd44.03 set" 
    bind $base.cpd44.01 <Button-1> {
        .top43.cpd44.01 activate @%x,%y
        showcomm l
    }
    scrollbar $base.cpd44.02  -borderwidth "1" -command ".top43.cpd44.01 xview"  -orient "horizontal" -width "10" 
    scrollbar $base.cpd44.03  -borderwidth "1" -command ".top43.cpd44.01 yview" -width "10" 
    frame $base.fra45  -borderwidth "2" -height "75" -relief "groove" -width "125" 
    entry $base.fra45.ent46  -borderwidth "1" -width "10" 
    label $base.fra45.lab47  -borderwidth "1" -relief "raised" -text ">" 
    entry $base.fra45.ent48  -borderwidth "1" -width "10" 
    bind $base.fra45.ent48 <Key-Return> {
        add_forw l
    }
    menubutton $base.fra45.men49  -indicatoron "1" -menu ".top43.fra45.men49.m" -padx "4" -pady "3"  -relief "groove" -text "Ports" 
    menu $base.fra45.men49.m  -borderwidth "1" -cursor "" -tearoff "0" 
    $base.fra45.men49.m add command  -command "insprot 110 l" -label "pop3" -state "active" 
    $base.fra45.men49.m add command  -command "insprot 143 l" -label "imap" 
    $base.fra45.men49.m add command  -command "insprot 80 l" -label "http" 
    $base.fra45.men49.m add command  -command "insprot 25 l" -label "smtp" 
    $base.fra45.men49.m add command  -command "insprot 23 l" -label "telnet" 
    $base.fra45.men49.m add command  -command "insprot 5901 l" -label "vnc" 
    button $base.fra45.but21  -command "add_forw l" -padx "9" -pady "3" -relief "groove"  -text "Add" 
    button $base.fra45.but17  -command "del_forw l" -padx "9" -pady "3" -relief "groove"  -text "Delete" 
    entry $base.fra45.ent17  -borderwidth "1" 
    label $base.fra45.lab17  -anchor "w" -borderwidth "1"  -text "Host - leave empty for target host:" 
    label $base.fra45.lab33  -anchor "w" -borderwidth "1" -text "Comment:" 
    entry $base.fra45.ent34  -borderwidth "1" 
    label $base.fra45.lab35  -anchor "w" -borderwidth "1" -text "Ports:" 
    button $base.but22  -command "Window destroy .top43" -padx "9" -pady "3" -relief "groove"  -text "Cancel" 
    button $base.but17  -command "save_forwards l" -padx "9" -pady "3" -relief "groove"  -text "OK" 
    label $base.lab25  -anchor "w" 
    label $base.lab26  -anchor "w" -borderwidth "1" -foreground "#000000"  -text "Local forwardings" 
    ###################
    # SETTING GEOMETRY
    ###################
    grid columnconf $base 0 -weight 1
    grid columnconf $base 1 -weight 1
    grid rowconf $base 1 -weight 1
    grid $base.cpd44  -in ".top43" -column "0" -row "1" -columnspan "2" -rowspan "1"  -padx "2" -pady "2" -sticky "nesw" 
    grid columnconf $base.cpd44 0 -weight 1
    grid rowconf $base.cpd44 0 -weight 1
    grid $base.cpd44.01  -in ".top43.cpd44" -column "0" -row "0" -columnspan "1" -rowspan "1"  -sticky "nesw" 
    grid $base.cpd44.02  -in ".top43.cpd44" -column "0" -row "1" -columnspan "1" -rowspan "1"  -sticky "ew" 
    grid $base.cpd44.03  -in ".top43.cpd44" -column "1" -row "0" -columnspan "1" -rowspan "1"  -sticky "ns" 
    grid $base.fra45  -in ".top43" -column "0" -row "3" -columnspan "2" -rowspan "1"  -ipadx "4" -ipady "4" -padx "4" -pady "4" -sticky "ew" 
    grid columnconf $base.fra45 0 -weight 1
    grid columnconf $base.fra45 2 -weight 1
    grid $base.fra45.ent46  -in ".top43.fra45" -column "0" -row "3" -columnspan "1" -rowspan "1"  -padx "4" -sticky "ew" 
    grid $base.fra45.lab47  -in ".top43.fra45" -column "1" -row "3" -columnspan "1" -rowspan "1" 
    grid $base.fra45.ent48  -in ".top43.fra45" -column "2" -row "3" -columnspan "1" -rowspan "1"  -padx "4" -sticky "ew" 
    grid $base.fra45.men49  -in ".top43.fra45" -column "2" -row "6" -columnspan "1" -rowspan "1" 
    grid $base.fra45.but21  -in ".top43.fra45" -column "1" -row "6" -columnspan "1" -rowspan "1" 
    grid $base.fra45.but17  -in ".top43.fra45" -column "0" -row "6" -columnspan "1" -rowspan "1" 
    grid $base.fra45.ent17  -in ".top43.fra45" -column "0" -row "1" -columnspan "3" -rowspan "1"  -padx "4" -sticky "ew" 
    grid $base.fra45.lab17  -in ".top43.fra45" -column "0" -row "0" -columnspan "3" -rowspan "1"  -sticky "ew" 
    grid $base.fra45.lab33  -in ".top43.fra45" -column "0" -row "4" -columnspan "3" -rowspan "1"  -sticky "ew" 
    grid $base.fra45.ent34  -in ".top43.fra45" -column "0" -row "5" -columnspan "3" -rowspan "1"  -padx "4" -sticky "ew" 
    grid $base.fra45.lab35  -in ".top43.fra45" -column "0" -row "2" -columnspan "3" -rowspan "1"  -sticky "ew" 
    grid $base.but22  -in ".top43" -column "0" -row "4" -columnspan "1" -rowspan "1" 
    grid $base.but17  -in ".top43" -column "1" -row "4" -columnspan "1" -rowspan "1" 
    grid $base.lab25  -in ".top43" -column "0" -row "2" -columnspan "2" -rowspan "1"  -padx "2" -sticky "ew" 
    grid $base.lab26  -in ".top43" -column "0" -row "0" -columnspan "2" -rowspan "1"  -padx "2" -pady "2" -sticky "new"
}

proc vTclWindow.top47 {base} {
    if {$base == ""} {
        set base .top47
    }
    if {[winfo exists $base]} {
        wm deiconify $base; return
    }
    ###################
    # CREATING WIDGETS
    ###################
    toplevel $base -class Toplevel  -borderwidth "4" 
    wm focusmodel $base passive
    wm geometry $base 567x303
    wm maxsize $base 1265 994
    wm minsize $base 1 1
    wm overrideredirect $base 0
    wm resizable $base 1 1
    wm title $base "SecPanel - Long listing"
    frame $base.cpd48  -borderwidth "1" -height "30" -width "30" 
    scrollbar $base.cpd48.01  -borderwidth "1" -command ".top47.cpd48.03 xview"  -orient "horizontal" -width "10" 
    scrollbar $base.cpd48.02  -borderwidth "1" -command ".top47.cpd48.03 yview" -width "10" 
    text $base.cpd48.03  -borderwidth "1" -font "fixed" -height "1" -width "8" -wrap "none"  -xscrollcommand ".top47.cpd48.01 set"  -yscrollcommand ".top47.cpd48.02 set" 
    button $base.but49  -command "Window destroy .top47" -relief "groove" -text "Close" 
    ###################
    # SETTING GEOMETRY
    ###################
    grid columnconf $base 0 -weight 1
    grid rowconf $base 0 -weight 1
    grid $base.cpd48  -in ".top47" -column "0" -row "0" -columnspan "1" -rowspan "1"  -padx "2" -pady "2" -sticky "nesw" 
    grid columnconf $base.cpd48 0 -weight 1
    grid rowconf $base.cpd48 0 -weight 1
    grid $base.cpd48.01  -in ".top47.cpd48" -column "0" -row "1" -columnspan "1" -rowspan "1"  -sticky "ew" 
    grid $base.cpd48.02  -in ".top47.cpd48" -column "1" -row "0" -columnspan "1" -rowspan "1"  -sticky "ns" 
    grid $base.cpd48.03  -in ".top47.cpd48" -column "0" -row "0" -columnspan "1" -rowspan "1"  -sticky "nesw" 
    grid $base.but49  -in ".top47" -column "0" -row "1" -columnspan "1" -rowspan "1"  -padx "2" -pady "2"
}

proc vTclWindow.top50 {base} {
    if {$base == ""} {
        set base .top50
    }
    if {[winfo exists $base]} {
        wm deiconify $base; return
    }
    ###################
    # CREATING WIDGETS
    ###################
    toplevel $base -class Toplevel  -borderwidth "4" 
    wm focusmodel $base passive
    wm geometry $base 315x236
    wm maxsize $base 1009 738
    wm minsize $base 1 1
    wm overrideredirect $base 0
    wm resizable $base 1 1
    wm title $base "SecPanel - Edit known hosts"
    frame $base.fra51  -borderwidth "1" -height "75" -width "125" 
    frame $base.fra51.fra53  -height "75" -width "125" 
    button $base.fra51.fra53.but56  -command "hostkey view" -padx "9" -pady "3" -relief "groove"  -text "View" 
    button $base.fra51.fra53.but57  -command "hostkey delete" -padx "9" -pady "3" -relief "groove"  -text "Delete" 
    button $base.fra51.fra53.but18  -command "hostkey export" -padx "9" -pady "3" -relief "groove"  -text "Export" 
    frame $base.fra51.cpd54  -height "30" -relief "raised" -width "30" 
    listbox $base.fra51.cpd54.01  -borderwidth "1" -xscrollcommand ".top50.fra51.cpd54.02 set"  -yscrollcommand ".top50.fra51.cpd54.03 set" 
    bind $base.fra51.cpd54.01 <Double-Button-1> {
        hostkey view
    }
    scrollbar $base.fra51.cpd54.02  -borderwidth "1" -command ".top50.fra51.cpd54.01 xview"  -orient "horizontal" -width "10" 
    scrollbar $base.fra51.cpd54.03  -borderwidth "1" -command ".top50.fra51.cpd54.01 yview" -width "10" 
    label $base.fra51.lab17  -anchor "w" -borderwidth "1" -text "Collected hostkeys" 
    button $base.but38  -command "Window destroy .top50" -padx "9" -pady "3" -relief "groove"  -text "Close" 
    ###################
    # SETTING GEOMETRY
    ###################
    grid columnconf $base 0 -weight 1
    grid rowconf $base 0 -weight 1
    grid $base.fra51  -in ".top50" -column "0" -row "0" -columnspan "1" -rowspan "1"  -ipadx "2" -ipady "2" -padx "2" -pady "2" -sticky "nesw" 
    grid columnconf $base.fra51 0 -weight 1
    grid rowconf $base.fra51 1 -weight 1
    grid $base.fra51.fra53  -in ".top50.fra51" -column "1" -row "1" -columnspan "1" -rowspan "1"  -ipadx "2" -padx "2" -sticky "n" 
    grid $base.fra51.fra53.but56  -in ".top50.fra51.fra53" -column "0" -row "0" -columnspan "1"  -rowspan "1" -sticky "ew" 
    grid $base.fra51.fra53.but57  -in ".top50.fra51.fra53" -column "0" -row "2" -columnspan "1"  -rowspan "1" -sticky "ew" 
    grid $base.fra51.fra53.but18  -in ".top50.fra51.fra53" -column "0" -row "1" -columnspan "1"  -rowspan "1" 
    grid $base.fra51.cpd54  -in ".top50.fra51" -column "0" -row "1" -columnspan "1" -rowspan "1"  -sticky "nesw" 
    grid columnconf $base.fra51.cpd54 0 -weight 1
    grid rowconf $base.fra51.cpd54 0 -weight 1
    grid $base.fra51.cpd54.01  -in ".top50.fra51.cpd54" -column "0" -row "0" -columnspan "1"  -rowspan "1" -sticky "nesw" 
    grid $base.fra51.cpd54.02  -in ".top50.fra51.cpd54" -column "0" -row "1" -columnspan "1"  -rowspan "1" -sticky "ew" 
    grid $base.fra51.cpd54.03  -in ".top50.fra51.cpd54" -column "1" -row "0" -columnspan "1"  -rowspan "1" -sticky "ns" 
    grid $base.fra51.lab17  -in ".top50.fra51" -column "0" -row "0" -columnspan "2" -rowspan "1"  -sticky "ew" 
    grid $base.but38  -in ".top50" -column "0" -row "1" -columnspan "1" -rowspan "1"  -padx "2" -pady "2" -sticky "ew"
}

proc vTclWindow.top51 {base} {
    if {$base == ""} {
        set base .top51
    }
    if {[winfo exists $base]} {
        wm deiconify $base; return
    }
    ###################
    # CREATING WIDGETS
    ###################
    toplevel $base -class Toplevel  -borderwidth "4" -menu ".top51.m20" 
    wm focusmodel $base passive
    wm geometry $base 243x349
    wm maxsize $base 1009 738
    wm minsize $base 1 1
    wm overrideredirect $base 0
    wm resizable $base 1 1
    wm title $base "SecPanel - Remote forwardings"
    frame $base.cpd53  -borderwidth "1" -height "30" -width "30" 
    listbox $base.cpd53.01  -borderwidth "1" -height "5" -xscrollcommand ".top51.cpd53.02 set"  -yscrollcommand ".top51.cpd53.03 set" 
    bind $base.cpd53.01 <Button-1> {
        .top51.cpd53.01 activate @%x,%y
        showcomm r
    }
    scrollbar $base.cpd53.02  -borderwidth "1" -command ".top51.cpd53.01 xview"  -orient "horizontal" -width "10" 
    scrollbar $base.cpd53.03  -borderwidth "1" -command ".top51.cpd53.01 yview" -width "10" 
    button $base.but21  -command "Window destroy .top51" -padx "9" -pady "3" -relief "groove"  -text "Cancel" 
    button $base.but18  -command "save_forwards r" -padx "9" -pady "3" -relief "groove"  -text "OK" 
    label $base.lab24  -anchor "w" 
    frame $base.fra36  -borderwidth "2" -height "75" -relief "groove" -width "125" 
    entry $base.fra36.01  -borderwidth "1" -width "10" 
    label $base.fra36.02  -borderwidth "1" -relief "raised" -text ">" 
    entry $base.fra36.03  -borderwidth "1" -width "10" 
    bind $base.fra36.03 <Key-Return> {
        add_forw r
    }
    menubutton $base.fra36.04  -indicatoron "1" -menu ".top51.fra36.04.05" -padx "4" -pady "3"  -relief "groove" -text "Ports" 
    menu $base.fra36.04.05  -borderwidth "1" -cursor "" -tearoff "0" 
    $base.fra36.04.05 add command  -command "insprot 110 r" -label "pop3" -state "active" 
    $base.fra36.04.05 add command  -command "insprot 143 r" -label "imap" 
    $base.fra36.04.05 add command  -command "insprot 80 r" -label "http" 
    $base.fra36.04.05 add command  -command "insprot 25 r" -label "smtp" 
    $base.fra36.04.05 add command  -command "insprot 23 r" -label "telnet" 
    $base.fra36.04.05 add command  -command "insprot 5901 r" -label "vnc" 
    button $base.fra36.06  -command "add_forw r" -padx "9" -pady "3" -relief "groove"  -text "Add" 
    button $base.fra36.07  -command "del_forw r" -padx "9" -pady "3" -relief "groove"  -text "Delete" 
    entry $base.fra36.08  -borderwidth "1" 
    label $base.fra36.09  -anchor "w" -borderwidth "1"  -text "Host - leave empty for local host:" 
    label $base.fra36.010  -anchor "w" -borderwidth "1" -text "Comment:" 
    entry $base.fra36.011  -borderwidth "1" 
    label $base.fra36.012  -anchor "w" -borderwidth "1" -text "Ports:" 
    label $base.lab27  -anchor "w" -borderwidth "1" -foreground "#000000"  -text "Remote forwardings" 
    ###################
    # SETTING GEOMETRY
    ###################
    grid columnconf $base 0 -weight 1
    grid columnconf $base 1 -weight 1
    grid rowconf $base 1 -weight 1
    grid $base.cpd53  -in ".top51" -column "0" -row "1" -columnspan "2" -rowspan "1"  -padx "2" -pady "2" -sticky "nesw" 
    grid columnconf $base.cpd53 0 -weight 1
    grid rowconf $base.cpd53 0 -weight 1
    grid $base.cpd53.01  -in ".top51.cpd53" -column "0" -row "0" -columnspan "1" -rowspan "1"  -sticky "nesw" 
    grid $base.cpd53.02  -in ".top51.cpd53" -column "0" -row "1" -columnspan "1" -rowspan "1"  -sticky "ew" 
    grid $base.cpd53.03  -in ".top51.cpd53" -column "1" -row "0" -columnspan "1" -rowspan "1"  -sticky "ns" 
    grid $base.but21  -in ".top51" -column "0" -row "4" -columnspan "1" -rowspan "1" 
    grid $base.but18  -in ".top51" -column "1" -row "4" -columnspan "1" -rowspan "1" 
    grid $base.lab24  -in ".top51" -column "0" -row "2" -columnspan "2" -rowspan "1"  -padx "2" -sticky "ew" 
    grid $base.fra36  -in ".top51" -column "0" -row "3" -columnspan "2" -rowspan "1"  -ipadx "4" -ipady "4" -padx "4" -pady "4" -sticky "ew" 
    grid columnconf $base.fra36 0 -weight 1
    grid columnconf $base.fra36 2 -weight 1
    grid $base.fra36.01  -in ".top51.fra36" -column "0" -row "3" -columnspan "1" -rowspan "1"  -padx "4" -sticky "ew" 
    grid $base.fra36.02  -in ".top51.fra36" -column "1" -row "3" -columnspan "1" -rowspan "1" 
    grid $base.fra36.03  -in ".top51.fra36" -column "2" -row "3" -columnspan "1" -rowspan "1"  -padx "4" -sticky "ew" 
    grid $base.fra36.04  -in ".top51.fra36" -column "2" -row "6" -columnspan "1" -rowspan "1" 
    grid $base.fra36.06  -in ".top51.fra36" -column "1" -row "6" -columnspan "1" -rowspan "1" 
    grid $base.fra36.07  -in ".top51.fra36" -column "0" -row "6" -columnspan "1" -rowspan "1" 
    grid $base.fra36.08  -in ".top51.fra36" -column "0" -row "1" -columnspan "3" -rowspan "1"  -padx "4" -sticky "ew" 
    grid $base.fra36.09  -in ".top51.fra36" -column "0" -row "0" -columnspan "3" -rowspan "1"  -sticky "ew" 
    grid $base.fra36.010  -in ".top51.fra36" -column "0" -row "4" -columnspan "3" -rowspan "1"  -sticky "ew" 
    grid $base.fra36.011  -in ".top51.fra36" -column "0" -row "5" -columnspan "3" -rowspan "1"  -padx "4" -sticky "ew" 
    grid $base.fra36.012  -in ".top51.fra36" -column "0" -row "2" -columnspan "3" -rowspan "1"  -sticky "ew" 
    grid $base.lab27  -in ".top51" -column "0" -row "0" -columnspan "2" -rowspan "1"  -padx "2" -pady "2" -sticky "new"
}

proc vTclWindow.top52 {base} {
    if {$base == ""} {
        set base .top52
    }
    if {[winfo exists $base]} {
        wm deiconify $base; return
    }
    ###################
    # CREATING WIDGETS
    ###################
    toplevel $base -class Toplevel  -borderwidth "4" 
    wm focusmodel $base passive
    wm geometry $base 353x420
    wm maxsize $base 1265 994
    wm minsize $base 1 1
    wm overrideredirect $base 0
    wm resizable $base 1 1
    wm title $base "SecPanel - Key-management"
    frame $base.fra18  -borderwidth "2" -height "75" -relief "groove" -width "125" 
    label $base.fra18.lab23  -anchor "w" -borderwidth "1" -text "Comment" 
    entry $base.fra18.ent24  -borderwidth "1" 
    label $base.fra18.lab25  -anchor "w" -borderwidth "1" -text "Keyfile" 
    entry $base.fra18.ent26  -borderwidth "1" 
    button $base.fra18.but27  -command "keygen chpath" -height "1" -padx "9" -pady "3"  -relief "groove" -text "Browse" 
    label $base.fra18.lab28  -anchor "w" -borderwidth "1" -text "Passphrase" 
    entry $base.fra18.ent29  -borderwidth "1" -show "*" 
    checkbutton $base.fra18.che30  -borderwidth "1" -text "No passphrase" -variable "nopass" 
    button $base.fra18.but54  -command "keygen gen" -padx "9" -pady "3" -relief "groove"  -text "Generate keypair" 
    entry $base.fra18.ent25  -borderwidth "1" -show "*" 
    label $base.fra18.lab26  -anchor "w" -borderwidth "1" -text "Repeat pass." 
    checkbutton $base.fra18.che17  -anchor "w" -borderwidth "1" -justify "left"  -text "Enter passphrase
in textmode" -variable "passintext" 
    menubutton $base.fra18.men30  -indicatoron "1" -menu ".top52.fra18.men30.m" -padx "4" -pady "3"  -relief "groove" -text "menu" -textvariable "keytype" 
    menu $base.fra18.men30.m  -cursor "" -tearoff "0" 
    $base.fra18.men30.m add radiobutton  -value "SSH2 RSA" -variable "keytype" -label "SSH2 RSA"
    $base.fra18.men30.m add radiobutton  -value "SSH2 DSA" -variable "keytype" -label "SSH2 DSA"
    $base.fra18.men30.m add radiobutton  -value "SSH1 RSA1" -label "SSH1 RSA1" 
    label $base.fra18.lab31  -anchor "w" -borderwidth "0" -relief "raised" -text "Keytype" 
    label $base.lab32  -anchor "w" -borderwidth "1" -text "Key-generation" 
    label $base.lab33  -anchor "w" -borderwidth "1"  -text "Editing, conversion and information" 
    frame $base.fra34  -borderwidth "2" -height "75" -relief "groove" -width "125" 
    frame $base.fra34.cpd17  -borderwidth "1" -height "30" -width "30" 
    listbox $base.fra34.cpd17.01  -borderwidth "1" -xscrollcommand ".top52.fra34.cpd17.02 set"  -yscrollcommand ".top52.fra34.cpd17.03 set" 
    bind $base.fra34.cpd17.01 <Double-Button-1> {
        keygen info
    }
    scrollbar $base.fra34.cpd17.02  -borderwidth "1" -command ".top52.fra34.cpd17.01 xview"  -orient "horizontal" -width "10" 
    scrollbar $base.fra34.cpd17.03  -borderwidth "1" -command ".top52.fra34.cpd17.01 yview" -width "10" 
    frame $base.fra34.fra20  -borderwidth "1" -height "75" -width "125" 
    button $base.fra34.fra20.but21  -command "keygen info" -padx "9" -pady "3" -relief "groove"  -text "Show information" 
    button $base.fra34.fra20.but22  -command "keygen 1" -padx "9" -pady "3" -relief "groove"  -text "Change passphrase" 
    checkbutton $base.fra34.fra20.che23  -anchor "w" -borderwidth "1" -justify "left"  -text "Change passphrase in textmode" -variable "pwtextmode" 
    button $base.fra34.fra20.but24  -command "keygen del" -padx "9" -pady "3" -relief "groove"  -text "Delete key" 
    menubutton $base.fra34.fra20.men33  -menu ".top52.fra34.fra20.men33.m" -padx "4" -pady "3"  -relief "groove" -state "disabled" -text "Convert key to..." 
    menu $base.fra34.fra20.men33.m  -cursor "" -tearoff "0" 
    $base.fra34.fra20.men33.m add command  -label "SECSH Public Key File Format" 
    $base.fra34.fra20.men33.m add command  -label "OpenSSH compatible" 
    $base.fra34.fra20.men33.m add command  -label "OpenSSH public key" 
    button $base.fra34.fra20.but17  -command "update_keylist" -relief "groove" -text "Refresh list" 
    button $base.but37  -command "Window destroy .top52" -padx "9" -pady "3" -relief "groove"  -text "Close" 
    ###################
    # SETTING GEOMETRY
    ###################
    grid columnconf $base 0 -weight 1
    grid rowconf $base 3 -weight 1
    grid $base.fra18  -in ".top52" -column "0" -row "1" -columnspan "2" -rowspan "1"  -ipadx "2" -ipady "2" -padx "2" -pady "2" -sticky "ew" 
    grid columnconf $base.fra18 1 -weight 1
    grid $base.fra18.lab23  -in ".top52.fra18" -column "0" -row "2" -columnspan "1" -rowspan "1"  -sticky "ew" 
    grid $base.fra18.ent24  -in ".top52.fra18" -column "1" -row "2" -columnspan "1" -rowspan "1"  -pady "2" -sticky "ew" 
    grid $base.fra18.lab25  -in ".top52.fra18" -column "0" -row "3" -columnspan "1" -rowspan "1"  -sticky "ew" 
    grid $base.fra18.ent26  -in ".top52.fra18" -column "1" -row "3" -columnspan "1" -rowspan "1"  -pady "2" -sticky "ew" 
    grid $base.fra18.but27  -in ".top52.fra18" -column "2" -row "3" -columnspan "1" -rowspan "1"  -sticky "nsw" 
    grid $base.fra18.lab28  -in ".top52.fra18" -column "0" -row "4" -columnspan "1" -rowspan "1"  -sticky "ew" 
    grid $base.fra18.ent29  -in ".top52.fra18" -column "1" -row "4" -columnspan "1" -rowspan "1"  -pady "2" -sticky "ew" 
    grid $base.fra18.che30  -in ".top52.fra18" -column "2" -row "4" -columnspan "1" -rowspan "1"  -sticky "w" 
    grid $base.fra18.but54  -in ".top52.fra18" -column "2" -row "6" -columnspan "2" -rowspan "1" 
    grid $base.fra18.ent25  -in ".top52.fra18" -column "1" -row "5" -columnspan "1" -rowspan "1"  -pady "2" -sticky "ew" 
    grid $base.fra18.lab26  -in ".top52.fra18" -column "0" -row "5" -columnspan "1" -rowspan "1"  -sticky "ew" 
    grid $base.fra18.che17  -in ".top52.fra18" -column "2" -row "5" -columnspan "1" -rowspan "1"  -sticky "ew" 
    grid $base.fra18.men30  -in ".top52.fra18" -column "1" -row "6" -columnspan "1" -rowspan "1"  -sticky "ew" 
    grid $base.fra18.lab31  -in ".top52.fra18" -column "0" -row "6" -columnspan "1" -rowspan "1"  -sticky "ew" 
    grid $base.lab32  -in ".top52" -column "0" -row "0" -columnspan "2" -rowspan "1"  -padx "2" -pady "2" -sticky "ew" 
    grid $base.lab33  -in ".top52" -column "0" -row "2" -columnspan "1" -rowspan "1"  -padx "2" -pady "2" -sticky "ew" 
    grid $base.fra34  -in ".top52" -column "0" -row "3" -columnspan "1" -rowspan "1"  -ipadx "2" -ipady "2" -padx "2" -pady "2" -sticky "nesw" 
    grid columnconf $base.fra34 0 -weight 1
    grid rowconf $base.fra34 0 -weight 1
    grid rowconf $base.fra34 1 -weight 1
    grid $base.fra34.cpd17  -in ".top52.fra34" -column "0" -row "0" -columnspan "1" -rowspan "1"  -sticky "nesw" 
    grid columnconf $base.fra34.cpd17 0 -weight 1
    grid rowconf $base.fra34.cpd17 0 -weight 1
    grid $base.fra34.cpd17.01  -in ".top52.fra34.cpd17" -column "0" -row "0" -columnspan "1"  -rowspan "1" -sticky "nesw" 
    grid $base.fra34.cpd17.02  -in ".top52.fra34.cpd17" -column "0" -row "1" -columnspan "1"  -rowspan "1" -sticky "ew" 
    grid $base.fra34.cpd17.03  -in ".top52.fra34.cpd17" -column "1" -row "0" -columnspan "1"  -rowspan "1" -sticky "ns" 
    grid $base.fra34.fra20  -in ".top52.fra34" -column "1" -row "0" -columnspan "1" -rowspan "1"  -sticky "n" 
    grid $base.fra34.fra20.but21  -in ".top52.fra34.fra20" -column "0" -row "0" -columnspan "1"  -rowspan "1" -sticky "ew" 
    grid $base.fra34.fra20.but22  -in ".top52.fra34.fra20" -column "0" -row "1" -columnspan "1"  -rowspan "1" -sticky "ew" 
    grid $base.fra34.fra20.che23  -in ".top52.fra34.fra20" -column "0" -row "2" -columnspan "2"  -rowspan "1" -sticky "ew" 
    grid $base.fra34.fra20.but24  -in ".top52.fra34.fra20" -column "0" -row "4" -columnspan "1"  -rowspan "1" -sticky "ew" 
    grid $base.fra34.fra20.men33  -in ".top52.fra34.fra20" -column "0" -row "3" -columnspan "1"  -rowspan "1" -sticky "ew" 
    grid $base.fra34.fra20.but17  -in ".top52.fra34.fra20" -column "0" -row "5" -columnspan "1"  -rowspan "1" -sticky "ew" 
    grid $base.but37  -in ".top52" -column "0" -row "4" -columnspan "2" -rowspan "1"  -padx "2" -pady "2" -sticky "ew"
}

Window show .
Window show .top17
