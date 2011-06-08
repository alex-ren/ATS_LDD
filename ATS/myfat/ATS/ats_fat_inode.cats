//
// Author: Zhiqiang Ren (aren AT cs DOT bu DOT edu)
// Time: March 17th., 2011
//
/* ****** ****** */

#ifndef ATS_FAT_INODE_CATS
#define ATS_FAT_INODE_CATS

/* ****** ****** */

#include "ATS/ats_fat_inode.h"

#include "fat_dir.h"
#include "fat_inode.h"

/* ****** ****** */

ATSinline ()
struct inode * atsfs_fat_build_inode(
  ats_ref_type sb, struct msdos_dir_entry *de, loff_t i_pos)
{
    return fat_build_inode(sb, de, i_pos);
}

#endif


