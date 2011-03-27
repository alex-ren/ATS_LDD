#ifndef FAT_MISC_H
#define FAT_MISC_H

#include <linux/fs.h>
    
extern void fat_fs_error(struct super_block *s, const char *fmt, ...)
	__attribute__ ((format (printf, 2, 3))) __cold;

#endif


