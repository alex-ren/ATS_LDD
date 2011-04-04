
#include "sb_mgr.h"
#include "fat_inode.h"
#include "fat_entry.h"

#include <linux/module.h>
#include <linux/init.h>
#include <linux/time.h>
#include <linux/slab.h>
#include <linux/smp_lock.h>
#include <linux/seq_file.h>
#include <linux/pagemap.h>
#include <linux/mpage.h>
#include <linux/buffer_head.h>
#include <linux/exportfs.h>
#include <linux/mount.h>
#include <linux/vfs.h>
#include <linux/parser.h>
#include <linux/uio.h>
#include <linux/writeback.h>
#include <linux/log2.h>
#include <linux/hash.h>
#include <asm/unaligned.h>

#include <linux/msdos_fs.h>

#ifndef CONFIG_FAT_DEFAULT_IOCHARSET
/* if user don't select VFAT, this is undefined. */
#define CONFIG_FAT_DEFAULT_IOCHARSET	""
#endif

#include "ATS/ats_fat_inode.h"

static inline int myfoo (int i) {
  return foo (i);
}

static int fat_default_codepage = CONFIG_FAT_DEFAULT_CODEPAGE;
static char fat_default_iocharset[] = CONFIG_FAT_DEFAULT_IOCHARSET;

enum {
	Opt_check_n, Opt_check_r, Opt_check_s, Opt_uid, Opt_gid,
	Opt_umask, Opt_dmask, Opt_fmask, Opt_allow_utime, Opt_codepage,
	Opt_usefree, Opt_nocase, Opt_quiet, Opt_showexec, Opt_debug,
	Opt_immutable, Opt_dots, Opt_nodots,
	Opt_charset, Opt_shortname_lower, Opt_shortname_win95,
	Opt_shortname_winnt, Opt_shortname_mixed, Opt_utf8_no, Opt_utf8_yes,
	Opt_uni_xl_no, Opt_uni_xl_yes, Opt_nonumtail_no, Opt_nonumtail_yes,
	Opt_obsolate, Opt_flush, Opt_tz_utc, Opt_rodir, Opt_err_cont,
	Opt_err_panic, Opt_err_ro, Opt_err,
};

static const match_table_t fat_tokens = {
	{Opt_check_r, "check=relaxed"},
	{Opt_check_s, "check=strict"},
	{Opt_check_n, "check=normal"},
	{Opt_check_r, "check=r"},
	{Opt_check_s, "check=s"},
	{Opt_check_n, "check=n"},
	{Opt_uid, "uid=%u"},
	{Opt_gid, "gid=%u"},
	{Opt_umask, "umask=%o"},
	{Opt_dmask, "dmask=%o"},
	{Opt_fmask, "fmask=%o"},
	{Opt_allow_utime, "allow_utime=%o"},
	{Opt_codepage, "codepage=%u"},
	{Opt_usefree, "usefree"},
	{Opt_nocase, "nocase"},
	{Opt_quiet, "quiet"},
	{Opt_showexec, "showexec"},
	{Opt_debug, "debug"},
	{Opt_immutable, "sys_immutable"},
	{Opt_flush, "flush"},
	{Opt_tz_utc, "tz=UTC"},
	{Opt_err_cont, "errors=continue"},
	{Opt_err_panic, "errors=panic"},
	{Opt_err_ro, "errors=remount-ro"},
	{Opt_obsolate, "conv=binary"},
	{Opt_obsolate, "conv=text"},
	{Opt_obsolate, "conv=auto"},
	{Opt_obsolate, "conv=b"},
	{Opt_obsolate, "conv=t"},
	{Opt_obsolate, "conv=a"},
	{Opt_obsolate, "fat=%u"},
	{Opt_obsolate, "blocksize=%u"},
	{Opt_obsolate, "cvf_format=%20s"},
	{Opt_obsolate, "cvf_options=%100s"},
	{Opt_obsolate, "posix"},
	{Opt_err, NULL},
};
static const match_table_t msdos_tokens = {
	{Opt_nodots, "nodots"},
	{Opt_nodots, "dotsOK=no"},
	{Opt_dots, "dots"},
	{Opt_dots, "dotsOK=yes"},
	{Opt_err, NULL}
};
static const match_table_t vfat_tokens = {
	{Opt_charset, "iocharset=%s"},
	{Opt_shortname_lower, "shortname=lower"},
	{Opt_shortname_win95, "shortname=win95"},
	{Opt_shortname_winnt, "shortname=winnt"},
	{Opt_shortname_mixed, "shortname=mixed"},
	{Opt_utf8_no, "utf8=0"},		/* 0 or no or false */
	{Opt_utf8_no, "utf8=no"},
	{Opt_utf8_no, "utf8=false"},
	{Opt_utf8_yes, "utf8=1"},		/* empty or 1 or yes or true */
	{Opt_utf8_yes, "utf8=yes"},
	{Opt_utf8_yes, "utf8=true"},
	{Opt_utf8_yes, "utf8"},
	{Opt_uni_xl_no, "uni_xlate=0"},		/* 0 or no or false */
	{Opt_uni_xl_no, "uni_xlate=no"},
	{Opt_uni_xl_no, "uni_xlate=false"},
	{Opt_uni_xl_yes, "uni_xlate=1"},	/* empty or 1 or yes or true */
	{Opt_uni_xl_yes, "uni_xlate=yes"},
	{Opt_uni_xl_yes, "uni_xlate=true"},
	{Opt_uni_xl_yes, "uni_xlate"},
	{Opt_nonumtail_no, "nonumtail=0"},	/* 0 or no or false */
	{Opt_nonumtail_no, "nonumtail=no"},
	{Opt_nonumtail_no, "nonumtail=false"},
	{Opt_nonumtail_yes, "nonumtail=1"},	/* empty or 1 or yes or true */
	{Opt_nonumtail_yes, "nonumtail=yes"},
	{Opt_nonumtail_yes, "nonumtail=true"},
	{Opt_nonumtail_yes, "nonumtail"},
	{Opt_rodir, "rodir"},
	{Opt_err, NULL}
};

