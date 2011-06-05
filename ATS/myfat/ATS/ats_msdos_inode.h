//
// Author: Zhiqiang Ren (aren AT cs DOT bu DOT edu)
// Time: May 31st., 2011
//
/* ****** ****** */

#ifndef ATS_MSDOS_INODE_H
#define ATS_MSDOS_INODE_H


#include <linux/fs.h>

#include "ATS/include/atsfs_types.h"

// int msdos_create(struct inode *dir, struct dentry *dentry, int mode,
// 			struct nameidata *nd)
ats_int_type
atsfs_msdos_create (ats_ref_type arg0, ats_ref_type arg1, 
  ats_int_type arg2, ats_ref_type arg3);

#endif

