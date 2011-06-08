#include "fat.h"
#include "sb_mgr.h"
#include "fat_entry.h"
#include "fat_misc.h"
#include "fat_cache.h"

#include <linux/blkdev.h>

// struct fatent_operations {
// 	void (*ent_blocknr)(struct super_block *, int, int *, sector_t *);
// 	void (*ent_set_ptr)(struct fat_entry *, int);
// 	int (*ent_bread)(struct super_block *, struct fat_entry *,
// 			 int, sector_t);
// 	int (*ent_get)(struct fat_entry *);
// 	void (*ent_put)(struct fat_entry *, int);
// 	int (*ent_next)(struct fat_entry *);
// };

void fatent_init(struct fat_entry *fatent)
{
	fatent->nr_bhs = 0;
	fatent->entry = 0;
	fatent->u.ent32_p = NULL;
	fatent->bhs[0] = fatent->bhs[1] = NULL;
	fatent->fat_inode = NULL;
}

void fatent_set_entry(struct fat_entry *fatent, int entry)
{
	fatent->entry = entry;
	fatent->u.ent32_p = NULL;
}

void fatent_brelse(struct fat_entry *fatent)
{
	int i;
	fatent->u.ent32_p = NULL;
	for (i = 0; i < fatent->nr_bhs; i++)
		brelse(fatent->bhs[i]);
	fatent->nr_bhs = 0;
	fatent->bhs[0] = fatent->bhs[1] = NULL;
	fatent->fat_inode = NULL;
}


/*
* Desc: find the block no. and offset for an entry in FAT
* Para:
* In:
*   entry: the no. of entry in FAT
* Out:
*   offset: offset in a block (in byte)
*   blocknr: block num
*/
void fat_ent_blocknr(struct super_block *sb, int entry,
			    int *offset, sector_t *blocknr)
{
    struct fat_sb_info *sbi = MSDOS_SB(sb);
    int bytes = (entry << sbi->fatent_shift);
    WARN_ON(entry < FAT_START_ENT || sbi->max_cluster <= entry);
    *offset = bytes & (sb->s_blocksize - 1);
    *blocknr = sbi->fat_start + (bytes >> sb->s_blocksize_bits);
}

void fat32_ent_set_ptr(struct fat_entry *fatent, int offset)
{
    WARN_ON(offset & (4 - 1));
    fatent->u.ent32_p = (__le32 *)(fatent->bhs[0]->b_data + offset);
}

/*
* Desc: read (a block) from volume, the block contains certain entry of FAT
* Para:
* In:
*   offset: offset in the block
*   blocknr: no. of block in the volume
* InOut:
*   fatent: a newly created fat_entry
*
*/
int fat_ent_bread(struct super_block *sb, struct fat_entry *fatent,
			 int offset, sector_t blocknr)
{
    WARN_ON(blocknr < MSDOS_SB(sb)->fat_start);
    fatent->fat_inode = MSDOS_SB(sb)->fat_inode;  // for what purpose?
    fatent->bhs[0] = sb_bread(sb, blocknr);
    if (!fatent->bhs[0]) {
    	printk(KERN_ERR "FAT: FAT read failed (blocknr %llu)\n",
    	       (llu)blocknr);
    	return -EIO;
    }
    fatent->nr_bhs = 1;
    fat32_ent_set_ptr(fatent, offset);
    return 0;
}

/*
* Desc: Get info of the current entry pointed by the fatent
* Ret: no. of clustr
*/
int fat32_ent_get(struct fat_entry *fatent)
{
    int next = le32_to_cpu(*fatent->u.ent32_p) & 0x0fffffff;
    WARN_ON((unsigned long)fatent->u.ent32_p & (4 - 1));
    if (next >= BAD_FAT32)  // No one else shall see BAD_FAT32
    	next = FAT_ENT_EOF;
    return next;
}

