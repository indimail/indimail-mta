if {![catch {namespace children ::hugecombo}]} { return }
namespace eval ::hugecombo \
{
# beginning of the ::hugecombo namespace definition

  namespace export hugecombo

# ####################################
#
#   hugecombo
#       a combo box based on the hugelist widget
#
set version 1.1
#
#   ulis, (C) 2003
#
#   NOL (no obligation licence)
#
# ------------------------------------
# 0.9
#   first released
# ------------------------------------
# 1.0
#   new -alternatecolor, -compound, -height, -rowheight options
#   new list, load operations
#   dynamicly adapted width for toplevel
# ------------------------------------
# 1.0.1, 2003-03-01
#   dynamic resizing is missing (Jorge Suit Perez Ronda, 2003-02-27)
#   parameters management is missing
# ------------------------------------
# 1.1, 2003-03-29
#   new picklist -state value (Jorge Suit Perez Ronda, 2003-03-12)
# ####################################

  # ====================
  #
  #   packages
  #
  # ====================
  
  package provide Hugecombo $version
  package provide hugecombo $version
  
  package require Tcl 8.4
  package require Tk 8.4
 
 global libdir
  if {[catch {package require Hugelist 1.2}]} \
  {
	if {[info exists libdir] && $libdir != ""} {
		source $libdir/hugelist.tcl
	} else {
    	source [file join [file dirname [info script]] hugelist.tcl]
	}
    package require Hugelist 1.2
  }
  namespace import ::hugelist::hugelist
  
  # ====================
  #
  # entry point
  #
  # ====================
  
    proc hugecombo {args} \
    {
      set rc [catch \
      {
        switch -glob -- $args \
        {
          opt*    { eval [linsert [lrange $args 1 end] 0 option] }
          par*    { eval [linsert [lrange $args 1 end] 0 param] }
          ver*    { set ::hugecombo::version }
          .*      { eval [linsert $args 0 create] }
          default { error "unknown option \"[lindex $args 0]\": should be pathName, option or param" }
        }
      } msg]
      # return result
      if {$rc} { set code error } else { set code ok }
      return -code $code $msg
    }
  
  # ====================
  #
  # options
  #
  # ====================
  
  set w ._hugecombo_entry_test_
  entry $w
  set keys \
  {
    background 
    disabledbackground
    disabledforeground
    foreground
    highlightbackground
    highlightcolor
    insertbackground
    readonlybackground
    selectbackground
    selectforeground
  }
  foreach key $keys \
  { set $key [$w cget -$key] }
  eval [format {
  set woptions \
  {
    {-alternatecolor alternateColor AlternateColor {}}
    {-background background Background %s}
    {-bd -borderwidth}
    {-bg -background}
    {-borderwidth borderWidth BorderWidth 2}
    {-compound compound Compound 0}
    {-cursor cursor Cursor {}}
    {-disabledbackground disabledBackground DisabledBackground %s}
    {-disabledforeground disabledForeground DisabledForeground %s}
    {-exportselection exportSelection ExportSelection 1}
    {-fg -foreground}
    {-font font Font {}}
    {-foreground foreground Foreground %s}
    {-height height Height 5}
    {-highlightbackground highlightBackground HighlightBackground %s}
    {-highlightcolor highlightColor HighlightColor %s}
    {-highlightthickness highlightThickness HighlightThickness 0}
    {-image image Image {}}
    {-img -image}
    {-insertbackground insertBackground Foreground %s}
    {-insertborderwidth insertBorderWidth BorderWidth 0}
    {-insertofftime insertOffTime OffTime 300}
    {-insertontime insertOnTime OnTime 600}
    {-insertwidth insertWidth InsertWidth 2}
    {-invalidcommand invalidCommand InvalidCommand {}}
    {-invcmd -invalidcommand}
    {-justify justify Justify left}
    {-listvariable listVariable Variable {}}
    {-readonlybackground readonlyBackground ReadonlyBackground %s}
    {-relief relief Relief sunken}
    {-rowheight rowHeight RowHeight {}}
    {-selectbackground selectBackground Foreground %s}
    {-selectborderwidth selectBorderWidth BorderWidth 0}
    {-selectforeground selectForeground Background %s}
    {-show show Show {}}
    {-state state State normal}
    {-takefocus takeFocus TakeFocus 0}
    {-textvariable textVariable Variable {}}
    {-validate validate Validate none}
    {-validatecommand validateCommand ValidateCommand {}}
    {-values values Values {}}
    {-vcmd -validatecommand}
    {-width width Width 20}
    {-xscrollcommand xScrollCommand ScrollCommand {}}
  }
               } $background $disabledbackground $disabledforeground \
                 $foreground $highlightbackground $highlightcolor \
                 $insertbackground $readonlybackground \
                 $selectbackground $selectforeground]               
  destroy $w
  unset w
  foreach key $keys { unset $key }

