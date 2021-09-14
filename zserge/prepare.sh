mkdir rootfs/tmp || true
mkdir rootfs/sys || true
mkdir rootfs/proc || true

sudo sysctl -w "net.ipv4.ping_group_range=0 2000000"
