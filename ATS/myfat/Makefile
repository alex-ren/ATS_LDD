#
# Makefile for the Linux fat filesystem support.
#
######

ATSCC=$(ATSHOME)/bin/atscc
ATSOPT=$(ATSHOME)/bin/atsopt

######

KVERS = $(shell uname -r)

######
# Comment/uncomment the following line to disable/enable debugging
#DEBUG = y

######
# Add your debugging flag (or not) to CFLAGS
ifeq ($(DEBUG),y)
  DEBFLAGS = -O -g -DATS_FAT_DEBUG # "-O" is needed to expand inlines
else
  DEBFLAGS = -O2
endif

######
#
# Specify flags for the module compilation.
#
EXTRA_CFLAGS += $(DEBFLAGS)
EXTRA_CFLAGS += -I$(LDDINC)
EXTRA_CFLAGS += -I$(ATSHOME)
EXTRA_CFLAGS += -D_ATS_HEADER_NONE -D_ATS_PRELUDE_NONE

######

ifneq ($(KERNELRELEASE),)
# call from kernel build system
# Kernel modules
obj-m += ats_fat.o
# obj-m += ats_vfat.o
obj-m += ats_msdos.o

# ats_fat-objs := cache.o dir.o fatent.o file.o inode.o misc.o ats_sb_mgr.o
ats_fat-objs := sb_mgr.o ats_fat_inode_dats.o fat_inode.o fat_entry.o \
               fat_dir.o fat_cache.o fat_misc.o
# ats_vfat-objs := namei_vfat.o
ats_msdos-objs := namei_msdos.o msdos_inode.o msdos_dentry.o msdos_misc.o
else

KERNELDIR ?= /lib/modules/$(KVERS)/build
PWD       := $(shell pwd)

modules:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) LDDINC=$(PWD)/../include modules

endif

######


# ats_fat: ats_inode_dats.c
# 
ats_inode_dats.o: ats_fat_inode_dats.c

.PHONY: ats

ats: ats_fat_inode_dats.c

ats_fat_inode_dats.c: ATS/ats_fat_inode.dats ATS/ats_fat_inode.sats
	$(ATSOPT) -IATS ATS -o $@ -d $<
# 

######

.PHONY: clean
clean:
	rm -f *~
	rm -f *_?ats.c *_?ats.o
	make -C /lib/modules/$(KVERS)/build M=$(PWD) clean


depend .depend dep:
	$(CC) $(EXTRA_CFLAGS) -M *.c > .depend


ifeq (.depend,$(wildcard .depend))
include .depend
endif
###### end of [Makefile] ######


