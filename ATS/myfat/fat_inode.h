#ifndef ATS_INODE_H
#define ATS_INODE_H

#include <linux/exportfs.h>
#include <linux/msdos_fs.h>

#include "sb_mgr.h"

extern const struct super_operations fat_sops;
extern const struct export_operations fat_export_ops;

extern const struct inode_operations fat_file_inode_operations;

int fat_getattr(struct vfsmount *mnt, struct dentry *dentry, struct kstat *stat);
/*
 * MS-DOS file system inode data in memory
 */
struct fat_inode_info {
	spinlock_t cache_lru_lock;
	struct list_head cache_lru;
	int nr_caches;  // number of fat_cache in the cache_lru list
	/* for avoiding the race between fat_free() and fat_get_cluster() */
	unsigned int cache_valid_id;

	/* NOTE: mmu_private is 64bits, so must hold ->i_mutex to access */
	loff_t mmu_private;	/* physically allocated size */

	int i_start;		/* first cluster or 0 */
	int i_logstart;		/* logical first cluster */
	int i_attrs;		/* unused attribute bits */
	loff_t i_pos;		/* on-disk position of directory entry or 0 */
	struct hlist_node i_fat_hash;	/* hash by i_location */
	struct inode vfs_inode;  // real inode of linux VFS
};

static inline struct fat_inode_info *MSDOS_I(struct inode *inode)
{
	return container_of(inode, struct fat_inode_info, vfs_inode);
}

int fat_read_root(struct inode *inode);

static inline mode_t fat_make_mode(struct fat_sb_info *sbi,
				   u8 attrs, mode_t mode)
{
	if (attrs & ATTR_RO && !((attrs & ATTR_DIR) && !sbi->options.rodir))
		mode &= ~S_IWUGO;

	if (attrs & ATTR_DIR)
		return (mode & ~sbi->options.fs_dmask) | S_IFDIR;
	else
		return (mode & ~sbi->options.fs_fmask) | S_IFREG;
}

/*
 * If ->i_mode can't hold S_IWUGO (i.e. ATTR_RO), we use ->i_attrs to
 * save ATTR_RO instead of ->i_mode.
 *
 * If it's directory and !sbi->options.rodir, ATTR_RO isn't read-only
 * bit, it's just used as flag for app.
 */
static inline int fat_mode_can_hold_ro(struct inode *inode)
{
	struct fat_sb_info *sbi = MSDOS_SB(inode->i_sb);
	mode_t mask;

	if (S_ISDIR(inode->i_mode)) {
		if (!sbi->options.rodir)
			return 0;
		mask = ~sbi->options.fs_dmask;
	} else
		mask = ~sbi->options.fs_fmask;

	if (!(mask & S_IWUGO))
		return 0;
	return 1;
}

static inline void fat_save_attrs(struct inode *inode, u8 attrs)
{
	if (fat_mode_can_hold_ro(inode))
		MSDOS_I(inode)->i_attrs = attrs & ATTR_UNUSED;
	else
		MSDOS_I(inode)->i_attrs = attrs & (ATTR_UNUSED | ATTR_RO);
}

struct inode *fat_iget(struct super_block *sb, loff_t i_pos);

void fat_detach(struct inode *inode);

int fat_fill_inode(struct inode *inode, struct msdos_dir_entry *de);

void fat_attach(struct inode *inode, loff_t i_pos);

struct inode *fat_build_inode(struct super_block *sb,
			struct msdos_dir_entry *de, loff_t i_pos);


#endif


