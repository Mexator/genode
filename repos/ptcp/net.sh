#!/bin/sh

# This will make host route packets between guests
iptables -I FORWARD -m physdev --physdev-is-bridged -j ACCEPT

net=br0

if [ ! -e /sys/class/net/$net ]; then
  echo "creating bridge"
  ip link add $net type bridge
  ip link set $net up
  ip addr add 172.20.0.1/16 dev $net
  dnsmasq --interface=$net --bind-interfaces --dhcp-range=172.20.0.50,172.20.0.254
fi