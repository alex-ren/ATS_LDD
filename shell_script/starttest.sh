insmod module_test/ATS/myfat/ats_fat.ko
insmod module_test/ATS/myfat/ats_msdos.ko
mount -t mymsdos /dev/loop0 ./testbed

