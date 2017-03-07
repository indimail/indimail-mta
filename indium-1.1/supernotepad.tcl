#!/bin/sh
# Use the shell to start the wish interpreter: \
exec wish "$0" ${1+"$@"}

# $Log: supernotepad.tcl,v $
# Revision 2.8  2009-02-23 13:50:42+05:30  Cprogrammer
# changed indimaildir to @sharelibdir@
#
# Revision 2.7  2003-09-16 12:33:34+05:30  Cprogrammer
# bug fix - file getting created without newline in the last line
# added -r (restricted mode)
#
# Revision 2.6  2003-08-21 09:27:59+05:30  Cprogrammer
# error checking while opening files added
#
# Revision 2.5  2003-08-11 00:46:56+05:30  Cprogrammer
# added error handling while opening files
#
# Revision 2.4  2003-08-07 22:42:09+05:30  Cprogrammer
# changed user to indimail
#
# Revision 2.3  2003-08-06 22:26:21+05:30  Cprogrammer
# *** empty log message ***
#
# Revision 2.2  2003-05-26 12:58:41+05:30  Cprogrammer
# changed textcolor to white
#
# Revision 2.1  2003-04-07 15:39:48+05:30  Cprogrammer
# tk editor
#
#
# WISH Supernotepad 0.8.0
# by David McClamrock <mcclamrock@locl.net>
# based on Tk NotePad 0.5.0 by Joseph Acosta
# and "textedit.tcl" by Eric Foster-Johnson
# with help from Eric Foster-Johnson,
# Graphical Applications with Tcl & Tk (2nd edition)
# and Christopher Nelson, Tcl/Tk Programmer's Reference

# Copyright © 2001 David H. McClamrock
# Freely available under Maximum Use License for Everyone
# You should have received a copy of this license with this program.
# If you didn't, e-mail the author to get one.

# WISH Supernotepad will work only with Tk 8.0 or greater!

# Initialize a few basic things:
# $Id: supernotepad.tcl,v 2.8 2009-02-23 13:50:42+05:30 Cprogrammer Exp mbhangui $

package require Tcl
if {[package vcompare [package provide Tcl] "8.0"] < 0} {
	error "This program requires at least Tcl version 8.0"
}

package require Tk
if {[package vcompare [package provide Tk] "8.0"] < 0} {
	error "This program requires at least Tk version 8.0"
}
global indimaildir
set idstring "\$Id: supernotepad.tcl,v 2.8 2009-02-23 13:50:42+05:30 Cprogrammer Exp mbhangui $"
set returnstatus [catch {set indimaildir [eval {exec grep ^indimail /etc/passwd | awk -F: {{print $6}}}]} result]
if {$returnstatus != 0} {
	error "failed to get home dir for indimail"
}

wm title . "IndiMail Supernotepad"
#wm geometry . "80x32"
wm minsize . 25 1
wm protocol . WM_DELETE_WINDOW {doExit}
#tk_setPalette background LightSeaGreen selectBackground cyan
set version "0.8.0"
set exitmessage "The contents of this file may have changed. \
	Do you wish to save your changes, if any?"
set exitanswer ""
#focus .

# Make the text area and scrollbars:
set wordwrap word
text .textinhere -xscrollcommand ".xbar set" \
	-yscrollcommand ".ybar set" -width 80 -height 25 \
	-fg black -bg white -wrap $wordwrap \
	-tabs { 32 64 96 128 160 192 224 256 }
set fontsize -adobe-helvetica-medium-r-normal--14-*-*-*-*-*-*
.textinhere configure -font $fontsize -setgrid 1
scrollbar .ybar -width 12 -command ".textinhere yview" -trough black
scrollbar .xbar -width 12 -command ".textinhere xview" -orient horizontal -trough black
grid config .textinhere -row 0  -column 0  -sticky news
grid config .ybar -row 0  -column 1  -sticky news
grid config .xbar -row 1  -column 0 -columnspan 2 -sticky news
grid rowconfigure . 0 -weight 1
grid columnconfigure . 0 -weight 1
focus .textinhere

# Set initial values of variables for bytes of text,
# file name, and unsaved changes:
set savedbytes 0
set bytesnow 0
set currentfile ""
set savechanges ""

# Procedure to check whether unsaved text exists
proc savecheck {} {
	global currentfile savechanges textnow savedbytes bytesnow

	# Find out number of bytes in text widget
	# (minus one undeletable newline at end):
	set textnow [.textinhere get 1.0 {end -1c}]
	#set bytesnow [string bytelength $textnow]
	set x [catch {string bytelength $textnow} bytesnow]
	if { $x } {
		set bytesnow [string length $textnow]
	}

	# Find out when text in widget changes;
	# then indicate that unsaved text exists:
	if { [string match $savedbytes $bytesnow] == 1 } {
                	after 1000 savecheck
	} else {
		set savechanges "Save changes?"
		wm title . "$currentfile - $savechanges"
	}
}

# Check whether text is being entered right after program is loaded:
savecheck

# Procedure to ask user whether unsaved text should be saved
# before moving on (procedure "file_save" is under "File -- Save" below)
proc needsaved {} {
	global exitmessage exitanswer
	set exitanswer [tk_messageBox -message $exitmessage \
		-title "Save changes?" -type yesnocancel \
		-icon question]
	if { [string match $exitanswer yes] == 1 } { file_save }
}

# Create main menu ###########################################

menu .filemenu -bg LightSeaGreen -tearoff 0
.filemenu  add cascade -label "File" -background orange -hidemargin 0 -underline 0 -menu .filemenu.files

# File menu ############################################
menu .filemenu.files -tearoff 0

# Procedure to get ready to remove old contents out of text area:
proc readytogo {} {
	global savechanges exitanswer
	set exitanswer ""
	savecheck
	if { $savechanges != "" } { needsaved }
}

# Procedure to remove old contents from text area:
proc outwithold {} {
	global currentfile savechanges
	set currentfile ""
	.textinhere delete 1.0 end
	set savechanges ""
	wm title . "IndiMail Supernotepad"
}

# Procedure to put contents of new file into text area:
proc inwithnew {} {
	global newfile

	if {[file exists $newfile] && [file isfile $newfile] == 0} {
		tk_messageBox -message "Not a regular file" -title $newfile -type ok
		return 1
	}
	if { [string match $newfile ""] == 0 } {
		if {[catch {open $newfile r} startfile] == 0} {
			if {[catch {set filecontents [read $startfile]} result] == 0} {
				close $startfile
				set filecontents [string trimright $filecontents]
				.textinhere insert insert $filecontents
			} else {
				tk_messageBox -message $result -title $newfile -type ok
			}
		} else {
			tk_messageBox -message $startfile -title $newfile -type ok
		}
	}
}

# Procedure to start checking new contents of text area:
proc findoutnew {} {
	global savedbytes bytesnow
	set textin [.textinhere get 1.0 {end -1c}]
	set x [catch {string bytelength $textin} savedbytes]
	if { $x } {
		set savedbytes [string length $textin]
	}
	set bytesnow $savedbytes
	savecheck
}

# File -- New

.filemenu.files add command -label "New" -underline 0 -command "file_new"
	
proc file_new {} {
	global exitanswer
	readytogo
	if { [string match $exitanswer cancel] == 0 } {
		outwithold
		wm title . "IndiMail Supernotepad"
		findoutnew
	}
}

# File -- Open

.filemenu.files add command -label "Open" -underline 0 \
	-command "file_open"

proc file_open {} {
	global currentfile savechanges newfile exitanswer filetosave
	readytogo
	if { [string match $exitanswer cancel] == 0 } {
		outwithold
		set newfile [tk_getOpenFile]
		inwithnew
		.textinhere mark set insert 1.0
		set savechanges ""
		set currentfile $newfile
		if { $currentfile != "" } {
			wm title . "$currentfile"
			findoutnew
		} else {
			wm title . "IndiMail Supernotepad"
			findoutnew
		}
	} else { 
		unset newfile
		wm title . "IndiMail Supernotepad"
		findoutnew
	}	
}

# Open file from command line, if you wish:
global restricted
if { [info exists argc] } {
	for {set i 0} {$i < [llength $argv]} {incr i} {
		switch -regexp -- [lindex $argv $i] {
	   		-r {
				set restricted 1
			}
	    	default {
				if {[info exists newfile]} {
					puts stderr "USAGE: supernotepad \[-r\] filename"
					puts stderr "multiple files not allowed"
					exit 1
				}
				if {[file exists [lindex $argv $i]]} {
					set newfile [lindex $argv $i]
					inwithnew
					.textinhere mark set insert 1.0
					set currentfile $newfile
					set savechanges ""
					findoutnew
				} else {
					set newfile [lindex $argv $i]
					set currentfile $newfile
					set savechanges ""
				}
				wm title . "$currentfile"
	    	}
		}
	}
}

# File -- Save

.filemenu.files add command -label "Save" -underline 0 \
	-command "file_save" -accelerator Ctrl+s

proc file_save {} {
	global currentfile savechanges
	set filecontents [.textinhere get 1.0 {end -1c}]
	set texttosave [string trimright $filecontents]
	if { $currentfile != "" } {
		if {[catch {open $currentfile w} fileid] == 0} {
			puts $fileid $filecontents
			close $fileid
			set savechanges ""
			wm title . "$currentfile"
			findoutnew
		} else {
			tk_messageBox -message $fileid -title $currentfile -type ok
		}
	} else { file_saveas }
}	

bind . <Control-s> {file_save}
	
# File -- Save As

.filemenu.files add command -label "Save As" -underline 5 \
	-command "file_saveas"

proc file_saveas {} {
	global currentfile savechanges exitanswer newfile filetosave
	set filecontents [.textinhere get 1.0 {end -1c}]
	set texttosave [string trimright $filecontents]
	set filetosave [tk_getSaveFile]
	if { $filetosave != "" } {
		if {[catch {open $filetosave w} fileid] == 0} {
			puts $fileid $filecontents
			close $fileid
			set currentfile $filetosave
			set savechanges ""
			wm title . "$currentfile"
			findoutnew
		} else {
			tk_messageBox -message $fileid -title $filetosave -type ok
		}
	} elseif { [string match $exitanswer yes] } {
		unset filetosave
		set exitanswer cancel
		findoutnew
	} else {
		unset filetosave
		findoutnew
	}
}

.filemenu.files add separator

# File -- Insert File

.filemenu.files add command -label "Insert File" -underline 5 \
	-command "file_insert" -accelerator Shift+F1

proc file_insert {} {
	global exitanswer newfile
	set exitanswer ""
	set newfile [tk_getOpenFile]
	if { [string match $exitanswer cancel] == 0 } {
		findoutnew
		inwithnew
		.textinhere see insert
	} else {
		unset newfile
		findoutnew
	}
}

bind . <Shift-F1> { file_insert }

.filemenu.files add separator

# File -- Exit

.filemenu.files add command -label "Exit" -underline 1 -command doExit

proc doExit {} {
	global savechanges

	set exitanswer ""
	if { $savechanges != "" } { needsaved }
	if { [string match $exitanswer yes] == 1 } {
		file_save; exit
	} elseif { [string match $exitanswer cancel] != 1 } {
		exit
	}
}

