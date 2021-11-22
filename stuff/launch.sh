#!/bin/sh

echo "creating bridge"

ip link add br0 type bridge
ip link set br0 up
ip addr add 172.20.0.1/16 dev br0
dnsmasq --interface=br0 --bind-interfaces --dhcp-range=172.20.0.2,172.20.255.254

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
-display sdl \
-cpu core2duo \
-machine q35 \
-serial mon:stdio \
-m 512 \
-netdev tap,id=net0 \
-net nic,model=e1000,netdev=net0,macaddr=$macaddr \
-cdrom "$image"

ip link delete br0
killall dnsmasq
