#!/bin/sh

# This will make host route packets between guests
iptables -I FORWARD -m physdev --physdev-is-bridged -j ACCEPT

net=br0
isowner=false

if [ ! -e /sys/class/net/$net ]; then
  echo "creating bridge"
  isowner=true
  ip link add $net type bridge
  ip link set $net up
  ip addr add 172.20.0.1/16 dev $net
  dnsmasq --interface=$net --bind-interfaces --dhcp-range=172.20.0.50,172.20.0.254
fi

if [ -z "$1" ]
  then
    echo "No argument supplied"
fi

image=$1

random_byte()
{
  n=`echo "$RANDOM % 256" | bc`
  printf "%02x" $n
}

macaddr=`echo 52:54:$(random_byte):$(random_byte):$(random_byte):$(random_byte)`

echo $macaddr

qemu-system-x86_64 \
-display none \
-cpu core2duo \
-machine q35 \
-serial mon:stdio \
-m 2048 \
-netdev tap,id=net0 \
-net nic,model=e1000,netdev=net0,macaddr=$macaddr \
-cdrom "$image"

if $isowner; then
  echo "removing net"
  ip link delete br0
  killall dnsmasq
fi

