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
ats_ptr_type atsfs_fat_sb2fat_mount_options(ats_ref_type sbi)
{
    return (ats_ptr_type)(&((struct fat_sb_info *)sbi)->options);
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

ATSinline ()
ats_bool_type atsfs_IS_ERROR(ats_ptr_type p)
{
    return IS_ERR(p);
}

ATSinline ()
ats_int_type atsfs_error_neg(ats_int_type errno)
{
    return -errno;
}

ATSinline ()
ats_void_type atsfs_inode_init_time(struct inode *inode, ats_ref_type ts)
{
    struct timespec *t = (struct timespec *)ts;
    inode->i_mtime = *t;
    inode->i_atime = *t;
    inode->i_ctime = *t;
}

ATSinline ()
ats_void_type atsfs_d_instantiate(ats_ref_type dentry, struct inode *inode)
{
    d_instantiate(dentry, inode);
}

ATSinline ()
ats_int_type atsfs_opt_ptr_err_bad(ats_ptr_type ptr)
{
    return PTR_ERR(ptr);
}


#endif


