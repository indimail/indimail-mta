if {[info exists ::hugelist::version]} { return }
namespace eval ::hugelist \
{
# beginning of ::hugelist namespace definition

  namespace export hugelist

# ####################################
#
#   hugelist widget
#
set version 1.3
#
#   ulis, (C) 2002-2003
#
# ------------------------------------
# 1.0 (2002-12-08)
#   first publication 
# ------------------------------------
# 1.1 (2003-02-09)
#   new -left item option 
#   new -rowheight widget option 
# ------------------------------------
# 1.2 (2003-03-01)
#   dynamic resizing (Scott Gamon, 2003-02-27)
#   parameters management 
#   new -version & -user options
# ------------------------------------
# 1.2.1 (2003-03-29)
#   bug: -listvar option doesn't resize 
# ------------------------------------
# 1.2.2 (2003-03-30)
#   bug: insert & delete don't resize 
# ------------------------------------
# 1.3 
#   new: item font
#   new: info operation
# ####################################

  # ==========================
  #
  # debugging
  #
  # ==========================

  proc ::PUTS {args} {}
  if {[info exists ::DEBUG] && $::DEBUG} \
  {
    uplevel 1 \
    {
      rename proc ::PROC
      ::PROC proc {name parms script} \
      {
        uplevel 1 [list ::PROC $name $parms [concat ::hugelist::debug \; $script]]
      }
      ::PROC ::hugelist::debug {} \
      {
        uplevel 1 { puts ..[string repeat "| " [info level]][info level 0] }
      }
    }
  }

  # ==========================
  #
  # package
  #
  # ==========================

  package provide Hugelist $version
  package provide hugelist $version

  package require Tcl 8.4
  package require Tk 8.4
 
 global libdir
  if {[catch {package require Ranges 1.1}]} \
  {
	if {[info exists libdir] && $libdir != ""} {
		source $libdir/ranges.tcl
	} else {
    	source [file join [file dirname [info script]] ranges.tcl]
	}
    package require Ranges 1.1
  }
  namespace import ::ranges::ranges
  
  # ====================
  #
  # entry point
  #
  # ====================
  
    proc hugelist {args} \
    {
      set rc [catch \
      {
        switch -glob -- $args \
        {
          opt*    { eval [linsert [lrange $args 1 end] 0 option] }
          ver*    { set ::hugelist::version }
          .*      { eval [linsert $args 0 create] }
          default { error "unknown option \"[lindex $args 0]\": should be pathName, item or version" }
        }
      } msg]
      # return result
      if {$rc} { set code error } else { set code ok }
      return -code $code $msg
    }
  
  # ==========================
  #
  # widget options
  #
  # ==========================

  # widget default config
  set w ._hugelist_listbox_test_
  listbox $w
  set keys \
  {
    activestyle
    background
    borderwidth
    cursor
    disabledforeground
    exportselection
    font
    foreground
    height
    highlightbackground
    highlightcolor
    highlightthickness
    listvariable
    relief
    selectbackground
    selectborderwidth
    selectforeground
    selectmode
    state
    takefocus
    width
    xscrollcommand
    yscrollcommand
  }
  foreach key $keys { set $key [$w cget -$key] }
  eval [format {
  set woptions \
  {
    {-activestyle activeStyle ActiveStyle {%s}}
    {-alternatecolor alternatecolor Background {}}
    {-background background Background {%s}}
    {-bd -borderwidth}
    {-bg -background}
    {-borderwidth borderWidth BorderWidth {%s}}
    {-colwidth colWidth ColWidth {}}
    {-cursor cursor Cursor {%s}}
    {-deep deep Deep 6}
    {-disabledforeground disabledForeground DisabledForeground {%s}}
    {-exportselection exportSelection ExportSelection {%s}}
    {-fg -foreground}
    {-font font Font {}}
    {-foreground foreground Foreground {%s}}
    {-height height Height %s}
    {-highlightbackground highlightBackground HighlightBackground {%s}}
    {-highlightcolor highlightColor HighlightColor {%s}}
    {-highlightthickness highlightThickness HighlightThickness {%s}}
    {-img -image}
    {-image image Image {}}
    {-ipadx ipadx Pad 0}
    {-ipady ipady Pad 4}
    {-left left Left 0}
    {-listvariable listVariable Variable {%s}}
    {-relief relief Relief {%s}}
    {-rowheight rowHeight RowHeight {}}
    {-selectbackground selectBackground Background {%s}}
    {-selectborderwidth selectBorderWidth BorderWidth {%s}}
    {-selectforeground selectForeground Foreground {%s}}
    {-selectmode selectMode SelectMode {%s}}
    {-setgrid setGrid SetGrid 0}
    {-state state State {%s}}
    {-takefocus takeFocus TakeFocus {%s}}
    {-text text Text {}}
    {-user user User {}}
    {-values values Values {}}
    {-width width Width {%s}}
    {-xscrollcommand xScrollCommand ScrollCommand {%s}}
    {-yscrollcommand yScrollCommand ScrollCommand {%s}}
  }
          } $activestyle $background \
            $borderwidth $cursor $disabledforeground \
            $exportselection \
            $foreground $height $highlightbackground \
            $highlightcolor $highlightthickness \
            $listvariable $relief \
            $selectbackground $selectborderwidth \
            $selectforeground $selectmode $state \
            $takefocus $width \
            $xscrollcommand $yscrollcommand]
  set default(font) $font
  set default(charheight) [font metrics $font -linespace]
  set default(charwidth) [font measure $font "0"]
  destroy $w
  unset w
  foreach key $keys { unset $key }
  
    # --------------------
    # option
    # --
    # manage the widget options default values
    # --------------------
    # parm1: get or set
    # parm2: args of get or set
    # --------------------
    # return: return of get or empty
    # --------------------
    proc option {cmd args} \
    {
      switch -glob -- $cmd \
      {
        get     { eval [linsert $args 0 option:get] }
        set     { eval [linsert $args 0 option:set] }
        default { error "wrong option command \"$cmd\": should be option get or option set" }
      }
    }
    
    # --------------------
    # option:get
    # --
    # get a widget option default value
    # --------------------
    # parm1: key of the option or empty
    # --------------------
    # return: default value or all definitions if empty parm1
    # --------------------
    proc option:get {{key ""}} \
    {
      variable woptions
      if {$key == ""} \
      { 
        # all options
        return $woptions 
      } \
      else \
      {
        # the selected option
        foreach item $woptions \
        { 
          if {[string match $key* [lindex $item 0]]} \
          { 
            if {[llength $item] == 2} { return [option:get [lindex $item 1]] } \
            else { return [lindex $item 3] }
          }
        }
        error "\"$key\" does not match a hugelist option"
      }
    }
  
