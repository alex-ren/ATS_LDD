#ifndef FAT_MISC_H
#define FAT_MISC_H

#include "sb_mgr.h"

#include <linux/buffer_head.h>
#include <linux/string.h>
#include <linux/nls.h>
#include <linux/fs.h>
#include <linux/mutex.h>
#include <linux/msdos_fs.h>

extern void fat_fs_error(struct super_block *s, const char *fmt, ...)
	__attribute__ ((format (printf, 2, 3))) __cold;
extern void fat_time_fat2unix(struct fat_sb_info *sbi, struct timespec *ts,
			      __le16 __time, __le16 __date, u8 time_cs);
extern void fat_time_unix2fat(struct fat_sb_info *sbi, struct timespec *ts,
			      __le16 *time, __le16 *date, u8 *time_cs);
#endif


