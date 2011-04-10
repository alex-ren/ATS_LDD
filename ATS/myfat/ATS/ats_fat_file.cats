//
// Author: Zhiqiang Ren (aren AT cs DOT bu DOT edu)
// Time: April 6th., 2011
//
/* ****** ****** */

#ifndef ATS_FAT_FILE_CATS
#define ATS_FAT_FILE_CATS

/* ****** ****** */

#include "ATS/ats_fat_file.h"
#include "fat_inode.h"

/* ****** ****** */

struct inode * atsfs_file2inode(struct file *fp) 
{
    return fp->f_dentry->d_inode;
}

struct inode * atsfs_file2inode_read_acquire(struct file *fp)
{
    struct inode *i = fp->f_dentry->d_inode;
    mutex_lock(&i->i_mutex);
    return i;
}

void atsfs_file2inode_read_release(struct inode *i)
{
    mutex_unlock(&i->i_mutex);
}

struct fat_inode_info * inode2fat_inode (struct inode *inode)
{
    return MSDOS_I(inode);
}


#endif