# Edit menu ###########################################
# using built-in procedures tk_textCut, tk_textCopy, tk_textPaste

menu .filemenu.edit -tearoff 0
.filemenu add cascade -label "Edit" -background orange -hidemargin 0 -underline 0 -menu .filemenu.edit

# Edit -- Cut

.filemenu.edit add command -label "Cut" -underline 1 \
	-command "tk_textCut .textinhere" -accelerator Ctrl+x
# binding <Control-x> is built-in

# Edit -- Copy

.filemenu.edit add command -label "Copy" -underline 0 \
	-command "tk_textCopy .textinhere" -accelerator Ctrl+c
# binding <Control-c> is built-in

# Edit -- Paste
proc paste_text {} {
	tk_textPaste .textinhere
	.textinhere see insert
}
.filemenu.edit add command -label "Paste" -underline 0 \
	-command "paste_text" -accelerator F11
bind . <F11> {paste_text}

# Edit -- Delete

.filemenu.edit add command -label "Delete" -underline 0 \
	-command ".textinhere delete sel.first sel.last" -accelerator Del

.filemenu.edit add separator

# Edit -- Undo
# Unmodified "Undo" code from Tk NotePad 0.5.0 will be found
# below code for last menu item

.filemenu.edit add command -label "Undo" -underline 1 \
	-command " undo_menu_proc" -accelerator Ctrl+z
bind . <Control-z> { undo_menu_proc }

# Edit -- Redo
# Unmodified "Redo" code from Tk NotePad 0.5.0 will be found
# below code for last menu item

.filemenu.edit add command -label "Redo" -underline 0 \
	-command "redo_menu_proc" -accelerator Ctrl+y
bind . <Control-y> { redo_menu_proc }

.filemenu.edit add separator

# Edit -- Select All

.filemenu.edit add command -label "Select all" -underline 7 \
	-command ".textinhere tag add sel 1.0 end" -accelerator Ctrl+/
# binding <Control-/> is built-in

# Edit -- Time/Date

.filemenu.edit add command -label "Time/Date" -underline 0 -command "printtime"

#procedure to set the time
proc printtime {} {
	set nowtime [clock seconds]
	set clocktime [clock format $nowtime -format "%R %p %D"]
	.textinhere insert insert $clocktime
	savecheck
}

.filemenu.edit add separator

# Edit -- Word Wrap Off

.filemenu.edit add checkbutton -variable wordwrap \
	-label "Word wrap off" -onvalue none -offvalue word \
	-underline 0 -command wraponoroff

proc wraponoroff {} {
	global wordwrap
	if { [string match $wordwrap none] == 1 } {
		.textinhere configure -wrap none
	} else {
		.textinhere configure -wrap word
	}
}

# Search menu ########################################

menu .filemenu.search -tearoff 0
.filemenu add cascade -label "Search" -background orange -hidemargin 0 -underline 0 -menu .filemenu.search

# Search -- Find

.filemenu.search add command -label "Find" -underline 0 \
	-command "search_find" -accelerator Ctrl+F1
bind . <Control-F1> {search_find}

# Procedures for finding text

# Initialize some variables:
set casematch nocase
set searchway forward
set search_for ""

# Set up "Find" box 
proc search_find {} {
	global search_for casematch searchway
	toplevel .findbox
	wm title .findbox "Find"
	tk_setPalette background bisque selectBackground cyan
	label .findbox.findwhat -text "Find what?"
	entry .findbox.enterhere -width 40
	.findbox.enterhere insert 0 $search_for
	if { [string match $search_for ""] == 0 } {
		set searchlength [string length $search_for]
		.findbox.enterhere selection range 0 $searchlength
	}
	button .findbox.findnext -text "Find Next" \
		-default active -command find_text
	button .findbox.cancel -text "Cancel" -default normal \
		-command { destroy .findbox }
	checkbutton .findbox.matchcase -text "Match case?" \
		-variable casematch -onvalue "exact" -offvalue "nocase"
	radiobutton .findbox.up -text "Up" -variable searchway -value "backward"
	radiobutton .findbox.down -text "Down" -variable searchway -value "forward"
	grid config .findbox.findwhat -row 1 -column 0 -sticky news
	grid config .findbox.enterhere -row 1 -column 1 -columnspan 4 -sticky news
	grid config .findbox.findnext -row 2 -column 0 -sticky news
	grid config .findbox.cancel -row 2 -column 1 -sticky news
	grid config .findbox.matchcase -row 2 -column 2 -sticky news
	grid config .findbox.up -row 2 -column 3 -sticky news
	grid config .findbox.down -row 2 -column 4 -sticky news
	bind .findbox.enterhere <Key-Return> { find_text }
	focus .findbox.enterhere
}

# Set search direction and case sensitivity
# (Variables "place," "present_place," and "findlength"
# are set in "proc find_text," below)	
proc whichway {} {
	global casematch searchway place search_for present_place \
		findlength
	if { [string match $casematch nocase] == 1 && [string match \
		$searchway forward] == 1 } {
		set place [.textinhere search -nocase -forward \
			$search_for $present_place end]
	} elseif { [string match $casematch exact] == 1 && [string match \
		$searchway forward] == 1 } {
		set place [.textinhere search -exact -forward \
			$search_for $present_place end]
	} elseif { [string match $casematch nocase] == 1 && [string match \
		$searchway backward] == 1 } {
		set place [.textinhere search -nocase -backward \
			$search_for $present_place 1.0]
	} else {
		set place [.textinhere search -exact -backward \
			$search_for $present_place 1.0]
	}
}

# Actually find some matching text
proc find_text {} {
	global place present_place starting_place findlength \
		replacelength casematch searchway findway \
		search_for
	set starting_place [.textinhere index insert]
	set present_place $starting_place
	set search_for [.findbox.enterhere get]
	set findlength [string length $search_for]
	whichway
	if { $place == "" } { 
		set present_place $starting_place
		tk_messageBox -message "Not Found" \
			-title "Not Found" -type ok
		destroy .findbox
	} else {
		catch { .textinhere tag remove sel sel.first sel.last }
		.textinhere tag add sel $place "$place + $findlength chars"
		.textinhere see $place
		if { [string match $searchway forward] == 1 } {
			.textinhere mark set insert "$place + $findlength chars"
		} else {
			.textinhere mark set insert "$place - $findlength chars"
		}
		focus .textinhere
	}
}

# Search -- Find Next

.filemenu.search add command -label "Find Next" -underline 5 \
	-command "find_text" -accelerator F2

bind . <F2> {find_text}

# Search -- Replace

.filemenu.search add command -label "Replace" -underline 0 \
	-command "search_replace" -accelerator Ctrl+r

bind . <Control-r> {search_replace}

# Procedures for replacing text

# Set up "Replace" box
proc search_replace {} {
	global casematch searchway starting_place
	set searchway forward
	set starting_place [.textinhere index insert]
	toplevel .replacebox
	wm title .replacebox "Replace"
	tk_setPalette background bisque selectBackground cyan
	label .replacebox.findwhat -text "Replace:"
	entry .replacebox.enterhere -width 50
	label .replacebox.replacewith -text "With:"
	entry .replacebox.leavehere -width 50 
	label .replacebox.yesorno -text "Replace?"
	button .replacebox.yesdo -text "Yes" -default active -command replace_one
	button .replacebox.nodont -text "No" -default normal -command find_instance
	button .replacebox.all -text "All" -default normal -command replace_all
	button .replacebox.cancel -text "Cancel" -default normal \
		-command { destroy .replacebox }
	checkbutton .replacebox.matchcase -text "Match case?" -variable casematch \
		-onvalue "exact" -offvalue "nocase"
	radiobutton .replacebox.up -text "Up" -variable searchway \
		-value "backward"
	radiobutton .replacebox.down -text "Down" -variable searchway \
		-value "forward"
	grid config .replacebox.findwhat -row 1 -column 0 -sticky news
	grid config .replacebox.enterhere -row 1 -column 1 -columnspan 7 -sticky news
	grid config .replacebox.replacewith -row 2 -column 0 -sticky news
	grid config .replacebox.leavehere -row 2 -column 1 -columnspan 7 -sticky news
	grid config .replacebox.yesorno -row 3 -column 0 -sticky news
	grid config .replacebox.yesdo -row 3 -column 1 -sticky news
	grid config .replacebox.nodont -row 3 -column 2 -sticky news
	grid config .replacebox.all -row 3 -column 3 -sticky news
	grid config .replacebox.cancel -row 3 -column 4 -sticky news
	grid config .replacebox.up -row 3 -column 5 -sticky news
	grid config .replacebox.down -row 3 -column 6 -sticky news
	grid config .replacebox.matchcase -row 3 -column 7 -sticky news
	bind .replacebox.leavehere <Key-Return> { replace_one }
	focus .replacebox.enterhere
}

# Find an instance of matching text
proc find_instance {} {
	global place present_place findlength casematch \
		searchway search_for replace_with starting_place
	set present_place [.textinhere index insert]
	set search_for [.replacebox.enterhere get]
	set replace_with [.replacebox.leavehere get]
	set findlength [string length $search_for]
	set replacelength [string length $replace_with]
	whichway
	if { $place == "" } { 
		set present_place $starting_place
		tk_messageBox -message "Done" \
			-title "Done" -type ok
		destroy .replacebox
	} else {
		selection clear
		.textinhere tag add sel $place "$place + $findlength chars"
		.textinhere see $place
		if { [string match $searchway forward] == 1 } {
			.textinhere mark set insert "$place + $findlength chars"
		} else {
			.textinhere mark set insert "$place - $findlength chars"
		}
	}
}	

# Replace one instance at a time, with confirmation or disconfirmation
proc replace_one {} {
	global place findlength searchway replacelength savechanges \
		starting_place present_place search_for replace_with currentfile	
	set present_place [.textinhere index insert]
	set search_for [.replacebox.enterhere get]
	set replace_with [.replacebox.leavehere get]
	set findlength [string length $search_for]
	set replacelength [string length $replace_with]
	if { [expr $present_place - $starting_place == 0] } {
		find_instance
	} else {
		.textinhere delete $place "$place + $findlength chars"
		.textinhere insert $place $replace_with
	 	if { [string match $searchway backward] == 1 } {
			.textinhere mark set insert "$place - $replacelength chars"
		}
		find_instance
	}
	set savechanges "Save changes?"
	wm title . "$currentfile - $savechanges"
}

# Replace all instances, without confirmation
proc replace_all {} {
	global place searchway replace_with replacelength savechanges \
		findlength starting_place search_for present_place currentfile
	set starting_place [.textinhere index insert]
	set present_place $starting_place 
	set search_for [.replacebox.enterhere get]
	set replace_with [.replacebox.leavehere get]
	set findlength [string length $search_for]
	set replacelength [string length $replace_with]
	whichway
	if { $place == "" } {
		.textinhere see insert
		tk_messageBox -message "Done" \
			-title "Done" -type ok
		destroy .replacebox
		if { $savechanges == "" } {
			set savechanges "Save changes?"
		}
		wm title . "$currentfile - $savechanges"	
	} else {
		.textinhere delete $place "$place + $findlength chars"
		.textinhere mark set insert $place
		.textinhere insert insert $replace_with
		if { [string match $searchway backward] == 1 } {
			.textinhere mark set insert \
			"$place - $replacelength chars"
		}
		replace_all
	}
}

