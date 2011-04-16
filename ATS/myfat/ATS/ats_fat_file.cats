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

typedef struct inode inode_struct;
typedef struct file file_struct;
typedef struct super_block super_block_struct;
typedef struct fat_inode_info fat_inode_info_struct;
typedef struct fat_sb_info fat_sb_info_struct;
/* ****** ****** */

ATSinline ()
struct inode * atsfs_file2inode(file_struct *fp) 
{
    return fp->f_dentry->d_inode;
}

ATSinline ()
struct inode * atsfs_file2inode_acquire(file_struct *fp)
{
    struct inode *i = fp->f_dentry->d_inode;
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
ats_ptr_type atsfs_inode2fat_inode(inode_struct *inode)
{
    return MSDOS_I(inode);
}

ATSinline ()
ats_ptr_type atsfs_inode2fat_inode_own(inode_struct *inode)
{
    return MSDOS_I(inode);
}

#endif


