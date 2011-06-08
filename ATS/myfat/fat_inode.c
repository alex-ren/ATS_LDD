#include "fat_inode.h"
#include "fat_cache.h"
#include "fat_file.h"
#include "fat_dir.h"
#include "fat_misc.h"

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

static int fat_sync_fs(struct super_block *sb, int wait)
{
	int err = 0;
        printk (KERN_INFO "myfat: fat_sync_fs\n");

	if (sb->s_dirt) {
		lock_super(sb);
		sb->s_dirt = 0;
		err = fat_clusters_flush(sb);  // todo check the code
		unlock_super(sb);
	}

	return err;
}

static void fat_clear_inode(struct inode *inode)
{
	// fat_cache_inval_inode(inode);  // don't cache yet
        printk (KERN_INFO "myfat: fat_clear_inode\n");
	fat_detach(inode);
}

static void fat_write_super(struct super_block *sb)
{
	lock_super(sb);
	sb->s_dirt = 0;

	if (!(sb->s_flags & MS_RDONLY))
		fat_clusters_flush(sb); // todo not flush yet
	unlock_super(sb);
}

static void fat_put_super(struct super_block *sb)
{
	struct fat_sb_info *sbi = MSDOS_SB(sb);
        printk (KERN_INFO "myfat: fat_put_super\n");

	lock_kernel();

	if (sb->s_dirt)
		fat_write_super(sb);

	iput(sbi->fat_inode);

	unload_nls(sbi->nls_disk);
	unload_nls(sbi->nls_io);

	if (sbi->options.iocharset != fat_default_iocharset)
		kfree(sbi->options.iocharset);

	sb->s_fs_info = NULL;
	kfree(sbi);

	unlock_kernel();
}

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

        printk (KERN_INFO "myfat: fat_read_root\n");

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

static inline unsigned long fat_hash(loff_t i_pos)
{
	return hash_32(i_pos, FAT_HASH_BITS);
}


struct inode *fat_iget(struct super_block *sb, loff_t i_pos)
{
	struct fat_sb_info *sbi = MSDOS_SB(sb);
	struct hlist_head *head = sbi->inode_hashtable + fat_hash(i_pos);
	struct hlist_node *_p;
	struct fat_inode_info *i;
	struct inode *inode = NULL;

	spin_lock(&sbi->inode_hash_lock);
	hlist_for_each_entry(i, _p, head, i_fat_hash) {
		BUG_ON(i->vfs_inode.i_sb != sb);
		if (i->i_pos != i_pos)
			continue;
		inode = igrab(&i->vfs_inode);
		if (inode)
			break;
	}
	spin_unlock(&sbi->inode_hash_lock);
	return inode;
}

void fat_detach(struct inode *inode)
{
	struct fat_sb_info *sbi = MSDOS_SB(inode->i_sb);
	spin_lock(&sbi->inode_hash_lock);
	MSDOS_I(inode)->i_pos = 0;
	hlist_del_init(&MSDOS_I(inode)->i_fat_hash);
	spin_unlock(&sbi->inode_hash_lock);
}
EXPORT_SYMBOL_GPL(fat_detach);

static int is_exec(unsigned char *extension)
{
	unsigned char *exe_extensions = "EXECOMBAT", *walk;

	for (walk = exe_extensions; *walk; walk += 3)
		if (!strncmp(extension, walk, 3))
			return 1;
	return 0;
}

