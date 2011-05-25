#include "msdos_inode.h"
#include "sb_mgr.h"
#include "msdos_misc.h"
#include "fat_dir.h"
#include "msdos_dentry.h"
#include "fat_inode.h"


/*
 * AV. Wrappers for FAT sb operations. Is it wise?
 */

/***** Locates a directory entry.  Uses unformatted name. */
static int msdos_find(struct inode *dir, const unsigned char *name, int len,
		      struct fat_slot_info *sinfo)
{
	struct fat_sb_info *sbi = MSDOS_SB(dir->i_sb);
	unsigned char msdos_name[MSDOS_NAME];
	int err;

	err = msdos_format_name(name, len, msdos_name, &sbi->options);
	if (err)
		return -ENOENT;

	err = fat_scan(dir, msdos_name, sinfo);
	if (!err && sbi->options.dotsOK) {
		if (name[0] == '.') {
			if (!(sinfo->de->attr & ATTR_HIDDEN))
				err = -ENOENT;
		} else {
			if (sinfo->de->attr & ATTR_HIDDEN)
				err = -ENOENT;
		}
		if (err)
			brelse(sinfo->bh);
	}
	return err;
}

/***** Get inode using directory and name */
static struct dentry *msdos_lookup(struct inode *dir, struct dentry *dentry,
				   struct nameidata *nd)
{
	struct super_block *sb = dir->i_sb;
	struct fat_slot_info sinfo;
	struct inode *inode;
	int err;

        char szBuffer[20];
        int len = min((int)dentry->d_name.len, 19);
        memset(szBuffer, 0, 20);
        __memcpy(szBuffer, dentry->d_name.name, len);
        printk (KERN_INFO "myfat: msdos_lookup, dir is %s\n", szBuffer);

	lock_super(sb);

	err = msdos_find(dir, dentry->d_name.name, dentry->d_name.len, &sinfo);
	if (err) {
		if (err == -ENOENT) {
			inode = NULL;
			goto out;
		}
		goto error;
	}

	inode = fat_build_inode(sb, sinfo.de, sinfo.i_pos);  // by rzq: hold the reference count
	brelse(sinfo.bh);
	if (IS_ERR(inode)) {
		err = PTR_ERR(inode);
		goto error;
	}
out:
        printk (KERN_INFO "myfat: msdos_lookup, succees: inode addr is %p\n", inode);
	unlock_super(sb);
	dentry->d_op = &msdos_dentry_operations;
	dentry = d_splice_alias(inode, dentry);
	if (dentry)
		dentry->d_op = &msdos_dentry_operations;

       // printk (KERN_INFO "myfat: msdos_lookup, inode in dentry, addr is %p\n", dentry->d_inode);
	return dentry;

error:
	unlock_super(sb);
	return ERR_PTR(err);
}


const struct inode_operations msdos_dir_inode_operations = {
	.create		= 0,  // todo msdos_create,
	.lookup		= msdos_lookup,
	.unlink		= 0,  // todo msdos_unlink,
	.mkdir		= 0,  // todo msdos_mkdir,
	.rmdir		= 0,  // todo msdos_rmdir,
	.rename		= 0,  // todo msdos_rename,
	.setattr	= 0,  // todo fat_setattr,
	.getattr	= fat_getattr,
};

