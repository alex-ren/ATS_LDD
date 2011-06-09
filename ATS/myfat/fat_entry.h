/*
* File for handling entry in FAT
*
*
*/

#ifndef ATS_FAT_ENTRY_H
#define ATS_FAT_ENTRY_H

#include <linux/fs.h>

// fat_entry describes the entry in the fat table.
struct fat_entry {
	int entry;  // no. of entry represented by this fat_entry
	union {
		u8 *ent12_p[2];
		__le16 *ent16_p;
		__le32 *ent32_p;  // value of the entry
	} u;
	int nr_bhs;  // no. of buffer_heads
	struct buffer_head *bhs[2];
	struct inode *fat_inode;
};

void fatent_init(struct fat_entry *fatent);

void fatent_set_entry(struct fat_entry *fatent, int entry);

void fatent_brelse(struct fat_entry *fatent);

void fat_ent_access_init(struct super_block *sb);


void fat_ent_blocknr(struct super_block *sb, int entry,
			    int *offset, sector_t *blocknr);

void fat32_ent_set_ptr(struct fat_entry *fatent, int offset);

int fat_ent_bread(struct super_block *sb, struct fat_entry *fatent,
			 int offset, sector_t blocknr);

int fat32_ent_get(struct fat_entry *fatent);

int fat32_ent_next(struct fat_entry *fatent);

void fat32_ent_put(struct fat_entry *fatent, int new);

/*
* Desc: get the no. of the next entry in the link list of FAT
* Info: can return FAT_ENT_EOF and FAT_ENT_FREE
* todo into ATS, the special kind of return value
*
*/
int fatent_next(struct inode *inode, int curent, int *nxtent);
int fatent_next_sb(struct super_block *sb, int curent, int *nxtent);

int fat_alloc_clusters(struct inode *inode, int *cluster,
			      int nr_cluster);
int fat_free_clusters(struct inode *inode, int cluster);

int fat_ent_read(struct inode *inode, struct fat_entry *fatent,
			int entry);
int fat_ent_write(struct inode *inode, struct fat_entry *fatent,
			 int new, int wait);

#endif


