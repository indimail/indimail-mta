##################################################################
# SecPanel
#
# Exporting profiles related code
##################################################################

proc export {mode form} {
    global sites env widget configs

    # source def-files
    if {$mode == "def"} {
	if {[$widget(defsites) index end] > 0} {
	    source "$env(HOME)/.indimail/profiles/default.profile"
	    set actconn [$widget(defsites) get active]
	    set host [lindex [split $sites($actconn) #] 0]
	    
	    set userentry [lindex [split $sites($actconn) #] 1]
	    
	    if {$userentry == "<ASKFORUSER>"} {
		set user [askforuser]
		if {$user == "#####"} {
		    return
		}
	    } else {
		set user $userentry
	    }
	    
	    set title $actconn
	} else {
	    showmessage "No conncections available, please use \"New\"" ""
	    return
	}
    }
    
    # source spec-files
    if {$mode == "spec"} {
	if {[$widget(specsites) index end] > 0} {
	    set actconn [retprof [$widget(specsites) get active]]
	    source "$env(HOME)/.indimail/profiles/$actconn.profile"
	} else {
	    showmessage "No conncections available, please use \"New\"" ""
	    return
	}
    }

    # forwardings
    if {[array size lfs] > 0} {
	foreach lf [array names lfs] {
	    if {[regsub {<TARGET-HOST>} [lindex [split $lf :] 1] $host th]} {
		append lf_tag  " -L [lindex [split $lf :] 0]:$th:[lindex [split $lf :] 2] "
	    } else {
		append lf_tag  " -L [lindex [split $lf :] 0]:[lindex [split $lf :] 1]:[lindex [split $lf :] 2] "
	    }
	}
    } else {
	set lf_tag " "
    }

    set localhost [info hostname]
    if {[array size rfs] > 0} {
	foreach rf [array names rfs] {
	    if {[regsub {<LOCAL-HOST>} [lindex [split $rf :] 1] $localhost lh]} {
		append rf_tag  " -R [lindex [split $rf :] 0]:$lh:[lindex [split $rf :] 2] "
	    } else {
		append rf_tag  " -R [lindex [split $rf :] 0]:[lindex [split $rf :] 1]:[lindex [split $rf :] 2] "
	    }
	}
    } else {
	set rf_tag " "
    }

    if {$user != ""} {
	set user_tag "-l $user "
    } else {
	set user_tag " "
    }
    
    if {$port == 22 || $port == ""} {
	set port_tag " "
    } else {
	set port_tag "-p $port "
    }
    
    if {$algo != "default" || $algo == ""} {
	set algo_tag "-c $algo "
    } else {
	set algo_tag " "
    }
    
    if {$identity != ""} {
	set ident_tag "-i $identity "
    } else {
	set ident_tag " "
    }

    if {$command != ""} {
	set command_tag "$command"
    } else {
	set command_tag ""
    }
    
    if $compress {
	# openssh
	if {$configs(sshver) == "OpenSSH"} {
	    set compressval_tag "-o \'CompressionLevel [set compressval]\' "
	} else {
	    set compressval_tag "-o CompressionLevel=$compressval "
	}
    } else {
	set compressval_tag " "
    }
    
    array set bools {
	"agentforward" "-a" \
		"x11forward" "-x" \
		"nopriv" "-P"  "verbose" "-v" \
		"quiet" "-q" \
		"fork" "-f" \
		"gateway" "-g" \
		"compress" "-C"
    }
    
    # foreach f [array names $bools]
    foreach f {agentforward x11forward nopriv verbose \
	    quiet fork gateway compress} {
	if [set $f] {
	    set [set f]_tag "$bools($f) "
	} else {
	    set [set f]_tag " "
	}
    }

    # foreach f [array names $bools]
    foreach f {agentforward x11forward nopriv verbose \
	    quiet fork gateway compress} {
	if [set $f] {
	    set [set f]_tagssh "yes"
	} else {
	    set [set f]_tagssh "no"
	}
    }

    set sshvertag "-[set sshverconnect] "
    set ipvertag "-[set ipverconnect] "

    # openssh
    if {! $x11forward} {
	if {$configs(sshver) == "OpenSSH"} {
	    set x11forward_tag "-X "
	}
    }

    set connstring "$configs(sshbin) $user_tag \
	    $agentforward_tag $x11forward_tag $sshvertag $ipvertag $port_tag $algo_tag \
	    $ident_tag $nopriv_tag $verbose_tag $quiet_tag \
	    $fork_tag $gateway_tag $compress_tag $compressval_tag \
	    $lf_tag $rf_tag $host $command_tag"


    switch -exact $form {
	"ssh" {
	    puts "Host $host"
	    puts "Compression $compress_tagssh"
	    puts "CompressionLevel $compressval_tag"
	    puts "ForwardAgent $agentforward_tagssh"
	    puts "ForwardX11 $x11forward_tagssh"
	    puts "GatewayPorts $gateway_tagssh"
	    puts "HostName $host"
	    puts "IdentityFile $ident_tag"
	    puts "LocalForward"
	    puts "Port $port_tag"
	    puts "Protocol $sshvertag"
	    puts "RemoteForward"
	    puts "StrictHostKeyChecking"
	    puts "User $user_tag"
	}
	"sh" {
	    puts "#!/bin/sh"
	    puts "#\n#Exported by SecPanel\n#"
	    puts "$connstring"
	}
    }
}
