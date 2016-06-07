#!/bin/bash

##########################################################################
# SecPanel.remoteconf - Client for Remote Account Manager
# Shellscript for fetching and setting remote ssh access configuration
#
# Version SecPanel 0.6.0
# Author: Steffen Leich <steffen.leich _at_ gmail.com>
##########################################################################

function do_exit {
    echo
    echo -e "Remote account management: transfer finished ($MODE)\n\nPress <Return> to continue"
    read
    exit
}

if [ -z $5 ]
then
cat <<EOF

	SecPanel
	Shellscript for fetching and setting remote ssh access configuration
	Usage: secpanel_remoteconf.sh <host> <port> <user> <scpbin> <mode> <Runfile Timestamp>

EOF
    exit 2
fi

HOST=$1
PORT=$2
USER=$3
SCPBIN=$4
MODE=$5
RFTS=$6

cat <<EOF

    SecPanel - Client for Remote Account Manager
    ------------------------------------------------------

    Connecting to $HOST as $USER


EOF


RF="$HOME/.indimail/.runfiles/ram.$RFTS"
mkdir $RF 2> /dev/null

if [ $MODE = "read" ]
    then

    $SCPBIN -P $PORT $USER@$HOST:.shosts $USER@$HOST:.ssh/authorized_keys $RF
    chmod 600 $RF/authorized_keys $RF/.shosts 2>/dev/null

elif [ $MODE = "write" ]
    then    

    $SCPBIN -P $PORT $RF/.shosts $USER@$HOST:
    $SCPBIN -P $PORT $RF/authorized_keys $USER@$HOST:.ssh

else
    echo "nothing to do..."
fi

do_exit
