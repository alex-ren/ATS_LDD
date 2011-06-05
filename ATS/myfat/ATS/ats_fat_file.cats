//
// Author: Zhiqiang Ren (aren AT cs DOT bu DOT edu)
// Time: April 6th., 2011
//
/* ****** ****** */

#ifndef ATS_FAT_FILE_CATS
#define ATS_FAT_FILE_CATS


/* ****** ****** */
#include "ATS/ats_fs_types.h"

#include "ATS/ats_fat_file.h"

#include "ATS/atsfs_user_types_opr.cats"

/* ****** ****** */

#include "fat_inode.h"
#include "fat_entry.h"

/* ****** ****** */

#include <linux/buffer_head.h>

/* ****** ****** */

ATSinline ()
struct inode * atsfs_file2inode(file_struct *fp) 
{
    // struct inode *i = fp->f_dentry->d_inode;
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

#endif