.filemenu.search add separator

# Search -- Line Number

.filemenu.search add command -label "Line Number" -underline 0 \
	-command "line_number ; linebox"

# Find out what line number the cursor is on
proc line_number {} {
	global lineno
	set youarehere [.textinhere index insert]
	set linanchar [split $youarehere .]
	set lineno [lindex $linanchar 0]
}

# Set up "Line" box
proc linebox {} {
	global lineno
	toplevel .linenum
	wm title .linenum "Line"
	tk_setPalette background bisque selectBackground cyan
	label .linenum.gotoline -text "Go to line:"
	entry .linenum.numberhere -width 6
	bind .linenum.numberhere <Key-Return> { gotoline }
	button .linenum.ok -text "OK" -default active -command { gotoline }
	button .linenum.cancel -text "Cancel" -default normal -command { destroy .linenum }
	.linenum.numberhere insert 0 $lineno
	set linedigits [string length $lineno]
	.linenum.numberhere selection range 0 $linedigits
	grid config .linenum.gotoline -row 1 -column 0 -columnspan 2
	grid config .linenum.numberhere -row 1 -column 2
	grid config .linenum.ok -row 2 -column 0
	grid config .linenum.cancel -row 2 -column 1 -columnspan 2
	focus .linenum.numberhere
}

# Go to another line, identified by number
proc gotoline {} {
	global lineno
	set newlineno [.linenum.numberhere get]
	.textinhere mark set insert $newlineno.0
	.textinhere see insert
	destroy .linenum
}

# HTML menu ########################################
menu .filemenu.html -tearoff 0
.filemenu add cascade -label "HTML" -background orange -hidemargin 0 -underline 0 -menu .filemenu.html

# Procedure to insert starting and ending codes
# and put cursor in the right place
proc dualcodes {} {
	global codestart codend tagonsel
	set tagonsel [.textinhere dump -tag 1.0 end]
	if { [string first "tagon sel" $tagonsel] == -1 } {
		.textinhere insert insert "$codestart$codend"
		set goback [string length $codend]
 		.textinhere mark set insert "[.textinhere index insert] - $goback chars"
	} else {
		.textinhere insert sel.first $codestart
		.textinhere insert sel.last $codend
		set goforth [string length $codend]
		.textinhere mark set insert "sel.last + $goforth chars"
	}	
	selection clear
	savecheck
}

# HTML -- New-HTML

.filemenu.html add command -label "New-HTML" -underline 0 \
	-command "new_html"

proc new_html {} {
	global exitanswer
	readytogo
	if { [string match $exitanswer cancel] == 0 } {
		outwithold
		.textinhere insert 1.0 "<HTML>\n<HEAD>\n\n<TITLE>\
			\n<!--Document title (to be displayed on title bar of browser) goes in\
			space below>\n\n\n\n</TITLE>\n</HEAD>\n\n<BODY\
			TEXT=\"#000000\" LINK=\"#0000FF\" VLINK=\"#FF0000\" ALINK\
			=\"#FF0000\" BGCOLOR=\"#FFFFFF\">\n\
			\n<!--Contents of document (to be displayed in main browser\
			window) go in space below>\n\n\n\n</BODY>\n</HTML>"
		.textinhere mark set insert 7.0
		wm title . "IndiMail Supernotepad"
		findoutnew
	}
}

# HTML -- Convert to HTML

.filemenu.html add command -label "Convert to HTML" -underline 0 \
	-command "convert_to_html"

proc convert_to_html {} {
	global lineno exitanswer
	set textutnunc [.textinhere get 1.0 {end -1c}]
	new_html
	if { [string match $exitanswer cancel] == 0 } {
		.textinhere insert 16.0 $textutnunc\n
		set lastend [.textinhere index end]
		set lastnums [split $lastend .]
		set lastline [lindex $lastnums 0]
		set lastbutfour [expr $lastline - 4]
		.textinhere mark set insert 16.0
		.textinhere insert 16.0 "<P>"
		line_number
		while { $lineno < $lastbutfour } {
			set getline [expr $lineno + 1]
			set nextline [expr $getline + 1]
			set linfin [.textinhere index "$getline.0 lineend"]
			if { [expr $linfin - $getline.0] == 0.0 } {
				.textinhere insert $nextline.0 "<P>"
			}
			set parvelnon [.textinhere get $getline.0 $linfin]
			if { [string first "<P>" $parvelnon] == -1 } {
				if { [string match $parvelnon ""] == 0 } {
					.textinhere insert $getline.0 "<BR>"
				}
			}
			incr lineno
		}
		.textinhere mark set insert 7.0
		savecheck
	}
}

.filemenu.html add separator

# HTML -- Heading

.filemenu.html add command -label "Heading" -underline 0 \
	-command headingbox -accelerator F12

set headingsize H1

# Procedure to set up heading selection box:
proc headingbox {} {
	global headingsize
	toplevel .headingbox
	wm title .headingbox "HTML Heading"
	tk_setPalette background bisque selectBackground cyan
	label .headingbox.text -text "Heading Text:"
	entry .headingbox.enterhere -width 50
	label .headingbox.size -text "Heading Size:"
	tk_optionMenu .headingbox.sizemenu headingsize H1 H2 H3 H4 H5 H6
	button .headingbox.insert -text "Insert" -default active \
		-command insert_heading
	button .headingbox.cancel -text "Cancel" -default normal \
		-command { destroy .headingbox }
	grid config .headingbox.text -row 0  -column 0 -sticky news
	grid config .headingbox.enterhere -row 0  -column 1 -columnspan 3 -sticky news
	grid config .headingbox.size -row 1  -column 0  -sticky news
	grid config .headingbox.sizemenu -row 1  -column 1  -sticky news
	grid config .headingbox.insert -row 1  -column 2 -sticky news
	grid config .headingbox.cancel -row 1  -column 3 -sticky news
	bind .headingbox <Key-Return> { insert_heading }
	focus .headingbox.enterhere
}
bind . <F12> { headingbox }

# Procedure to insert heading in HTML code:
proc insert_heading {} {
	global codestart codend headingsize tagonsel
	set codestart "<$headingsize>"
	set codend "</$headingsize>"
	dualcodes
	set headingtext [.headingbox.enterhere get]
	if { $headingtext != "" } {
		.textinhere insert insert $headingtext
	}
	destroy .headingbox
}

# HTML -- Font

.filemenu.html add command -label "Font" -underline 0 \
	-command fontbox -accelerator Shift+F12

set html_fontsize 0
set html_fontcolor ""
set colorcall ""

proc dumpboxes {} {
	if { [winfo exists .colorbox] } { destroy .colorbox	}
	destroy .fontbox
}

# Procedure to set up font selection box:
proc fontbox {} {
	global color html_fontsize html_fontcolor colorcall
	toplevel .fontbox
	wm title .fontbox "HTML Font"
	tk_setPalette background bisque selectBackground cyan
	label .fontbox.size -text "Font Size:"
	tk_optionMenu .fontbox.sizemenu html_fontsize "-2" "-1" "0" "+1" "+2" "+3" "+4"
	label .fontbox.color -text "Font Color:"
	label .fontbox.colornum -width 12 -textvariable color
	button .fontbox.select -text "Open Color Selector" -command {
		if { [string match $colorcall ""] == 0 } { set colorcall "" }
		colorbox
	}
	button .fontbox.insertcolor -text "Insert Color" -command insert_fontcolor
	button .fontbox.insertsize -text "Insert Size" -command insert_fontsize
	button .fontbox.insertboth -text "Insert Size + Color" \
		-command insert_sizencolor
	button .fontbox.cancel -text "Cancel" -command dumpboxes		
	grid config .fontbox.size -row 0  -column 0 -sticky news
	grid config .fontbox.sizemenu -row 0  -column 1  -sticky news
	grid config .fontbox.color -row 0  -column 2 -sticky news
	grid config .fontbox.colornum -row 0  -column 3 -sticky news
	grid config .fontbox.select -row 1  -column 0 -columnspan 3 -sticky news
	grid config .fontbox.insertcolor -row 1  -column 3 -sticky news
	grid config .fontbox.insertsize -row 2  -column 0 -sticky news
	grid config .fontbox.insertboth -row 2  -column 1 -columnspan 2 -sticky news
	grid config .fontbox.cancel -row 2  -column 3 -sticky news
}

# Procedure to insert font color in HTML code:
proc insert_fontcolor {} {
	global codestart codend color html_fontcolor
	set html_fontcolor $color
	set codestart "<FONT COLOR=\"$html_fontcolor\">"
	set codend "</FONT>"
	dualcodes
	dumpboxes
}

# Procedure to insert font size in HTML code:
proc insert_fontsize {} {
	global codestart codend html_fontsize
	set codestart "<FONT SIZE=\"$html_fontsize\">"
	set codend "</FONT>"
	dualcodes
	dumpboxes
}

# Procedure to insert font size and color in HTML code:
proc insert_sizencolor {} {
	global codestart codend color html_fontsize html_fontcolor
	set html_fontcolor $color
	set codestart "<FONT SIZE=\"$html_fontsize\" COLOR=\"$html_fontcolor\">"
	set codend "</FONT>"
	dualcodes
	dumpboxes
}

bind . <Shift-F12> { fontbox }

.filemenu.html add separator

# HTML -- Anchor

.filemenu.html add command -label "Anchor" -underline 0 \
	-command "anchorbox" -accelerator Shift+F7

set lastanchor ""

proc anchorbox {} {
	toplevel .anchorbox
	wm title .anchorbox "HTML Anchor"
	tk_setPalette background bisque selectBackground cyan
	label .anchorbox.name -text "Name:"
	entry .anchorbox.enterhere -width 25
	button .anchorbox.insert -text "Insert" -default active \
		-command insert_anchor
	button .anchorbox.cancel -text "Cancel" -default normal \
		-command { destroy .anchorbox }
	grid config .anchorbox.name .anchorbox.enterhere \
		.anchorbox.insert .anchorbox.cancel
	bind .anchorbox.enterhere <Key-Return> { insert_anchor }
	focus .anchorbox.enterhere
}

proc insert_anchor {} {
	global lastanchor codestart codend
	set lastanchor [.anchorbox.enterhere get]
	set codestart "<A NAME=\"\#$lastanchor\">"
	set codend </A>
	dualcodes
	destroy .anchorbox
}

bind . <Shift-F7> { anchorbox }

# HTML -- Link

.filemenu.html add command -label "Link" -underline 3 \
	-command "linkbox" -accelerator F7

set linktype "http://www."

