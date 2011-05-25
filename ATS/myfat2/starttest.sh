insmod ./ats_fat.ko
insmod ./ats_msdos.ko
mount -t mymsdos /dev/loop0 ./testbed

