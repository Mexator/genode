export USR="anton"
export TAP_DEV="tap0"

sudo ip tuntap add dev $TAP_DEV mode tap user $USER
sudo ip address flush dev $TAP_DEV
sudo ip address add 10.0.2.1/24 brd 10.0.2.255 dev $TAP_DEV
sudo ip link set dev $TAP_DEV addr 02:00:00:ca:fe:01
sudo ip link set dev $TAP_DEV up