# Procedure to set up link entry box:
proc linkbox {} {
	global linktype lastanchor
	toplevel .linkbox
	wm title .linkbox "Link"
	tk_setPalette background bisque selectBackground cyan
	label .linkbox.linkurl -text "Link to URL:"
	entry .linkbox.linkhere -width 60
	.linkbox.linkhere insert 0 $linktype
	label .linkbox.showname -text "Show name:"
	entry .linkbox.namehere -width 60
	button .linkbox.www -text "WWW" -default normal -command {
		if { [string match $linktype "http://www."] == 0 } {
			set linktype "http://www."
			.linkbox.linkhere delete 0 end
			.linkbox.linkhere insert 0 $linktype
		}
	}
	button .linkbox.email -text "E-mail" -default normal -command {
		if { [string match $linktype "mailto:"] == 0 } {
			set linktype "mailto:"
			.linkbox.linkhere delete 0 end
			.linkbox.linkhere insert 0 $linktype
		}
	}
	button .linkbox.ftp -text "FTP" -default normal -command {
		if { [string match $linktype "ftp://"] == 0 } {
			set linktype "ftp://"
			.linkbox.linkhere delete 0 end
			.linkbox.linkhere insert 0 $linktype
		}
	}
	button .linkbox.anchor -text "Anchor" -default normal -command {
		if { [string match $linktype "#"] == 0 } {
			set linktype "#"
			.linkbox.linkhere delete 0 end
			.linkbox.linkhere insert 0 $linktype$lastanchor
		}
	}
	button .linkbox.other -text "Other" -default normal -command {
		if { [string match $linktype ""] == 0 } {
			set linktype ""
			.linkbox.linkhere delete 0 end
		}
	}
	button .linkbox.insert -text "Insert Link" -default active \
		-command insert_link
	button .linkbox.cancel -text "Cancel" -default normal \
		-command { destroy .linkbox }
	grid config .linkbox.linkurl -row 0 -column 0 -sticky news
	grid config .linkbox.linkhere -row 0 -column 1 \
		-columnspan 6 -sticky news
	grid config .linkbox.showname -row 1 -column 0 \
		-columnspan 1 -sticky news
	grid config .linkbox.namehere -row 1 -column 1 \
		-columnspan 6 -sticky news
	grid config .linkbox.www -row 2 -column 0 -sticky news
	grid config .linkbox.email -row 2 -column 1 -sticky news
	grid config .linkbox.ftp -row 2 -column 2 -sticky news
	grid config .linkbox.anchor -row 2  -column 3  -sticky news
	grid config .linkbox.other -row 2 -column 4 -sticky news
	grid config .linkbox.insert -row 2 -column 5 -sticky news
	grid config .linkbox.cancel -row 2 -column 6 -sticky news
	bind .linkbox.linkhere <Key-Return> { insert_link }
	bind .linkbox.namehere <Key-Return> { insert_link }
	focus .linkbox.linkhere
}

# Procedure to insert link:

proc insert_link {} {
	global linktype codestart codend
	set link_place [.textinhere index insert]
	set link_id [.linkbox.linkhere get]
	set link_name [.linkbox.namehere get]
	set codestart "<A HREF=\"$link_id\">"
	set codend </A>
	dualcodes
	if { [string match $link_name ""] == 0 } {
		.textinhere insert insert $link_name
	}
	destroy .linkbox
}

bind . <F7> { linkbox }

.filemenu.html add separator

# HTML -- Image

.filemenu.html add command -label "Image" -underline 1 -command "imagebox"

set alignimage LEFT


proc imagebox {} {
	global alignimage
	toplevel .imagebox
	wm title .imagebox "Insert Image"
	tk_setPalette background bisque selectBackground cyan
	label .imagebox.filename -text "Image file name:"
	entry .imagebox.enterhere -width 40
	label .imagebox.alignment -text "Alignment:"
	tk_optionMenu .imagebox.alignimage alignimage \
		LEFT RIGHT TOP MIDDLE BOTTOM
	label .imagebox.optinfo -text "O P T I O N A L   I N F O R M A T I O N :" -pady 4
	label .imagebox.alt -text "Image description:"
	entry .imagebox.altinhere -width 60
	label .imagebox.horspace -text "Spacing: Horiz"
	entry .imagebox.horizhere -width 2
	label .imagebox.vertspace -text "Vert"
	entry .imagebox.vertinhere -width 2
	label .imagebox.height -text "Height"
	entry .imagebox.heightinhere -width 4
	label .imagebox.width -text "Width"
	entry .imagebox.widthinhere -width 4
	label .imagebox.bordo -text "Border"
	entry .imagebox.bordohere -width 2
	label .imagebox.allinpixels -text "(all in pixels)"
	button .imagebox.insert -text "Insert" -default active -command insert_image
	button .imagebox.cancel -text "Cancel" -default normal -command { destroy .imagebox }
	grid config .imagebox.filename -row 0  -column 0  -columnspan 2 -sticky news
	grid config .imagebox.enterhere -row 0  -column 2 -columnspan 9  -sticky news
	grid config .imagebox.alignment -row 0  -column 10 -columnspan 2  -sticky news
	grid config .imagebox.alignimage -row 0  -column 12  -sticky news
	grid config .imagebox.optinfo -row 1  -column 0 -columnspan 13 -sticky news
	grid config .imagebox.alt -row 2  -column 0  -columnspan 2 -sticky news
	grid config .imagebox.altinhere -row 2  -column 2 -columnspan 11  -sticky news
	grid config .imagebox.horspace -row 3  -column 0  -sticky news
	grid config .imagebox.horizhere -row 3  -column 1  -sticky news
	grid config .imagebox.vertspace -row 3  -column 2  -sticky news
	grid config .imagebox.vertinhere -row 3  -column 3  -sticky news
	grid config .imagebox.height -row 3  -column 4  -sticky news
	grid config .imagebox.heightinhere -row 3  -column 5  -sticky news
	grid config .imagebox.width -row 3  -column 6  -sticky news
	grid config .imagebox.widthinhere -row 3  -column 7  -sticky news
	grid config .imagebox.bordo -row 3  -column 8  -sticky news
	grid config .imagebox.bordohere -row 3  -column 9  -sticky news
	grid config .imagebox.allinpixels -row 3  -column 10 -columnspan 3 -sticky news
	grid config .imagebox.insert -row 4  -column 0 -columnspan 7  -sticky news
	grid config .imagebox.cancel -row 4  -column 7 -columnspan 6  -sticky news
	bind .imagebox <Key-Return> { insert_image }
	focus .imagebox.enterhere
}

proc insert_image {} {
	global alignimage codestart codend
	set img_src [.imagebox.enterhere get]
	set alttext [.imagebox.altinhere get]
	set image_hspace [.imagebox.horizhere get]
	set image_vspace [.imagebox.vertinhere get]
	set imageheight [.imagebox.heightinhere get]
	set imagewidth [.imagebox.widthinhere get]
	set imagebordo [.imagebox.bordohere get]
	if { [string match $alttext ""] == 0 } { set alttext "ALT=\"$alttext\""}
	if { [string match $image_hspace ""] == 0 } {
		set image_hspace " HSPACE=\"$image_hspace\""
	}
	if { [string match $image_vspace ""] == 0 } {
		set image_vspace " VSPACE=\"$image_vspace\""
	}
	if { [string match $imageheight ""] == 0 } {
		set imageheight " HEIGHT=\"$imageheight\"" 
	}
	if { [string match $imagewidth ""] == 0 } {
		set imagewidth " WIDTH=\"$imagewidth\""
	}
	if { [string match $imagebordo ""] == 0 } {
		set imagebordo " BORDER=\"$imagebordo\""
	}
	.textinhere insert insert "<IMG SRC=\"$img_src\" ALIGN=\"$alignimage\" $alttext\
		 $image_hspace$image_vspace$imageheight$imagewidth$imagebordo>"
	destroy .imagebox
}

.filemenu.html add separator

# HTML -- List

.filemenu.html add command -label "List" -underline 2 -command "html_list"

set listtype 1
set liston 0

# Procedure to set up list item entry box:
proc html_list {} {
	global listtype liston
	if { $liston == 1 } { set liston 0 }
	toplevel .html_list
	wm title .html_list "HTML List"
	tk_setPalette background bisque selectBackground cyan
	label .html_list.item -text "Item:"
	entry .html_list.itemhere -width 48
	label .html_list.style -text "Style:"
	tk_optionMenu .html_list.choices listchoice "1-2-3" "A-B-C" "a-b-c" \
		"I-II-III" "i-ii-iii" "Discs" "Circles" "Squares"
	button .html_list.insert -text "Insert" -default active -command insert_item
	button .html_list.done -text "Done" -default normal -command {
		set liston 0
		line_number
		set godown [expr $lineno + 3]
		.textinhere mark set insert $godown.0
		destroy .html_list
	}
	grid config .html_list.item -row 0  -column 0  -sticky news
	grid config .html_list.itemhere -row 0  -column 1 -columnspan 3 -sticky news
	grid config .html_list.style -row 1 -column 0 -sticky news
	grid config .html_list.choices -row 1  -column 1  -sticky news
	grid config .html_list.insert -row 1  -column 2  -sticky news
	grid config .html_list.done -row 1  -column 3  -sticky news
	focus .html_list.itemhere
	bind .html_list.itemhere <Key-Return> { insert_item }
}

# Procedure to create list and insert items:
proc insert_item {} {
	global listchoice listtype liston codestart codend ordo
	set list_item [.html_list.itemhere get]
	if { $liston == 0 } {
		switch $listchoice {
			"1-2-3" {set listtype 1 ; set ordo 1}
			"A-B-C" {set listtype A ; set ordo 1}
			"a-b-c" {set listtype a ; set ordo 1}
			"I-II-III" {set listtype I ; set ordo 1}
			"i-ii-iii" {set listtype i ; set ordo 1}
			"Discs" {set listtype DISC ; set ordo 0}
			"Circles" {set listtype CIRCLE ; set ordo 0}
			"Squares" {set listtype SQUARE ; set ordo 0}
		}
		if { $ordo == 1 } {
			set codestart "<OL TYPE=$listtype>\n\t<LI>"
			set codend "\n</OL>"
		} else {
			set codestart "<UL TYPE=$listtype>\n\t<LI>"
			set codend "\n</UL>"
		}
		set liston 1
	} else {
		set codestart "\n\t<LI>"
		set codend ""
	}
	dualcodes
	if { $list_item != "" } {
		.textinhere insert insert $list_item
	}
	.html_list.itemhere delete 0 end
	focus .html_list.itemhere
}

# HTML -- Table

.filemenu.html add command -label "Table" -underline 0 -command "tablebox"

