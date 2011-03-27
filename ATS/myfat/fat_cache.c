#include "fat_cache.h"
#include "sb_mgr.h"
#include "fat_inode.h"
#include "fat_entry.h"
#include "fat_misc.h"


static inline int getNextEntry(struct inode *inode, int curent, int *nxtent)
{
    struct super_block *sb = inode->i_sb;
    struct fat_entry ent;

    int offset = 0;
    sector_t blocknr = 0;

    int ret = 0;

    fatent_init (&ent);
    fatent_set_entry(&ent, curent);  // set the no. of entry
    fat_ent_blocknr(sb, curent, &offset, &blocknr);  // get the block no. and offset of dclus
    
    if (fat_ent_bread(sb, &ent, offset, blocknr))
    {
        ret = -EIO;
        goto out;
    }

    *nxtent = fat32_ent_get(&ent);
    if (FAT_ENT_EOF == *nxtent)
    {
        ret = -EIO;
        goto out;
    }

out:
    fatent_brelse(&ent);
    return ret;
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
*   
*/
int fat_get_cluster(struct inode *inode, int cluster, int *fclus, int *dclus)
{
    struct super_block *sb = inode->i_sb;

    // one file can at most use "limit" clusters
    const int limit = sb->s_maxbytes >> MSDOS_SB(sb)->cluster_bits;
    int nclus = 0;

    int nr = 0;

    // first cluster should be known already, which is not 0
    BUG_ON(MSDOS_I(inode)->i_start == 0);
    *fclus = 0;
    *dclus = MSDOS_I(inode)->i_start;

    while (*fclus < cluster)
    {
        if (*fclus > limit)
        {
            
            fat_fs_error(sb, "%s: detected the cluster chain loop"
                    " (i_pos %lld)", __func__,
                    MSDOS_I(inode)->i_pos);
            nr = -EIO;
            goto out;
        }

        // dclus is equivalent to entry
        if (getNextEntry(inode, *dclus, &nclus))
        {
            // end of file or error, return
            nr = 0;
            goto out;
        }
        *fclus = *fclus + 1;
        *dclus = nclus;
    }

out:

    return nr;
}

