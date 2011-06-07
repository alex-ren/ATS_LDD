//
// Author: Zhiqiang Ren (aren AT cs DOT bu DOT edu)
// Time: April 6th., 2011
//

/* ****** ****** */

#ifndef ATS_FS_TYPES_CATS
#define ATS_FS_TYPES_CATS

#include "ATS/ats_fs_types.h"

#include "fat_inode.h"

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
ats_void_type atsfs_lock_super (super_block_struct *sb)
{
    lock_super(sb);
}

ATSinline ()
ats_void_type atsfs_unlock_super (super_block_struct *sb)
{
    unlock_super(sb);
}

ATSinline ()
ats_ptr_type atsfs_dentry_get_d_name(dentry_struct *de, int *n)
{
    *n = de->d_name.len;
    return (ats_ptr_type)de->d_name.name;
}

#endif


