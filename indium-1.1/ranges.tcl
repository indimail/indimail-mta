if {![catch {namespace children ::ranges}]} { return }
namespace eval ::ranges \
{
# beginning of ::ranges namespace definition

  namespace export ranges

# ####################################
#
#   ranges package
#
set version 1.1
#
#   ulis, (C) 2002
#
# ------------------------------------
# associate a value to a range of numbered items
#
# ####################################

  # ==========================
  #
  # package
  #
  # ==========================

  package provide Ranges $version
  package provide ranges $version

  # ------------------------
  # constructor
  # ------------------------
  # parm1: ranges name
  # ------------------------
  # return: name
  # ------------------------
  proc ranges {name args} \
  {
    variable {}
    # init private data
    set ($name:ranges) {{0}} 
      # ranges syntax:
      # {<starting item> <ID>}...{<starting item> <ID>} {<items count>}
    set ($name:values) {{}}
    set ($name:curID) 0
    set ($name:lastIDs) {0}
    set ($name:deep) 4
    set ($name:stat:-all) 0
    set ($name:stat:-cur) 0
    set ($name:stat:0) 0
    set ($name:lastref) {0 0 0}
    # create associated command
    interp alias {} $name {} ::ranges::dispatch $name
    if {![catch {package present Tcl 8.4}]} \
    { trace add command $name delete [list ::ranges::dispose $name] }
    # configure
    eval config $name $args
    # return name
    return $name
  }
  
  # ------------------------
  # destructor
  # ------------------------
  # parm1: ranges name
  # ------------------------
  proc destroy {name} \
  {
    # delete private data
    dispose $name
    # delete associated command
    interp alias {} $name {}
  }
  
  # ------------------------
  # internal destructor
  # ------------------------
  # parm1: ranges name
  # parm2: don't care
  # ------------------------
  proc dispose {name args} \
  { 
    variable {}
    # delete private data
    array unset {} $name:*
  }
  
  # ------------------------
  # dispatch
  # ------------------------
  # parm1: ranges name
  # parm2: operation
  # parm3: operation args
  # ------------------------
  # return: operation result
  # ------------------------
  proc dispatch {name cmd args} \
  { #if {$cmd != "get"} { puts "$name $cmd $args" }
    set rc [catch {
      switch -glob -- $cmd \
      {
        cge*  { eval [linsert $args 0 cget        $name] }
        con*  { eval [linsert $args 0 config      $name] }
        del*  { eval [linsert $args 0 delete      $name] }
        des*  { eval [linsert $args 0 destroy     $name] }
        get   { eval [linsert $args 0 get         $name] }
        ins*  { eval [linsert $args 0 insert      $name] }
        rep*  { eval [linsert $args 0 replace     $name] }
        set   { eval [linsert $args 0 _set        $name] }
        default { error "Unknown $name operation: $cmd" }
      }
    } msg]
    set code [expr {$rc ? "error" : "ok"}]
    return -code $code $msg
  }

  # ------------------------
  # config
  # ------------------------
  # parm1: ranges name
  # parm2: option-pairs list
  # ------------------------
  proc config {name args} \
  {
    variable {}
    foreach {key value} $args \
    {
      switch -glob -- $key \
      {
        -dee*   \
        { 
          if {$value < 1} { error "$name ranges: -deep option value must be greater than 0" }
          set ($name:deep) $value 
          if {[llength $($name:lastIDs)] > $value} \
          {
            incr value -1
            set ($name:lastIDs) [lrange $($name:lastIDs) end-$value end]
          }
        }
        default { error "$name ranges: unknown option key '$key'" }
      }
    }
  }
  
  # ------------------------
  # cget
  # ------------------------
  # parm1: ranges name
  # parm2: option key
  # ------------------------
  # return: option value
  # ------------------------
  proc cget {name key} \
  {
    variable {}
    switch -glob -- $key \
    {
      -cou*   { lindex $($name:ranges) end }
      -dee*   { set ($name:deep) }
      -sta*   \
      {
        set res {}
        foreach n [array names {} $name:stat:*] \
        {
          set v $($n)
          if {$v > 0} \
          {
            set ID [lindex [split $n :] 2]
            lappend res [list $ID $v]
          }
        }
        return $res
      }
      default { error "$name ranges: unknown option key '$key'" }
    }
  }
  