/* doesn't deal with root inode */
int fat_fill_inode(struct inode *inode, struct msdos_dir_entry *de)
{
	struct fat_sb_info *sbi = MSDOS_SB(inode->i_sb);
	int error;

        printk(KERN_INFO "myfat: fat_fill_inode, inode no: %lu\n", inode->i_ino);
        printk(KERN_INFO "myfat: fat_fill_inode, inode addr: %p\n", inode);
        printk(KERN_INFO "myfat: fat_fill_inode, fat_inode addr: %p\n", MSDOS_I(inode));

	MSDOS_I(inode)->i_pos = 0;
	inode->i_uid = sbi->options.fs_uid;
	inode->i_gid = sbi->options.fs_gid;
	inode->i_version++;
	inode->i_generation = get_seconds();

	if ((de->attr & ATTR_DIR) && !IS_FREE(de->name)) {
                printk(KERN_INFO "myfat: fat_fill_inode ATTR_DIR\n");
		inode->i_generation &= ~1;
		inode->i_mode = fat_make_mode(sbi, de->attr, S_IRWXUGO);
		inode->i_op = sbi->dir_ops;
		inode->i_fop = &fat_dir_operations;

		MSDOS_I(inode)->i_start = le16_to_cpu(de->start);
		if (sbi->fat_bits == 32)
			MSDOS_I(inode)->i_start |= (le16_to_cpu(de->starthi) << 16);

                printk(KERN_INFO "myfat: fat_fill_inode i_start is %d\n", MSDOS_I(inode)->i_start);
		MSDOS_I(inode)->i_logstart = MSDOS_I(inode)->i_start;
		error = fat_calc_dir_size(inode);
		if (error < 0)
			return error;
		MSDOS_I(inode)->mmu_private = inode->i_size;

		inode->i_nlink = fat_subdirs(inode);
	} else { /* not a directory */
                printk(KERN_INFO "myfat: fat_fill_inode not dir\n");
		inode->i_generation |= 1;
		inode->i_mode = fat_make_mode(sbi, de->attr,
			((sbi->options.showexec && !is_exec(de->name + 8))
			 ? S_IRUGO|S_IWUGO : S_IRWXUGO));
		MSDOS_I(inode)->i_start = le16_to_cpu(de->start);
		if (sbi->fat_bits == 32)
			MSDOS_I(inode)->i_start |= (le16_to_cpu(de->starthi) << 16);

                printk(KERN_INFO "myfat: fat_fill_inode i_start is %d\n", MSDOS_I(inode)->i_start);
		MSDOS_I(inode)->i_logstart = MSDOS_I(inode)->i_start;
		inode->i_size = le32_to_cpu(de->size);
		inode->i_op = &fat_file_inode_operations;
		inode->i_fop = &fat_file_operations;
		inode->i_mapping->a_ops = NULL;  // &fat_aops;:we don't use it anymore.
		MSDOS_I(inode)->mmu_private = inode->i_size;
	}
	if (de->attr & ATTR_SYS) {
		if (sbi->options.sys_immutable)
			inode->i_flags |= S_IMMUTABLE;
	}
	fat_save_attrs(inode, de->attr);

	inode->i_blocks = ((inode->i_size + (sbi->cluster_size - 1))
			   & ~((loff_t)sbi->cluster_size - 1)) >> 9;

	fat_time_fat2unix(sbi, &inode->i_mtime, de->time, de->date, 0);
	if (sbi->options.isvfat) {
		fat_time_fat2unix(sbi, &inode->i_ctime, de->ctime,
				  de->cdate, de->ctime_cs);
		fat_time_fat2unix(sbi, &inode->i_atime, 0, de->adate, 0);
	} else
		inode->i_ctime = inode->i_atime = inode->i_mtime;

	return 0;
}

void fat_attach(struct inode *inode, loff_t i_pos)
{
	struct fat_sb_info *sbi = MSDOS_SB(inode->i_sb);
	struct hlist_head *head = sbi->inode_hashtable + fat_hash(i_pos);

	spin_lock(&sbi->inode_hash_lock);
	MSDOS_I(inode)->i_pos = i_pos;
	hlist_add_head(&MSDOS_I(inode)->i_fat_hash, head);
	spin_unlock(&sbi->inode_hash_lock);
}
EXPORT_SYMBOL_GPL(fat_attach);

struct inode *fat_build_inode(struct super_block *sb,
			struct msdos_dir_entry *de, loff_t i_pos)
{
	struct inode *inode;
	int err;

	inode = fat_iget(sb, i_pos);
	if (inode)
		goto out;
	inode = new_inode(sb);  // by rzq: counter is 1and the inode has been chained into sb
	if (!inode) {
		inode = ERR_PTR(-ENOMEM);
		goto out;
	}
	inode->i_ino = iunique(sb, MSDOS_ROOT_INO);
	inode->i_version = 1;
	err = fat_fill_inode(inode, de);
	if (err) {
		iput(inode);
		inode = ERR_PTR(err);
		goto out;
	}
	fat_attach(inode, i_pos);
	insert_inode_hash(inode);  // don't use ref counter
out:
	return inode;
}
EXPORT_SYMBOL_GPL(fat_build_inode);

int fat_getattr(struct vfsmount *mnt, struct dentry *dentry, struct kstat *stat)
{
	struct inode *inode = dentry->d_inode;
        printk (KERN_INFO "myfat: fat_getattr\n");
	generic_fillattr(inode, stat);
	// generic_fillattr would fill stat->blksize with the size of the block
	// but we want to fill it with the size of a cluster of the FAT
	stat->blksize = MSDOS_SB(inode->i_sb)->cluster_size;
	return 0;
}
EXPORT_SYMBOL_GPL(fat_getattr);


