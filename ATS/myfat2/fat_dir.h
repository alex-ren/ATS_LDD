#ifndef FAT_DIR_H
#define FAT_DIR_H

#include <linux/fs.h>


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

#endif


