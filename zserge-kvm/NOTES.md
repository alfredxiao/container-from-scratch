cc kvm-host.c -o kvm-host
./kvm-host test-bzImage

cc kvm-host-simple.c -o kvm-simple
./kvm-simple guest.bin

# guest bin is output from building guest.S
