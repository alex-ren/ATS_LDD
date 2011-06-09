#ifndef FAT_DIR_H
#define FAT_DIR_H

#include <linux/fs.h>

// by Zhiqiang Ren
// One file name may correspond to multiple slots if the name is long [for vfat].
// For msdos, each file name only needs one slot [or entry].

// fat_slot_info describes the information about the dir entry in a directory file.
// i_pos: The address [in byte] of the slot in the data region of the volume
// slot_off: offset of this slot [entry] inside the dir file
// nr_slots: suppose this slot is the first slot for the filename, then nr_slots is 0 + 1 = 1
//     for msdos, each filename only has one slot, so the nr_slots is always 1.
// msdos_dir_entry: entry is the unit on physical volume. It corresponds to a slot.
//    msdos_dir_entry points to certain memory which is inside the buffer pointed by bh.
// bh: the buffer_head which contains the buffer for this die entry
struct fat_slot_info {
	loff_t i_pos;		/* on-disk position of directory entry */
	loff_t slot_off;	/* offset for slot or de start */
	int nr_slots;		/* number of slots + 1(de) in filename */
	struct msdos_dir_entry *de;
	struct buffer_head *bh;
};

int fat_readdir(struct file *filp, void *dirent, filldir_t filldir);

/*
 * fat_subdirs counts the number of sub-directories of dir. It can be run
	if (error)
 * on directories being created.
 */
int fat_subdirs(struct inode *dir);

/*
 * Scans a directory for a given file (name points to its formatted name).
 * Returns an error code or zero.
 */
int fat_scan(struct inode *dir, const unsigned char *name, struct fat_slot_info *sinfo);

int fat_add_entries(struct inode *dir, void *slots, int nr_slots,
			   struct fat_slot_info *sinfo);
#endif