/*
* Desc: increase the no. of entry in the fat_entry
*/
int fat32_ent_next(struct fat_entry *fatent)
{
    const struct buffer_head *bh = fatent->bhs[0];
    fatent->entry++;
    if (fatent->u.ent32_p < (__le32 *)(bh->b_data + (bh->b_size - 4))) {
    	fatent->u.ent32_p++;
    	return 1;
    }
    fatent->u.ent32_p = NULL;
    return 0;
}

void fat32_ent_put(struct fat_entry *fatent, int new)
{
	if (new == FAT_ENT_EOF)
		new = EOF_FAT32;

	WARN_ON(new & 0xf0000000);
	new |= le32_to_cpu(*fatent->u.ent32_p) & ~0x0fffffff;
	*fatent->u.ent32_p = cpu_to_le32(new);
	mark_buffer_dirty_inode(fatent->bhs[0], fatent->fat_inode);
}

// static struct fatent_operations fat12_ops = {
// 	.ent_blocknr	= 0,  // fat12_ent_blocknr,
// 	.ent_set_ptr	= 0,  // fat12_ent_set_ptr,
// 	.ent_bread	= 0,  // fat12_ent_bread,
// 	.ent_get	= 0,  // fat12_ent_get,
// 	.ent_put	= 0,  // fat12_ent_put,
// 	.ent_next	= 0,  // fat12_ent_next,
// };
// 
// static struct fatent_operations fat16_ops = {
// 	.ent_blocknr	= 0,  // fat_ent_blocknr,
// 	.ent_set_ptr	= 0,  // fat16_ent_set_ptr,
// 	.ent_bread	= 0,  // fat_ent_bread,
// 	.ent_get	= 0,  // fat16_ent_get,
// 	.ent_put	= 0,  // fat16_ent_put,
// 	.ent_next	= 0,  // fat16_ent_next,
// };
// 
// static struct fatent_operations fat32_ops = {
// 	.ent_blocknr	= fat_ent_blocknr,
// 	.ent_set_ptr	= fat32_ent_set_ptr,
// 	.ent_bread	= fat_ent_bread,
// 	.ent_get	= fat32_ent_get,
// 	.ent_put	= fat32_ent_put,
// 	.ent_next	= fat32_ent_next,
// };
// 

static inline void lock_fat(struct fat_sb_info *sbi)
{
	mutex_lock(&sbi->fat_lock);
}

static inline void unlock_fat(struct fat_sb_info *sbi)
{
	mutex_unlock(&sbi->fat_lock);
}

void fat_ent_access_init(struct super_block *sb)
{
	struct fat_sb_info *sbi = MSDOS_SB(sb);
        printk (KERN_INFO "myfat: fat_ent_access_init\n");

	mutex_init(&sbi->fat_lock);

	switch (sbi->fat_bits) {
	case 32:
		sbi->fatent_shift = 2;
		// sbi->fatent_ops = 0;  // &fat32_ops;  // now never use fatent_ops
		break;
	case 16:  // this will not happen
		sbi->fatent_shift = 1;
		// sbi->fatent_ops = 0;  // &fat16_ops;
		break;
	case 12: // this will not happen
		sbi->fatent_shift = -1;
		// sbi->fatent_ops = 0;  // &fat12_ops;
		break;
	}
}

/*
* Desc: get the no. of the next entry in the link list of FAT
* Info: can return FAT_ENT_EOF and FAT_ENT_FREE
* todo into ATS, the special kind of return value
*
*/
int fatent_next_sb(struct super_block *sb, int curent, int *nxtent)
{
    struct fat_entry ent;

    int offset = 0;
    sector_t blocknr = 0;

    int ret = 0;

    fatent_init (&ent);
    fatent_set_entry(&ent, curent);  // set the no. of entry
    fat_ent_blocknr(sb, curent, &offset, &blocknr);  // get the block no. and offset of dclus
    printk (KERN_INFO "myfat: fatent_next_sb offset is %d, blocknr is %lld\n", offset, blocknr);
    
    if (fat_ent_bread(sb, &ent, offset, blocknr))
    {
        ret = -EIO;
        goto out;
    }

    *nxtent = fat32_ent_get(&ent);

out:
    fatent_brelse(&ent);
    return ret;
}

