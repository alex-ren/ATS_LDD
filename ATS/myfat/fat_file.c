#include "fat_file.h"
#include "fat_dir.h"

#include "ATS/ats_fat_file.h"

static ssize_t fat_sync_read(struct file *filp, 
    char __user *buf, size_t len, loff_t *ppos)
{
    return atsfs_fat_sync_read(filp, buf, len, ppos);
}

const struct file_operations fat_dir_operations = {
	.llseek		= 0,  // todo my_generic_file_llseek,
	.read		= 0,  // todo my_generic_read_dir,
	.readdir	= fat_readdir,
	.ioctl		= 0,  // todo fat_dir_ioctl,
#ifdef CONFIG_COMPAT
	.compat_ioctl	= 0,  // todo fat_compat_dir_ioctl,
#endif
	.fsync		= 0,  // todo fat_file_fsync,
};

const struct file_operations fat_file_operations = {
	.llseek		= 0,  // generic_file_llseek,
	.read		= fat_sync_read,
	.write		= 0,  // do_sync_write,
	.aio_read	= 0,  // generic_file_aio_read,
	.aio_write	= 0,  // generic_file_aio_write,
	.mmap		= 0,  // generic_file_mmap,
	.release	= 0,  // fat_file_release,
	.ioctl		= 0,  // fat_generic_ioctl,
	.fsync		= 0,  // fat_file_fsync,
	.splice_read	= 0,  // generic_file_splice_read,
};



