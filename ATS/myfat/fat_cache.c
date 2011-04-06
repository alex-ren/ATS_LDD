#include "fat_cache.h"
#include "sb_mgr.h"
#include "fat_inode.h"
#include "fat_entry.h"
#include "fat_misc.h"

// #include <linux/msdos_fs.h>
/*
* Desc: transform no. of entry in FAT (or the no. of cluster in data region)
*   to the number of sectors in the volume
*
*/
static inline sector_t fat_clus_to_blknr(struct fat_sb_info *sbi, int clus)
{
        printk (KERN_INFO "myfat: fat_clus_to_blknr\n");
	return ((sector_t)clus - FAT_START_ENT) * sbi->sec_per_clus
		+ sbi->data_start;
}

/*
* fun: map no. of cluster in file to no. of cluster in the data region on the disk
* Para:
*   in:
*   cluster: no. of cluster in the file which we want to map to the cluster
*       in the volumn
*   out:
*   fclus: no. of cluster in the file we finally work on (most of time, it
*       is equal to cluster, could be < cluster if file is not long enough)
*  Info: can return FAT_ENT_EOF 
*/
int fat_get_cluster(struct inode *inode, int cluster, int *fclus, int *dclus)
{
    struct super_block *sb = inode->i_sb;

    // one file can at most use "limit" clusters
    const int limit = sb->s_maxbytes >> MSDOS_SB(sb)->cluster_bits;
    int nclus = 0;

    int ret = 0;

    // first cluster should be known already, which is not 0
    BUG_ON(MSDOS_I(inode)->i_start == 0);
    *fclus = 0;
    *dclus = MSDOS_I(inode)->i_start;

    printk (KERN_INFO "myfat: fat_get_cluster cluster is %d, \
fclus is %d, dclus is %d\n", cluster, *fclus, *dclus);
    while (*fclus < cluster)
    {
        if (*fclus > limit)
        {
            
            fat_fs_error(sb, "%s: detected the cluster chain loop"
                    " (i_pos %lld)", __func__,
                    MSDOS_I(inode)->i_pos);
            ret = -EIO;
            goto out;
        }

        // dclus is equivalent to entry
        ret = fatent_next(inode, *dclus, &nclus);
        if (ret < 0)
        {
            goto out;
        }
        else if (FAT_ENT_FREE == nclus)
        {
            fat_fs_error(sb, "%s: invalid cluster chain"
                        "(i_pos %lld)", __func__,
                        MSDOS_I(inode)->i_pos);
            ret = -EIO;
            goto out;
        }
        else if (FAT_ENT_EOF == nclus)
        {
            goto out;
        }  // FAT_ENT_BAD has been handled underneath

        *fclus = *fclus + 1;
        *dclus = nclus;
    }

out:

    return ret;
}

/*
* Desc: transform the cluster in file to cluster in the data region
*
*/
static int fat_bmap_cluster(struct inode *inode, int cluster)
{
	struct super_block *sb = inode->i_sb;
	int ret, fclus, dclus;

	if (MSDOS_I(inode)->i_start == 0)
		return 0;

	ret = fat_get_cluster(inode, cluster, &fclus, &dclus);
	if (ret < 0)
		return ret;
	else if (ret == FAT_ENT_EOF) {
		fat_fs_error(sb, "%s: request beyond EOF (i_pos %lld)",
			     __func__, MSDOS_I(inode)->i_pos);
		return -EIO;
	}
	return dclus;
}

/*
* Desc: transform sector (inside a file) to a sector in the volume
* Para:
*   In:
*   sector: no. of sector (inside a file)
*   Out:
*   phys: no. of cluster in volume
*   mapped_blocks: of no use now
*   create: of no use now
*
*/
int fat_bmap(struct inode *inode, sector_t sector, sector_t *phys,
	     unsigned long *mapped_blocks, int create)
{
    struct super_block *sb = inode->i_sb;
    struct fat_sb_info *sbi = MSDOS_SB(sb);
    const unsigned long blocksize = sb->s_blocksize;
    const unsigned char blocksize_bits = sb->s_blocksize_bits;
    sector_t last_block;
    int cluster, offset;
    
    *phys = 0;
    *mapped_blocks = 0;
    if ((sbi->fat_bits != 32) && (inode->i_ino == MSDOS_ROOT_INO)) {
        // We only handle FAT 32
    
    	// if (sector < (sbi->dir_entries >> sbi->dir_per_block_bits)) {
    	// 	*phys = sector + sbi->dir_start;
    	// 	*mapped_blocks = 1;
    	// }
    	// return 0;
    }
    
    last_block = (i_size_read(inode) + (blocksize - 1)) >> blocksize_bits;
    if (sector >= last_block) {
    	if (!create)
    		return 0;
    
    	/*
    	 * ->mmu_private can access on only allocation path.
    	 * (caller must hold ->i_mutex)
    	 */
    	// last_block = (MSDOS_I(inode)->mmu_private + (blocksize - 1))
    	// 	>> blocksize_bits;
    	// if (sector >= last_block)
    	// 	return 0;
        // todo what's this?
        return 0;  // add by rzq
    }
    
    // no. of cluster in a file
    cluster = sector >> (sbi->cluster_bits - sb->s_blocksize_bits);
    // offset in a block
    offset  = sector & (sbi->sec_per_clus - 1);
    // cluster: no. of cluster in the data region
    cluster = fat_bmap_cluster(inode, cluster);
    if (cluster < 0)
    	return cluster;
    else if (cluster) {
        // no. of sectors in the volume
    	*phys = fat_clus_to_blknr(sbi, cluster) + offset;
    	*mapped_blocks = sbi->sec_per_clus - offset;
    	if (*mapped_blocks > last_block - sector)
    		*mapped_blocks = last_block - sector;
    }
    return 0;
}