// parse options from kernel, probably options is NULL
// fill the structure fat_mount_options
static int parse_options(char *options, int is_vfat, int silent, int *debug,
			 struct fat_mount_options *opts)
{
	char *p;
	substring_t args[MAX_OPT_ARGS];
	int option;
	char *iocharset;

	opts->isvfat = is_vfat;

	opts->fs_uid = current_uid();
	opts->fs_gid = current_gid();
	opts->fs_fmask = opts->fs_dmask = current_umask();
	opts->allow_utime = -1;
	opts->codepage = fat_default_codepage;
	opts->iocharset = fat_default_iocharset;
	if (is_vfat) {
		opts->shortname = VFAT_SFN_DISPLAY_WINNT|VFAT_SFN_CREATE_WIN95;
		opts->rodir = 0;
	} else {
		opts->shortname = 0;
		opts->rodir = 1;
	}
	opts->name_check = 'n';
	opts->quiet = opts->showexec = opts->sys_immutable = opts->dotsOK =  0;
	opts->utf8 = opts->unicode_xlate = 0;
	opts->numtail = 1;
	opts->usefree = opts->nocase = 0;
	opts->tz_utc = 0;
	opts->errors = FAT_ERRORS_RO;
	*debug = 0;

	if (!options)
		goto out;