  # ------------------------
  # get the value, the range or the id of an item
  # ------------------------
  # parm1: ranges name
  # parm2: <item>, end, value <item>, range <item> or id <item>
  # ------------------------
  # return: value, range or id
  # ------------------------
  proc get {name args} \
  {
    variable {}
    # check args
    switch [llength $args] \
    {
      1       { set key value; set item $args }
      2       { foreach {key item} $args break }
      default \
      { error "should be $name get item or $name get op item" }
    }
    # get ranges
    set ranges $($name:ranges)
    set items_count [lindex [lindex $ranges end] 0]
    if {[string match e* $item]} { set item [expr {$items_count - 1}] }
    if {![string is integer -strict $item]} { error "ranges $name: item should be integer" }
    if {$item == -1} { error "can't get item $item: $name ranges is empty" }
    if {$item < 0 || $item > $items_count} \
    { error "item $item is outside $name ranges (0-$items_count)" }
    # search item range
    set ID 0
    set last 0
    foreach range $($name:ranges) \
    {
      foreach {start newID} $range break
      if {$start > $item} { break }
      set last $start
      set ID $newID
    }
    # return result
    switch -glob -- $key \
    {
      value   { return [expr {$ID == "" ? "" : [lindex $($name:values) $ID]}] }
      range   { return [expr {$last == $start ? {} : [list $last [incr start -1]]}] }
      id      { return $ID }
      default { error "ranges $name get: <item>, end, value <item>, range <item> or id <item>" }
    }
  }
  
  # ------------------------
  # set a value
  # ------------------------
  # parm1: ranges name
  # parm2: value ID
  # parm3: new value
  # ------------------------
  proc _set {name ID value} \
  {
    variable {}
    # check args
    set values $($name:values)
    if {$ID < 0 || $ID >= [llength $values]} \
    { error "ranges $name: value index is out of range" }
    set ($name:values) [lreplace $values $ID $ID $value]
  }
  
  # ------------------------
  # newvalue
  # ------------------------
  # parm1: ranges name
  # parm2: value
  # ------------------------
  # return: ID corresponding to value
  # ------------------------
  proc newvalue {name value} \
  {
    variable {}
    # get ID
    set ID $($name:curID)
    set curvalue [lindex $($name:values) $ID]
    if {$value != $curvalue} \
    {
      set found 0
      foreach ID $($name:lastIDs) \
      {
        set lastvalue [lindex $($name:values) $ID]
        if {$value == $lastvalue} \
        { 
          incr ($name:stat:$ID)
          set found 1
          break 
        }
      }
      if {!$found} \
      {
        set ID [incr ($name:curID)]
        lappend ($name:values) $value
        lappend ($name:lastIDs) $ID
        set n [expr {$($name:deep) - 1}]
        set ($name:lastIDs) [lrange $($name:lastIDs) end-$n end]
        set ($name:stat:$ID) 0
      }
    } \
    else { incr ($name:stat:-cur) }
    incr ($name:stat:-all)
    # return ID
    return $ID
  }
  
  # ------------------------
  # insert
  # ------------------------
  # parm1: ranges name
  # parm2: item before which to insert
  # parm3: count of items to insert
  # parm4: value
  # ------------------------
  # return: ID corresponding to value
  # ------------------------
  proc insert {name first delta value} \
  {
    variable {}
    # get ID
    set ID [newvalue $name $value]
    # get ranges
    set ranges $($name:ranges)
    set items_count [lindex [lindex $ranges end] 0]
    # check args
    if {$delta < 0} { error "items count must be positive" }
    if {$delta == 0} { return }
    if {[string match e* $first]} { set first $items_count }
    set last [expr {$first + $delta - 1}]
    if {$first < 0 || $first > $items_count} \
    { error "range $first-$last is outside $name ranges (0-$items_count)" }
    # compute ranges
    set count [llength $ranges]
    set step 1
    set curID ""
    set n 0
    while {$n < $count} \
    {
      set range [lindex $ranges $n]
      foreach {start newID} $range break
      switch $step \
      {
        1 \
        {
          if {$start < $first} \
          { 
            # before first
              # keep old range
            lappend newranges $range
            set curID $newID
            incr n
          } \
          else \
          {
            # at first or after
              # append new range
            if {$ID != $curID} \
            { 
              lappend newranges [list $first $ID]
              if {$start != $first && $newID != ""} \
              { 
                lappend newranges [list [incr first $delta] $curID]
              } \
              elseif {$ID == $newID} { incr n }
            }
            set step 3
          }
        }
        3 \
        {
          # after last
            # keep old range
          if {$newID != ""} \
          { lappend newranges [list [incr start $delta] $newID] }
          incr n
        }
      }
    }
    # append items count
    lappend newranges [incr items_count $delta]
    # update ranges
    set ($name:ranges) $newranges
    # return ID
    return $ID
  }

