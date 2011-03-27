#include <linux/module.h>
#include <linux/init.h>
#include <linux/time.h>
#include <linux/slab.h>
#include <linux/smp_lock.h>
#include <linux/seq_file.h>
#include <linux/pagemap.h>
#include <linux/mpage.h>
#include <linux/buffer_head.h>
#include <linux/mount.h>
#include <linux/vfs.h>
#include <linux/parser.h>
#include <linux/uio.h>
#include <linux/writeback.h>
#include <linux/log2.h>
#include <linux/hash.h>
#include <asm/unaligned.h>

#include "fat_inode.h"
#include "fat_cache.h"
#include "fat_dir.h"


static struct kmem_cache *fat_inode_cachep;

static struct inode *fat_alloc_inode(struct super_block *sb)
{
	struct fat_inode_info *ei;
        printk (KERN_INFO "myfat: fat_alloc_inode\n");
	ei = kmem_cache_alloc(fat_inode_cachep, GFP_NOFS);
	if (!ei)
		return NULL;
	return &ei->vfs_inode;
}

static void fat_destroy_inode(struct inode *inode)
{
        printk (KERN_INFO "myfat: fat_destroy_inode\n");
	kmem_cache_free(fat_inode_cachep, MSDOS_I(inode));
}

// by rzq: for supporting /proc file system
static int fat_show_options(struct seq_file *m, struct vfsmount *mnt)
{
	struct fat_sb_info *sbi = MSDOS_SB(mnt->mnt_sb);
	struct fat_mount_options *opts = &sbi->options;
	int isvfat = opts->isvfat;

        printk (KERN_INFO "myfat: fat_show_options\n");

	if (opts->fs_uid != 0)
		seq_printf(m, ",uid=%u", opts->fs_uid);
	if (opts->fs_gid != 0)
		seq_printf(m, ",gid=%u", opts->fs_gid);
	seq_printf(m, ",fmask=%04o", opts->fs_fmask);
	seq_printf(m, ",dmask=%04o", opts->fs_dmask);
	if (opts->allow_utime)
		seq_printf(m, ",allow_utime=%04o", opts->allow_utime);
	if (sbi->nls_disk)
		seq_printf(m, ",codepage=%s", sbi->nls_disk->charset);
	if (isvfat) {
		if (sbi->nls_io)
			seq_printf(m, ",iocharset=%s", sbi->nls_io->charset);

		switch (opts->shortname) {
		case VFAT_SFN_DISPLAY_WIN95 | VFAT_SFN_CREATE_WIN95:
			seq_puts(m, ",shortname=win95");
			break;
		case VFAT_SFN_DISPLAY_WINNT | VFAT_SFN_CREATE_WINNT:
			seq_puts(m, ",shortname=winnt");
			break;
		case VFAT_SFN_DISPLAY_WINNT | VFAT_SFN_CREATE_WIN95:
			seq_puts(m, ",shortname=mixed");
			break;
		case VFAT_SFN_DISPLAY_LOWER | VFAT_SFN_CREATE_WIN95:
			seq_puts(m, ",shortname=lower");
			break;
		default:
			seq_puts(m, ",shortname=unknown");
			break;
		}
	}
	if (opts->name_check != 'n')
		seq_printf(m, ",check=%c", opts->name_check);
	if (opts->usefree)
		seq_puts(m, ",usefree");
	if (opts->quiet)
		seq_puts(m, ",quiet");
	if (opts->showexec)
		seq_puts(m, ",showexec");
	if (opts->sys_immutable)
		seq_puts(m, ",sys_immutable");
	if (!isvfat) {
		if (opts->dotsOK)
			seq_puts(m, ",dotsOK=yes");
		if (opts->nocase)
			seq_puts(m, ",nocase");
	} else {
		if (opts->utf8)
			seq_puts(m, ",utf8");
		if (opts->unicode_xlate)
			seq_puts(m, ",uni_xlate");
		if (!opts->numtail)
			seq_puts(m, ",nonumtail");
		if (opts->rodir)
			seq_puts(m, ",rodir");
	}
	if (opts->flush)
		seq_puts(m, ",flush");
	if (opts->tz_utc)
		seq_puts(m, ",tz=UTC");
	if (opts->errors == FAT_ERRORS_CONT)
		seq_puts(m, ",errors=continue");
	else if (opts->errors == FAT_ERRORS_PANIC)
		seq_puts(m, ",errors=panic");
	else
		seq_puts(m, ",errors=remount-ro");

	return 0;
}

