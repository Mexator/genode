#!/bin/bash

ip tuntap add dev tap0 mode tap user anton
ip a flush dev tap0
ip address add 10.0.2.1/24 brd 10.0.2.255 dev tap0
ip link set dev tap0 addr 02:00:00:ca:fe:01
ip link set dev tap0 up