# Procedure to create Table Setup box:
proc tablebox {} {
	global color tablecolor blankrows blankcols
	toplevel .tablebox
	wm title .tablebox "HTML Table Setup"
	tk_setPalette background bisque selectBackground cyan
	button .tablebox.withdata -text "Make table, enter data" -default active \
		-command { get_tablecodes ; databox ; destroy .tablebox	}
	button .tablebox.nodata -text "Make table, no data" -default normal \
		-command { get_tablecodes ; nodatabox ; destroy .tablebox }
	button .tablebox.cancel -text "Cancel" -default normal \
		-command { destroy .tablebox }
	label .tablebox.optinfo -text "O P T I O N A L   I N F O R M A T I O N :" -pady 4
	label .tablebox.sum -text "Table summary:"
	entry .tablebox.suminhere -width 40
	label .tablebox.horspace -text "Spacing: Horiz"
	entry .tablebox.horizhere -width 2
	label .tablebox.vertspace -text "Vert"
	entry .tablebox.vertinhere -width 2
	label .tablebox.height -text "Height"
	entry .tablebox.heightinhere -width 4
	label .tablebox.width -text "Width"
	entry .tablebox.widthinhere -width 4
	label .tablebox.bordo -text "Border"
	entry .tablebox.bordohere -width 2
	label .tablebox.allinpixels -text "(all in pixels)"
	label .tablebox.cellpad -text "Space inside cells"
	entry .tablebox.padhere -width 2
	label .tablebox.cellspace -text "Space between cells"
	entry .tablebox.spacehere -width 2
	label .tablebox.tablecolor -text "Background color:"
	label .tablebox.colorcode -textvariable color
	button .tablebox.colorsel -text "Select color" -command {
		if { [string match $colorcall ""] == 0 } { set colorcall "" }
		colorbox
	}
	button .tablebox.colordesel -text "Deselect color" -command { set color "" }
	grid config .tablebox.withdata -row 0  -column 0 -columnspan 4 -sticky news
	grid config .tablebox.nodata -row 0  -column 4 -columnspan 4 -sticky news
	grid config .tablebox.cancel -row 0  -column 8 -columnspan 4 -sticky news
	grid config .tablebox.optinfo -row 1  -column 0 -columnspan 11 -sticky news
	grid config .tablebox.sum -row 2  -column 0  -columnspan 2 -sticky news
	grid config .tablebox.suminhere -row 2  -column 2 -columnspan 9  -sticky news
	grid config .tablebox.horspace -row 3  -column 0  -columnspan 2 -sticky news
	grid config .tablebox.horizhere -row 3  -column 2  -sticky news
	grid config .tablebox.vertspace -row 3  -column 3  -sticky news
	grid config .tablebox.vertinhere -row 3  -column 4  -sticky news
	grid config .tablebox.height -row 3  -column 5  -sticky news
	grid config .tablebox.heightinhere -row 3  -column 6  -sticky news
	grid config .tablebox.width -row 3  -column 7  -sticky news
	grid config .tablebox.widthinhere -row 3  -column 8  -sticky news
	grid config .tablebox.bordo -row 3  -column 9  -sticky news
	grid config .tablebox.bordohere -row 3  -column 10  -sticky news
	grid config .tablebox.cellpad -row 4  -column 0 -columnspan 2 -sticky news
	grid config .tablebox.padhere -row 4  -column 2  -sticky news
	grid config .tablebox.cellspace -row 4  -column 3 -columnspan 3 -sticky news
	grid config .tablebox.spacehere -row 4  -column 6  -sticky news
	grid config .tablebox.allinpixels -row 4  -column 7 -columnspan 4 -sticky news
	grid config .tablebox.tablecolor -row 5  -column 0 -columnspan 2 -sticky news
	grid config .tablebox.colorcode -row 5  -column 2 -columnspan 2 -sticky news
	grid config .tablebox.colorsel -row 5  -column 4 -columnspan 3 -sticky news
	grid config .tablebox.colordesel -row 5  -column 7 -columnspan 4 -sticky news
	bind .tablebox <Key-Return> { get_tablecodes ; databox ; destroy .tablebox	}
	focus .tablebox.suminhere
}

# Initialize variables for table attributes:
set tablesum ""
set table_hspace ""
set table_vspace ""
set tableheight ""
set tablewidth ""
set tablebordo ""
set cellpad ""
set cellspace ""
set tablecolor ""

# Procedure to get HTML codes for table attributes from user input:
proc get_tablecodes {} {
	global blankcols blankrows color tablecolor table_hspace table_vspace \
		tableheight tablewidth tablebordo cellpad cellspace tablesum
	set tablesum [.tablebox.suminhere get]
	set table_hspace [.tablebox.horizhere get]
	set table_vspace [.tablebox.vertinhere get]
	set tableheight [.tablebox.heightinhere get]
	set tablewidth [.tablebox.widthinhere get]
	set tablebordo [.tablebox.bordohere get]
	set cellpad [.tablebox.padhere get]
	set cellspace [.tablebox.spacehere get]
	set tablecolor $color
	if { [string match $tablecolor ""] == 0 } {
		set tablecolor " BGCOLOR=\"$tablecolor\""
	}
	if { [string match $tablesum ""] == 0 } {
		set tablesum " SUMMARY=\"$tablesum\""
	}
	if { [string match $table_hspace ""] == 0 } {
		set table_hspace " HSPACE=\"$table_hspace\""
	}
	if { [string match $table_vspace ""] == 0 } {
		set table_vspace " VSPACE=\"$table_vspace\""
	}
	if { [string match $tableheight ""] == 0 } {
		set tableheight " HEIGHT=\"$tableheight\"" 
	}
	if { [string match $tablewidth ""] == 0 } {
		set tablewidth " WIDTH=\"$tablewidth\""
	}
	if { [string match $tablebordo ""] == 0 } {
		set tablebordo " BORDER=\"$tablebordo\""
	}
	if { [string match $cellpad ""] == 0 } {
		set cellpad " CELLPADDING=\"$cellpad\""
	}
	if { [string match $cellspace ""] == 0 } {
		set cellspace " CELLSPACING=\"$cellspace\""
	}
}

# Initialize variables for row and cell attributes:
set horowalign LEFT
set vertrowalign MIDDLE
set horcellalign LEFT
set vertcellalign MIDDLE
set horowin ""
set vertrowin ""
set horcellin ""
set vertcellin ""
set rowspannum ""
set colspannum ""
set rowcolor ""
set cellcolor ""
set tableon 0

# Procedure to get HTML codes for row attributes from user input
# (LEFT and MIDDLE are defaults for horizontal and vertical alignment of contents):
proc get_rowcodes {} {
	global horowalign vertrowalign vertrowin horowin rowcolor
	if { [string match $horowalign LEFT] } {
		set horowin ""
	} else {
		set horowin " ALIGN=\"$horowalign\""
	}
	if { [string match $vertrowalign MIDDLE] } {
		set vertowin ""
	} else {
		set vertrowin " VALIGN=\"$vertrowalign\""
	}
	if { [string match $rowcolor ""] == 0 } {
		set rowcolor " BGCOLOR=\"$rowcolor\""
	}
}

# Procedure to get HTML codes for cell attributes from user input:
proc get_cellcodes {} {
	global colspannum rowspannum horcellalign vertcellalign \
		horcellin vertcellin cellcolor
	set rowspannum [.databox.rowspannum get]
	set colspannum [.databox.colspannum get]
	if { [string match $rowspannum ""] == 0 } {
		set rowspannum " ROWSPAN=\"$rowspannum\""
	}
	if { [string match $colspannum ""] == 0 } {
		set colspannum " COLUMNSPAN=\"$colspannum\""
	}
	if { [string match $horcellalign LEFT] } {
		set horcellin ""
	} else {
		set horcellin " ALIGN=\"$horcellalign\""
	}
	if { [string match $vertcellalign MIDDLE] } {
		set vertcellin ""
	} else {
		set vertcellin " VALIGN=\"$vertcellalign\""
	}
	if { [string match $cellcolor ""] == 0 } {
		set cellcolor " BGCOLOR=\"$cellcolor\""
	}
}

# Procedure to insert HTML codes for beginning of table:
proc make_table {} {
	global tablecolor table_hspace table_vspace horowin vertrowin rowcolor \
		tableheight tablewidth tablebordo cellpad cellspace tablesum
	.textinhere insert insert "<TABLE$tablesum$tablecolor$table_hspace$table_vspace\
		$tableheight$tablewidth$tablebordo$cellpad$cellspace>\
		\n\t<TR$horowin$vertrowin$rowcolor>\n"
}

# Procedure to create HTML Table Data Entry box:
proc databox {} {
	global horowalign vertrowalign horcellalign vertcellalign colspannum rowspannum \
		color rowcolor cellcolor celltype colorcall
	toplevel .databox
	wm title .databox "HTML Table Data Entry"
	tk_setPalette background bisque selectBackground cyan
	label .databox.cellcont -text "Cell contents:"
	entry .databox.continhere -width 50
	set celltype Header
	label .databox.celltype -text "Cell type:"
	tk_optionMenu .databox.cellmenu celltype Header Data
	label .databox.colspan -text "Column span:"
	entry .databox.colspannum -width 2
	.databox.colspannum insert 0 $colspannum
	label .databox.rowspan -text "Row span:"
	entry .databox.rowspannum -width 2
	.databox.rowspannum insert 0 $rowspannum
	button .databox.insert -text "Insert" -default active -command insert_cell
	button .databox.newrow -text "Begin new row" -default normal \
		-command newrow
	button .databox.done -text "Done" -default normal -command {
		if { $tableon == 1 } { .textinhere insert insert "\t</TR>\n</TABLE>\n" }
		set tableon 0
		destroy .databox
	}
	label .databox.optinfo -text "O P T I O N A L   I N F O R M A T I O N :" -pady 4
	label .databox.rowalign -text "Align in row:"
	label .databox.horowalign -text "Horizontal" 
	tk_optionMenu .databox.horowmenu horowalign LEFT CENTER RIGHT
	label .databox.vertrowalign -text "Vertical"
	tk_optionMenu .databox.vertrowmenu vertrowalign \
		TOP MIDDLE BOTTOM BASELINE
	label .databox.cellalign -text "Align in cell:"
	label .databox.horcellalign -text "Horizontal" 
	tk_optionMenu .databox.horcellmenu horcellalign LEFT CENTER RIGHT
	label .databox.vertcellalign -text "Vertical"
	tk_optionMenu .databox.vertcellmenu vertcellalign \
		TOP MIDDLE BOTTOM BASELINE
	button .databox.rowcolorsel -text "Select row color" -command {
		set colorcall row ; colorbox
	}
	button .databox.cellcolorsel -text "Select cell color" -command {
		set colorcall cell ; colorbox
	}
	button .databox.rowcolordesel -text "Deselect row color" -command {
		set color "" ; set rowcolor ""
	}
	button .databox.cellcolordesel -text "Deselect cell color" -command {
		set color "" ; set cellcolor ""
	}
	grid config .databox.cellcont -row 0  -column 0 -sticky news
	grid config .databox.continhere -row 0  -column 1 -columnspan 8 -sticky news
	grid config .databox.celltype -row 1  -column 0  -sticky news
	grid config .databox.cellmenu -row 1  -column 1  -sticky news
	grid config .databox.colspan -row 1  -column 2  -columnspan 2 -sticky news
	grid config .databox.colspannum -row 1  -column 4  -sticky news
	grid config .databox.rowspan -row 1  -column 6  -columnspan 2 -sticky news
	grid config .databox.rowspannum -row 1 -column 8 -sticky news
	grid config .databox.insert -row 2  -column 0 -sticky news
	grid config .databox.newrow -row 2  -column 1 -columnspan 5 -sticky news
	grid config .databox.done -row 2  -column 6 -columnspan 3 -sticky news
	grid config .databox.optinfo -row 3  -column 0 -columnspan 9 -sticky news
	grid config .databox.rowalign -row 4 -column 0 -sticky news
	grid config .databox.horowalign -row 4  -column 1 -sticky news
	grid config .databox.horowmenu -row 4  -column 2 -sticky news
	grid config .databox.vertrowalign -row 4  -column 4 -columnspan 3 -sticky news
	grid config .databox.vertrowmenu -row 4  -column 7  -columnspan 2 -sticky news
	grid config .databox.cellalign -row 5  -column 0  -sticky news
	grid config .databox.horcellalign -row 5  -column 1  -sticky news
	grid config .databox.horcellmenu -row 5  -column 2  -sticky news
	grid config .databox.vertcellalign -row 5  -column 4 -columnspan 3 -sticky news
	grid config .databox.vertcellmenu -row 5 -column 7 -columnspan 2 -sticky news
	grid config .databox.rowcolorsel -row 6  -column 0 -columnspan 3 -sticky news
	grid config .databox.cellcolorsel -row 6  -column 3 -columnspan 6 -sticky news
	grid config .databox.rowcolordesel -row 7  -column 0 -columnspan 3 -sticky news
	grid config .databox.cellcolordesel -row 7  -column 3 -columnspan 6 -sticky news
	bind .databox <Key-Return> { insert_cell }
	focus .databox.continhere
}

