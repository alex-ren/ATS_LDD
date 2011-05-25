//
// Author: Zhiqiang Ren (aren AT cs DOT bu DOT edu)
// Time: April 6th., 2011
//
/* ****** ****** */

#ifndef ATS_FAT_FILE_CATS
#define ATS_FAT_FILE_CATS


/* ****** ****** */
/* ****** ****** */
#include "ATS/ats_fat_file.h"
#include "ATS/atsfs_user_types_opr.h"

#include "fat_inode.h"
#include "fat_entry.h"


#include <linux/buffer_head.h>

typedef struct inode inode_struct;
typedef struct file file_struct;
typedef struct super_block super_block_struct;
typedef struct fat_inode_info fat_inode_info_struct;
typedef struct fat_sb_info fat_sb_info_struct;

/* ****** ****** */

ATSinline ()
struct inode * atsfs_file2inode(file_struct *fp) 
{
    struct inode *i = fp->f_dentry->d_inode;
    // printk(KERN_INFO "myfat: atsfs_file2inode_acquire, inode addr is %p\n", i);
    return fp->f_dentry->d_inode;
}

ATSinline ()
struct inode * atsfs_file2inode_acquire(file_struct *fp)
{
    struct inode *i = fp->f_dentry->d_inode;
    // printk(KERN_INFO "myfat: atsfs_file2inode_acquire, inode addr is %p\n", i);
    mutex_lock(&i->i_mutex);
    return i;
}

ATSinline ()
void atsfs_file2inode_release(inode_struct *i)
{
    mutex_unlock(&i->i_mutex);
}

ATSinline ()
ats_ptr_type atsfs_inode_own2inode(inode_struct *i)
{
    return i;
}

ATSinline ()
ats_ptr_type atsfs_inode2fat_inode(inode_struct *i)
{
    return MSDOS_I(i);
}

ATSinline ()
ats_ptr_type atsfs_inode2super_block(inode_struct *i)
{
    return i->i_sb;
}

ATSinline ()
ats_ptr_type atsfs_sb2fat_sb(super_block_struct *sb)
{
    return MSDOS_SB(sb);
}

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
    *bh = sb_bread(sb, blocknr);
    if (NULL == *bh)
    {
        return NULL;
    }
    else
    {
        return *bh;
    }
}

// ats guratantees that bh is not null
ATSinline ()
void atsfs_bufferheadptr_free (struct buffer_head *bh)
{
    brelse(bh);
}
    
ATSinline ()
char* atsfs_bufferheadptr_get_buf (struct buffer_head *bh)
{
    return bh->b_data;
}

ATSinline ()
int atsfs_get_first_cluster(struct inode * inode)
{
    int ncls = MSDOS_I(inode)->i_start;
    // printk(KERN_INFO 
    //   "myfat: atsfs_get_first_cluster, fat_inode addr is %p\n", 
    //   MSDOS_I(inode));
    // printk(KERN_INFO "myfat: atsfs_get_first_cluster, inode addr is %p\n", inode);
    // printk(KERN_INFO "myfat: atsfs_get_first_cluster, inode no is %lu\n", inode->i_ino);
    // printk(KERN_INFO "myfat: atsfs_get_first_cluster, i_start is %d\n", ncls);
    BUG_ON(0 == ncls);
    return ncls;
}

ATSinline ()
int atsfs_get_next_cluster(super_block_struct *sb, int cur_cls, int *nxt_cls)
{
    return fatent_next_sb(sb, cur_cls, nxt_cls);
}


ATSinline ()
sector_t atsfsnblock_plus_loff_t(sector_t s, loff_t off)
{
    return s + off;
}

#endif







