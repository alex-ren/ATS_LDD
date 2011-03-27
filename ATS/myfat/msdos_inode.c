#include "msdos_inode.h"


const struct inode_operations msdos_dir_inode_operations = {
	.create		= 0,  // todo msdos_create,
	.lookup		= 0,  // todo msdos_lookup,
	.unlink		= 0,  // todo msdos_unlink,
	.mkdir		= 0,  // todo msdos_mkdir,
	.rmdir		= 0,  // todo msdos_rmdir,
	.rename		= 0,  // todo msdos_rename,
	.setattr	= 0,  // todo fat_setattr,
	.getattr	= 0,  // todo fat_getattr,
};

