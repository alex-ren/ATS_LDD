//
// Author: Zhiqiang Ren (aren AT cs DOT bu DOT edu)
// Time: April 6th., 2011
//
/* ****** ****** */

#ifndef ATS_FAT_FILE_CATS
#define ATS_FAT_FILE_CATS

/* ****** ****** */
#include "ATS/atsfs_basic_types.h"

#include "ATS/ats_fat_file.h"
#include "fat_inode.h"

#include "ATS/atsfs_types_opr.h"

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
char * atsfs_sbread(super_block_struct *sb, sector_t blocknr)
{
    struct buffer_head *bh = sb_bread(sb, blocknr);
    if (NULL = bh)
    {
        return NULL:
    }
    else
    {
        return bh->b_data;
    }
}


#endif