const struct super_operations fat_sops = {
	.alloc_inode	= fat_alloc_inode,
	.destroy_inode	= fat_destroy_inode,
	.write_inode	= 0,  // fat_write_inode,    // todo
	.delete_inode	= 0,  // fat_delete_inode,   // todo
	.put_super	= 0,  // fat_put_super,      // todo
	.write_super	= 0,  // fat_write_super,    // todo
	.sync_fs	= 0,  // fat_sync_fs,        // todo
	.statfs		= 0,  // fat_statfs,         // todo
	.clear_inode	= 0,  // fat_clear_inode,    // todo
	.remount_fs	= 0,  // fat_remount,        // todo

	.show_options	= fat_show_options,
};

const struct export_operations fat_export_ops = {
	.encode_fh	= 0,  //  fat_encode_fh,  // todo
	.fh_to_dentry	= 0,  //  fat_fh_to_dentry,  // todo
	.get_parent	= 0,  //  fat_get_parent,  // todo
};

/*
* Desc: Return the size of a directory file in bytes
*
*/
static int fat_calc_dir_size(struct inode *inode)
{
	struct fat_sb_info *sbi = MSDOS_SB(inode->i_sb);
	int ret, fclus, dclus;

	inode->i_size = 0;
	if (MSDOS_I(inode)->i_start == 0)
		return 0;

	ret = fat_get_cluster(inode, FAT_ENT_EOF, &fclus, &dclus);
	if (ret < 0)
		return ret;
	inode->i_size = (fclus + 1) << sbi->cluster_bits;

	return 0;
}

// root the info for root directory from disk and store it in inode
// Return Value: -1 : error
int fat_read_root(struct inode *inode)
{
	struct super_block *sb = inode->i_sb;
	struct fat_sb_info *sbi = MSDOS_SB(sb);
	int error = 0;

	MSDOS_I(inode)->i_pos = 0;
	inode->i_uid = sbi->options.fs_uid;
	inode->i_gid = sbi->options.fs_gid;
	inode->i_version++;
	inode->i_generation = 0;
	inode->i_mode = fat_make_mode(sbi, ATTR_DIR, S_IRWXUGO);
	inode->i_op = sbi->dir_ops;
	inode->i_fop = &fat_dir_operations;
	if (sbi->fat_bits == 32) {
		MSDOS_I(inode)->i_start = sbi->root_cluster;
                // the size of the file for Root Dir
		error = fat_calc_dir_size(inode);
		if (error < 0)
			return error;
	} else {
		MSDOS_I(inode)->i_start = 0;
                // the size of the file for Root Dir
		inode->i_size = sbi->dir_entries * sizeof(struct msdos_dir_entry);
	}
	inode->i_blocks = ((inode->i_size + (sbi->cluster_size - 1))
			   & ~((loff_t)sbi->cluster_size - 1)) >> 9;  // why 9?
                                                // what if cluster size isn't 512?  todo
	MSDOS_I(inode)->i_logstart = 0;
	MSDOS_I(inode)->mmu_private = inode->i_size;  // rzq: I don't use this now
                                                    // no idea what it is

	fat_save_attrs(inode, ATTR_DIR);  // the is a directory
	inode->i_mtime.tv_sec = inode->i_atime.tv_sec = inode->i_ctime.tv_sec = 0;
	inode->i_mtime.tv_nsec = inode->i_atime.tv_nsec = inode->i_ctime.tv_nsec = 0;
	inode->i_nlink = fat_subdirs(inode)+2;  // count in . and ..

	return 0;
}

static void init_once(void *foo)
{
	struct fat_inode_info *ei = (struct fat_inode_info *)foo;

	spin_lock_init(&ei->cache_lru_lock);
	ei->nr_caches = 0;
	ei->cache_valid_id = FAT_CACHE_VALID + 1;
	INIT_LIST_HEAD(&ei->cache_lru);
	INIT_HLIST_NODE(&ei->i_fat_hash);
	inode_init_once(&ei->vfs_inode);
}

static int __init fat_init_inodecache(void)
{
	fat_inode_cachep = kmem_cache_create("fat_inode_cache",
					     sizeof(struct fat_inode_info),
					     0, (SLAB_RECLAIM_ACCOUNT|
						SLAB_MEM_SPREAD),
					     init_once);
	if (fat_inode_cachep == NULL)
		return -ENOMEM;
	return 0;
}

static void __exit fat_destroy_inodecache(void)
{
	kmem_cache_destroy(fat_inode_cachep);
}

static int __init init_fat_fs(void)
{
	int err;

	// err = fat_cache_init();  // todo
	// if (err)
	// 	return err;

	err = fat_init_inodecache();
	if (err)
		goto failed;

	return 0;

failed:
	// fat_cache_destroy();
	return err;
}

static void __exit exit_fat_fs(void)
{
	// fat_cache_destroy();  // todo
	fat_destroy_inodecache();
}

module_init(init_fat_fs)
module_exit(exit_fat_fs)

MODULE_LICENSE("GPL");

