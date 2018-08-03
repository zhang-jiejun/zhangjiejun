#!/usr/bin/bash

NIC=ens160
IP=`ifconfig $NIC | grep inet | grep -v "inet6" | awk '{print $2}' `
echo $IP
