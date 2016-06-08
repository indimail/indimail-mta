#!/bin/sh
# the next line restarts using wish \
exec wish "$0" "$@"

proc Browse {mode argum} {
	global filenum fonts browsefont TextTypes
	variable w

	incr filenum
	set BQFcol MistyRose
	if {$mode == "command"} {
	    set stat "[catch {set f [eval open \"|$argum\" r]} msg]"
	} else {
	    if [file isdirectory $argum] {
		dialog .msg Alert {Can not view a Directory}  warning -1 OK
		return
	    }
	    set stat "[catch {set f [open $argum]} msg]"
	}
	if $stat {
	    catch {close $f}
	    dialog .msg Alert $msg warning -1 OK
	    return
	}
	set w .f$filenum
	toplevel $w
	wm protocol $w WM_DELETE_WINDOW {destroy .;return}
    #wm iconify $w
	#wm title $w $argum
	wm title $w "IndiMail ManPage/Command Browser"
	frame $w.mbar -relief raised -bd 3
	pack $w.mbar -side top -fill x
	menubutton $w.mbar.fonts -text "Fonts" -menu $w.mbar.fonts.menu
	menubutton $w.mbar.colors -text "Colors" -menu $w.mbar.colors.menu
	pack $w.mbar.fonts $w.mbar.colors -side left -padx 2m
	menu $w.mbar.fonts.menu
	foreach font $fonts {
	    $w.mbar.fonts.menu add radiobutton -label "[lindex $font 0]" \
		-command "ChangeFont $w.fr.text [lindex $font 1]" \
		-variable browsefont -value "[lindex $font 1]"
	}
	menu $w.mbar.colors.menu
	set dumTypes {foreground background}
	eval lappend dumTypes $TextTypes
	foreach TextType $dumTypes {
	    $w.mbar.colors.menu add command -label $TextType \
		-command "ListColor $w.fr.text $TextType"
	}
	MakeST $w 80 30
	ChangeFont $w.fr.text $browsefont
	SetupTags $w.fr.text
	frame $w.butfr -bd 4 -relief groove
	pack $w.butfr -side bottom -padx 2m -pady 2m
	button $w.butfr.but -text Close -bd 3 -bg $BQFcol \
		-command "destroy $w; exit"
	pack $w.butfr.but -side left -padx 2m -pady 2m
	button $w.butfr.search -text Search -bd 3 -bg $BQFcol \
		-command "DoSel $w.fr.text \$search"
	pack $w.butfr.search -side left -padx 2m -pady 2m
	emacsEntry $w.butfr.term -width 20 -relief sunken -bd 2 \
		-textvariable search -bg peachpuff
	bind $w.butfr.term <Return> "DoSel $w.fr.text \$search"
	pack $w.butfr.term -side left -padx 2m -pady 2m
	ResizeST $w
	$w.fr.text configure -cursor {watch red white}
	update
	while {[gets $f line] >= 0} {
		DoLine $w.fr.text $line
	}
	catch {close $f}
	#wm deiconify $w
	$w.fr.text configure -cursor {top_left_arrow red white}
	$w.fr.text configure -state disabled
	focus $w.butfr.term
	bind $w.butfr.term <Key-Home>  {$w.fr.text see 1.0}
	bind $w.butfr.term <Key-End>   {$w.fr.text see end}
	bind $w.butfr.term <Key-Prior> {$w.fr.text yview scroll -1 pages}
	bind $w.butfr.term <Key-Next>  {$w.fr.text yview scroll 1 pages}
	bind $w.butfr.term <Key-Up>    {$w.fr.text yview scroll -1 units}
	bind $w.butfr.term <Key-Down>  {$w.fr.text yview scroll 1 units}
}

proc DoSel {w term} {

	if {$term == ""} {return}
	$w tag remove Search 1.0 end 
	forAllMatches $w $term "$w tag add Search first last" 
	gotoFirstMatch $w $term
}

proc DoLine {Text line} {
   global TextTypes oldTK

   if {$line == ""} {
	$Text insert end "\n"
	return
   }
   if {$oldTK} {
	set endind end
   } else {
	set endind "end -1 chars"
   }
   regsub -all "\017" $line {} line
   regsub -all "\033\\\[\[0-7\]?m" $line {} dummy
   $Text insert end $dummy
   set lastl [lindex [split [$Text index $endind] .] 0]
   $Text insert end "\n"
   while {[regexp -indices "\033\\\[(\[1-7\])m(\[^\033\]*)\033\\\[0?m" $line dummy typeid matchid]} {
	set type [string index $line [lindex $typeid 0]]
	switch $type {
		1	{set TextType bold}	
		4	{set TextType underline}	
		5	{set TextType blink}	
		7	{set TextType reverse}	
		default {set TextType default}
	}
	if {$TextType != "default"} {
	    eval set match \[string range \$line $matchid\]
	    set ind1 [lindex $dummy 0]
	    set line "[string range $line 0 [expr $ind1 - 1]]$match[string range $line [expr [lindex $dummy 1] + 1] end]"
	    set ind2 [expr $ind1 + [string length $match]]
	    $Text mark set first $lastl.$ind1
	    $Text mark set last $lastl.$ind2
	    $Text tag add $TextType first last
	}
   }
}

