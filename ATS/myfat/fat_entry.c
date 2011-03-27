#include "fat.h"
#include "sb_mgr.h"
#include "fat_entry.h"


// struct fatent_operations {
// 	void (*ent_blocknr)(struct super_block *, int, int *, sector_t *);
// 	void (*ent_set_ptr)(struct fat_entry *, int);
// 	int (*ent_bread)(struct super_block *, struct fat_entry *,
// 			 int, sector_t);
// 	int (*ent_get)(struct fat_entry *);
// 	void (*ent_put)(struct fat_entry *, int);
// 	int (*ent_next)(struct fat_entry *);
// };

void fatent_init(struct fat_entry *fatent)
{
	fatent->nr_bhs = 0;
	fatent->entry = 0;
	fatent->u.ent32_p = NULL;
	fatent->bhs[0] = fatent->bhs[1] = NULL;
	fatent->fat_inode = NULL;
}

void fatent_set_entry(struct fat_entry *fatent, int entry)
{
	fatent->entry = entry;
	fatent->u.ent32_p = NULL;
}

void fatent_brelse(struct fat_entry *fatent)
{
	int i;
	fatent->u.ent32_p = NULL;
	for (i = 0; i < fatent->nr_bhs; i++)
		brelse(fatent->bhs[i]);
	fatent->nr_bhs = 0;
	fatent->bhs[0] = fatent->bhs[1] = NULL;
	fatent->fat_inode = NULL;
}


/*
* Desc: find the block no. and offset for an entry in FAT
* Para:
* In:
*   entry: the no. of entry in FAT
* Out:
*   offset: offset in a block (in byte)
*   blocknr: block num
*/
void fat_ent_blocknr(struct super_block *sb, int entry,
			    int *offset, sector_t *blocknr)
{
    struct fat_sb_info *sbi = MSDOS_SB(sb);
    int bytes = (entry << sbi->fatent_shift);
    WARN_ON(entry < FAT_START_ENT || sbi->max_cluster <= entry);
    *offset = bytes & (sb->s_blocksize - 1);
    *blocknr = sbi->fat_start + (bytes >> sb->s_blocksize_bits);
}

void fat32_ent_set_ptr(struct fat_entry *fatent, int offset)
{
    WARN_ON(offset & (4 - 1));
    fatent->u.ent32_p = (__le32 *)(fatent->bhs[0]->b_data + offset);
}

/*
* Desc: read (a block) from volume, the block contains certain entry of FAT
* Para:
* In:
*   offset: offset in the block
*   blocknr: no. of block in the volume
* InOut:
*   fatent: a newly created fat_entry
*
*/
int fat_ent_bread(struct super_block *sb, struct fat_entry *fatent,
			 int offset, sector_t blocknr)
{
    WARN_ON(blocknr < MSDOS_SB(sb)->fat_start);
    fatent->fat_inode = MSDOS_SB(sb)->fat_inode;  // for what purpose?
    fatent->bhs[0] = sb_bread(sb, blocknr);
    if (!fatent->bhs[0]) {
    	printk(KERN_ERR "FAT: FAT read failed (blocknr %llu)\n",
    	       (llu)blocknr);
    	return -EIO;
    }
    fatent->nr_bhs = 1;
    fat32_ent_set_ptr(fatent, offset);
    return 0;
}

/*
* Desc: Get info of the current entry pointed by the fatent
* Ret: no. of clustr
*/
int fat32_ent_get(struct fat_entry *fatent)
{
    int next = le32_to_cpu(*fatent->u.ent32_p) & 0x0fffffff;
    WARN_ON((unsigned long)fatent->u.ent32_p & (4 - 1));
    if (next >= BAD_FAT32)
    	next = FAT_ENT_EOF;
    return next;
}

/*
* Desc: increase the no. of entry in the fat_entry
*/
int fat32_ent_next(struct fat_entry *fatent)
{
    const struct buffer_head *bh = fatent->bhs[0];
    fatent->entry++;
    if (fatent->u.ent32_p < (__le32 *)(bh->b_data + (bh->b_size - 4))) {
    	fatent->u.ent32_p++;
    	return 1;
    }
    fatent->u.ent32_p = NULL;
    return 0;
}

void fat32_ent_put(struct fat_entry *fatent, int new)
{
    return;  // todo
	// if (new == FAT_ENT_EOF)
	// 	new = EOF_FAT32;

	// WARN_ON(new & 0xf0000000);
	// new |= le32_to_cpu(*fatent->u.ent32_p) & ~0x0fffffff;
	// *fatent->u.ent32_p = cpu_to_le32(new);
	// mark_buffer_dirty_inode(fatent->bhs[0], fatent->fat_inode);
}

// static struct fatent_operations fat12_ops = {
// 	.ent_blocknr	= 0,  // fat12_ent_blocknr,
// 	.ent_set_ptr	= 0,  // fat12_ent_set_ptr,
// 	.ent_bread	= 0,  // fat12_ent_bread,
// 	.ent_get	= 0,  // fat12_ent_get,
// 	.ent_put	= 0,  // fat12_ent_put,
// 	.ent_next	= 0,  // fat12_ent_next,
// };
// 
// static struct fatent_operations fat16_ops = {
// 	.ent_blocknr	= 0,  // fat_ent_blocknr,
// 	.ent_set_ptr	= 0,  // fat16_ent_set_ptr,
// 	.ent_bread	= 0,  // fat_ent_bread,
// 	.ent_get	= 0,  // fat16_ent_get,
// 	.ent_put	= 0,  // fat16_ent_put,
// 	.ent_next	= 0,  // fat16_ent_next,
// };
// 
// static struct fatent_operations fat32_ops = {
// 	.ent_blocknr	= fat_ent_blocknr,
// 	.ent_set_ptr	= fat32_ent_set_ptr,
// 	.ent_bread	= fat_ent_bread,
// 	.ent_get	= fat32_ent_get,
// 	.ent_put	= fat32_ent_put,
// 	.ent_next	= fat32_ent_next,
// };
// 

void fat_ent_access_init(struct super_block *sb)
{
	struct fat_sb_info *sbi = MSDOS_SB(sb);
        printk (KERN_INFO "myfat: fat_ent_access_init\n");

	mutex_init(&sbi->fat_lock);

	switch (sbi->fat_bits) {
	case 32:
		sbi->fatent_shift = 2;
		sbi->fatent_ops = 0;  // &fat32_ops;
		break;
	case 16:  // this will not happen
		sbi->fatent_shift = 1;
		sbi->fatent_ops = 0;  // &fat16_ops;
		break;
	case 12: // this will not happen
		sbi->fatent_shift = -1;
		sbi->fatent_ops = 0;  // &fat12_ops;
		break;
	}
}


