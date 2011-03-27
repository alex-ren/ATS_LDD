#include "fat_dir.h"
#include "fat.h"
#include "sb_mgr.h"
#include "fat_cache.h"

#include <linux/msdos_fs.h>

const struct file_operations fat_dir_operations = {
	.llseek		= 0,  // todo my_generic_file_llseek,
	.read		= 0,  // todo my_generic_read_dir,
	.readdir	= fat_readdir,
	.ioctl		= 0,  // todo fat_dir_ioctl,
#ifdef CONFIG_COMPAT
	.compat_ioctl	= 0,  // todo fat_compat_dir_ioctl,
#endif
	.fsync		= 0,  // todo fat_file_fsync,
};

/* Returns the inode number of the directory entry at offset pos. If bh is
   non-NULL, it is brelse'd before. Pos is incremented. The buffer header is
   returned in bh.
   AV. Most often we do it item-by-item. Makes sense to optimize.
   AV. OK, there we go: if both bh and de are non-NULL we assume that we just
   AV. want the next entry (took one explicit de=NULL in vfat/namei.c).
   AV. It's done in fat_get_entry() (inlined), here the slow case lives.
   AV. Additionally, when we return -1 (i.e. reached the end of directory)
   AV. we make bh NULL.
 */
// todo into ATS
/*
* Desc: make *de to point to the entry desiganated by *pos, then increase *pos
*
*
*/
static int fat__get_entry(struct inode *dir, loff_t *pos,
			  struct buffer_head **bh, struct msdos_dir_entry **de)
{
	struct super_block *sb = dir->i_sb;
	sector_t phys, iblock;
	unsigned long mapped_blocks;  // of no use now
	int err, offset;

next:
	if (*bh)
		brelse(*bh);

	*bh = NULL;
	iblock = *pos >> sb->s_blocksize_bits;
	err = fat_bmap(dir, iblock, &phys, &mapped_blocks, 0);  // todo
	if (err || !phys)
		return -1;	/* beyond EOF or error */

	// fat_dir_readahead(dir, iblock, phys);  // no read ahead now

	*bh = sb_bread(sb, phys);
	if (*bh == NULL) {
		printk(KERN_ERR "FAT: Directory bread(block %llu) failed\n",
		       (llu)phys);
		/* skip this block */
		*pos = (iblock + 1) << sb->s_blocksize_bits;
		goto next;
	}

	offset = *pos & (sb->s_blocksize - 1);
	*pos += sizeof(struct msdos_dir_entry);
	*de = (struct msdos_dir_entry *)((*bh)->b_data + offset);

	return 0;
}

/*
* Desc: 
* Para:
*   InOut:
*   pos: the offset (in byte) of the entry in the whole dir file
* Info:
*   This function may alloc memory and put it into bh
*/
static inline int fat_get_entry(struct inode *dir, loff_t *pos,
				struct buffer_head **bh,
				struct msdos_dir_entry **de)
{
    /* Fast stuff first */
    /* entry is in the block held by *bh */
    if (*bh && *de &&
      (*de - (struct msdos_dir_entry *)(*bh)->b_data) < 
        MSDOS_SB(dir->i_sb)->dir_per_block - 1) 
    {
        *pos += sizeof(struct msdos_dir_entry);
        (*de)++;
        return 0;
    }
    return fat__get_entry(dir, pos, bh, de);
}

static int fat_get_short_entry(struct inode *dir, loff_t *pos,
			       struct buffer_head **bh,
			       struct msdos_dir_entry **de)
{
	while (fat_get_entry(dir, pos, bh, de) >= 0) {
		/* free entry or long name entry or volume label */
		if (!IS_FREE((*de)->name) && !((*de)->attr & ATTR_VOLUME))
			return 0;
	}
	return -ENOENT;
}

/*
 * fat_subdirs counts the number of sub-directories of dir. It can be run
 * on directories being created.
 */
int fat_subdirs(struct inode *dir)
{
	struct buffer_head *bh = NULL;
	struct msdos_dir_entry *de = NULL;
	loff_t cpos = 0;
	int count = 0;

	while (fat_get_short_entry(dir, &cpos, &bh, &de) >= 0) {
		if (de->attr & ATTR_DIR)
			count++;
	}
	brelse(bh);
	return count;
}