proc ChangeFont {w font} {
    global TextTypes
    eval global $TextTypes

    $w configure -font $font
    foreach TextType $TextTypes {
	if {$TextType == "bold" && [regexp {[0-9]x[0-9]} $font]} {
	    $w tag configure $TextType -font "${font}bold"
	} else {
	    $w tag configure $TextType -font $font
	}
    }
}

proc SetupTags {Text} {
   global TextTypes Tags
   eval global $TextTypes

   foreach TextType $TextTypes {
	eval $Text tag configure $TextType $Tags($TextType)
   }
}

proc InitTags {} {
   global TextTypes Tags TextBGColor TextFGColor

   set TextTypes {bold underline blink reverse Search}
   eval global $TextTypes

   set Tags(bold) "-foreground \$bold"
   set Tags(underline) "-foreground \$underline -underline 1"
   set Tags(blink) "-foreground \$blink"
   set Tags(reverse) "-foreground \$reverse"
   set Tags(Search) \
    "-background \$Search -borderwidth 2 -relief flat -font -Adobe-Helvetica-Medium-R-Normal--*-120-*"
}

proc ListColor {win ColType} {
   global X11PATH oldTK

   set w .color
   if {[info commands $w] == ""} {
	if {![file readable $X11PATH/rgb.txt]} {
	    dialog .msg Alert {Couldn't locate the Color Database} error -1 OK
	    return
	}
	toplevel $w
	wm title $w "Color Select"
	message $w.msg -text "Select $ColType Color" -aspect 800 \
		-relief raised -bg PowderBlue
	pack $w.msg -side top -expand 1 -fill both
	makeLB $w.colors -relief sunken
	pack $w.colors -expand 1 -fill both
	if {$oldTK} {tk_listboxSingleSelect $w.colors.l}

	set Colors {}
	set f [open $X11PATH/rgb.txt]
	while {[gets $f line] >= 0} {
	    if {[llength $line] == 4} {
		lappend Colors "[lrange $line 3 end]"
	    }
	}
	close $f
	set Colors [lsort $Colors]
	foreach color $Colors {
	    $w.colors.l insert end $color
	}
	button $w.but -text Cancel -command "destroy $w" -bg MistyRose
	pack $w.but -side top -expand 1 -fill x -padx 5 -pady 5
   } else {
	wm deiconify $w
	raise $w
   }
   $w.msg configure -text "Select $ColType Color"
   if {$ColType == "background"} {
	bind $w.colors.l <Double-Button-1> \
	"set TextBGColor \"\[selection get\]\";$win configure -bg \$TextBGColor"
   } elseif {$ColType == "foreground"} {
	bind $w.colors.l <Double-Button-1> \
	"set TextFGColor \"\[selection get\]\";$win configure -fg \$TextFGColor"
   } elseif {$ColType == "Search"} {
	bind $w.colors.l <Double-Button-1> \
	"set $ColType \"\[selection get\]\"; \
	$win tag configure $ColType -background \$$ColType"
   } else {
	bind $w.colors.l <Double-Button-1> \
	"set $ColType \"\[selection get\]\"; \
	$win tag configure $ColType -foreground \$$ColType"
   }
}
#
# Make Scrolled Text
#
proc MakeST {w width height} {
	global TextBGColor TextFGColor 
	frame $w.fr
	pack $w.fr -side top -padx 2m -pady 2m \
		-expand 1 -fill both
	text $w.fr.text -relief sunken -bd 3 -fg $TextFGColor -bg $TextBGColor \
		-yscrollcommand "$w.fr.scroll set" \
		-cursor {top_left_arrow red white} \
		-width $width -height $height
	scrollbar $w.fr.scroll -command "$w.fr.text yview"
	pack $w.fr.scroll -side right -fill y
	pack $w.fr.text -side left -expand 1 -fill both
	$w.fr.text delete 1.0 end
}
#
# Enable Resizing of Text
#
proc ResizeST {w} {
	update idletasks
        $w.fr.text configure -setgrid 1
	wm minsize $w [lindex [wm grid $w] 0] \
		[lindex [wm grid $w] 1]
}

#
proc dialog {w title text bitmap default args} {
    global button

    # 1. Create the top-level window and divide it into top
    # and bottom parts.

    toplevel $w -class Dialog
    wm title $w $title
    wm iconname $w Dialog
    frame $w.top -relief raised -bd 1
    pack $w.top -side top -fill both
    frame $w.bot -relief raised -bd 1
    pack $w.bot -side bottom -fill both

    # 2. Fill the top part with bitmap and message.

    if {[string length $text] > 300} {
	set text "[string range $text 0 300]"
    }
    message $w.top.msg -width 3i -text $text -relief raised \
	    -font -Adobe-Helvetica-Bold-R-Normal--*-120-* -bg peachpuff
    pack $w.top.msg -side right -expand 1 -fill both -padx 3m -pady 3m
    if {$bitmap != ""} {
	label $w.top.bitmap -bitmap $bitmap
	pack $w.top.bitmap -side left -padx 3m -pady 3m
    }

    # 3. Create a row of buttons at the bottom of the dialog.

    set i 0
    foreach but $args {
	button $w.bot.button$i -text $but -bg MistyRose -command "set button $i"
	if {$i == $default} {
	    frame $w.bot.default -relief sunken -bd 1
	    raise $w.bot.button$i
	    pack $w.bot.default -side left -expand 1 -padx 3m -pady 2m
	    pack $w.bot.button$i -in $w.bot.default -side left \
			-padx 2m -pady 2m -ipadx 2m -ipady 1m
	} else {
	    pack $w.bot.button$i -side left -expand 1 \
		    -padx 3m -pady 3m -ipadx 2m -ipady 1m
	}
	incr i
    }

    # 4. Set up a binding for <Return>, if there's a default,
    # set a grab, and claim the focus too.

    if {$default >= 0} {
	bind $w <Return> $w.bot.button$default flash; \
		set button $default
    }

    set oldFocus [focus]
    tkwait visibility $w
    grab set $w
    focus $w

    # 5. Wait for the user to respond, then restore the focus and
    # return the index of the selected button.

    tkwait variable button
    destroy $w
    focus $oldFocus
    return $button
}

#
# text scan dedicated routines
#

proc forAllMatches {w pattern script} {
	scan [$w index end] %d numLines
	for {set i 1} {$i < $numLines} {incr i} {
		$w mark set last $i.0
		while {[regexp -nocase -indices $pattern \
			[$w get last "last lineend"] indices]} {
			$w mark set first \
				"last + [lindex $indices 0] chars"
			$w mark set last "last + 1 chars \
				+ [lindex $indices 1] chars"
			uplevel $script
		}
	}
}

proc gotoFirstMatch {w pattern} {

	scan [$w index end] %d numLines
	for {set i 1} {$i < $numLines} {incr i} {
		$w mark set last $i.0
		if {[regexp -nocase -indices $pattern \
			[$w get last "last lineend"] indices]} {
			if {$i > 0} {
			    $w yview -pickplace [expr $i -1]
			} else {
			    $w yview -pickplace 0
			}
			break
		}
	}
}

proc makeLB {win args} {
    frame $win -relief raised
    frame $win.f -borderwidth 0
    scrollbar $win.v -command "$win.l yview"
    eval listbox $win.l -xscrollcommand "{$win.h set}" \
      -yscrollcommand "{$win.v set}" $args
    pack $win.l -side left -expand 1 -fill both -in $win.f -padx 2m
    pack $win.v -side left -fill y -in $win.f

    frame $win.g -borderwidth 0
    scrollbar $win.h -command "$win.l xview" -orient horizontal

    frame $win.g.p
    pack $win.h -side left -expand 1 -fill x -in $win.g -padx 2m
    pack $win.g.p -side right -padx 5
    pack $win.g -fill x -side bottom
    pack $win.f -expand 1 -fill both -side top
    return $win
}

#
#
# entry dedicated stuff
#
#
proc emacsInsertSelect {ent} {
    if {![catch {selection get} bf] && \
      ![string match ""  $bf]} {
	$ent insert insert $bf
	tk_entrySeeCaret $ent
	focus $ent
    }
}

proc emacsEntry {args} {
    global oldTK

    set name [eval entry $args]
    if {!$oldTK} {return $name}

    bind $name <Control-a> { %W icursor 0 }
    bind $name <Control-b> {
	%W icursor [expr {[%W index insert] - 1}]
    }
    bind $name <Left> {
	%W icursor [expr {[%W index insert] - 1}]
	%W view [%W index insert]
    }
    bind $name <Control-d> { %W delete insert }
    bind $name <Control-e> { %W icursor end }
    bind $name <Control-f> {
	%W icursor [expr {[%W index insert] + 1}]
    }
    bind $name <Right> {
	%W icursor [expr {[%W index insert] + 1}]
	%W view [%W index insert]
    }
    bind $name <Control-k> { %W delete insert end }
    bind $name <Control-u> { %W delete 0 end }
    bind $name <ButtonPress-2> {emacsInsertSelect %W}
    bind $name <Delete> \
      {tk_entryBackspace %W; tk_entrySeeCaret %W}
    bind $name <BackSpace> \
      {tk_entryBackspace %W; tk_entrySeeCaret %W}
    bind $name <Control-h> \
      {tk_entryBackspace %W; tk_entrySeeCaret %W}
    bind $name <Meta-b> \
      { %W insert insert \002 ;tk_entrySeeCaret %W }
    bind $name <Meta-o> \
      { %W insert insert \017 ;tk_entrySeeCaret %W }
    bind $name <Meta-u> \
      { %W insert insert \037 ; tk_entrySeeCaret %W }
    bind $name <Meta-v> \
      { %W insert insert \026 ; tk_entrySeeCaret %W }
    return $name
}

proc entrySet {win val} { ${win} delete 0 end ; ${win} insert end $val }

#
#
# Composed objects
#
#
proc labelEntry {name opts textvar width init code} {
#
#	name	frame name
#	opts	label options
#	textvar	textvariable on entry, may be supplied as {}
#	init	init value of entry
#	code	code for bind <return> on entry
#
    frame $name
    eval label $name.label $opts
    if [string match {} $textvar] {
	emacsEntry $name.entry -relief sunken \
	-bd 3 -bg BlanchedAlmond -width $width
    } else {
	emacsEntry $name.entry -textvariable $textvar -relief sunken \
	-bd 3 -bg BlanchedAlmond -width $width
    }
    if {![string match {} $init]} {
	$name.entry insert end $init
    }
    pack $name.label -side left
    pack $name.entry -side left -expand 1 -fill x
    bind $name.entry <Return> "$code"
}

proc InitTags {} {
   global TextTypes Tags TextBGColor TextFGColor

   set TextTypes {bold underline blink reverse Search}
   eval global $TextTypes

   set Tags(bold) "-foreground \$bold"
   set Tags(underline) "-foreground \$underline -underline 1"
   set Tags(blink) "-foreground \$blink"
   set Tags(reverse) "-foreground \$reverse"
   set Tags(Search) \
    "-background \$Search -borderwidth 2 -relief flat -font -Adobe-Helvetica-Medium-R-Normal--*-120-*"
}

proc ReadRC {} {
    global env
    global browsefont TextTypes TextFGColor TextBGColor
    eval global $TextTypes

    set file $env(HOME)/.indimail/.filemanrc
    if {[file readable $file]} {
	set stat [catch {set f [open $file r]} msg]
	if {$stat} {
	    dialog .d {Alert} $msg warning -1 OK
	    return
	}
	set mode 0
	while {[gets $f line] >= 0} {
	    if {![regexp {^#} $line]} {
		switch $line {
			{[BUTTONS]} {set mode 1;continue}
			{[GROUPS]} {set mode 2;continue}
			{[ITEMS]} {set mode 3;continue}
			{[BROWSEFONT]} {set mode 4;continue}
			{[BROWSECOLORS]} {set mode 5;continue}
			{[EXTCOMMANDS]} {set mode 6;continue}
		}
		switch $mode {
			1   -
			2   -
			3   -
			6   {}
			4   {set browsefont $line}
			5   {set TextFGColor $line
			     gets $f TextBGColor
			     foreach color $TextTypes {
				gets $f $color
			     }
			    }
		}
	    }
	}
	catch {close $f}
    }
}

wm withdraw .
set oldTK 1
set TK_MAYOR [lindex [split $tk_version .] 0]
set TK_MINOR [lindex [split $tk_version .] 1]
if {$TK_MAYOR >= 4 && $TK_MINOR >= 0} {
    set oldTK 0
}
if {!$oldTK} {
    bind Entry <Return> {set dum 0}
}
set X11PATH {/usr/lib/X11}
set fonts { \
        {default *-Courier-Medium-R-Normal-*-80-*} {fixed fixed} \
        {6x10 6x10} {7x13 7x13} {7x14 7x14} {8x13 8x13} {9x15 9x15} \
        {times8 *times*-r-*-80*} {times12 *times*-r-*-120*} \
        {times14 *times*-r-*-140*} {times18 *times*-r-*-180*} \
        {times24 *times*-r-*-240*}
        }
set browsefont "[lindex [lindex $fonts 0] 1]"
InitTags
ReadRC
set TextBGColor gray41
set TextFGColor white
set bold yellow
set underline green
set blink red
set reverse turquoise
set Search OrangeRed
set filenum 0
set mode [lindex $argv 0]
if {$mode != "command" && $mode != "file"} {
    dialog .msg Alert {Unkown mode should be: file or command}  warning -1 OK
    exit
}
set argv [lrange $argv 1 end]
eval Browse $mode $argv
