#ifndef MSDOS_INODE_H
#define MSDOS_INODE_H
 
#include <linux/fs.h>

#include <linux/buffer_head.h>
#include <linux/string.h>
#include <linux/nls.h>
#include <linux/fs.h>
#include <linux/mutex.h>
#include <linux/msdos_fs.h>


struct inode *fat_build_inode(struct super_block *sb,
			struct msdos_dir_entry *de, loff_t i_pos);

extern const struct inode_operations msdos_dir_inode_operations;
 


#endif