	while ((p = strsep(&options, ",")) != NULL) {
		int token;
		if (!*p)
			continue;

		token = match_token(p, fat_tokens, args);
		if (token == Opt_err) {
			if (is_vfat)
				token = match_token(p, vfat_tokens, args);
			else
				token = match_token(p, msdos_tokens, args);
		}
		switch (token) {
		case Opt_check_s:
			opts->name_check = 's';
			break;
		case Opt_check_r:
			opts->name_check = 'r';
			break;
		case Opt_check_n:
			opts->name_check = 'n';
			break;
		case Opt_usefree:
			opts->usefree = 1;
			break;
		case Opt_nocase:
			if (!is_vfat)
				opts->nocase = 1;
			else {
				/* for backward compatibility */
				opts->shortname = VFAT_SFN_DISPLAY_WIN95
					| VFAT_SFN_CREATE_WIN95;
			}
			break;
		case Opt_quiet:
			opts->quiet = 1;
			break;
		case Opt_showexec:
			opts->showexec = 1;
			break;
		case Opt_debug:
			*debug = 1;
			break;
		case Opt_immutable:
			opts->sys_immutable = 1;
			break;
		case Opt_uid:
			if (match_int(&args[0], &option))
				return 0;
			opts->fs_uid = option;
			break;
		case Opt_gid:
			if (match_int(&args[0], &option))
				return 0;
			opts->fs_gid = option;
			break;
		case Opt_umask:
			if (match_octal(&args[0], &option))
				return 0;
			opts->fs_fmask = opts->fs_dmask = option;
			break;
		case Opt_dmask:
			if (match_octal(&args[0], &option))
				return 0;
			opts->fs_dmask = option;
			break;
		case Opt_fmask:
			if (match_octal(&args[0], &option))
				return 0;
			opts->fs_fmask = option;
			break;
		case Opt_allow_utime:
			if (match_octal(&args[0], &option))
				return 0;
			opts->allow_utime = option & (S_IWGRP | S_IWOTH);
			break;
		case Opt_codepage:
			if (match_int(&args[0], &option))
				return 0;
			opts->codepage = option;
			break;
		case Opt_flush:
			opts->flush = 1;
			break;
		case Opt_tz_utc:
			opts->tz_utc = 1;
			break;
		case Opt_err_cont:
			opts->errors = FAT_ERRORS_CONT;
			break;
		case Opt_err_panic:
			opts->errors = FAT_ERRORS_PANIC;
			break;
		case Opt_err_ro:
			opts->errors = FAT_ERRORS_RO;
			break;

		/* msdos specific */
		case Opt_dots:
			opts->dotsOK = 1;
			break;
		case Opt_nodots:
			opts->dotsOK = 0;
			break;

		/* vfat specific */
		case Opt_charset:
			if (opts->iocharset != fat_default_iocharset)
				kfree(opts->iocharset);
			iocharset = match_strdup(&args[0]);
			if (!iocharset)
				return -ENOMEM;
			opts->iocharset = iocharset;
			break;
		case Opt_shortname_lower:
			opts->shortname = VFAT_SFN_DISPLAY_LOWER
					| VFAT_SFN_CREATE_WIN95;
			break;
		case Opt_shortname_win95:
			opts->shortname = VFAT_SFN_DISPLAY_WIN95
					| VFAT_SFN_CREATE_WIN95;
			break;
		case Opt_shortname_winnt:
			opts->shortname = VFAT_SFN_DISPLAY_WINNT
					| VFAT_SFN_CREATE_WINNT;
			break;
		case Opt_shortname_mixed:
			opts->shortname = VFAT_SFN_DISPLAY_WINNT
					| VFAT_SFN_CREATE_WIN95;
			break;
		case Opt_utf8_no:		/* 0 or no or false */
			opts->utf8 = 0;
			break;
		case Opt_utf8_yes:		/* empty or 1 or yes or true */
			opts->utf8 = 1;
			break;
		case Opt_uni_xl_no:		/* 0 or no or false */
			opts->unicode_xlate = 0;
			break;
		case Opt_uni_xl_yes:		/* empty or 1 or yes or true */
			opts->unicode_xlate = 1;
			break;
		case Opt_nonumtail_no:		/* 0 or no or false */
			opts->numtail = 1;	/* negated option */
			break;
		case Opt_nonumtail_yes:		/* empty or 1 or yes or true */
			opts->numtail = 0;	/* negated option */
			break;
		case Opt_rodir:
			opts->rodir = 1;
			break;

		/* obsolete mount options */
		case Opt_obsolate:
			printk(KERN_INFO "FAT: \"%s\" option is obsolete, "
			       "not supported now\n", p);
			break;
		/* unknown option */
		default:
			if (!silent) {
				printk(KERN_ERR
				       "FAT: Unrecognized mount option \"%s\" "
				       "or missing value\n", p);
			}
			return -EINVAL;
		}
	}

out:
	/* UTF-8 doesn't provide FAT semantics */
	if (!strcmp(opts->iocharset, "utf8")) {
		printk(KERN_ERR "FAT: utf8 is not a recommended IO charset"
		       " for FAT filesystems, filesystem will be "
		       "case sensitive!\n");
	}

	/* If user doesn't specify allow_utime, it's initialized from dmask. */
	if (opts->allow_utime == (unsigned short)-1)
		opts->allow_utime = ~opts->fs_dmask & (S_IWGRP | S_IWOTH);
	if (opts->unicode_xlate)
		opts->utf8 = 0;

	return 0;
}

static void fat_hash_init(struct super_block *sb)
{
    struct fat_sb_info *sbi = MSDOS_SB(sb);
    int i;

    spin_lock_init(&sbi->inode_hash_lock);
    for (i = 0; i < FAT_HASH_SIZE; i++)
        INIT_HLIST_HEAD(&sbi->inode_hashtable[i]);

    return;
}

