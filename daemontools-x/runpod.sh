#
# $Id: runpod.sh,v 1.8 2022-12-07 00:56:36+05:30 Cprogrammer Exp mbhangui $
#
usage()
{
  (
  echo "`basename $0` -i|--id imageid [-n|--name name]"
  echo "  [-a|--args extra_args1 -a|--args extra_args2 ... -a|--args extra_argsn]"
  echo "  [-h|--host host] [-g|--cgroup cgroup]"
  echo "  [-p|--port lport1:rport1 -p|--port lport2:rport2 ... -p|--port lportn:rportn]"
  echo "  [-v|--volume.dir:mount_point1 -v|--volume dir:mount_point2 ... -v|--volume dir:mount_pointn]"
  echo "  [-C|--capability cap1 -C|--capability cap2 ... -C|--capability cap3] | [-C|--capability auto]"
  echo "  [-c|--cmd podman|docker] prog args args ..."
  echo
  echo "lport - local port, rport - remote port"
  ) 1>&2
  exit $1
}

set_volume()
{
  word_count=$(echo $1 | cut --output-delimiter=" " -d: -f1,2,3 | wc -w)
  if [ $word_count -ne 2 ] ; then
    echo "volume should be of the form \"dir:mount_point\"" 1>&2
    exit 1
  fi
  dir=$(echo $1|cut -d: -f1)
  first_char=$(echo $dir | cut -c1)
  if [ " $first_char" = " /" ] ; then
    if [ ! -d $dir ] ; then
      echo "$dir: No such directory" 1>&2
      usage 1
    fi
  else
    $command volume inspect $dir > /dev/null
    if [ $? -ne 0 ] ; then
      (
      echo "$dir: No such volume"
      echo "List of volumes are"
      $command volume ls
      ) 1>&2
      usage 1
    fi
  fi
  dir=$(echo $1|cut -d: -f2)
  first_char=$(echo $dir | cut -c1)
  if [ " $first_char" != " /" ] ; then
    echo "$dir should be a valid path" 1>&2
    usage 1
  fi
  vol_args="$vol_args""-v $1 "
}

set_defaults()
{
  case $name in
    indimail|indimail-mta|webmail|devel|svscan|test|mail)
    if [ -z "$port_args" ] ; then
      case $name in
        indimail|indimail-mta|webmail)
        port_args="$port_args-p 2025:25   -p 2106:106  -p 2110:110  -p 2143:143 "
        port_args="$port_args-p 2209:209  -p 2366:366  -p 2465:465  -p 2587:587 "
        port_args="$port_args-p 2628:628  -p 2993:993  -p 2995:995  -p 3110:4110 "
        port_args="$port_args-p 3143:4143 -p 5110:9110 -p 5143:9143 -p 8080:80"

        # port_str is used just for display purpose
        NEWLINE=$'\n'
        port_str=""
        port_str="${port_str}  -p 2025:25   -p 2106:106  -p 2110:110  -p 2143:143${NEWLINE}"
        port_str="${port_str}  -p 2209:209  -p 2366:366  -p 2465:465  -p 2587:587${NEWLINE}"
        port_str="${port_str}  -p 2628:628  -p 2993:993  -p 2995:995  -p 3110:4110${NEWLINE}"
        port_str="${port_str}  -p 3143:4143 -p 5110:9110 -p 5143:9143 -p 8080:80"
      ;;
      esac
    fi
    if [ -z "$cap_args" ] ; then
      cap_args="$cap_args--cap-add SYS_PTRACE --cap-add SYS_ADMIN "
      cap_args="$cap_args--cap-add IPC_LOCK   --cap-add SYS_RESOURCE "
      cap_args="$cap_args--cap-add=NET_ADMIN  --cap-add=CAP_NET_RAW "
      cap_args="$cap_args--cap-add=SYS_NICE "
    else
      echo $cap_args|grep SYS_ADMIN > /dev/null
      if [ $? -ne 0 ] ; then
        cap_args="$cap_args--cap-add SYS_ADMIN "
      fi
    fi
    if [ -z "$host_arg" ] ; then
      host_arg="-h indimail.org"
    fi
    if [ -z "$vol_args" ] ; then
      if [ -d /home/$command/queue ] ; then
        vol_args="$vol_args-v /home/$command/queue:/var/queue "
      fi
      if [ -d /home/$command/mail ] ; then
        vol_args="$vol_args-v /home/$command/mail:/home/mail "
      fi
      if [ " $name" = " devel" -a  -d /usr/local/src ] ; then
        vol_args="$vol_args-v /usr/local/src:/usr/local/src"
      fi
    fi
    if [ " $name" = " devel" -o " $name" = " test" ] ; then
      if [ $# -eq 0 ] ; then
        systemd="bash"
		extra_args="-ti --rm"
      fi
    fi
    if [ " $name" = " svscan" ] ; then
        extra_args="$extra_args""-ti --rm "
    fi
    if [ -z "$extra_args" ] ; then
      detached=1
      extra_args="-d --rm"
    fi
    ;;
  esac
  if [ -n "$cgroup" -a "$cgroup" = "auto" ] ; then
    cgroup="-v /sys/fs/cgroup:/sys/fs/cgroup:rw"
  fi
}

