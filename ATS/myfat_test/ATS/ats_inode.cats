//
// Author: Zhiqiang Ren (aren AT cs DOT bu DOT edu)
// Time: March 17th., 2011
//
/* ****** ****** */

#ifndef ATS_INODE_CATS
#define ATS_INODE_CATS

/* ****** ****** */

#include <linux/fs.h>
/* ****** ****** */

struct inode * ats_new_inode(struct super_block *sb);

#endif


