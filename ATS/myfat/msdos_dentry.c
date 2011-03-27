#include "msdos_dentry.h"




const struct dentry_operations msdos_dentry_operations = {
	.d_hash		= 0,  // todo msdos_hash,
	.d_compare	= 0,  // todo msdos_cmp,
};