# Procedure to insert new data cell in existing row of HTML table:
proc insert_cell {} {
	global codestart codend colspannum rowspannum horowin vertrowin \
		horcellin vertcellin rowcolor cellcolor celltype lineno tableon
	set cellcontents [.databox.continhere get]
	if { $tableon == 0 } {
		get_rowcodes
		get_cellcodes
		make_table
		set tableon 1	
	} else {
		get_cellcodes
	}
	if { [string match $celltype Header] } {
		set codestart \t\t<TH$colspannum$rowspannum$horcellin$vertcellin$cellcolor>
		set codend "</TH>\n"
	} else {
		set codestart \t\t<TD$colspannum$rowspannum$horcellin$vertcellin$cellcolor>
		set codend "</TD>\n"
	}
	dualcodes
	if { [string match $cellcontents "" ] == 0 } {
		.textinhere insert insert $cellcontents
	}
	line_number
	set godownone [expr $lineno + 1]
	.textinhere mark set insert $godownone.0
	.databox.continhere delete 0 end
	focus .databox.continhere
}

# Procedure to insert new row in HTML table with data contents:
proc newrow {} {
	global celltype horowin vertrowin rowcolor
	if { [string match $celltype Header] } { set celltype Data }
	get_rowcodes
	.textinhere insert insert "\t</TR>\n\t<TR$horowin$vertrowin$rowcolor>\n"
}

set blankcols 2
set blankrows 2

# Procedure to create Blank HTML Table setup box:
proc nodatabox {} {
	global blankcols blankrows
	toplevel .nodatabox
	wm title .nodatabox "Blank HTML Table"
	tk_setPalette background bisque selectBackground cyan
	label .nodatabox.blankcols -text "Columns:"
	entry .nodatabox.colsinhere -width 2
	.nodatabox.colsinhere insert 0 $blankcols
	label .nodatabox.blankrows -text "Rows:"
	entry .nodatabox.rowsinhere -width 3
	.nodatabox.rowsinhere insert 0 $blankrows
	button .nodatabox.maketable -text "Make table" -default active -command {
		nodata_table ; destroy .nodatabox
	}
	button .nodatabox.cancel -text "Cancel" -default normal -command {
		destroy .nodatabox
	}
	grid config .nodatabox.blankcols -row 0  -column 0  -sticky news
	grid config .nodatabox.colsinhere -row 0  -column 1  -sticky news
	grid config .nodatabox.blankrows -row 0  -column 2  -sticky news
	grid config .nodatabox.rowsinhere -row 0  -column 3  -sticky news
	grid config .nodatabox.maketable -row 1  -column 0 -columnspan 2 -sticky news
	grid config .nodatabox.cancel -row 1  -column 2 -columnspan 2 -sticky news
	bind .nodatabox <Key-Return> { nodata_table ; destroy .nodatabox }
	focus .nodatabox.colsinhere
}

# Procedure to make blank row for HTML table, with number of columns set by user:
proc ablankrow {} {
	global blankcols horowin vertrowin colspannum rowspannum horcellin vertcellin
	.textinhere insert insert "\t<TR$horowin$vertrowin>\n"
	for { set i 0 } { $i < $blankcols } { incr i } {
		.textinhere insert insert "\t\t<TD$colspannum$rowspannum\
			$horcellin$vertcellin></TD>\n"
	}
	.textinhere insert insert "\t</TR>\n"
}

# Procedure to create blank HTML table, with number of rows and columns set by user:
proc nodata_table {} {
	global blankcols blankrows color tablecolor table_hspace table_vspace \
		tableheight tablewidth tablebordo cellpad cellspace tablesum
	set blankcols [.nodatabox.colsinhere get]
	set blankrows [.nodatabox.rowsinhere get]
	.textinhere insert insert "<TABLE$tablesum$tablecolor$table_hspace$table_vspace\
		$tableheight$tablewidth$tablebordo$cellpad$cellspace>\n"
	for { set i 0 } { $i < $blankrows } { incr i } { ablankrow }
	.textinhere insert insert "</TABLE>\n"
	savecheck
}

.filemenu.html add separator

# HTML -- Paragraph

.filemenu.html add command -label "Paragraph <P>" -underline 0 -command {
	set codestart <P>
	set codend </P>
	dualcodes
	} -accelerator F1

bind . <F1> {
	set codestart <P>
	set codend </P>
	dualcodes
	}

# HTML -- Line Break

.filemenu.html add command -label "Line Break <BR>" -underline 0 -command {
	.textinhere insert insert "<BR>"
	savecheck
	} -accelerator F9

bind . <F9> {
	.textinhere insert insert "<BR>"
	savecheck
	}

# HTML -- Emphasis

.filemenu.html add command -label "Italics <EM>" -underline 0 -command {
	set codestart <EM>
	set codend </EM>
	dualcodes
	} -accelerator F8

bind . <F8> {
	set codestart <EM>
	set codend </EM>
	dualcodes
	}

# HTML -- Strong

.filemenu.html add command -label "Bold <STRONG>" -underline 0 -command {
	set codestart <STRONG>
	set codend </STRONG>
	dualcodes
	} -accelerator F6

bind . <F6> {
	set codestart <STRONG>
	set codend </STRONG>
	dualcodes
	}

# HTML -- Center

.filemenu.html add command -label "Center <CENTER>" -underline 5 -command {
	set codestart <CENTER>
	set codend </CENTER>
	dualcodes
	} -accelerator Shift+F6

bind . <Shift-F6> {
	set codestart <CENTER>
	set codend </CENTER>
	dualcodes
}

# WISH (Tcl/Tk) menu ###################################

menu .filemenu.wish -tearoff 0
.filemenu add cascade -label "IndiMail (Tcl/Tk)" -background orange -hidemargin 0 -underline 0 -menu .filemenu.wish

# WISH -- New Script

.filemenu.wish add command -label "New Script" -underline 0 \
	-command new_wish

proc new_wish {} {
	readytogo
	outwithold
	.textinhere insert 1.0 "#!/bin/sh\n# Use the shell to start the wish\
		interpreter: \\\nexec wish \"\$0\" \$\{1+\"\$@\"\}\n\n#"
	wm title . "IndiMail Supernotepad"
	findoutnew
}

.filemenu.wish add separator

# WISH -- Matching Braces { }

.filemenu.wish add command -label "Curly Braces \{ \}" -underline 0 -command {
	set codestart "{"
	set codend "}"
	dualcodes
	} -accelerator F3

bind . <F3> {
	set codestart "{"
	set codend "}"
	dualcodes
	}

# WISH -- Matching Brackets [ ]

.filemenu.wish add command -label "Square Brackets \[ \]" -underline 0 -command {
	set codestart {[}
	set codend {]}
	dualcodes
	} -accelerator F4

bind . <F4> {
	set codestart {[}
	set codend {]}
	dualcodes
	}

.filemenu.wish add separator

# WISH -- Run Selected Code

.filemenu.wish add command -label "Run Selected Code" -underline 0 \
	-command runcode -accelerator F5

bind . <F5> { runcode }
	
proc runcode {} {
	if {[interp exists testrunner]} {interp delete testrunner}
	set codesel [.textinhere dump -tag 1.0 end]
	if { [string first "tagon sel" $codesel] != -1 } {
		set codetorun [selection get]
		interp create testrunner
		load {} Tk testrunner
		testrunner eval $codetorun
	} else {
		tk_messageBox -message "No Code Selected" \
			-title "No Code" -type ok
	}
}

.filemenu.wish add separator

# WISH -- Grid Config

.filemenu.wish add command -label "Grid Config" -underline 0 \
	-command gridbox -accelerator Ctrl+g

bind . <Control-g> {gridbox}

set rownum 0
set colnum 0
set spannum 1
set stickies "news"

# Procedure for setting up "Grid Config" entry box:
proc gridbox {} {
	global rownum colnum spannum stickies
	toplevel .gridbox
	wm title .gridbox "Grid Config"
	tk_setPalette background bisque selectBackground cyan
	label .gridbox.gridconfig -text "grid config"
	entry .gridbox.widgetname -width 40
	label .gridbox.row -text "-row"
	entry .gridbox.rownum -width 3 -textvariable rownum
	label .gridbox.col -text "-column"
	entry .gridbox.colnum -width 3 -textvariable colnum
	label .gridbox.span -text "-columnspan"
	entry .gridbox.spannum -width 3 -textvariable spannum
	label .gridbox.sticky -text "-sticky"
	entry .gridbox.stickies -width 4
	set stickies $stickies
	.gridbox.stickies insert 0 $stickies
	button .gridbox.insert -text "Insert" -default active -command gridconfig
	button .gridbox.cancel -text "Cancel" -default normal -command {destroy .gridbox}
	grid config .gridbox.gridconfig -row 0 -column 0 -sticky news
	grid config .gridbox.widgetname -row 0 -column 1 \
		-columnspan 5 -sticky news
	grid config .gridbox.row -row 1 -column 0 -sticky news
	grid config .gridbox.rownum -row 1 -column 1 -sticky news
	grid config .gridbox.col -row 1 -column 2 -sticky news
	grid config .gridbox.colnum -row 1 -column 3 -sticky news
	grid config .gridbox.span -row 1 -column 4 -sticky news
	grid config .gridbox.spannum -row 1 -column 5 -sticky news
	grid config .gridbox.sticky -row 2 -column 0 -sticky news
	grid config .gridbox.stickies -row 2 -column 1 -sticky news
	grid config .gridbox.insert -row 2 -column 2 \
		-columnspan 2 -sticky news
	grid config .gridbox.cancel -row 2 -column 4 \
		-columnspan 2 -sticky news
	bind .gridbox.widgetname <Key-Return> {gridconfig}
	bind .gridbox.colnum <Key-Return> {gridconfig}
	bind .gridbox.spannum <Key-Return> {gridconfig}
	bind .gridbox.stickies <Key-Return> {gridconfig}
	focus .gridbox.widgetname
}

