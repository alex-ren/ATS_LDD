#ifndef FAT_CACHE_H
#define FAT_CACHE_H

#define FAT_CACHE_VALID 0

#include "sb_mgr.h"
#include <linux/fs.h>

extern void fat_cache_inval_inode(struct inode *inode);

/*
* fun: map no. of cluster in file to no. of cluster in the data region on the disk
* Para:
*   in:
*   cluster: no. of cluster in the file which we want to map to the cluster
*       in the volumn
*   out:
*   fclus: no. of cluster in the file we finally work on (most of time, it
*       is equal to cluster, could be < cluster if file is not long enough)
*   
*/
extern int fat_get_cluster(struct inode *, int cluster, int *fclus, int *dclus);

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
* Desc: transform sector (inside a file) to no. of sector in the volume
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
	     unsigned long *mapped_blocks, int create);

int fat_cache_init(void);
void fat_cache_destroy(void);

#endif

