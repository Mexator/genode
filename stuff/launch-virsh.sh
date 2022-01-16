#!/bin/sh

stuff/net-up.sh

if [ -z "$1" ]
  then
    echo "ERROR: ISO boot file not specified"
    exit -1
fi

export DISK_FILE=$1
firstchar=${DISK_FILE:0:1} 
if [ $firstchar != '/' ]
  then
    # Resolve relative path to absolute
    DISK_FILE=`readlink -f $DISK_FILE`
fi

export VM_DOMAIN_NAME=$2

if [ -z "$2" ]
  then
    export VM_DOMAIN_NAME=`basename $1 .iso`
    echo "Domain name not specified. Defaulting to $VM_DOMAIN_NAME"
fi

export SNAPSHOT_FILE=/tmp/genode/snapshots/$VM_DOMAIN_NAME.qcow2

mkdir -p /tmp/genode/domain-defs/
mkdir -p /tmp/genode/snapshots/
qemu-img create -f qcow2 $SNAPSHOT_FILE 1G

vm_definition=`envsubst < stuff/vm-template.xml`
echo $vm_definition > /tmp/genode/domain-defs/$VM_DOMAIN_NAME
virsh create /tmp/genode/domain-defs/$VM_DOMAIN_NAME --console