  # ------------------------
  # replace
  # ------------------------
  # parm1: ranges name
  # parm2: first item
  # parm3: last item
  # parm4: value
  # ------------------------
  # return: ID corresponding to value
  # ------------------------
  proc replace {name first last value} \
  {
    variable {}
    # get ID
    set ID [newvalue $name $value]
    # get ranges
    set ranges $($name:ranges)
    set items_count [lindex [lindex $ranges end] 0]
    # check args
    if {[string match e* $last]} { set last $items_count }
    if {$last < $first} { foreach {first last} [list $last $first] break }
    if {$first < 0 || $first > $items_count} \
    { error "range $first-$last is outside $name ranges (0-$items_count)" }
    # compute ranges
    set count [llength $ranges]
    set step 1
    set curID ""
    set n 0
    while {$n < $count} \
    {
      set range [lindex $ranges $n]
      foreach {start newID} $range break
      switch $step \
      {
        1 \
        {
          if {$start < $first} \
          { 
            # before first
              # keep old range
            lappend newranges $range
            set curID $newID
            incr n
          } \
          else \
          {
            # at first or after
              # start new range
            if {$ID != $curID} { lappend newranges [list $first $ID] }
            set step 2
          }
        }
        2 \
        {
          if {$start > $last} \
          {
            # after last
              # end new range
            set next [expr {$last + 1}]
            if {$ID != $curID && $next != $start} \
            { lappend newranges [list $next $curID] }
            set step 3
          } \
          else \
          { 
            # at last or before
              # ignore old range
            set curID $newID 
            incr n
          }
        }
        3 \
        {
          # after last
            # keep old range
          if {$newID != ""} { lappend newranges $range }
          incr n
        }
      }
    }
    # append items count
    if {[incr last] > $items_count} { set items_count $last }
    lappend newranges $items_count
    # update ranges
    set ($name:ranges) $newranges
    # return ID
    return $ID
  }

  # ------------------------
  # delete
  # ------------------------
  # parm1: ranges name
  # parm2: first item
  # parm3: last item
  # ------------------------
  proc delete {name first last} \
  {
    variable {}
    # get ranges
    set ranges $($name:ranges)
    set items_count [lindex [lindex $ranges end] 0]
    # check args
    if {[string match e* $last]} { set last [expr {$items_count - 1}] }
    if {$last == -1} { return ; # empty }
    if {$last < $first} { foreach {first last} [list $last $first] break }
    if {$first < 0 || $first > $items_count} \
    { error "range $first-$last is outside $name ranges (0-$items_count)" }
    # check delete all
    if {$first == 0 && $last == [cget $name -count]} \
    {
      set ($name:ranges) {0}
      return
    }
    # compute ranges
    set count [llength $ranges]
    set step 1
    set encID ""
    set regID ""
    set delta [expr {$first - $last - 1}]
    set n 0
    while {$n < $count} \
    {
      set range [lindex $ranges $n]
      foreach {start newID} $range break
      switch $step \
      {
        1 \
        {
          if {$start < $first} \
          { 
            # before first
              # keep old range
            lappend newranges $range
            set encID $newID
            set regID $newID
            incr n
          } \
          else \
          {
            # at first or after
              # skip
            set step 2
          }
        }
        2 \
        {
          if {$start <= $last} \
          {
            # at last or before
              # skip
            set encID $regID
            incr n
          } \
          elseif {$start == $last + 1} \
          { 
            # just after last
              # keep old range, adjusting item 
            set step 3
          } \
          else \
          { 
            # after last
              # restart old range
            if {$newID != $encID && $encID != $regID} \
            {
              incr last
              incr last $delta
              lappend newranges [list $last $encID]
              set regID $encID
            }
            set step 3
          }
          set encID $newID
        }
        3 \
        {
          # after last
            # keep old range
          if {$newID != "" && $newID != $regID} \
          { 
            incr start $delta
            lappend newranges [list $start $newID]
            set regID $newID
          }
          incr n
        }
      }
    }
    # append items count
    lappend newranges [incr items_count $delta]
    # update ranges
    set ($name:ranges) $newranges
  }
  
# end of ::ranges namespace definition
}

