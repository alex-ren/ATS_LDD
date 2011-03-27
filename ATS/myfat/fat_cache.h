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

#endif