/*
* Desc: get the no. of the next entry in the link list of FAT
* Info: can return FAT_ENT_EOF and FAT_ENT_FREE
* todo into ATS, the special kind of return value
*
*/
int fatent_next(struct inode *inode, int curent, int *nxtent)
{
    struct super_block *sb = inode->i_sb;
    return fatent_next_sb(sb, curent, nxtent);
}


static inline int fat_ent_update_ptr(struct super_block *sb,
				     struct fat_entry *fatent,
				     int offset, sector_t blocknr)
{
	struct fat_sb_info *sbi = MSDOS_SB(sb);
	// struct fatent_operations *ops = sbi->fatent_ops;
	struct buffer_head **bhs = fatent->bhs;

	/* Is this fatent's blocks including this entry? */
	if (!fatent->nr_bhs || bhs[0]->b_blocknr != blocknr)
		return 0;
	if (sbi->fat_bits == 12) {
		if ((offset + 1) < sb->s_blocksize) {
			/* This entry is on bhs[0]. */
			if (fatent->nr_bhs == 2) {
				brelse(bhs[1]);
				fatent->nr_bhs = 1;
			}
		} else {
			/* This entry needs the next block. */
			if (fatent->nr_bhs != 2)
				return 0;
			if (bhs[1]->b_blocknr != (blocknr + 1))
				return 0;
		}
	}
	// ops->ent_set_ptr(fatent, offset);
	fat32_ent_set_ptr(fatent, offset);
	return 1;
}


int fat_ent_read(struct inode *inode, struct fat_entry *fatent, int entry)
{
	struct super_block *sb = inode->i_sb;
	struct fat_sb_info *sbi = MSDOS_SB(inode->i_sb);
	// struct fatent_operations *ops = sbi->fatent_ops;
	int err, offset;
	sector_t blocknr;

	if (entry < FAT_START_ENT || sbi->max_cluster <= entry) {
		fatent_brelse(fatent);
		fat_fs_error(sb, "invalid access to FAT (entry 0x%08x)", entry);
		return -EIO;
	}

	fatent_set_entry(fatent, entry);
	// ops->ent_blocknr(sb, entry, &offset, &blocknr);
	fat_ent_blocknr(sb, entry, &offset, &blocknr);

	if (!fat_ent_update_ptr(sb, fatent, offset, blocknr)) {
		fatent_brelse(fatent);
		// err = ops->ent_bread(sb, fatent, offset, blocknr);
		err = fat_ent_bread(sb, fatent, offset, blocknr);
		if (err)
			return err;
	}
	// return ops->ent_get(fatent);
	return fat32_ent_get(fatent);
}

/* FIXME: We can write the blocks as more big chunk. */
static int fat_mirror_bhs(struct super_block *sb, struct buffer_head **bhs,
			  int nr_bhs)
{
	struct fat_sb_info *sbi = MSDOS_SB(sb);
	struct buffer_head *c_bh;
	int err, n, copy;

	err = 0;
	for (copy = 1; copy < sbi->fats; copy++) {
		sector_t backup_fat = sbi->fat_length * copy;

		for (n = 0; n < nr_bhs; n++) {
			c_bh = sb_getblk(sb, backup_fat + bhs[n]->b_blocknr);
			if (!c_bh) {
				err = -ENOMEM;
				goto error;
			}
			memcpy(c_bh->b_data, bhs[n]->b_data, sb->s_blocksize);
			set_buffer_uptodate(c_bh);
			mark_buffer_dirty_inode(c_bh, sbi->fat_inode);
			if (sb->s_flags & MS_SYNCHRONOUS)
				err = sync_dirty_buffer(c_bh);
			brelse(c_bh);
			if (err)
				goto error;
		}
	}
error:
	return err;
}

