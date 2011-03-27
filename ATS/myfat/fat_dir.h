#ifndef FAT_DIR_H
#define FAT_DIR_H

#include <linux/fs.h>

extern const struct file_operations fat_dir_operations;

/*
 * fat_subdirs counts the number of sub-directories of dir. It can be run
 * on directories being created.
 */
int fat_subdirs(struct inode *dir);


#endif


