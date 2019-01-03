#!/bin/bash

RELEASEFILE="https://github.com/Xcgtech/Wallet/releases/download/0.24.1/Xchangecore-0.24.1.tar.gz"
SENTINELGIT="https://github.com/cryptforall/sentinel.git" # leave empty if coin has no sentinel

instance="" # only when doing multiple instances
daemon="Xchanged"
cli="Xchange-cli"
stopcli="stop"
archive_path="Xchangecore-0.24.1/bin/"
core_dir=".Xchangecore"
config_path="$core_dir/Xchange.conf"
node_user="xcg""$instance"
mainnet="8693"
disablewallet="" # risky, a lot of coins that implement zerocoin/darksend functionality break the daemon with this

# this variable is used to keep track of the upgrades to our environment
checkpoint="20190104"
installer_checkpoint="/home/$node_user/$core_dir/.installer_checkpoint"

# other variables
DISTRO=$(lsb_release -s -c)
export DEBIAN_FRONTEND=noninteractive

# environment setup, make it pretty
tred=$(tput setaf 1); tgreen=$(tput setaf 2); tyellow=$(tput setaf 3); tblue=$(tput setaf 4); tmagenta=$(tput setaf 5); tcyan=$(tput setaf 6); treset=$(tput sgr0); tclear=$(tput clear); twbg=$(tput setab 7)

# clear the screen, my ladies
echo "$tclear"

echo -e "\n$tmagenta""Masternode UPGRADE Script for $daemon$treset\n"

# checking for current user id, we should be running as root
if (( EUID != 0 )); then
  echo -e "$tred""I need to be root to run this. Please switch to root with $twbg sudo -i $treset$tred or log in directly as root, then run the command again.$treset\n"
  exit 1
fi

if [[ $(lsb_release -s -i) != "Ubuntu" ]]; then
  echo "$tred""This doesn't seem to be Ubuntu. Distribution is unsuported.\n""$treset"
  exit 1
fi

# Checking if there's a running daemon already
BINPID=$(pidof $daemon)


if (( BINPID )); then
	echo "Found $tcyan$daemon$treset running with:"
	echo " * PID: $tyellow$BINPID$treset."
	BINPATH=$(readlink -f /proc/"$BINPID"/exe)
	echo " * Path: $tyellow$BINPATH$treset."
	BINUID=$(awk '/^Uid:/{print $2}' /proc/"$BINPID"/status)
	BINUSER=$(getent passwd "$BINUID" | awk -F: '{print $1}')
	echo " * Running as: $tyellow$BINUSER$treset."
	echo " * Running under: $tyellow$DISTRO$treset."
	echo "------------------------------"
	if [[ "$BINUSER" != "$node_user" ]]; then
		echo "The user that this $tyellow$daemon$treset is running does not match what we expect. Aborting."
		exit 1
	fi
else
	echo "We are unable to find a $tyellow$daemon$treset running. Aborting."
	exit 1
fi

if [[ -f "$installer_checkpoint" ]]; then
	# ok, a checkpoint exists, we only had one so far, so it should be before this one
	current_checkpoint=$(< $installer_checkpoint)
	if (( current_checkpoint >= checkpoint )); then
		echo -e "\nYou are at the current checkpoint. You do not need this update, you already have it.\n"
		exit 1
	fi
fi

function download_unpack_install {
  # grabbing the new release
  FILENAME=$(basename "$RELEASEFILE")
  echo "Downloading package:"
  curl -LJO -k "$RELEASEFILE" -o "$FILENAME"

  # unpacking in /usr/local/bin, we're unpacking only the daemon and cli, don't need the rest
  tar -C /usr/bin/ -xzf "$FILENAME" --strip-components 2 "$archive_path$daemon" "$archive_path$cli"
  # remove the archive, keep it clean
  rm -f "$FILENAME"

  # making sure the files have the correct permission, maybe someone cross-compiled and moved to a non-posix filesystem
  chmod +x /usr/bin/{"$daemon","$cli"}
}

function stop_services {
	echo "Stopping running services."
  systemctl disable "$daemon""$instance"-reindex
	systemctl disable "$daemon""$instance"
  systemctl stop "$daemon""$instance"-reindex
	systemctl stop "$daemon""$instance"
	sleep 3
	killall -9 $BINPID 2>/dev/null
}

function start_services {
	echo "Starting our services again."

	systemctl enable "$daemon""$instance"-reindex
	systemctl enable "$daemon""$instance"
	systemctl start "$daemon""$instance"
	
}

function update_checkpoint {
	echo $checkpoint > installer_checkpoint
}

stop_services
download_unpack_install
update_checkpoint
start_services

# set checkpoint
echo "$checkpoint" > "$installer_checkpoint"
echo -e "\n"
echo "We have updated to version$tyellow 0.24.1$treset."
echo "$tred""IF$treset you ever need to$tmagenta reindex$treset the blockchain - please use:$tyellow systemctl start $daemon-reindex$treset"
echo -e "\n"
echo "Done. Updated to checkpoint: $tyellow$checkpoint$treset".
