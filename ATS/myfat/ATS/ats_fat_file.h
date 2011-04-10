//
// Author: Zhiqiang Ren (aren AT cs DOT bu DOT edu)
// Time: April 6th., 2011
//
/* ****** ****** */

#ifndef ATS_FAT_FILE_H
#define ATS_FAT_FILE_H

/* ****** ****** */

#include <linux/fs.h>
/* ****** ****** */

ssize_t ats_fat_sync_read(struct file *filp, 
    char __user *buf, size_t len, loff_t *ppos);

// struct inode * ats_new_inode(struct super_block *sb);

#endif


