
//
// Author: Zhiqiang Ren (aren AT cs DOT bu DOT edu)
// Time: April 6th., 2011
//

/* ****** ****** */

#ifndef ATS_FS_TYPES_H
#define ATS_FS_TYPES_H

#include "linux/namei.h"
#include "linux/dcache.h"
#include "linux/time.h"

typedef struct nameidata nameidata_struct;

typedef struct inode inode_struct;
typedef struct file file_struct;
typedef struct dentry dentry_struct;
typedef struct super_block super_block_struct;
typedef struct fat_inode_info fat_inode_info_struct;
typedef struct fat_sb_info fat_sb_info_struct;

typedef struct timespec timespec_struct;


#endif
