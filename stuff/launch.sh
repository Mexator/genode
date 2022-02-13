#!/bin/sh

stuff/net-up.sh

if [ -z "$1" ]
  then
    echo "No argument supplied"
    exit -1
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
-serial chardev:uart \
-chardev socket,id=uart,port=5555,host=localhost,server,nowait,ipv4 \
-m 2048 \
-netdev tap,id=net0 \
-net nic,model=e1000,netdev=net0,macaddr=$macaddr \
-cdrom "$image"
