#!/bin/bash

DEV=tun0

ip tuntap add dev $DEV mode tun user anton
ip a flush dev $DEV 
ip address add 10.0.2.1/24 brd 10.0.2.255 dev $DEV 
ip link set dev $DEV addr 02:00:00:ca:fe:01
ip link set dev $DEV up
