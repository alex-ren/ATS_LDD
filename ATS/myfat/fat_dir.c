#include "fat_dir.h"

#include <linux/fs.h>


const struct file_operations fat_dir_operations = {
	.llseek		= 0,  // todo my_generic_file_llseek,
	.read		= 0,  // todo my_generic_read_dir,
	.readdir	= 0,  // todo fat_readdir,
	.ioctl		= 0,  // todo fat_dir_ioctl,
#ifdef CONFIG_COMPAT
	.compat_ioctl	= 0,  // todo fat_compat_dir_ioctl,
#endif
	.fsync		= 0,  // todo fat_file_fsync,
};