int fat_ent_write(struct inode *inode, struct fat_entry *fatent,
		  int new, int wait)
{
	struct super_block *sb = inode->i_sb;
	// struct fatent_operations *ops = MSDOS_SB(sb)->fatent_ops;
	int err;

	// ops->ent_put(fatent, new);
	fat32_ent_put(fatent, new);
	if (wait) {
		err = fat_sync_bhs(fatent->bhs, fatent->nr_bhs);
		if (err)
			return err;
	}
	return fat_mirror_bhs(sb, fatent->bhs, fatent->nr_bhs);
}

static inline int fat_ent_next(struct fat_sb_info *sbi,
			       struct fat_entry *fatent)
{
	// if (sbi->fatent_ops->ent_next(fatent)) {
	if (fat32_ent_next(fatent)) {
		if (fatent->entry < sbi->max_cluster)
			return 1;
	}
	return 0;
}

static inline int fat_ent_read_block(struct super_block *sb,
				     struct fat_entry *fatent)
{
	// struct fatent_operations *ops = MSDOS_SB(sb)->fatent_ops;
	sector_t blocknr;
	int offset;

	fatent_brelse(fatent);
	// ops->ent_blocknr(sb, fatent->entry, &offset, &blocknr);
	fat_ent_blocknr(sb, fatent->entry, &offset, &blocknr);
	// return ops->ent_bread(sb, fatent, offset, blocknr);
	return fat_ent_bread(sb, fatent, offset, blocknr);
}

static void fat_collect_bhs(struct buffer_head **bhs, int *nr_bhs,
			    struct fat_entry *fatent)
{
	int n, i;

	for (n = 0; n < fatent->nr_bhs; n++) {
		for (i = 0; i < *nr_bhs; i++) {
			if (fatent->bhs[n] == bhs[i])
				break;
		}
		if (i == *nr_bhs) {
			get_bh(fatent->bhs[n]);
			bhs[i] = fatent->bhs[n];
			(*nr_bhs)++;
		}
	}
}

int fat_alloc_clusters(struct inode *inode, int *cluster, int nr_cluster)
{
	struct super_block *sb = inode->i_sb;
	struct fat_sb_info *sbi = MSDOS_SB(sb);
	// struct fatent_operations *ops = sbi->fatent_ops;
	struct fat_entry fatent, prev_ent;
	struct buffer_head *bhs[MAX_BUF_PER_PAGE];
	int i, count, err, nr_bhs, idx_clus;

	BUG_ON(nr_cluster > (MAX_BUF_PER_PAGE / 2));	/* fixed limit */

	lock_fat(sbi);
	if (sbi->free_clusters != -1 && sbi->free_clus_valid &&
	    sbi->free_clusters < nr_cluster) {
		unlock_fat(sbi);
		return -ENOSPC;
	}

	err = nr_bhs = idx_clus = 0;
	count = FAT_START_ENT;
	fatent_init(&prev_ent);
	fatent_init(&fatent);
	fatent_set_entry(&fatent, sbi->prev_free + 1);
	while (count < sbi->max_cluster) {
		if (fatent.entry >= sbi->max_cluster)
			fatent.entry = FAT_START_ENT;
		fatent_set_entry(&fatent, fatent.entry);
		err = fat_ent_read_block(sb, &fatent);
		if (err)
			goto out;

		/* Find the free entries in a block */
		do {
			// if (ops->ent_get(&fatent) == FAT_ENT_FREE) {
			if (fat32_ent_get(&fatent) == FAT_ENT_FREE) {
				int entry = fatent.entry;

				/* make the cluster chain */
				// ops->ent_put(&fatent, FAT_ENT_EOF);
				fat32_ent_put(&fatent, FAT_ENT_EOF);
				if (prev_ent.nr_bhs)
					// ops->ent_put(&prev_ent, entry);
					fat32_ent_put(&prev_ent, entry);

				fat_collect_bhs(bhs, &nr_bhs, &fatent);

				sbi->prev_free = entry;
				if (sbi->free_clusters != -1)
					sbi->free_clusters--;
				sb->s_dirt = 1;

				cluster[idx_clus] = entry;
				idx_clus++;
				if (idx_clus == nr_cluster)
					goto out;

				/*
				 * fat_collect_bhs() gets ref-count of bhs,
				 * so we can still use the prev_ent.
				 */
				prev_ent = fatent;
			}
			count++;
			if (count == sbi->max_cluster)
				break;
		} while (fat_ent_next(sbi, &fatent));
	}

	/* Couldn't allocate the free entries */
	sbi->free_clusters = 0;
	sbi->free_clus_valid = 1;
	sb->s_dirt = 1;
	err = -ENOSPC;

out:
	unlock_fat(sbi);
	fatent_brelse(&fatent);
	if (!err) {
		if (inode_needs_sync(inode))
			err = fat_sync_bhs(bhs, nr_bhs);
		if (!err)
			err = fat_mirror_bhs(sb, bhs, nr_bhs);
	}
	for (i = 0; i < nr_bhs; i++)
		brelse(bhs[i]);

	if (err && idx_clus)
		fat_free_clusters(inode, cluster[0]);

	return err;
}


