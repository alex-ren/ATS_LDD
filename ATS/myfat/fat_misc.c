#include "fat_misc.h"
#include "sb_mgr.h"

#include <linux/module.h>

void fat_fs_error(struct super_block *s, const char *fmt, ...)
{
	struct fat_mount_options *opts = &MSDOS_SB(s)->options;
	va_list args;

	printk(KERN_ERR "myfat: Filesystem error (dev %s)\n", s->s_id);

	printk(KERN_ERR "    ");
	va_start(args, fmt);
	vprintk(fmt, args);
	va_end(args);
	printk("\n");

	if (opts->errors == FAT_ERRORS_PANIC)
		panic("    FAT fs panic from previous error\n");
	else if (opts->errors == FAT_ERRORS_RO && !(s->s_flags & MS_RDONLY)) {
		s->s_flags |= MS_RDONLY;
		printk(KERN_ERR "    File system has been set read-only\n");
	}
}

EXPORT_SYMBOL_GPL(fat_fs_error);