options=$(getopt -a -n runpod -o "a:n:h:c:i:p:C:g:v:" -l args:,name:,host:,cmd:,id:,port:,capa:,cgroups:,volume: -- "$@")
if [ $? != 0 ]; then
  usage
fi

command=podman
name="test"
detached=0
port_str=""
port_args=""
host_args=""
vol_args=""
cap_args=""
extra_args=""
systemd=""
imageid=""
eval set -- "$options"
while :
do
  case "$1" in
  -a | --args)
  echo $2 | grep "\-d" >/dev/null && detached=1 
  extra_args="$extra_args""$2 "
  shift 2
  ;;

  -n | --name)
  name=$2
  shift 2
  ;;

  -h | --host)
  host_arg="-h $2"
  shift 2
  ;;

  -c | --cmd)
  command=$2
  shift 2
  if [ " $command" != " podman" -a " $command" != " docker" ] ; then
    echo "command should be podman or docker" 1>&2
    usage 1
  fi
  ;;

  -i | --id)
  imageid=$2
  shift 2
  ;;

  -p | --port)
  port_args="$port_args""-p $2 "
  shift 2
  ;;

  -C | --capability)
  if [ " $2" = " auto" ] ; then
    cap_args="$cap_args--cap-add SYS_PTRACE --cap-add SYS_ADMIN "
    cap_args="$cap_args--cap-add IPC_LOCK   --cap-add SYS_RESOURCE "
    cap_args="$cap_args--cap-add SYS_NICE "
  else
    cap_args="$cap_args--cap-add $2 "
  fi
  shift 2
  ;;

  -g | --cgroup)
  cgroups="$optarg"
  ;;

  -v | --volume)
  set_volume $2
  shift 2
  ;;

  --) # end of options
  shift
  break
  ;;

  *)
  echo "Unexpected option: $1 - this should'nt happen." 1>&2
  usage
  ;;
  esac
done
if [ -z "$imageid" ] ; then
  usage 1
fi
set_defaults
if [ $# -gt 0 -a "$1" = "auto" ] ; then
  shift
  tag=`$command images | grep  $imageid | awk '{print $2}'`
  case $tag in
    xenial*|debian*|focal*|bionic*|archlinux*|ubi*)
    systemd=/lib/systemd/systemd
    ;;
    alpine*|gentoo*)
    systemd="/sbin/init"
    ;;
    *)
    systemd=/usr/lib/systemd/systemd
    ;;
  esac
fi
if [ -n "$systemd" ] ; then
  set "$systemd $*"
fi
echo "$command run $cgroup $extra_args"
echo "  --publish-all --name $name"
echo "  --device /dev/fuse"
if [ -n "$host_arg" ] ; then
  echo "  $host_arg"
fi
if [ -n "$cap_args" ] ; then
  echo $extra_args | grep "\--privileged" >/dev/null
  if [ $? -eq 0 ] ; then
    cap_args=""
  else
    echo "  $cap_args"
  fi
fi
if [ -z "$port_args" ] ; then
  if [ -n "$port_str" ] ; then
    port_args=$port_str
  fi
fi
if [ -n "$port_args" ] ; then
  echo "  $port_args"
fi
if [ -n "$vol_args" ] ; then
  echo "  $vol_args"
fi
echo "  $imageid $*"

$command run $cgroup $extra_args \
  --publish-all --name $name \
  --device /dev/fuse \
  $host_arg $cap_args $port_args $vol_args \
  $imageid $*
if [ $? -eq 0 ] ; then
  if [ $detached -eq 1 ] ; then
    $command exec -ti $name bash
  fi
else
  echo "Removing container id $name" 1>&2
  $command rm $name > /dev/null 2>&1
fi

#
# $Log: runpod.sh,v $
# Revision 1.8  2022-12-07 00:56:36+05:30  Cprogrammer
# run in foreground if name is test or devel
#
# Revision 1.7  2022-11-10 16:55:40+05:30  Cprogrammer
# redirect all error message to fd 2
#
# Revision 1.6  2022-11-10 12:49:11+05:30  Cprogrammer
# fixed extra_args variable getting clobbered
#
# Revision 1.5  2022-11-08 23:27:52+05:30  Cprogrammer
# added case for name=svscan
#
# Revision 1.4  2022-11-07 22:43:38+05:30  Cprogrammer
# prevent output of test on stdout
#
# Revision 1.3  2022-11-01 23:53:11+05:30  Cprogrammer
# fixed port clash
#
# Revision 1.2  2022-09-19 21:21:22+05:30  Cprogrammer
# added SYS_NICE capability to defaults
#
# Revision 1.1  2022-09-12 22:50:12+05:30  Cprogrammer
# Initial revision
#
#
