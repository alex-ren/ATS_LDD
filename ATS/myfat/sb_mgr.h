#ifndef ATS_SB_MGR_H
#define ATS_SB_MGR_H

#include <linux/buffer_head.h>
#include <linux/string.h>
#include <linux/nls.h>
#include <linux/fs.h>
#include <linux/mutex.h>
#include <linux/msdos_fs.h>
/*
 * vfat shortname flags
 */
#define VFAT_SFN_DISPLAY_LOWER	0x0001 /* convert to lowercase for display */
#define VFAT_SFN_DISPLAY_WIN95	0x0002 /* emulate win95 rule for display */
#define VFAT_SFN_DISPLAY_WINNT	0x0004 /* emulate winnt rule for display */
#define VFAT_SFN_CREATE_WIN95	0x0100 /* emulate win95 rule for create */
#define VFAT_SFN_CREATE_WINNT	0x0200 /* emulate winnt rule for create */

#define FAT_ERRORS_CONT		1      /* ignore error and continue */
#define FAT_ERRORS_PANIC	2      /* panic on error */
#define FAT_ERRORS_RO		3      /* remount r/o on error */

struct fat_mount_options {
	uid_t fs_uid;
	gid_t fs_gid;
	unsigned short fs_fmask;
	unsigned short fs_dmask;
	unsigned short codepage;  /* Codepage for shortname conversions */
	char *iocharset;          /* Charset used for filename input/display */
	unsigned short shortname; /* flags for shortname display/create rule */
	unsigned char name_check; /* r = relaxed, n = normal, s = strict */
	unsigned char errors;	  /* On error: continue, panic, remount-ro */
	unsigned short allow_utime;/* permission for setting the [am]time */
	unsigned quiet:1,         /* set = fake successful chmods and chowns */
		 showexec:1,      /* set = only set x bit for com/exe/bat */
		 sys_immutable:1, /* set = system files are immutable */
		 dotsOK:1,        /* set = hidden and system files are named '.filename' */
		 isvfat:1,        /* 0=no vfat long filename support, 1=vfat support */
		 utf8:1,	  /* Use of UTF-8 character set (Default) */
		 unicode_xlate:1, /* create escape sequences for unhandled Unicode */
		 numtail:1,       /* Does first alias have a numeric '~1' type tail? */
		 flush:1,	  /* write things quickly */
		 nocase:1,	  /* Does this need case conversion? 0=need case conversion*/
		 usefree:1,	  /* Use free_clusters for FAT32 */
		 tz_utc:1,	  /* Filesystem timestamps are in UTC */
		 rodir:1;	  /* allow ATTR_RO for directory */
};

#define FAT_HASH_BITS	8
#define FAT_HASH_SIZE	(1UL << FAT_HASH_BITS)

/*
 * MS-DOS file system in-core superblock data
 */
struct fat_sb_info {
	unsigned short sec_per_clus; /* sectors/cluster */
	unsigned short cluster_bits; /* log2(cluster_size) */
	unsigned int cluster_size;   /* cluster size */
	unsigned char fats,fat_bits; /* number of FATs, FAT bits (12 or 16) */
	unsigned short fat_start;  // by rzq: number of the start sector of FAT
	unsigned long fat_length;    /* FAT start & length (sec.) */
	unsigned long dir_start;  // by rzq: no. of the start sector of the Root Directory [FAT12/16 only]
	unsigned short dir_entries;  /* root dir start & entries */  // by rzq: no. of entries in Root Directory [FAT12/16 only]
	unsigned long data_start;    /* first data sector */
	unsigned long max_cluster;   /* maximum cluster number */
	unsigned long root_cluster;  /* first cluster of the root directory */
	unsigned long fsinfo_sector; /* sector number of FAT32 fsinfo */
	struct mutex fat_lock;
	unsigned int prev_free;      /* previously allocated cluster number */
	unsigned int free_clusters;  /* -1 if undefined */
	unsigned int free_clus_valid; /* is free_clusters valid? */
	struct fat_mount_options options;
	struct nls_table *nls_disk;  /* Codepage used on disk */
	struct nls_table *nls_io;    /* Charset used for input and display */
	const void *dir_ops;		     /* Opaque; default directory operations */
	int dir_per_block;	     /* dir entries per block */
	int dir_per_block_bits;	     /* log2(dir_per_block) */

	int fatent_shift;  // by rzq: different value for FAT12, 16 and 32
	struct fatent_operations *fatent_ops;
	struct inode *fat_inode;

	spinlock_t inode_hash_lock;
        // by rzq: hold all those msdos_inode_info
        // the key is the address (in the data region) of the dir entry 
        // for that file
	struct hlist_head inode_hashtable[FAT_HASH_SIZE];  
};

static inline struct fat_sb_info *MSDOS_SB(struct super_block *sb)
{
	return sb->s_fs_info;
}

/*
 * Read the super block of an MS-DOS FS.
 */
int fat_fill_super(struct super_block *sb, void *data, int silent,
		   const struct inode_operations *fs_dir_inode_ops, int isvfat);


#endif


