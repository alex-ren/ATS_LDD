#include "fat_cache.h"
#include "sb_mgr.h"
#include "fat_inode.h"
#include "fat_entry.h"
#include "fat_misc.h"

// #include <linux/msdos_fs.h>

/* this must be > 0. */
#define FAT_MAX_CACHE	8

struct fat_cache {
	struct list_head cache_list;
	int nr_contig;	/* number of contiguous clusters */
	int fcluster;	/* cluster number in the file. */
	int dcluster;	/* cluster number on disk. */
};

static struct kmem_cache *fat_cache_cachep;



static void init_once(void *foo)
{
	struct fat_cache *cache = (struct fat_cache *)foo;

	INIT_LIST_HEAD(&cache->cache_list);
}

int __init fat_cache_init(void)
{
	fat_cache_cachep = kmem_cache_create("fat_cache",
				sizeof(struct fat_cache),
				0, SLAB_RECLAIM_ACCOUNT|SLAB_MEM_SPREAD,
				init_once);
	if (fat_cache_cachep == NULL)
		return -ENOMEM;
	return 0;
}

void fat_cache_destroy(void)
{
	kmem_cache_destroy(fat_cache_cachep);
}

static inline struct fat_cache *fat_cache_alloc(struct inode *inode)
{
	return kmem_cache_alloc(fat_cache_cachep, GFP_NOFS);
}

static inline void fat_cache_free(struct fat_cache *cache)
{
	BUG_ON(!list_empty(&cache->cache_list));
	kmem_cache_free(fat_cache_cachep, cache);
}


/* by the original author
 * Cache invalidation occurs rarely, thus the LRU chain is not updated. It
 * fixes itself after a while.
 */
/*
* I don't understand the comment above.
* To me, this function just remove the cache list for the inode.
*/
// static void __fat_cache_inval_inode(struct inode *inode)
// {
// 	struct fat_inode_info *i = MSDOS_I(inode);
// 	struct fat_cache *cache;
// 
// 	while (!list_empty(&i->cache_lru)) {
// 		cache = list_entry(i->cache_lru.next, struct fat_cache, cache_list);
// 		list_del_init(&cache->cache_list);
// 		i->nr_caches--;
// 		fat_cache_free(cache);
// 	}
// 	/* Update. The copy of caches before this id is discarded. */
// 	i->cache_valid_id++;
// 	if (i->cache_valid_id == FAT_CACHE_VALID)
// 		i->cache_valid_id++;
// }

// void fat_cache_inval_inode(struct inode *inode)
// {
// 	spin_lock(&MSDOS_I(inode)->cache_lru_lock);
// 	__fat_cache_inval_inode(inode);
// 	spin_unlock(&MSDOS_I(inode)->cache_lru_lock);
// }


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
*   phys: no. of block in volume
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