# Procedure for inserting "grid config" in WISH (Tcl/Tk) code:

proc gridconfig {} {
	global rownum oldrownum colnum spannum stickies
	set widgetname [.gridbox.widgetname get]
	set oldrownum $rownum
	set rownum [.gridbox.rownum get]
	set colnum [.gridbox.colnum get]
	set spannum [.gridbox.spannum get]
	if { [string match $spannum 1] == 1 } {
		set spanhere ""
	} else {
		set spanhere "-columnspan $spannum"
	}
	set stickies [.gridbox.stickies get]
	.textinhere insert insert "grid config $widgetname -row $rownum \
		-column $colnum $spanhere -sticky $stickies\n"
	incr colnum $spannum
	set spannum 1
	destroy .gridbox
	savecheck
}

# Special menu ########################################################

menu .filemenu.special -tearoff 0
.filemenu add cascade -label "Special" -background orange -hidemargin 0 -underline 3 -menu .filemenu.special

set count 0
foreach {name char html} {
amp       "&" "&amp;"
lt        "<" "&lt;"
gt        ">" "&gt;"
iexcl     "¡" "&iexcl;"
cent      "¢" "&cent;"
pound     "£" "&pound;"
sect      "§" "&sect;"
copy      "©" "&copy;"
reg       "®" "&reg;"
para      "¶" "&para;"
plusmn    "±" "&plusmn;"
frac14    "¼" "&frac14;"
frac12    "½" "&frac12;"
frac34    "¾" "&frac34;"
iquest    "¿" "&iquest;"
bigagrave "À" "&Agrave;"
bigaacute "Á" "&Aacute;"
bigacirc  "Â" "&Acirc;"
bigatilde "Ã" "&Atilde;"
bigauml   "Ä" "&Auml;"
bigaelig  "Æ" "&AElig;"
bigccedil "Ç" "&Ccedil;"
bigegrave "È" "&Egrave;"
bigeacute "É" "&Eacute;"
bigecirc  "Ê" "&Ecirc;"
bigeuml   "Ë" "&Euml;"
bigiacute "Í" "&Iacute;"
bigicirc  "Î" "&Icirc;"
bigiuml   "Ï" "&Iuml;"
bigntilde "Ñ" "&Ntilde;"
bigoacute "Ó" "&Oacute;"
bigocirc  "Ô" "&Ocirc;"
bigouml   "Ö" "&Ouml;"
bigoslash "Ø" "&Oslash;"
biguacute "Ú" "&Uacute;"
bigucirc  "Û" "&Ucirc;"
biguuml   "Ü" "&Uuml;"
szlig     "ß" "&szlig;"
agrave    "à" "&agrave;"
aacute    "á" "&aacute;"
acirc     "â" "&acirc;"
atilde    "ã" "&atilde;"
auml      "ä" "&auml;"
aelig     "æ" "&aelig;"
ccedil    "ç" "&ccedil;"
egrave    "è" "&egrave;"
eacute    "é" "&eacute;"
ecirc     "ê" "&ecirc;"
euml      "ë" "&euml;"
iacute    "í" "&iacute;"
icirc     "î" "&icirc;"
iuml      "ï" "&iuml;"
ntilde    "ñ" "&ntilde;"
oacute    "ó" "&oacute;"
ocirc     "ô" "&ocirc;"
ouml      "ö" "&ouml;"
oslash    "ø" "&oslash;"
uacute    "ú" "&uacute;"
ucirc     "û" "&ucirc;"
uuml      "ü" "&uuml;"
} {
	set special_names($count) $name
	set special_plain($count) $char
	set special_html($count)  $html
	incr count
}

proc putintext {charo} {
	.textinhere insert insert $charo
	focus .textinhere
	savecheck
}

# Procedure for setting up special-character selection box:
proc specialbox {{texttype plain}} {
	global special_names special_plain special_html
	toplevel .spec
	if { [string match $texttype html ] } {
		wm title .spec "Special Characters - HTML"
	} else {
		wm title .spec "Special Characters - Plain Text"
	}
	set buff -adobe-helvetica-bold-r-normal--18-*-*-*-*-*-*
	tk_setPalette background bisque selectBackground cyan

	set row 0
	set col 0
	foreach button [lsort -integer [array names special_names]] {
		set name      [set special_names($button)]
		set chr_plain [set special_plain($button)]
		set chr_html  [set special_html($button)]
		button .spec.$name \
		 -text $chr_plain -font $buff \
		 -command [list putintext [set chr_$texttype] ]
		grid config .spec.$name -sticky news -row $row -col $col
		incr col
		if {$col > 8} {
			set col 0
			incr row
		}
	}
	button .spec.close -text "Close" -font $buff -command { destroy .spec }
	grid config .spec.close -row $row -column $col -columnspan [expr {9 - $col}] -sticky news
}

# Special -- HTML

.filemenu.special add command -label "HTML" -underline 0 -command {
	specialbox html
}

# Special -- Plain Text

.filemenu.special add command -label "Plain Text" -underline 6 -command {
	specialbox plain
}

.filemenu.special add separator

# Special -- Color

.filemenu.special add command -label "Color" -underline 0 -command colorbox

# Procedure for setting up color selection box:
# RGB Color-setting Scale
# from Graphical Applications with Tcl and Tk, 2nd edition, Chapter 3
# by Eric Foster-Johnson
# slightly modified by David McClamrock

proc colorbox {} {
	# Set variables for red, green, blue, color selected,
	# and hexadecimal code color; set background
	# (number 128 will be midpoint of color scale):
	toplevel .colorbox
	wm title .colorbox "Select a Color"
	global red green blue color hex rowcolor cellcolor colorcall
	set red 255
	set green 204
	set blue 153
	set color ""
	set hex black
	tk_setPalette background bisque selectBackground cyan

	# Make a button to show the color selected:
	button .colorbox.color -textvariable color -borderwidth 10 \
		-pady 10 -padx 52 -command { 
			.textinhere insert insert \"[.colorbox.color cget -text]\"
			focus .textinhere
	}
	grid config .colorbox.color -row 0  -column 1 \
		-columnspan 2 -sticky news

	# Establish a procedure for changing the color selected:
	proc modify_color { which_color value } {
		global color red green blue
		switch $which_color {
			red { set red $value }
			green { set green $value }
			blue { set blue $value }
		}
		if { [expr $red + $green + $blue < 480] && \
			[expr $green < 180] } then {
			set hex white
		} else {
			set hex black
		}

		set color [format "#%2.2X%2.2X%2.2X" \
			$red $green $blue]
		.colorbox.color configure -background $color \
			-foreground $hex
	}

	# Make a sliding scale to change the amount of red;
	# show numbers at interval of 64
	label .colorbox.reddo -text "Red"
	scale .colorbox.red -from 0 -to 255 -length 300 -orient horizontal \
		-command "modify_color red" -tickinterval 64
	.colorbox.red set $red
	grid config .colorbox.reddo -row 1  -column 0  -sticky news
	grid config .colorbox.red -row 1  -column 1 -columnspan 4 -sticky news

	# Do the same for green and blue:
	label .colorbox.greeno -text "Green"
	scale .colorbox.green -from 0 -to 255 -length 300 -orient horizontal \
		-command "modify_color green" -tickinterval 64
	.colorbox.green set $green
	grid config .colorbox.greeno -row 2  -column 0  -sticky news
	grid config .colorbox.green -row 2  -column 1  -columnspan 4 -sticky news

	label .colorbox.bluey -text "Blue"
	scale .colorbox.blue -from 0 -to 255 -length 300 -orient horizontal \
		-command "modify_color blue" -tickinterval 64
	.colorbox.blue set $blue
	grid config .colorbox.bluey -row 3  -column 0  -sticky news
	grid config .colorbox.blue -row 3  -column 1  -columnspan 4 -sticky news

	# Make a button to close the color selection box:
	button .colorbox.close -text "Close" -borderwidth 10 \
		-default active -command {
			if { [string match $colorcall row] }	{	set rowcolor $color }
			if { [string match $colorcall cell] } { set cellcolor $color }
			destroy .colorbox
	}
	grid config .colorbox.close -row 4  -column 1 -columnspan 2 -sticky news
}

# Help menu ########################################################

menu .filemenu.help -tearoff 0
.filemenu add cascade -label "Help" -background orange -hidemargin 0 -underline 2 -menu .filemenu.help

# Help -- About WISH Supernotepad

.filemenu.help add command -label "About IndiMail Supernotepad" \
	-underline 0 -command {
		tk_messageBox -message "IndiMail Supernotepad $version\n\
			by David McClamrock\n <mcclamrock@locl.net>\n\n\
			Based on Tk NotePad 0.5.0\n by Joseph Acosta\n\
			and \"textedit.tcl\"\n by Eric Foster-Johnson\n"\
			-title "About IndiMail Supernotepad" -type ok
	}

.filemenu.help add separator

# Help -- User Help

.filemenu.help add command -label "User Help" -underline 0 -command userhelp

# Procedure for setting up user help display
proc userhelp {} {
	global fontsize indimaildir
	toplevel .userhelp
	wm title .userhelp "IndiMail Supernotepad - User Help"
	button .userhelp.find -text "Find (F2)" -command findhelp
	entry .userhelp.lookup -bd 2
	text .userhelp.text -width 80 -height 32 -bg white -fg black \
		-font $fontsize -yscrollcommand ".userhelp.scrolly set" -wrap word
	scrollbar .userhelp.scrolly -width 12 -command ".userhelp.text yview" -trough black
	button .userhelp.close -text "Close" -borderwidth 4 -bg orange -command { destroy .userhelp }
	if {[catch {open @sharelibdir@/userhelp.txt r} helpfile] == 0} {
		set helpcontents [read $helpfile]
		close $helpfile
	} else {
		tk_messageBox -message $helpfile -title "userhelp.txt" -type ok
	}
	.userhelp.text insert 1.0 $helpcontents
	.userhelp.text mark set insert 1.0
	.userhelp.text see 1.0
	grid config .userhelp.find    -row 0 -column 0  -sticky news
	grid config .userhelp.lookup  -row 0 -column 1  -columnspan 1 -sticky news
	grid config .userhelp.close   -row 0 -column 2  -columnspan 2 -sticky news
	grid config .userhelp.text    -row 1 -column 0  -columnspan 3 -sticky news
	grid config .userhelp.scrolly -row 1 -column 3  -sticky news
	grid rowconfigure .userhelp 1 -weight 1
	grid columnconfigure .userhelp 1 -weight 1
	bind .userhelp <F2> { findhelp }
	focus .userhelp.lookup
}

# Procedure for searching for user help text
proc findhelp {} {
	set startout [.userhelp.text index insert]
	set wherenow $startout
	set look_for [.userhelp.lookup get]
	set stringlength [string length $look_for]
	set foundit [.userhelp.text search -nocase -forward $look_for \
		$wherenow end]
	if { $foundit == "" } { 
		set wherenow $startout
		tk_messageBox -message "Not Found" \
			-title "Not Found" -type ok
	} else {
		catch { .userhelp.text tag remove sel sel.first sel.last }
		.userhelp.text tag add sel $foundit "$foundit + $stringlength chars"
		.userhelp.text mark set insert "$foundit + $stringlength chars"
		.userhelp.text see insert
		focus .userhelp.text
	}
}

