//
// Author: Zhiqiang Ren (aren AT cs DOT bu DOT edu)
// Time: April 6th., 2011
//
/* ****** ****** */

#ifndef ATS_FAT_FILE_H
#define ATS_FAT_FILE_H

/* ****** ****** */

#include <linux/fs.h>

#include "ATS/include/atsfs_basic_types.h"

/* ****** ****** */
/* ****** ****** */

// ats_ssize_type atsfs_fat_sync_read(struct file *filp, 
//     char __user *buf, size_t len, loff_t *ppos);
ats_ssize_type
atsfs_fat_sync_read (ats_ref_type arg0, ats_ptr_type arg1, 
    ats_size_type arg2, ats_ref_type arg3);

#endif


