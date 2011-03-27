#ifndef FAT_CACHE_H
#define FAT_CACHE_H

#define FAT_CACHE_VALID 0

#include <linux/fs.h>

// todo: go to ATS
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

#endif

