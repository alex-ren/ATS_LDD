//
// Author: Zhiqiang Ren (aren AT cs DOT bu DOT edu)
// Time: May 31st., 2011
//
/* ****** ****** */

#ifndef ATS_MSDOS_INODE_H
#define ATS_MSDOS_INODE_H


#include <linux/fs.h>

#include "ATS/include/atsfs_basic_types.h"

// int msdos_create(struct inode *dir, struct dentry *dentry, int mode,
// 			struct nameidata *nd)
ats_int_type
atsfs_msdos_create(ats_ptr_type p_buf, ats_ref_type dir, ats_ref_type dentry, 
  ats_int_type mode, ats_ref_type nd);

#endif