int fat_free_clusters(struct inode *inode, int cluster)
{
	struct super_block *sb = inode->i_sb;
	struct fat_sb_info *sbi = MSDOS_SB(sb);
	// struct fatent_operations *ops = sbi->fatent_ops;
	struct fat_entry fatent;
	struct buffer_head *bhs[MAX_BUF_PER_PAGE];
	int i, err, nr_bhs;
	int first_cl = cluster;

	nr_bhs = 0;
	fatent_init(&fatent);
	lock_fat(sbi);
	do {
		cluster = fat_ent_read(inode, &fatent, cluster);
		if (cluster < 0) {
			err = cluster;
			goto error;
		} else if (cluster == FAT_ENT_FREE) {
			fat_fs_error(sb, "%s: deleting FAT entry beyond EOF",
				     __func__);
			err = -EIO;
			goto error;
		}

		/* 
		 * Issue discard for the sectors we no longer care about,
		 * batching contiguous clusters into one request
		 */
		if (cluster != fatent.entry + 1) {
			int nr_clus = fatent.entry - first_cl + 1;

			sb_issue_discard(sb, fat_clus_to_blknr(sbi, first_cl),
					 nr_clus * sbi->sec_per_clus);
			first_cl = cluster;
		}

		// ops->ent_put(&fatent, FAT_ENT_FREE);
		fat32_ent_put(&fatent, FAT_ENT_FREE);
		if (sbi->free_clusters != -1) {
			sbi->free_clusters++;
			sb->s_dirt = 1;
		}

		if (nr_bhs + fatent.nr_bhs > MAX_BUF_PER_PAGE) {
			if (sb->s_flags & MS_SYNCHRONOUS) {
				err = fat_sync_bhs(bhs, nr_bhs);
				if (err)
					goto error;
			}
			err = fat_mirror_bhs(sb, bhs, nr_bhs);
			if (err)
				goto error;
			for (i = 0; i < nr_bhs; i++)
				brelse(bhs[i]);
			nr_bhs = 0;
		}
		fat_collect_bhs(bhs, &nr_bhs, &fatent);
	} while (cluster != FAT_ENT_EOF);

	if (sb->s_flags & MS_SYNCHRONOUS) {
		err = fat_sync_bhs(bhs, nr_bhs);
		if (err)
			goto error;
	}
	err = fat_mirror_bhs(sb, bhs, nr_bhs);
error:
	fatent_brelse(&fatent);
	for (i = 0; i < nr_bhs; i++)
		brelse(bhs[i]);
	unlock_fat(sbi);

	return err;
}

EXPORT_SYMBOL_GPL(fat_free_clusters);



