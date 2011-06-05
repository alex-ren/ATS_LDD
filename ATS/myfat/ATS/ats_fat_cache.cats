//
// Author: Zhiqiang Ren (aren AT cs DOT bu DOT edu)
// Time: April 6th., 2011
//

/* ************* *************** */

#ifndef ATS_FAT_CACHE_CATS
#define ATS_FS_CACHE_CATS 

#include "ATS/ats_fs_types.h"

/* ************* *************** */

ATSinline ()
ats_size_type atsfs_get_blocksize(super_block_struct *sb)
{
    return sb->s_blocksize;
}

ATSinline ()
ats_size_type atsfs_get_clustersize(fat_sb_info_struct *sbi)
{
    return sbi->cluster_size;
}

ATSinline ()
ats_ptr_type atsfs_sbread(super_block_struct *sb, sector_t blocknr,
                          struct buffer_head **bh)
{
    char ch = '\0';
    char temp = '\0';
    char *raw_data = 0;
    printk(KERN_INFO "myfat: atsfs_sbread\n");

    *bh = sb_bread(sb, blocknr);
    if (NULL == *bh)
    {
        return NULL;
    }
    else
    {
        raw_data = (*bh)->b_data;
        printk(KERN_INFO "myfat: atsfs_sbread addr of b_data is %p\n", (*bh)->b_data);
        temp = raw_data[1];
        ch = temp;
        if (ch == 'z')
        {
            ch = 'a';
        }
        else
        {
            ch++;
        }

        raw_data[1] = ch;
	mark_buffer_dirty(*bh);
        sync_dirty_buffer(*bh);

        if (NULL == *bh)
        {
            return NULL;
        }
        else
        {
            return *bh;
        }
    }
}

// ats guratantees that bh is not null
ATSinline ()
void atsfs_bufferheadptr_free (struct buffer_head *bh)
{
    printk(KERN_INFO "myfat: atsfs_bufferheadptr_free\n");
    brelse(bh);
}
    
ATSinline ()
char* atsfs_bufferheadptr_get_buf (struct buffer_head *bh)
{
    return bh->b_data;
}

ATSinline ()
sector_t atsfsnblock_plus_loff_t(sector_t s, loff_t off)
{
    return s + off;
}

#endif