# At last, make the menu visible ###########################################

. configure -menu .filemenu

####################################################################

# APPENDIX
# Unmodified code for unlimited undo and redo from Tk NotePad 0.5.0 
# (ugly and incomprehensible--to me, at least--but it works!)

#set zed_dir [file dirname [info script]] 
# here is where the undo stuff begins
if {![info exists classNewId]} {
    # work around object creation between multiple include of this file problem
    set classNewId 0
}

proc new {className args} {
    # calls the constructor for the class with optional arguments
    # and returns a unique object identifier independent of the class name

    global classNewId
    # use local variable for id for new can be called recursively
    set id [incr classNewId]
    if {[llength [info procs ${className}:$className]]>0} {
        # avoid catch to track errors
        eval ${className}:$className $id $args
    }
    return $id
}

proc delete {className id} {
    # calls the destructor for the class and delete all the object data members

    if {[llength [info procs ${className}:~$className]]>0} {
        # avoid catch to track errors
        ${className}:~$className $id
    }
    global $className
    # and delete all this object array members if any (assume that they were stored as $className($id,memberName))
    foreach name [array names $className "$id,*"] {
        unset ${className}($name)
    }
}

proc lifo:lifo {id {size 2147483647}} {
    global lifo
    set lifo($id,maximumSize) $size
    lifo:empty $id
}

proc lifo:push {id data} {
    global lifo

    lifo:tidyUp $id
    if {$lifo($id,size)>=$lifo($id,maximumSize)} {
        unset lifo($id,data,$lifo($id,first))
        incr lifo($id,first)
        incr lifo($id,size) -1
    }
    set lifo($id,data,[incr lifo($id,last)]) $data
    incr lifo($id,size)
}

proc lifo:pop {id} {
    global lifo
    lifo:tidyUp $id
    if {$lifo($id,last)<$lifo($id,first)} {
        error "lifo($id) pop error, empty"
    }
    # delay unsetting popped data to improve performance by avoiding a data copy
    set lifo($id,unset) $lifo($id,last)
    incr lifo($id,last) -1
    incr lifo($id,size) -1
    return $lifo($id,data,$lifo($id,unset))
}

proc lifo:tidyUp {id} {
    global lifo
    if {[info exists lifo($id,unset)]} {
        unset lifo($id,data,$lifo($id,unset))
        unset lifo($id,unset)
    }
}

proc lifo:empty {id} {
    global lifo
    lifo:tidyUp $id
    foreach name [array names lifo $id,data,*] {
        unset lifo($name)
    }
    set lifo($id,size) 0
    set lifo($id,first) 0
    set lifo($id,last) -1
}


proc textUndoer:textUndoer {id widget {depth 2147483647}} {
    global textUndoer

    if {[string compare [winfo class $widget] Text]!=0} {
        error "textUndoer error: widget $widget is not a text widget"
    }
    set textUndoer($id,widget) $widget
    set textUndoer($id,originalBindingTags) [bindtags $widget]
    bindtags $widget [concat $textUndoer($id,originalBindingTags) UndoBindings($id)]

    bind UndoBindings($id) <Control-u> "textUndoer:undo $id"

    # self destruct automatically when text widget is gone
    bind UndoBindings($id) <Destroy> "delete textUndoer $id"

    # rename widget command
    rename $widget [set textUndoer($id,originalCommand) textUndoer:original$widget]
    # and intercept modifying instructions before calling original command
    proc $widget {args} "textUndoer:checkpoint $id \$args; 
		global search_count;
		eval $textUndoer($id,originalCommand) \$args"

    set textUndoer($id,commandStack) [new lifo $depth]
    set textUndoer($id,cursorStack) [new lifo $depth]
    #lee 
    textRedoer:textRedoer $id $widget $depth 
}


proc textUndoer:~textUndoer {id} {
    global textUndoer

    bindtags $textUndoer($id,widget) $textUndoer($id,originalBindingTags)
    rename $textUndoer($id,widget) ""
    rename $textUndoer($id,originalCommand) $textUndoer($id,widget)
    delete lifo $textUndoer($id,commandStack)
    delete lifo $textUndoer($id,cursorStack)
    #lee
    textRedoer:~textRedoer $id
}

proc textUndoer:checkpoint {id arguments} {
    global textUndoer
    #lee
    global textRedoer

    # do nothing if non modifying command
    if {[string compare [lindex $arguments 0] insert]==0} {
        textUndoer:processInsertion $id [lrange $arguments 1 end]
        if {$textRedoer($id,redo) == 0} {
           textRedoer:reset $id
        }
    }
    if {[string compare [lindex $arguments 0] delete]==0} {
        textUndoer:processDeletion $id [lrange $arguments 1 end]
        if {$textRedoer($id,redo) == 0} {
           textRedoer:reset $id
        }
    }
}

proc textUndoer:processInsertion {id arguments} {
    global textUndoer

    set number [llength $arguments]
    set length 0
    # calculate total insertion length while skipping tags in arguments
    for {set index 1} {$index<$number} {incr index 2} {
        incr length [string length [lindex $arguments $index]]
    }
    if {$length>0} {
        set index [$textUndoer($id,originalCommand) index [lindex $arguments 0]]
        lifo:push $textUndoer($id,commandStack) "delete $index $index+${length}c"
        lifo:push $textUndoer($id,cursorStack) [$textUndoer($id,originalCommand) index insert]
    }
}

proc textUndoer:processDeletion {id arguments} {
    global textUndoer

    set command $textUndoer($id,originalCommand)
    lifo:push $textUndoer($id,cursorStack) [$command index insert]

    set start [$command index [lindex $arguments 0]]
    if {[llength $arguments]>1} {
        lifo:push $textUndoer($id,commandStack) "insert $start [list [$command get $start [lindex $arguments 1]]]"
	#I changed line above : instead "{ [$command ...] }" -> " [list [$command ...] ]"
	#See explanation in file undo
    } else {
        lifo:push $textUndoer($id,commandStack) "insert $start [list [$command get $start]]"
	#I changed line above : instead "{ [$command ...] }" -> " [list [$command ...] ]"
	#See explanation in file undo
    }
}

proc textUndoer:undo {id} {
    global textUndoer

    if {[catch {set cursor [lifo:pop $textUndoer($id,cursorStack)]}]} {
        return
    }
    
    set popArgs [lifo:pop $textUndoer($id,commandStack)] 
    textRedoer:checkpoint $id $popArgs
    
    eval $textUndoer($id,originalCommand) $popArgs
#    eval $textUndoer($id,originalCommand) [list [lifo:pop $textUndoer($id,commandStack)] ]
    # now restore cursor position
    $textUndoer($id,originalCommand) mark set insert $cursor
    # make sure insertion point can be seen
    $textUndoer($id,originalCommand) see insert
}


proc textUndoer:reset {id} {
    global textUndoer
    lifo:empty $textUndoer($id,commandStack)
    lifo:empty $textUndoer($id,cursorStack)
}

#########################################################################
proc textRedoer:textRedoer {id widget {depth 2147483647}} {
    global textRedoer
    if {[string compare [winfo class $widget] Text]!=0} {
        error "textRedoer error: widget $widget is not a text widget"
    }
    set textRedoer($id,commandStack) [new lifo $depth]
    set textRedoer($id,cursorStack) [new lifo $depth]
    set textRedoer($id,redo) 0
}

proc textRedoer:~textRedoer {id} {
    global textRedoer
    delete lifo $textRedoer($id,commandStack)
    delete lifo $textRedoer($id,cursorStack)
}


proc textRedoer:checkpoint {id arguments} {
    global textUndoer
    global textRedoer
    # do nothing if non modifying command
    if {[string compare [lindex $arguments 0] insert]==0} {
        textRedoer:processInsertion $id [lrange $arguments 1 end]
    }
    if {[string compare [lindex $arguments 0] delete]==0} {
        textRedoer:processDeletion $id [lrange $arguments 1 end]
    }
}

proc textRedoer:processInsertion {id arguments} {
    global textUndoer
    global textRedoer
    set number [llength $arguments]
    set length 0
    # calculate total insertion length while skipping tags in arguments
    for {set index 1} {$index<$number} {incr index 2} {
        incr length [string length [lindex $arguments $index]]
    }
    if {$length>0} {
        set index [$textUndoer($id,originalCommand) index [lindex $arguments 0]]
        lifo:push $textRedoer($id,commandStack) "delete $index $index+${length}c"
        lifo:push $textRedoer($id,cursorStack) [$textUndoer($id,originalCommand) index insert]
    }
}

proc textRedoer:processDeletion {id arguments} {
    global textUndoer
    global textRedoer
    set command $textUndoer($id,originalCommand)
    lifo:push $textRedoer($id,cursorStack) [$command index insert]

    set start [$command index [lindex $arguments 0]]
    if {[llength $arguments]>1} {
        lifo:push $textRedoer($id,commandStack) "insert $start [list [$command get $start [lindex $arguments 1]]]"
	#I changed line above : instead "{ [$command ...] }" -> " [list [$command ...] ]"
	#See explanation in file undo
    } else {
        lifo:push $textRedoer($id,commandStack) "insert $start [list [$command get $start]]"
	#I changed line above : instead "{ [$command ...] }" -> " [list [$command ...] ]"
	#See explanation in file undo.txt
    }
}
proc textRedoer:redo {id} {
    global textUndoer
    global textRedoer
    if {[catch {set cursor [lifo:pop $textRedoer($id,cursorStack)]}]} {
        return
    }
    set textRedoer($id,redo) 1
    set popArgs [lifo:pop $textRedoer($id,commandStack)] 
    textUndoer:checkpoint $id $popArgs
    eval $textUndoer($id,originalCommand) $popArgs
    set textRedoer($id,redo) 0
    # now restore cursor position
    $textUndoer($id,originalCommand) mark set insert $cursor
    # make sure insertion point can be seen
    $textUndoer($id,originalCommand) see insert
}


proc textRedoer:reset {id} {
    global textRedoer
    lifo:empty $textRedoer($id,commandStack)
    lifo:empty $textRedoer($id,cursorStack)
}

# end of where youd source in undo.tcl

set undo_id [new textUndoer .textinhere]
proc undo_menu_proc {} {
	global undo_id
	textUndoer:undo $undo_id
	savecheck
}

proc redo_menu_proc {} {
	global undo_id
	textRedoer:redo $undo_id
	savecheck
}

if {[info exists restricted] && $restricted == 1} {
	.filemenu.files   entryconfigure 0 -state disabled
	.filemenu.files   entryconfigure 1 -state disabled
	.filemenu.files   entryconfigure 3 -state disabled
	.filemenu         entryconfigure 3 -state disabled
	.filemenu         entryconfigure 4 -state disabled
	.filemenu         entryconfigure 5 -state disabled
}