static inline loff_t fat_i_pos_read(struct fat_sb_info *sbi,
				    struct inode *inode)
{
	loff_t i_pos;
#if BITS_PER_LONG == 32
	spin_lock(&sbi->inode_hash_lock);
#endif
	i_pos = MSDOS_I(inode)->i_pos;
#if BITS_PER_LONG == 32
	spin_unlock(&sbi->inode_hash_lock);
#endif
	return i_pos;
}

static int fat_write_inode(struct inode *inode, int wait)
{
	struct super_block *sb = inode->i_sb;
	struct fat_sb_info *sbi = MSDOS_SB(sb);
	struct buffer_head *bh;
	struct msdos_dir_entry *raw_entry;
	loff_t i_pos;
	int err;

        printk (KERN_INFO "myfat: fat_write_inode\n");

	if (inode->i_ino == MSDOS_ROOT_INO)
		return 0;

retry:
	i_pos = fat_i_pos_read(sbi, inode);
	if (!i_pos)
		return 0;

	bh = sb_bread(sb, i_pos >> sbi->dir_per_block_bits);
	if (!bh) {
		printk(KERN_ERR "FAT: unable to read inode block "
		       "for updating (i_pos %lld)\n", i_pos);
		return -EIO;
	}
	spin_lock(&sbi->inode_hash_lock);
	if (i_pos != MSDOS_I(inode)->i_pos) {
		spin_unlock(&sbi->inode_hash_lock);
		brelse(bh);
		goto retry;
	}

	raw_entry = &((struct msdos_dir_entry *) (bh->b_data))
	    [i_pos & (sbi->dir_per_block - 1)];
	if (S_ISDIR(inode->i_mode))
		raw_entry->size = 0;
	else
		raw_entry->size = cpu_to_le32(inode->i_size);
	raw_entry->attr = fat_make_attrs(inode);
	raw_entry->start = cpu_to_le16(MSDOS_I(inode)->i_logstart);
	raw_entry->starthi = cpu_to_le16(MSDOS_I(inode)->i_logstart >> 16);
	fat_time_unix2fat(sbi, &inode->i_mtime, &raw_entry->time,
			  &raw_entry->date, NULL);
	if (sbi->options.isvfat) {
		__le16 atime;
		fat_time_unix2fat(sbi, &inode->i_ctime, &raw_entry->ctime,
				  &raw_entry->cdate, &raw_entry->ctime_cs);
		fat_time_unix2fat(sbi, &inode->i_atime, &atime,
				  &raw_entry->adate, NULL);
	}
	spin_unlock(&sbi->inode_hash_lock);
	mark_buffer_dirty(bh);
	err = 0;
	if (wait)
		err = sync_dirty_buffer(bh);
	brelse(bh);
	return err;
}

// update the inode info onto disk
int fat_sync_inode(struct inode *inode)
{
	return fat_write_inode(inode, 1);
}

EXPORT_SYMBOL_GPL(fat_sync_inode);

const struct inode_operations fat_file_inode_operations = {
	.truncate	= 0,  // fat_truncate,
	.setattr	= 0,  // fat_setattr,
	.getattr	= fat_getattr,
};

const struct super_operations fat_sops = {
	.alloc_inode	= fat_alloc_inode,
	.destroy_inode	= fat_destroy_inode,
	.write_inode	= 0,  // fat_write_inode,    // todo
	.delete_inode	= 0,  // fat_delete_inode,   // todo
	.put_super	= fat_put_super,      // todo
	.write_super	= 0,  // fat_write_super,    // todo
	.sync_fs	= fat_sync_fs,
	.statfs		= 0,  // fat_statfs,         // todo
	.clear_inode	= fat_clear_inode,    // todo
	.remount_fs	= 0,  // fat_remount,        // todo

	.show_options	= fat_show_options,
};

const struct export_operations fat_export_ops = {
	.encode_fh	= 0,  //  fat_encode_fh,  // todo
	.fh_to_dentry	= 0,  //  fat_fh_to_dentry,  // todo
	.get_parent	= 0,  //  fat_get_parent,  // todo
};

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

	err = fat_cache_init();
	if (err)
		return err;

	err = fat_init_inodecache();
	if (err)
		goto failed;

	return 0;

failed:
	fat_cache_destroy();
	return err;
}

static void __exit exit_fat_fs(void)
{
	fat_cache_destroy();
	fat_destroy_inodecache();
}

module_init(init_fat_fs)
module_exit(exit_fat_fs)

MODULE_LICENSE("GPL");