    # --------------------
    # option
    # --
    # manage the options default values
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
    # get an option default value
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
        error "\"$key\" does not match a hugecombo option"
      }
    }
  
    # --------------------
    # option:set
    # --
    # set options default value
    # --------------------
    # parm1: list of key/value pairs
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
              # option:get abrev value
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
          0   { error "\"$key\" does not match a hugecombo widget option" }
          1   { set woptions $new }
          2   { # nothing to do }
        }
      }
    }
  
  # ====================
  #
  # parameters
  #
  # ====================
  
  set w ._hugecombo_
  entry $w
  set font [$w cget -font]
  destroy $w
  unset w
  set inits \
  {
    TAG1                  HugeCombo
    TAG2                  ComboList
    TAG3                  HugeEntry
    b:borderwidth         2
    b:highlightthickness  0
    b:image               _hugecombo_
    b:relief              raised
    b:width               16
    e:borderwidth         0
    e:highlightthickness  0
    e:relief              flat
    e:selectborderwidth   0
    l:activestyle         none
    l:borderwidth         0
    l:highlightthickness  0
    l:relief              flat
    t:relief              raised
  }

  set inits0 \
  {
    class                 Hugecombo
    count                 0
    curselection          0
    find                  0
    listvariable          {}
    loading               0
    oldvalue              ""
    opened                0
    scrolled              1
    selection             {}
    value                 {}
    values                {}
  }

    # --------------------
    # param
    # --
    # manage the parameters initial values
    # --------------------
    # parm1: get or set
    # parm2: args of get or set
    # --------------------
    # return: return of get or empty
    # --------------------
    proc param {cmd args} \
    {
      switch -glob -- $cmd \
      {
        get     { eval [linsert $args 0 param:get] }
        set     { eval [linsert $args 0 param:set] }
        default { error "wrong param command \"$cmd\": should be param get or param set" }
      }
    }
    
    # --------------------
    # param:get
    # --
    # get a parameter initial value
    # --------------------
    # parm1: key of the parameter
    # --------------------
    # return: initial value
    # --------------------
    proc param:get {key} \
    {
      variable inits
      array set ia $inits
      if {![info exists ia($key)]} \
      { error "\"$key\" does not match a hugecombo parameter" }
      return $ia($key)
    }
  
    # --------------------
    # param:set
    # --
    # set a parameter initial value
    # --------------------
    # parm1: list of key/value pairs
    # --------------------
    proc param:set {args} \
    { 
      variable inits
      array set ia $inits
      foreach {key value} $args \
      {
        if {![info exists ia($key)]} \
        { error "\"$key\" does not match a hugecombo parameter" }
        set ia($key) $value
      }
      set inits [array get ia]
    }
  
  # ==========================
  #
  # commands syntax check
  #
  # ==========================

  set optmsg \
  {bbox, cget, component, configure, curselection, delete, get, icursor, index, insert, itemcget, itemconfigure, list, load, scan, selection, send, size, validate or xview}
  set patcmd \
  { 
    bbo*        bbox
    cge*        cget
    com*        component
    con*        config
    cur*        cursel
    del*        delete
    fin*        find
    get         get
    icu*        icursor
    ind*        index
    ins*        insert
    itemcg*     itemcget
    itemco*     itemconf
    lis*        list
    loa*        load
    sca*        scan
    sel*        selection
    sen*        send
    siz*        size
    val*        validate
    xvi*        xview
  }
  array set cmdmsg \
  { 
    bbox        {}
    cget:1      {}
    cget        {cget option}
    component:1 {}
    component   {component name}
    config      {}
    cursel:0    {}
    cursel      {curselection}
    delete      {}
    get         {}
    icursor     {}
    index       {}
    insert      {}
    itemcget    {}
    itemconf    {}
    list:0      {list command ?args?}
    list        {}
    load:0      {}
    load:1      {}
    load        {load index}
    scan        {}
    selection   {}
    send:1      {}
    send        {send <keyname>}
    size        {}
    validate    {}
    xview       {}
  }
  set listlist {close, curselection, find, open or select}
  set patlist \
  {
    clo*        close
    cur*        cursel
    fin*        find
    ope*        open 
    sel*        select
  }
  array set listmsg \
  {
    close:0     {}
    close       {close}
    cursel:0    {}
    cursel      {curselection}
    find:1      {}
    find        {find pattern}
    open:0      {}
    open        {open}
    select:1    {}
    select      {select rowindex}
  }

  # ====================
  #
  # standard bindings
  #
  # ====================

  bind HugeCombo <ButtonRelease>    [namespace code {top:close  %W}]
  bind HugeCombo <FocusIn>          { focus -force %W.e }

  bind ComboList <<ListboxSelect>>  [namespace code {top:load   %W}]
  bind ComboList <Motion>           [namespace code {top:move   %W @0,%y}]

  bind HugeEntry <Destroy>          { destroy [winfo parent %W] }
  bind HugeEntry <FocusIn>          [namespace code {top:raise  %W}]
  bind HugeEntry <Escape>           [namespace code {top:close  %W}]
  bind HugeEntry <Return>           [namespace code {top:load   %W}]
  bind HugeEntry <Control-Return>   [namespace code {top:find   %W}]
  bind HugeEntry <Control-Home>     [namespace code {top:home   %W}]
  bind HugeEntry <Prior>            [namespace code {top:prior  %W}]
  bind HugeEntry <Up>               [namespace code {top:up     %W}]
  bind HugeEntry <Down>             [namespace code {top:down   %W}]
  bind HugeEntry <Next>             [namespace code {top:next   %W}]
  bind HugeEntry <Control-End>      [namespace code {top:end    %W}]
  
  # ====================
  #
  # hugecombo image
  #
  # ====================

  set data "
  #define v_width 8
  #define v_height 4
  static unsigned char v_bits[] = { 0xff, 0x7e, 0x3c, 0x18 };"
  image create bitmap _hugecombo_ -data $data

  # ====================
  #
  # create/destroy widget
  #
  # ====================

    # --------------------
    # widget constructor
    # --
    # create a hugecombo
    # --------------------
    # parm1: widget path, widget config
    # --------------------
    # return: path of the widget
    # --------------------
    proc create {args} \
    {
      variable {}
      variable woptions
      variable inits
      variable inits0
      # check args
      if {[llength $args] < 1} \
      { error "wrong # args: should be \"hugecombo pathName ?options?\"" }
      set w [lindex $args 0]
      set args [lrange $args 1 end]
      # init
      set config {}
      foreach item $woptions \
      {
        if {[llength $item] > 2} \
        {  
          foreach {key - - value} $item break
          set ($w:$key) $value
          if {$value != ""} { lappend config $key $value }
        }
      }
      foreach {key value} $inits { set ($w:$key) $value }
      foreach {key value} $inits0 { set ($w:$key) $value }
      set ($w:listvariable) ::hugecombo::($w:values)
      # create frame, canvas, entry & button
      frame $w -class $($w:class) -relief $($w:-relief) -bd $($w:-borderwidth)
      entry $w.e -width $($w:-width) -relief $($w:e:relief) -exportselection 0 \
                 -bd $($w:e:borderwidth) -highlightthickness $($w:e:highlightthickness) \
                 -selectborderwidth $($w:e:selectborderwidth)
      canvas $w.c -width 0 -height 0 -bd 0 -highlightthickness 0 -insertwidth 0 \
                  -bg [$w.e cget -bg]
      $w.c create image 0 0 -anchor nw -tags {img}
      button $w.b -image $($w:b:image) -width $($w:b:width) \
                  -relief $($w:b:relief) -borderwidth $($w:b:borderwidth) \
                  -highlightthickness $($w:b:highlightthickness) \
                  -command [namespace code [list top:toggle $w]]
      # --- Jorge Suit Perez Ronda's patch ---
      grid $w.c -row 0 -column 0 -sticky ns
      grid $w.e -row 0 -column 1 -sticky ew
      grid $w.b -row 0 -column 2 -sticky ns
      grid columnconfigure $w 1 -weight 1
      # --- end ---
      # bindings
      bindtags $w [list $w $($w:TAG1) Hugecombo Frame . all]
      bindtags $w.e [list $w.e $($w:TAG3) Entry . all]
      bind $w <Destroy> [namespace code [list dispose $w %W]]
      # widget reference
      rename $w ::_$w
      interp alias {} ::$w {} ::hugecombo::dispatch $w
      set ($w:reference) $w
      trace add command $w {rename delete} [namespace code [list reference $w]]
      # create the toplevel
      top:create $w
      # configure widget
      set rc [catch \
      {
        eval [linsert $config 0 w:config $w]
        if {$args != ""} { eval [linsert $args 0 w:config $w] }
      } msg]
      if {$rc} \
      {
        catch { destroy $w }
        return -code error $msg
      }
      # return path
      return $w
    }
    
    # --------------------
    # widget destructor
    # --
    # release all ressources
    # --------------------
    # parm1: path of the widget
    # parm2: current destroyed widget path
    # --------------------
    proc dispose {w W} \
    {
      # delete reference proc
      catch { interp expose {} $W }
      catch { rename $W "" }
      # skip if not hugelist
      if {$w != $W} { return }
      # delete toplevel
      catch { destroy $w.t }
      # delete old trace
      variable {}
      set oldname $($w:listvariable)
      if {$oldname != ""} \
      {
        global $oldname
        trace vdelete $oldname w [namespace code [list set:list $w]]
      }
      # delete reference proc
      catch { rename $($w:reference) "" }
      # delete all variables
      array unset {} $w:* 
    }

    # -------------
    # dispatch 
    # -
    # dispatch the operations of a hugecombo widget
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
        if {![info exists oper]} { error "bad option \"$cmd\": must be $optmsg" }
        # check args
        set n [llength $args]
        if {[info exists cmdmsg($oper:$n)]} { set msg $cmdmsg($oper:$n) } \
        else { set msg $cmdmsg($oper) }
        if {$msg != ""} { error "wrong # args: should be \"$w $msg\"" }
        # eval operation
        eval [linsert $args 0 w:$oper $w]
      } msg]
      # return result
      if {$rc} \
      {
        set code error
        if {![string match "unknown component*" $msg]} \
        {
          set map [list $w.e $w $w.t.f.l $w entry hugecombo hugelist hugecombo]
          set msg [string map $map $msg]
        }
      } \
      else { set code ok }
      return -code $code $msg
    }

    # -------------
    # reference
    # -
    # trace reference name 
    # -------------
    # parm1: widget path
    # parm2: reference old name
    # parm3: reference new name
    # parm4: triggered op
    # -------------

    proc reference {w old new op} \
    {
      if {$new == ""} \
      { 
        # deleting
        catch { destroy $w } 
      } \
      else \
      { 
        # renaming
        uplevel 1 [format { set ::hugecombo::var [namespace which %s] } $new]
        set new $::hugecombo::var
        set ::hugecombo::($w:reference) $new 
      }
    }
  
  # ====================
  #
  # options
  #
  # ====================

    # --------------------
    # w:config
    # --
    # configure the widget
    # --------------------
    # parm1: path of the widget
    # parm2: widget config or option key or empty
    # -------------
    # return: option(s) description if parm2 is a key or empty
    # --------------------
    proc w:config {w args} \
    {
      variable {}
      # check if description request
      if {[llength $args] < 2} { return [eval get:option $w $args] }
      # update config
      foreach {key value} $args \
      {
        switch -glob -- $key \
        {
          -alt*       \
          {
            # -alternatecolor
            set ($w:-alternatecolor) $value
            $w.t.f.l config -altern $value
          }
          -bg         -
          -bac*       \
          {
            # -background
            set ($w:-background) $value
            $w.c config -bg $value
            $w.e config -bg $value
            $w.t.f.l config -bg $value
          }
          -bd         -
          -bor*       \
          {
            # -borderwidth
            set ($w:-borderwidth) [winfo pixels . $value]
            _$w config -bd $value
            $w.t.f.l config -bd $value
          }
          -com*       \
          {
            # -compound
            set ($w:-compound) [expr {$value ? 1 : 0}]
            if {$value} \
            { 
              
              set hh [winfo reqheight $w.e]
              $w.c config -height $hh -width $hh
              $w.c itemconfig img -image $($w:-image)
            } \
            else \
            { $w.c config -width 0 -height 0 }
          }
          -cur*       \
          {
            # -cursor
            set ($w:-cursor) $value
            _$w config -cursor $value
          }
          -disb*      -
          -disabledb* \
          {
            # -disabledbackground
            set ($w:-disabledbackground) $value
            $w.e config -disabledbackground $value
          }
          -disf*      -
          -disabledf* \
          {
            # -disabledforeground
            set ($w:-disabledforeground) $value
            $w.e config -disabledforeground $value
          }
          -exp*       \
          {
            # -exportselection
            set ($w:-exportselection) [expr {$value ? 1 : 0}]
            if {$value} \
            { 
              ::selection handle $w [namespace code [list xsel:handle $w]] 
              if {$($w:selection) != ""} { ::selection own $w }
            }
          }
          -fon*       \
          {
            # -font
            set ($w:-font) $value
            if {$value == ""} { set value $::hugecombo::font }
            $w.e config -font $value
            $w.t.f.l config -font $value
          }
          -fg         -
          -for*       \
          {
            # -foreground
            set ($w:-foreground) $value
            $w.e config -fg $value
            $w.t.f.l config -fg $value
          }
          -hei*       \
          {
            # -height
            if {![string is integer -strict $value]} \
            { error "expected integer but got \"$value\"" }
            set ($w:-height) $value
            $w.t.f.l config -height $value
          }
          -highb*     -
          -highlightb*  \
          {
            # -highlightbackground
            set ($w:-highlightbackground) $value
            _$w config -highlightbackground $value
          }
          -highc*     -
          -highlightc*  \
          {
            # -highlightcolor
            set ($w:-highlightcolor) $value
            _$w config -highlightcolor $value
          }
          -hight*     -
          -highlightt*  \
          {
            # -highlightthickness
            _$w config -highlightthickness $value
            set ($w:-highlightthickness) [_$w cget -highlightthickness]
          }
          -img        -
          -ima*       \
          {
            # -image
            set ($w:-image) $value
            if {$($w:-compound)} { $w.c itemconfig img -image $value }
          }
          -insbg      -
          -insba*     -
          -insertba*  \
          {
            # -insertbackground
            set ($w:-insertbackground) $value
            $w.e config -insertbackground $value
          }
          -insbg      -
          -insbo*     -
          -insertbo*  \
          {
            # -insertborderwidth
            set ($w:-insertborderwidth) [winfo pixels . $value]
            $w.e config -insertborderwidth $value
          }
          -insof*     -
          -insertof*  \
          {
            # -insertofftime
            set ($w:-insertofftime) $value
            $w.e config -insertofftime $value
          }
          -inson*     -
          -inserton*  \
          {
            # -insertontime
            set ($w:-insertontime) $value
            $w.e config -insertontime $value
          }
          -insw*      -
          -insertw*   \
          {
            # -insertwidth
            set ($w:-insertwidth) [winfo pixels . $value]
            $w.e config -insertwidth $value
          }
          -invc*      -
          -inva*      \
          {
            # -invalidcommand/-invcmd
            set ($w:-invalidcommand) $value
            if {$($w:-state) != "picklist"} { $w.e config -invalidcommand $value }
          }
          -jus*       \
          {
            # -justify
            set ($w:-justify) $value
            $w.e config -justify $value
          }
          -lis*       \
          {
            # -listvariable
            listvar $w $value
          }
          -rea*       \
          {
            # -readonlybackground
            set ($w:-readonlybackground) $value
            $w.e config -readonlybackground $value
          }
          -rel*       \
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
          -row*       \
          {
            # -rowheight
            if {$value != ""} { set value [winfo pixels . $value] }
            set ($w:-rowheight) $value
            $w.t.f.l config -rowheight $value
          }
          -selbg      -
          -selba*     -
          -selectba*  \
          {
            # -selectbackground
            set ($w:-selectbackground) $value
            $w.e config -selectbackground $value
            $w.t.f.l config -selectbackground $value
          }
          -selbd      -
          -selbo*     -
          -selectbo*  \
          {
            # -selectborderwidth
            set ($w:-selectborderwidth) [winfo pixels . $value]
            $w.e config -selectborderwidth $value
            $w.t.f.l config -selectborderwidth $value
          }
          -self*      -
          -selectf*   \
          {
            # -selectforeground
            set ($w:-selectforeground) $value
            $w.e config -selectforeground $value
            $w.t.f.l config -selectforeground $value
          }
          -sho*       \
          {
            # -show
            set ($w:-show) $value
            $w.e config -show $value
          }
          -sta*       \
          {
            # -state
            switch -glob -- $value \
            {
              d*    { set value disabled }
              n*    { set value normal }
              p*    { set value picklist }
              r*    { set value readonly }
              default { error "bad state \"$value\": must be disabled, normal, picklist or readonly" }
            }
            set old $($w:-state)
            if {$old == "picklist" && $value != $old} \
            {
              # apply -validate, -vcmd & -invcmd options
              $w.e config -validate $($w:-validate) \
                          -vcmd $($w:-validatecommand) \
                          -invcmd $($w:-invalidcommand)
            }
            if {$value == "picklist"} \
            {
              # entry can only get a picklist value
              $w.e config -validate all \
                          -vcmd [namespace code [list top:validate $w %P %S %d]] \
                          -invcmd [namespace code [list top:invalid $w]]
            }
            set ($w:-state) $value
            if {$value == "picklist"} { set value normal }
            $w.e config -state $value
          }
          -tak*       \
          {
            # -takefocus
            set ($w:-takefocus) $value
            _$w config -takefocus $value
          }
          -tex*       \
          {
            # -textvariable
            set ($w:-textvariable) $value
            $w.e config -textvariable $value
          }
          -validate   \
          {
            # -validate
            set ($w:-validate) $value
            switch -glob -- $value \
            {
              all     { set value all }
              focus   { set value focus }
              focusi* { set value focusin }
              focuso* { set value focusout }
              key     { set value key }
              non*    { set value none }
              default { error "bad validation mode \"$value\": must be none, focus, focusin, focusout, key or all" }
            }
            set ($w:-validate) $value
            if {$($w:-state) != "picklist"} { $w.e config -validate $value }
          }
          -vcm*       -
          -validatec* \
          {
            # -validatecommand/-vcmd
            set ($w:-validatecommand) $value
            if {$($w:-state) != "picklist"} { $w.e config -validatecommand $value }
          }
          -valu*      \
          {
            # -values
            set ($w:values) $value
            listvar $w ""
          }
          -wid*       \
          {
            # -width
            if {![string is integer -strict $value]} \
            { error "expected integer but got \"$value\"" }
            set ($w:-width) $value
            $w.e config -width $value
          }
          -xsc*       \
          {
            # -xscrollcommand
            set ($w:-xscrollcommand) $value
            $w.e config -xscrollcommand $value
          }
          default     { error "unknown option \"$key\"" }
        }
      }
    }
    
    # --------------------
    # get:option
    # --
    # get option(s) description
    # --------------------
    # parm1: path of the widget
    # parm2: key or empty
    # -------------
    # return: option(s) description
    # --------------------
    proc get:option {w {key ""}} \
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
          if {[string match $key $name]} \
          {
            if {[llength $option] == 2} \
            { set result [get:option $w [lindex $option 1]] } \
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
    
    # --------------------
    # w:cget
    # --
    # get an option value
    # --------------------
    # parm1: path of the widget
    # parm2: key
    # -------------
    # return: associated option value
    # --------------------
    proc w:cget {w key} \
    {
      variable {}
      switch -glob -- $key \
      {
        -alt*       \
        {
          # -alternatecolor
          set ($w:-alternatecolor)
        }
        -bg         -
        -bac*       \
        {
          # -background
          set ($w:-background)
        }
        -bd         -
        -bor*       \
        {
          # -borderwidth
          set ($w:-borderwidth)
        }
        -com*       \
        {
          # -compound
          set ($w:-compound)
        }
        -cur*       \
        {
          # -cursor
          set ($w:-cursor)
        }
        -disb*      -
        -disabledb* \
        {
          # -disabledbackground
          set ($w:-disabledbackground)
        }
        -disf*      -
        -disabledf* \
        {
          # -disabledforeground
          set ($w:-disabledforeground)
        }
        -exp*       \
        {
          # -exportselection
          set ($w:-exportselection)
        }
        -fon*       \
        {
          # -font
          set ($w:-font)
        }
        -fg         -
        -for*       \
        {
          # -foreground
          set ($w:-foreground)
        }
        -hei*       \
        {
          # -height
          set ($w:-height)
        }
        -highb*     -
        -highlightb*  \
        {
          # -highlightbackground
          set ($w:-highlightbackground)
        }
        -highc*     -
        -highlightc*  \
        {
          # -highlightcolor
          set ($w:-highlightcolor)
        }
        -hight*     -
        -highlightt*  \
        {
          # -highlightthickness
          set ($w:-highlightthickness)
        }
        -img        -
        -ima*       \
        {
          # -image
          set ($w:-image)
        }
        -insbg      -
        -insba*     -
        -insertba*  \
        {
          # -insertbackground
          set ($w:-insertbackground)
        }
        -insbg      -
        -insbo*     -
        -insertbo*  \
        {
          # -insertborderwidth
          set ($w:-insertborderwidth)
        }
        -insof*     -
        -insertof*  \
        {
          # -insertofftime
          set ($w:-insertofftime)
        }
        -inson*     -
        -inserton*  \
        {
          # -insertontime
          set ($w:-insertontime)
        }
        -insw*      -
        -insertw*   \
        {
          # -insertwidth
          set ($w:-insertwidth)
        }
        -invc*      -
        -inva*      \
        {
          # -invalidcommand/-invcmd
          set ($w:-invalidcommand)
        }
        -jus*       \
        {
          # -justify
          set ($w:-justify)
        }
        -lis*       \
        {
          # -listvariable
          set ($w:-listvariable)
        }
        -rea*       \
        {
          # -readonlybackground
          set ($w:-readonlybackground)
        }
        -rel*       \
        {
          # -relief
          set ($w:-relief)
        }
        -row*       \
        {
          # -rowheight
          set ($w:-rowheight)
        }
        -selbg      -
        -selba*     -
        -selectba*  \
        {
          # -selectbackground
          set ($w:-selectbackground)
        }
        -selbd      -
        -selbo*     -
        -selectbo*  \
        {
          # -selectborderwidth
          set ($w:-selectborderwidth)
        }
        -self*      -
        -selectf*   \
        {
          # -selectforeground
          set ($w:-selectforeground)
        }
        -sho*       \
        {
          # -show
          set ($w:-show)
        }
        -sta*       \
        {
          # -state
          set ($w:-state)
        }
        -tak*       \
        {
          # -takefocus
          set ($w:-takefocus)
        }
        -tex*       \
        {
          # -textvariable
          set ($w:-textvariable)
        }
        -validate   \
        {
          # -validate
          $w.e cget -validate
        }
        -vcm*       -
        -validatec* \
        {
          # -validatecommand/-vcmd
          set ($w:-validatecommand)
        }
        -valu*      \
        {
          # -values
          set [set ($w:listvariable)]
        }
        -wid*       \
        {
          # -width
          set ($w:-width)
        }
        -xsc*       \
        {
          # -xscrollcommand
          set ($w:-xscrollcommand)
        }
        default     { error "unknown option \"$key\"" }
      }
    }
    
  # ====================
  #
  # create/reference toplevel
  #
  # ====================

    # --------------------
    # toplevel constructor
    # --
    # create the widget associated toplevel
    # --------------------
    # parm1: path of the widget
    # --------------------
    proc top:create {w} \
    {
      variable {}
      # create toplevel, hugelist and scroll bar
      toplevel $w.t
      wm withdraw $w.t
      wm overrideredirect $w.t 1
      frame $w.t.f -relief $($w:t:relief) -bd $($w:-borderwidth)
      scrollbar $w.t.f.vs -command [namespace code [list top:yview $w]]
      hugelist $w.t.f.l -listvariable $($w:listvariable) \
               -height $($w:-height) -bg $($w:-background)\
               -relief $($w:l:relief) -bd $($w:l:borderwidth) \
               -font [$w.e cget -font] -alt $($w:-alternatecolor) \
               -activestyle $($w:l:activestyle) -rowheight $($w:-rowheight) \
               -highlightthickness $($w:l:highlightthickness) \
               -yscrollcommand [namespace code [list top:yset $w]]              
      pack $w.t.f
      grid $w.t.f.l  -row 0 -column 0 -padx 1 -pady 1
      grid $w.t.f.vs -row 0 -column 1 -sticky ns
      # binding
      bind $w.t.f.l  <<ListboxSelect>>  [namespace code [list top:load $w]]
      bind $w.t.f.l  <Motion>           [namespace code [list top:move   $w @0,%y]]
      bindtags $w.t.f.l [linsert [bindtags $w.t.f.l] 1 $($w:TAG2)]
    }

    # --------------------
    # toplevel resize
    # --
    # resize the toplevel
    # --------------------
    # parm1: path of the widget
    # --------------------
    proc top:resize {w} \
    {
      variable {}
      set fw [winfo width $w]
      set sw 0
      if {$($w:scrolled)} { set sw [winfo width $w.t.f.vs] }
      set font [$w.t.f.l cget -font]
      set cw [font measure $font -displayof $w.t.f.l "0"]
      set bd 0
      if {[$w.t cget -relief] != "flat"} { incr bd [$w.t cget -bd] }
      if {[$w.t.f cget -relief] != "flat"} { incr bd [$w.t.f cget -bd] }
      if {[$w.t.f.l cget -relief] != "flat"} { incr bd [$w.t.f.l cget -bd] }
      $w.t.f.l config -width [expr {($fw - $sw - 2 * $bd) / $cw}]
    }

  # ====================
  #
  # toplevel actions
  #
  # ====================

  # --------------------
  # dispatching
  # --------------------

    # --------------------
    # w:list
    # --
    # dispatch the command
    # --------------------
    # parm1: command
    # parm2: args of the command or empty
    # --------------------
    # return: return of the command
    # --------------------
    proc w:list {w cmd args} \
    {
      variable {}
      variable listlist
      variable patlist
      variable listmsg
      # retrieve command
      foreach {pattern op} $patlist \
      { if {[string match $pattern $cmd]} { set oper $op; break } }
      if {![info exists oper]} { error "bad option \"$cmd\": must be $listlist" }
      # check args
      set n [llength $args]
      if {[info exists listmsg($oper:$n)]} { set msg $listmsg($oper:$n) } \
      else { set msg $listmsg($oper) }
      if {$msg != ""} { error "wrong # args: should be \"$w $msg\"" }
      # eval operation
      eval [linsert $args 0 list:$oper $w]
    }
    
  # --------------------
  # getting widget path
  # --------------------

    # --------------------
    # top:widget
    # --
    # get widget path
    # --------------------
    # parm1: path of the subwidget
    # --------------------
    # return: path of the widget
    # --------------------
    proc top:widget {w} \
    {
      switch [winfo class $w] \
      {
        Entry     { set w [winfo parent $w] }
        Listbox   -
        Hugelist  { set w [join [lrange [split $w .] 0 end-3] .] }
      }
      return $w
    }
  
  # --------------------
  # opening/closing
  # --------------------

    # --------------------
    # top:toggle
    # --
    # toggle between open & close
    # --------------------
    # parm1: path of the widget
    # --------------------
    proc top:toggle {w} \
    {
      variable {}
      set w [top:widget $w]
      # toggle between open & close
      if {$($w:opened)} { top:close $w } else { top:open $w }
    }
    
    # --------------------
    # list:openlist
    # --
    # open the toplevel
    # --------------------
    # parm1: path of the widget
    # --------------------
    proc list:open {w} { update; top:open $w }
    
    # --------------------
    # top:open
    # --
    # open the toplevel
    # --------------------
    # parm1: path of the widget
    # --------------------
    proc top:open {w} \
    {
      variable {}
      set w [top:widget $w]
      # check if openable
      switch $($w:-state) disabled - readonly { return }
      # set opened flag
      set ($w:opened) 1
      # open toplevel
      focus -force $w.e
      set x [expr [winfo rootx $w] - [$w.t cget -bd] - $($w:-borderwidth)]
      set y [expr [winfo rooty $w] + [winfo height $w]]
      if {[expr {$y + [winfo height $w.t]}] > [winfo screenheight $w.t]} \
      { set y [expr [winfo rooty $w] - [winfo height $w.t]] }
      # resize, place & show the toplevel
      top:resize $w
      wm geometry $w.t +$x+$y
      wm deiconify $w.t
      raise $w.t
      # set grab
      array unset {} $w:grab:*
      foreach win [grab current] { set ($w:grab:$win) [grab status $win] }
      grab set -global $w
      # init finding
      set ($w:find) 0
    }

    # --------------------
    # list:close
    # --
    # close the toplevel
    # --------------------
    # parm1: path of the widget
    # --------------------
    proc list:close {w} { top:close $w }
    
    # --------------------
    # top:close
    # --
    # close the toplevel
    # --------------------
    # parm1: path of the widget
    # --------------------
    proc top:close {w} \
    {
      variable {}
      set w [top:widget $w]
      if {!$($w:opened)} { return }
      # reset opened flag
      set ($w:opened) 0
      # close window
      wm withdraw $w.t
      # restitute old grab
      grab release $w
      foreach win [array names ::grab] \
      {
        switch $::grab($win) \
        {
          local     { grab set $win }
          global    { grab set -global $win }
        }
      }
    }

    # --------------------
    # top:validate
    # --
    # check the value
    # --------------------
    # parm1: path of the widget
    # parm2: the new value
    # parm3: the search letter
    # parm4: the operation
    # --------------------
    proc top:validate {w new search oper} \
    {
      variable {}
      if {$($w:loading)} { return 1 }
      # check entry value
      if {[lsearch [set $($w:listvariable)] $new] == -1} \
      { # not found in the list
        if {$oper} \
        { 
          # find search letter
          if {!$($w:opened)} { top:toggle $w }
          set first ""
          set index [$w.t.f.l curselection]
          if {$index != ""} \
          {
            set current [$w.t.f.l get $index]
            if {[string index $current 0] != $search} { set first 0 }
          }
          list:find $w $search* $first
        }
        # restore old or set enw value
        after idle [namespace code [list top:invalid $w]]
      }
      return 1
    }
  
    # --------------------
    # top:invalid
    # --
    # restitute the old value
    # --------------------
    # parm1: path of the widget
    # parm2: old value
    # --------------------
    proc top:invalid {w} \
    {
      variable {}
      set ($w:loading) 1
      $w.e delete 0 end
      $w.e insert 0 $($w:oldvalue)
      after idle [list $w.e config -validate all]
      set ($w:loading) 0
    }
  
  # --------------------
  # raising 
  # --------------------

    # --------------------
    # top:raise
    # --
    # raise the toplevel
    # --------------------
    # parm1: path of the widget
    # --------------------
    proc top:raise {w} \
    {
      set w [top:widget $w]
      raise $w.t $w
    }

  # --------------------
  # scrolling
  # --------------------

    # --------------------
    # top:yview
    # --
    # reset focus to entry after scroll bar activation
    # --------------------
    # parm1: path of the widget
    # --------------------
    proc top:yview {w args} \
    {
      set w [top:widget $w]
      eval $w.t.f.l yview $args
      focus -force $w.e
    }

    # --------------------
    # top:yset
    # --
    # show/hide the scroll bar
    # --------------------
    # parm1: path of the widget
    # --------------------
    proc top:yset {w args} \
    {
      variable {}
      set w [top:widget $w]
      eval $w.t.f.vs set $args
      if {$args == {0 1}} \
      { 
        grid forget $w.t.f.vs 
        set ($w:scrolled) 0
      } \
      else \
      { 
        grid $w.t.f.vs -row 0 -column 1 -sticky ns 
        set ($w:scrolled) 1
      }
      top:resize $w
    }

  # --------------------
  # loading
  # --------------------

    # --------------------
    # w:load
    # --
    # load the entry value with the value of a row
    # --------------------
    # parm1: path of the widget
    # parm2: index of the row or empty
    # --------------------
    # return: selected value
    # --------------------
    proc w:load {w {index ""}} \
    { 
      variable {}
      if {$index == ""} { set index [$w.t.f.l curselection] }
      if {$index == ""} { set index 0 }
      top:load $w $index 0
      $w.e get
    }
    
    # --------------------
    # top:load
    # --
    # change the entry value with a row value
    # --------------------
    # parm1: path of the widget
    # parm2: index of the row or empty
    # parm3: close flag or empty
    # --------------------
    proc top:load {w {index ""} {close 1}} \
    {
      variable {}
      set ($w:loading) 1
      set w [top:widget $w]
      if {$index == ""} \
      {
        if {$close} \
        { 
          if {$index == "end"} \
          { set index [expr {$($w:count) - 1}] } \
          else { set index [$w.t.f.l curselection] }
          set ($w:curselection) $index
        } \
        else \
        { set index $($w:curselection) }
        if {$index == ""} { set index 0 }
      }
      $w.e delete 0 end
      if {$($w:count) == 0} \
      {
        $w.c itemconfig img -image {}
        set ($w:-image) {}
      } \
      else \
      { 
        set value [lindex [set $($w:listvariable)] $index]
        set ($w:oldvalue) $value
        $w.e insert 0 $value
        set img [$w.t.f.l itemcget $index -img]
        $w.c itemconfig img -image $img
        set ($w:-image) $img
        if {$($w:-compound) && $img != ""} \
        { 
          set hh [winfo reqheight $w.e]
          set x [expr {($hh - [image width $img]) / 2}]
          set y [expr {($hh - [image height $img]) / 2}]
          $w.c coords img $x $y 
        }
      }
      if {$close} { top:close $w}
      set ($w:loading) 0
    }

  # --------------------
  # moving
  # --------------------

    # --------------------
    # w:send
    # --
    # simulate a key send
    # --------------------
    # parm1: path of the widget
    # parm2: key name
    # --------------------
    proc w:send {w key} \
    {
      variable {}
      update
      switch -- $key \
      {
        <Escape>          { top:close  $w }
        <Return>          { top:load $w }
        <Control-Return>  { top:find   $w }
        <Control-Home>    { top:home   $w }
        <Prior>           { top:prior  $w }
        <Up>              { top:up     $w }
        <Down>            { top:down   $w }
        <Next>            { top:next   $w }
        <Control-End>     { top:end    $w }
        default           { error "unknown key symbol \"$key\", should be <Control-End>, <Control-Home>, <Control-Return>, <Down>, <Escape>, <Next>, <Prior>, <Return> or <Up>" }
      }
    }
    
    # --------------------
    # list:select
    # --
    # move the index of the selected row
    # --------------------
    # parm1: path of the widget
    # parm2: index of the new selected row
    # --------------------
    proc list:select {w index} { top:move $w $index }

    # --------------------
    # top:move
    # --
    # move the index of the selected row
    # --------------------
    # parm1: path of the widget
    # parm2: index of the new selected row
    # --------------------
    proc top:move {w index} \
    {
      set w [top:widget $w]
      $w.t.f.l selection clear 0 end
      $w.t.f.l selection set $index
      $w.t.f.l see $index
    }

    # --------------------
    # top:home
    # --
    # move the selected row to the top
    # --------------------
    # parm1: path of the widget
    # --------------------
    proc top:home {w} \
    {
      variable {}
      set w [top:widget $w]
      if {!$($w:opened)} { top:toggle $w }
      top:move $w 0
    }

    # --------------------
    # top:prior
    # --
    # move the selected one page before
    # --------------------
    # parm1: path of the widget
    # --------------------
    proc top:prior {w} \
    {
      variable {}
      set w [top:widget $w]
      if {!$($w:opened)} { top:toggle $w }
      set index [$w.t.f.l curselection]
      if {$index == ""} { top:home $w } \
      else \
      {
        incr index
        incr index -$($w:-height)
        if {$index < 0} { top:home $w } \
        else { top:move $w $index }
      }
    }

    # --------------------
    # top:up
    # --
    # move the selected one row before
    # --------------------
    # parm1: path of the widget
    # --------------------
    proc top:up {w} \
    {
      variable {}
      set w [top:widget $w]
      if {!$($w:opened)} { top:toggle $w }
      set index [$w.t.f.l curselection]
      if {$index == "" || $index == 0} { top:home $w } \
      else { top:move $w [incr index -1] }
    }

    # --------------------
    # top:down
    # --
    # move the selected one row after
    # --------------------
    # parm1: path of the widget
    # --------------------
    proc top:down {w} \
    {
      variable {}
      set w [top:widget $w]
      if {!$($w:opened)} { top:toggle $w }
      set index [$w.t.f.l curselection]
      if {$index == ""} { top:home $w } \
      elseif {$index + 1 < $($w:count)} { top:move $w [incr index] }
    }

    # --------------------
    # top:next
    # --
    # move the selected one page after
    # --------------------
    # parm1: path of the widget
    # --------------------
    proc top:next {w} \
    {
      variable {}
      set w [top:widget $w]
      if {!$($w:opened)} { top:toggle $w }
      set index [$w.t.f.l curselection]
      if {$index == ""} { top:home $w } \
      else \
      {
        incr index -1
        incr index $($w:-height)
        if {$index >= $($w:count)} { top:end $w } \
        else { top:move $w $index }
      }
    }

    # --------------------
    # top:home
    # --
    # move the selected row to the bottom
    # --------------------
    # parm1: path of the widget
    # --------------------
    proc top:end {w} \
    {
      variable {}
      set w [top:widget $w]
      if {!$($w:opened)} { top:toggle $w }
      top:move $w [expr {$($w:count) - 1}]
    }

  # --------------------
  # finding
  # --------------------

    # --------------------
    # list:cursel
    # --
    # return the index of the current selected row
    # --------------------
    # parm1: path of the widget
    # --------------------
    # return: the index of the current selected row
    # --------------------
    proc list:cursel {w} { variable {}; return [$w.t.f.l curselection] }
    
    # --------------------
    # list:find
    # --
    # find the next occurrence of the pattern
    # --------------------
    # parm1: path of the widget
    # parm2: pattern to search
    # parm3: starting index or empty
    # --------------------
    # return: the selected value
    # --------------------
    proc list:find {w pattern {index ""}} \
    {
      variable {}
      if {$index == ""} \
      { 
        set index [$w.t.f.l curselection]
        if {$index == ""} { set index 0 } \
        elseif {$($w:find)} { incr index }
      }
      set ($w:find) 1
      for {set i $index} {$i < $($w:count)} {incr i} \
      {
        set item [lindex [set $($w:listvariable)] $i]
        if {[string match $pattern $item]} { top:move $w $i; return }
      }
    }
    
    # --------------------
    # top:find
    # --
    # find the next occurrence of the pattern
    # --------------------
    # parm1: path of the widget
    # --------------------
    proc top:find {w} \
    {
      variable {}
      set w [top:widget $w]
      if {!$($w:opened)} { top:toggle $w; return }
      list:find $w [$w.e get]
    }

  # ====================
  #
  # entry operations
  #
  # ====================

    # --------------------
    # w:bbox
    # --
    # 
    # --------------------
    # parm1: path of the widget
    # --------------------
    proc w:bbox {w args} { eval $w.e bbox $args }

    # --------------------
    # w:cursel
    # --
    # return the entry selection
    # --------------------
    # parm1: path of the widget
    # --------------------
    proc w:cursel {w} { variable {}; return $($w:selection) }

    # --------------------
    # w:delete
    # --
    # 
    # --------------------
    # parm1: path of the widget
    # --------------------
    proc w:delete {w args} { eval $w.e delete $args }

    # --------------------
    # w:get
    # --
    # 
    # --------------------
    # parm1: path of the widget
    # --------------------
    proc w:get {w args} { eval $w.e get $args }

    # --------------------
    # w:icursor
    # --
    # 
    # --------------------
    # parm1: path of the widget
    # --------------------
    proc w:icursor {w args} { eval $w.e icursor $args }

    # --------------------
    # w:index
    # --
    # 
    # --------------------
    # parm1: path of the widget
    # --------------------
    proc w:index {w args} { eval $w.e index $args }

    # --------------------
    # w:insert
    # --
    # 
    # --------------------
    # parm1: path of the widget
    # --------------------
    proc w:insert {w args} { eval $w.e insert $args }

    # --------------------
    # w:scan
    # --
    # 
    # --------------------
    # parm1: path of the widget
    # --------------------
    proc w:scan {w args} { eval $w.e scan $args }

    # --------------------
    # w:selection
    # --
    # 
    # --------------------
    # parm1: path of the widget
    # --------------------
    proc w:selection {w args} \
    { 
      set res [eval $w.e selection $args ]
      # set X selection
      variable {}
      if {![catch { set first [$w.e index sel.first] }]} \
      {
        set last [$w.e index sel.last]
        set ($w:selection) [string range [$w.e get] $first [incr last -1]]
      } \
      else { set ($w:selection) "" }
      if {$($w:-exportselection)} \
      { 
        if {$($w:selection) == ""} { ::selection clear -selection PRIMARY } \
        else { ::selection own $w }
      }
      return $res
    }

    # --------------------
    # w:validate
    # --
    # 
    # --------------------
    # parm1: path of the widget
    # --------------------
    proc w:validate {w args} { eval $w.e validate $args }
  
    # --------------------
    # w:xview
    # --
    # 
    # --------------------
    # parm1: path of the widget
    # --------------------
    proc w:xview {w args} { eval $w.e xview $args }
  
  # ====================
  #
  # list operations
  #
  # ====================

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
      # save new name & old name
      set realname $listname
      set oldname $($w:listvariable)
      # get name
      if {$listname == ""} \
      { 
        if {$oldname != ""} { set ::hugecombo::($w:values) [set $oldname] }
        set listname ::hugecombo::($w:values) 
      } \
      else \
      {
        if {[string range $listname 0 1] != "::"} \
        {
          uplevel 3 { set ::hugecombo::var [namespace current] }
          append ::hugecombo::var ::$listname
          set listname $::hugecombo::var
        }
      }
      # check if name changed
      if {$oldname == $listname && $realname != ""} { return }
      # delete old trace
      if {$oldname != ""} { trace vdelete $oldname w [namespace code [list set:list $w]] }
      # create list if not exists
      if {![info exists $listname]} \
      { 
        # get old values
        set $listname {}
        if {$oldname != ""} { set $listname [set $oldname] }
      }
      # update items count & listvariable
      set ($w:count) [llength [set $listname]]
      set ($w:listvariable) $listname 
      # save real name
      set ($w:-listvariable) $realname 
      # set new trace
      trace variable $listname w [namespace code [list set:list $w]]
      # set hugelist
      $w.t.f.l config -listvar $listname
      # set entry
      top:load $w "" 0
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
      # set list count
      set listname $::hugecombo::($w:listvariable)
      set ::hugecombo::($w:count) [llength [set $listname]]
    }

    # --------------------
    # w:itemcget
    # --
    # 
    # --------------------
    # parm1: path of the widget
    # --------------------
    proc w:itemcget {w args} { eval $w.t.f.l itemcget $args }

    # --------------------
    # w:itemconf
    # --
    # 
    # --------------------
    # parm1: path of the widget
    # --------------------
    proc w:itemconf {w args} \
    {
      variable {}
      # do the operation
      eval $w.t.f.l itemconf $args
      # check if selected item changed
      if {$($w:-compound)} \
      {
        set index [$w.t.f.l curselection]
        if {$index == ""} { set index 0 }
        set index [$w.t.f.l index $index]
        if {$index == [$w.t.f.l index [lindex $args 0]]} \
        { top:load $w $index 0 }
      }
    }

    # --------------------
    # w:size
    # --
    # 
    # --------------------
    # parm1: path of the widget
    # --------------------
    proc w:size {w} { variable {}; return $($w:count) }

  # ====================
  #
  # miscellaneous
  #
  # ====================

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
      set res [string range $($w:selection) $offset [expr {$offset + $maxchars - 1}]]
      if {$($w:-show) != ""} { set res [string repeat $($w:-show) [string length $res]] }
      return $res
    }
  
    # -------------
    # component
    # -
    # return a component path
    # -------------
    # parm1: widget path
    # parm2: component name
    # -------------
    proc w:component {w name} \
    {
      variable {}
      switch -glob -- $name \
      {
        but*    { return $w.b }
        can*    { return $w.c }
        ent*    { return $w.e }
        hug*    -
        lis*    { return $w.t.f.l }
        scr*    { return $w.t.f.vs }
        default { error "unknown component \"$name\": must be button, canvas, entry, hugelist, listbox or scrollbar" }
      }
    }
    
}
# end of the ::hugecombo namespace definition