    # --------------------
    # option:set
    # --
    # set widget options default value
    # --------------------
    # parm1: list of key/value pairs or key
    # --------------------
    # return: key associated entry if key
    # --------------------
    proc option:set {args} \
    {
      variable woptions
      set len [llength $args]
      foreach {key value} $args \
      {
        set new {}
        set result 0
        foreach item $woptions \
        { 
          if {$result == 0 && [string match $key* [lindex $item 0]]} \
          { 
            if {$len == 1} { return $item ; # option:set key }
            if {[llength $item] == 2} \
            { 
              # option:get abrev key
              # get the true key and go
              option:set [lindex $item 1] $value
              set result 2
              break
            }
            # option:set key value
            set item [lreplace $item 3 3 $value]
            set result 1
          } 
          lappend new $item
        }
        switch $result \
        {
          0   { error "\"$key\" does not match a hugelist option" }
          1   { set woptions $new }
          2   { # nothing to do }
        }
      }
    }
  
  # ==========================
  #
  # items options
  #
  # ==========================

  # item default config
  set ioptions \
  {
    {-background background Background {}}
    {-bg -background}
    {-fg -foreground}
    {-font font Font {}}
    {-foreground foreground Foreground {}}
    {-img -image}
    {-image image Image {}}
    {-left left Left 0}
    {-selectbackground selectBackground Background {}}
    {-selectforeground selectForeground Foreground {}}
    {-text text Text {}}
  }
  array set idxconf \
  {
    -background       0
    -font             1
    -foreground       2
    -left             3
    -selectbackground 4
    -selectforeground 5
  }

  # ==========================
  #
  # internal variables
  #
  # ==========================
  
  set inits \
  {
      active          0
      anchor          0
      canvasrows      0
      class           Hugelist
      charheight      0
      charwidth       0
      colwidth        0
      creating        0
      display         {}
      firstrow        0
      font            {}
      lastrow         0
      resizing        0
      rcols           0
      rheight         0
      rowheight       0
      rowmax          0
      rrows           0
      rwidth          0
      selection       {}
      values          {}
      vheight         0
      vleft           0
      vmax            0
      vrows           0
      vtop            0
      vwidth          0
      ydelta          0
  }

  # ==========================
  #
  # commands syntax check
  #
  # ==========================

  set optmsg \
  {activate, bbox, cget, configure, curselection, delete, get, index, info, insert, itemcget, itemconfigure, nearest, scan, see, selection, size, xview, or yview}
  set patcmd \
  { 
    c       c
    in      in
    s       s
    se      se
    act*    activate
    bbo*    bbox
    cge*    cget
    con*    config
    cur*    cursel
    del*    delete
    get     get
    ind*    index
    inf*    w:info
    ins*    insert
    itemcg* itemcget
    itemco* itemconfig
    nea*    nearest
    sca*    w:scan
    see     see
    sel*    selection
    siz*    size
    xvi*    xview
    yvi*    yview
  }
  array set cmdmsg \
  { 
    activate:1  {}
    activate    {wrong # args: should be "$w activate index"}
    bbox:1      {}
    bbox        {wrong # args: should be "$w bbox index"}
    cget:1      {}
    cget        {wrong # args: should be "$w cget option"}
    config      {}
    cursel:0    {}
    cursel      {wrong # args: should be "$w curselection"}
    delete:1    {}
    delete:2    {}
    delete      {wrong # args: should be "$w delete firstIndex ?lastIndex?"}
    get:1       {}
    get:2       {}
    get         {wrong # args: should be "$w get firstIndex ?lastIndex?"}
    index:1     {}
    index       {wrong # args: should be "$w index index"}
    w:info:1    {}
    w:info      {wrong # args: should be "$w info name"}
    insert:0    {wrong # args: should be "$w insert index ?element element ...?"}
    insert      {}
    itemcget:2  {}
    itemcget    {wrong # args: should be "$w itemcget index option"}
    itemconfig  {}
    nearest:1   {}
    nearest     {wrong # args: should be "$w nearest y"}
    w:scan:3    {}
    w:scan      {wrong # args: should be "$w scan mark|dragto x y"}
    see:1       {}
    see         {wrong # args: should be "$w see index"}
    selection:2 {}
    selection:3 {}
    selection   {wrong # args: should be "$w selection option index ?index?"}
    size:0      {}
    size        {wrong # args: should be "$w size"}
    xview       {}
    yview       {}
    c           {ambiguous option "c":}
    in          {ambiguous option "in":}
    s           {ambiguous option "s":}
    se          {ambiguous option "se":}
  }
  append cmdmsg(c)  $optmsg
  append cmdmsg(in) $optmsg
  append cmdmsg(s)  $optmsg
  append cmdmsg(se) $optmsg

  # ==========================
  #
  # create/destroy
  #
  # ==========================

  # -------------
  # constructor
  # -
  # create a hugelist widget
  # -------------
  # parm1: widget path, widget config
  # -------------
  # return: widget path
  # -------------
  
  proc create {args} \
  {
    variable {}
    variable woptions
    variable ioptions
    variable idxconf
    variable inits
    variable default
    # check args
    if {[llength $args] < 1} \
    { error "wrong # args: should be \"hugelist pathName ?options?\"" }
    set w [lindex $args 0]
    set args [lrange $args 1 end]
    # clean-up
    catch { dispose $w $w.f }
    # widget default values
    foreach {item} $woptions \
    { 
      if {[llength $item] == 4} \
      {
        foreach {key - - value} $item break
        set ($w:$key) $value
        if {$value != {}} { lappend wdefconf $key $value }
      }
    }
    # items default values
    foreach item $ioptions \
    { 
      if {[llength $item] == 4} \
      {
        foreach {key - - value} $item break
        set ($w:$key) $value
        if {[info exists idxconf($key)]} { lappend ($w:defconf) $($w:$key) }
      }
    }
    # init parameters
    foreach {key value} $inits { set ($w:$key) $value }
    set ($w:listvariable) ::hugelist::($w:values)
    set ($w:font) $default(font)
    set ($w:charwidth) $default(charwidth)
    set ($w:charheight) $default(charheight)
    # init ranges
    ranges wranges:$w -deep 6
    ranges iranges:$w -deep 6
    # create frame
    frame $w -class $($w:class)
    # redirect frame reference
    rename $w ::_$w
    # widget reference
    interp alias {} $w {} ::hugelist::dispatch $w
    set ($w:reference) $w
    trace add command $w {rename delete} [namespace code [list ref:rename $w]]
    # create intermediate frame
    frame $w.f -bd 0 -highlightthickness 0
    pack $w.f -fill both -expand 1
    # create canvas
    canvas $w.f.c -bd 0 -insertwidth 0 \
                  -highlightthickness 0 -selectborderwidth 0 \
                  -xscrollincrement 1 -yscrollincrement 1
    # place canvas
    pack $w.f.c -fill both -expand 1
    # redirect canvas reference
    rename $w.f.c ::_$w.f.c
    # canvas reference
    interp alias {} $w.f.c {} ::hugelist::dispatch $w
    # bindings
    bindtags $w [list $w $($w:class) Listbox . all]
    bindtags $w.f.c [list $w $($w:class) Listbox . all]
    bind $w.f <Destroy> [namespace code [list dispose $w %W]]
    bind $w.f <FocusIn> [namespace code [list set:focus $w]]
    bind $w.f <Configure> [namespace code [list conf:event $w]]
    # configure widget
    set ($w:creating) 1
    set rc [catch \
    {
      eval [linsert $wdefconf 0 config $w]
      if {$args != ""} { eval [linsert $args 0 config $w] }
    } msg]
    set ($w:creating) 0
    if {$rc} \
    {
      catch { destroy $w }
      return -code error $msg
    }
    # return path
    return $w
  }
  
  # -------------
  # destructor
  # -
  # free the hugelist resources
  # -------------
  # parm1: widget path
  # parm2: current destroyed widget path
  # -------------
  
  proc dispose {w W} \
  {
    # delete reference proc
    # skip if not hugelist or frame
    if {$W != $w && $W != "$w.f"} { return }
    variable {}
    # delete callback
    catch { after cancel $($w:display) }
    catch { unset ($w:dx) }
    # delete aliases
    catch { interp expose {} $w }
    catch { interp alias {} $w {} }
    # delete old trace
    catch \
    {
      set oldname $($w:listvariable)
      if {$oldname != ""} \
      {
        global $oldname
        trace vdelete $oldname w [namespace code [list set:list $w]]
      }
    }
    # delete canvas
    catch { destroy $w.f.c }
    # delete reference procs
    catch { rename $($w:reference) "" }
    catch { rename ::_$w.f.c "" }
    catch { rename ::$w.f.c "" }
    # delete all variables
    array unset {} $w:* 
    # delete all ranges
    catch \
    {
      wranges:$w destroy
      iranges:$w destroy
    }
  }
  
  # -------------
  # dispatch 
  # -
  # dispatch the operations of a hugelist widget
  # -------------
  # parm1: widget path
  # parm2: called operation, operation args
  # -------------
  # return: depending on operation
  # -------------
  proc dispatch {w {cmd ""} args} \
  { 
    variable {}
    variable optmsg
    variable patcmd
    variable cmdmsg
    # check args
    if {$cmd == ""} { error "wrong # args: should be \"$w option ?arg arg ...?\"" }
    # catch error
    set rc [catch \
    {
      # retrieve command
      foreach {pattern op} $patcmd \
      { if {[string match $pattern $cmd]} { set oper $op; break } }
      if {![info exists oper]} \
      { error "bad option \"$cmd\": must be $optmsg" }
      # check args
      set n [llength $args]
      if {[info exists cmdmsg($oper:$n)]} { set msg $cmdmsg($oper:$n) } \
      else { set msg $cmdmsg($oper) }
      if {$msg != ""} { error [string map [list \$w $w] $msg] } 
      # eval command
      eval [linsert $args 0 $oper $w]
    } msg]
    # return result
    if {![info exists ($w:values)]} \
    { return "" } \
    else \
    {
      set code [expr {$rc ? "error" : "ok"}]
      return -code $code $msg
    }
  }

  # -------------
  # ref:rename
  # -
  # trace reference name 
  # -------------
  # parm1: widget path
  # parm2: reference old name
  # parm3: reference new name
  # parm4: triggered op
  # -------------
  
  proc ref:rename {w old new op} \
  {
    if {$new == ""} { catch { destroy $w } } \
    else \
    { 
      uplevel 1 [format { set ::hugelist::var [namespace which %s] } $new]
      set new $::hugelist::var
      set ::hugelist::($w:reference) $new 
    }
  }
  
  # ==========================
  #
  # widget options
  #
  # ==========================

  # -------------
  # config
  # -
  # config a hugelist widget
  # -------------
  # parm1: widget path
  # parm2: widget config or option key or empty
  # -------------
  # return: option(s) description if parm2 is a key or empty
  # -------------
  
  proc config {w args} \
  {
    variable {}
    variable idxconf
    variable default
    # check if description request
    if {[llength $args] < 2} { return [eval get:woption $w $args] }
    # update config
    set fv 0; set fh 0; set fd 0
    foreach {key value} $args \
    {
      switch -glob -- $key \
      {
        -act*     \
        { 
          # -activestyle (not used)
          switch -glob -- $value \
          {
            und*    { set value underline }
            non*    { set value none }
            dot*    { set value dotbox }
            default { error "bad activestyle \"$value\": must be dotbox, none, or underline" }
          }
          set ($w:-activestyle) $value 
          set fd 1
        }
        -alt*     \
        { 
          # -alternatecolor (hugelist specific)
          if {$value != ""} { winfo rgb . $value }
          set ($w:-alternatecolor) $value 
          set fd 1
        }
        -bd       -
        -bor*     \
        { 
          # -borderwidth
          set ($w:-borderwidth) [winfo pixels . $value]
          _$w config -bd $value
        }
        -bg       -
        -bac*     \
        { 
          # -background
          set ($w:-background) $value
          set index $idxconf(-background)
          lset ($w:defconf) $index $value
          _$w config -bg $value
          $w.f config -bg $value
          _$w.f.c config -bg $value
          set fd 1
        }
        -col*      \
        {
          # -colwidth
          if {$value != ""} { set value [winfo pixels . $value] }
          set ($w:-colwidth) $value
          set fh 1
        }
        -cur*     \
        { 
          # -cursor
          set ($w:-cursor) $value
          _$w config -cursor $value
        }
        -dee*     \
        { 
          # -deep (ranges specific)
          if {![string is integer -strict $value]} \
          { error "expected integer but got \"$value\"" }
          set ($w:-deep) $value
          wranges:$w config -deep $value 
          iranges:$w config -deep $value 
        }
        -dis*     \
        { 
          # -disabledforeground
          winfo rgb . $value
          set ($w:-disabledforeground) $value
          set fd 1
        }
        -exp* \
        { 
          set ($w:-exportselection) [expr {$value ? 1 : 0}]
          if {$value} \
          { 
            ::selection handle $w [namespace code [list xsel:handle $w]] 
            if {$($w:selection) != ""} { ::selection own $w }
          }
        }
        -fg       -
        -for*     \
        { 
          # -foreground
          winfo rgb . $value
          set ($w:-foreground) $value 
          set index $idxconf(-foreground)
          lset ($w:defconf) $index $value
          set fd 1
        }
        -fon*     \
        { 
          # font (empty font add)
          set ($w:-font) $value
          set index $idxconf(-font)
          lset ($w:defconf) $index $value
          if {$value == ""} { set ($w:font) $default(font) } \
          else { set ($w:font) $value }
          set fv 1; set fh 1
        }
        -hei*     \
        { 
          # -height
          if {![string is integer -strict $value]} \
          { error "expected integer but got \"$value\"" }
          if {$($w:-height) != $value} \
          {
            set ($w:-height) $value
            set fv 1
          }
        }
        -highlightb*     \
        { 
          # -highlightbackground
          set ($w:-highlightbackground) $value
          _$w config -highlightbackground $value
        }
        -highlightc*     \
        { 
          # -highlightcolor
          set ($w:-highlightcolor) $value
          _$w config -highlightcolor $value
        }
        -highlightt*     \
        { 
          # -highlightthickness
          _$w config -highlightthickness $value
          set ($w:-highlightthickness) [_$w cget -highlightthickness]
        }
        -img      -
        -ima*     \
        { 
          # -image
          if {$value != ""} { image width $value }
          set ($w:-image) $value 
          set fd 1
        }
        -ipadx    \
        { 
          # -ipadx
          set ($w:-ipadx) [winfo pixels . $value]
          set fh 1
        }
        -ipady    \
        { 
          # -ipady
          set ($w:-ipady) [winfo pixels . $value]
          set fv 1
        }
        -lef*     \
        { 
          # -left
          set ($w:-left) [winfo pixels . $value]
          set index $idxconf(-left)
          lset ($w:defconf) $index $value
          set fd 1
        }
        -lis*     \
        { 
          # -listvariable
          listvar $w $value 
        }
        -rel*     \
        { 
          # -relief
          switch -glob -- $value \
          {
            fla*    { set value flat }
            gro*    { set value groove }
            rai*    { set value raised }
            rid*    { set value ridge }
            sol*    { set value solid }
            sun*    { set value sunken }
            default { error "bad relief \"$value\": must be flat, groove, raised, ridge, solid, or sunken" }
          }
          set ($w:-relief) $value
          _$w config -relief $value
        }
        -row*      \
        {
          # -rowheight
          if {$value != ""} { set value [winfo pixels . $value] }
          set ($w:-rowheight) $value
          set fv 1
        }
        -selectbg  -
        -selectba* \
        { 
          # -selectbackground
          winfo rgb . $value
          set ($w:-selectbackground) $value 
          set index $idxconf(-selectbackground)
          lset ($w:defconf) $index $value
          set fd 1
        }
        -selectbo* { set ($w:-selectborderwidth) [winfo pixels . $value] }
        -selectf* \
        { 
          # -selectforeground
          winfo rgb . $value
          set ($w:-selectforeground) $value 
          set index $idxconf(-selectforeground)
          lset ($w:defconf) $index $value
          set fd 1
        }
        -selectm* \
        { 
          # -selectmode
          set ($w:-selectmode) $value 
        }
        -set*     \
        { 
          # -setgrid (not used)
          set ($w:-setgrid) [expr {$value ? 1 : 0}] 
        }
        -sta*     \
        { 
          # -state
          switch -glob -- $value \
          {
            dis*    { set value disabled }
            nor*    { set value normal }
            default { error "bad state \"$value\": must be disabled or normal" }
          }
          set ($w:-state) $value
          _$w.f.c config -state $value
          set fd 1
        }
        -tak*     \
        { 
          # -takefocus
          set ($w:-takefocus) $value
          _$w config -takefocus $value
        }
        -tex*     \
        { 
          # -text
          set ($w:-text) $value
          set fd 1
        }
        -use*      \
        {
          # -user
          set ($w:user) $value
        }
        -val*      \
        {
          # -values
          set ($w:values) $value
          listvar $w ""
        }
        -ver*      \
        {
          # -version
          error "-version option is read-only"
        }
        -wid*     \
        { 
          # -width
          if {![string is integer -strict $value]} \
          { error "expected integer but got \"$value\"" }
          if {$($w:-width) != $value} \
          {
            set ($w:-width) $value
            set fh 1
          }
        }
        -xsc*     \
        { 
          # -xscrollcommand
          set ($w:-xscrollcommand) $value
        }
        -ysc*     \
        { 
          # -yscrollcommand
          set ($w:-yscrollcommand) $value
          after 10 [namespace code [list yset $w]]
        }
        default   { error "unknown option \"$key\"" }
      }
    }
    if {$fv} { vsize:changed $w; set fd 1 }
    if {$fh} { hsize:changed $w; set fd 1 }
    if {$fd} { new:display $w }
  }
  
  # -------------
  # get:woption
  # -
  # return a widget option description
  # -------------
  # parm1: widget path
  # parm2: option name or empty
  # -------------
  # return: option description
  # -------------
  
  proc get:woption {w {key ""}} \
  {
    variable {}
    variable woptions
    if {$key == ""} \
    {
      # all options
      set result {}
      foreach option $woptions \
      {
        if {[llength $option] > 2} \
        { 
          set key [lindex $option 0]
          if {$key == "values"} \
          { lappend option [set $($w:listvariable)] } \
          else \
          { lappend option $($w:$key) }
        }
        lappend result $option
      }
    } \
    else \
    {
      # the specified option
      foreach option $woptions \
      {
        set name [lindex $option 0]
        if {[string match $key* $name]} \
        {
          if {[llength $option] == 2} \
          { set result [get:woption $w [lindex $option 1]] } \
          elseif {$name == "-values"} \
          { set result [concat $option [list [set $($w:listvariable)]]] } \
          else \
          { set result [concat $option [list $($w:$name)]] }
          break
        }
      }
      if {![info exists result]} { error "unknown option \"$key\"" }
    }
    return $result
  }
  
  # -------------
  # cget
  # -
  # return a widget option value
  # -------------
  # parm1: widget path
  # parm2: option name
  # -------------
  # return: option value
  # -------------
  
  proc cget {w key} \
  {
    variable {}
    switch -glob -- $key \
    {
      -act*         { set ($w:-activestyle) }
      -alt*         { set ($w:-alternatecolor) }
      -bd           -
      -bor*         { set ($w:-borderwidth) }
      -bg           -
      -bac*         { set ($w:-background) }
      -col*         { set ($w:-colwidth) }
      -cur*         { set ($w:-cursor) }
      -dee*         { set ($w:-deep) }
      -dis*         { set ($w:-disabledforeground) }
      -exp*         { set ($w:-exportselection) }
      -fg           -
      -for*         { set ($w:-foreground) }
      -fon*         { set ($w:-font) }
      -hei*         { set ($w:-height) }
      -highlightb*  { set ($w:-highlightbackground) }
      -highlightc*  { set ($w:-highlightcolor) }
      -highlightt*  { set ($w:-highlightthickness) }
      -img          -
      -ima*         { set ($w:-image) }
      -ipadx        { set ($w:-ipadx) }
      -ipady        { set ($w:-ipady) }
      -lef*         { set ($w:-left) }
      -lis*         { set ($w:-listvariable) }
      -rel*         { set ($w:-relief) }
      -row*         { set ($w:-rowheight) }
      -scr*         { _$w.f.c cget -scrollregion }
      -selectbg     -
      -selectba*    { set ($w:-selectbackground) }
      -selectbo*    { set ($w:-selectborderwidth) }
      -selectf*     { set ($w:-selectforeground) }
      -selectm*     { set ($w:-selectmode) }
      -set*         { set ($w:-setgrid) }
      -sta*         { set ($w:-state) }
      -tak*         \
      { 
        if {$($w:-state) == "normal"} { set ($w:-takefocus) } \
        else { set x 0 }
      }
      -tex*         { set ($w:-text) }
      -use*         { set ($w:-user) }
      -val*         { set [set ($w:listvariable)] }
      -wid*         { set ($w:-width) }
      -xsc*         { set ($w:-xscrollcommand) }
      -ysc*         { set ($w:-yscrollcommand) }
      default       { error "unknown option \"$key\"" }
    }
  }

  # -------------
  # :info
  # -
  # return an info value
  # -------------
  # parm1: widget path
  # parm2: info name
  # -------------
  # return: info value
  # -------------
  
  proc w:info {w name} \
  {
    variable {}
    switch -glob -- $name \
    {
      fon*          { set ($w:font) }
      charh*        { set ($w:charheight) }
      charw*        { set ($w:charwidth) }
      col*          { set ($w:colwidth) }
      cou*          { set ($w:vrows) }
      row*          { set ($w:rowheight) }
      default       { error "unknown info name \"$name\"" }
    }
  }
  
  # ==========================
  #
  # item options
  #
  # ==========================
  
  # -------------
  # itemconfig
  # -
  # config a range of items
  # -------------
  # parm1: widget path
  # parm2: first item index
  # parm3: optional last item index
  # parm3/4: item config or option key or empty
  # -------------
  # return: option(s) description if parm3/4 is a key or empty
  # -------------
  
  proc itemconfig {w args} \
  {
    variable {}
    variable iioptions
    variable idxconf
    # check args
    switch [llength $args] \
    {
      0     \
      { error "wrong # args: should be \".l itemconfigure first ?last? ?option? ?value? ?option value ...?\"" }
      1     -
      2     { return [eval get:ioption $w $args] }
    }
    set first [lindex $args 0]
    set args [lrange $args 1 end]
    if {$($w:vrows) == 0} { error "bad index \"$first\"" }
    # get item config
    set first [iindex $w $first 3]
    set oldconf [wranges:$w get $first]
    if {$oldconf == ""} { set iconfig $($w:defconf) } \
    else { set iconfig $oldconf }
    # get range
    if {[string index $args 0] == "-"} \
    { 
      # degenerated range
      set last $first 
    } \
    else \
    {
      set last [lindex $args 0]
      set last [iindex $w $last 3]
      set args [lrange $args 1 end]
    }
    # update config
      # widget options flag
    set wflag 0
      # image flag
    set iflag 0
      # text flag
    set tflag 0
    foreach {key value} $args \
    {
      switch -glob -- $key \
      {
        -bg       -
        -bac*     \
        { 
          # -background
          lset iconfig $idxconf(-background) $value; set wflag 1 
        }
        -fg       -
        -for*     \
        { 
          # -foreground
          lset iconfig $idxconf(-foreground) $value; set wflag 1 
        }
        -fon*     \
        { 
          # -font
          lset iconfig $idxconf(-font) $value; set wflag 1 
        }
        -img      -
        -ima*     \
        { 
          # -image (hugelist specific)
          if {$value != ""} { image width $value }
          set image $value; set iflag 1 
        }
        -lef*     \
        { 
          # -left
          if {![string is integer -strict $value]} \
          { error "expected integer but got \"$value\"" }
          lset iconfig $idxconf(-left) $value; set wflag 1 
        }
        -selectb* \
        { 
          # -selectbackground
          lset iconfig $idxconf(-selectbackground) $value; set wflag 1 
        }
        -selectf* \
        { 
          # -selectforeground
          lset iconfig $idxconf(-selectforeground) $value; set wflag 1 
        }
        -tex*     \
        { 
          # -text (hugelist specific)
          set text $value; set tflag 1 
        }
        default   { error "unknown option \"$key\"" }
      }
    }
    # update ranges or texts list
    if {$wflag} \
    { 
      # update configs ranges
      wranges:$w replace $first $last $iconfig 
    }
    if {$iflag} \
    { 
      # update images ranges
      iranges:$w replace $first $last $image 
    }
    if {$tflag} \
    {
      # update texts list
      set listname $($w:listvariable)
      set names [set $listname]
      for {set i $first} {$i <= $last} {incr i} \
      { set names [lreplace $names $i $i $text] }
      set $listname $names
    }
    # display
    new:display $w
  }
  
  # -------------
  # get:ioption
  # -
  # return an item option description
  # -------------
  # parm1: widget path
  # parm2: item index
  # parm3: option name or empty
  # -------------
  # return: option description
  # -------------
  
  proc get:ioption {w index {key ""}} \
  {
    variable {}
    variable ioptions
    variable idxconf
    set index [iindex $w $index 3]
    if {$key != ""} \
    {
      # get an option description
      foreach item $ioptions \
      {
        set name [lindex $item 0]
        if {[string match $key* $name]} \
        {
          if {[llength $item] == 2} \
          { return [get:ioption $w $index [lindex $item 1]] } \
          else \
          { 
            set n -1
            switch -- $name \
            {
              -image            { set value [iranges:$w get $index] }
              -text             { set value [lindex [set $($w:listvariable)] $index] }
              default
              { 
                set iconfig [wranges:$w get $index]
                if {$iconfig == ""} { set iconfig $($w:defconf) }
                set value [lindex $iconfig $idxconf($name)] 
              }
            }
            return [lappend item $value] 
          }
        }
      }
      error "unknown option \"$key\""
    } \
    else \
    {
      # get all options description
      foreach item $ioptions \
      {
        if {[llength $item] == 2} { lappend res $item } \
        else { lappend res [get:ioption $w $index [lindex $item 0]] }
      }
      return $res
    }
  }
  
  # -------------
  # itemcget
  # -
  # return an item option value
  # -------------
  # parm1: widget path
  # parm2: item index
  # parm3: option name
  # -------------
  # return: option value
  # -------------
  
  proc itemcget {w index key} \
  {
    variable {}
    variable idxconf
    set index [iindex $w $index 3]
      # widget options flag
    set wflag 0
      # image flag
    set iflag 0
      # text flag
    set tflag 0
    switch -glob -- $key \
    {
      -bg       -
      -bac*     { set wflag 1; set name -background }
      -fg       -
      -for*     { set wflag 1; set name -foreground }
      -fon*     { set wflag 1; set name -font }
      -img      -
      -ima*     { set iflag 1 }
      -lef*     { set wflag 1; set name -left }
      -selectb* { set wflag 1; set name -selectbackground }
      -selectf* { set wflag 1; set name -selectforeground }
      -tex*     { set tflag 1 }
      default   { error "Unknown $w option: $key" }
    }
    # return value
    if {$wflag} \
    {
      # widget option
      set iconfig [wranges:$w get $index]
      if {$iconfig == ""} \
      {
        # default config
        set iconfig $($w:defconf)
      }
      return [lindex $iconfig $idxconf($name)]
    }
    if {$iflag} \
    { 
      # image
      set image [iranges:$w get $index]
      if {$image == ""} { set image $($w:-image) }
      return $image
    }
    if {$tflag} \
    { 
      # text
      set listname $($w:listvariable)
      set text [lindex [set $listname] $index] 
      if {$text == ""} { set text $($w:-text) }
      return $text
    }
  }
  
  # ==========================
  #
  # insert/delete items
  #
  # ==========================

  # -------------
  # insert
  # -
  # insert items at an index
  # -------------
  # parm1: widget path
  # parm2: item index
  # parm3: items to insert
  # -------------
  
  proc insert {w index args} \
  {
    variable {}
    if {$($w:-state) == "disabled"} { return }
    # check index
    set oldcount $($w:vrows)
    set index [iindex $w $index 0]
    if {$index < 0} { set index 0 }
    if {$index > $oldcount} { set index $oldcount }
    if {$args == ""} { return "" }
    # save firstrow
    set firstrow $($w:firstrow)
    # update count
    set delta [llength $args]
    incr ($w:vrows) $delta
    # update ranges with default config
    wranges:$w insert $index [llength $args] ""
    iranges:$w insert $index [llength $args] ""
    # insert items
    set listname $($w:listvariable)
    set beg [lrange [set $listname] 0 [expr {$index - 1}]]
    set end [lrange [set $listname] $index end]
    set $listname [concat $beg $args $end]
    # update selection variables
    set delta2 $delta
    if {$oldcount == 0} { incr delta2 -1 }
    sel:insert $w $index $delta2
    # update display area
    vsize:changed $w
    # reset display
    if {$firstrow > $index} { yview $w [expr {$firstrow + $delta}] } \
    else { yview $w $firstrow }
    # display
    new:display $w 
    return ""
  }
  
  # -------------
  # sel:insert
  # -
  # delete a range of items
  # -------------
  # parm1: widget path
  # parm2: first item index
  # parm3: last item index or empty
  # -------------
  
  proc sel:insert {w index delta} \
  {
    variable {}
    # update anchor & active
    if {$($w:anchor) >= $index} \
    { if {[incr ($w:anchor) $delta] >= $($w:vrows)} { set ($w:anchor) 0 } }
    if {$($w:active) >= $index} \
    { if {[incr ($w:active) $delta] >= $($w:vrows)} { set ($w:active) 0 } }
    # update selection
    set newsel {}
    set oldsel $($w:selection)
    set sellen [llength $oldsel]
    for {set i 0} {$i < $sellen} {incr i} \
    {
      set item [lindex $oldsel $i]
      if {$item >= $index} { incr item $delta }
      lappend newsel $item
    }
    set ($w:selection) $newsel
  }
  
  # -------------
  # delete
  # -
  # delete a range of items
  # -------------
  # parm1: widget path
  # parm2: first item index
  # parm3: last item index or empty
  # -------------
  
  proc delete {w first {last ""}} \
  {
    variable {}
    if {$($w:-state) == "disabled"} { return }
    # get first, last
    set first [iindex $w $first]
    if {$last == ""} { set last $first }
    set last [iindex $w $last]
    if {$first > $last} { return }
    set oldcount $($w:vrows)
    if {$oldcount == 0 || $first >= $oldcount} { return; # empty }
    if {$last < 0} { return; # empty }
    if {$first < 0} { set first 0 }
    if {$last >= $oldcount} { set last [expr {$oldcount - 1}] }
    # save firstrow
    set firstrow $($w:firstrow)
    # update count
    set delta [expr {$first - $last - 1}]
    set listname $($w:listvariable)
    incr ($w:vrows) $delta
    # update ranges
    wranges:$w delete $first $last
    iranges:$w delete $first $last
    # delete mid range
    set beg [lrange [set $listname] 0 [expr {$first - 1}]]
    set end [lrange [set $listname] [expr {$last + 1}] end]
    set $listname [concat $beg $end]
    # update selection variables
    sel:delete $w $first $last $delta
    # update display area
    vsize:changed $w
    # reset display
    if {$firstrow > $first} { yview $w [expr {$firstrow + $delta}] } \
    else { yview $w $firstrow }
    # display
    new:display $w 
    return ""
  }
  
  # -------------
  # sel:delete
  # -
  # update selection when delete
  # -------------
  # parm1: widget path
  # parm2: first index
  # parm3: last index name
  # parm4: count increment
  # -------------
    
  proc sel:delete {w first last delta} \
  {
    variable {}
    set count $($w:vrows)
    if {$count == 0} \
    {
      # emptied
      set ($w:anchor) 0
      set ($w:active) 0
      set ($w:selection) {}
    } \
    else \
    {
      # not emptied
      # update anchor
      set anchor $($w:anchor)
      if     {$anchor >= $count} { set  ($w:anchor) [expr {$count - 1}] } \
      elseif {$anchor > $last}   { incr ($w:anchor) $delta } \
      elseif {$anchor > $first}  { set  ($w:anchor) $first }
      # update active
      set active $($w:active)
      if     {$active >= $count} { set  ($w:active) [expr {$count - 1}] } \
      elseif {$active > $last}   { incr ($w:active) $delta } \
      elseif {$active > $first}  { set  ($w:active) $first }
      # update selection
      set newsel {}
      set oldsel $($w:selection)
      set sellen [llength $oldsel]
      for {set i 0} {$i < $sellen} {incr i} \
      {
        set item [lindex $oldsel $i]
        if {$item < $first} { lappend newsel $item } \
        elseif {$item > $last} { lappend newsel [incr item $delta] }
      }
      set ($w:selection) $newsel
    }
  }

  # ==========================
  #
  # items list
  #
  # ==========================

  # -------------
  # listvar
  # -
  # set the items list variable
  # -------------
  # parm1: widget path
  # parm2: list name
  # -------------
  
  proc listvar {w listname} \
  {
    variable {}
    set realname $listname
    # delete old trace
    set oldname $($w:listvariable)
    if {$oldname != ""} { trace vdelete $oldname w [namespace code [list set:list $w]] }
    # get name
    if {$listname == ""} \
    { 
      if {$oldname != ""} { set ::hugelist::($w:values) [set $oldname] }
      set listname ::hugelist::($w:values) 
    } \
    else \
    {
      if {[string range $listname 0 1] != "::"} \
      {
        set level 3
        if {$($w:creating)} { incr level }
        uplevel $level { set ::hugelist::var [namespace current] }
        if {$::hugelist::var == "::"} { set ::hugelist::var ::$listname } \
        else { append ::hugelist::var ::$listname }
        set listname $::hugelist::var
      }
    }
    # create list if not exists
    if {![info exists $listname]} \
    { 
      # get old values
      set $listname {}
      if {$oldname != ""} { set $listname [set $oldname] }
    } \
    else \
    {
      # set new config
      if {[catch { set count [llength [set $listname]] } errmsg] } \
      { 
        return -code error $errmsg 
      }
      if {$count != 0} \
      { 
        wranges:$w delete 0 end
        wranges:$w insert end $count ""
        iranges:$w delete 0 end
        iranges:$w insert end $count ""
      }
    }
    # update listvariable
    set ($w:listvariable) $listname 
    # update vrows
    set ($w:vrows) [llength [set $listname]]
    # save real name
    set ($w:-listvariable) $realname 
    # set new trace
    trace variable $listname w [namespace code [list set:list $w]]
    # set selection variables
    set ($w:anchor) 0
    set ($w:active) 0
    set ($w:selection) {}
    # vertical size changed
    vsize:changed $w
    # display
    new:display $w 
  }

  # -------------
  # set:list
  # -
  # called on list change
  # -------------
  # parm1: widget path
  # parm2: dummy
  # -------------
  
  proc set:list {w args} \
  {
    # get items list & count
    set listname $::hugelist::($w:listvariable)
    # check value 
    # ----(deleted because the cost is too high)----
    #if {[catch {llength [set $listname]]} errmsg]} \
    #{ error "invalid listvar value" }
    # set list count
    set oldcount $::hugelist::($w:vrows)
    set newcount [llength [set $listname]]
    set ::hugelist::($w:vrows) $newcount
    # check items config
    set delta [expr {$newcount - $oldcount}]
    if {$delta > 0} \
    { 
      # inserted items
      wranges:$w insert $oldcount [expr {$newcount - $oldcount}] "" 
      iranges:$w insert $oldcount [expr {$newcount - $oldcount}] "" 
      sel:insert $w $oldcount $delta
    } \
    elseif {$delta < 0} \
    { 
      # deleted items
      wranges:$w delete [incr newcount] $oldcount
      iranges:$w delete $newcount $oldcount
      sel:delete $w $newcount $oldcount $delta
    }
    # vertical size changed
    ::hugelist::vsize:changed $w
    # reset horizontal size
    set ::hugelist::($w:vwidth) 0
    # display
    ::hugelist::new:display $w 
  }

  # ==========================
  #
  # displaying
  #
  # ==========================

  # -------------
  # conf:event
  # -
  # called on <Configure> event
  # -------------
  # parm1: widget path
  # -------------
  proc conf:event {w} \
  {
    variable {}
    update
    set rwidth $($w:rwidth)
    set iwidth [winfo width $w.f]
    set rheight $($w:rheight)
    set iheight [winfo height $w.f]
    if {![info exists ($w:dx)]} \
    {
      set ($w:dx) [expr {$iwidth - $rwidth}]
      set ($w:dy) [expr {$iheight - $rheight}]
    }
    set fd 0
    if {$rwidth != $iwidth - $($w:dx)} \
    {
      set ($w:rwidth) [expr {$iwidth - $($w:dx)}]
      hsize:changed $w 1
      set fd 1
    }
    if {$rheight != $iheight - $($w:dy)} \
    {
      set ($w:rheight) [expr {$iheight - $($w:dy)}]
      vsize:changed $w 1
      set fd 1
    }
    if {$fd} { new:display $w }
  }
  
  # -------------
  # vsize:changed
  # -
  # visible area stuff when vertical size changed
  # -------------
  # parm1: widget path
  # parm2: event flag or empty
  # -------------
  proc vsize:changed {w {event 0}} \
  {
    # changes :
    #       -font option
    #       -ipady option
    #       -rowheight option
    #       -height options
    #       rheight
    #       vrows
    
    variable {}
    variable default
    # used font
    set font $($w:-font)
    # char height
    if {$font == ""} { set charheight $default(charheight) } \
    else { set charheight [font metrics $font -linespace] }
    set ($w:charheight) $charheight
    # row height
    set rowheight $($w:-rowheight)
    if {$rowheight == "" || $rowheight == 0} \
    { set rowheight [expr {$charheight + $($w:-ipady)}] }
    set ($w:rowheight) $rowheight
    # canvas height
    set vheight [expr {$($w:vrows) * $rowheight}]
    set ($w:vheight) $vheight
    if {$event} \
    {
    # resizing event
      # visible area height, in pixels
      set rheight $($w:rheight)
      # count of visible rows 
      set rrows [expr {($rheight + $rowheight - 1) / $rowheight}]
      set ($w:rrows) $rrows
    } \
    else \
    {
    # options sizing
      # count of visible rows 
      set height $($w:-height)
      if {$height <= 0} { set height [llength [set $($w:listvariable)]] }
      set rrows $height
      set ($w:rrows) $rrows
      # visible area height, in pixels
      set rheight [expr {$height * $rowheight}]
      set ($w:rheight) $rheight
    }
    # visible area max y-position
    set vmax [expr {$($w:vheight) - $rheight}]
    if {$vmax < 0} { set vmax 0 }
    set ($w:vmax) $vmax
    # visible area virtual top
    set vtop $($w:vtop)
    if {$vtop > $vmax} { set vtop $vmax }
    set ($w:vtop) $vtop
    # visible area max first item
    set rowmax [expr {$($w:vrows) - $rrows}]
    if {$rowmax < 0} { set rowmax 0 }
    set ($w:rowmax) $rowmax
    
    # set visible area coordinates and indexes
    vposition:changed $w
    
    # update canvas rows
    if {$font == ""} { set font $default(font) }
    set disabled $($w:-disabledforeground)
    set canvasrows $($w:canvasrows)
    for {set row 0} {$row <= $rrows} {incr row} \
    {
      if {$row >= $canvasrows} \
      {
        # create the canvas row
        _$w.f.c create text 0 0 -anchor nw -tags [list text$row row$row] -disabledfill $disabled -font $font
        _$w.f.c create image 0 0 -anchor nw -tags [list img$row row$row]
        _$w.f.c create rectangle 0 0 0 $rowheight -tags [list rect$row row$row] -outline ""
        _$w.f.c lower rect$row
        incr canvasrows
      } \
      else \
      {
        # update the canvas row
        _$w.f.c itemconfig text$row -font $font -state normal
        _$w.f.c itemconfig row$row -state normal
      }
    }
    set ($w:canvasrows) $canvasrows
    # hide supplementary rows
    for {} {$row < $canvasrows} {incr row} { _$w.f.c itemconf row$row -state hidden }
    
    # update canvas size
    _$w.f.c config -height $rheight
    # update vscroller
    yset $w
  }
  
  # -------------
  # vposition:changed
  # -
  # visible area stuff when vertical position changed
  # -------------
  # parm1: widget path
  # -------------
  proc vposition:changed {w} \
  {
    # changes :
    #       rowheight
    #       vtop
    
    variable {}
    # row height
    set rowheight $($w:rowheight)
    # visible area top coordinate
    set vtop $($w:vtop)
    # visible area max first item
    set rowmax $($w:rowmax)
    # visible area first item
    set firstrow [expr {$vtop / $rowheight}]
    if {$firstrow > $rowmax} { set firstrow $rowmax }
    set ($w:firstrow) $firstrow
    # index of the last visible item 
    set ($w:lastrow) [expr {$firstrow + $($w:rrows) - 1}]
    # height of the first visible item outside of the visible area, in pixels
    set ($w:ydelta) [expr {$vtop - $firstrow * $rowheight}]
  }
  
  # -------------
  # hsize:changed
  # -
  # visible area stuff when horizontal size changed
  # -------------
  # parm1: widget path
  # parm2: event flag or empty
  # -------------
  proc hsize:changed {w {event 0}} \
  {
    # changes :
    #       -font option
    #       -ipadx option
    #       -colwidth option
    #       -width options
    #       rwidth
    
    variable {}
    variable default
    # used font
    set font $($w:-font)
    # char width
    if {$font == ""} { set charwidth $default(charwidth) } \
    else { set charwidth [font measure $font "0"] }
    set ($w:charwidth) $charwidth
    # column width, in pixels
    set colwidth $($w:-colwidth)
    if {$colwidth == "" || $colwidth == 0} \
    { set colwidth [expr {$charwidth + $($w:-ipadx)}] }
    set ($w:colwidth) $colwidth
    # canvas width : computed by display
    if {$event} \
    {
    # resizing event
      # visible area width, in pixels
      set rwidth $($w:rwidth)
      # count of visible columns 
      set ($w:rcols) [expr {($rwidth + $colwidth - 1) / $colwidth}]
    } \
    else \
    {
    # options sizing
      # count of visible columns 
      set width $($w:-width)
      set ($w:rcols) $width
      # visible area width, in pixels
      set rwidth [expr {$width * $colwidth}]
      set ($w:rwidth) $rwidth
    }
    # update canvas width
    incr rwidth [expr {$($w:-borderwidth) + $($w:-selectborderwidth) + $($w:-highlightthickness)}]
    _$w.f.c config -width $rwidth
  }
  
  # -------------
  # new:display
  # -
  # display visible area
  # -------------
  # parm1: widget path
  # -------------
  proc new:display {w} \
  {
    variable {}
    if {$($w:display) != ""} { after cancel $($w:display) }
    set ($w:display) [after 0 [namespace code [list display $w]]]
  }
  
  proc display {w} \
  {
    if {![winfo exists $w]} { return }
    variable {}
    variable idxconf
    set ($w:display) ""
    set font $($w:font)
    set rowheight $($w:rowheight)
    set colwidth $($w:colwidth)
    set alternate $($w:-alternatecolor)
    set vleft $($w:vleft)
    set vright [expr {$vleft + $($w:rwidth)}]
    set vdx [expr {2 * $($w:-borderwidth) + 2 * $($w:-highlightthickness)}]
    set rrows $($w:rrows)
    set vrows $($w:vrows)
    set selection $($w:selection)
    set items [set $($w:listvariable)]
    if {$($w:-activestyle) == "none"} { set active -1 } \
    else { set active $($w:active) }
    set disabled [expr {[$w.f.c cget -state] == "disabled"}]
    set ydelta $($w:ydelta)
    set y -$ydelta
    set vwidth $($w:vwidth)
    set _image $($w:-image)
    set _text $($w:-text)
    for {set row 0; set index $($w:firstrow)} {$row <= $rrows} {incr row; incr index} \
    {
      if {$row < $vrows && !($ydelta == 0 && $row == $rrows)} \
      {
        # get item config
        set iconfig [wranges:$w get $index]
        if {$iconfig == ""} { set iconfig $($w:defconf) }
        foreach key [array names idxconf] { set $key [lindex $iconfig $idxconf($key)] }
        set image [iranges:$w get $index]
        if {$image == ""} { set image $_image }
        set text [lindex $items $index]
        if {$text == ""} { set text $_text }
        if {${-font} == ""} { set -font $font }
        # display item image
        set x ${-left}
        set imgheight 0
        if {$image != ""} \
        {
          set iy [expr {$y + ($rowheight - [image height $image]) / 2}]
          _$w.f.c coord img$row $x $iy
          incr x [image width $image]
          incr x 4
        }
        _$w.f.c itemconf img$row -image $image -state normal
        # display item text
        _$w.f.c coord text$row [expr {$x + 2}] [expr {$y + 2}] 
        _$w.f.c itemconf text$row -text $text -state normal -font ${-font}
        set rowwidth [expr {$x + [string length $text] * $colwidth}]
        if {$rowwidth > $vwidth} { set vwidth $rowwidth }
      } \
      else \
      {
        # get default config
        set iconfig $($w:defconf)
        foreach key [array names idxconf] \
        { set $key [lindex $iconfig $idxconf($key)] }
        # display empty row
        _$w.f.c itemconfig img$row -state hidden
        _$w.f.c itemconfig text$row -state hidden
      }
      # place background
      _$w.f.c coord rect$row $vleft $y [expr {$vright + $vdx}] [incr y $rowheight]
      # set background & foreground colors
      if {$disabled || [lsearch $selection $index] == -1} \
      { 
        set color ${-background}
        if {$index % 2 && $alternate != ""} { set color $alternate }
        _$w.f.c itemconf rect$row -fill $color 
        _$w.f.c itemconf text$row -fill ${-foreground}
      } \
      else \
      { 
        _$w.f.c itemconf rect$row -fill ${-selectbackground}
        _$w.f.c itemconf text$row -fill ${-selectforeground}
      }
      # set boxing
      if {!$disabled && $index == $active} \
      { _$w.f.c itemconf rect$row -outline gray50 } \
      else \
      { _$w.f.c itemconf rect$row -outline "" }
    }
    # update horizontal width
    set ($w:vwidth) $vwidth
    if {$($w:-width) == 0 && [winfo reqwidth $w.f.c] < $vwidth} \
    { 
      # dynamic width
      _$w.f.c config -width $vwidth
      set ($w:rwidth) $vwidth
      hsize:changed $w 1
    }
    # update scroll region
    _$w.f.c config -scrollregion [list 0 0 $vwidth $($w:vheight)]
    # update hscroller
    if {$vwidth == 0 || $vright > $vwidth} { xset $w 0 1 } \
    else { xset $w [expr {double($vleft) / $vwidth}] [expr {double($vright) / $vwidth}] }
  }
  
  # ==========================
  #
  # selecting
  #
  # ==========================

  # -------------
  # xsel:handle
  # -
  # handle X selection
  # -------------
  # parm1: widget path
  # parm2: offset inside the selection
  # parm3: chunk max chars count
  # -------------
  
  proc xsel:handle {w offset maxchars} \
  {
    variable {}
    if {!$($w:-exportselection)} { error "can't export selection" }
    set list [set $($w:listvariable)]
    set res {}
    foreach item $($w:selection) { lappend res [lindex $list $item] }
    return [string range [join $res \n] $offset [expr {$offset + $maxchars - 1}]]
  }
  
  # -------------
  # selection
  # -
  # selection operations
  # -------------
  # parm1: widget path
  # parm2: selection operation
  # parm3: first operation arg
  # parm4: second operation arg or empty
  # -------------
  
  proc selection {w cmd first {last ""}} \
  {
    variable {}
    set ($w:own) 1
    set count $($w:vrows)
    set first [iindex $w $first]
    if {$last != ""} { set last [iindex $w $last] }
    switch -glob -- $cmd \
    {
      anc*    \
      {
        # selection anchor
        if {$($w:-state) == "disabled"} { return }
        if {$last != ""} \
        { error "wrong # args: should be \"$w selection anchor index\"" }
        set ($w:anchor) [iindex $w $first 2]
      }
      cle*    \
      {
        # selection clear first ?last?
        if {$($w:-state) == "disabled"} { return }
        if {$count == 0} { return 0 }
        if {$last == ""} { set last $first }
        if {$first > $last} { foreach {last first} [list $first $last] break }
        set selection $($w:selection)
        foreach item $selection \
        {
          if {$item >= $first && $item <= $last} \
          {
            if {[set n [lsearch $selection $item]] != -1} \
            { set selection [lreplace $selection $n $n] }
          }
        }
        set ($w:selection) $selection
      }
      inc*    \
      { 
        # selection includes index
        if {$last != ""} \
        { error "wrong # args: should be \"$w selection includes index\"" }
        return [expr {[lsearch $($w:selection) $first] != -1}] 
      }
      set     \
      {
        # selection set first ?last?
        if {$($w:-state) == "disabled"} { return }
        if {$count == 0} { return 0 }
        if {$last == ""} { set last $first }
        if {$first > $last} { foreach {last first} [list $first $last] break }
        if {$last < 0 || $first >= $count} { return }
        if {$first < 0} { set first 0 }
        if {$last >= $count} { set last [expr {$count - 1}] }
        for {set i $first} {$i <= $last} {incr i} \
        {
          if {[set n [lsearch $($w:selection) $i]] == -1} \
          { lappend ($w:selection) $i }
        }
      }
      default { error "bad option \"$cmd\": must be anchor, clear, includes, or set" }
    }
    # set X selection
    if {$($w:-exportselection) && $($w:selection) != ""} { ::selection own $w }
    # update display
    new:display $w
  }

  # -------------
  # cursel
  # -
  # return current selection
  # -------------
  # parm1: widget path
  # -------------
  # return: current selection
  # -------------
  
  proc cursel {w} \
  {
    variable {}
    return [lsort -dictionary $($w:selection)]
  }
  
  # -------------
  # activate
  # -
  # set the active item
  # -------------
  # parm1: widget path
  # parm2: item index
  # -------------
  
  proc activate {w index} \
  {
    variable {}
    if {$($w:-state) == "disabled"} { return }
    set index [iindex $w $index]
    if {$index < 0} { set index 0 }
    if {$index >= $($w:vrows)} { set index [expr {$($w:vrows) - 1}] }
    set ($w:active) $index
    new:display $w
  }
  
  # ==========================
  #
  # scrolling
  #
  # ==========================

  # -------------
  # xview
  # -
  # called on horizontal scrolling
  # -------------
  # parm1: widget path
  # parm2: new scrolling or empty
  # -------------
  # return: scrolling info if parm2 is empty
  # -------------
  
  proc xview {w args} \
  {
    variable {}
    set vwidth $($w:vwidth)
    switch [llength $args] \
    {
      0     \
      {
        # return fractions
        if {$($w:vrows) == 0} { return {0 1} } \
        else \
        { 
          set vleft $($w:vleft)
          set beg [expr {double($vleft)  / $vwidth}]
          set end [expr {double($vleft + $($w:rwidth)) / $vwidth}]
          if {$end == 0 || $end > 1.0} { set end 1 }
          return [list [format %g $beg] [format %g $end]]
        }
      }
      1     \
      {
        # show index
        set index $args
        if {![string is integer -strict $index]} \
        { error "expected integer but got \"$index\"" }
        set vleft [expr {$index * $($w:colwidth)}]
      }
      2     \
      {
        # absolute movement
        foreach {cmd fraction} $args break
        if {![string match mov* $cmd]} \
        { error "unknown option \"$cmd\": must be moveto or scroll" }
        set vleft [expr {int($fraction * $vwidth)}]
      }
      3     \
      {
        # relative movement
        foreach {cmd count unit} $args break
        if {![string match scr* $cmd]} \
        { error "unknown option \"$cmd\": must be moveto or scroll" } \
        else \
        { 
          if {[string match p* $unit]} \
          { set count [expr {$count * $($w:rcols)}] }
          set vleft [expr {$($w:vleft) + $count * $($w:colwidth)}]
        }
      }
    }
    # set left & right
    set rwidth $($w:rwidth)
    if {$vleft > $vwidth - $rwidth} { set vleft [expr {$vwidth - $rwidth}] }
    if {$vleft < 0} { set vleft 0 }
    if {$vleft != $($w:vleft)} \
    {
      set ($w:vleft) $vleft
      # scroll
      _$w.f.c xview moveto [expr {double($vleft) / $vwidth}]
      # display
      new:display $w
    }
  }
  
  # -------------
  # yview
  # -
  # called on vertical scrolling
  # -------------
  # parm1: widget path
  # parm2: new scrolling or empty
  # -------------
  # return: scrolling info if parm2 is empty
  # -------------
  
  proc yview {w args} \
  {
    variable {}
    set rowheight $($w:-rowheight)
    if {$rowheight == ""} { set rowheight $($w:rowheight) }
    set oldvtop $($w:vtop)
    switch [llength $args] \
    {
      0       \
      { 
        # return fractions
        if {$($w:vrows) <= $($w:rrows)} { return {0 1} } \
        else \
        { 
          set vheight $($w:vheight)
          if {$vheight == 0} \
          { return {0 1} } \
          else \
          {
            set vtop $oldvtop
            set first [expr {double($vtop) / $vheight}]
            set last  [expr {double($vtop + $($w:rheight)) / $vheight}]
            return [list [format %g $first] [format %g $last]]
          }
        }
      }
      1       \
      { 
        # show index
        set index [iindex $w $args 2]
        set vtop [expr {int(double($index) * $rowheight)}] 
      }
      2       \
      { 
        # absolute movement
        foreach {cmd fraction} $args break
        if {![string match mov* $cmd]} \
        { error "unknown option \"$cmd\": must be moveto or scroll" }
        set vtop [expr {int(double($fraction) * $($w:vheight))}]
      }
      3       \
      { 
        # relative movement
        foreach {cmd count unit} $args break
        if {![string match scr* $cmd]} \
        { error "unknown option \"$cmd\": must be moveto or scroll" } \
        else \
        {
          if {[string match p* $unit]} \
          { set count [expr {$count * $($w:rrows)}] }
          set vtop [expr {$oldvtop - $($w:ydelta) + $count * $rowheight}]
        }
      }
    }
    # --------------
    # compute ($w:firstrow), ($w:lastrow), ($w:vtop) & ($w:ydelta)
    # --------------
    set vmax $($w:vmax)
    if {$vtop > $vmax} { set vtop $vmax }
    if {$vtop < 0} { set vtop 0 }
    if {$vtop != $oldvtop} \
    {
      set ($w:vtop) $vtop
      vposition:changed $w
      yset $w
      new:display $w
    }
  }
  
  # -------------
  # xset
  # -
  # set the horizontal scroll widget
  # -------------
  # parm1: widget path
  # parm2: fraction before the visible area
  # parm3: fraction before the end of the visible area
  # -------------
  
  proc xset {w first last} \
  {
    variable {}
    if {![info exists ($w:-xscrollcommand)]} { return }
    set cmd $($w:-xscrollcommand)
    if {$cmd == ""} { return } \
    else \
    { 
      set rc [catch { eval $cmd [format %g $first] [format %g $last] } errmsg]
      if {$rc} \
      { 
        if {[string index $cmd 0] == "." && ![winfo exists [lindex $cmd 0]]} { return }
        error "$w -xscrollcommand: $errmsg" 
      }
    }
  }
  
  # -------------
  # yset
  # -
  # set the vertical scroll widget
  # -------------
  # parm1: widget path
  # -------------
  # return: registered yscrollcommand result
  # -------------
  
  proc yset {w} \
  {
    variable {}
    if {![info exists ($w:-yscrollcommand)]} { return }
    set cmd $($w:-yscrollcommand)
    if {$cmd == ""} { return "" } \
    else \
    { 
      set args [yview $w]
      set rc [catch { eval $cmd $args } errmsg]
      if {$rc} \
      { 
        if {[string index $cmd 0] == "." && ![winfo exists [lindex $cmd 0]]} { return }
        error "$w -yscrollcommand: $errmsg" 
      }
    }
  }
  
  # -------------
  # see
  # -
  # vertically adjust the visible area
  # -------------
  # parm1: widget path
  # parm2: item index
  # -------------
  
  proc see {w index} \
  {
    variable {}
    set vrows $($w:vrows)
    if {$vrows == 0} { return }
    set index [iindex $w $index 2]
    set firstrow $($w:firstrow)
    set lastrow $($w:lastrow)
    set delta [expr {$($w:rrows) / 2}]
    if {$index < $firstrow} \
    {
      if {$index + 1 == $firstrow} \
      { 
        set move $index 
      } \
      else \
      { 
        set move [expr {$index - $delta}] 
      }
    } \
    elseif {$index > $lastrow} \
    {
      if {$index - 1 == $lastrow} \
      { 
        set move [incr firstrow] 
      } \
      else \
      { 
        set move [expr {$index - $delta}] 
      }
    }
    if {[info exists move]} { yview $w $move }
  }

  # -------------
  # :scan
  # -
  # rapid scrolling operations
  # -------------
  # parm1: widget path
  # parm2: scrolling operation
  # parm3: x coordinate
  # parm4: y coordinate
  # -------------
  
  proc w:scan {w cmd x y} \
  { 
    variable {}
    if {![string is integer -strict $x]} \
    { error "expected integer but got \"$x\"" }
    if {![string is integer -strict $y]} \
    { error "expected integer but got \"$y\"" }
    switch -glob -- $cmd \
    {
      mar*    { set ($w:markx) $x; set ($w:marky) $y }
      dra*    \
      {
        if {![info exists ($w:markx)]} \
        { set ($w:markx) $x; set ($w:marky) $y }
        set vleft $($w:vleft)
        set xx [expr {$vleft + ($($w:markx) - $x) * 10}]
        if {$xx != $vleft} \
        { 
          xview $w moveto [expr {double($xx) / $($w:vwidth)}] 
        }
        set vtop $($w:vtop)
        set yy [expr {$vtop + ($($w:marky) - $y) * 10}]
        if {$yy != $vtop} \
        { 
          yview $w moveto [expr {double($yy) / $($w:vheight)}] 
        }
        set ($w:markx) $x; set ($w:marky) $y
      }
      default { error "bad option \"$cmd\": must be mark or dragto" }
    }
    return ""
  }
  
  # ==========================
  #
  # miscellaneous
  #
  # ==========================

  # -------------
  # index
  # -
  # translate symbolic index
  # -------------
  # parm1: widget path
  # parm2: item index
  # -------------
  # return: numeric index
  # -------------
  
  proc index {w index} { iindex $w $index 0 }
  
  # -------------
  # iindex
  # -
  # translate symbolic index
  # -------------
  # parm1: widget path
  # parm2: item index
  # parm3: special flag
  # -------------
  # return: numeric index
  # -------------
  
  proc iindex {w index {flag 1}} \
  {
    variable {}
    set vrows $($w:vrows) 
    switch -glob -- $index \
    {
      act*    { return $($w:active) }
      anc*    { return $($w:anchor) }
      end     \
      { 
        set count $vrows
        if {$count != 0 && $flag} { incr count -1 }
        return $count
      }
      end-*   \
      { 
        set number [string range $index 3 end]
        if {![string is integer -strict $number]} \
        { error "bad hugelist index \"$index\": should be end-<number>" }
        set count $vrows
        if {$count != 0} \
        {
          if {$number == 0 && $flag} { incr count -1 } \
          else { incr count $number; incr count -1 }
        }
        if {$count < 0} { set count 0 } 
        return $count
      }
      @*      \
      {
        foreach {x y} [split [string range $index 1 end] ,] break
        if {![info exists y] || ![string is integer -strict $y]} \
        { error "bad hugelist index \"$index\": must be active, anchor, end, @x,y, or a number" }
        set rowheight $($w:-rowheight)
        if {$rowheight == ""} { set rowheight $($w:rowheight) }
        set index [expr {($($w:vtop) + $y) / $rowheight}]
        if {$index < 0} { set index 0}
        if {$index >= $vrows} { set index [expr {$vrows - 1}] }
      }
      default \
      { 
        if {![string is integer -strict $index]} \
        { error "bad hugelist index \"$index\": must be active, anchor, end, @x,y, or a number" }
      }
    }
    if {$flag == 3} \
    {
      # check
      if {$index < 0 || $index >= $vrows } { error "item number \"$index\" out of range" }
    }
    if {$flag == 2} \
    {
      # adjust index
      if {$index < 0} { set index 0 } \
      elseif {$vrows != 0 && $index >= $vrows} { set index [incr vrows -1] }
    }
    return $index
  }
  
  # -------------
  # bbox
  # -
  # return item text bbox coordinates
  # -------------
  # parm1: widget path
  # parm2: item index
  # -------------
  # return: bbox coordinates
  # -------------
  
  proc bbox {w index} \
  {
    variable {}
    if {$($w:display) != ""} { after cancel $($w:display); display $w }
    set index [iindex $w $index]
    set firstrow $($w:firstrow)
    set lastrow $($w:lastrow)
    if {$($w:vrows) == 0 || $index < $firstrow || $index > $lastrow} \
    { return {} } \
    else \
    {
      set row [expr {$index - $firstrow}]
      foreach {x1 y1 x2 y2} [_$w.f.c bbox text$row] break
      if {$x1 == ""} { return "" } \
      else \
      {
        set d $($w:-borderwidth)
        incr d $($w:-selectborderwidth)
        incr d $($w:-highlightthickness)
        set x2 [expr {$x2 - $x1}]
        set y2 [expr {$y2 - $y1}]
        incr x1 $d; incr x1 -2
        incr y1 $d; incr y1 -2
        return [list $x1 $y1 $x2 $y2]
      }
    }
  }
  
  # -------------
  # get
  # -
  # return a range of items texts
  # -------------
  # parm1: widget path
  # parm2: first item index
  # parm3: last item index or empty
  # -------------
  # return: list of texts
  # -------------
  
  proc get {w args} \
  {
    variable {}
    set listname $($w:listvariable)
    set items [set $listname]
    set len [llength $args]
    set vrows $($w:vrows)
    switch $len \
    {
      1       \
      { 
        # get index
        set index [iindex $w $args]
        if {$index < 0 || $index >= $vrows} { return {} }
        return [lindex $items $index]
      }
      2       \
      { 
        # get first last
        foreach {first last} $args break
        set first [iindex $w $first]
        set last [iindex $w $last]
        if {$first > $last} { foreach {last first} [list $first $last] break }
        if {$first >= $vrows} { return {} }
        if {$last < 0} { return {} }
        if {$first < 0} { set first 0 }
        if {$last >= $vrows} { set last [incr vrows -1] }
        return [lrange $items $first $last]
      }
      default { error "Should be: $w get first ?last?" }
    }
  }

  # -------------
  # nearest
  # -
  # return the item index from the item coordinates
  # -------------
  # parm1: widget path
  # parm2: y coordinate
  # -------------
  # return: nearest item index
  # -------------
  
  proc nearest {w y} \
  { 
    variable {}
    set index [iindex $w @0,$y]
    set firstrow $($w:firstrow)
    if {$index < $firstrow} { set index $firstrow }
    set lastrow $($w:lastrow)
    if {$index >= $lastrow} { set index $lastrow }
    return $index
  }

  # -------------
  # size
  # -
  # return the items count
  # -------------
  # parm1: widget path
  # -------------
  # return: items count
  # -------------
  
  proc size {w} { variable {}; return $($w:vrows) }

  # -------------
  # set:focus
  # -
  # called on focus change
  # -------------
  # parm1: widget path
  # -------------
  
  proc set:focus {w} \
  { 
    variable {}
    if {$($w:-state) == "normal"} { focus -force $w.f.c } 
  }

# end of ::hugelist namespace definition
}
