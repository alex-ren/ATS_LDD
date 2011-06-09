/*
 *  linux/fs/msdos/namei.c
 *
 *  Written 1992,1993 by Werner Almesberger
 *  Hidden files 1995 by Albert Cahalan <albert@ccs.neu.edu> <adc@coe.neu.edu>
 *  Rewritten for constant inumbers 1999 by Al Viro
 */

#include <linux/module.h>
#include <linux/time.h>
#include <linux/buffer_head.h>

// #include "fat.h"
#include "sb_mgr.h"
#include "msdos_inode.h"
#include "msdos_dentry.h"


// sb points to super_block already allocated
// data: no idea of what it is but didn't use it at all
// silent: flag => whether to output msg when error occurrs
static int msdos_fill_super(struct super_block *sb, void *data, int silent)
{
	int res;
        printk (KERN_INFO "myfat: msdos_fill_super\n");

        // fill most part of the super block
	res = fat_fill_super(sb, data, silent, &msdos_dir_inode_operations, 0);
	if (res)
		return res;

	sb->s_flags |= MS_NOATIME;
	sb->s_root->d_op = &msdos_dentry_operations;
	return 0;
}

// read info for the super block from the disk
// create inode for the root
// data: options passed in by kernel, probably be NULL (no options at all)
static int msdos_get_sb(struct file_system_type *fs_type,
			int flags, const char *dev_name,
			void *data, struct vfsmount *mnt)
{
        printk (KERN_INFO "fat: msdos_get_sb, dev is %s\n", dev_name);
        // get_sb_bdev is linux general function
	return get_sb_bdev(fs_type, flags, dev_name, data, msdos_fill_super,
			   mnt);
}

static struct file_system_type msdos_fs_type = {
	.owner		= THIS_MODULE,
	.name		= "mymsdos",
	.get_sb		= msdos_get_sb,
	.kill_sb	= kill_block_super,
	.fs_flags	= FS_REQUIRES_DEV,
};

static int __init init_msdos_fs(void)
{
        printk (KERN_INFO "myfat: init_msdos_fs\n");
	return register_filesystem(&msdos_fs_type);
}

static void __exit exit_msdos_fs(void)
{
        printk (KERN_INFO "myfat: exit_msdos_fs\n");
	unregister_filesystem(&msdos_fs_type);
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Werner Almesberger");
MODULE_DESCRIPTION("MS-DOS filesystem support");

module_init(init_msdos_fs)
module_exit(exit_msdos_fs)


