brctl addbr br0
ip addr add dev br0 172.16.0.100/24
ip link set br0 up
IFNAME=enp0s3
sudo iptables -A FORWARD -i $IFNAME -o br0 -j ACCEPT
sudo iptables -A FORWARD -o $IFNAME -i br0 -j ACCEPT
sudo iptables -t nat -A POSTROUTING -s 172.16.0.0/16 -j MASQUERADE