/*
 * Read the super block of an MS-DOS FS.
 */
int fat_fill_super(struct super_block *sb, void *data, int silent,
		   const struct inode_operations *fs_dir_inode_ops, int isvfat)
{
	struct inode *root_inode = NULL, *fat_inode = NULL;
	struct buffer_head *bh;
	struct fat_boot_sector *b;
	struct fat_sb_info *sbi;
	u16 logical_sector_size;
	u32 total_sectors, total_clusters, fat_clusters, rootdir_sectors;
	int debug;
	unsigned int media;
	long error;
	char buf[50];

        printk (KERN_INFO "myfat: fat_fill_super\n");

	/*
	 * GFP_KERNEL is ok here, because while we do hold the
	 * supeblock lock, memory pressure can't call back into
	 * the filesystem, since we're only just about to mount
	 * it and have no inodes etc active!
	 */
	sbi = kzalloc(sizeof(struct fat_sb_info), GFP_KERNEL);
	if (!sbi)
		return -ENOMEM;
	sb->s_fs_info = sbi;

	sb->s_flags |= MS_NODIRATIME;
	sb->s_magic = MSDOS_SUPER_MAGIC;
	sb->s_op = &fat_sops;
	sb->s_export_op = &fat_export_ops;
	sbi->dir_ops = fs_dir_inode_ops;  // this is the input argument
                                          // different between msdos and vfat

	error = parse_options(data, isvfat, silent, &debug, &sbi->options);
	if (error)
		goto out_fail;

	error = -EIO;
	sb_min_blocksize(sb, 512);  // set the block size to 512 if the 
                                    // block device supports

        // read the first block
	bh = sb_bread(sb, 0);  // read one block, don't forget to release bh
                            // using brelse(bh);
	if (bh == NULL) {
		printk(KERN_ERR "FAT: unable to read boot sector\n");
		goto out_fail;
	}

        // bh->b_data is the data read from hard disk
	b = (struct fat_boot_sector *) bh->b_data;
        // b->reserved: no. of reserved sectors (starting from sector 0 of 
        // the volume including the Boot Sector), after which is the FAT table
	if (!b->reserved) {
		if (!silent)
			printk(KERN_ERR "FAT: bogus number of reserved sectors\n");
		brelse(bh);
		goto out_invalid;
	}
        // b->fats: no. of FAT table
	if (!b->fats) {
		if (!silent)
			printk(KERN_ERR "FAT: bogus number of FAT structure\n");
		brelse(bh);
		goto out_invalid;
	}

	/*
	 * Earlier we checked here that b->secs_track and b->head are nonzero,
	 * but it turns out valid FAT filesystems can have zero there.
	 */

	media = b->media;
	if (!fat_valid_media(media)) {
		if (!silent)
			printk(KERN_ERR "FAT: invalid media value (0x%02x)\n",
			       media);
		brelse(bh);
		goto out_invalid;
	}

        // no. of bytes in one sector
        // block is the concept we use to read from block device
        // we read a block of data for one time;
        // block is kind of related to the physical property of the hardware

        // sector is the logic concept from FAT, its size is decided by 
        // FAT and written on the boot sector (here)
        // volume is divided into sectors, which in turn are
        // numbered from 0
	logical_sector_size = get_unaligned_le16(&b->sector_size);
	if (!is_power_of_2(logical_sector_size)
	    || (logical_sector_size < 512)
	    || (logical_sector_size > 4096)) {
		if (!silent)
			printk(KERN_ERR "FAT: bogus logical sector size %u\n",
			       logical_sector_size);
		brelse(bh);
		goto out_invalid;
	}
        // no. of sectors per allocation unit (cluster)
	sbi->sec_per_clus = b->sec_per_clus;
	if (!is_power_of_2(sbi->sec_per_clus)) {
		if (!silent)
			printk(KERN_ERR "FAT: bogus sectors per cluster %u\n",
			       sbi->sec_per_clus);
		brelse(bh);
		goto out_invalid;
	}

	if (logical_sector_size < sb->s_blocksize) {
		printk(KERN_ERR "FAT: logical sector size too small for device"
		       " (logical sector size = %u)\n", logical_sector_size);
		brelse(bh);
		goto out_fail;
	}
	if (logical_sector_size > sb->s_blocksize) {
		brelse(bh);

                // guarantee that block equals sector
		if (!sb_set_blocksize(sb, logical_sector_size)) {
			printk(KERN_ERR "FAT: unable to set blocksize %u\n",
			       logical_sector_size);
			goto out_fail;
		}
                // read the first sector
		bh = sb_bread(sb, 0);
		if (bh == NULL) {
			printk(KERN_ERR "FAT: unable to read boot sector"
			       " (logical sector size = %lu)\n",
			       sb->s_blocksize);
			goto out_fail;
		}
		b = (struct fat_boot_sector *) bh->b_data;
	}

        // one entry in the FAT corresponds to a cluster
	sbi->cluster_size = sb->s_blocksize * sbi->sec_per_clus;
        printk (KERN_INFO "myfat: sbi->cluster_size = %d\n", sbi->cluster_size);
	sbi->cluster_bits = ffs(sbi->cluster_size) - 1;
	sbi->fats = b->fats;
	sbi->fat_bits = 0;		/* Don't know yet */
	sbi->fat_start = le16_to_cpu(b->reserved);  // starting sector of FAT
	sbi->fat_length = le16_to_cpu(b->fat_length);
	sbi->root_cluster = 0;
	sbi->free_clusters = -1;	/* Don't know yet */
	sbi->free_clus_valid = 0;
	sbi->prev_free = FAT_START_ENT;  // is 2

	if (!sbi->fat_length && b->fat32_length) {
                // it's FAT 32
		struct fat_boot_fsinfo *fsinfo;
		struct buffer_head *fsinfo_bh;

		/* Must be FAT32 */
		sbi->fat_bits = 32;  // FAT 32
                // length of FAT (how many sectors one FAT has)
		sbi->fat_length = le32_to_cpu(b->fat32_length);
                // the cluster no. of the root directory
		sbi->root_cluster = le32_to_cpu(b->root_cluster);

		sb->s_maxbytes = 0xffffffff;

		/* MC - if info_sector is 0, don't multiply by 0 */
		sbi->fsinfo_sector = le16_to_cpu(b->info_sector);
		if (sbi->fsinfo_sector == 0)
			sbi->fsinfo_sector = 1;

		fsinfo_bh = sb_bread(sb, sbi->fsinfo_sector);
		if (fsinfo_bh == NULL) {
			printk(KERN_ERR "FAT: bread failed, FSINFO block"
			       " (sector = %lu)\n", sbi->fsinfo_sector);
			brelse(bh);
			goto out_fail;
		}

		fsinfo = (struct fat_boot_fsinfo *)fsinfo_bh->b_data;
		if (!IS_FSINFO(fsinfo)) {
			printk(KERN_WARNING "FAT: Invalid FSINFO signature: "
			       "0x%08x, 0x%08x (sector = %lu)\n",
			       le32_to_cpu(fsinfo->signature1),
			       le32_to_cpu(fsinfo->signature2),
			       sbi->fsinfo_sector);
		} else {
			if (sbi->options.usefree)
				sbi->free_clus_valid = 1;
			sbi->free_clusters = le32_to_cpu(fsinfo->free_clusters);
			sbi->prev_free = le32_to_cpu(fsinfo->next_cluster);
		}

		brelse(fsinfo_bh);
	}

        // the following code applies to FAT 12/16/32
	sbi->dir_per_block = sb->s_blocksize / sizeof(struct msdos_dir_entry);
	sbi->dir_per_block_bits = ffs(sbi->dir_per_block) - 1;

        // For FAT 12/16, it's the sector no. of the root dir
        // For FAT 32, it's the start of data region
	sbi->dir_start = sbi->fat_start + sbi->fats * sbi->fat_length;

        // for FAT12/16 use, the no. of entries of the Root Directory
	sbi->dir_entries = get_unaligned_le16(&b->dir_entries);
	if (sbi->dir_entries & (sbi->dir_per_block - 1)) {
        // sbi->dir_entries / sbi->dir_per_block should be integer
		if (!silent)
			printk(KERN_ERR "FAT: bogus directroy-entries per block"
			       " (%u)\n", sbi->dir_entries);
		brelse(bh);
		goto out_invalid;
	}

        // for FAT 32, rootdir_sectors is 0 since sbi->dir_entries is 0
	rootdir_sectors = sbi->dir_entries
		* sizeof(struct msdos_dir_entry) / sb->s_blocksize;

        // starting sector of data region
	sbi->data_start = sbi->dir_start + rootdir_sectors;
        
        // for FAT 32 b->sectors is 0
        // for FAT 32 it is the total no. of sectors of the volume
	total_sectors = get_unaligned_le16(&b->sectors);
	if (total_sectors == 0)
        // is FAT 32
		total_sectors = le32_to_cpu(b->total_sect);

        // the no. of clusters of the data region
	total_clusters = (total_sectors - sbi->data_start) / sbi->sec_per_clus;

        // We only support FAT 32
	if (sbi->fat_bits != 32)
        {
	    sbi->fat_bits = (total_clusters > MAX_FAT12) ? 16 : 12;
	    printk(KERN_ERR "myfat: FAT %d is not supported.\n", sbi->fat_bits);
	    brelse(bh);
            goto out_invalid;
        }
            
        // finally we can distinguish 12 / 16 / 32

	/* check that FAT table does not overflow */
        // how many entries the FAT can hold
	fat_clusters = sbi->fat_length * sb->s_blocksize * 8 / sbi->fat_bits;
        // The first two entries of FAT are special entries.
	total_clusters = min(total_clusters, fat_clusters - FAT_START_ENT);
	if (total_clusters > MAX_FAT((void*)3)) {
		if (!silent)
			printk(KERN_ERR "FAT: count of clusters too big (%u)\n",
			       total_clusters);
		brelse(bh);
		goto out_invalid;
	}

	sbi->max_cluster = total_clusters + FAT_START_ENT;
	/* check the free_clusters, it's not necessarily correct */
	if (sbi->free_clusters != -1 && sbi->free_clusters > total_clusters)
		sbi->free_clusters = -1;
	/* check the prev_free, it's not necessarily correct */
	sbi->prev_free %= sbi->max_cluster;
	if (sbi->prev_free < FAT_START_ENT)
		sbi->prev_free = FAT_START_ENT;

	brelse(bh);  // bh is of no use any more, so release it

	/* set up enough so that it can read an inode */
	fat_hash_init(sb);
	fat_ent_access_init(sb);

	/*
	 * The low byte of FAT's first entry must have same value with
	 * media-field.  But in real world, too many devices is
	 * writing wrong value.  So, removed that validity check.
	 *
	 * if (FAT_FIRST_ENT(sb, media) != first)
	 */

	error = -EINVAL;
	sprintf(buf, "cp%d", sbi->options.codepage);
        // load codepage
	sbi->nls_disk = load_nls(buf);
	if (!sbi->nls_disk) {
		printk(KERN_ERR "FAT: codepage %s not found\n", buf);
		goto out_fail;
	}

	/* FIXME: utf8 is using iocharset for upper/lower conversion */
	if (sbi->options.isvfat) {
		sbi->nls_io = load_nls(sbi->options.iocharset);
		if (!sbi->nls_io) {
			printk(KERN_ERR "FAT: IO charset %s not found\n",
			       sbi->options.iocharset);
			goto out_fail;
		}
	}

	error = -ENOMEM;
	fat_inode = new_inode(sb);
	if (!fat_inode)
		goto out_fail;
	MSDOS_I(fat_inode)->i_pos = 0;
	sbi->fat_inode = fat_inode;  // what is the fat_inode for? seems to have sth.
                                    // to do with entry. But I don't use it.
	root_inode = new_inode(sb);
	if (!root_inode)
		goto out_fail;
	root_inode->i_ino = MSDOS_ROOT_INO;  // defined by linux = 1
	root_inode->i_version = 1;
	error = fat_read_root(root_inode);
	if (error < 0)
		goto out_fail;
	error = -ENOMEM;
	insert_inode_hash(root_inode);  // add inode to kernel's inode_hashtable
	sb->s_root = d_alloc_root(root_inode);
	if (!sb->s_root) {
		printk(KERN_ERR "FAT: get root inode failed\n");
		goto out_fail;
	}

	return 0;

out_invalid:
	error = -EINVAL;
	if (!silent)
		printk(KERN_INFO "VFS: Can't find a valid FAT filesystem"
		       " on dev %s.\n", sb->s_id);

out_fail:
	if (fat_inode)
		iput(fat_inode);
	if (root_inode)
		iput(root_inode);
	if (sbi->nls_io)
		unload_nls(sbi->nls_io);
	if (sbi->nls_disk)
		unload_nls(sbi->nls_disk);
	if (sbi->options.iocharset != fat_default_iocharset)
		kfree(sbi->options.iocharset);
	sb->s_fs_info = NULL;
	kfree(sbi);
	return error;
}

EXPORT_SYMBOL_GPL(fat_fill_super);

